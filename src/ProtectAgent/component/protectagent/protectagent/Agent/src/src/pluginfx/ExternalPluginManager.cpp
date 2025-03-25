/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginParse.h
 * @brief  The implemention about ExternalPluginParse.h
 * @version 1.1.0
 * @date 2021-10-13
 * @author machenglin mwx1011302
 */
#include "pluginfx/ExternalPluginManager.h"
#include <fstream>
#ifdef WIN32
#include <io.h>
#include <tlhelp32.h>
#else
#include <dirent.h>
#endif
#include <thrift/transport/TTransportException.h>
#include "pluginfx/AutoReleasePlugin.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/CMpTime.h"
#include "common/Path.h"
#include "common/Uuid.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Path.h"
#include "common/MpString.h"
#include "common/CSystemExec.h"
#include "securecom/CryptAlg.h"
#include "securecom/RootCaller.h"
#include "securecom/SecureUtils.h"
#include "host/host.h"
#include "common/ConfigXmlParse.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "servicecenter/certificateservice/include/ICertificateService.h"
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "common/Utils.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "alarm/AlarmCode.h"
#include "alarm/AlarmMgr.h"
#include "message/tcp/CSocket.h"
#include "pluginfx/ExternalPlugin.h"

using namespace thriftservice::detail;
using namespace thriftservice;
using namespace AppProtect;
using namespace servicecenter;
using namespace certificateservice;
using namespace messageservice;

namespace {
const mp_string START_PLUGIN_FAILED_CODE = "1577209912"; // 代理主机缺失java环境
const mp_string HADOOP_PLUGIN_NAME = "HadoopPlugin";
const mp_string V2_VERSION = "v2";
const mp_string THRIFT_SERVER_PORT = "thriftserverport";
const std::string DEFAULT_LISTEN_IP = "127.0.0.1";
const mp_string DEFAULT_CRT_FILE_PATH = "thrift/client/client.crt.pem";
const mp_string SERVER_CERT_FILE = "server.pem";
const mp_string IPV4_STRING = "127.0.0.1";
const mp_string IPV6_STRING = "::1";
#ifdef WIN32
const mp_string ETC_HOSTS_PATH = GetSystemDiskChangedPathInWin("C:/Windows/System32/drivers/etc/hosts");
#else
const mp_string ETC_HOSTS_PATH = "/etc/hosts";
#endif

const int32_t DEFAULT_PLUGIN_RPC_PORT = 59570;
const mp_int32 CHECK_EXTERNAL_PLUGIN_STATUS_INTERVAL = 1000;
constexpr int32_t THRIFT_SSL_CONNECT = 1;
constexpr int32_t THRIFT_PORT_RETRY_TIME = 3;
constexpr int32_t APP_INTERFACE_RETRY_TIME = 3;
const mp_string SERVER_PEM_NAME = "server.pem";
const mp_uint32 MB_TO_BYTE = 1024 * 1024;
using PluginAction = std::function<mp_int32(const mp_string &, CRequestMsg &, CResponseMsg &)>;


mp_int32 InvokingRpcInterface(std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> appServiceClient,
    ListResourceRequest &request, CResponseMsg &responseMsg)
{
    LOGGUARD("");
    ResourceResultByPage _return;
    mp_int32 ret = MP_SUCCESS;
    mp_string errMsg;
    std::vector<mp_string> errorParam;
    try {
        appServiceClient->ListApplicationResourceV2(_return, request);
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        errMsg = ex.what();
        ret = MP_FAILED;
    } catch (AppProtectPluginException &ex) {
        COMMLOG(OS_LOG_ERROR, "AppProtectPluginException. %s", ex.message.c_str());
        errMsg = ex.message;
        ret = ex.code;
        errorParam = ex.codeParams;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "C++ Exception");
        ret = MP_FAILED;
    }

    Json::Value &jValue = responseMsg.GetJsonValueRef();
    if (MP_SUCCESS != ret) {
        jValue["errorCode"] = ret;
        jValue["errorMessage"] = errMsg;
        for (const mp_string &str : errorParam) {
            jValue["detailParams"].append(str);
        }
    } else {
        Json::Value jResList;
        StructToJson(_return, jResList);
        jValue["errorCode"] = "0";
        jValue["errorMessage"] = errMsg;
        jValue["resourceList"] = std::move(jResList);
    }

    return MP_SUCCESS;
}

mp_int32 AsyncListApplicationResourceInner(ListResourceRequest &request, mp_string jobId,
    CResponseMsg &responseMsg, std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> appServiceClient)
{
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();
    ActionResult _return;
    try {
        INFOLOG("Enter AsyncListApplicationResourceInner. id=%s.", request.id.c_str());
        appServiceClient->AsyncListApplicationResource(_return, request);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (AppProtectPluginException &ex) {
        ERRLOG("AppProtectPluginException. code=%d, message=%s.", ex.code, ex.message.c_str());
        _return.code = MP_FAILED;
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
        _return.code = MP_FAILED;
    }
    if (_return.code == MP_SUCCESS) {
        jValueRsp["code"] = _return.code;
        jValueRsp["message"] = _return.message;
        INFOLOG("AsyncListApplicationResource Inner success.");
    } else {
        jValueRsp["code"] = _return.code;
        jValueRsp["bodyErr"] = _return.bodyErr;
        jValueRsp["message"] = _return.message;
        for (mp_string str : _return.bodyErrParams) {
            jValueRsp["bodyErrParams"].append(str);
        }
        WARNLOG("AsyncListApplicationResource Inner failed, jobId=%s, code=%d, bodyErr=%d, message=%s.",
            jobId.c_str(), _return.code, _return.bodyErr, _return.message.c_str());
    }

    return _return.code;
}

} // namespace

ExternalPluginManager::ExternalPluginManager()
{
    m_pluginParseMng = std::make_shared<ExternalPluginParse>();
    (mp_void)memset_s(&m_monRunStatusTid, sizeof(m_monRunStatusTid), 0, sizeof(m_monRunStatusTid));
    m_monRunStatusFlag = MP_TRUE;
}

ExternalPluginManager::~ExternalPluginManager()
{
    m_monRunStatusFlag = MP_FALSE;
    if (m_monRunStatusTid.os_id != 0) {
        CMpThread::WaitForEnd(&m_monRunStatusTid, nullptr);
    }
    StopMonitor();
    if (m_monitorTh) {
        m_monitorTh->join();
        m_monitorTh.reset();
    }
    auto thriftService = ServiceFactory::GetInstance()->GetService<IThriftService>("IThriftService");
    thriftService->UnRegisterServer(DEFAULT_LISTEN_IP, m_thriftServerPort);
}

mp_int32 ExternalPluginManager::InvokingPlugins(const Json::Value &requestParam, CResponseMsg &responseMsg,
    const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient)
{
    CHECK_JSON_VALUE(requestParam, "pageNo")
    CHECK_JSON_VALUE(requestParam, "pageSize")
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "applications")

    ListResourceRequest request;
    ApplicationEnvironment appEnv;
    std::vector<Application> applications;
    JsonToStruct(requestParam["appEnv"], appEnv);
    if (!requestParam.isMember("applications") ||
        (!requestParam["applications"].empty() && !requestParam["applications"].isArray())) {
        ERRLOG("Check 'applications' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    for (Json::ArrayIndex index = 0; index < requestParam["applications"].size(); ++index) {
        Application applicationTmp;
        JsonToStruct(requestParam["applications"][index], applicationTmp);
        applications.push_back(applicationTmp);
    }
    request.__set_appEnv(appEnv);
    request.__set_applications(applications);
    std::vector<std::string> orders;
    std::string conditions;
    GET_JSON_INT32_OPTION(requestParam, "pageNo", request.condition.pageNo);
    GET_JSON_INT32_OPTION(requestParam, "pageSize", request.condition.pageSize);
    if (requestParam["conditions"].type() != Json::nullValue) {
        GET_JSON_STRING_OPTION(requestParam, "conditions", conditions);
        request.condition.__set_conditions(conditions);
    }
    for (std::vector<std::string>::const_iterator it = orders.begin(); it != orders.end(); ++it) {
        request.condition.orders.push_back(*it);
    }
    return InvokingRpcInterface(appServiceClient, request, responseMsg);
}

mp_int32 ExternalPluginManager::AsyncListApplicationResource(const Json::Value &requestParam, CResponseMsg &responseMsg,
    const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient, const mp_string id)
{
    LOGGUARD("");
    INFOLOG("Enter the ExternalPluginManager::AsyncListApplicationResource.");
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "applications")
    CHECK_JSON_VALUE(requestParam, "conditions")
    
    ListResourceRequest request;
    ApplicationEnvironment appEnv;
    std::vector<Application> applications;
    JsonToStruct(requestParam["appEnv"], appEnv);
    if (!requestParam.isMember("applications") ||
        (!requestParam["applications"].empty() && !requestParam["applications"].isArray())) {
        ERRLOG("Check 'applications' failed.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    for (Json::ArrayIndex index = 0; index < requestParam["applications"].size(); ++index) {
        Application applicationTmp;
        JsonToStruct(requestParam["applications"][index], applicationTmp);
        applications.push_back(applicationTmp);
    }
    request.__set_appEnv(appEnv);
    request.__set_applications(applications);
    std::vector<std::string> orders;
    std::string conditions;
    GET_JSON_INT32_OPTION(requestParam, "pageNo", request.condition.pageNo);
    GET_JSON_INT32_OPTION(requestParam, "pageSize", request.condition.pageSize);
    if (requestParam["conditions"].type() != Json::nullValue) {
        GET_JSON_STRING_OPTION(requestParam, "conditions", conditions);
        request.condition.__set_conditions(conditions);
    }
    for (std::vector<std::string>::const_iterator it = orders.begin(); it != orders.end(); ++it) {
        request.condition.orders.push_back(*it);
    }
    INFOLOG("the job id in AsyncListApplicationResource is %s.", id.c_str());
    request.__set_id(id);
    return AsyncListApplicationResourceInner(request, id, responseMsg, appServiceClient);
}

mp_int32 ExternalPluginManager::FinalizeClear(const Json::Value &requestParam, CResponseMsg &responseMsg,
    const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient)
{
    LOGGUARD("");
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "application")
    CHECK_JSON_VALUE(requestParam, "extendInfo")
    
    ApplicationEnvironment appEnv;
    Application productApp;
    std::map<std::string, std::string> extendInfo;
    JsonToStruct(requestParam["appEnv"], appEnv);
    JsonToStruct(requestParam["application"], productApp);
    Json::Value::Members extendInfoMember = requestParam["extendInfo"].getMemberNames();
    for (auto it = extendInfoMember.begin(); it != extendInfoMember.end(); ++it) {
        extendInfo[*it] = requestParam["extendInfo"][*it].asString();
    }
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();
    ActionResult _return;
    try {
        appServiceClient->FinalizeClear(_return, appEnv, productApp, extendInfo);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (AppProtectPluginException &ex) {
        ERRLOG("AppProtectPluginException. code=%d, message=%s.", ex.code, ex.message.c_str());
        _return.code = MP_FAILED;
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
        _return.code = MP_FAILED;
    }
    if (_return.code == MP_SUCCESS) {
        jValueRsp["errorCode"] = "0";
        jValueRsp["errorMessage"] = _return.message;
        INFOLOG("Finalize Clear success.");
    } else {
        jValueRsp["code"] = _return.code;
        jValueRsp["bodyErr"] = _return.bodyErr;
        jValueRsp["message"] = _return.message;
        for (mp_string str : _return.bodyErrParams) {
            jValueRsp["detailParams"].append(str);
        }
    }

    return _return.code;
}
 
#ifdef WIN32
mp_int32 ExternalPluginManager::CheckAndKillPlug(const mp_string& pid, const mp_string& pluginPidPath)
{
    mp_int32 iRet;
    mp_uint64 uint64_ProcessPid = 0;
    CMpString::StringToUInteger(pid, uint64_ProcessPid);
    mp_uint32 uint32_ProcessPid = static_cast<mp_uint32>(uint64_ProcessPid);
    if (uint32_ProcessPid == 0) {
        ERRLOG("ProcessPid is 0, can't kill System Idle Process");
        return MP_FAILED;
    }

    mp_semaf hProcess = ::OpenProcess(PROCESS_TERMINATE, MP_FAILED, uint32_ProcessPid);
    if (hProcess) {
        mp_string strCommand = "cdm.exe /c taskkill /f /pid " + pid + ">nul";
        iRet = CSystemExec::ExecSystemWithoutEcho(strCommand);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Kill process failed, and pid is %d, return is %d.", uint32_ProcessPid, iRet);
            return MP_FAILED;
        }
        CloseHandle(hProcess);
        INFOLOG("Kill process succssed, and pid is %d.", uint32_ProcessPid);
        return MP_SUCCESS;
    } else {
        // 打开进程句柄失败，即当前不存在该插件进程，效果等同插件被kill，返回成功
        WARNLOG("Open Process(%d) is failed, and ERR code is %d, ", uint32_ProcessPid, GetLastError());
        return MP_SUCCESS;
    }
}
#else
mp_int32 ExternalPluginManager::CheckAndKillPlug(const mp_string& pid, const mp_string& pluginPidPath)
{
    LOGGUARD("");
    std::vector<mp_string> lsRstList;
    mp_string processPath = "/proc/" + pid + PATH_SEPARATOR + "exe";
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_LS, processPath, &lsRstList);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Exec ls -l %s failed.", processPath.c_str());
        return MP_FAILED;
    }
    if (lsRstList.size() != 1) {
        ERRLOG("Unexpect result list size %d.", lsRstList.size());
        return MP_FAILED;
    }
    lsRstList = Awk(lsRstList, AWK_COL_LAST_1);
    mp_string pluginInstallPath = CPath::GetInstance().GetPluginInstallPath();
    if (CMpString::FormattingPath(pluginInstallPath) != MP_TRUE) {
        ERRLOG("Convert path(%s) to realpath failed.", pluginInstallPath.c_str());
        return MP_FAILED;
    }
    iRet = strncmp(lsRstList[0].c_str(), pluginInstallPath.c_str(), pluginInstallPath.length());
    if (iRet != 0) {
        ERRLOG("Soft link (%s) does not equal to path(%s).", lsRstList[0].c_str(), pluginInstallPath.c_str());
        CMpFile::DelFile(pluginPidPath);
        return MP_FAILED;
    }
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_KILL, pid, NULL);
    INFOLOG("Kill process(%s) ret(%d).", lsRstList[0].c_str(), iRet);
    return iRet;
}
#endif

mp_void ExternalPluginManager::KillExternalPlugs()
{
    LOGGUARD("");
    std::ifstream stream;
    mp_string pid;
    mp_int32 iRet;
    mp_string pluginPidDir = CPath::GetInstance().GetPluginPidPath();
    if (!CMpFile::DirExist(pluginPidDir.c_str())) {
        INFOLOG("Not exist plugin pid path(%s).", pluginPidDir.c_str());
        return;
    }
    std::vector<mp_string> PluginFileNames;
    CMpFile::GetFolderFile(pluginPidDir, PluginFileNames);
    for (const auto &item : PluginFileNames) {
        mp_string pluginPidPath = pluginPidDir + PATH_SEPARATOR + item;
        stream.open(pluginPidPath.c_str(), std::ios::in);
        if (!stream.is_open()) {
            ERRLOG("Failed to open file(%s).", pluginPidPath.c_str());
            continue;
        }
        getline(stream, pid);
#ifdef WIN32
        CheckAndKillPlug(pid, pluginPidPath);
#else
        mp_string processPath = "/proc/" + pid;
        if (!CMpFile::DirExist(processPath.c_str())) {
            WARNLOG("Pid %s does not exist.", pid.c_str());
            CMpFile::DelFile(pluginPidPath);
        } else if (CheckAndKillPlug(pid, pluginPidPath) == MP_SUCCESS) {
            CMpFile::DelFile(pluginPidPath);
        }
#endif
        stream.close();
        CMpFile::DelFile(pluginPidPath);
    }
}

mp_int32 ExternalPluginManager::InitThriftServer()
{
    auto tmpThriftService = ServiceFactory::GetInstance()->GetService<ThriftService>("IThriftService");
    if (!tmpThriftService) {
        ERRLOG("Init thrift server failed.");
        return MP_FAILED;
    }

    if (!InitThriftServerPort()) {
        ERRLOG("All ports are occupied, InitThriftServer failed.");
        return MP_FAILED;
    }
    mp_string certFile = CPath::GetInstance().GetNginxConfFilePath(SERVER_PEM_NAME);
    std::string cnNameOfServerCrt;
    SecureCom::GetHostFromCert(certFile, cnNameOfServerCrt);
    if (m_ssl) {
        auto certifiService = ServiceFactory::GetInstance()->GetService<ICertificateService>("ICertificateService");
        auto certHandler = certifiService->GetCertificateHandler();
        m_thriftServer = tmpThriftService->RegisterSslServer(cnNameOfServerCrt, m_thriftServerPort, certHandler);
    } else {
        m_thriftServer = tmpThriftService->RegisterServer(cnNameOfServerCrt, m_thriftServerPort);
    }

    INFOLOG("Init thrift server success.");
    return MP_SUCCESS;
}

EXTER_ATTACK bool ExternalPluginManager::InitThriftServerPort()
{
    mp_int32 nBeginPort = DEFAULT_PLUGIN_RPC_PORT;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, CFG_FRAME_THRIFT_SERVER_START_PORT,
        nBeginPort);
    mp_int32 nEndPort = DEFAULT_PLUGIN_RPC_PORT;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, CFG_FRAME_THRIFT_SERVER_END_PORT, nEndPort);

    mp_int32 retryTime = 0;
    while (retryTime++ < THRIFT_PORT_RETRY_TIME) {
        mp_uint64 randNum = DEFAULT_PLUGIN_RPC_PORT;
        GetRandom(randNum, false);
        mp_int32 uiPort = DEFAULT_PLUGIN_RPC_PORT;
        if (nEndPort > nBeginPort) {
            uiPort = (randNum % (nEndPort - nBeginPort)) + nBeginPort;
        }

        mp_socket sock = MP_INVALID_SOCKET;
        CSocket::CreateTcpSocket(sock);
        if (CSocket::Bind(sock, DEFAULT_LISTEN_IP, uiPort) != MP_SUCCESS) {
            CSocket::Close(sock);
            WARNLOG("Port %d is occupied, try next.", uiPort);
        } else {
            CSocket::Close(sock);
            INFOLOG("Thrift server port is %d.", uiPort);
            m_thriftServerPort = uiPort;
            break;
        }
    }
    if (WriteThriftServerPort(m_thriftServerPort) != MP_SUCCESS) {
        ERRLOG("Write thrift server port file failed!");
        return MP_FALSE;
    }
    return m_thriftServerPort != 0;
}

void ExternalPluginManager::InitMonitorThread()
{
    INFOLOG("Initialize Monitor Thread.");
    m_monitorTh = std::make_shared<std::thread>([this]() { return MonitorPluginThread(); });
}

mp_int32 ExternalPluginManager::Init()
{
    INFOLOG("Start to initialize external plugin manager.");
    KillExternalPlugs();
#ifndef WIN32
    INFOLOG("Register sig handler ret(%d).", SignalRegister(SIGUSR1, StopAllPlugsHandle));
#endif

    mp_int32 iRet = CMpThread::Create(&m_monRunStatusTid, MonPluginStatusThread, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create monitor thread failed! iRet = %d.", iRet);
        CMpThread::WaitForEnd(&m_monRunStatusTid, nullptr);
        return iRet;
    }

    m_alarmManage.Init();

    iRet = InitThriftServer();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create Thrift server failed! iRet = %d.", iRet);
        return iRet;
    }

    auto msgService = ServiceFactory::GetInstance()->GetService<IMessageService>("IMessageService");
    m_subject = msgService->GetSubject();

    InitRpcObserver();
    std::shared_ptr<IEvent> publishEvent = std::make_shared<RpcPublishEvent>(m_thriftServer);
    if (m_subject && !m_subject->Notify(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE, publishEvent)) {
        ERRLOG("Notify RPC_PUBLISH_TYPE event failed.");
        return MP_FAILED;
    }

    if (m_thriftServer && !m_thriftServer->Start()) {
        ERRLOG("Thrift server start failed.Ip(%s), Port(%d).", DEFAULT_LISTEN_IP.c_str(), m_thriftServerPort);
        AlarmMgr::GetInstance().SendAlarm(ALARM_THRIFT_SERVER_START_FAILED, std::to_string(m_thriftServerPort));
        return MP_FAILED;
    } else {
        AlarmMgr::GetInstance().ResumeAlarm(ALARM_THRIFT_SERVER_START_FAILED);
    }
    InitMonitorThread();
    INFOLOG("Initialize external plugin manager success.");
    return MP_SUCCESS;
}

void ExternalPluginManager::MonitorPluginThread()
{
    INFOLOG("MonitorPluginThread start to run.");
    while (!m_stop) {
        if (WaitMonitor()) {
            MonitorPlugin();
            SleepForMS(CHECK_EXTERNAL_PLUGIN_STATUS_INTERVAL);
        } else {
            INFOLOG("Monitor Thread stopping");
        }
    }
    INFOLOG("MonitorPluginThread stop to run.");
}

bool ExternalPluginManager::WaitMonitor()
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_monitorCond.wait(lock, [this] { return m_startMonitor || m_stop; });
    return !m_stop;
}

void ExternalPluginManager::PauseMonitor()
{
    INFOLOG("ExternalPluginManager not plugin runing, pause monitor thread");
    std::unique_lock<std::mutex> lock(m_lock);
    m_startMonitor = false;
    m_monitorCond.notify_one();
}

void ExternalPluginManager::StopMonitor()
{
    INFOLOG("ExternalPluginManager stop notify monitor thread");
    std::unique_lock<std::mutex> lock(m_lock);
    m_stop = true;
    m_monitorCond.notify_one();
}

void ExternalPluginManager::StartMonitor()
{
    INFOLOG("ExternalPluginManager notify monitor thread");
    std::unique_lock<std::mutex> lock(m_lock);
    m_startMonitor = true;
    m_monitorCond.notify_one();
}

void ExternalPluginManager::MonitorPlugin()
{
    std::vector<mp_string> erasePlugins;
    std::vector<std::shared_ptr<ExternalPlugin>> restartPlugins;
    std::vector<std::pair<mp_string, mp_string>> reloadedPlugins;
    MonitorPluginCheck(erasePlugins, restartPlugins, reloadedPlugins);  // 检查需要移除的插件
    {
        std::lock_guard<std::mutex> lock(m_pluginMutex);
        for (const auto &plugin : erasePlugins) {
            auto it = m_pluginMap.find(plugin);
            if (it != m_pluginMap.end()) {
                m_pluginMap.erase(it);
                INFOLOG("Plugin %s erased.", plugin.c_str());
            }
        }
        // 插件异常crash，需要重新拉起的插件
        for (const auto &plugin : restartPlugins) {
            ReloadPlugin(plugin);
            INFOLOG("Plugin %s reload.", plugin->GetName().c_str());
        }
    }
    // 通知需要重启的插件
    for (const auto &plugin : reloadedPlugins) {
        INFOLOG("Plugin:[%s] reload, notify task to redo", plugin.first.c_str());
        AppProtectJobHandler::GetInstance()->NotifyPluginReload(plugin.first, plugin.second);
    }
}

void ExternalPluginManager::MonitorPluginCheck(std::vector<mp_string> &erasePlugins,
    std::vector<std::shared_ptr<ExternalPlugin>> &restartPlugins,
    std::vector<std::pair<mp_string, mp_string>> &reloadedPlugins)
{
    // 监控每个插件
    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto iter = m_pluginMap.begin();
    for (; iter != m_pluginMap.end(); ++iter) {
        // 升级情况 插件被卸载 目录不存在
        if (!IsPluginFolderExist(iter->first)) {
            erasePlugins.push_back(iter->first);
            DBGLOG("Plugin:[%s], the plugin folder not exist", iter->first.c_str());
            continue;
        }
        // 任务等待注册
        if (iter->second->IsWaitPluginRegister()) {
            auto state = HandlePlguinWait(iter->first, iter->second);
            // Success 插件注册成功，且插件重启，通知任务处理 TimeOut 插件注册失败，删除任务
            if (state == PluginRegisterdState::Success && iter->second->IsReload()) {
                INFOLOG("Plugin:[%s] restart succees", iter->first.c_str());
                reloadedPlugins.push_back({iter->first, iter->second->GetPluginInfo().processId});
            } else if (state == PluginRegisterdState::TimeOut) {
                WARNLOG("Plugin:[%s] register timeout", iter->first.c_str());
                erasePlugins.push_back(iter->first);
            }
            // 插件运行中，检查插件是否异常退出或无响应
        } else if (iter->second->IsPluginRunning()) {
            if (!iter->second->IsPluginProcessExist()) {
                WARNLOG("Plugin:[%s] down, restart plugin", iter->first.c_str());
                erasePlugins.push_back(iter->first);
                restartPlugins.push_back(iter->second->Clone());
            } else if (!iter->second->IsPluginResponding()) {
                WARNLOG("Plugin:[%s] up, but no response, restart plugin", iter->first.c_str());
                iter->second->ExecStopPlugin();
                erasePlugins.push_back(iter->first);
                restartPlugins.push_back(iter->second->Clone());
            } else if (iter->second->IsNoUseTimeout()) {
                INFOLOG("Plugin:[%s] have timeout when no use, stop plugin", iter->first.c_str());
                iter->second->ExecStopPlugin();
                erasePlugins.push_back(iter->first);
            } else {
                DBGLOG("Plugin:[%s] Is Ok", iter->first.c_str());
            }
        }
    }
}

ExternalPluginManager::PluginRegisterdState ExternalPluginManager::HandlePlguinWait(const mp_string &pluginName,
    std::shared_ptr<ExternalPlugin> plugin)
{
    PluginRegisterdState state = PluginRegisterdState::Checking;
    bool timeout = false;
    auto pluginRuning = plugin->CheckPluginRegister(timeout);
    if (pluginRuning) {
        m_alarmManage.ClearAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, pluginName);
        INFOLOG("Plugin:[%s] is running, clear alarm finish.", pluginName.c_str());
        state = PluginRegisterdState::Success;
    } else if (timeout) {
        m_alarmManage.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, pluginName);
        INFOLOG("Plugin:[%s] register timeout, send alarm finish.", pluginName.c_str());
        state = PluginRegisterdState::TimeOut;
    }
    return state;
}

void ExternalPluginManager::InitRpcObserver()
{
    ServiceFactory::GetInstance()->GetService<IService>("ShareResourceService");
    ServiceFactory::GetInstance()->GetService<IService>("PluginRegisterService");
    ServiceFactory::GetInstance()->GetService<IService>("SecurityService");
    ServiceFactory::GetInstance()->GetService<IService>("IJobServer");
}

void ExternalPluginManager::RegisterObserver(messageservice::EVENT_TYPE type, std::shared_ptr<IObserver> observer)
{
    m_subject->Register(type, observer);
}

#ifndef WIN32
mp_void ExternalPluginManager::StopAllPlugsHandle(mp_int32 signum)
{
    INFOLOG("Receive stop plug msg.");
    if (signum != SIGUSR1) {
        WARNLOG("Wrong signal type(%d).", signum);
        return;
    }
    ExternalPluginManager::GetInstance().StopAllPlugs();
}
#endif

mp_void ExternalPluginManager::StopAllPlugs()
{
    mp_int32 iRet;
    INFOLOG("Total %d external plugs will be stopped.", m_pluginMap.size());
    for (auto iter = m_pluginMap.begin(); iter != m_pluginMap.end(); ++iter) {
        iRet = iter->second->ExecStopPlugin();
        INFOLOG("Stop plug %s ret %d.", iter->second->GetName().c_str(), iRet);
        if (iRet == MP_SUCCESS) {
            continue;
        }
        iter->second->KillProcess();
    }
}

mp_void ExternalPluginManager::StopPluginEx(const mp_string &appType)
{
    mp_string pluginName;
    mp_uint32 iRet = m_pluginParseMng->GetPluginNameByAppType(appType, pluginName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Apptype %s doesn't belong to any plugin.", appType.c_str());
        return;
    }
    mp_string stopScriptPath = CPath::GetInstance().GetRootPath() + PATH_SEPARATOR + ".." + PATH_SEPARATOR +
        EXTERNAL_PLUGIN_PATH + PATH_SEPARATOR + pluginName + PATH_SEPARATOR + EXTERNAL_PLUGIN_STOP_SCRIPT;
#ifdef WIN32
    iRet = CSystemExec::ExecSystemWithoutEcho(stopScriptPath);
#else
    CRootCaller rootCaller;
    iRet = rootCaller.ExecUserDefineScript((mp_int32)ROOT_COMMAND_USER_DEFINED, stopScriptPath);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Fail to exec stop script, %s.", stopScriptPath.c_str());
    }
}

std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> ExternalPluginManager::GetServiceClient(
    const mp_string &strAppType)
{
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return nullptr;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return nullptr;
    }
    return GetApplicationServiceClient(pClient);
}

mp_int32 ExternalPluginManager::QueryRemoteCluster(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetServiceClient(strAppType);
    if (appServiceClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Get thrift sevice client failed,apptype = %s", strAppType.c_str());
        return MP_FAILED;
    }

    ApplicationEnvironment appEnv;
    Application productApp;
    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
    if (!requestParam.isObject() || !requestParam.isMember("appEnv") || !requestParam.isMember("application")) {
        ERRLOG("Invalide requestParam.");
        return MP_FAILED;
    }
    JsonToStruct(requestParam["appEnv"], appEnv);
    JsonToStruct(requestParam["application"], productApp);

    ApplicationEnvironment _return;
    try {
        appServiceClient->DiscoverAppCluster(_return, appEnv, productApp);
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        return MP_FAILED;
    } catch (const std::exception &ex) {
        COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        return MP_FAILED;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        return MP_FAILED;
    }
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();
    StructToJson(_return, jValueRsp);
    COMMLOG(OS_LOG_INFO, "Query remote cluster sucess.");
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::QueryPluginResource(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);

    CHost host;
    mp_string hostSN;
    mp_int32 iRet = host.GetHostSN(hostSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetHostSN failed, iRet %d.", iRet);
        return iRet;
    }

    std::vector<Application> _return;
    try {
        appServiceClient->DiscoverApplications(_return, strAppType);
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        return MP_FAILED;
    } catch (const std::exception &ex) {
        COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        return MP_FAILED;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        return MP_FAILED;
    }

    Json::Value &jValue = responseMsg.GetJsonValueRef();
    jValue["uuid"] = hostSN;
    jValue["appNum"] = _return.size();
    Json::Value jAppList;
    for (auto iter = _return.begin(); iter != _return.end(); ++iter) {
        Json::Value jApp;
        StructToJson(*iter, jApp);
        jAppList.append(std::move(jApp));
    }
    jValue["appLists"] = std::move(jAppList);

    COMMLOG(OS_LOG_INFO, "Query Plugin Resource success.");
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::QueryPluginDetail(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    INFOLOG("Start QueryPluginDetail.");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);
    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
    return QueryPluginDetailV1(requestParam, responseMsg, appServiceClient);
}

mp_int32 ExternalPluginManager::QueryPluginDetailV2(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    INFOLOG("Start QueryPluginDetailV2.");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);

    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();

    return InvokingPlugins(requestParam, responseMsg, appServiceClient);
}

mp_int32 ExternalPluginManager::PluginAsyncListApplicationResource(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    INFOLOG("Start PluginAsyncListApplicationResource.");
    mp_string strTaskID = requestMsg.GetURL().GetSpecialQueryParam("id");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);
 
    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
 
    return AsyncListApplicationResource(requestParam, responseMsg, appServiceClient, strTaskID);
}

mp_int32 ExternalPluginManager::PluginFinalizeClear(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    INFOLOG("Start PluginFinalizeClear.");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);
 
    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
 
    return FinalizeClear(requestParam, responseMsg, appServiceClient);
}
 
mp_void ExternalPluginManager::QueryPluginState(const mp_string &strAppType, Json::Value &jValueRsp)
{
    LOGGUARD("");
    mp_string pluginName;
    mp_uint32 iRet = m_pluginParseMng->GetPluginNameByAppType(strAppType, pluginName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Apptype %s doesn't belong to any plugin.", strAppType.c_str());
        return;
    }

    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto it = m_pluginMap.find(pluginName);
    if (it == m_pluginMap.end()) {
        if (strcmp(pluginName.c_str(), HADOOP_PLUGIN_NAME.c_str()) == 0) {
            // 启动插件失败上报
            jValueRsp["code"] = "200";
            jValueRsp["message"] = START_PLUGIN_FAILED_CODE;
        }
        ERRLOG("Failed to start the plugin[%s].", pluginName.c_str());
    }
}

mp_int32 ExternalPluginManager::CheckPlugin(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();

    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        QueryPluginState(strAppType, jValueRsp);
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);

    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "application")
    ApplicationEnvironment appEnv;
    Application productApp;
    JsonToStruct(requestParam["appEnv"], appEnv);
    JsonToStruct(requestParam["application"], productApp);

    ActionResult _return;
    try {
        appServiceClient->CheckApplication(_return, appEnv, productApp);
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (const std::exception &ex) {
        COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        _return.code = MP_FAILED;
    }
    if (_return.code == MP_SUCCESS) {
        jValueRsp["errorCode"] = "0";
        jValueRsp["errorMessage"] = _return.message;
    } else {
        jValueRsp["code"] = _return.code;
        jValueRsp["bodyErr"] = _return.bodyErr;
        jValueRsp["message"] = _return.message;
        for (mp_string str : _return.bodyErrParams) {
            jValueRsp["detailParams"].append(str);
        }
    }

    return _return.code;
}

mp_int32 ExternalPluginManager::QueryPluginConfig(const mp_string &strAppType, CRequestMsg &requestMsg,
    CResponseMsg &responseMsg)
{
    LOGGUARD("");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);

    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);
    // parse script
    mp_string script = requestMsg.GetURL().GetSpecialQueryParam("script");

    std::map<std::string, std::string> _return;
    try {
        appServiceClient->ListApplicationConfig(_return, script);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
        return MP_FAILED;
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
        return MP_FAILED;
    } catch (...) {
        ERRLOG("Unknown exception.");
        return MP_FAILED;
    }
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();
    for (const auto &it : _return) {
        JsonHelper::JsonStringToJsonValue(it.second, jValueRsp[it.first]);
    }
    return MP_SUCCESS;
}

std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> ExternalPluginManager::GetApplicationServiceClient(
    const std::shared_ptr<thriftservice::IThriftClient>& pThriftClient)
{
    return pThriftClient->GetConcurrentClientIf<ApplicationServiceConcurrentClient>("ApplicationService");
}

bool ExternalPluginManager::CheckIfCNExist(const mp_string &hostName)
{
    std::vector<mp_string> contentVec;
    if (CMpFile::ReadFile(ETC_HOSTS_PATH, contentVec) != MP_SUCCESS) {
        return false;
    }
    for (auto contentStr : contentVec) {
        DBGLOG("The string is %s.", contentStr.c_str());
        if (contentStr.find(hostName) != mp_string::npos) {
            return true;
        }
    }
    return false;
}

mp_int32 ExternalPluginManager::AddHostNameToFile(const mp_string& hostName)
{
    mp_uint32 fileSize = 0;
    CMpFile::FileSize(ETC_HOSTS_PATH.c_str(), fileSize);
    if (fileSize > MB_TO_BYTE) {
        ERRLOG("The file %s is too big, exit!");
        return MP_FAILED;
    }
    if (CheckIfCNExist(hostName)) {
        INFOLOG("The host name is in file, no need to add, hostname is %s.", hostName.c_str());
        return MP_SUCCESS;
    }

    std::vector<mp_string> content;
    content.push_back(IPV4_STRING + " " + hostName);
    content.push_back(IPV6_STRING + " " + hostName);
    if (CIPCFile::AppendFile(ETC_HOSTS_PATH, content) != MP_SUCCESS) {
        ERRLOG("Add failed!");
        return MP_FAILED;
    }
    INFOLOG("Add successfully!");
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::CheckIfCNExistInHostsFile()
{
    mp_string certPath;
#ifndef WIN32
    certPath = CPath::GetInstance().GetConfFilePath(DEFAULT_CRT_FILE_PATH);
#else
    certPath = CPath::GetInstance().GetNginxConfFilePath(SERVER_CERT_FILE);
#endif
    if (!CMpFile::FileExist(certPath)) {
        WARNLOG("Cert path is not exists.");
        return MP_FAILED;
    }
    mp_string hostName;
    SecureCom::GetHostFromCert(certPath, hostName);
    mp_int32 iRet = MP_SUCCESS;
#ifndef WIN32
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_ADD_HOSTS, hostName, NULL);
#else
    iRet = AddHostNameToFile(hostName);
#endif
    return iRet;
}

mp_int32 ExternalPluginManager::StartPlugin(const mp_string &pluginName)
{
    INFOLOG("Start to start plugin with pluginName %s.", pluginName.c_str());
    mp_string startUser = m_pluginParseMng->GetStartUser(pluginName);
    if (startUser.empty()) {
        ERRLOG("Fail to get start user for %s.", pluginName.c_str());
        return MP_FAILED;
    }
    auto plugin = std::make_shared<ExternalPlugin>(pluginName, startUser, m_ssl, m_thriftServerPort);
    auto iRet = plugin->StartPlugin();
    // 设置 cpulimt 参数
    plugin->SetCgroupInfo(m_pluginParseMng->GetCpuLimitByPluginName(pluginName),
        m_pluginParseMng->GetMemoryLimitByPluginName(pluginName),
        m_pluginParseMng->GetBlkioWeightByPluginName(pluginName));
    if (iRet != MP_SUCCESS) {
        m_alarmManage.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, pluginName);
        ERRLOG("Start plugin %s fail, Send START_ERROR_ALARM alarm.", pluginName.c_str());
        return iRet;
    }
    m_pluginMap[pluginName] = std::move(plugin);
    INFOLOG("Start pluginName %s. success.", pluginName.c_str());
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::ReloadPlugin(std::shared_ptr<ExternalPlugin> plugin)
{
    auto pluginName = plugin->GetName();
    INFOLOG("RestartPlugin pluginName %s.", pluginName.c_str());
    auto iRet = plugin->StartPlugin();
    if (iRet != MP_SUCCESS) {
        ERRLOG("RestartPlugin pluginName %s Failed.", pluginName.c_str());
        m_alarmManage.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, pluginName);
        return iRet;
    }
    m_pluginMap[pluginName] = plugin;
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::GetPluginName(const mp_string &appType, mp_string &plugin)
{
    mp_uint32 iRet = m_pluginParseMng->GetPluginNameByAppType(appType, plugin);
    if (iRet != MP_SUCCESS || plugin.empty()) {
        ERRLOG("Apptype %s doesn't belong to any plugin.", appType.c_str());
        m_alarmManage.BroadcastAlarm(ExternalPluginAlarmMng::ExPluginAlarmType::START_ERROR_ALARM, plugin);
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::UpdatePluginStatus(const mp_string &pluginName, EX_PLUGIN_STATUS status)
{
    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto iter = m_pluginMap.find(pluginName);
    if (iter == m_pluginMap.end()) {
        ERRLOG("Update status failed for no plugin %s.", pluginName.c_str());
        return MP_FAILED;
    }
    /* closed status, then unregister plugin
      two case:1. Agent stop plugin,and plugin unregister 2.plugin crash,state trun to close
    */
    switch (status) {
        case EX_PLUGIN_STATUS::CLOSED: {
            iter->second->PluginUnregistered();
            break;
        }
        case EX_PLUGIN_STATUS::ISREGISTERED: {
            iter->second->PluginRegistered();
            break;
        }
        default:
            break;
    }
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::UpdatePluginInfo(const mp_string &pluginName, const ApplicationPlugin &pluginInfo)
{
    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto iter = m_pluginMap.find(pluginName);
    if (iter == m_pluginMap.end()) {
        ERRLOG("Update process id failed for no plugin %s.", pluginName.c_str());
        return MP_FAILED;
    }

    iter->second->SetPluginInfo(pluginInfo);
    return MP_SUCCESS;
}

std::shared_ptr<ExternalPlugin> ExternalPluginManager::GetPluginFromPluginMap(const mp_string &pluginName)
{
    std::shared_ptr<ExternalPlugin> ret;
    auto iter = m_pluginMap.find(pluginName);
    if (iter != m_pluginMap.end()) {
        ret = iter->second;
    }
    return ret;
}

bool ExternalPluginManager::PluginExisted(const mp_string &pluginName)
{
    return GetPluginFromPluginMap(pluginName) != nullptr;
}

mp_void ExternalPluginManager::RemovePlugin(const std::string &pluginName)
{
    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto iter = m_pluginMap.find(pluginName);
    if (iter != m_pluginMap.end()) {
        iter->second->StopPlugin();
        m_pluginMap.erase(iter);
    }
}

bool ExternalPluginManager::IsPluginFolderExist(const std::string &pluginName)
{
    mp_string pluginFolderPath = CPath::GetInstance().GetPluginInstallPath() + "/" + pluginName;
    return CMpFile::DirExist(pluginFolderPath.c_str());
}

mp_void ExternalPluginManager::MonPluginStatus()
{
    std::lock_guard<std::mutex> lock(m_pluginMutex);
    auto iter = m_pluginMap.begin();
    for (; iter != m_pluginMap.end(); ++iter) {
        if ((iter->second->GetRunningStatus() == EX_PLUGIN_STATUS::CLOSING) ||
            (iter->second->GetRunningStatus() == EX_PLUGIN_STATUS::CLOSED)) {
            if (iter->second->KillProcess() == MP_SUCCESS) {
                WARNLOG("Plugin %s is killed.", iter->second->GetName().c_str());
                iter = m_pluginMap.erase(iter);
            }
        }
    }
}

bool ExternalPluginManager::GetMonRunStatusFlag()
{
    return m_monRunStatusFlag;
}

#ifdef WIN32
DWORD WINAPI ExternalPluginManager::MonPluginStatusThread(mp_void *pThis)
#else
mp_void *ExternalPluginManager::MonPluginStatusThread(mp_void *pThis)
#endif
{
    LOGGUARD("");
    const int monInterval = 60 * 60 * 1000;
    ExternalPluginManager *pluginCtrl = static_cast<ExternalPluginManager *>(pThis);
    while (pluginCtrl->GetMonRunStatusFlag()) {
        CMpTime::DoSleep(monInterval);
        pluginCtrl->MonPluginStatus();
    }
#ifdef WIN32
    return 0;
#else
    return nullptr;
#endif
}

std::shared_ptr<ExternalPlugin> ExternalPluginManager::GetPluginImpl(const mp_string &appType,
    const ModifyPluginCounter& modiftCounter)
{
    std::string pluginName;
    if (GetPluginName(appType, pluginName) != MP_SUCCESS) {
        WARNLOG("Get Plugin of app type %s fail.", appType.c_str());
        return nullptr;
    }

    std::shared_ptr<ExternalPlugin> plugin;
    {
        std::lock_guard<std::mutex> lock(m_pluginMutex);
        if (!PluginExisted(pluginName)) {
            StartPlugin(pluginName);
        }
        plugin = GetPluginFromPluginMap(pluginName);
    }
    if (plugin) {
        modiftCounter(plugin);
        CheckIfCNExistInHostsFile();
        StartMonitor();
    }
    // new plugin, start monitor thread to check if plugin register in 60s
    return plugin;
}

void ExternalPluginManager::ReleasePluginImpl(const mp_string &appType, const ModifyPluginCounter& modifyCounter)
{
    mp_string pluginName;
    mp_uint32 iRet = m_pluginParseMng->GetPluginNameByAppType(appType, pluginName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Apptype %s doesn't belong to any plugin.", appType.c_str());
        return;
    }
    {
        std::lock_guard<std::mutex> lock(m_pluginMutex);
        auto iter = m_pluginMap.find(pluginName);
        if (iter == m_pluginMap.end()) {
            ERRLOG("Plugin %s not exist.", pluginName.c_str());
            return;
        }
    modifyCounter(iter->second);
    }
    return;
}

std::shared_ptr<ExternalPlugin> ExternalPluginManager::GetPlugin(const mp_string &appType)
{
    return GetPluginImpl(appType, [](std::shared_ptr<ExternalPlugin> plugin) { plugin->AddTaskCounter(); });
}

std::shared_ptr<ExternalPlugin> ExternalPluginManager::GetPluginByRest(const mp_string &appType)
{
    return GetPluginImpl(appType, [](std::shared_ptr<ExternalPlugin> plugin) { plugin->AddRestCounter(); });
}

void ExternalPluginManager::ReleasePlugin(const mp_string &appType)
{
    return ReleasePluginImpl(appType, [](std::shared_ptr<ExternalPlugin> plugin) { plugin->ReduceTaskCounter(); });
}

void ExternalPluginManager::ReleasePluginByRest(const mp_string &appType)
{
    return ReleasePluginImpl(appType, [](std::shared_ptr<ExternalPlugin> plugin) { plugin->ReduceRestCounter(); });
}

mp_int32 ExternalPluginManager::WriteThriftServerPort(const mp_int32 &thriftServerPort)
{
    mp_string thriftServerPortPath = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + THRIFT_SERVER_PORT;
    if (!CMpFile::FileExist(thriftServerPortPath)) {
        if (CMpFile::CreateFile(thriftServerPortPath) != MP_SUCCESS) {
            ERRLOG("Create thrift server port file failed, path[%s].", thriftServerPortPath.c_str());
            return MP_FAILED;
        }
    }
    std::vector<mp_string> vecThriftServerPort = { CMpString::to_string(thriftServerPort) };
    return CIPCFile::WriteFile(thriftServerPortPath, vecThriftServerPort);
}

mp_int32 ExternalPluginManager::QueryPluginDetailV1(const Json::Value &requestParam, CResponseMsg &responseMsg,
    const std::shared_ptr<AppProtect::ApplicationServiceConcurrentClient> &appServiceClient)
{
    LOGGUARD("");
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "application")
    CHECK_JSON_VALUE(requestParam, "parentResource")
    ApplicationEnvironment appEnv;
    Application productApp;
    ApplicationResource parentResource;
    JsonToStruct(requestParam["appEnv"], appEnv);
    JsonToStruct(requestParam["application"], productApp);
    JsonToStruct(requestParam["parentResource"], parentResource);
    Json::Value &jValue = responseMsg.GetJsonValueRef();
    std::vector<ApplicationResource> _return;
    try {
        appServiceClient->ListApplicationResource(_return, appEnv, productApp, parentResource);
    } catch (apache::thrift::transport::TTransportException &ex) {
        ERRLOG("TTransportException. %s", ex.what());
        return MP_FAILED;
    } catch (AppProtectPluginException &ex) {
        ERRLOG("AppProtectPluginException. code=%d, message=%s.", ex.code, ex.message.c_str());
        Json::Value errCodeParams;
        for (const auto &iter : ex.codeParams) {
            errCodeParams.append(std::move(iter));
        }
        jValue["code"] = ex.code;
        jValue["bodyErr"] = ex.code;
        jValue["message"] = ex.message;
        jValue["detailParams"] = std::move(errCodeParams);
        return MP_FAILED;
    } catch (const std::exception &ex) {
        ERRLOG("Standard C++ Exception. %s", ex.what());
        return MP_FAILED;
    }
    Json::Value jResList;
    for (auto iter = _return.begin(); iter != _return.end(); ++iter) {
        Json::Value jAppRes;
        StructToJson(*iter, jAppRes);
        jResList.append(std::move(jAppRes));
    }
    jValue["resourceList"] = std::move(jResList);
    INFOLOG("Query Plugin Detail success.");
    return MP_SUCCESS;
}

mp_int32 ExternalPluginManager::RemoveProtect(const mp_string &strAppType,
    CRequestMsg& requestMsg, CResponseMsg& responseMsg)
{
    LOGGUARD("");
    responseMsg.SetHttpType(CResponseMsg::RSP_JSON_TYPE2);
    Json::Value &jValueRsp = responseMsg.GetJsonValueRef();

    auto plugin = GetPluginByRest(strAppType);
    if (plugin == nullptr) {
        ERRLOG("Get plugin failed. strAppType:%s", strAppType.c_str());
        QueryPluginState(strAppType, jValueRsp);
        return MP_FAILED;
    }
    auto pClient = plugin->GetPluginClient();
    if (pClient == nullptr) {
        ERRLOG("Get thrift client failed. strAppType:%s", strAppType.c_str());
        return MP_FAILED;
    }
    AutoReleasePlugin autoRelease(strAppType);
    auto appServiceClient = GetApplicationServiceClient(pClient);

    const Json::Value &requestParam = requestMsg.GetMsgBody().GetJsonValueRef();
    CHECK_JSON_VALUE(requestParam, "appEnv")
    CHECK_JSON_VALUE(requestParam, "application")
    ApplicationEnvironment appEnv;
    Application productApp;
    JsonToStruct(requestParam["appEnv"], appEnv);
    JsonToStruct(requestParam["application"], productApp);

    ActionResult _return;
    try {
        appServiceClient->RemoveProtect(_return, appEnv, productApp);
    } catch (apache::thrift::transport::TTransportException &ex) {
        COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (const std::exception &ex) {
        COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        _return.code = MP_FAILED;
    } catch (...) {
        COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        _return.code = MP_FAILED;
    }
    if (_return.code == MP_SUCCESS) {
        jValueRsp["errorCode"] = "0";
        jValueRsp["errorMessage"] = _return.message;
    } else {
        jValueRsp["code"] = _return.code;
        jValueRsp["bodyErr"] = _return.bodyErr;
        jValueRsp["message"] = _return.message;
        for (mp_string str : _return.bodyErrParams) {
            jValueRsp["detailParams"].append(str);
        }
    }

    return _return.code;
}