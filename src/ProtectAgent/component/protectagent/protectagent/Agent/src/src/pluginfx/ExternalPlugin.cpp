/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file PluginRunController.h
 * @brief  The implemention about ExternalPlugin.h
 * @version 1.0.0.0
 * @date 2021-10-13
 * @author jwx966562
 */
#include <fstream>
#include <mutex>
#include "common/Defines.h"
#ifdef WIN32
#include <tlhelp32.h>
#endif
#include <chrono>
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "thriftservice/detail/ThriftClient.h"
#include "servicefactory/include/ServiceFactory.h"
#include "thriftservice/include/IThriftService.h"
#include "common/Utils.h"
#include "certificateservice/include/ICertificateService.h"
#include "securecom/SecureUtils.h"
#include "common/ErrorCode.h"
#include "pluginfx/ExternalPlugin.h"

using namespace servicecenter;
using namespace thriftservice;
using namespace thriftservice::detail;
using namespace AppProtect;
using namespace std;

namespace {
    // 插件注册后，如果第一次无法连上插件，后续定期连接的间隔
    const mp_int32 RECONNECT_EXTERNAL_PLUGIN_STATUS_INTERVAL = 10 * 1000;
    const mp_int32 HB_INTERFACE_TIMEOUT = 30 * 1000; // 心跳接口超时时间
    const mp_int32 HB_INTERVAL = 60; // 心跳间隔 60 * 1s
    const mp_int32 HB_FAILED_NUM = 3; // 心跳失败的次数 3
    const mp_string DEFAULT_LISTEN_IP = "127.0.0.1";
    const mp_string VIRTUAL_PLUGIN_NAME = "VirtualizationPlugin";
    constexpr int32_t MAX_PLUGIN_NO_USE_INTERVAL = 10;
}

ExternalPlugin::ExternalPlugin(const std::string &pluginName, const mp_string &startUser, bool ssl, mp_int32 serverPort)
    : m_ssl(ssl), m_thriftServerPort(serverPort)
{
    m_currentState = std::make_unique<PluginIdleState>(this);
    m_pluginInfo.name = pluginName;
    m_startUser = startUser;
}

ExternalPlugin::ExternalPlugin(const ExternalPlugin& plugin)
{
    m_pluginInfo.name = plugin.m_pluginInfo.name;
    m_startUser = plugin.m_startUser;
    m_thriftServerPort = plugin.m_thriftServerPort;
    m_restCounter = std::atomic_load(&plugin.m_restCounter);
    m_taskCounter = std::atomic_load(&plugin.m_taskCounter);
    m_ssl = plugin.m_ssl;
    m_currentState = std::make_unique<PluginIdleState>(this);
    m_cpuLimit = plugin.m_cpuLimit;
    m_memoryLimit = plugin.m_memoryLimit;
    m_blkioWeight = plugin.m_blkioWeight;
}

ExternalPlugin::~ExternalPlugin()
{
    // 关闭与插件服务端连接
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftservice->UnRegisterClient(m_pluginInfo.endPoint, m_pluginInfo.port);
}

void ExternalPlugin::ChangeStatus(EX_PLUGIN_STATUS status)
{
    INFOLOG("External plugin ChangeStatus :%d plugin:%s", static_cast<int32_t>(status), m_pluginInfo.name.c_str());
    std::lock_guard<std::mutex> lock(m_statusMutex);
    switch (status) {
        case EX_PLUGIN_STATUS::STARTING: {
            m_currentState = std::make_unique<PluginStartingState>(this);
            break;
        }
        case EX_PLUGIN_STATUS::ISREGISTERED: {
            m_currentState = std::make_unique<PluginRegisterdState>(this);
            m_waitCond.notify_all();
            break;
        }
        case EX_PLUGIN_STATUS::CLOSING: {
            m_currentState = std::make_unique<PluginClosingState>(this);
            break;
        }
        default:
            break;
    }
    m_runningStatus = status;
}

mp_void ExternalPlugin::SavePid(const ApplicationPlugin &pluginInfo)
{
    std::ofstream stream;
    mp_string pidDir = CPath::GetInstance().GetPluginPidPath();
    if (!CMpFile::DirExist(pidDir.c_str())) {
        ERRLOG("Plugin pid dir(%s) does not exist.", pidDir.c_str());
        return;
    }
    mp_string pidPath = pidDir + PATH_SEPARATOR + m_pluginInfo.name;
    INFOLOG("Save plug(%s) pid(%s).", pluginInfo.name.c_str(), pluginInfo.processId.c_str());
    try {
        stream.open(pidPath.c_str(), std::ifstream::out);
        if (!stream.is_open()) {
            WARNLOG("Failed to open file(%s).", pidPath.c_str());
        } else {
            stream << pluginInfo.processId.c_str();
            stream.flush();
            stream.close();
#ifndef WIN32
            (void)ChmodFile(pidPath, S_IRUSR | S_IWUSR);
#endif
        }
    } catch (std::exception& e) {
        ERRLOG("Hanle file open occur exception(%s).", e.what());
    }
}

void ExternalPlugin::SetPluginInfo(const ApplicationPlugin &pluginInfo)
{
    m_pluginInfo = pluginInfo;
#ifdef LINUX
    ostringstream scriptParam;
    scriptParam << "PluginName=" << m_pluginInfo.name << NODE_COLON << "PluginPID=" << m_pluginInfo.processId
        << NODE_COLON << "CpuLimit=" << m_cpuLimit << NODE_COLON << "MemoryLimit=" << m_memoryLimit
        << NODE_COLON << "BlkioWeight=" << m_blkioWeight;
    CRootCaller rootCaller;
    if (rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_SET_CGROUP, scriptParam.str(), NULL) != MP_SUCCESS) {
        WARNLOG("Limit Plugin cpu or memory Failed.");
    }
#endif
    SavePid(pluginInfo);
}

mp_uint32 ExternalPlugin::StopPlugin()
{
    return (m_currentState.get() ? m_currentState->PluginClosing() : MP_FAILED);
}

mp_uint32 ExternalPlugin::PluginRegistered()
{
    uint32_t ret =  (m_currentState.get() ? m_currentState->PluginRegistered() : MP_FAILED);
    if (ret != MP_SUCCESS) {
        ERRLOG("External plugin %s switch to registered status failed.", m_pluginInfo.name.c_str());
    } else {
        INFOLOG("External plugin Registered :%s", m_pluginInfo.name.c_str());
    }
    return ret;
}

mp_uint32 ExternalPlugin::PluginUnregistered()
{
    return (m_currentState.get() ? m_currentState->PluginClosed() : MP_FAILED);
}

mp_string ExternalPlugin::GetExternalScriptPath(const mp_string &pluginName)
{
    return CPath::GetInstance().GetRootPath() + PATH_SEPARATOR + ".." + PATH_SEPARATOR +
            EXTERNAL_PLUGIN_PATH + PATH_SEPARATOR + pluginName + PATH_SEPARATOR;
}

mp_uint32 ExternalPlugin::StartPlugin()
{
    return (m_currentState.get() ? m_currentState->PluginStarting() : MP_FAILED);
}

mp_uint32 ExternalPlugin::ExecStopPlugin()
{
    LOGGUARD("");
    // 组装停止脚本路径
    mp_string stopScriptPath = GetExternalScriptPath(m_pluginInfo.name) + EXTERNAL_PLUGIN_STOP_SCRIPT;
    if (!CMpFile::FileExist(stopScriptPath)) {
        ERRLOG("Plugin start script(%s) not exist.", stopScriptPath.c_str());
        return KillProcess();
    }
#ifdef WIN32
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(stopScriptPath);
#else
    CHECK_FAIL_EX(CheckCmdDelimiter(stopScriptPath));
    mp_uint32 iRet = MP_SUCCESS;
    if (m_startUser == AGENT_ROOT_USER) {
        // 如果以root用户启动
        CRootCaller rootCaller;
        iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, stopScriptPath);
    } else if (m_startUser == AGENT_RUNNING_USER) {
        // 如果以rdadmin用户启动
        iRet = CSystemExec::ExecSystemWithoutEcho(stopScriptPath);
    } else if (m_startUser == EXAGENT_RUNNING_USER && m_pluginInfo.name == VIRTUAL_PLUGIN_NAME) {
        // 虚拟化插件在内置代理以exrdadmin用户启动
        CRootCaller rootCaller;
        iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, stopScriptPath);
    } else {
        ERRLOG("Can not stop plugin %s with user %s.", m_pluginInfo.name.c_str(), m_startUser.c_str());
        return MP_FAILED;
    }
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Fail to exec stop script, %s.", stopScriptPath.c_str());
        return MP_FAILED;
    }
    CMpFile::DelFile(CPath::GetInstance().GetPluginPidPath() + PATH_SEPARATOR + m_pluginInfo.name);
    INFOLOG("Exec plugin stop script(%s) success.", stopScriptPath.c_str());
    return MP_SUCCESS;
}

bool ExternalPlugin::IsPluginProcessExist()
{
#ifdef WIN32
    mp_uint64 uint64_ProcessPid = 0;
    CMpString::StringToUInteger(m_pluginInfo.processId, uint64_ProcessPid);
    mp_uint32 uint32_ProcessPid = static_cast<mp_uint32>(uint64_ProcessPid);

    mp_semaf hProcess = ::OpenProcess(PROCESS_TERMINATE, MP_FAILED, uint32_ProcessPid);
    if (hProcess) {
        CloseHandle(hProcess);
        return MP_TRUE;
    } else {
        return MP_FALSE;
    }
#else
    std::string processPath = "/proc/" + m_pluginInfo.processId;
    if (!CMpFile::DirExist(processPath.c_str())) {
        INFOLOG("Plugin:%s,pid:%s has already been killed.", m_pluginInfo.name.c_str(),
            m_pluginInfo.processId.c_str());
        return false;
    }
    return true;
#endif
}

bool ExternalPlugin::IsPluginResponding()
{
    if (++m_heartbeatNum < HB_INTERVAL) {
        return true;
    }
    LOGGUARD("");
    m_heartbeatNum = 0;
    thriftservice::ClientSocketOpt opt = { m_pluginInfo.endPoint, m_pluginInfo.port,
        HB_INTERFACE_TIMEOUT, HB_INTERFACE_TIMEOUT, HB_INTERFACE_TIMEOUT };
    opt.msgType = thriftservice::THRIFT_MSG_TYPE_HEARTBEAT;
    std::shared_ptr<thriftservice::IThriftClient> heartClient = GetPluginClient(std::move(opt));
    if (heartClient == nullptr) {
        ERRLOG("Get heartbeat client failed. pluginName:%s", m_pluginInfo.name.c_str());
        return false;
    }
    auto pluginServiceClient = heartClient->GetConcurrentClientIf<PluginServiceConcurrentClient>("PluginService");
    ApplicationPlugin _return;
    try {
        pluginServiceClient->QueryPlugin(_return);
        m_noRespondNum = 0;
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "heartbeat, TTransportException. %s", ex.what());
        ++m_noRespondNum;
    } catch (const std::exception &ex) {
        COMMLOG(OS_LOG_ERROR, "heartbeat, Standard C++ Exception. %s", ex.what());
        ++m_noRespondNum;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "heartbeat, Unknown exception.");
        m_noRespondNum++;
    }
    return m_noRespondNum < HB_FAILED_NUM;
}


mp_uint32 ExternalPlugin::KillProcess()
{
    // 查询进程是否存在
    if (!IsPluginProcessExist()) {
        return MP_SUCCESS;
    }
    mp_uint32 iRet = MP_FAILED;
#ifdef WIN32
    mp_string strCommand = "cmd.exe /c taskkill /f /pid " + m_pluginInfo.processId + " > nul";
    iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
#else
    std::string param = " " + m_pluginInfo.processId;
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_uint32)ROOT_COMMAND_KILL, param, NULL);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Fail to kill process %s plugin:%s.", m_pluginInfo.processId.c_str(), m_pluginInfo.name.c_str());
    }
    CMpFile::DelFile(CPath::GetInstance().GetPluginPidPath() + PATH_SEPARATOR + m_pluginInfo.name);
    return iRet;
}

EXTER_ATTACK mp_uint32 ExternalPlugin::ExecStartPlugin()
{
    // 组装插件安装脚本路径
    mp_string startScriptPath = GetExternalScriptPath(m_pluginInfo.name) + EXTERNAL_PLUGIN_START_SCRIPT;
    if (!CMpFile::FileExist(startScriptPath)) {
        ERRLOG("Plugin start script(%s) not exist.", startScriptPath.c_str());
        return MP_FAILED;
    }
    // 获取参数
    mp_string params;
    mp_uint32 iRet = GenerateParam(params);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Generate start script params failed.");
        return iRet;
    }
#ifdef WIN32
    startScriptPath = "\"" + startScriptPath + "\"" + " " + params;
    iRet = CSystemExec::ExecSystemWithoutEcho(startScriptPath);
#else
    CHECK_FAIL_EX(CheckCmdDelimiter(startScriptPath));
    if (m_startUser == AGENT_ROOT_USER) {
        // 如果以root用户启动
        startScriptPath = "\"" + startScriptPath + " " + params + "\"";
        CRootCaller rootCaller;
        iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, startScriptPath);
    } else if (m_startUser == AGENT_RUNNING_USER) {
        // 如果以rdadmin用户启动
        startScriptPath = CMpString::BlankComma(startScriptPath);
        startScriptPath = startScriptPath + mp_string(" ") + params;
        iRet = CSystemExec::ExecSystemWithoutEcho(startScriptPath);
    } else if (m_startUser == EXAGENT_RUNNING_USER && m_pluginInfo.name == VIRTUAL_PLUGIN_NAME) {
        // 虚拟化插件在内置代理可以exrdadmin(22222) 启动
        startScriptPath = "\"" + startScriptPath + " " + params + "\"";
        CRootCaller rootCaller;
        iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, startScriptPath);
    } else {
        ERRLOG("Can not start plugin %s with user %s.", m_pluginInfo.name.c_str(), m_startUser.c_str());
        return MP_FAILED;
    }
#endif

    if (iRet != MP_SUCCESS) {
        ERRLOG("Fail to exec start script.");
        return MP_FAILED;
    }
    INFOLOG("Exec plugin start script %s success.", startScriptPath.c_str());
    return MP_SUCCESS;
}

mp_uint32 ExternalPlugin::GenerateParam(mp_string &params)
{
    // 获取启动插件参数：可用的端口范围，绑定的thrift端口
    mp_int32 startPort;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION,
        CFG_PLUGIN_START_PORT, startPort);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get plugin for %s start port failed.", m_pluginInfo.name.c_str());
        return iRet;
    }
    mp_int32 endPort;
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, CFG_PLUGIN_END_PORT, endPort);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get plugin for %s end port failed.", m_pluginInfo.name.c_str());
        return iRet;
    }
    // 准备启动参数: 日志目录 + 开始端口 + 结束端口 + Agent的RPC IP + Agent的RPC端口
    mp_string logPath;
#ifdef WIN32
    logPath = CPath::GetInstance().GetExPluginLogPath() + PATH_SEPARATOR + m_pluginInfo.name;
#else
    if (m_startUser == AGENT_ROOT_USER) {
        // 如果以root用户启动
        logPath = CPath::GetInstance().GetExPluginSlogPath() + PATH_SEPARATOR + m_pluginInfo.name;
    } else if (m_startUser == AGENT_RUNNING_USER) {
        // 如果以rdadmin用户启动
        logPath = CPath::GetInstance().GetExPluginLogPath() + PATH_SEPARATOR + m_pluginInfo.name;
    } else if (m_startUser == EXAGENT_RUNNING_USER && m_pluginInfo.name == VIRTUAL_PLUGIN_NAME) {
        // 虚拟化插件在内置代理可以exrdadmin(22222) 启动
        logPath = CPath::GetInstance().GetExPluginLogPath() + PATH_SEPARATOR + m_pluginInfo.name;
    } else {
        ERRLOG("Can not start plugin %s with user %s.", m_pluginInfo.name.c_str(), m_startUser.c_str());
        return MP_FAILED;
    }
#endif
    params = logPath + " " + CMpString::to_string(startPort) + " " + CMpString::to_string(endPort) + " "
        + DEFAULT_LISTEN_IP + " " + CMpString::to_string(m_thriftServerPort);

    INFOLOG("Genterate start script params(%s) success.", params.c_str());
    return MP_SUCCESS;
}

std::shared_ptr<thriftservice::IThriftClient> ExternalPlugin::GetThriftClientFromService(
    const thriftservice::ClientSocketOpt& opt)
{
    auto thriftservice = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    std::shared_ptr<thriftservice::IThriftClient> thriftclient;
    if (m_ssl) {
        auto certifiService =
            ServiceFactory::GetInstance()->GetService<certificateservice::ICertificateService>("ICertificateService");
        auto certHandler = certifiService->GetCertificateHandler();
        thriftclient = thriftservice->RegisterSslClient(opt, certHandler);
    } else {
        thriftclient = thriftservice->RegisterClient(opt);
    }
    return thriftclient;
}

void ExternalPlugin::SetThriftClientConfig(thriftservice::ClientSocketOpt& opt)
{
    mp_int32 timout = thriftservice::THRIFT_CLIENT_TIMEOUT_DEFAULT;
    if (opt.msgType == thriftservice::THRIFT_MSG_TYPE_HEARTBEAT) {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
            CFG_THRIFT_SECTION, CFG_THRIFT_PLUGIN_HEARTBEAT_TIMEOUT, timout);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Get config thrift client heartbeat timeout fail.");
            timout = HB_INTERFACE_TIMEOUT;
        }
    } else {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
            CFG_THRIFT_SECTION, CFG_THRIFT_CLIENT_SEND_TIMEOUT, timout);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Get config thrift client timeout fail.");
            timout = thriftservice::THRIFT_CLIENT_TIMEOUT_DEFAULT;
        }
    }
    DBGLOG("Get config thrift client timeout is %d.", timout);
    opt.connTimeout = timout;
    opt.recvTimeout = timout;
    opt.sendTimeout = timout;
}

std::shared_ptr<thriftservice::IThriftClient> ExternalPlugin::GetPluginClient(thriftservice::ClientSocketOpt opt)
{
    {
        DBGLOG("Check plugin registered or timeout");
        std::unique_lock<std::mutex> lock(m_statusMutex);
        m_waitCond.wait(lock, [this] {
            return m_waitTimeout == 0 || IsPluginRunning();
        });
        if (!IsPluginRunning()) {
            ERRLOG("Plugin register timeout, plugin:[%s]", m_pluginInfo.name.c_str());
            ExecStopPlugin();
            return nullptr;
        }
    }

    if (m_pluginInfo.port == 0 || m_pluginInfo.endPoint.empty()) {
        ERRLOG("Plugin info error, plugin:[%s]", m_pluginInfo.name.c_str());
        return nullptr;
    }
    opt.host = m_pluginInfo.endPoint;
    opt.port = m_pluginInfo.port;
    SetThriftClientConfig(opt);
    std::shared_ptr<thriftservice::IThriftClient> thriftclient = GetThriftClientFromService(opt);
    if (thriftclient && thriftclient->Start()) {
        DBGLOG("Start the thrift client success");
        return thriftclient;
    } else {
        ERRLOG("Start the thrift client Failed");
        return nullptr;
    }
}

/* 当前插件没有任务和rest请求是否已经超过超时时间 */
bool ExternalPlugin::IsNoUseTimeout()
{
    return (!IsUsing()) &&
           (std::chrono::steady_clock::now() - m_noUsePoint) > std::chrono::minutes(MAX_PLUGIN_NO_USE_INTERVAL);
}

bool ExternalPlugin::IsUsing()
{
    DBGLOG("ExternalPlugin TaskCounter:%d rest Counter:%d", std::atomic_load(&m_taskCounter),
        std::atomic_load(&m_restCounter));
    return m_taskCounter || m_restCounter > 0;
}

void ExternalPlugin::AddTaskCounter()
{
    m_taskCounter = true;
}

void ExternalPlugin::ReduceTaskCounter()
{
    m_taskCounter = false;
    // record start time for stopping this plugin
    if (!IsUsing()) {
        m_noUsePoint = std::chrono::steady_clock::now();
    }
}

void ExternalPlugin::AddRestCounter()
{
    ++m_restCounter;
}

void ExternalPlugin::ReduceRestCounter()
{
    --m_restCounter;
    // record start time for stopping this plugin
    if (!IsUsing()) {
        m_noUsePoint = std::chrono::steady_clock::now();
    }
}

bool ExternalPlugin::IsPluginRunning()
{
    return GetRunningStatus() == EX_PLUGIN_STATUS::ISREGISTERED;
}

bool ExternalPlugin::IsWaitPluginRegister()
{
    return m_waitTimeout > 0;
}

bool ExternalPlugin::CheckPluginRegister(bool &timeout)
{
    --m_waitTimeout;
    if (m_waitTimeout <= 0) {
        timeout = true;
    }
    bool isRunning = IsPluginRunning();
    if (timeout || isRunning) {
        m_waitTimeout = 0;
        std::lock_guard<std::mutex> lock(m_statusMutex);
        m_waitCond.notify_all();
        return isRunning;
    }
    return false;
}

std::shared_ptr<ExternalPlugin> ExternalPlugin::Clone()
{
    auto copy = std::make_shared<ExternalPlugin>(*this);
    copy->m_reloading = true;
    return copy;
}