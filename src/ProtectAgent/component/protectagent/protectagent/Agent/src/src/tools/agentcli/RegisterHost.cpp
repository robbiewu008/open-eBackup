/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file RegisterHost.cpp
 * @brief  Contains function declarations Register to Host
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "tools/agentcli/RegisterHost.h"

#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/ConfigXmlParse.h"
#include "host/host.h"
#include "common/AppVersion.h"
#include "common/File.h"
#include "common/Ip.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "securecom/Password.h"
#include "message/tcp/CSocket.h"
#include "message/curlclient/RestClientCommon.h"

namespace {
const mp_string REGISTER_HOST("RegisterHost");
const mp_string DELETE_HOST("DeleteHost");
const mp_string REGISTER_UPGRADE("Upgrade");

// host info
const mp_string PARAM_KEY_HOSTID = "host_id";
const mp_string PARAM_KEY_HOSTNAME = "name";
const mp_string PARAM_KEY_ENVTYPE = "env_type";
const mp_string PARAM_KEY_HOSTIP = "ip";
const mp_string PARAM_KEY_HOSTPORT = "port";
const mp_string PARAM_KEY_REGISTER_TYPE = "register_type";
const mp_string PARAM_KEY_HOSTLINKSTATUS = "link_status";
const mp_string PARAM_KEY_HOSTOSTYPE = "os_type";
const mp_string PARAM_KEY_HOSTOSVER = "os_ver";
const mp_string PARAM_KEY_HOSTPROXYTYPE = "proxy_type";
const mp_string USER_ID = "userid";
const mp_string DUPLICATE_IP_ERROR = "error_duplicate_ip";
const mp_string PARAM_CLIENT_VERSION = "agent_version";
const mp_string PARAM_VERSION_TIME_STAMP = "agent_timestamp";
// pm rspbody key
const mp_string PARAM_KEY_ERRORCODE = "error_code";
const mp_string PARAM_KEY_ERRORMSG = "error_msg";
const mp_string PARAM_KEY_EXTENDINFO = "extendInfo";
const mp_string PARAM_KEY_INNER_ESN = "internal_agent_esn";
// ESN
const mp_string INNER_ESN = "esn";
const mp_int32 MAX_RETRY_TIMES = 3;
const mp_int32 DELAY_TIME = 3 * 1000;
const mp_int32 INITIAL_REGISTER_FLAG = 0;  // 初始注册
const mp_int32 RE_REGISTER_FLAG = 1;       // 二次注册
const mp_int32 UPGRADE_FLAG = 1;           // 升级注册
const mp_int32 INSTALL_FLAG = 0;           // 安装注册

// 注册 旧接口使用 v1 新接口使用 v2
const mp_string REGISTER_URL_V1 = "/v1/resource/host/";
const mp_string REGISTER_URL_V2 = "/v2/internal/environments";

// openstack metadata查询接口，OP服务化场景使用
const mp_string OPENSTACK_METADATA_URL = "/openstack/latest/meta_data.json";

// host info
const mp_string PARAM_KEY_HOST_UUID = "uuid";
const mp_string PARAM_KEY_HOST_NAME = "name";
const mp_string PARAM_KEY_HOST_TYPE = "type";
const mp_string PARAM_KEY_HOST_ENDPOINT = "endpoint";
const mp_string PARAM_KEY_HOST_PORT = "port";
const mp_string PARAM_KEY_HOST_SUBTYPE = "subType";  // 0:host 2:vmware 3:dws 4:plugins
const mp_string PARAM_KEY_HOST_USERNAME = "username";
const mp_string PARAM_KEY_HOST_PASSWORD = "password";
const mp_string PARAM_KEY_HOST_OSTYPE = "osType";
const mp_string PARAM_KEY_HOST_VERSION = "version";
const mp_string PARAM_KEY_HOST_PACKAGE_VERSION = "version";
const mp_string PARAM_KEY_HOST_PACKAGE_TIME_STAMP = "createdTime";
const mp_string PARAM_KEY_HOST_EXTENDINFO = "extendInfo";
const mp_string PARAM_KEY_HOST_AUTH = "auth";
const mp_string PARAM_KEY_HOST_AUTH_TYPE = "authType";
const mp_string PARAM_KEY_HOST_AUTH_KEY = "authKey";
const mp_string PARAM_KEY_HOST_AUTH_PWD = "authPwd";
const mp_string PARAM_KEY_HOST_EXTEND_SCENARIO = "scenario";
const mp_string PARAM_KEY_HOST_EXTEND_AGENTIPLIST = "agentIpList";
const mp_string PARAM_KEY_HOST_SRC_DEDUPTION = "src_deduption";
const mp_string PARAM_KEY_AUTO_SYNC_HOST_NAME = "is_auto_synchronize_host_name";
const mp_string PARAM_SANCLIENT_UUID_SUFFIX = "_sanclient";
const mp_string PARAM_KEY_HOST_APPLICATIONS = "agent_applications";
const mp_string PARAM_KEY_HOST_AVAILABLE_ZONE = "availableZone";
const mp_string PARAM_KEY_HOST_SUBNET_FIXED_IP = "subNetFixedIp";
const mp_string PARAM_KEY_HOST_PROJECT_ID = "projectId";
const mp_string PARAM_KEY_HOST_NATIVE_ID = "nativeId";
const mp_string PARAM_KEY_HOST_VPC_ID = "vpcId";
const mp_string PARAM_KEY_PUSH_REGISTER_FLAG = "pushRegister";
const mp_string PARAM_KEY_HOST_IS_SHARED = "isShared";
const mp_string PARAM_KEY_REGISTERTYPE = "registerType";
// backup role
const mp_int32 BACKUP_ROLE_GENERAL_PLUGIN = 4;
const mp_int32 BACKUP_ROLE_SANCLIENT_PLUGIN = 5;
// register response param key
const mp_string PARAM_KEY_ERRORCODE_V2 = "errorCode";
const mp_string PARAM_KEY_ERRORMSG_V2 = "errorMessage";
const mp_string PARAM_KEY_DETAILPARAMS_V2 = "parameters";

// key in client.conf which created by PM
const mp_string PM_KEY_APPLICATION_INFO = "application_info";
const mp_string RUN_CFG_KEY_APPLICATION_INFO = "APPLICATION_INFO";

// unssport plugins info
const mp_string UNSUPPORT_PLUGINS_INFO_FILE = "unssport_plugins.info";
const mp_string UNSUPPORT_APPS_INFO_FILE = "unssport_apps.info";

// error code
const mp_int32 REGISTER_ERROR_CODE_DUPLICATE_NAME = 0x64032f06;
// port
const mp_string REGISTER_PORT = "25082";

const mp_string INNER_AGENT_ESN_FILE = "/opt/cluster_config/CLUSTER_ESN";
}  // namespace
std::vector<mp_string> RegisterHost::m_PMIpVec;
mp_string RegisterHost::m_PMPort;
mp_string RegisterHost::m_outputStr;
mp_string RegisterHost::pm_ip;  // Temporary storage ip port
mp_string RegisterHost::pm_port;
mp_int32 RegisterHost::m_registerFlag = INITIAL_REGISTER_FLAG;  // 注册标志
mp_int32 RegisterHost::m_registerType = INSTALL_FLAG;
mp_int32 RegisterHost::m_proxyRole = -1;

mp_int32 RegisterHost::Handle(const mp_string& actionType, const mp_string& actionType2, const mp_string& actionType3)
{
    m_registerFlag = INITIAL_REGISTER_FLAG;
    m_registerType = INSTALL_FLAG;

    if (actionType != REGISTER_HOST && actionType != DELETE_HOST) {
        std::cout << "Input action type is error, only support \"RegisterHost\" and \"DeleteHost\" type." << std::endl;
        return MP_FAILED;
    }

    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, pm_ip);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Obtaining the PM ip address of the configuration file failed.");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, pm_port);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Obtaining the PM port address of the configuration file failed.");
        return iRet;
    }

    // Re-register PM
    if (!actionType2.empty() && actionType3.empty() && actionType2 != REGISTER_UPGRADE) {
        COMMLOG(OS_LOG_ERROR, "registerHost parameter input error.");
        std::cout << "please input [path]agentcli registerHost <RegisterHost> <pm_ip> <pm_port>." << std::endl;
        return MP_FAILED;
    } else if (actionType2 == REGISTER_UPGRADE) {
        m_registerType = UPGRADE_FLAG;
        COMMLOG(OS_LOG_INFO, "m_registerType is upgrade.");
    } else if (!actionType2.empty()) {
        std::vector<mp_string> pmIpVec;
        CMpString::StrSplit(pmIpVec, actionType2, ',');
        for (mp_int32 i = 0; i < pmIpVec.size(); i++) {
            if (!CIP::CheckIsIPv6OrIPv4(pmIpVec[i]) && pmIpVec[i] != ANY_IP) {
                COMMLOG(OS_LOG_ERROR, "Obtaining the PM port address of the configuration file failed.");
                return MP_FAILED;
            }
        }
        CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, actionType2);
        CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_IAM_PORT, actionType3);
        m_registerFlag = RE_REGISTER_FLAG;
    }

    iRet = GetPMIPandPort();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip and port failed, iRet = %d.", iRet);
        return iRet;
    }
    iRet = (actionType == REGISTER_HOST) ? ReportHost() : DeleteHost();

    PrintResult(actionType, iRet);

    return iRet;
}

mp_int32 RegisterHost::ReportHost()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to register host to ProtectManager.");
    // step1: if agent is inner agent, send request to get esn
    mp_int32 installType = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_SCENE, installType);
    if (iRet == MP_SUCCESS && installType == AGENT_INSTALL_TYPE_INTERNAL) {
        iRet = ObtainEsn();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Obtain esn failed.");
        }
    }
    // step2: register host to ProtectManager
    iRet = RegisterHost2PM();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Register host to pm success.");
    return MP_SUCCESS;
}

mp_int32 RegisterHost::DeleteHost()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to delete host from ProtectManager.");
    mp_int32 iRet = DeleteHostFromPM();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Delete host from pm success.");
    return MP_SUCCESS;
}

mp_int32 RegisterHost::GetPMIPandPort()
{
    mp_string ipstr;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipstr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }

    // 按“，”分割Ip,ip之间注意不要有空格
    CMpString::StrSplit(m_PMIpVec, ipstr, ',');
    if (m_PMIpVec.empty()) {
        COMMLOG(OS_LOG_ERROR, "Split PM ip failed, PM ip list is empty(%s).", ipstr.c_str());
        return MP_FAILED;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_IAM_PORT, m_PMPort);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM port failed.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Get PM ip list %s, port %s success.", ipstr.c_str(), m_PMPort.c_str());
    return iRet;
}

mp_int32 RegisterHost::UpdateNginxConf()
{
    mp_int32 iRet = MP_FAILED;

    std::string strNginxConfig = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    std::string strNginxConfigBack = strNginxConfig + "back";
    if (CMpFile::CopyFileCoverDest(strNginxConfig, strNginxConfigBack) != MP_SUCCESS) {
        return MP_FAILED;
    }

    std::ifstream stream;
    stream.open(strNginxConfig.c_str(), std::ifstream::in);
    std::ofstream streamout;
    streamout.open(strNginxConfigBack.c_str(), std::ifstream::out);
    std::string line;

    mp_string ipstr;
    std::vector<mp_string> pMIpVec;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, ipstr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get PM ip list address failed.");
        return iRet;
    }
    // 按“，”分割Ip,ip之间注意不要有空格
    CMpString::StrSplit(pMIpVec, ipstr, ',');

    std::string strAllowIPlist = "";
    for (int i = 0; i < pMIpVec.size(); i++) {
        strAllowIPlist = "            allow " + pMIpVec[i] + ";" + STR_CODE_WARP + strAllowIPlist;
    }

    if (stream.is_open() && streamout.is_open()) {
        while (getline(stream, line)) {
            if (line.find("allow ") == -1) {
                streamout << line << STR_CODE_WARP;
            }
            if (line.find("location /{") != -1) {
                streamout << strAllowIPlist;
                iRet = MP_SUCCESS;
            }
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Open Nginx config file failed.");
    }

    if (stream.is_open()) {
        stream.close();
    }
    if (streamout.is_open()) {
        streamout.flush();
        streamout.close();
    }

    if (iRet == MP_SUCCESS) {
        CMpFile::CopyFileCoverDest(strNginxConfigBack, strNginxConfig);
    }
    CMpFile::DelFile(strNginxConfigBack);
    return iRet;
}

mp_int32 RegisterHost::RestartNginx()
{
    LOGGUARD("");

#ifdef WIN32
    mp_string strCmd = mp_string("sc stop ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
    strCmd = mp_string("sc start ") + AGENT_SERVICE_NAME;
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#else
    mp_string strCmd = CPath::GetInstance().GetBinFilePath(STOP_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

    strCmd = CPath::GetInstance().GetBinFilePath(START_SCRIPT);
    strCmd = CMpString::BlankComma(strCmd);
    CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
    CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));
#endif

    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 RegisterHost::RegisterHost2PM()
{
    HttpRequest req;
    mp_string rspBody;
    mp_string registerPMIp = "";
    mp_int32 iRet = -1;
    mp_uint32 statusCode = 0;

    // 发送请求在更新nginx配置文件之后，防止PM第一次访问和重启nginx过程冲突
    for (int i = 0; i < m_PMIpVec.size(); i++) {
        iRet = InitRegisterReq(req, m_PMIpVec[i]);
        if (iRet != MP_SUCCESS) {
            return MP_FAILED;
        }
        iRet = SendRequest(req, rspBody, statusCode);
        if (iRet == MP_SUCCESS) {
            registerPMIp = m_PMIpVec[i];
            break;
        } else {
            continue;
        }
        COMMLOG(OS_LOG_ERROR, "Failed to initialize the registration request, iRet = %d.", iRet);
    }

    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        return ParseRegisterHostRespondsV2(rspBody, statusCode);
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d. m_registerFlag = %d", iRet, m_registerFlag);
        if (m_registerFlag == RE_REGISTER_FLAG) {  // 二次注册需要回滚
            CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_ADMINNODE_IP, pm_ip);
            CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_IAM_PORT, pm_port);
        }
        return iRet;
    }

    // prase errCode
    return ParseRegisterHostResponds(rspBody);
}

mp_int32 RegisterHost::InitRegisterReq(HttpRequest& req, mp_string& m_PMIp)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, m_proxyRole);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse backup role config failed, set default value %d.", m_proxyRole);
        return iRet;
    }

    iRet = InitUrl(req, m_PMIp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init register url fail.");
        return iRet;
    }

    RegisterHost::SecurityConfiguration(req, "caInfo");
    RegisterHost::SecurityConfiguration(req, "sslCert");
    RegisterHost::SecurityConfiguration(req, "sslKey");
    RegisterHost::SecurityConfiguration(req, "cert");

    Json::Value registerReq;
    iRet = GetHostInfo(registerReq);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }
    
    Json::StreamWriterBuilder builder;
    req.body = Json::writeString(builder, registerReq);
    return MP_SUCCESS;
}

mp_int32 RegisterHost::ObtainEsn()
{
    COMMLOG(OS_LOG_INFO, "Start to obtain for esn.");
    std::ifstream stream;
    mp_string line;
    stream.open(INNER_AGENT_ESN_FILE.c_str(), std::ifstream::in);

    if (!stream.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Obtain esn form configuration failed.");
        return MP_FAILED;
    }
    std::vector<mp_string> vecEsn;
    while (getline(stream, line)) {
        vecEsn.push_back(line);
    }
    if (vecEsn.empty()) {
        COMMLOG(OS_LOG_ERROR, "VecEsn is empty.");
        return MP_FAILED;
    }
    mp_string esn = vecEsn.front();
    mp_int32 iRet = CConfigXmlParser::GetInstance().SetValue(CFG_SYSTEM_SECTION, CFG_INNER_ESN, esn);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Write esn to configuration failed.");
    }
    return iRet;
}

mp_int32 RegisterHost::DeleteHostFromPM()
{
    HttpRequest req;
    CHost host;
    mp_string hostSN;
    mp_int32 iRet = host.GetHostSN(hostSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query host information failed, iRet %d.", iRet);
        return iRet;
    }

    mp_string rspBody;
    mp_uint32 statusCode = 0;
    for (int i = 0; i < m_PMIpVec.size(); i++) {
        iRet = InitDeleteHostReq(hostSN, req, m_PMIpVec[i]);
        if (iRet != MP_SUCCESS) {
            continue;
        }
        iRet = SendRequest(req, rspBody, statusCode);
        if (iRet == MP_SUCCESS) {
            break;
        } else {
            continue;
        }
    }

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Register to PM failed, iRet = %d.", iRet);
        return iRet;
    }

    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        INFOLOG("Delete host from PM success.");
        return MP_SUCCESS;
    }

    // prase errCode
    Json::Value jsonRspBody;
    iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert String to Json failed.");
        return iRet;
    }

    if ((!jsonRspBody.isMember(PARAM_KEY_ERRORCODE)) || (!jsonRspBody.isMember(PARAM_KEY_ERRORMSG))) {
        COMMLOG(OS_LOG_ERROR, "rspBody string have no error_code, %s.", rspBody.c_str());
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Delete host from PM success.");

    return MP_SUCCESS;
}

mp_int32 RegisterHost::InitDeleteHostReq(const mp_string& hostid, HttpRequest& req, mp_string& m_PMIp)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, m_proxyRole);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse backup role config failed, set default value %d.", m_proxyRole);
        return iRet;
    }

    mp_int32 Ret = InitUrl(req, m_PMIp);
    if (Ret != MP_SUCCESS) {
        ERRLOG("Init register url fail.");
        return Ret;
    }

    RegisterHost::SecurityConfiguration(req, "caInfo");
    RegisterHost::SecurityConfiguration(req, "sslCert");
    RegisterHost::SecurityConfiguration(req, "sslKey");
    RegisterHost::SecurityConfiguration(req, "cert");

    Json::Value registerReq;
    iRet = GetHostInfo(registerReq);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get delete host info failed, iRet %d.", iRet);
        return iRet;
    }

    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        mp_string envId;
        GET_JSON_STRING(registerReq, PARAM_KEY_HOST_UUID, envId);
        req.url.append("/");
        req.url.append(envId);
        req.method = "DELETE";
        DBGLOG("DeleteHost url: %s", req.url.c_str());
    } else {
        registerReq[PARAM_KEY_HOSTLINKSTATUS] = "0";
        Json::StreamWriterBuilder builder;
        req.body = Json::writeString(builder, registerReq);
    }
    return MP_SUCCESS;
}

mp_int32 RegisterHost::SendRequest(const HttpRequest& req, mp_string& responseBody, mp_uint32& _statusCode)
{
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == NULL) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when register to PM.");
        return MP_FAILED;
    }

    mp_int32 retryTimes = 0;
    IHttpResponse* dpaHttpRespone = NULL;

    while (retryTimes < MAX_RETRY_TIMES) {
        dpaHttpRespone = httpClient->SendRequest(req, HTTP_TIME_OUT);  // 内部通过new分配
        if (dpaHttpRespone == NULL) {
            retryTimes++;
            COMMLOG(OS_LOG_ERROR, "curl http initialization response failed.");
            continue;
        }

        mp_int32 errCode = dpaHttpRespone->GetErrCode();
        mp_uint32 statusCode = dpaHttpRespone->GetHttpStatusCode();
        if (dpaHttpRespone->Success()) {
            COMMLOG(OS_LOG_DEBUG, "status: %u, send times = %d.", dpaHttpRespone->GetHttpStatusCode(), retryTimes + 1);
            responseBody = dpaHttpRespone->GetBody();
            _statusCode = statusCode;
            break;
        } else {
            responseBody = dpaHttpRespone->GetBody();
            _statusCode = statusCode;
            COMMLOG(OS_LOG_WARN, "req token failed now, err %d, status code %u.", errCode, statusCode);
            delete dpaHttpRespone;
            dpaHttpRespone = NULL;
        }

        retryTimes++;
        DoSleep(DELAY_TIME);
    }

    if (dpaHttpRespone) {
        delete dpaHttpRespone;
    }

    IHttpClient::ReleaseInstance(httpClient);

    if (retryTimes >= MAX_RETRY_TIMES) {
        COMMLOG(OS_LOG_ERROR, "send url:%s info with %d times failed.", req.url.c_str(), retryTimes);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "send url:%s info with %d times success.", req.url.c_str(), retryTimes);
    return MP_SUCCESS;
}

mp_int32 RegisterHost::GetHostInfo(Json::Value& hostMsg)
{
    mp_string userid;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, USER_ID, userid);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse backup userid failed, set default value %s.", userid.c_str());
    }
    hostMsg[USER_ID] = userid;

    mp_string isAutoSynchronizeHostName;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_IS_AUTO_SYNCHRONIZE_HOST_NAME,
        isAutoSynchronizeHostName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Parse isAutoSynchronizeHostName failed, set default value %s.", isAutoSynchronizeHostName.c_str());
    }
    hostMsg[PARAM_KEY_AUTO_SYNC_HOST_NAME] = (isAutoSynchronizeHostName == "false") ? false : true;

    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        return GetHostInfoV2(hostMsg);
    }

    CHost host;
    host_info_t hostInfo;
    iRet = host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host information failed, iRet %d.", iRet);
        return iRet;
    }

    hostMsg[PARAM_KEY_HOSTID] = hostInfo.sn;
    hostMsg[PARAM_KEY_HOSTNAME] = hostInfo.hostName;
    hostMsg[PARAM_KEY_ENVTYPE] = "0";
    hostMsg[PARAM_KEY_HOSTIP] = hostInfo.endPoint;
    hostMsg[PARAM_KEY_HOSTPORT] = std::to_string(hostInfo.port);
    hostMsg[PARAM_KEY_HOSTLINKSTATUS] = "1";
    hostMsg[PARAM_KEY_HOSTOSTYPE] = hostInfo.strOS;
    hostMsg[PARAM_KEY_HOSTOSVER] = hostInfo.osVersion;
    hostMsg[PARAM_KEY_HOSTPROXYTYPE] = hostInfo.subType;
    hostMsg[PARAM_KEY_HOST_SRC_DEDUPTION] = IsInstallDataTurbo() ? true : false;
    (mp_void) GetOtherHostInfo(hostMsg);

    mp_string esn = "";
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_INNER_ESN, esn);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set default value %s.", esn);
    }
    hostMsg[PARAM_KEY_REGISTER_TYPE] = (m_registerType == UPGRADE_FLAG) ? "2" : "1";
    hostMsg[PARAM_KEY_EXTENDINFO][PARAM_KEY_INNER_ESN] = esn;
    COMMLOG(OS_LOG_INFO,
        "Set inner esn :%s,registerType is :%s.",
        esn,
        hostMsg[PARAM_KEY_REGISTER_TYPE].toStyledString().c_str());
    return MP_SUCCESS;
}

mp_void SetHostInfo(Json::Value& hostMsg, const host_info_t& hostInfo, const mp_int32& proxyRole)
{
    if (proxyRole == BACKUP_ROLE_GENERAL_PLUGIN) {
        hostMsg[PARAM_KEY_HOST_UUID] = hostInfo.sn;
    } else {
        hostMsg[PARAM_KEY_HOST_UUID] = hostInfo.sn + PARAM_SANCLIENT_UUID_SUFFIX;
    }
    hostMsg[PARAM_KEY_HOST_NAME] = hostInfo.hostName;
    hostMsg[PARAM_KEY_HOST_TYPE] = "Host";
    hostMsg[PARAM_KEY_HOST_PORT] = hostInfo.port;
    if (proxyRole == BACKUP_ROLE_GENERAL_PLUGIN) {
        hostMsg[PARAM_KEY_HOST_SUBTYPE] = "UBackupAgent";
    } else {
        hostMsg[PARAM_KEY_HOST_SUBTYPE] = "SBackupAgent";
    }
    hostMsg[PARAM_KEY_HOST_USERNAME] = "";
    hostMsg[PARAM_KEY_HOST_PASSWORD] = "";
    hostMsg[PARAM_KEY_HOST_OSTYPE] = hostInfo.strOS;
    hostMsg[PARAM_KEY_HOST_VERSION] = hostInfo.osVersion;
}

mp_int32 RegisterHost::GetExtendInfo(Json::Value& jvalueExtendInfo, mp_string& endpoint)
{
    CHost host;
    mp_int32 iRet = host.GetHostExtendInfo(jvalueExtendInfo, m_proxyRole);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host extend information failed, iRet %d.", iRet);
        return MP_FAILED;
    }
    Json::Value jIpList;
    iRet = host.GetHostAgentIplist(jIpList);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host extend information agentiplist failed, iRet %d.", iRet);
        return MP_FAILED;
    }
    mp_string applications = "";
    iRet = GetRegisterAppInfo(applications);
    if (iRet == MP_SUCCESS  && !applications.empty()) {
        DBGLOG("Query applications success, application is [%s]", applications.c_str());
        jvalueExtendInfo[PARAM_KEY_HOST_APPLICATIONS] = applications;
    }
    mp_string installMode = "";
    if (host.GetInstallMode(installMode) == MP_SUCCESS &&
        installMode == "push") {
        jvalueExtendInfo[PARAM_KEY_PUSH_REGISTER_FLAG] = true;
    }
    jvalueExtendInfo[PARAM_KEY_HOST_APPLICATIONS] = applications;

    jvalueExtendInfo[PARAM_KEY_HOST_SRC_DEDUPTION] = IsInstallDataTurbo() ? true : false;
    jvalueExtendInfo[PARAM_KEY_HOST_EXTEND_AGENTIPLIST] = jIpList[PARAM_KEY_HOST_EXTEND_AGENTIPLIST];
    if (QueryEcsMetaDataInfo(jIpList[PARAM_KEY_HOST_EXTEND_AGENTIPLIST].asString(),
        jvalueExtendInfo) != MP_SUCCESS) {
        WARNLOG("Query metadata information failed.");
        // query metadata failed, try to get available_zone from testcfg.tmp
        mp_string az = "";
        host.GetAvailableZone(az);
        jvalueExtendInfo[PARAM_KEY_HOST_AVAILABLE_ZONE] = az;
    } else {
        INFOLOG("Query metadata information success.");
        jvalueExtendInfo[PARAM_KEY_HOST_SUBNET_FIXED_IP] = endpoint;
    }
    mp_string esn = "";
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_INNER_ESN, esn);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set default value %s.", esn.c_str());
    }
    jvalueExtendInfo[PARAM_KEY_INNER_ESN] = esn;
    return MP_SUCCESS;
}

mp_int32 RegisterHost::GetHostInfoV2(Json::Value& hostMsg)
{
    CHost host;
    host_info_t hostInfo;
    mp_int32 iRet = host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host information failed, iRet %d.", iRet);
        return iRet;
    }
    SetHostInfo(hostMsg, hostInfo, m_proxyRole);
    mp_string eip = "";
    mp_string floatingIp = "";
    if (host.GetEipInfo(eip) == MP_SUCCESS && !eip.empty()) {
        COMMLOG(OS_LOG_INFO, "Query eip information success, eip = %s.", eip.c_str());
        hostMsg[PARAM_KEY_HOST_ENDPOINT] = eip;
    } else if (host.GetFloatingIp(floatingIp) == MP_SUCCESS && !floatingIp.empty()) {
        COMMLOG(OS_LOG_INFO, "Query ecs floating ip success, floatingIp = %s.", floatingIp.c_str());
        hostMsg[PARAM_KEY_HOST_ENDPOINT] = floatingIp;
    } else {
        hostMsg[PARAM_KEY_HOST_ENDPOINT] = hostInfo.endPoint;
    }
    Json::Value jvalueExtendInfo;
    if (GetExtendInfo(jvalueExtendInfo, hostInfo.endPoint) != MP_SUCCESS) {
        ERRLOG("Query extend information failed, iRet %d.", iRet);
        return MP_FAILED;
    }
    hostMsg[PARAM_KEY_HOST_EXTENDINFO] = std::move(jvalueExtendInfo);
    COMMLOG(OS_LOG_DEBUG, "Host extend info = %s.",
        Json::FastWriter().write(hostMsg[PARAM_KEY_HOST_EXTENDINFO]).c_str());
    Json::Value jvaluePackageInfo;
    (mp_void) GetOtherHostInfo(jvaluePackageInfo);
    hostMsg[PARAM_KEY_HOST_PACKAGE_VERSION] = jvaluePackageInfo[PARAM_CLIENT_VERSION];
    hostMsg[PARAM_KEY_HOST_PACKAGE_TIME_STAMP] = jvaluePackageInfo[PARAM_VERSION_TIME_STAMP];
    mp_string isShared = "";
    host.GetIsSharedAgent(isShared);
    if (isShared == "true") {
        hostMsg[PARAM_KEY_HOST_IS_SHARED] = true;
    } else {
        hostMsg[PARAM_KEY_HOST_IS_SHARED] = false;
    }
    hostMsg[PARAM_KEY_REGISTERTYPE] = (m_registerType == UPGRADE_FLAG) ? "2" : "1";
    COMMLOG(OS_LOG_INFO, "registerType is :%s.", hostMsg[PARAM_KEY_REGISTERTYPE].toStyledString().c_str());
    return MP_SUCCESS;
}

mp_void RegisterHost::PrintResult(const mp_string& actionType, const mp_int32 iRet)
{
    m_outputStr.clear();
    PrintActionType(actionType);

    if (iRet == MP_SUCCESS) {
        m_outputStr = m_outputStr + "success";
    } else {
        m_outputStr = m_outputStr + "failed";
    }

    std::cout << m_outputStr << std::endl;
}

mp_void RegisterHost::PrintActionType(const mp_string& actionType)
{
    if (actionType == REGISTER_HOST) {
        m_outputStr = m_outputStr + "Register host to ProtectManager ";
    } else {
        m_outputStr = m_outputStr + "Delete host from ProtectManager ";
    }
}
mp_void RegisterHost::SecurityConfiguration(HttpRequest& req, const mp_string& actionType)
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

    return;
}

mp_int32 RegisterHost::ReplaceHostSN(const Json::Value& jsonRspBody)
{
    mp_string strInput;
    CHost m_host;
    mp_string strAgentModeFilePath = CPath::GetInstance().GetLogFilePath(AGENT_PUSH_MODEFILE_PATH);
    mp_int32 iRet = m_host.CheckAgentInstallMode();
    if (iRet == MP_SUCCESS) {
        ERRLOG("Push installation is not supported in the current scenario.");
        return MP_FAILED;
    }
    mp_string strDestDir = CPath::GetInstance().GetConfPath();
    strDestDir = CMpString::BlankComma(strDestDir);
    std::cout << CHANGE_HOSTSN_NOTICE << CHANGE_HOSTSN_NOTICE1;
    std::cout << CHANGE_HOSTSN_VALUE;
    CPassword::GetInput(CHANGE_HOSTSN_VALUE, strInput);
    mp_bool bRet = (strInput == "y" || strInput == "Y");
    if (bRet == MP_TRUE) {
        mp_string hostsnLastValue;
        GET_JSON_STRING(jsonRspBody, PARAM_KEY_HOSTID, hostsnLastValue);
        if (hostsnLastValue.empty()) {
            ERRLOG("The ProtectManager returns an empty host uuid.");
            return MP_FAILED;
        }
        std::vector<mp_string> vecMacs;
        vecMacs.push_back(hostsnLastValue);
        // delete old hostsn file
        mp_string hostsnFile = HOSTSN_DIR + HOSTSN_FILE;
#ifdef WIN32
        mp_string strCmd = mp_string("cmd.exe /c del ") + hostsnFile + mp_string(" ") +
            strDestDir + mp_string("\\") + HOSTSN_FILE + " >nul";
#else
        mp_string strCmd = mp_string("rm ") + hostsnFile + mp_string(" ") + strDestDir + mp_string("/") + HOSTSN_FILE;
        CHECK_FAIL_EX(CheckCmdDelimiter(strCmd));
#endif
        COMMLOG(OS_LOG_INFO, "The current HostSN file is about to be deleted. cmd=%s", strCmd.c_str());
        CHECK_FAIL_EX(CSystemExec::ExecSystemWithoutEcho(strCmd));

        mp_int32 iRet = CIPCFile::WriteFile(hostsnFile, vecMacs);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Write hostsn into hostsn file failed [%d].", iRet);
            return iRet;
        }
        RegisterHost2PM();
    } else {
        printf("If you want to use the current host, please go to ProtectManager to delete the existing host.\n");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 RegisterHost::ParseRegisterHostResponds(const mp_string& rspBody)
{
    Json::Value jsonRspBody;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert String to Json failed.");
        return iRet;
    }

    if (jsonRspBody.isMember(DUPLICATE_IP_ERROR)) {
        mp_string duplicateIp;
        GET_JSON_STRING(jsonRspBody, DUPLICATE_IP_ERROR, duplicateIp);
        if (duplicateIp != "Unknown") {  // if no duplicate IP address exists, the PM returns Unknown.
            return ReplaceHostSN(jsonRspBody);
        }
    }

    if ((!jsonRspBody.isMember(PARAM_KEY_ERRORCODE)) || (!jsonRspBody.isMember(PARAM_KEY_ERRORMSG))) {
        COMMLOG(OS_LOG_INFO, "rspBody string have no error_code.");
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 RegisterHost::GetAndReplaceHostSN(const mp_string& rspBody)
{
    mp_int32 installType = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_SCENE, installType);
    if (iRet == MP_SUCCESS && installType != AGENT_INSTALL_TYPE_INTERNAL) {
        INFOLOG("Not inner agent do not need replace host SN.");
        return MP_SUCCESS;
    }

    if (rspBody.empty()) {
        ERRLOG("Body is empty.");
        return MP_FAILED;
    }

    Json::Value jsonRspBody;
    iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS || Json::ValueType::objectValue != jsonRspBody.type()) {
        ERRLOG("Convert String to Json failed.");
        return iRet;
    }

    if (!jsonRspBody.isMember("uuid") || !jsonRspBody["uuid"].isString()) {
        ERRLOG("RspBody string have no host SN.");
        return MP_FAILED;
    }

    mp_string hostSN = jsonRspBody["uuid"].asString();
    if (hostSN.empty()) {
        INFOLOG("Host SN return from PM is empty.");
        return MP_SUCCESS;
    }
    INFOLOG("Host SN return from PM: %s", hostSN.c_str());

    mp_string strDestDir = CPath::GetInstance().GetConfPath();
    strDestDir = CMpString::BlankComma(strDestDir);
    mp_string confHostsnFile = strDestDir + mp_string("/") + HOSTSN_FILE;
    std::vector<mp_string> vecInput = { hostSN };
    iRet = CIPCFile::WriteFile(confHostsnFile, vecInput);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write HostSN file failed, iRet = %d.", iRet);
        return iRet;
    }

    INFOLOG("Host SN replace to %s success.", hostSN.c_str());
    return MP_SUCCESS;
}

mp_int32 RegisterHost::ParseRegisterHostRespondsV2(const mp_string& rspBody, mp_uint32& statusCode)
{
    if (statusCode == SC_OK) {
        GetAndReplaceHostSN(rspBody);
        return MP_SUCCESS;
    }

    if (rspBody.empty()) {
        ERRLOG("Body is empty.");
        return MP_FAILED;
    }

    Json::Value jsonRspBody;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(rspBody, jsonRspBody);
    if (iRet != MP_SUCCESS || Json::ValueType::objectValue != jsonRspBody.type()) {
        ERRLOG("Convert String to Json failed.");
        return iRet;
    }

    if ((!jsonRspBody.isMember(PARAM_KEY_ERRORCODE_V2)) || (!jsonRspBody.isMember(PARAM_KEY_ERRORMSG_V2))) {
        ERRLOG("RspBody string have no errorCode.");
        return MP_FAILED;
    }

    mp_string strErrCode;
    mp_string errMsg;
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORCODE_V2, strErrCode);
    GET_JSON_STRING(jsonRspBody, PARAM_KEY_ERRORMSG_V2, errMsg);
    ERRLOG("Responds errcode=%s, errmsg=%s.", strErrCode.c_str(), errMsg.c_str());

    mp_int32 nErrCode = atoi(strErrCode.c_str());
    ERRLOG("nErrCode=%d, code=%d.", nErrCode, REGISTER_ERROR_CODE_DUPLICATE_NAME);
    if (nErrCode == REGISTER_ERROR_CODE_DUPLICATE_NAME) {
        INFOLOG("Duplicate environment exists");
        
        const mp_int32 detailCountMix = 2;
        std::vector<mp_string> vecDetail;
        GET_JSON_ARRAY_STRING(jsonRspBody, PARAM_KEY_DETAILPARAMS_V2, vecDetail);
        if (vecDetail.size() < detailCountMix) {
            ERRLOG("DetailParams size: %d err", vecDetail.size());
            return MP_FAILED;
        }

        const mp_int32 idxType = 0;
        const mp_int32 idexMsg = 1;
        if (vecDetail[idxType] == "uuid") {
            INFOLOG("Duplicate uuid exists");
            jsonRspBody[PARAM_KEY_HOSTID] = vecDetail[idexMsg];
            return ReplaceHostSN(jsonRspBody);
        } else if (vecDetail[idxType] == "endpoint") {
            INFOLOG("Duplicate endpoint exists");
            return MP_SUCCESS;
        }
    }
    return MP_FAILED;
}

mp_void RegisterHost::GetOtherHostInfo(Json::Value& hostMsg)
{
    mp_int32 proxyRole = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_ROLE, proxyRole);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup role config failed, set default value %d.", proxyRole);
    }

    hostMsg[PARAM_KEY_HOSTPROXYTYPE] = proxyRole;
    COMMLOG(OS_LOG_INFO, "Set backup role %d.", proxyRole);

    mp_string userid;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, USER_ID, userid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup userid failed, set default value %s.", userid.c_str());
    }
    hostMsg[USER_ID] = userid;
    COMMLOG(OS_LOG_INFO, "Set USER_ID %s.", userid.c_str());

    mp_string curVersion;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_CLIENT_VERSION, curVersion);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup curVersion failed, set default value %s.", curVersion.c_str());
    }
    hostMsg[PARAM_CLIENT_VERSION] = curVersion;
    COMMLOG(OS_LOG_INFO, "Set host version %s.", curVersion.c_str());

    mp_string versionTimeStamp;
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_VERSION_TIME_STAMP, versionTimeStamp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "parse backup userid failed, set default value %s.", versionTimeStamp.c_str());
    }
    hostMsg[PARAM_VERSION_TIME_STAMP] = versionTimeStamp;
    COMMLOG(OS_LOG_INFO, "Set client packing timestamp %s.", versionTimeStamp.c_str());
    CHost host;
    host.GetHostExtendInfo(hostMsg, proxyRole);
}

mp_int32 RegisterHost::UpdateNginxConfAndRestart()
{
    COMMLOG(OS_LOG_DEBUG, "Begin to update nginx config and restart.");
    if (pm_ip.empty()) {
        COMMLOG(OS_LOG_ERROR, "The ProtectManger ip address of the configuration file is empty.");
        return MP_FAILED;
    }

    mp_int32 iRet = UpdateNginxConf();
    if (iRet == MP_SUCCESS) {
        if (RestartNginx() != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Failed to restart nginx.");
            std::cout << "Because changed PM ip, please restart nginx server." << std::endl;
            COMMLOG(OS_LOG_ERROR, "Because changed PM ip, please restart nginx server.");
            return MP_FAILED;
        }
    } else {  // 更新nginx配置失败后，因为无法跟新ip进行通讯，所以需要回滚ip等参数
        COMMLOG(OS_LOG_ERROR, "Failed to update nginx config.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Update nginx config and restart success.");
    return MP_SUCCESS;
}

mp_int32 RegisterHost::InitUrl(HttpRequest& req, mp_string& m_PMIp)
{
    mp_int32 secure_channel;
    mp_int32 Ret = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_SYSTEM_SECTION, CFG_SECURE_CHANNEL, secure_channel);
    if (Ret != MP_SUCCESS) {
        ERRLOG("Failed to obtain the secure communication method.");
        return Ret;
    }
    mp_string pmPort = m_PMPort;
    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        pmPort = REGISTER_PORT;
    }
    Ret = IHttpClient::InitStructHttpRequest(req);
    if (Ret != MP_SUCCESS) {
        ERRLOG("Init http request fail.");
        return Ret;
    }
    req.method = "POST";
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
        req.hostinfo.append(pmPort);
        req.hostinfo.append(":");
        req.hostinfo.append(m_PMIp);
    } else {
        req.url.append("http://");
        m_PMIp = CIP::FormatFullUrl(m_PMIp);
        req.url.append(m_PMIp);
    }
    req.url.append(":");
    req.url.append(pmPort);

    // 通用插件注册接口变更
    if (m_proxyRole == BACKUP_ROLE_GENERAL_PLUGIN || m_proxyRole == BACKUP_ROLE_SANCLIENT_PLUGIN) {
        req.url.append(REGISTER_URL_V2);
    } else {
        req.url.append(REGISTER_URL_V1);
    }
    return MP_SUCCESS;
}

mp_bool RegisterHost::IsInstallDataTurbo()
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

mp_int32 RegisterHost::QueryEcsMetaDataInfo(const mp_string& agentIpList, Json::Value& jv)
{
    mp_int32 installType = 0;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_BACKUP_SECTION, CFG_BACKUP_SCENE, installType);
    if (iRet == MP_SUCCESS && installType == AGENT_INSTALL_TYPE_INTERNAL) {
        COMMLOG(OS_LOG_WARN, "Inner agent do not need query Ecs meta info.");
        return MP_FAILED;
    }
    // 首先检查连通性
    mp_string metadataServerIp;
    mp_string metadataServerPort;
    if (CConfigXmlParser::GetInstance().GetValueString(
        CFG_SYSTEM_SECTION, CFG_OPENSTACK_METADATA_SERVER_IP, metadataServerIp) == MP_FAILED) {
        ERRLOG("Get openstack_metadata_server_ip from config file failed.");
        return MP_FAILED;
    }
    if (CConfigXmlParser::GetInstance().GetValueString(
        CFG_SYSTEM_SECTION, CFG_OPENSTACK_METADATA_SERVER_PORT, metadataServerPort) == MP_FAILED) {
        ERRLOG("Get openstack_metadata_server_port from config file failed.");
        return MP_FAILED;
    }
    mp_int32 linkMetadataServerStatus = MP_FAILED;
    const mp_int32 socketTimeoutMills = 1000;
    std::vector<mp_string> agentIps;
    CMpString::StrSplit(agentIps, agentIpList, ',');
    for (auto iter = agentIps.begin(); iter != agentIps.end(); ++iter) {
        mp_string srcIp = *iter;
        if (CSocket::CheckHostLinkStatus(srcIp, metadataServerIp,
            atoi(metadataServerPort.c_str()), socketTimeoutMills) == MP_SUCCESS) {
            INFOLOG("Test linking from %s to %s success.", srcIp.c_str(), metadataServerIp.c_str());
            linkMetadataServerStatus = MP_SUCCESS;
            break;
        }
        WARNLOG("Test linking from %s to %s failed.", srcIp.c_str(), metadataServerIp.c_str());
    }
    if (linkMetadataServerStatus == MP_SUCCESS) {
        // 连通性检查成功后，调用接口获取当前所在AZ
        HttpRequest req;
        if (IHttpClient::InitStructHttpRequest(req) != MP_SUCCESS) {
            ERRLOG("Init http request fail.");
            return MP_FAILED;
        }
        req.method = "GET";
        req.url = "http://" + CIP::FormatFullUrl(metadataServerIp) + ":" + metadataServerPort + OPENSTACK_METADATA_URL;
        RegisterHost::SecurityConfiguration(req, "caInfo");
        RegisterHost::SecurityConfiguration(req, "sslCert");
        RegisterHost::SecurityConfiguration(req, "sslKey");
        RegisterHost::SecurityConfiguration(req, "cert");
        return SendMetaDataRequest(req, jv);
    }
    WARNLOG("Test linking to openstack metadata server %s failed.", metadataServerIp.c_str());
    return MP_FAILED;
}

mp_int32 RegisterHost::SendMetaDataRequest(HttpRequest& req, Json::Value& jv)
{
    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == NULL) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when query metadata info.");
        return MP_FAILED;
    }
    mp_int32 iRet = MP_FAILED;
    mp_int32 tryCnt = 0;
    HttpResponse resp = {};
    while (tryCnt++ < MAX_RETRY_TIMES) {
        INFOLOG("Begin send to meta data server times %d.", tryCnt);
        if (RestClientCommon::Send(httpClient, req, resp) == MP_SUCCESS) {
            Json::Value jsonRoot;
            mp_string& rspBody = resp.body;
            if (CJsonUtils::ConvertStringtoJson(rspBody, jsonRoot) != MP_SUCCESS ||
                Json::ValueType::objectValue != jsonRoot.type() ||
                !jsonRoot.isObject()) {
                ERRLOG("Parse response to Json failed.");
                iRet = MP_FAILED;
            } else {
                jv[PARAM_KEY_HOST_AVAILABLE_ZONE] = jsonRoot["availability_zone"];
                jv[PARAM_KEY_HOST_PROJECT_ID] = jsonRoot["project_id"];
                jv[PARAM_KEY_HOST_NATIVE_ID] = jsonRoot["uuid"];
                jv[PARAM_KEY_HOST_VPC_ID] = jsonRoot["meta"]["vpc_id"];
                INFOLOG("Parse response to Json success.");
                iRet = MP_SUCCESS;
                break;
            }
        } else {
            ERRLOG("Send request failed when get metadata info.");
            iRet = MP_FAILED;
        }
    }
    IHttpClient::ReleaseInstance(httpClient);
    return iRet;
}

mp_int32 RegisterHost::GetRegisterAppInfo(std::string& registerAppInfo)
{
    // 获取要求安装的插件应用
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_string strRegisterAppInfo;
    if (GetValueFromConfFile(strFilePath, RUN_CFG_KEY_APPLICATION_INFO, strRegisterAppInfo) != MP_SUCCESS) {
        ERRLOG("Get value of key(%s) from conf file(%s) fail.", RUN_CFG_KEY_APPLICATION_INFO.c_str(),
            strFilePath.c_str());
        return MP_FAILED;
    }

    RegisterApplicationInfo registerApplicationInfo;
    if (!JsonHelper::JsonStringToStruct(strRegisterAppInfo, registerApplicationInfo)) {
        ERRLOG("JsonStringToStruct fail.");
        return MP_FAILED;
    }

    // 取不支持的插件
    std::vector<std::string> unsupportPlugins;
    GetUnSupportPlugins(unsupportPlugins);

    // 去除不支持的插件应用
    std::vector<std::string> unsupportApps;
    for (const std::string& plugin : unsupportPlugins) {
        INFOLOG("Plugin(%s) installation not support.", plugin.c_str());
        // delete plugin
        RegisterApplicationInfo& info = registerApplicationInfo;
        for (auto iter = info.pluginNames.begin(); iter != info.pluginNames.end();) {
            if (*iter == plugin) {
                iter = info.pluginNames.erase(iter);
            } else {
                ++iter;
            }
        }
        // delete app from menu
        DelAppFromMenu(info, plugin, unsupportApps);
    }
    
    // 重新打包消息
    if (!JsonHelper::StructToJsonString(registerApplicationInfo, registerAppInfo)) {
        ERRLOG("JsonStringToStruct fail.");
        return MP_FAILED;
    }

    mp_string filePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_int32 iRet = ModifyLineData(filePath, RUN_CFG_KEY_APPLICATION_INFO, registerAppInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Failed to modify the configuration.");
        return iRet;
    }

    // 将不支持的应用记录
    SetUnSupportApplications(unsupportApps);

    return MP_SUCCESS;
}

mp_void RegisterHost::DelAppFromMenu(RegisterApplicationInfo& info, const std::string& plugin,
    std::vector<std::string>& unsupportApps)
{
    INFOLOG("begin to delete application.");
    for (RegisterMenu& menu : info.registerMenu) {
        for (auto iter = menu.applications.begin(); iter != menu.applications.end();) {
            RegisterApplication& app = *iter;
            if (app.pluginName == plugin) {
                unsupportApps.push_back(app.appValue);
                iter = menu.applications.erase(iter);
            } else {
                ++iter;
            }
        }
    }
}

mp_int32 RegisterHost::GetUnSupportPlugins(std::vector<std::string>& unsupportPlugins)
{
    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(UNSUPPORT_PLUGINS_INFO_FILE);
    mp_int32 iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        WARNLOG("The file %s does not exist.", strFilePath.c_str());
        return MP_FAILED;
    }

    iRet = CMpFile::ReadFile(strFilePath, unsupportPlugins);
    if (iRet != MP_SUCCESS || unsupportPlugins.empty()) {
        ERRLOG("Read file %s failed, size: %d.", strFilePath.c_str(), unsupportPlugins.size());
        return MP_FAILED;
    }
    DBGLOG("Get unsupport plugins from file %s success.", strFilePath.c_str());
    return MP_SUCCESS;
}

mp_int32 RegisterHost::SetUnSupportApplications(std::vector<std::string>& unsupportApps)
{
    if (unsupportApps.size() == 0) {
        WARNLOG("No unsupport apps exist.");
        return MP_SUCCESS;
    }

    mp_string strFilePath = CPath::GetInstance().GetTmpFilePath(UNSUPPORT_APPS_INFO_FILE);
    mp_int32 iRet = CMpFile::FileExist(strFilePath);
    if (iRet == MP_TRUE) {
        WARNLOG("The file %s already exist.", strFilePath.c_str());
        return MP_FAILED;
    }

    std::string strOutput = CMpString::StrJoin(unsupportApps, ",");
    std::vector<std::string> strOutputVec;
    strOutputVec.push_back(strOutput);

    iRet = CIPCFile::WriteFile(strFilePath, strOutputVec);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write file %s failed, str: %d.", strFilePath.c_str(), strOutput.c_str());
        return MP_FAILED;
    }
    DBGLOG("Write unsupport apps %s to file %s success.", strOutput.c_str(), strFilePath.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :  通过key从confile里读取value, 可自定义分隔符delimiter
Area         :  适用于通过分割符设置的配置文件，一行一条，key=value形式
------------------------------------------------------------- */
mp_int32 RegisterHost::GetValueFromConfFile(const std::string& confFile,
    const std::string& key, std::string& value, const std::string& delimiter)
{
    mp_int32 iRet = CMpFile::FileExist(confFile);
    if (iRet != MP_TRUE) {
        ERRLOG("The conf file %s does not exist.", confFile.c_str());
        return MP_FAILED;
    }

    std::vector<std::string> confInfoVec;
    iRet = CMpFile::ReadFile(confFile, confInfoVec);
    if (iRet != MP_SUCCESS || confInfoVec.empty()) {
        ERRLOG("Read file %s failed", confFile.c_str());
        return MP_FAILED;
    }

    for (const std::string& item : confInfoVec) {
        std::string tmpKey;
        std::string tmpValue;
        CMpString::StrSplitToKeyVal(item, tmpKey, tmpValue, delimiter);
        INFOLOG("Read conf file %s key(%s) value(%s).", confFile.c_str(), tmpKey.c_str(), tmpValue.c_str());
        if (tmpKey == key) {
            value = tmpValue;
            return MP_SUCCESS;
        }
    }
    ERRLOG("Key %s does not exists in file %s.", key.c_str(), CFG_RUNNING_PARAM.c_str());
    return MP_FAILED;
}