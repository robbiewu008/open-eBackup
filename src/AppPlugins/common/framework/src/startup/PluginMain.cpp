/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <iostream>
#include <string>
#include <sstream>
#include <memory>
#include <cstdlib>
#include <thread>
#include <chrono>

#ifdef WIN32
#include <process.h>
#include <io.h>
#endif

#include "securec.h"

#include <thrift/TOutput.h>

#include "define/Defines.h"
#include "log/Log.h"
#include "common/EnvVarManager.h"
#include "common/JsonHelper.h"
#include "common/Path.h"
#include "common/File.h"
#include "CertCN.h"
#include "config_reader/ConfigIniReader.h"
#include "param_checker/ParamChecker.h"

#include "ExternalPluginSDK.h"

#include "PluginThriftServer.h"
#include "PluginThriftClient.h"
#include "PluginTypes.h"
#include "JobMgr.h"

#if !defined(_AIX) && !defined(SOLARIS)
/* Linux Crash Handler */
#include "AppStackTracer.h"
#endif

#ifdef WIN32
/* Win32 Crash Handler */
#include "DumpCollector.h"
#endif

#include "common/Thread.h"
#include "OpenLibMgr.h"
#include "PluginThriftClientConfig.h"
#include "PluginConfig.h"
#ifdef INTERNAL_PLUGIN_ENABLED
#include "curl_http/CurlHttpClient.h"
#endif

using namespace std;
using namespace common::jobmanager;
using namespace AppProtect;
using namespace apache::thrift;
using namespace startup;

namespace {
    constexpr int PLUGIN_ARG_BIN_PATH = 0;
    constexpr int PLUGIN_ARG_LOG_PATH = 1;
    constexpr int PLUGIN_ARG_START_PORT = 2;
    constexpr int PLUGIN_ARG_END_PORT = 3;
    constexpr int PLUGIN_ARG_AGENT_IP = 4;
    constexpr int PLUGIN_ARG_AGENT_PORT = 5;
    constexpr int PLUGIN_ARG_MAX = 6;
    constexpr int PLUGIN_START_PORT = 1025;
    constexpr int PLUGIN_END_PORT = 65535;
    constexpr int EXIT_SLEEP_TIME = 30;
    constexpr int HEART_BEAT_COUNT = 16; // 16 *  30s = 8min
    constexpr int AGENT_SDK_LOG_LEVEL = 1;
    const string PLUGIN_LOG_FILE = "AppPlugins.log";
    constexpr auto MODULE = "PluginMain";
#ifdef WIN32
    const string APP_LIB_JSON = "\\conf\\app_lib.json";
    const std::string PLUGIN_ATT_JSON = "\\conf\\plugin_attribute_1.0.0.json";
    const std::string PARAM_CHECK_XML = "\\conf\\param_check.xml";
    const std::string PROTECT_CLIENT_E_PATH = R"(\DataBackup\ProtectClient\ProtectClient-E)";
    const std::string THRIFT_CERT_PATH = R"(\DataBackup\ProtectClient\ProtectClient-E\nginx\conf\server.pem)";
#else
    const string APP_LIB_JSON = "/conf/app_lib.json";
    const std::string PLUGIN_ATT_JSON = "/conf/plugin_attribute_1.0.0.json";
    const std::string PARAM_CHECK_XML = "/conf/param_check.xml";
    const std::string PROTECT_CLIENT_E_PATH = "/DataBackup/ProtectClient/ProtectClient-E";
    const std::string THRIFT_CERT_PATH = "/DataBackup/ProtectClient/ProtectClient-E/nginx/conf/server.pem";
#endif

    const int MAX_ERR_MSG_LEN = 256;
    const int MAX_FULL_PATH_LEN = 300;
    const std::string MS_CFG_GENERAL_SECTION = "General";
    const std::string MS_CFG_LOG_LEVEL = "LogLevel";
    const std::string MS_CFG_LOG_COUNT = "LogCount";
    const std::string MS_CFG_LOG_MAX_SIZE = "LogMaxSize";

struct NasPluginStartParam {
    string fullBinPath;
    string logPath;
    string agentIP;
    uint32_t agentPort { 0 };
    uint32_t pluginStartPort { 0 };
    uint32_t pluginEndPort { 0 };
};

using AppInitFun = int(const std::string& logPath);

bool ParseStartParameter(int argc, const char** argv, NasPluginStartParam& startParam)
{
    stringstream ss;

    if (argc != PLUGIN_ARG_MAX) {
        return false;
    }

    startParam.fullBinPath = argv[PLUGIN_ARG_BIN_PATH];
    startParam.logPath = argv[PLUGIN_ARG_LOG_PATH];
    startParam.agentIP = argv[PLUGIN_ARG_AGENT_IP];

    ss << argv[PLUGIN_ARG_START_PORT];
    ss >> startParam.pluginStartPort;
    ss.clear();

    ss << argv[PLUGIN_ARG_END_PORT];
    ss >> startParam.pluginEndPort;
    ss.clear();

    ss << argv[PLUGIN_ARG_AGENT_PORT];
    ss >> startParam.agentPort;
    ss.clear();

    std::cout << "logPath: " << startParam.logPath << endl;
    std::cout << "startPort: " << startParam.pluginStartPort << endl;
    std::cout << "endPort: " << startParam.pluginEndPort << endl;
    std::cout << "FullBinPath: " << startParam.fullBinPath << endl;
    std::cout << "AgentIP: " << startParam.agentIP << endl;
    std::cout << "AgentPort: " << startParam.agentPort << endl;

    return true;
}

bool CheckStartParameter(const NasPluginStartParam& startParam)
{
    if (startParam.pluginStartPort > startParam.pluginEndPort) {
        return false;
    }
    if (startParam.pluginStartPort < PLUGIN_START_PORT || startParam.pluginStartPort > PLUGIN_END_PORT) {
        return false;
    }
    if (startParam.pluginEndPort < PLUGIN_START_PORT || startParam.pluginEndPort > PLUGIN_END_PORT) {
        return false;
    }
    return true;
}

void IsExit()
{
    int heartBeatCount = 0;
    for (;;) {
        // 30s 查询一次agent，16次查询失败（8min），就认为agent异常，停止所有任务
        ActionResult returnValue;
        // FrameworkService::HeartBeat(returnValue) // 先注释等待和agnet联调
        if (returnValue.code != 0) {
            ERRLOG("HeartBeat failed %d", heartBeatCount);
            heartBeatCount++;
        } else {
            DBGLOG("HeartBeat success");
            heartBeatCount = 0;
        }
        if (heartBeatCount == HEART_BEAT_COUNT) {
            // JobMgr::GetInstance().PauseAllJob()
            heartBeatCount = 0;
            WARNLOG("Pause all job due to loss connection to agent");
        }
        Module::SleepFor(std::chrono::seconds(EXIT_SLEEP_TIME));
    }
}

bool ConfigPlugin(const NasPluginStartParam& startParam)
{
    char fullBinPath[MAX_FULL_PATH_LEN + 1] = {0};
    int ret = strncpy_s(
        fullBinPath, sizeof(fullBinPath), startParam.fullBinPath.c_str(), startParam.fullBinPath.length());
    if (ret != Module::SUCCESS) {
        return false;
    }
    ret = Module::CPath::GetInstance().Init(fullBinPath);
    if (ret != Module::SUCCESS) {
        return false;
    }

    std::string confpath = Module::CPath::GetInstance().GetConfPath();
    int iLogLevel = Module::ConfigReader::getInt(string(MS_CFG_GENERAL_SECTION), string(MS_CFG_LOG_LEVEL));
    int iLogCount = Module::ConfigReader::getInt(string(MS_CFG_GENERAL_SECTION), string(MS_CFG_LOG_COUNT));
    int iLogMaxSize = Module::ConfigReader::getInt(string(MS_CFG_GENERAL_SECTION), string(MS_CFG_LOG_MAX_SIZE));

    Module::CLogger::GetInstance().Init(PLUGIN_LOG_FILE.c_str(), startParam.logPath);
    Module::CLogger::GetInstance().SetLogConf(iLogLevel, iLogCount, iLogMaxSize);

    INFOLOG("==== Plugin Start ===");
    INFOLOG("Plugin root path: %s", Module::CPath::GetInstance().GetRootPath().c_str());
    INFOLOG("Plugin log level %d, count %d, max size %d", iLogLevel, iLogCount, iLogMaxSize);
    INFOLOG("Plugin StartPort: %u", startParam.pluginStartPort);
    INFOLOG("Plugin EndPort: %u", startParam.pluginEndPort);
    INFOLOG("Plugin FullBinPath: %s", startParam.fullBinPath.c_str());
    INFOLOG("AgentIP: %s", startParam.agentIP.c_str());
    INFOLOG("AgentPort: %u", startParam.agentPort);
    return true;
}

bool InitPluginParam(NasPluginStartParam& startParam, int argc, const char** argv)
{
    if (!ParseStartParameter(argc, argv, startParam)) {
        HCP_Log(ERR, MODULE) << "Failed to parse start parameters" << HCPENDLOG;
        return false;
    }

    if (!CheckStartParameter(startParam)) {
        HCP_Log(ERR, MODULE) << "Invalid start parameters" << HCPENDLOG;
        return false;
    }

    if (!ConfigPlugin(startParam)) {
        HCP_Log(ERR, MODULE) << "Faild to configure log file" << HCPENDLOG;
        return false;
    }

    std::string xmlPath = Module::CPath::GetInstance().GetRootPath() + PARAM_CHECK_XML;
    Module::StructChecker::Instance().Init(xmlPath);
    return true;
}

void ConfigNasPluginClient(const NasPluginStartParam& startParam)
{
    PluginThriftClientConfig::GetInstance().Configure(startParam.agentIP, startParam.agentPort);
}

bool RegisterNasPlugin(const std::string& agentIp, uint32_t pluginPort)
{
    bool registerResult = false;
    ActionResult result;
    ApplicationPlugin plugin;

    plugin.name = PluginConfig::GetInstance().m_pluginName;
    plugin.endPoint = agentIp;
    plugin.port = pluginPort;
#ifdef WIN32
    plugin.processId = to_string(_getpid());
#else
    plugin.processId = to_string(getpid());
#endif
    INFOLOG("Plugin name: %s, pluginIp: %s, pluginPort: %u", plugin.name.c_str(), plugin.endPoint.c_str(), pluginPort);
    PluginThriftClient client;
    auto agentClient = client.GetAgentClient<RegisterPluginServiceConcurrentClient>("RegisterPluginService");
    if (agentClient == nullptr) {
        HCP_Log(ERR, MODULE) << "no found agentClient." << HCPENDLOG;
        return false;
    }
    try {
        agentClient->RegisterPlugin(result, plugin);
        registerResult = true;
    }  catch (TException& ex) {
        HCP_Log(ERR, MODULE) << "Thrift call exception : " << ex.what() << HCPENDLOG;
        registerResult = false;
    }
    return registerResult;
}

// dlopen打开应用动态库，不做dlclose，因为插件进程直接由agent关闭
bool GetPluginName()
{
    std::string pluginAttributeContent;
    std::string path = Module::CPath::GetInstance().GetRootPath() + PLUGIN_ATT_JSON;
    if (Module::CFile::ReadFile(path, pluginAttributeContent) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Read attribute file error: " << path << HCPENDLOG;
        return false;
    }
    Json::Value js;
    if (!Module::JsonHelper::JsonStringToJsonValue(pluginAttributeContent, js)) {
        HCP_Log(ERR, MODULE) << "JsonStringToJsonValue error, str" <<
            WIPE_SENSITIVE(pluginAttributeContent) << HCPENDLOG;
        return false;
    }
    std::string pluginName = js["name"].asString();
    if (pluginName.empty()) {
        ERRLOG("Plugin name is empty");
        return false;
    }
    HCP_Log(INFO, MODULE) << "app plugin name: " << pluginName << HCPENDLOG;
    PluginConfig::GetInstance().m_pluginName = pluginName;
    return true;
}

std::string GetWholeLibName(const std::string& libNamePrefix)
{
#ifdef WIN32
    std::string libName = libNamePrefix + ".dll";
#elif defined(_AIX)
    std::string libName = libNamePrefix + ".a(" + libNamePrefix + ".so)";
#else
    std::string libName = libNamePrefix + ".so";
#endif
    return libName;
}

// dlopen打开应用动态库，不做dlclose，因为插件进程直接由agent关闭
bool GetAppLibName()
{
    std::string libContent;
    if (Module::CFile::ReadFile(
        Module::CPath::GetInstance().GetRootPath() + APP_LIB_JSON, libContent) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Read lib attribute file error." << HCPENDLOG;
        return false;
    }
    Json::Value jsLib;
    if (!Module::JsonHelper::JsonStringToJsonValue(libContent, jsLib)) {
        HCP_Log(ERR, MODULE) << "JsonStringToJsonValue error, str" << libContent << HCPENDLOG;
        return false;
    }
    std::string pluginName = PluginConfig::GetInstance().m_pluginName;
    std::string libNamePrefix;
    bool checkJsLIb = jsLib.isObject() && jsLib.isMember("Plugin");
    if (!checkJsLIb) {
        ERRLOG("jsLib is not a effective json memeber");
        return false;
    }
    Json::Value plugin = jsLib["Plugin"];
    for (unsigned int i = 0; i < plugin.size(); i++) {
        if (plugin[i].isObject() && plugin[i].isMember("PluginName") && plugin[i]["PluginName"].isString() &&
            plugin[i]["PluginName"].asString() == pluginName) {
            libNamePrefix = plugin[i]["LibName"].asString();
            break;
        }
    }
    if (libNamePrefix.empty()) {
        HCP_Log(ERR, MODULE) << "Not get lib name from app_lib.json" << HCPENDLOG;
        return false;
    }
    std::string libName = GetWholeLibName(libNamePrefix);
    INFOLOG("Plugin lib name is %s", libName.c_str());
    PluginConfig::GetInstance().m_libName = libName;
    if (!(jsLib.isMember("PluginUsageScene") && jsLib["PluginUsageScene"].isString())) {
        return false;
    }
    std::string scene = jsLib["PluginUsageScene"].asString();
    INFOLOG("Plugin usage scene in app_lib.json is %s", scene.c_str());
    if (scene == "external") {
        PluginConfig::GetInstance().m_scene = PluginUsageScene::EXTERNAL;
    } else if (scene == "internal") {
        PluginConfig::GetInstance().m_scene = PluginUsageScene::INTERNAL;
        Module::ConfigReader::ReadLogLevelFromPM(true);
    } else {
        ERRLOG("Plugin usage scene in app_lib.json is %s", scene.c_str());
        return false;
    }
    return true;
}

bool InitLibHandle()
{
    std::string libName = PluginConfig::GetInstance().m_libName;
#ifdef WIN32
    std::string fullPath = Module::CPath::GetInstance().GetRootPath() + "\\bin\\" + libName;
#else
    std::string fullPath = Module::CPath::GetInstance().GetRootPath() + "/lib/service/" + libName;
#endif
    HCP_Log(INFO, MODULE) << "app lib full path: " << fullPath << HCPENDLOG;
    bool result;
    if (PluginConfig::GetInstance().m_pluginName == "VirtualizationPlugin") {
        HCP_Log(INFO, MODULE) << "VirtualizationPlugin InitLibHandleEx global." << HCPENDLOG;
        result = OpenLibMgr::GetInstance().InitLibHandleEx(fullPath, false);
    } else {
        result = OpenLibMgr::GetInstance().InitLibHandle(fullPath);
    }
    if (!result) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get lib handle fail: " << Module::DlibError(errMsg, sizeof(errMsg))<< HCPENDLOG;
        return false;
    }
    return true;
}

bool InvokeAppInit(const std::string& logPath)
{
    HCP_Log(INFO, MODULE) << "Enter AppInitFun" << HCPENDLOG;
    auto fun = OpenLibMgr::GetInstance().GetObj<AppInitFun>("AppInit");
    if (fun == nullptr) {
        char errMsg[MAX_ERR_MSG_LEN] = {0};
        HCP_Log(ERR, MODULE) << "Get AppInit function failed error" <<
            Module::DlibError(errMsg, sizeof(errMsg)) << HCPENDLOG;
            return false;
    }
    int ret = fun(logPath);
    if (ret != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Failed to init app" << HCPENDLOG;
        return false;
    }
    return true;
}

bool InitApp(const std::string& logPath)
{
    if (!GetPluginName()) {
        ERRLOG("GetPluginName failed");
        return false;
    }

    if (!GetAppLibName()) {
        ERRLOG("GetAppLibName failed");
        return false;
    }

    if (!InitLibHandle()) {
        ERRLOG("InitLibHandle failed");
        return false;
    }

    if (!InvokeAppInit(logPath)) {
        ERRLOG("InvokeAppInit failed");
        return false;
    }
    INFOLOG("Init app lib success");
    return true;
}

void ThriftOutputLog(const char* message)
{
    Module::CLogger::GetInstance().Log(DEBUG, 0, "Thrift", "Thrift", message);
}

void WriteLogCallBack(int32_t level, const std::string& filePathName,
    int32_t lineNum, const std::string& funcName, const std::string& logString)
{
#ifdef WIN32
    Module::CLogger::GetInstance().Log(
        level, lineNum, filePathName.c_str(), funcName.c_str(), logString.c_str());
#else
    Module::CLogger::GetInstance().Log(level, lineNum, filePathName, funcName, logString);
#endif
}

// TO打桩留给内置插件
bool InitKmc()
{
#ifdef INTERNAL_PLUGIN_ENABLED
    // 内置agent共用一套kmc初始化机制
    if (!Module::InitKmcInstance()) {
        HCP_Log(ERR, MODULE) << "Init Kmc Instance failed!" << HCPENDLOG;
        return false;
    }
#endif
    return true;
}

bool InitAgentSDK()
{
    OpaCallbacks callBack {};
    callBack.writeLog = WriteLogCallBack;
    OpaRegFunc(callBack);

    auto scene = PluginConfig::GetInstance().m_scene;
    HCP_Log(DEBUG, MODULE) << "agent plugin type: " << static_cast<int>(scene) << HCPENDLOG;
    if (scene == PluginUsageScene::EXTERNAL) {
        string protectClientEPath = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + PROTECT_CLIENT_E_PATH;
        INFOLOG("Init agent SDK, ProtectClient-E path: %s", protectClientEPath.c_str());
        if (OpaInitialize(protectClientEPath) != Module::SUCCESS) {
            ERRLOG("OpaInitialize failed");
            return false;
        }
    } else {
        if (!InitKmc()) {
            ERRLOG("Init kmc for interal sence failed");
            return false;
        }
        if (OpaInitialize("", false) != Module::SUCCESS) {
            ERRLOG("OpaInitialize failed");
            return false;
        }
    }
    return true;
}

bool StartThriftServer(const NasPluginStartParam& startParam)
{
    // 遍历并使用可用的端口
    uint32_t port = startParam.pluginStartPort;
    string certCN;
    bool isServerStart = false;
    string thriftCertFile = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + THRIFT_CERT_PATH;
    GetHostFromCert(thriftCertFile, certCN);
    do {
        PluginThriftServer::GetInstance().Configure(certCN, port);
        if (PluginThriftServer::GetInstance().Start()) {
            isServerStart = true;
            break;
        }
    } while (++port <= startParam.pluginEndPort);
    if (!isServerStart) {
        ERRLOG("Start plugin server failed");
        return false;
    }
    // 向agent注册插件
    ConfigNasPluginClient(startParam);
    if (!RegisterNasPlugin(certCN, port)) {
        HCP_Log(ERR, MODULE) << "!" << HCPENDLOG;
        ERRLOG("Register plugin to agent failed");
        return false;
    }
    INFOLOG("Start plugin server and register to agent success");
    return true;
}
}

// 1: log path 2: min port 3: max port 4: agent thrift server IP 5: agent thrift server port
int main(int argc, const char** argv)
{
    NasPluginStartParam startParam;
    // 解析插件入参
    if (!InitPluginParam(startParam, argc, argv)) {
        return Module::FAILED;
    }

    // print stack in Plugin_lle.log
#ifdef WIN32
    wincrash::DumpCollector::Init();
    if (!wincrash::DumpCollector::SetDumpFileRoot(startParam.logPath)) {
        ERRLOG("dumpcollector failed to set root %s, will use default root!", startParam.logPath.c_str());
    }
#elif defined(_AIX) || defined (SOLARIS)
    WARNLOG("AIX/Solaris System, no tracker.");
#else
    AppStackTracer stackTracer(startParam.logPath);
    stackTracer.Init();
#endif
    if (!InitApp(startParam.logPath)) {
        HCP_Log(ERR, MODULE) << "Faild to init app lib" << HCPENDLOG;
        return Module::FAILED;
    }
    HCP_Log(DEBUG, MODULE) << "Success nas plugin configuration." << HCPENDLOG;

    // init sdk and kmc
    if (!InitAgentSDK()) {
        return Module::FAILED;
    }

    // start JogMgr monitor
    if (JobMgr::GetInstance().StartMonitorJob() != Module::SUCCESS) {
        return Module::FAILED;
    }

    // inti thrift log output
    apache::thrift::GlobalOutput.setOutputFunction(ThriftOutputLog);

    // start thrift server
    if (!StartThriftServer(startParam)) {
        ERRLOG("StartThriftServer failed");
        return Module::FAILED;
    }

    // 保持插件进程常驻和agent保持心跳
    IsExit();

    return Module::FAILED;
}
