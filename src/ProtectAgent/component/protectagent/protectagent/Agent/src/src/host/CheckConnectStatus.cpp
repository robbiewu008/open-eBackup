/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CheckConnectStatus.cpp
 * @brief  Contains function declarations UUID
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "host/CheckConnectStatus.h"
#include <iostream>
#include <sstream>
#include <vector>
#include <map>
#include <functional>
#include <ctime>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "common/AppVersion.h"
#include "common/Ip.h"
#include "common/Log.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "host/ConnectivityManager.h"

using namespace std;
CheckConnectStatus CheckConnectStatus::m_Instance;
namespace {
const mp_string REGISTER_HOST = "registerHost";
mp_int32 isRegisteredToPM = MP_FALSE;
mp_uint64 lastReportFinishTime = 0;

const mp_string DELETE_HOST("DeleteHost");
const mp_int32 MAX_RETRY_TIMES = 3;
const mp_uint32 HALF_MINUTE = 30 * 1000;  // 30S
const mp_uint32 DELAY_TIME = 3 * 1000;  // 3s delay
const mp_uint32 ONE_SECOND = 1 * 1000;  // 1000 ms
const mp_uint32 FIVE_MINUTES = 5 * 60 * 1000;
const mp_uint32 THREE_MINUTES = 3 * 60 * 1000;
const mp_uint32 CHECH_VSPHERE_STATE_INTERVAL = 2 * 60 * 60;     // 2h/7200s
const mp_uint32 DISABLE_CHECK_INTERVAL = 3 * 1000;
const mp_uint32 TO_MILLISECOND = 1000;
const int DEFAULT_TEST_CONNECTIVITY_TIMEOUT = 5;
const mp_uint32 HTTP_CONNECT_TIMEOUT = 120; // 2min

// host info
const mp_string PARAM_KEY_HOSTID = "host_id";
const mp_string PARAM_KEY_HOSTNAME = "name";
const mp_string PARAM_KEY_ENVTYPE = "env_type";
const mp_string PARAM_KEY_HOSTIP = "ip";
const mp_string PARAM_KEY_HOSTPORT = "port";
const mp_string PARAM_KEY_HOSTLINKSTATUS = "link_status";
const mp_string PARAM_KEY_HOSTOSTYPE = "os_type";
const mp_string PARAM_KEY_HOSTOSVER = "os_ver";
const mp_string PARAM_KEY_HOSTPROXYTYPE = "proxy_type";
const mp_string PARAM_KEY_PM_IP = "PM_IP";
const mp_string PARAM_SANCLIENT_UUID_SUFFIX = "_sanclient";
const mp_int32 BACKUP_ROLE_SANCLIENT = 5;

// agent role
const mp_string DBBACKUP_AGENT = "DBBackupAgent";
const mp_string VMBACKUP_AGENT = "VMBackupAgent";
const mp_string DWSBACKUP_AGENT = "DWSBackupAgent";
const mp_string PLUGINBACKUP_AGENT = "UBackupAgent";
const mp_string SANCLIENT_AGENT_PLUIN = "SBackupAgent";
const mp_string SANCLIENT_BACKUP_ROLE = "5";

// pm rspbody key
const mp_string PARAM_KEY_ERRORCODE = "error_code";
const mp_string PARAM_KEY_ERRORMSG = "error_msg";
// 1能ping通   0不能
const mp_int32 CAN_CONNECT = 1;
const mp_int32 CAN_NOT_CONNECT = 0;
const mp_int32 CK_MAX_TIMER_INTERVAL = 300 * 1000;

// resource usage
const mp_int32 DEFAULT_RESOURCE_USAGE = 50;
const mp_float DEFAULT_RESOURCE_USAGE_RATE = 0.5;

// interface
const mp_string CHECKCONNECTPAGESIZE = "200";
const mp_string GET_INTERFACE = "/v1/internal/environments?page_no=0&page_size=200";
const mp_string GET_CLUSTERS_NODES = "/v1/internal/clusters/local/nodes";
const mp_string PUT_INTERFACE = "/v1/internal/service-links";
const mp_string PROTOCOL = "http://";
const mp_string PROTOCOL_SECURE = "https://";
const mp_string PUT_HEART_BEAT_TO_PM = "/v1/host-agent/heartbeat";
const mp_string PUT_RESOURCE_USAGE_TO_PM = "/v1/internal/host-agent/monitor/statistics";
const mp_string GET_IP_LIST_FROM_PM = "/v1/internal/clusters/backup/member/service-ips";

const mp_string DISABLE_HEART_BEAT_FLAG_FILE = "disable_heartbeat_to_pm";

const mp_string DPC_AGENT_PATH = "fusionstorage/dpc/bin/dpc";
}

CheckConnectStatus::~CheckConnectStatus()
{
    m_bNeedExit = MP_TRUE;
    if (m_CheckConnectivityThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_CheckConnectivityThread, NULL);
    }
    if (m_heartBeatToPMThread != nullptr) {
        m_heartBeatToPMThread->join();
        m_heartBeatToPMThread.reset();
    }
}

mp_int32 CheckConnectStatus::Handle(const mp_string& actionType)
{
    if (actionType != REGISTER_HOST && actionType != DELETE_HOST) {
        cout << "Input action type is error, only support \"RegisterHost\" and \"DeleteHost\" type." << endl;
        return MP_FAILED;
    }

    mp_int32 iRet = GetPMIPandPort();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed, iRet = %d.", iRet);
        return iRet;
    }

    if (actionType == REGISTER_HOST) {
        iRet = ReportHost();
    } else {
        iRet = DeleteHost();
    }

    PrintResult(actionType, iRet);

    return iRet;
}

mp_int32 CheckConnectStatus::ReportHost()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to register host to ProtectManager.");
    // step2: register host to ProtectManager
    mp_int32 iRet = RegisterHost2PM();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::DeleteHost()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to delete host from ProtectManager.");
    mp_int32 iRet = DeleteHostFromPM();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::RegisterHost2PM()
{
    HttpRequest req;
    mp_int32 iRet = InitRegisterReq(req);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Begin to register host to PM, url is %s.", req.url.c_str());

    mp_string rspBody;
    iRet = SendRequest(req, rspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d.", iRet);
        return iRet;
    }

    // prase errCode
    Json::Value jsonRspBody;
    iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert String to Json failed.");
        return iRet;
    }

    if ((!jsonRspBody.isObject()) || (!jsonRspBody.isMember(PARAM_KEY_ERRORCODE)) ||
        (!jsonRspBody.isMember(PARAM_KEY_ERRORMSG))) {
        COMMLOG(OS_LOG_ERROR, "rspBody string have no error_code.");
        return iRet;
    }
    mp_string strErrCode;
    mp_string errMsg;
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORCODE, strErrCode);
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORMSG, errMsg);

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::InitRegisterReq(HttpRequest& req)
{
    req.method = "POST";
    mp_int32 secure_channel;
    mp_int32 Ret =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (Ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Failed to obtain the secure communication method.");
        return Ret;
    }

    if (secure_channel == 1) {
        mp_string domain_name;
        mp_int32 iRet =
            CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, domain_name);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Failed to obtain the domain name.");
            return iRet;
        }
        req.url.append("https://").append(domain_name);
        req.domaininfo.append("https://").append(domain_name);
        req.hostinfo.append(domain_name).append(":").append(m_PMPort).append(":").append(m_PMIp);
    } else {
        req.url.append("http://");
        m_PMIp = CIP::FormatFullUrl(m_PMIp);
        req.url.append(m_PMIp);
    }
    req.url.append(":");
    req.url.append(m_PMPort);
    req.url.append("/v1/resource/host/");

    CheckConnectStatus::SecurityConfiguration(req, "caInfo");
    CheckConnectStatus::SecurityConfiguration(req, "sslCert");
    CheckConnectStatus::SecurityConfiguration(req, "sslKey");
    CheckConnectStatus::SecurityConfiguration(req, "cert");

    Json::Value registerReq;
    mp_int32 iRet = GetHostInfo(registerReq);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    Json::StreamWriterBuilder builder;
    req.body = Json::writeString(builder, registerReq);
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::DeleteHostFromPM()
{
    CHost host;
    mp_string hostSN;
    mp_int32 iRet = host.GetHostSN(hostSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query host information failed, iRet %d.", iRet);
        return iRet;
    }

    HttpRequest req;
    InitDeleteHostReq(hostSN, req);
    COMMLOG(OS_LOG_INFO, "Begin to delete host from PM, hostid is %s.", hostSN.c_str());

    mp_string rspBody;
    iRet = SendRequest(req, rspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d.", iRet);
        return iRet;
    }

    // prase errCode
    Json::Value jsonRspBody;
    iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert String to Json failed.");
        return iRet;
    }

    if ((!jsonRspBody.isObject()) || (!jsonRspBody.isMember(PARAM_KEY_ERRORCODE)) ||
        (!jsonRspBody.isMember(PARAM_KEY_ERRORMSG))) {
        COMMLOG(OS_LOG_ERROR, "rspBody string have no error_code.");
        return iRet;
    }
    mp_string strErrCode;
    mp_string errMsg;
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORCODE, strErrCode);
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORMSG, errMsg);

    COMMLOG(OS_LOG_DEBUG, "Delete host from PM success.");

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::InitDeleteHostReq(const mp_string hostid, HttpRequest& req)
{
    req.method = "POST";
    mp_int32 secure_channel;
    mp_int32 Ret =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (Ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Failed to obtain the secure communication method.");
        return Ret;
    }
    if (secure_channel == 1) {
        mp_string domain_name;
        // Obtaining Domain Name
        CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, domain_name);
        req.url.append("https://");
        req.url.append(domain_name);
        req.domaininfo.append("https://");
        req.domaininfo.append(domain_name);
        req.hostinfo.append(domain_name);
        req.hostinfo.append(":");
        req.hostinfo.append(m_PMPort);
        req.hostinfo.append(":");
        req.hostinfo.append(m_PMIp);
    } else {
        req.url.append("http://");
        m_PMIp = CIP::FormatFullUrl(m_PMIp);
        req.url.append(m_PMIp);
    }
    req.url.append(":");
    req.url.append(m_PMPort);
    req.url.append("/v1/resource/host/");

    CheckConnectStatus::SecurityConfiguration(req, "caInfo");
    CheckConnectStatus::SecurityConfiguration(req, "sslCert");
    CheckConnectStatus::SecurityConfiguration(req, "sslKey");
    CheckConnectStatus::SecurityConfiguration(req, "cert");

    Json::Value registerReq;
    mp_int32 iRet = GetHostInfo(registerReq);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get delete host info failed, iRet %d.", iRet);
        return iRet;
    }
    registerReq[PARAM_KEY_HOSTLINKSTATUS] = "0";
    Json::StreamWriterBuilder builder;
    req.body = Json::writeString(builder, registerReq);
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::GetHostInfo(Json::Value& hostMsg)
{
    CHost host;
    host_info_t hostInfo;
    mp_int32 iRet = host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query host information failed, iRet %d.", iRet);
        return iRet;
    }

    mp_string listenIP;
    mp_string listenPort;
    if (CIP::GetListenIPAndPort(listenIP, listenPort) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent listen IP and port failed.");
        return MP_FAILED;
    }

    hostMsg[PARAM_KEY_HOSTID] = hostInfo.sn;
    hostMsg[PARAM_KEY_HOSTNAME] = hostInfo.hostName;
    hostMsg[PARAM_KEY_ENVTYPE] = "0";
    hostMsg[PARAM_KEY_HOSTIP] = listenIP;
    hostMsg[PARAM_KEY_HOSTPORT] = listenPort;
    hostMsg[PARAM_KEY_HOSTLINKSTATUS] = "1";
    hostMsg[PARAM_KEY_HOSTOSTYPE] = hostInfo.nOS;
    hostMsg[PARAM_KEY_HOSTOSVER] = hostInfo.osVersion;
    COMMLOG(OS_LOG_INFO, "Query hostInfo.os is %d", hostInfo.nOS);

    mp_int32 proxyRole = 0;
    iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, proxyRole);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set default value %d.", proxyRole);
    }
    hostMsg[PARAM_KEY_HOSTPROXYTYPE] = proxyRole;
    COMMLOG(OS_LOG_DEBUG, "Set backup role %d.", proxyRole);
    return MP_SUCCESS;
}

mp_void CheckConnectStatus::PrintResult(const mp_string& actionType, const mp_int32& iRet)
{
    m_outputStr.clear();
    PrintActionType(actionType);

    if (iRet == MP_SUCCESS) {
        m_outputStr = m_outputStr + "success";
    } else {
        m_outputStr = m_outputStr + "failed";
    }

    cout << m_outputStr << endl;
}

mp_void CheckConnectStatus::PrintActionType(const mp_string& actionType)
{
    m_outputStr.clear();

    if (actionType == REGISTER_HOST) {
        m_outputStr = m_outputStr + "register host to protectmanager ";
    } else {
        m_outputStr = m_outputStr + "delete host from protectmanager ";
    }
}

mp_int32 CheckConnectStatus::CheckAgentRole()
{
    // Get current role: 0:Database 1:AnyBackup 2: VMware
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, m_AgentRole);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent role failed. iRet : %d", iRet);
        return iRet;
    } else {
        if (m_AgentRole == "0") {
            m_AgentRole = DBBACKUP_AGENT;
        } else if (m_AgentRole == "2") {
            m_AgentRole = VMBACKUP_AGENT;
        } else if (m_AgentRole == "3") {
            m_AgentRole = DWSBACKUP_AGENT;
        } else if (m_AgentRole == "4") {
            m_AgentRole = PLUGINBACKUP_AGENT;
        } else if (m_AgentRole == SANCLIENT_BACKUP_ROLE) {
            m_AgentRole = SANCLIENT_AGENT_PLUIN;
        } else {
            COMMLOG(OS_LOG_ERROR, "Current role is invalid, m_AgentRole: %s.", m_AgentRole.c_str());
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

#ifdef LINUX
mp_int32 CheckConnectStatus::GetDpcNodeConfig()
{
    m_isDpcNode = false;
    m_isInstalledDpcClient = false;
    mp_string isDpcComputeNode;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IS_DPC_COMPUTE_NODE, isDpcComputeNode);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get isDpcComputeNode failed.");
        return iRet;
    }
    m_isDpcNode = (isDpcComputeNode == "true") ? true : false;
    return MP_SUCCESS;
}
#endif

mp_int32 CheckConnectStatus::Init()
{
    CMpThread::InitLock(&m_Mutex);
    mp_int32 iRet = CheckAgentRole();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Current role is %s", m_AgentRole.c_str());

    iRet = CIP::GetListenIPAndPort(m_AgentIp, m_AgentPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent ip failed.");
        return iRet;
    }
    
    iRet = GetPMIPandPort();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed.");
        return iRet;
    }

#ifdef LINUX
    iRet = GetDpcNodeConfig();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get dpc node config failed.");
    }
#endif

    // 创建检查连通性线程 vcenter
    iRet = CMpThread::Create(&m_CheckConnectivityThread, CheckConnectivity, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create checkconnectivity handle thread failed, iRet %d.", iRet);
        return iRet;
    }

    // 上报心跳给PM
    m_heartBeatToPMThread = std::make_unique<std::thread>(std::bind(&CheckConnectStatus::ReportHeartBeatToPM, this));
    if (m_heartBeatToPMThread == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Create heartbeat to pm thread failed.");
        return iRet;
    }

    // 上报资源占用率给PM
    m_ResourceUsageToPMThread =
        std::make_unique<std::thread>(std::bind(&CheckConnectStatus::ReportResourceUsageToPM, this));
    if (m_ResourceUsageToPMThread == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Create resource usage to pm thread failed.");
        return iRet;
    }

    // 获取系统剩余可用资源
    m_GetSysResourceUsageThread =
        std::make_unique<std::thread>(std::bind(&CheckConnectStatus::GetSysResourceUsage, this));
    if (m_GetSysResourceUsageThread == nullptr) {
        COMMLOG(OS_LOG_ERROR, "Create getting system rest resource usage thread failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI CheckConnectStatus::CheckConnectivity(LPVOID param)
#else
mp_void* CheckConnectStatus::CheckConnectivity(mp_void* param)
#endif
{
    if (param == NULL) {
        CMPTHREAD_RETURN;
    }
    CheckConnectStatus* pthis = static_cast<CheckConnectStatus*>(param);
    if (strcmp(pthis->m_AgentRole.c_str(), VMBACKUP_AGENT.c_str())) {
        COMMLOG(OS_LOG_INFO, "Current role is not VMBackupAgent, no need check connect status.");
        CMPTHREAD_RETURN;
    }
    vector<pFuncGetComList> getComList;
    // if VMBackupAgent, need check connect status
    getComList.push_back(&CheckConnectStatus::GetPmControllerIpFromConfig);
    getComList.push_back(&CheckConnectStatus::GetvSphereIp);
    pthis->CheckConnectivitySub(pthis, getComList);
    CMPTHREAD_RETURN;
}

mp_void CheckConnectStatus::CheckVsphereConnectivity()
{
    mp_uint64 curTime = CMpTime::GetTimeSec();

    mp_int32 timeInterval;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION,
        CFG_CHECK_VSPHERE_CONN_TIME, timeInterval);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get check vcenter connnect time failed.");
        timeInterval = CHECH_VSPHERE_STATE_INTERVAL; // 从配置文件获取失败，赋予默认值2小时
    }

    if (curTime - m_lastUpdateVsphereConnectivityTime > timeInterval || m_lastUpdateVsphereConnectivityTime == 0) {
        m_lastUpdateVsphereConnectivityTime = curTime;
        UpdateVsphereConnectivity();
    }
}

mp_void* CheckConnectStatus::CheckConnectivitySub(CheckConnectStatus* pthis, vector<pFuncGetComList>& getComList)
{
    mp_uint32 intervalTime = 0;
    while (pthis->GetExitFlag() == MP_FALSE) {
        vector<Componet>().swap(pthis->m_ComList);
        for (int i = 0; i < getComList.size(); ++i) {
            mp_int32 iRet = (pthis->*getComList[i])();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "ComList parse failed, iRet = %d.", iRet);
            }
        }
        if (pthis->GetUpdateInterval(intervalTime) != MP_SUCCESS) {
            intervalTime = CK_MAX_TIMER_INTERVAL;
            COMMLOG(OS_LOG_ERROR, "GetUpdateInterval failed.");
        }
        if (pthis->m_ComList.size() < 1) {
            COMMLOG(OS_LOG_INFO, "ComList is null.");
            DoSleep(intervalTime);
        } else {
            mp_int32 iRet = pthis->CheckConnect();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "IP curl failed, iRet = %d.", iRet);
                return ((void*)0);
            }
            mp_uint64 tCurrentTime = CMpTime::GetTimeSec();
            COMMLOG(OS_LOG_DEBUG, "tCurrentTime %llu, lastReportFinishTime %llu", tCurrentTime, lastReportFinishTime);
            if (lastReportFinishTime <= tCurrentTime) {
                mp_uint32 sleepTime = ((tCurrentTime - lastReportFinishTime) * TO_MILLISECOND >= intervalTime) ? 0 :
                (intervalTime - ((tCurrentTime - lastReportFinishTime) * TO_MILLISECOND));
                COMMLOG(OS_LOG_DEBUG, "Will sleep %u ms", sleepTime);
                DoSleep(sleepTime);
            }
            iRet = pthis->UpdatePMInfo();
            if (iRet == MP_SUCCESS) {
                COMMLOG(OS_LOG_INFO, "Update link status success");
                lastReportFinishTime = CMpTime::GetTimeSec();
            }
        }
        // register agent to pm
        if (isRegisteredToPM == MP_FAILED) {
            mp_int32 iRet = MP_FAILED;
            iRet = pthis->Handle(REGISTER_HOST.c_str());
            if (iRet == MP_SUCCESS) {
                isRegisteredToPM = MP_SUCCESS;
            }
        }
    }
    return ((void*)0);
}

mp_int32 CheckConnectStatus::GetUpdateInterval(mp_uint32& intervaltime)
{
    mp_string timeStr;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_CHECK_CONN_TIME, timeStr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get check connnect time failed.");
        return iRet;
    }
    istringstream iss(timeStr);
    iss >> intervaltime;
    if (intervaltime > CK_MAX_TIMER_INTERVAL) {
        intervaltime = CK_MAX_TIMER_INTERVAL;
    }
    return MP_SUCCESS;
}

bool CheckConnectStatus::CheckEnabledReportHeartBeat()
{
    mp_string filePath = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + DISABLE_HEART_BEAT_FLAG_FILE;
    if (CMpFile::FileExist(filePath)) {
        COMMLOG(OS_LOG_DEBUG, "Disable report heartbeat to pm.");
        return false;
    }
    COMMLOG(OS_LOG_DEBUG, "Enable report heartbeat to pm.");
    return true;
}

void CheckConnectStatus::GetReportHeartBeatConfig()
{
    m_heartbeatToPmInterval = THREE_MINUTES;
    int interval = 0;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_HEARTBEAT_TO_PM_INTERVAL, interval);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cfg heartbeat to pm interval failed.");
        return;
    }
    interval *= ONE_SECOND;
    if (interval >= ONE_SECOND && interval <= FIVE_MINUTES) {
        m_heartbeatToPmInterval = interval;
    }
}

void CheckConnectStatus::GetReportResourceUsageConfig()
{
    int interval = 0;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_RESOURCE_USAGE_TO_PM_INTERVAL, interval);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cfg resource usage to pm interval failed.");
        return;
    }
    interval *= ONE_SECOND;
    if (interval >= ONE_SECOND && interval <= FIVE_MINUTES) {
        m_resourceUsageToPmInterval = interval;
    }
}

mp_int32 CheckConnectStatus::GetResourceAlarmThreshouldValue(mp_double& value, const mp_string& key)
{
    mp_int32 usage = DEFAULT_RESOURCE_USAGE;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_MONITOR_SECTION, CFG_RESOURCE_SECTION, key, usage);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cfg resource usage from config file failed.");
    }
    value = usage;
    return MP_SUCCESS;
}

std::vector<std::string> GetConnectIps(const std::vector<std::string>& ips, const std::string& port)
{
    std::vector<std::string> connectedIps;
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when register to PM.");
        IHttpClient::ReleaseInstance(httpClient);
        return connectedIps;
    }
 
    for (const auto& ip : ips) {
        mp_int32 retryTimes = 0;
        while (retryTimes < MAX_RETRY_TIMES) {
            if (httpClient->TestConnectivity(ip, port)) {
                connectedIps.push_back(ip);
                COMMLOG(OS_LOG_INFO, "Can cennect ip(%s).", ip.c_str());
                break;
            }
            retryTimes++;
            DoSleep(ONE_SECOND);
            COMMLOG(OS_LOG_WARN, "Can not cennect ip(%s).", ip.c_str());
        }
    }

    IHttpClient::ReleaseInstance(httpClient);
    return connectedIps;
}
void CheckConnectStatus::GetSysResourceUsage()
{
    COMMLOG(OS_LOG_DEBUG, "Start get system resource usage thread.");
    while (true) {
        if (m_stopFlag) {
            break;
        }
        GetSysReourceRestPrintConfig();
        GetSysResourceUsageInner();
        DoSleep(m_sysCpuMemRestPrint);
    }
    COMMLOG(OS_LOG_ERROR, "Exit get system resource usage thread.");
}

void CheckConnectStatus::GetSysReourceRestPrintConfig()
{
    int interval = 0;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_RESOURCE_REST_PRINT, interval);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cfg resource rest interval failed.");
        return;
    }
    interval *= ONE_SECOND;
    if (interval >= ONE_SECOND && interval <= FIVE_MINUTES) {
        m_sysCpuMemRestPrint = interval;
    }
}

void CheckConnectStatus::GetSysResourceUsageInner()
{
    COMMLOG(OS_LOG_DEBUG, "Start get system resource usage.");
    CHost host;
    SysResourceUsageRestT sysResourceUsageRest;
    mp_int32 iRet = host.GetCpuAndMemRate(sysResourceUsageRest);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get cpu and memory failed!");
        return;
    }
    double memLimite = 0;
    double cpuLimite = 0;
    GetResourceAlarmThreshouldValue(memLimite, "mem_avil_limit_log");
    GetResourceAlarmThreshouldValue(cpuLimite, "cpu_idle_limit_log");
    if (sysResourceUsageRest.sysCpuRateRest < cpuLimite || sysResourceUsageRest.sysMemRateRest < memLimite) {
        COMMLOG(OS_LOG_INFO, "cpu rest is %lf percent, avilable memory is %lf percent, %lu MB.",
            sysResourceUsageRest.sysCpuRateRest, sysResourceUsageRest.sysMemRateRest, sysResourceUsageRest.sysMemRest);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Do not need to print");
    }
    COMMLOG(OS_LOG_DEBUG, "Finish get system resource usage.");
}

void CheckConnectStatus::ReportResourceUsageToPM()
{
    COMMLOG(OS_LOG_DEBUG, "Start reporter resource usage thread.");    // 上报太频繁，日志改为debug
    while (true) {
        if (m_stopFlag) {
            break;
        }
        if (!CheckEnabledReportHeartBeat()) {
            DoSleep(DISABLE_CHECK_INTERVAL);
            continue;
        }
        GetReportResourceUsageConfig();
        ReportResourceUsageToPMInner();
        DoSleep(m_resourceUsageToPmInterval);
    }
    COMMLOG(OS_LOG_DEBUG, "Exit reporter resource usage thread.");
}
bool CheckConnectStatus::CheckIfIpsVecEmpty(std::vector<std::string>& VecIp)
{
    std::vector<std::string> allIps = m_PMIpVec;
    if (VecIp.empty()) {
        VecIp = ConnectivityManager::GetInstance().GetConnectedIps(
            allIps, CMpString::SafeStoi(m_PMPort));
    }
    if (VecIp.empty()) {
        COMMLOG(OS_LOG_ERROR, "Connected ips is empty.");
        return false;
    }
    return true;
}

mp_int32 CheckConnectStatus::ReportResourceUsageToPMInner()
{
    COMMLOG(OS_LOG_DEBUG, "Start report resource usage to PM.");
    // get uuid cpu mem info
    CHost host;
    ResourceUageInfoT ResourceUsageInfo;
    mp_int32 iRet = host.GetAgentResourceUsage(ResourceUsageInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query CPU and MEM failed, iRet %d.", iRet);
        return iRet;
    }
 
    // get ip and cfg
    if (GetPMIPandPort() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed.");
        return MP_FAILED;
    }

    // check connection
    if (!CheckIfIpsVecEmpty(m_connectedIps)) {
        COMMLOG(OS_LOG_ERROR, "Connected ips is empty.");
        return MP_FAILED;
    }

    // send request
    HttpRequest req;
    Json::Value jsonBody;
    jsonBody["uuid"] = ResourceUsageInfo.sn;
    jsonBody["cpuRate"] = ResourceUsageInfo.cpuUsage;
    jsonBody["memRate"] = ResourceUsageInfo.memUsage;

    GetResourceAlarmThreshouldValue(ResourceUsageInfo.cpuRateSendAlarmThreshold, "cpu_usage_send_alarm");
    GetResourceAlarmThreshouldValue(ResourceUsageInfo.memRateSendAlarmThreshold, "mem_usage_send_alarm");
    GetResourceAlarmThreshouldValue(ResourceUsageInfo.cpuRateClearAlarmThreshold, "cpu_usage_clear_alarm");
    GetResourceAlarmThreshouldValue(ResourceUsageInfo.memRateClearAlarmThreshold, "mem_usage_clear_alarm");

    jsonBody["cpuRateAlarmThreshold"] = ResourceUsageInfo.cpuRateSendAlarmThreshold;
    jsonBody["cpuRateClearAlarmThreshold"] = ResourceUsageInfo.cpuRateClearAlarmThreshold;
    jsonBody["memRateAlarmThreshold"] = ResourceUsageInfo.memRateSendAlarmThreshold;
    jsonBody["memRateClearAlarmThreshold"] = ResourceUsageInfo.memRateClearAlarmThreshold;

    for (const std::string& ip : m_connectedIps) {
        iRet = InitRequest("PUT", PUT_RESOURCE_USAGE_TO_PM, ip, req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Init request failed, iRet = %d.", iRet);
            continue;
        }
        Json::StreamWriterBuilder builder;
        req.body = Json::writeString(builder, jsonBody);
        COMMLOG(OS_LOG_DEBUG, "req body: %s.", req.body.c_str());
        mp_string rspBody;
        if (SendRequest(req, rspBody) == MP_SUCCESS) {
            COMMLOG(OS_LOG_DEBUG, "Report resource usage to PM success.");
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_WARN, "Report resource usage to PM use ip(%s) fail.", ip.c_str());
    }
    m_connectedIps.clear();
    COMMLOG(OS_LOG_ERROR, "Report resource usage to PM fail.");
    return MP_FAILED;
}

void CheckConnectStatus::ReportHeartBeatToPM()
{
    COMMLOG(OS_LOG_INFO, "Start reporter heartbeat thread.");
    while (true) {
        if (m_stopFlag) {
            break;
        }
        if (!CheckEnabledReportHeartBeat()) {
            DoSleep(DISABLE_CHECK_INTERVAL);
            continue;
        }
        GetReportHeartBeatConfig();
        RequestPmIpList();
        ReportHeartBeatToPMInner();
        DoSleep(m_heartbeatToPmInterval);
    }
    COMMLOG(OS_LOG_INFO, "Exit reporter heartbeat thread.");
}

#ifdef LINUX
mp_bool CheckConnectStatus::CheckDpcProcess()
{
    std::vector<mp_string> vecRlt;
    mp_string strCmd = "ps -ef | grep -w '" + DPC_AGENT_PATH + "' | grep -v 'grep'";
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    mp_bool bFlag = (iRet != MP_SUCCESS) || (vecRlt.empty());
    if (bFlag) {
        COMMLOG(OS_LOG_ERROR, "Dpc agent process is not started, iRet = %d, size of vecRlt is %d", iRet, vecRlt.size());
        return false;
    }
    return true;
}

mp_void CheckConnectStatus::ConfigDpcFlowControl()
{
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_DPC_CONFIG_FLOW_CONTROL, "", NULL);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Config dpc flow control failed.");
    }
    COMMLOG(OS_LOG_INFO, "Config dpc flow control succ.");
    return;
}
#endif

mp_bool CheckConnectStatus::IsInstallDataTurbo()
{
    // 判断源端重删是否启动
#ifdef WIN32
    // 查询dataturbo服务
    mp_string strCmd = "sc query dataturbo";
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
    if (iRet != MP_SUCCESS) {
        WARNLOG("The system not install dataturbo or the system does not support.");
        return MP_FALSE;
    }
#else
    mp_string strCmd = "ps -ef | grep /opt/oceanstor/dataturbo/bin/dpc | grep -v grep";
    std::vector<mp_string> vecRes;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRes);
    if (iRet != MP_SUCCESS || vecRes.size() == 0) {
        WARNLOG("The system not install dataturbo or the system does not support.");
        return MP_FALSE;
    }
#endif
    return MP_TRUE;
}

mp_int32 CheckConnectStatus::ReportHeartBeatToPMInner()
{
    COMMLOG(OS_LOG_INFO, "Start report heartbeat to PM.");
#ifdef LINUX
    if (m_isDpcNode && !m_isInstalledDpcClient) {
        if (CheckDpcProcess()) {
            m_isInstalledDpcClient = true;
            ConfigDpcFlowControl();
        } else {
            return MP_FAILED;
        }
    }
#endif
    // get host info
    CHost host;
    host_info_t hostInfo;
    mp_int32 iRet = host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host information failed, iRet %d.", iRet);
        return iRet;
    }
 
    // get ip and cfg
    iRet = GetPMIPandPort();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed.");
        return iRet;
    }
    std::vector<std::string> allIps = m_PMIpVec;

    // select ip which host can connect
    std::vector<std::string> connectedIps = ConnectivityManager::GetInstance().GetConnectedIps(
        allIps, CMpString::SafeStoi(m_PMPort));
    if (connectedIps.empty()) {
        COMMLOG(OS_LOG_ERROR, "Connected ips is empty.");
        return MP_FAILED;
    }
 
    // Get host IP List
    Json::Value jIpList;
    iRet = host.GetHostAgentIplist(jIpList);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host extend information agentiplist failed, iRet %d.", iRet);
    }

    Json::Value jsonBody;
    BuildHeatBeatBody(hostInfo, connectedIps, jIpList, jsonBody);
    iRet = SendHeatBeatRequest(connectedIps, jsonBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Report heartbeat to PM fail.");
    } else {
        COMMLOG(OS_LOG_INFO, "Report heartbeat to PM success.");
    }
    return iRet;
}

void CheckConnectStatus::BuildHeatBeatBody(host_info_t &hostInfo, std::vector<std::string> &connectedIps,
    Json::Value &jIpList, Json::Value &jsonBody)
{
    if (hostInfo.subType == BACKUP_ROLE_SANCLIENT) {
        jsonBody["uuid"] = hostInfo.sn + PARAM_SANCLIENT_UUID_SUFFIX;
    } else {
        jsonBody["uuid"] = hostInfo.sn;
    }
    jsonBody["hostname"] = hostInfo.hostName;
    jsonBody["agentVersion"] = hostInfo.version;
    for (auto ip : connectedIps) {
        jsonBody["connectedBusinessIps"].append(ip);
    }
    jsonBody["extendInfo"] = jIpList.toStyledString();

    mp_int32 logLevel = -1;
    auto iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_LOG_LEVEL, logLevel);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent log level failed.");
    } else {
        jsonBody["logLevel"] = logLevel;
    }
    jsonBody["src_deduption"] = IsInstallDataTurbo() ? true : false;
}

mp_int32 CheckConnectStatus::SendHeatBeatRequest(const std::vector<mp_string>& ipList, const Json::Value& jsonBody)
{
    HttpRequest req;
    mp_string reqmethod = "PUT";
    mp_string requrl = PUT_HEART_BEAT_TO_PM;
    for (const std::string& ip : ipList) {
        mp_int32 iRet = InitRequest(reqmethod, requrl, ip, req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Init request failed, iRet = %d.", iRet);
            continue;
        }
        Json::StreamWriterBuilder builder;
        req.body = Json::writeString(builder, jsonBody);
        COMMLOG(OS_LOG_DEBUG, "req body: %s.", req.body.c_str());
        mp_string rspBody;
        iRet = SendRequest(req, rspBody);
        if (iRet == MP_SUCCESS) {
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_WARN, "Report heartbeat to PM use ip(%s) fail.", ip.c_str());
    }
    return MP_FAILED;
}

mp_int32 CheckConnectStatus::RequestPmIpList()
{
    COMMLOG(OS_LOG_INFO, "Start to request for PM's ip-list.");
    // get ip and cfg
    mp_int32 iRet = GetPMIPandPort();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed.");
        return iRet;
    }
    std::vector<std::string> allIps = m_PMIpVec;

    // select ip which host can connect
    std::vector<std::string> connectedIps = ConnectivityManager::GetInstance().GetConnectedIps(
        allIps, CMpString::SafeStoi(m_PMPort));
    if (connectedIps.empty()) {
        COMMLOG(OS_LOG_ERROR, "Connected ip-list is empty.");
        return MP_FAILED;
    }

    // send request
    HttpRequest req;
    mp_string reqmethod = "GET";
    mp_string requrl = GET_IP_LIST_FROM_PM;
    mp_string responseBody;
    bool restSuccessFlag = false;
    for (const std::string& ip : connectedIps) {
        iRet = InitRequest(reqmethod, requrl, ip, req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Init request failed, iRet = %d.", iRet);
            continue;
        }
        Json::StreamWriterBuilder builder;
        iRet = SendRequest(req, responseBody);
        if (iRet == MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "Request for PM's ip-list successful.");
            restSuccessFlag = true;
            break;
        }
        COMMLOG(OS_LOG_WARN, "Request for PM's ip-list from ip(%s) failed.", ip.c_str());
    }
    if (!restSuccessFlag) {
        COMMLOG(OS_LOG_ERROR, "All requests for PM's ip-list are failed.");
        return MP_FAILED;
    }
    iRet = UpdatePmIpList(allIps, responseBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update PM's ip-list failed.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Request for PM's ip-list success.");
    return iRet;
}

mp_int32 CheckConnectStatus::UpdatePmIpList(const std::vector<mp_string>& oldIpList, const mp_string& newIpList)
{
    INFOLOG("Begin update manager server,newIpList is :%s.", newIpList.c_str());
    Json::Value jsonRspBody;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(newIpList, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ConvertStringtoJson failed, iRet = %d.", iRet);
        return iRet;
    }
    std::vector<mp_string> newIps;
    iRet = CJsonUtils::GetJsonArrayString(jsonRspBody, REST_MANAGER_SERVER_LIST, newIps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetJsonArrayJson failed, iRet = %d.", iRet);
        return iRet;
    }

    std::set<mp_string> oldIpsSet(oldIpList.begin(), oldIpList.end());
    std::set<mp_string> newIpsSet(newIps.begin(), newIps.end());
    mp_string emptyStr;
    newIpsSet.erase(emptyStr);
    if (oldIpsSet == newIpsSet) {
        INFOLOG("Pm ip list no change.");
        return MP_SUCCESS;
    }
    newIps.assign(newIpsSet.begin(), newIpsSet.end());
    if (newIps.empty()) {
        WARNLOG("Get pm ip list is empty, will not update, src newIpListStr is %s.", newIpList.c_str());
        return MP_FAILED;
    }
    mp_string ipStr = CMpString::StrJoin(newIps, ",");
    iRet = CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipStr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write configure failed.");
        return iRet;
    }

    mp_string filePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    iRet = ModifyLineData(filePath, PARAM_KEY_PM_IP, ipStr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to modify the configuration.");
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Update pm ip list to (%s) success.", ipStr.c_str());
    return iRet;
}

mp_int32 CheckConnectStatus::GetPMIPandPort()
{
    mp_string ipstr;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipstr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }

    // 按“，”分割Ip,ip之间注意不要有空格
    std::vector<std::string> ips;
    CMpString::StrSplit(ips, ipstr, ',');
    if (!ips.empty() && ips.back().empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM ip failed, PM ip list is empty(%s).", ipstr.c_str());
        return MP_FAILED;
    }
    m_PMIpVec = ips;

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, m_PMPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM port failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Get PM ip %s, port %s success.", ipstr.c_str(), m_PMPort.c_str());
    return iRet;
}

mp_int32 CheckConnectStatus::InitRequest(
    const mp_string& reqmethod, const mp_string& requrl, const mp_string& ip, HttpRequest& req)
{
    mp_int32 Ret = IHttpClient::InitStructHttpRequest(req);
    req.method = reqmethod;
    std::string tempIp = CIP::FormatFullUrl(ip);
    mp_int32 secure_channel;
    Ret = CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (Ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Failed to obtain the secure communication method.");
        return Ret;
    }
    if (secure_channel == 1) {
        mp_string domain_name;
        mp_int32 iRet =
            CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, domain_name);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_WARN, "Failed to obtain the domain name.");
            return iRet;
        }
        req.url.append(PROTOCOL_SECURE);
        req.url.append(domain_name);
        req.domaininfo.append(PROTOCOL_SECURE);
        req.domaininfo.append(domain_name);
        req.hostinfo.append(domain_name);
        req.hostinfo.append(":");
        req.hostinfo.append(m_PMPort);
        req.hostinfo.append(":");
        req.hostinfo.append(tempIp);
    } else {
        req.url.append(PROTOCOL);
        req.url.append(tempIp);
    }
    req.url.append(":");
    req.url.append(m_PMPort);
    req.url.append(requrl);

    CheckConnectStatus::SecurityConfiguration(req, "caInfo");
    CheckConnectStatus::SecurityConfiguration(req, "sslCert");
    CheckConnectStatus::SecurityConfiguration(req, "sslKey");
    CheckConnectStatus::SecurityConfiguration(req, "cert");

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::SendRequest(const HttpRequest& req, mp_string& responseBody)
{
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == NULL) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when register to PM.");
        return MP_FAILED;
    }

    mp_int32 retryTimes = 0;
    IHttpResponse* dpaHttpRespone = NULL;

    while (retryTimes < MAX_RETRY_TIMES) {
        // 内部通过new分配
        dpaHttpRespone = httpClient->SendRequest(req, HTTP_CONNECT_TIMEOUT);
        COMMLOG(OS_LOG_DEBUG, "\n method: %s\n hostinfo: %s\n domaininfo: %s\n",
            req.method.c_str(), req.hostinfo.c_str(), req.domaininfo.c_str());
        if (dpaHttpRespone == NULL) {
            ++retryTimes;
            COMMLOG(OS_LOG_ERROR, "curl http initialization response failed.");
            continue;
        }

        mp_int32 errCode = dpaHttpRespone->GetErrCode();
        mp_uint32 statusCode = dpaHttpRespone->GetHttpStatusCode();
        if (dpaHttpRespone->Success()) {
            COMMLOG(OS_LOG_DEBUG, "status: %u, send times = %d.", dpaHttpRespone->GetHttpStatusCode(), retryTimes + 1);
            // 获取接口信息
            responseBody = dpaHttpRespone->GetBody();
            break;
        } else {
            COMMLOG(OS_LOG_WARN, "req token failed now, err %d, status code %u.", errCode, statusCode);
            delete dpaHttpRespone;
            dpaHttpRespone = NULL;
        }

        ++retryTimes;
        DoSleep(DELAY_TIME);
    }

    if (dpaHttpRespone) {
        delete dpaHttpRespone;
        dpaHttpRespone = NULL;
    }

    IHttpClient::ReleaseInstance(httpClient);

    if (retryTimes >= MAX_RETRY_TIMES) {
        COMMLOG(OS_LOG_ERROR, "send url:%s info with %d times failed.", req.url.c_str(), retryTimes);
        return MP_FAILED;
    }
    COMMLOG(OS_LOG_DEBUG, "send url:%s info with %d times success.", req.url.c_str(), retryTimes);
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::UpdatePMInfo()
{
    Json::Value registerReq;

    mp_int32 iRet = BuildPMBody(registerReq);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }
    iRet = UpdatePMInfo(registerReq);

    return iRet;
}

mp_int32 CheckConnectStatus::UpdatePMInfo(Json::Value registerReq)
{
    HttpRequest req;
    mp_string reqmethod = "PUT";
    mp_string requrl = PUT_INTERFACE;

    mp_int32 iRet = MP_FAILED;
    if (m_connectedPMInLastRound.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "m_connectedPMInLastRound is empty");
    }
    for (int i = 0; i < m_connectedPMInLastRound.size(); ++i) {
        iRet = InitRequest(reqmethod, requrl, m_connectedPMInLastRound[i], req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "InitRequest failed, iRet = %d.", iRet);
            continue;
        }

        Json::StreamWriterBuilder builder;
        req.body = Json::writeString(builder, registerReq);

        mp_string rspBody;
        COMMLOG(OS_LOG_DEBUG, "UpdatePMInfo body");
        iRet = SendRequest(req, rspBody);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Update PM info failed, pm_ip = %s, iRet = %d.",
                m_connectedPMInLastRound[i].c_str(), iRet);
            continue;
        } else {
            COMMLOG(OS_LOG_DEBUG, "UpdatePMInfo success.");
            break;
        }
    }

    return iRet;
}

mp_bool CheckConnectStatus::VerifyLinkStatusBody(const Json::Value& body)
{
    uint64_t minLength = 1;
    std::set<std::string> keySet = {"sourceRole", "sourceAddr", "destRole", "destAddr"};
    for (const std::string& key : keySet) {
        if (!body.isMember(key) || !body[key].isString()) {
            COMMLOG(OS_LOG_ERROR, "VerifyLinkStatusBody Failed: %s", body.toStyledString().c_str());
            return MP_FALSE;
        }
        if (body[key].asString().size() < minLength) {
            COMMLOG(OS_LOG_ERROR, "VerifyLinkStatusBody Failed: %s", body.toStyledString().c_str());
            return MP_FALSE;
        }
    }
    return MP_TRUE;
}

mp_int32 CheckConnectStatus::BuildPMBody(Json::Value& PMMsg)
{
    time_t timeTmp;
    time(&timeTmp);
    mp_uint64 timeStamp = timeTmp;
    for (int i = 0; i < m_ComList.size(); ++i) {
        Json::Value PMBody;
        PMBody["sourceRole"] = m_AgentRole;
        PMBody["sourceAddr"] = m_AgentIp;
        PMBody["destRole"] = m_ComList[i].destRole;
        if (m_ComList[i].destName != "") {  // node 要上传名字
            PMBody["destAddr"] = m_ComList[i].destName;
        } else {
            auto leftBracket = m_ComList[i].destAddr6.find("[");
            if (leftBracket != std::string::npos) {
                m_ComList[i].destAddr6.replace(leftBracket, 1, "");
            }
            auto rightBracket = m_ComList[i].destAddr6.find("]");
            if (rightBracket != std::string::npos) {
                m_ComList[i].destAddr6.replace(rightBracket, 1, "");
            }
            mp_string ipType;
            if (CIP::IsIPv6(m_ComList[i].destAddr6)) { // ipv6和ipv4互斥，一个存在，另一个即为空
                ipType = "ipv6";
            }
            if (CIP::IsIPV4(m_ComList[i].destAddr)) {
                ipType = "ipv4";
            }

            if (ipType == "ipv6") {
                PMBody["destAddr"] = m_ComList[i].destAddr6;
            } else {
                PMBody["destAddr"] = m_ComList[i].destAddr;
            }
        }
        PMBody["state"] = m_ComList[i].state;
        PMBody["updateTime"] = static_cast<Json::UInt64>(timeStamp);
        if (!VerifyLinkStatusBody(PMBody)) {
            COMMLOG(OS_LOG_WARN, "VerifyLinkStatusBody failed.");
            continue;
        }
        COMMLOG(OS_LOG_DEBUG,
                "BuildPMBody success.\n sourceRole: %s\n sourceAddr: %s\
            \n destRole: %s\n destAddr: %s\n destAddr6: %s\n link_status: %d\n updateTime: %ul\n",
                m_AgentRole.c_str(), m_AgentIp.c_str(), m_ComList[i].destRole.c_str(), m_ComList[i].destAddr.c_str(),
                m_ComList[i].destAddr6.c_str(), m_ComList[i].state, timeStamp);
        PMMsg.append(std::move(PMBody));
    }
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::CheckComponentConnect(Componet& comp, const mp_string& MP_PORT)
{
    CURL* curl = curl_easy_init();
    if (curl == NULL) {
        return MP_FAILED;
    }
    int timeout = DEFAULT_TEST_CONNECTIVITY_TIMEOUT;
    CConfigXmlParser::GetInstance().GetValueInt32(CFG_SYSTEM_SECTION, CFG_CURL_CONNECTIVITY_TIMEOUT, timeout);
    timeout = (timeout > 0) ? timeout : DEFAULT_TEST_CONNECTIVITY_TIMEOUT;
    string url = comp.destAddr;
    string destRole = comp.destRole;
    if (destRole == CONTROLLER) {
        url = url + MP_PORT;
    }
    if (comp.mainType == "vSphere") {
        url = "https://" + url + ":" + comp.port;
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    }
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
    CURLcode CurlCode = curl_easy_perform(curl);
    if (CurlCode == CURLE_OK) {
        comp.state = CAN_CONNECT;
    } else if (destRole == CONTROLLER && comp.destAddr6 != "") { // 节点连通性ipv6
        COMMLOG(OS_LOG_DEBUG, "ComList destAddrv6=%s.", comp.destAddr6.c_str());
        url = comp.destAddr6 + MP_PORT;
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        CURLcode CurlCode = curl_easy_perform(curl);
        if (CurlCode == CURLE_OK) {
            comp.state = CAN_CONNECT;
        } else {
            comp.state = CAN_NOT_CONNECT;
        }
    } else {
        comp.state = CAN_NOT_CONNECT;
    }
    if (comp.state == CAN_CONNECT) {
        COMMLOG(OS_LOG_DEBUG, "CheckConnect success url=%s.", url.c_str());
    } else {
        COMMLOG(OS_LOG_DEBUG, "CheckConnect fail url=%s.", url.c_str());
    }
    if (comp.mainType == "vSphere" && CheckVDDK() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Invalid VDDK lib, stop reporting connectivity to Vsphere");
        comp.state = CAN_NOT_CONNECT;
    }
    curl_easy_cleanup(curl);
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::CheckConnect()
{
    // 获取port
    mp_string PM_PORT;
    CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, PM_PORT);
    mp_string MP_PORT = mp_string(":") + PM_PORT;

    curl_global_init(CURL_GLOBAL_ALL);
    for (int i = 0; i < m_ComList.size(); ++i) {
        if (m_ComList[i].mainType == "vSphere" && CheckVDDK() != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Invalid VDDK lib, stop reporting connectivity to Vsphere");
            m_ComList[i].state = CAN_NOT_CONNECT;
        } else {
            // only to check the connectivity of vSphere destinations (controllers have been checked before)
            if (m_ComList[i].mainType == "vSphere" && CheckComponentConnect(m_ComList[i], MP_PORT) != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Check connectivity of vSphere %s failed", m_ComList[i].destAddr.c_str());
                return MP_FAILED;
            }
        }
    }
    curl_global_cleanup();

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::GetvSphereJsonValue(
    mp_int32 page_no, const mp_string& requrl, vector<Json::Value>& vecJsonValue)
{
    HttpRequest req;
    mp_string reqmethod = "GET";
    mp_string responseBody;
    Json::Value jsonRspBody;
    for (int i = 0; i < m_connectedPMInLastRound.size(); ++i) {
        mp_int32 iRet = InitRequest(reqmethod, requrl, m_connectedPMInLastRound[i], req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "InitRequest failed, iRet = %d.", iRet);
            return iRet;
        }
        iRet = SendRequest(req, responseBody);
        if (iRet == MP_SUCCESS) {
            m_PMIp = m_connectedPMInLastRound[i];
            break;
        } else {
            COMMLOG(OS_LOG_ERROR,
                "SendRequest failed, PM ip = %s, iRet = %d.", m_connectedPMInLastRound[i].c_str(), iRet);
        }
    }
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(responseBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ConvertStringtoJson failed, iRet = %d.", iRet);
        return iRet;
    }
    vector<Json::Value> vecValue;
    iRet = CJsonUtils::GetJsonArrayJson(jsonRspBody, ITEMS, vecValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetJsonArrayJson failed, iRet = %d.", iRet);
        return iRet;
    }
    vecJsonValue.insert(vecJsonValue.end(), vecValue.begin(), vecValue.end());
    mp_int32 pages = 0;
    iRet = CJsonUtils::GetJsonInt32(jsonRspBody, PAGES, pages);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetJsonInt32 failed, iRet = %d.", iRet);
        return iRet;
    }
    ++page_no;
    // page_no is start from 0, so 'page_no < pages' means all the pages have been get from PM
    if (page_no < pages) {
        stringstream ss;
        ss << page_no;
        mp_string strPageNum = ss.str();
        mp_string newRequrl = "/v1/internal/environments?page_no=" + strPageNum +
                              "&page_size=" + CHECKCONNECTPAGESIZE;
        GetvSphereJsonValue(page_no, newRequrl, vecJsonValue);
    }

    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::GetvSphereIp()
{
    CThreadAutoLock lock(&m_Mutex);
    CheckVsphereConnectivity();

    int size = m_vecValue.size();
    for (int i = 0; i < size; ++i) {
        Componet temp;
        GET_JSON_STRING_OPTION(m_vecValue[i], TYPE, temp.mainType);
        GET_JSON_STRING_OPTION(m_vecValue[i], SUBTYPE, temp.destRole);
        if (temp.mainType == "vSphere") {
            GET_JSON_STRING_OPTION(m_vecValue[i], ENDPOINT, temp.destAddr);
            GET_JSON_INT32_OPTION(m_vecValue[i], LINKSTATUS, temp.state);
            temp.destAddr = CIP::FormatFullUrl(temp.destAddr);
            mp_string destAddr = CIP::FormatFullUrl(temp.destAddr);
            auto leftBracket = destAddr.find("[");
            if (leftBracket != std::string::npos) {
                destAddr.replace(leftBracket, 1, "");
            }
            auto rightBracket = destAddr.find("]");
            if (rightBracket != std::string::npos) {
                destAddr.replace(rightBracket, 1, "");
            }
            if (CIP::IsIPv6(destAddr)) {
                temp.destAddr6 = CIP::FormatFullUrl(temp.destAddr);
            }
            m_ComList.push_back(temp);

            COMMLOG(OS_LOG_DEBUG,
                "\n sourceRole: %s\n destRole: %s\n destAddr: %s\n destAddr6: %s\n link_status: %d\n",
                m_AgentRole.c_str(), temp.destRole.c_str(), temp.destAddr.c_str(), temp.destAddr6.c_str(), temp.state);
        }
    }
    if (m_ComList.size() < 1) {
        COMMLOG(OS_LOG_INFO, "vSphere comlist is null.");
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_DEBUG, "GetvSphereIp success.");
        return MP_SUCCESS;
    }
}

mp_int32 CheckConnectStatus::CheckVDDK()
{
    mp_string vddkPath = CPath::GetInstance().GetAgentVDDKPath();
    if (!CMpFile::DirExist(vddkPath.c_str())) {
        COMMLOG(OS_LOG_ERROR, "Vddk lib is not installed on the specified path '%s'", vddkPath.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::UpdateVsphereConnectivity()
{
    mp_int32 iRet = MP_SUCCESS;
    if (strcmp(m_AgentRole.c_str(), VMBACKUP_AGENT.c_str()) == 0) {
        vector<Json::Value> vecValue;
        iRet = GetvSphereJsonValue(0, GET_INTERFACE, vecValue);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_INFO, "GetvSphereJsonValue false");
        } else {
            m_vecValue = std::move(vecValue);
        }
    }
    return iRet;
}

mp_int32 CheckConnectStatus::DoSendRequestClusterNodes(const mp_string& requrl, vector<Json::Value>& vecJsonValue)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to Do SendRequestClusterNodes");
    mp_int32 pages;
    HttpRequest req;
    mp_string reqmethod = "GET";
    mp_string responseBody;
    Json::Value jsonRspBody;
    for (int i = 0; i < m_PMIpVec.size(); ++i) {
        mp_int32 iRet = InitRequest(reqmethod, requrl, m_PMIpVec[i], req);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "InitRequest failed, iRet = %d.", iRet);
            return iRet;
        }
        iRet = SendRequest(req, responseBody);
        if (iRet == MP_SUCCESS) {
            m_PMIp = m_PMIpVec[i];
            break;
        } else {
            COMMLOG(OS_LOG_ERROR, "SendRequest failed, PM ip = %s, iRet = %d.", m_PMIpVec[i].c_str(), iRet);
        }
    }
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(responseBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ConvertStringtoJson failed, iRet = %d.", iRet);
        return iRet;
    }
    vector<Json::Value> vecValue;
    iRet= CJsonUtils::GetArrayJson(jsonRspBody, vecValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetArrayJson failed, iRet = %d.", iRet);
        return iRet;
    }
    vecJsonValue.insert(vecJsonValue.end(), vecValue.begin(), vecValue.end());
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::GetPmControllerIp()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to Get PmController Ip");
    vector<Json::Value> vecValue;
    mp_int32 iRet = DoSendRequestClusterNodes(GET_CLUSTERS_NODES, vecValue);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "DoSendRequestClusterNodes failed, iRet = %d.", iRet);
        return iRet;
    }
    for (int i = 0; i < vecValue.size(); ++i) {
        Componet temp;
        temp.destRole = CONTROLLER;
        GET_JSON_STRING_OPTION(vecValue[i], CLUSTERS_NODES_V4, temp.destAddr);
        GET_JSON_STRING_OPTION(vecValue[i], CLUSTERS_NODES_V6, temp.destAddr6);
        GET_JSON_STRING_OPTION(vecValue[i], NODENAME, temp.destName);
        temp.destAddr6 = CIP::FormatFullUrl(temp.destAddr6);
        m_ComList.push_back(temp);
        COMMLOG(OS_LOG_DEBUG, "DoSendRequestClusterNodes %d destRole=%s destAddr=%s destAddr6=%s destName=%s",
            i, temp.destRole.c_str(), temp.destAddr.c_str(), temp.destAddr6.c_str(), temp.destName.c_str());
    }
    return MP_SUCCESS;
}

mp_int32 CheckConnectStatus::GetPmControllerIpFromConfig()
{
    LOGGUARD("");
    vector<mp_string> vecValue;
    mp_string strPmControllerIp;

    CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION,
        CFG_ADMINNODE_IP, strPmControllerIp);
    // 按“，”分割Ip,ip之间注意不要有空格
    CMpString::StrSplit(vecValue, strPmControllerIp, ',');
    if (!vecValue.empty() && vecValue.back().empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM controller ip failed, PM ip list is empty(%s).", strPmControllerIp.c_str());
        return MP_FAILED;
    }
    // check PM ip connectivity by ConnectivityManager
    m_connectedPMInLastRound =
        ConnectivityManager::GetInstance().GetConnectedIps(vecValue, CMpString::SafeStoi(m_PMPort));
    std::set<std::string> connectedIpSet;
    for (const std::string& a : m_connectedPMInLastRound) {
        COMMLOG(OS_LOG_DEBUG, "IP: %s Port: %s can be connected", a.c_str(), m_PMPort.c_str());
        connectedIpSet.insert(a);
    }
    for (int i = 0; i < vecValue.size(); ++i) {
        Componet temp;
        temp.state = CAN_NOT_CONNECT;
        temp.destRole = CONTROLLER;
        if (CIP::IsIPV4(vecValue[i]) && vecValue[i] != ANY_IP) {
            temp.destAddr = vecValue[i];
            if (connectedIpSet.count(temp.destAddr) > 0) {
                temp.state = CAN_CONNECT;
            }
        }

        if (CIP::IsIPv6(vecValue[i]) && vecValue[i] != ANY_IP) {
            temp.destAddr6 = vecValue[i];
            if (connectedIpSet.count(temp.destAddr6) > 0) {
                temp.state = CAN_CONNECT;
            }
        }
        temp.destAddr6 = CIP::FormatFullUrl(temp.destAddr6);
        m_ComList.push_back(temp);
        COMMLOG(OS_LOG_DEBUG, "ClusterNodes %d destRole=%s destAddr=%s destAddr6=%s destName=%s state:%llu",
            i, temp.destRole.c_str(), temp.destAddr.c_str(), temp.destAddr6.c_str(), temp.destName.c_str(), temp.state);
    }

    return MP_SUCCESS;
}

mp_void CheckConnectStatus::SecurityConfiguration(HttpRequest& req, const mp_string& actionType)
{
    if (actionType == "caInfo") {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(
            CFG_SECURITY_SECTION, CFG_PM_CA_INFO, req.caInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get caInfo value");
            return;
        }
    } else if (actionType == "sslCert") {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_CERT, req.sslCert);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get sslCert value");
            return;
        }
    } else if (actionType == "sslKey") {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_KEY, req.sslKey);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get sslKey value");
            return;
        }
    } else if (actionType == "cert") {
        mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_CERT, req.cert);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to get sslKey value");
            return;
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "The configuration file parameter is incorrect.");
        return;
    }
}