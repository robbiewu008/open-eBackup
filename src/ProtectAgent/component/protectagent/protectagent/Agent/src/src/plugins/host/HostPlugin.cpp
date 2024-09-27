/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file HostPlugin.cpp
 * @brief  The implemention host Plugin
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "plugins/host/HostPlugin.h"
#include <sstream>
#include <tuple>
#include <map>
#include "plugins/host/UpgradeHandle.h"
#include "plugins/host/ModifyPluginHandle.h"
#include "common/Log.h"
#include "common/AppVersion.h"
#include "common/ErrorCode.h"
#include "common/Path.h"
#include "common/File.h"
#include "securecom/RootCaller.h"
#include "common/Defines.h"
#include "common/MpString.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "message/rest/interfaces.h"
#include "message/rest/message_process.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/TCPClientHandler.h"
#ifdef FRAME_SIGN
#include "pluginfx/ExternalPluginManager.h"
#endif
#include "alarm/Trap.h"
#include "common/CSystemExec.h"
#include "common/File.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "host/CheckConnectStatus.h"

using namespace std;

REGISTER_PLUGIN(HostPlugin);
namespace {
const mp_string MESSAGE_BODY_IPV4 = "ipv4";
const mp_string MESSAGE_BODY_IPV6 = "ipv6";
const mp_string PARAM_KEY_LOG_LEVEL = "level";
const mp_string PARAM_KEY_LOG_LEVEL_INFO = "info";
const mp_string PARAM_KEY_LOG_LEVEL_ERROR = "error";
const mp_string PARAM_KEY_LOG_INFO_PARAM = "logInfoParam";
const mp_string PARAM_KEY_LOG_UPGRADE_SUCCESS_LABLE = "job_log_agent_storage_update_successful_label";
const mp_string PARAM_KEY_LOG_MODIFY_SUCCESS_LABLE = "job_log_agent_storage_modify_successful_label";
const mp_string PARAM_KEY_LOG_MODIFY_FAIL_LABLE = "job_log_agent_storage_modify_fail_label";
const mp_string PARAM_KEY_LOG_INFO = "logInfo";
const mp_string PARAM_KEY_TIME = "startTime";
const mp_string PARAM_KEY_CURRENT_VERSION = "version";
const mp_int32 BACKUP_ROLE_GENERA = 4;
const mp_int32 BACKUP_ROLE_SANCLIENT = 5;
const mp_string PARAM_KEY_PROTOCOL_TYPE_AUTH = "1";
const mp_int32 ITEM_COLLECT_STATUS = 0;
const mp_int32 ITEM_HTTP_CODE = 1;
const mp_int32 ITEM_RET_CODE = 2;
const mp_int32 ITEM_COLLECT_MSG = 3;
const mp_int32 NUM_1024 = 1024;
const mp_int64 LOG_EXPORT_MIN = 1;
const mp_int64 LOG_EXPORT_MAX = 2147483648;
const mp_string PARAM_SANCLIENT_UUID_SUFFIX = "_sanclient";

/* Key: log collecting status, Value: [status string, http status, return code, log string] */
using LogCollectStatusMapType = std::map<mp_int32, tuple<mp_string, mp_int32, mp_int32, mp_string>>;
const LogCollectStatusMapType LogCollectStatusMap = {
    {LOG_UNKOWN, make_tuple(REST_PARAM_LOG_COLLECT_INIT, SC_OK, MP_SUCCESS, "Log collect is not stared.")},
    {LOG_INIT, make_tuple(REST_PARAM_LOG_COLLECT_INIT, SC_OK, MP_SUCCESS, "Log collect is not stared.")},
    {LOG_COLLECTING, make_tuple(REST_PARAM_LOG_COLLECT_COLLECTING, SC_OK, MP_SUCCESS, "Log is beening collected.")},
    {LOG_COMPLETED, make_tuple(REST_PARAM_LOG_COLLECT_COMPLETED, SC_OK, MP_SUCCESS, "Log collect completed.")},
    {LOG_FAILED, make_tuple(REST_PARAM_LOG_COLLECT_FAILED, SC_INTERNAL_SERVER_ERROR, MP_FAILED, "Log collect failed.")}
    };
}

namespace UPGRADE_STATUS {
    // 升级状态： 0-failure 1-success 2-intermediate 3-disk check failed 8-abnormal 9-initial
    const mp_string UPGRADE_STATUS_FAILURE = "0";
    const mp_string UPGRADE_STATUS_SUCCESS = "1";
    const mp_string UPGRADE_STATUS_INTERMEDIATE = "2";
    const mp_string UPGRADE_STATUS_DISK_CHECK_FAILED = "3";
    const mp_string UPGRADE_STATUS_ABNORMAL = "8";
    const mp_string UPGRADE_STATUS_INITIAL = "9";
} // namespace UPGRADE_STATUS

namespace MODIFY_STATUS {
    // 升级状态： 0-failure 1-success 2-intermediate 3-disk check failed 8-abnormal 9-initial
    const mp_string MODIFYPLUGIN_STATUS_FAILURE = "0";
    const mp_string MODIFYPLUGIN_STATUS_SUCCESS = "1";
    const mp_string MODIFYPLUGIN_STATUS_INTERMEDIATE = "2";
    const mp_string MODIFYPLUGIN_STATUS_DISK_CHECK_FAILED = "3";
    const mp_string MODIFYPLUGIN_STATUS_ABNORMAL = "8";
    const mp_string MODIFYPLUGIN_STATUS_INITIAL = "9";
} // namespace UPGRADE_STATUS

HostPlugin::HostPlugin()
{
    // host
    REGISTER_ACTION(REST_HOST_QUERY_INITIATOR, REST_URL_METHOD_GET, &HostPlugin::QueryInitiators);
    REGISTER_ACTION(REST_HOST_QUERY_INFO, REST_URL_METHOD_GET, &HostPlugin::QueryHostInfo);
    REGISTER_ACTION(REST_HOST_V1_NOTIFY_MANAGER_SERVER, REST_URL_METHOD_PUT, &HostPlugin::NotifyManagerServer);
    REGISTER_ACTION(REST_HOST_LOG_COLLECT, REST_URL_METHOD_POST, &HostPlugin::CollectAgentLog);
    REGISTER_ACTION(REST_HOST_LOG_COLLECT_STATUS, REST_URL_METHOD_GET, &HostPlugin::CollectAgentLogStauts);
    REGISTER_ACTION(REST_HOST_LOG_EXPORT, REST_URL_METHOD_GET, &HostPlugin::ExportAgentLog);
    REGISTER_ACTION(REST_HOST_LOG_LEVEL, REST_URL_METHOD_PUT, &HostPlugin::UpdateAgentLogLevel);
    REGISTER_ACTION(REST_HOST_LOG_CLEAN, REST_URL_METHOD_POST, &HostPlugin::CleanAgentExportedLog);

#ifdef FRAME_SIGN
    REGISTER_ACTION(REST_HOST_V1_QUERY_INFO, REST_URL_METHOD_GET, &HostPlugin::QueryHostV1Info);
    REGISTER_ACTION(REST_HOST_V1_QUERY_APPLUGINS, REST_URL_METHOD_GET, &HostPlugin::QueryHostV1AppPlugins);
    REGISTER_ACTION(REST_HOST_V1_QUERY_WWPNS, REST_URL_METHOD_GET, &HostPlugin::QueryWwpns);
    REGISTER_ACTION(REST_HOST_V2_QUERY_WWPNS, REST_URL_METHOD_GET, &HostPlugin::QueryWwpnsV2);
    REGISTER_ACTION(REST_HOST_V1_QUERY_IQNS, REST_URL_METHOD_POST, &HostPlugin::QueryIqns);
    REGISTER_ACTION(REST_HOST_V1_SCAN_IQN, REST_URL_METHOD_GET, &HostPlugin::ScanIqns);
    REGISTER_ACTION(REST_HOST_V1_DATATURBO_RESCAN, REST_URL_METHOD_PUT, &HostPlugin::DataturboRescan);
#endif
    InitApi();

#ifdef REST_PUBLISH
    // host
    REGISTER_ACTION(REST_HOST_QUERY_AGENT_VERSION, REST_URL_METHOD_GET, &HostPlugin::QueryAgentVersion);
    REGISTER_ACTION(REST_HOST_SCAN_DISK, REST_URL_METHOD_PUT, &HostPlugin::ScanDisk);
    REGISTER_ACTION(REST_HOST_QUERY_DISKS, REST_URL_METHOD_GET, &HostPlugin::QueryDiskInfo);
    REGISTER_ACTION(REST_HOST_QUERY_TIMEZONE, REST_URL_METHOD_GET, &HostPlugin::QueryTimeZone);
    REGISTER_ACTION(REST_HOST_QUERY_FUSION_STORAGE, REST_URL_METHOD_GET, &HostPlugin::QueryFusionStorageIP);
    // thirdpart script
    REGISTER_ACTION(REST_HOST_THIRDPARTY_QUERY_FILE_INFO, REST_URL_METHOD_GET, &HostPlugin::QueryThirdPartyScripts);
    REGISTER_ACTION(REST_HOST_THIRDPARTY_EXEC_FILE, REST_URL_METHOD_POST, &HostPlugin::ExecThirdPartyScript);
    REGISTER_ACTION(REST_HOST_FREEZE_SCRIPT, REST_URL_METHOD_PUT, &HostPlugin::ExecFreezeScript);
    REGISTER_ACTION(REST_HOST_UNFREEZE_SCRIPT, REST_URL_METHOD_PUT, &HostPlugin::ExecThawScript);
    REGISTER_ACTION(REST_HOST_QUERY_STATUS_SCRIPT, REST_URL_METHOD_GET, &HostPlugin::QueryFreezeStatusScript);
    // snmp
    REGISTER_ACTION(REST_HOST_REG_TRAP_SERVER, REST_URL_METHOD_POST, &HostPlugin::RegTrapServer);
    REGISTER_ACTION(REST_HOST_UNREG_TRAP_SERVER, REST_URL_METHOD_POST, &HostPlugin::UnRegTrapServer);
    REGISTER_ACTION(REST_HOST_VERIFY_SNMP, REST_URL_METHOD_PUT, &HostPlugin::VerifySnmp);
#endif
#ifdef WIN32
    REGISTER_ACTION(REST_HOST_ONLINE, REST_URL_METHOD_PUT, &HostPlugin::DeviceOnline);
    REGISTER_ACTION(REST_HOST_BATCH_ONLINE, REST_URL_METHOD_PUT, &HostPlugin::DeviceBatchOnline);
    REGISTER_ACTION(REST_HOST_QUERY_PARTITIONS, REST_URL_METHOD_GET, &HostPlugin::QueryPartisions);
#endif

    // connect to dme business server
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_HOST_GETINITIATOR, &HostPlugin::GetInitiators);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_HOST_GET_IPS, &HostPlugin::GetHostIps);
    REGISTER_DPP_ACTION(MANAGE_CMD_NO_HOST_SCAN_DISK, &HostPlugin::ScanDiskByDpp);

    (mp_void) memset_s(&m_upgradThread, sizeof(m_upgradThread), 0, sizeof(m_upgradThread));
    CallAutoStartFunction();
}

HostPlugin::~HostPlugin()
{
    if (m_upgradThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_upgradThread, NULL);
    }
}

/* ------------------------------------------------------------
Description  : Host组件的统一接口入口，在此分发调用具体的接口
Input        : req -- 输入消息
Output       : rsp -- 返回消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 HostPlugin::DoAction(CRequestMsg& req, CResponseMsg& rsp)
{
    DO_ACTION(HostPlugin, req, rsp);
}

mp_int32 HostPlugin::DoAction(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    DO_DPP_ACTION(HostPlugin, reqMsg, rspMsg);
}

/**
 * 注册命令和函数的对应关系，后面命令分配时，插件管理器查询插件是否支持该命令，找到对应的插件并调用对应的函数
 */
mp_int32 HostPlugin::Init(vector<mp_uint32>& cmds)
{
    // 初始化支持的命令列表
    cmds.push_back(MANAGE_CMD_NO_HOST_GETINITIATOR);
    cmds.push_back(MANAGE_CMD_NO_HOST_REPORT);
    cmds.push_back(MANAGE_CMD_NO_HOST_GET_IPS);
    cmds.push_back(MANAGE_CMD_NO_HOST_SCAN_DISK);
    return MP_SUCCESS;
}

/**
 * 放在此函数内的方法，会随着HostPlugin对象创建(进程重启)而被调用
 */
void HostPlugin::CallAutoStartFunction()
{
    UpdateCertHandle::StartAutoCheck();
}

/* ------------------------------------------------------------
Description  : 查询第三方脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryThirdPartyScripts(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_int32 iRet;
    vector<string> vectFileList;

    COMMLOG(OS_LOG_INFO, "Begin query thirdparty files.");
    iRet = m_host.QueryThirdPartyScripts(vectFileList);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query thirdparty files failed, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& val = rsp.GetJsonValueRef();

    for (vector<mp_string>::iterator iter = vectFileList.begin(); iter != vectFileList.end(); ++iter) {
#ifdef WIN32
#else
        if (iter->length() < HOST_PLUGIN_NUM_3) {
            continue;
        }
        if (strcmp((iter->c_str() + (iter->length() - HOST_PLUGIN_NUM_3)), ".sh") != 0) {
            continue;
        }
#endif
        val.append(iter->c_str());
    }

    COMMLOG(OS_LOG_INFO, "Query thirdparty files succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询块客户端IP
Input        : req -- 请求消息
Output       : rsp -- 返回消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回空值
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryFusionStorageIP(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
#ifndef WIN32
    mp_int32 iRet;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_INFO, "Begin query FusionStorage IP.");
    iRet = m_host.QueryFusionStorageIP(vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query FusionStorage IP failed, iRet %d.", iRet);
        return iRet;
    }

    if (vecResult.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Query FusionStorage IP is empty.");
        return MP_SUCCESS;
    }
    mp_string strFusionStorageIP = vecResult.front();
    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_HOST_FUSION_STORAGE_IP] = strFusionStorageIP;

    COMMLOG(OS_LOG_INFO, "Query FusionStorage IP succ.");
#endif
    return MP_SUCCESS;
}

mp_void HostPlugin::InitApi()
{
    REGISTER_ACTION(REST_HOST_CONNECT_DME, REST_URL_METHOD_POST, &HostPlugin::ConnectDME);
    REGISTER_ACTION(REST_HOST_UPDATE_TRAP_SERVER, REST_URL_METHOD_POST, &HostPlugin::UpdateTrapServer);
    REGISTER_ACTION(REST_HOST_QUERY_AGENT_INFO, REST_URL_METHOD_GET, &HostPlugin::QueryAgentInfo);
    REGISTER_ACTION(REST_HOST_UPGRADE_AGENT, REST_URL_METHOD_POST, &HostPlugin::UpgradeAgent);
    REGISTER_ACTION(REST_HOST_QUERY_UPGRADE_STATUS, REST_URL_METHOD_GET, &HostPlugin::QueryUpgradeStatus);
    REGISTER_ACTION(REST_HOST_UPDATE_VCENTER_INFO, REST_URL_METHOD_GET, &HostPlugin::UpdateLinksInfo);
    REGISTER_ACTION(REST_HOST_CONNECT_DME, REST_URL_METHOD_POST, &HostPlugin::ConnectDME);
    REGISTER_ACTION(REST_HOST_UPDATE_TRAP_SERVER, REST_URL_METHOD_POST, &HostPlugin::UpdateTrapServer);
    REGISTER_ACTION(REST_HOST_QUERY_AGENT_INFO, REST_URL_METHOD_GET, &HostPlugin::QueryAgentInfo);
    REGISTER_ACTION(REST_HOST_UPGRADE_AGENT, REST_URL_METHOD_POST, &HostPlugin::UpgradeAgent);
    REGISTER_ACTION(REST_HOST_QUERY_UPGRADE_STATUS, REST_URL_METHOD_GET, &HostPlugin::QueryUpgradeStatus);
    REGISTER_ACTION(REST_HOST_UPDATE_VCENTER_INFO, REST_URL_METHOD_GET, &HostPlugin::UpdateLinksInfo);
    REGISTER_ACTION(REST_HOST_QUERY_MODIFY_STATUS, REST_URL_METHOD_GET, &HostPlugin::QueryModifyStatus);
    REGISTER_ACTION(REST_HOST_V1_MODIFY_PLUGINS, REST_URL_METHOD_POST, &HostPlugin::ModifyPlugin);

    REGISTER_ACTION(REST_HOST_PUSH_CERT_AGENT, REST_URL_METHOD_POST, &HostPlugin::PushNewCert);
    REGISTER_ACTION(REST_HOST_UPDATE_CERT_AGENT, REST_URL_METHOD_POST, &HostPlugin::RequestUpdateCert);
    REGISTER_ACTION(REST_HOST_CLEAN_CERT_AGENT, REST_URL_METHOD_POST, &HostPlugin::RequestCleanCert);
    REGISTER_ACTION(REST_HOST_ROLLBACK_CERT_AGENT, REST_URL_METHOD_POST, &HostPlugin::RequestRollBackCert);
    REGISTER_ACTION(REST_HOST_CHECK_CONNECT_AGENT, REST_URL_METHOD_POST, &HostPlugin::CheckConnectPmToAgent);
}
/* ------------------------------------------------------------
Description  :ִ 执行三方脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::ExecThirdPartyScript(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;
    mp_string fileName;                // 第三方文件的名称
    mp_string paramValues, tempParas;  // 脚本文件的参数值集合
    mp_string isUserDefined;           // 是否是用户自定义脚本
    vector<string> vecResult;
    COMMLOG(OS_LOG_INFO, "Begin execute thirdpart script.");

    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_ISUSERDEFINED_SCRIPT, isUserDefined);
    GET_JSON_STRING(jv, REST_PARAM_HOST_FILENAME, fileName);
    GET_JSON_KEY_STRING(jv, REST_PARAM_HOST_PARAMS, tempParas);
    paramValues.append(isUserDefined).append(":").append(tempParas);

    // 参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInc;
    mp_int32 lenEnd = 256;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInc, strExclude));
    CHECK_FAIL_EX(CheckCmdDelimiter(paramValues));

    mp_string strFile = CPath::GetInstance().GetThirdPartyFilePath(fileName, isUserDefined);
    CHECK_FAIL_EX(CheckPathString(strFile));

    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Rootcaller thirdparty script exec failed, iRet %d.", iRet);

        Json::Value& jRspValue = rsp.GetJsonValueRef();
        for (vector<string>::iterator it = vecResult.begin(); it != vecResult.end(); ++it) {
            Json::Value jValue;
            jValue["errorMessage"] = *it;
            jRspValue.append(std::move(jValue));
        }
    } else {
        COMMLOG(OS_LOG_INFO, "Execute thirdpart script succ.");
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  : 查询Agent版本信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryAgentVersion(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_int32 iRet;
    mp_string strBuildNum;
    mp_string strAgentVersion;
    COMMLOG(OS_LOG_INFO, "Begin query Agent version.");

    iRet = m_host.GetAgentVersion(strAgentVersion, strBuildNum);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query Agent version failed, iRet %d.", ERROR_COMMON_READ_CONFIG_FAILED);
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_HOST_VERSION] = strAgentVersion;
    jValue[REST_PARAM_HOST_BUILD_NUM] = strBuildNum;

    COMMLOG(OS_LOG_INFO, "End query Agent version.");
    return iRet;
}

EXTER_ATTACK mp_int32 HostPlugin::UpdateLinksInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    (mp_void) rsp;
    COMMLOG(OS_LOG_INFO, "Begin UpdateLinksInfo.");

    if (CheckConnectStatus::GetInstance().UpdateVsphereConnectivity() != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Start check connectivity failed.");
        return ERROR_COMMON_READ_CONFIG_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "End UpdateLinksInfo.");
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 HostPlugin::NotifyManagerServer(CRequestMsg& reqMsg, CResponseMsg& rspMsg)
{
    INFOLOG("Begin NotifyManagerServer.");
    const Json::Value& jsonValue = reqMsg.GetMsgBody().GetJsonValueRef();
    std::vector<mp_string> managerServerVec;
    GET_JSON_ARRAY_STRING(jsonValue, REST_PARAM_MANAGER_SERVER, managerServerVec);

    if (managerServerVec.empty()) {
        ERRLOG("The ip address list is empty..");
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int32 iRet = m_host.UpdateManagerServer(managerServerVec);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "The upgrade status is abnormal.");
        return iRet;
    }

    INFOLOG("End NotifyManagerServer.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :ִ 执行客户定制的三方冻结应用脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : xuchong 00300551
------------------------------------------------------------- */
mp_int32 HostPlugin::ExecFreezeScript(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string fileName, strScriptName;  // 第三方文件的名称
    mp_string paramValues, tempParas;   // 脚本文件的参数值集合
    mp_string isUserDefined;            // 是否是用户自定义脚本
    COMMLOG(OS_LOG_DEBUG, "Begin execute freeze script.");

    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_ISUSERDEFINED_SCRIPT, isUserDefined);
    GET_JSON_STRING(jv, REST_PARAM_HOST_FREEZE_SCRIPT_FILENAME, fileName);
    paramValues.append(isUserDefined).append(":");

    strScriptName = CPath::GetInstance().GetThirdPartyFilePath(fileName, isUserDefined);
    if (!CMpFile::FileExist(strScriptName)) {
        COMMLOG(OS_LOG_INFO, "Freeze file %s is not exists.", fileName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    GET_JSON_KEY_STRING(jv, REST_PARAM_HOST_FREEZE_SCRIPT_PARAM, tempParas);
    paramValues.append(tempParas);

    // check script exists
    GET_JSON_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME, strScriptName);
    strScriptName = CPath::GetInstance().GetThirdPartyFilePath(strScriptName, isUserDefined);
    if (!CMpFile::FileExist(strScriptName)) {
        COMMLOG(OS_LOG_INFO, "Unfreeze file %s is not exists.", strScriptName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    GET_JSON_STRING(jv, REST_PARAM_HOST_QUERY_SCRIPT_FILENAME, strScriptName);
    strScriptName = CPath::GetInstance().GetThirdPartyFilePath(strScriptName, isUserDefined);
    if (!CMpFile::FileExist(strScriptName)) {
        COMMLOG(OS_LOG_INFO, "Query status file %s is not exists.", strScriptName.c_str());
        return ERROR_COMMON_SCRIPT_FILE_NOT_EXIST;
    }

    // 参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 256;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckCmdDelimiter(paramValues));
    mp_string strFilePath = CPath::GetInstance().GetThirdPartyFilePath(fileName, isUserDefined);
    CHECK_FAIL_EX(CheckPathString(strFilePath));

    vector<string> vecResult;
    mp_int32 iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Exec freeze script failed, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "Exec freeze script succ.");
    }
    // 对脚本不存在错误码做特殊处理
    return iRet;
}

/* ------------------------------------------------------------
Description  :ִ 执行客户定制的三方解冻应用脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : xuchong 00300551
------------------------------------------------------------- */
mp_int32 HostPlugin::ExecThawScript(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) rsp;
    mp_int32 iRet;
    mp_string fileName, strFilePath;   // 第三方文件的名称
    mp_string paramValues, tempParas;  // 脚本文件的参数值集合
    mp_string isUserDefined;           // 是否是用户自定义脚本
    vector<string> vecResult;
    COMMLOG(OS_LOG_DEBUG, "Begin execute thaw script.");

    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_FILENAME, fileName);
    GET_JSON_STRING(jv, REST_ISUSERDEFINED_SCRIPT, isUserDefined);
    GET_JSON_KEY_STRING(jv, REST_PARAM_HOST_UNFREEZE_SCRIPT_PARAM, tempParas);
    paramValues.append(isUserDefined).append(":").append(tempParas);

    // 参数校验
    mp_string strExe("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 256;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExe));
    CHECK_FAIL_EX(CheckCmdDelimiter(paramValues));

    strFilePath = CPath::GetInstance().GetThirdPartyFilePath(fileName, isUserDefined);
    CHECK_FAIL_EX(CheckPathString(strFilePath));
    iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "Exec thaw script failed, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "Exec thaw script succ.");
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  : 执行客户定制的查询三方应用冻结状态脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : xuchong 00300551
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryFreezeStatusScript(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string fileName, strFilePath;    // 第三方文件的名称
    mp_string paramValues, queryParam;  // 脚本文件的参数值集合
    mp_string isUserDefined;            // 是否是用户自定义脚本
    vector<string> vecResult;
    COMMLOG(OS_LOG_DEBUG, "Begin execute query script.");

    fileName = req.GetURL().GetSpecialQueryParam(REST_PARAM_HOST_QUERY_SCRIPT_FILENAME);
    queryParam = req.GetURL().GetSpecialQueryParam(REST_PARAM_HOST_QUERY_SCRIPT_PARAM);
    paramValues.append(queryParam);

    // 参数校验
    mp_string strExclude("\\/:*?\"<>|");
    mp_string strInclude;
    mp_int32 lenEnd = 256;
    mp_int32 lenBeg = 1;
    CHECK_FAIL_EX(CheckParamString(fileName, lenBeg, lenEnd, strInclude, strExclude));
    CHECK_FAIL_EX(CheckCmdDelimiter(paramValues));
    strFilePath = CPath::GetInstance().GetThirdPartyFilePath(fileName, isUserDefined);
    CHECK_FAIL_EX(CheckPathString(strFilePath));

    // 第三方脚本状态通过脚本返回码返回
    mp_int32 iRet = m_host.ExecThirdPartyScript(fileName, paramValues, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Exec query script failed, iRet %d.", iRet);
        return iRet;
    } else {
        if (vecResult.empty()) {
            COMMLOG(OS_LOG_ERROR, "The result of get freeze state by scritp failed.");
            return ERROR_COMMON_OPER_FAILED;
        }

        mp_int32 iFreezeState = atoi(vecResult.front().c_str());
        Json::Value& jValue = rsp.GetJsonValueRef();
        jValue[REST_PARAM_HOST_THIRDPARTY_STATE] = iFreezeState;
        COMMLOG(OS_LOG_INFO, "Exec query script succ[%d].", iFreezeState);
        return MP_SUCCESS;
    }
}

/* ------------------------------------------------------------
Description  : 查询主机信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryHostInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_int32 iRet;
    host_info_t hostInfo;

    COMMLOG(OS_LOG_DEBUG, "Begin query host info.");
    iRet = m_host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }
    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_HOST_NAME] = hostInfo.hostName;
    jValue[REST_PARAM_HOST_OS] = hostInfo.nOS;
    jValue[REST_PARAM_HOST_SN] = hostInfo.sn;
    jValue[REST_PARAM_HOST_VERSION] = hostInfo.osVersion;
    jValue[REST_PARAM_HOST_UPGRAGE_TYPE] = PARAM_KEY_PROTOCOL_TYPE_AUTH;
    COMMLOG(OS_LOG_INFO, "Query host info succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询agent的信息(版本号、时间戳版本等)
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryAgentInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;

    COMMLOG(OS_LOG_INFO, "Begin query agent info.");
    mp_int32 iRet;
    agent_info_t agentInfo;
    iRet = m_host.GetAgentInfo(agentInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get agent info failed, iRet %d.", iRet);
        return ERROR_HOST_GETINFO_AGENT_FAILED;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_AGENT_CURRENT_VERSION] = agentInfo.curVersion;
    jValue[REST_PARAM_AGENT_VERSION_TIMESTAMP] = agentInfo.versionTimeStamp;
    COMMLOG(OS_LOG_INFO, "Get agent info successfully, version[%s]", agentInfo.curVersion.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 接收agent推送升级，拉起升级的线程
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::UpgradeAgent(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to receive the upgrade request.");
    
    const Json::Value& jReqBody = req.GetMsgBody().GetJsonValueRef();
    mp_string downloadLink;                      // 下载链接
    mp_string agentId, agentName, certSecretKey;
    vector<mp_string> vecReqInfo;                // 记录json请求字符传递给写文件函数
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK, downloadLink);
    CHECK_FAIL_EX(CheckParamStringEnd(downloadLink, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_AGENTID, agentId);
    CHECK_FAIL_EX(CheckParamStringEnd(agentId, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_AGENTNAME, agentName);
    CHECK_FAIL_EX(CheckParamStringEnd(agentName, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_JOBID, upgradeJobId);
    CHECK_FAIL_EX(CheckParamStringEnd(upgradeJobId, 0, MAX_STRING_LEN));
    GET_JSON_INT32(jReqBody, REST_PARAM_AGENT_UPGRADE_PACKAGESIZE, m_newPackageSize);
    CHECK_FAIL_EX(CheckParamInteger32(m_newPackageSize, 1, MAX_SINGED_INTEGER_VALUE));

    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK + "=" + downloadLink);
    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_AGENTID + "=" + agentId);
    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_AGENTNAME + "=" + agentName);
    // 将请求的报文体内容写到临时文件中，供异步处理的线程调用
    mp_string strUpdateTmpFile = CPath::GetInstance().GetTmpFilePath("tmpUpgradeInfo");
    mp_int32 iRet = CIPCFile::WriteFile(strUpdateTmpFile, vecReqInfo);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Write upgrade info to file failed.");
    }

    // 将更新状态置为中间状态: 0-失败 1-成功 2-中间状态 8-异常状态 9-初始状态
    iRet = UpgradeHandle::UpdateUpgradeStatus(UPGRADE_STATUS::UPGRADE_STATUS_INTERMEDIATE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Set value into testcfg.tmp file failed.");
    }

    // 创建异步处理更新流程的线程
    if (m_upgradThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_upgradThread, NULL);
    }
    (mp_void) memset_s(&m_upgradThread, sizeof(m_upgradThread), 0, sizeof(m_upgradThread));
    iRet = CMpThread::Create(&m_upgradThread, UpgradeHandle::UpgradeAgentHandle, this);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Create thread failed, iRet = %d.", iRet);
    }
    
    Json::Value& jRspBody = rsp.GetJsonValueRef();
    jRspBody[REST_PARAM_AGENT_UPGRADE_REVSTATUS] = MP_TRUE; // 1:成功收到请求

    COMMLOG(OS_LOG_INFO, "Succeed to receive the upgrade request.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 接受推送更新证书，推送证书
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::PushNewCert(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to receive the push new cert request.");
    mp_string errorMsg = "";

    Json::Value& jRspBody = rsp.GetJsonValueRef();
    // 证书校验,校验通过才能返回MP_SUCCESS
    mp_int32 iRet = UpdateCertHandle::GetInstance().PushNewCert(req);
    if (iRet != MP_SUCCESS) {
        errorMsg = "Push new cert failed!";
        COMMLOG(OS_LOG_ERROR, "Push new cert failed!!");
    }
    jRspBody[REST_PARAM_AGENT_ERROR_CODE] = iRet;
    jRspBody[REST_PARAM_AGENT_ERROR_MSG] = errorMsg;

    mp_string msgToReport = jRspBody.toStyledString();
    COMMLOG(OS_LOG_DEBUG, "msgToReport = %s.", msgToReport.c_str());
    COMMLOG(OS_LOG_INFO, "Succeed to receive the push new cert request.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 开始更新证书
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::RequestUpdateCert(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to receive the update cert request.");

    mp_string errorMsg = "";
    Json::Value& jRspBody = rsp.GetJsonValueRef();

    mp_int32 iRet = UpdateCertHandle::GetInstance().HandleUpdateRequest(req);
    if (iRet != MP_SUCCESS) {
        errorMsg = "Handle update cert request failed";
        COMMLOG(OS_LOG_ERROR, "Handle update cert request failed, iRet = %d.", iRet);
    }
    jRspBody[REST_PARAM_AGENT_ERROR_CODE] = iRet;
    jRspBody[REST_PARAM_AGENT_ERROR_MSG] = errorMsg;

    mp_string msgToReport = jRspBody.toStyledString();
    COMMLOG(OS_LOG_DEBUG, "msgToReport = %s.", msgToReport.c_str());
    COMMLOG(OS_LOG_INFO, "Succeed to receive the update cert request.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : PM调用检查agent连通性
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::CheckConnectPmToAgent(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Check Connect Pm To Agent cert request.");
    (mp_void) req;

    mp_string errorMsg = "";
    Json::Value& jRspBody = rsp.GetJsonValueRef();

    mp_int32 iRet = UpdateCertHandle::GetInstance().PmNotifyAgentUseNewCert();
    if (iRet != MP_SUCCESS) {
        errorMsg = "Check Connect Pm To Agent cert request failed!";
        COMMLOG(OS_LOG_ERROR, "Check Connect Pm To Agent cert request failed, iRet = %d.", iRet);
    }
    jRspBody[REST_PARAM_AGENT_ERROR_CODE] = iRet;
    jRspBody[REST_PARAM_AGENT_ERROR_MSG] = errorMsg;

    mp_string msgToReport = jRspBody.toStyledString();
    COMMLOG(OS_LOG_DEBUG, "msgToReport = %s.", msgToReport.c_str());
    COMMLOG(OS_LOG_INFO, "Succeed to receive the pm to agent connection check request.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 清理证书文件
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::RequestCleanCert(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to clean cert files.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string errorMsg = "";
    Json::Value& jRspBody = rsp.GetJsonValueRef();
    iRet = UpdateCertHandle::GetInstance().CleanCertFilesHandle(req);
    if (iRet != MP_SUCCESS) {
        errorMsg = "clean cert files failed!";
        COMMLOG(OS_LOG_ERROR, "clean cert files failed, iRet = %d.", iRet);
    }
    jRspBody[REST_PARAM_AGENT_ERROR_CODE] = iRet;
    jRspBody[REST_PARAM_AGENT_ERROR_MSG] = errorMsg;
    COMMLOG(OS_LOG_INFO, "Succeed to receive clean certs request.");
    return iRet;
}

/* ------------------------------------------------------------
Description  : 证书回退
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::RequestRollBackCert(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to rollback cert files.");
    mp_int32 iRet = MP_SUCCESS;
    mp_string errorMsg = "";
    Json::Value& jRspBody = rsp.GetJsonValueRef();
    iRet = UpdateCertHandle::GetInstance().FallbackCertHandle(req);
    if (iRet != MP_SUCCESS) {
        errorMsg = "Rollback cert files failed!";
        COMMLOG(OS_LOG_ERROR, "Rollback cert files failed, iRet = %d.", iRet);
    }
    jRspBody[REST_PARAM_AGENT_ERROR_CODE] = iRet;
    jRspBody[REST_PARAM_AGENT_ERROR_MSG] = errorMsg;
    COMMLOG(OS_LOG_INFO, "Succeed to receive rollback certs request.");
    return iRet;
}

/* ------------------------------------------------------------
Description  : 修改插件安装
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 HostPlugin::ModifyPlugin(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin to receive the modify plugins request.");

    const Json::Value& jReqBody = req.GetMsgBody().GetJsonValueRef();
    mp_string downloadLink;                      // 下载链接
    mp_string agentId;
    mp_string agentName;
    mp_string certSecretKey;
    vector<mp_string> vecReqInfo;                // 记录json请求字符传递给写文件函数
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK, downloadLink);
    CHECK_FAIL_EX(CheckParamStringEnd(downloadLink, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_AGENTID, agentId);
    CHECK_FAIL_EX(CheckParamStringEnd(agentId, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_AGENTNAME, agentName);
    CHECK_FAIL_EX(CheckParamStringEnd(agentName, 0, MAX_STRING_LEN));
    GET_JSON_STRING(jReqBody, REST_PARAM_AGENT_UPGRADE_JOBID, modifyJobId);
    CHECK_FAIL_EX(CheckParamStringEnd(modifyJobId, 0, MAX_STRING_LEN));
    GET_JSON_INT32(jReqBody, REST_PARAM_AGENT_UPGRADE_PACKAGESIZE, m_newPackageSize);
    CHECK_FAIL_EX(CheckParamInteger32(m_newPackageSize, 1, MAX_SINGED_INTEGER_VALUE));

    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK + "=" + downloadLink);
    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_AGENTID + "=" + agentId);
    vecReqInfo.push_back(REST_PARAM_AGENT_UPGRADE_AGENTNAME + "=" + agentName);
    // 将请求的报文体内容写到临时文件中，供异步处理的线程调用
    mp_string strUpdateTmpFile = CPath::GetInstance().GetTmpFilePath("tmpModifyInfo");
    mp_int32 iRet = CIPCFile::WriteFile(strUpdateTmpFile, vecReqInfo);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Write modify info to file failed.");
    }

    // 将更新状态置为中间状态: 0-失败 1-成功 2-中间状态 8-异常状态 9-初始状态
    iRet = ModifyPluginHandle::UpdateModifyPluginStatus(MODIFY_STATUS::MODIFYPLUGIN_STATUS_INTERMEDIATE);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Set value into testcfg.tmp file failed.");
    }

    // 创建异步处理更新流程的线程
    if (m_upgradThread.os_id != 0) {
        CMpThread::WaitForEnd(&m_upgradThread, NULL);
    }
    (mp_void) memset_s(&m_upgradThread, sizeof(m_upgradThread), 0, sizeof(m_upgradThread));
    iRet = CMpThread::Create(&m_upgradThread, ModifyPluginHandle::ModifyPluginAgentHandle, this);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Create thread failed, iRet = %d.", iRet);
    }
    
    Json::Value& jRspBody = rsp.GetJsonValueRef();
    jRspBody[REST_PARAM_AGENT_UPGRADE_REVSTATUS] = MP_TRUE; // 1:成功收到请求

    INFOLOG("Succeed to receive the modify request.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询agent升级的状态
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryUpgradeStatus(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;

    COMMLOG(OS_LOG_INFO, "Begin query upgrade status.");
    mp_string strUpgradeStatus;
    mp_string strText = "UPGRADE_STATUS=";

    Json::Value& jValue = rsp.GetJsonValueRef();
    mp_int32 iRet = m_host.GetTaskStatus(strText, strUpgradeStatus);
    if ((iRet != MP_SUCCESS) || (strUpgradeStatus.empty())) {
        COMMLOG(OS_LOG_ERROR, "The upgrade status is abnormal.");
        jValue[REST_PARAM_AGENT_UPGRADE_STATUS] = UPGRADE_STATUS::UPGRADE_STATUS_ABNORMAL;
        return MP_FAILED;
    }
    if (strUpgradeStatus != UPGRADE_STATUS::UPGRADE_STATUS_SUCCESS) {
        jValue[PARAM_KEY_LOG_LEVEL] = PARAM_KEY_LOG_LEVEL_ERROR;
        m_host.GetUpgradeErrorDetails(jValue);
    } else {
        jValue[PARAM_KEY_LOG_LEVEL] = PARAM_KEY_LOG_LEVEL_INFO;
        jValue[PARAM_KEY_LOG_INFO] = PARAM_KEY_LOG_UPGRADE_SUCCESS_LABLE;
        agent_info_t agentInfo;
        iRet = m_host.GetAgentInfo(agentInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get agent info failed, iRet %d.", iRet);
            return ERROR_HOST_GETINFO_AGENT_FAILED;
        }
        jValue[PARAM_KEY_LOG_INFO_PARAM].append(agentInfo.curVersion);
        jValue[PARAM_KEY_CURRENT_VERSION] = agentInfo.curVersion;
    }
    Json::Value::UInt64 startTime = CMpTime::GetTimeUsec();
    jValue[PARAM_KEY_TIME] = startTime / SECOND_AND_MILLISECOND_TIMES;
    jValue[REST_PARAM_AGENT_UPGRADE_STATUS] = strUpgradeStatus;
    COMMLOG(OS_LOG_INFO, "Query upgrade status successfully. status[%s]", strUpgradeStatus.c_str());
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询agent修改的状态
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryModifyStatus(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;

    COMMLOG(OS_LOG_INFO, "Begin query Modify status.");
    mp_string strModifyStatus;
    mp_string strText = "MODIFY_STATUS=";

    Json::Value& jValue = rsp.GetJsonValueRef();
    mp_int32 iRet = m_host.GetTaskStatus(strText, strModifyStatus);
    if ((iRet != MP_SUCCESS) || (strModifyStatus.empty())) {
        COMMLOG(OS_LOG_ERROR, "The modify status is abnormal.");
        jValue[REST_PARAM_AGENT_MODFIY_STATUS] = MODIFY_STATUS::MODIFYPLUGIN_STATUS_ABNORMAL;
        return MP_FAILED;
    }
    if (strModifyStatus != MODIFY_STATUS::MODIFYPLUGIN_STATUS_SUCCESS) {
        jValue[PARAM_KEY_LOG_LEVEL] = PARAM_KEY_LOG_LEVEL_ERROR;
        m_host.GetUpgradeErrorDetails(jValue);
    } else {
        jValue[PARAM_KEY_LOG_LEVEL] = PARAM_KEY_LOG_LEVEL_INFO;
        jValue[PARAM_KEY_LOG_INFO] = PARAM_KEY_LOG_MODIFY_SUCCESS_LABLE;
    }
    Json::Value::UInt64 startTime = CMpTime::GetTimeUsec();
    jValue[PARAM_KEY_TIME] = startTime / SECOND_AND_MILLISECOND_TIMES;
    jValue[REST_PARAM_AGENT_MODFIY_STATUS] = strModifyStatus;
    COMMLOG(OS_LOG_INFO, "Query modify status successfully. status[%s]", strModifyStatus.c_str());
    return MP_SUCCESS;
}
#ifdef FRAME_SIGN
/* ------------------------------------------------------------
Description  : 查询主机信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : kWX884906
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryHostV1Info(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    COMMLOG(OS_LOG_INFO, "Begin query host info.");

    host_info_t hostInfo;
    mp_int32 iRet = m_host.GetInfo(hostInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    CHost host;
    Json::Value jIpList;
    iRet = host.GetHostAgentIplist(jIpList);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query host extend information agentiplist failed, iRet %d.", iRet);
        return MP_FAILED;
    }
    Json::Value& jIpValue = jIpList;
    std::string jsonStr = jIpValue.toStyledString();

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_HOST_UUID] = hostInfo.sn;
    jValue[REST_PARAM_HOST_NAME] = hostInfo.hostName;
    jValue[REST_PARAM_HOST_TYPE] = hostInfo.type;
    if (hostInfo.subType == BACKUP_ROLE_GENERA) {
        jValue[REST_PARAM_HOST_SUBTYPE] = "UBackupAgent";
    } else if (hostInfo.subType == BACKUP_ROLE_SANCLIENT) {
        jValue[REST_PARAM_HOST_SUBTYPE] = "SBackupAgent";
        jValue[REST_PARAM_HOST_UUID] = hostInfo.sn + PARAM_SANCLIENT_UUID_SUFFIX;
    } else {
        jValue[REST_PARAM_HOST_SUBTYPE] = hostInfo.subType;
    }
    jValue[REST_PARAM_HOST_ENDPOINT] = hostInfo.endPoint;
    jValue[REST_PARAM_HOST_PORT] = hostInfo.port;
    jValue[REST_PARAM_HOST_USERNAME] = hostInfo.username;
    jValue[REST_PARAM_HOST_PASSWORD] = hostInfo.password;
    jValue[REST_PARAM_HOST_OSTYPE] = hostInfo.strOS;
    jValue[REST_PARAM_HOST_VERSION] = hostInfo.osVersion;
    jValue[REST_PARAM_HOST_EXTENDINFO] = jsonStr;

    COMMLOG(OS_LOG_INFO, "Query host info succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询外部插件列表
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : kWX884906
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryHostV1AppPlugins(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void)req;
    COMMLOG(OS_LOG_INFO, "Begin query AppPlugins.");

    mp_string hostSN;
    mp_int32 iRet = m_host.GetHostSN(hostSN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GetHostSN failed, iRet %d.", iRet);
        return iRet;
    }

    mp_string applications = "";
    iRet = CIP::GetApplications(applications);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query applications  failed, iRet %d.", iRet);
        return MP_FAILED;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue["uuid"] = hostSN;
    auto infos = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginsInfo();
    for (std::map<mp_string, plugin_info>::iterator iter = infos.begin(); iter != infos.end(); ++iter) {
        Json::Value jInfo;
        jInfo["pluginName"] = iter->second.name;
        jInfo["pluginVersion"] = iter->second.version;
        mp_int32 size = 0; // 新增一个变量来记录数组大小
        for (int i = 0; i < iter->second.application.size(); i++) {
            mp_string minVer = iter->second.application_version[iter->second.application[i]].minVer;
            mp_string maxVer = iter->second.application_version[iter->second.application[i]].maxVer;
            if (mp_string::npos != applications.find(iter->second.application[i])) {
                jInfo["supportApplications"][size]["application"] = iter->second.application[i];
                jInfo["supportApplications"][size]["min_version"] = minVer;
                jInfo["supportApplications"][size]["max_version"] = maxVer;
                size++;
            } else {
                continue;
            }
        }
        jValue["supportPlugins"].append(std::move(jInfo));
    }

    COMMLOG(OS_LOG_INFO, "Query host AppPlugins succ.");
    return MP_SUCCESS;
}
#endif
/* ------------------------------------------------------------
Description  : 查询Lun信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryDiskInfo(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    LOGGUARD("");
    mp_int32 iRet;
    vector<host_lun_info_t> vecLunInfo;
    vector<host_lun_info_t>::iterator iter;

    COMMLOG(OS_LOG_INFO, "Begin query host Lun info.");
    iRet = m_host.GetDiskInfo(vecLunInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get host Lun info failed, iRet %d.", iRet);

        return iRet;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    for (iter = vecLunInfo.begin(); iter != vecLunInfo.end(); ++iter) {
        Json::Value jInfo;
        jInfo[REST_PARAM_HOST_LUN_ID] = iter->lunId;
        jInfo[REST_PARAM_HOST_WWN] = iter->wwn;
        jInfo[REST_PARAM_HOST_ARRAY_SN] = iter->arraySn;
        jInfo[REST_PARAM_HOST_ARRAY_VENDOR] = iter->arrayVendor;
        jInfo[REST_PARAM_HOST_ARRAY_MODEL] = iter->arrayModel;
        jInfo[REST_RARAM_HOST_DEVICE_NAME] = iter->deviceName;
        jInfo[REST_PARAM_HOST_DEVICE_DISKNUM] = iter->diskNumber;

        jValue.append(std::move(jInfo));
    }

    COMMLOG(OS_LOG_INFO, "Query host Lun info succ.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询时区信息
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryTimeZone(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    COMMLOG(OS_LOG_INFO, "Begin query host time zone info.");
    mp_int32 iRet;
    timezone_info_t sttimezone;
    iRet = m_host.GetTimeZone(sttimezone);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query host time zone info failed, iRet is %d.", ERROR_HOST_GET_TIMEZONE_FAILED);

        return ERROR_HOST_GET_TIMEZONE_FAILED;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_HOST_TIMEZONE_ISDST] = sttimezone.iIsDST;
    jValue[REST_PARAM_HOST_TIMEZONE_BIAS] = sttimezone.strTzBias;

    COMMLOG(OS_LOG_INFO, "Query host time zone info succ.");

    return iRet;
}

/* ------------------------------------------------------------
Description  : 查询启动器
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryInitiators(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_int32 iRet;
    vector<mp_string>::iterator iter;
    initiator_info_t initInfo;

    COMMLOG(OS_LOG_INFO, "Begin query initialtor infos.");
    iRet = m_host.GetInitiators(initInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get initiator info failed, iRet %d.", iRet);

        return iRet;
    }

    Json::Value& val = rsp.GetJsonValueRef();
    for (iter = initInfo.fcs.begin(); iter != initInfo.fcs.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "Append to json, fc wwn %s.", iter->c_str());
        val[REST_PARAM_HOST_INIT_FC].append(*iter);
    }

    for (iter = initInfo.iscsis.begin(); iter != initInfo.iscsis.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "Append to json, iscsi iqn");
        val[REST_PARAM_HOST_INIT_ISCSI].append(*iter);
        ClearString(*iter);
    }

    COMMLOG(OS_LOG_INFO, "Query initialtor infos succs.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机FC卡的所有wwpn
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangfan zwx1151941
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryWwpns(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin QueryWwpns");
    (mp_void) req;

    mp_string hostSN;
    mp_int32 iRet = m_host.GetHostSN(hostSN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return iRet;
    }

    std::vector<mp_string> vecWwpns;
    iRet = m_host.QueryWwpns(vecWwpns);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query wwpns faield, iRet %d.", iRet);
        return iRet;
    }

    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue["uuid"] = hostSN;
    for (auto item : vecWwpns) {
        jValue["wwpns"].append(std::move(item));
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机FC卡的所有wwpn
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryWwpnsV2(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin QueryWwpnsV2");
    (mp_void) req;
    Json::Value& jValue = rsp.GetJsonValueRef();
    Json::Value jValueTmp;

    mp_int32 iRet = m_host.GetHostInfo(jValueTmp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    std::map<mp_string, mp_string> mapWwpns;
    iRet = m_host.QueryWwpnsV2(mapWwpns);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query wwpns faield, iRet %d.", iRet);
        return iRet;
    }

    jValue["uuid"] = jValueTmp["uuid"];
    jValue["type"] = jValueTmp["type"];
    Json::Value wwpnInfo;
    for (auto item : mapWwpns) {
        wwpnInfo["configKey"] = item.first;
        wwpnInfo["configValue"] = item.second;
        jValue["wwpnInfoList"].append(std::move(wwpnInfo));
        jValue["wwpns"].append(std::move(item.first));
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 校验下发的iqns
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 HostPlugin::QueryIqns(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin QueryIqns");
    Json::Value& jValue = rsp.GetJsonValueRef();
    Json::Value jValueTmp;

    mp_int32 iRet = m_host.GetHostInfo(jValueTmp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    std::map<mp_string, mp_string> mapIqns;
    iRet = m_host.QueryIqns(mapIqns);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query iqns faield, iRet %d.", iRet);
        return iRet;
    }

    const Json::Value& jReqBody = req.GetMsgBody().GetJsonValueRef();
    vector<mp_string> reqIqns;
    GET_JSON_ARRAY_STRING(jReqBody, "configValues", reqIqns);
    if (reqIqns.empty()) {
        ERRLOG("There is no iqns in request message.");
        mapIqns.clear();
        return MP_FAILED;
    }

    jValue["uuid"] = jValueTmp["uuid"];
    jValue["type"] = jValueTmp["type"];
    Json::Value iqnValue;
    for (const auto &iter : reqIqns) {
        iqnValue["configKey"] = iter;
        iqnValue["configValue"] = (mapIqns.find(iter) == mapIqns.end() ? HBA_STATUS_OFFLINE : HBA_STATUS_ONLINE);
        jValue["wwpns"].append(std::move(iter));
        jValue["wwpnInfoList"].append(std::move(iqnValue));
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 查询主机ISCSI卡的所有iqn
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : luozhao lwx1154006
------------------------------------------------------------- */
mp_int32 HostPlugin::ScanIqns(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin ScanIqns");
    (mp_void) req;
    Json::Value& jValue = rsp.GetJsonValueRef();
    Json::Value jValueTmp;

    mp_int32 iRet = m_host.GetHostInfo(jValueTmp);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get host info failed, iRet %d.", iRet);
        return iRet;
    }

    std::map<mp_string, mp_string> mapIqns;
    iRet = m_host.QueryIqns(mapIqns);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Query iqns faield, iRet %d.", iRet);
        return iRet;
    }

    jValue["uuid"] = jValueTmp["uuid"];
    jValue["type"] = jValueTmp["type"];
    Json::Value iqnValue;
    for (const auto &iter : mapIqns) {
        iqnValue["configKey"] = iter.first;
        iqnValue["configValue"] = HBA_STATUS_ONLINE;
        jValue["wwpns"].append(iter.first);
        jValue["wwpnInfoList"].append(iqnValue);
    }
    mapIqns.clear();

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 执行Dataturbo的重新扫描脚本
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : zhangfan zwx1151941
------------------------------------------------------------- */
mp_int32 HostPlugin::DataturboRescan(CRequestMsg& req, CResponseMsg& rsp)
{
    INFOLOG("Begin DataturboRescan");
    (mp_void) req;
    (mp_void) rsp;

    mp_int32 iRet = m_host.DataturboRescan();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Scan disk faield, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 扫描磁盘
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 HostPlugin::ScanDisk(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;
    (mp_void) req;
    (mp_void) rsp;

    COMMLOG(OS_LOG_INFO, "Begin scan disk info.");
    iRet = m_host.ScanDisk();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Scan disk faield, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Scan disk succ.");
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 扫描磁盘
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::ScanDiskByDpp(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value reqBodyParams;
    mp_string taskId;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));
    if (reqBodyParams.isObject() && reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no body.");
        Json::Value& reqBody = reqBodyParams[MANAGECMD_KEY_BODY];
        GET_JSON_STRING_OPTION(reqBody, "taskId", taskId);
        CHECK_FAIL_EX(CheckParamStringEnd(taskId, 0, MAX_TASKID_LEN));
    }

    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_HOST_SCAN_DISK_ACK;
    COMMLOG(OS_LOG_INFO, "Begin scan disk by dpp channel info.");
    mp_int32 iRet = m_host.ScanDisk();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Scan disk by dpp channel faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "Scan disk by dpp channel succ.");
    }

    if (!taskId.empty()) {
        Json::Value body;
        body["taskId"] = taskId;
        rspBody[MANAGECMD_KEY_BODY] = std::move(body);
    }

    rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    rspBody[MANAGECMD_KEY_ERRORDETAIL] = "";
    rspMsg.SetMsgBody(rspBody);
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 HostPlugin::GetHostIps(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    Json::Value reqBodyParams;
    CHECK_NOT_OK(reqMsg.GetManageBody(reqBodyParams));

    Json::Value jsBody;
    if (reqBodyParams.isObject() && reqBodyParams.isMember(MANAGECMD_KEY_BODY)) {
        jsBody = reqBodyParams[MANAGECMD_KEY_BODY];
    }

    // ipv4
    std::vector<mp_string> tIpv4Strs;
    GET_JSON_ARRAY_STRING_OPTION(jsBody, MESSAGE_BODY_IPV4, tIpv4Strs);
    for (mp_string ip : tIpv4Strs) {
        if (!ip.empty()) {
            CHECK_FAIL_EX(CheckParamStringIsIP(ip));
        }
    }
    
    // ipv6
    std::vector<mp_string> tIpv6Strs;
    GET_JSON_ARRAY_STRING_OPTION(jsBody, MESSAGE_BODY_IPV6, tIpv6Strs);
    for (mp_string ip : tIpv6Strs) {
        if (!ip.empty()) {
            CHECK_FAIL_EX(CheckParamStringIsIP(ip));
        }
    }

    Json::Value rspBody;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_HOST_GET_IPS_ACK;
    COMMLOG(OS_LOG_INFO, "Begin to query host ip list.");

    std::vector<mp_string> ipv4List;
    std::vector<mp_string> ipv6List;
    if (m_host.GetHostIPList(tIpv4Strs, tIpv6Strs, ipv4List, ipv6List) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get ip list failed.");
        return MP_FAILED;
    }

    Json::Value body;
    vector<mp_string>::iterator iter;
    for (iter = ipv4List.begin(); iter != ipv4List.end(); ++iter) {
        body["ipv4"].append(*iter);
    }
    for (iter = ipv6List.begin(); iter != ipv6List.end(); ++iter) {
        body["ipv6"].append(*iter);
    }

    if (ipv4List.size() == 0 && ipv6List.size() == 0) {
        body[MANAGECMD_KEY_ERRORCODE] = ERROR_DISCONNECT_STORAGE_NETWORK;
        rspBody[MANAGECMD_KEY_ERRORCODE] = ERROR_DISCONNECT_STORAGE_NETWORK;
        body[MANAGECMD_KEY_ERRORDETAIL] = "all ip link failed.";
        rspBody[MANAGECMD_KEY_ERRORDETAIL] = "all ip link failed.";
    } else {
        rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
    }
    rspBody[MANAGECMD_KEY_BODY] = std::move(body);
    rspMsg.SetMsgBody(rspBody);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 更新trap
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::UpdateTrapServer(CRequestMsg& req, CResponseMsg& rsp)
{
    LOGGUARD("");
    (mp_void) rsp;
    std::vector<trap_server> vecTrapServer;
    snmp_v3_param stParam;
    mp_int32 iRet = GenerateTrapInfo(req, vecTrapServer, stParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "GenerateTrapInfo faield, iRet %d.", iRet);
    }
    
    iRet = m_host.UpdateTrapServer(vecTrapServer, stParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "UpdateTrapServer faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "UpdateTrapServer success");
    }
    alarm_param_t alarmParam;
    alarmParam.iAlarmID = "0x64032C0003";  // 测试告警
    iRet = CTrapSenderManager::CreateSender(A8000).SendAlarm(alarmParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "test trap faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "test trap success");
    }
    CTrapSenderManager::CreateSender(A8000).ResumeAlarm(alarmParam);

    return iRet;
}

mp_int32 HostPlugin::GenerateTrapInfo(
    CRequestMsg& req, std::vector<trap_server>& vecTrapServer, snmp_v3_param& stParam)
{
    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    trap_server stTrapServer;
    stTrapServer.strListenIP = req.GetHttpReq().GetHead(LISTEN_ADDR);
    if (stTrapServer.strListenIP.empty()) {
        stTrapServer.strListenIP = UNKNOWN;
    }

    vector<mp_int32> vecExclude;
    mp_string strTmp;
    bool ret = jv.isObject() && jv.isMember("trap_addresses") && jv["trap_addresses"].isArray() &&
        jv.isMember("trap_config") && jv["trap_config"].isObject();
    if (!ret) {
        return ERROR_COMMON_INVALID_PARAM;
    }
    const Json::Value& addrJson = jv["trap_addresses"];
    const Json::Value& confJson = jv["trap_config"];

    GET_JSON_STRING(confJson, "version", strTmp);
    stTrapServer.iVersion = TrSNMPVersion(strTmp);
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iVersion, 1, HOST_PLUGIN_NUM_3, vecExclude));
    if (addrJson.isArray()) {
        vecTrapServer.reserve(addrJson.size());
        for (Json::ArrayIndex i = 0; i < addrJson.size(); i++) {
            GET_JSON_STRING(addrJson[i], "trapIp", stTrapServer.strServerIP);
            CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strServerIP));
            GET_JSON_INT32(addrJson[i], "port", stTrapServer.iPort);
            CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iPort, 0, HOST_PLUGIN_NUM_65535, vecExclude));
            vecTrapServer.emplace_back(stTrapServer);
        }
    }
    CJsonUtils::GetJsonString(confJson, "contextEngineId", stParam.strContextEngineId);
    CHECK_FAIL_EX(CheckParamStringEnd(stParam.strContextEngineId, 0, MAX_STRING_LEN));
    if (stTrapServer.iVersion == SNMP_V3) {
        CJsonUtils::GetJsonString(confJson, "contextName", stParam.strContextName);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strContextName, 0, MAX_STRING_LEN));
        CJsonUtils::GetJsonString(confJson, "securityName", stParam.strSecurityName);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strSecurityName, 0, MAX_STRING_LEN));
        CJsonUtils::GetJsonString(confJson, "authPwd", stParam.strAuthPassword);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strAuthPassword, 0, MAX_STRING_LEN));
        CJsonUtils::GetJsonString(confJson, "encryptPwd", stParam.strPrivPassword);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strPrivPassword, 0, MAX_STRING_LEN));
        CJsonUtils::GetJsonString(confJson, "authProtocol", strTmp);
        stParam.iAuthProtocol = TrAuthProtocol(strTmp);
        CJsonUtils::GetJsonString(confJson, "encryptProtocol", strTmp);
        stParam.iPrivProtocol = TrPrivateProtocol(strTmp);
        stParam.iSecurityLevel = TrSecurityLevel(
            AUTH_PROTOCOL_TYPE(stParam.iAuthProtocol), PRIVATE_PROTOCOL_TYPE(stParam.iPrivProtocol));
    } else if (stTrapServer.iVersion == SNMP_V2C) {
        CJsonUtils::GetJsonString(confJson, "contextNameV2C", stParam.strContextName);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strContextName, 0, MAX_STRING_LEN));
        CJsonUtils::GetJsonString(confJson, "securityNameV2C", stParam.strSecurityName);
        CHECK_FAIL_EX(CheckParamStringEnd(stParam.strSecurityName, 0, MAX_STRING_LEN));
    }
    stParam.iSecurityModel = TrSecurityModel(SNMP_TYPE(stTrapServer.iVersion));
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 注册trap
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 HostPlugin::RegTrapServer(CRequestMsg& req, CResponseMsg& rsp)
{
    LOGGUARD("");
    (mp_void) rsp;
    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    trap_server stTrapServer;

    COMMLOG(OS_LOG_INFO, "Begin register trap server.");
    GET_JSON_STRING(jv, REST_PARAM_HOST_IP, stTrapServer.strServerIP);
    GET_JSON_INT32(jv, REST_PARAM_HOST_PORT, stTrapServer.iPort);
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMPTYPE, stTrapServer.iVersion);
    stTrapServer.strListenIP = req.GetHttpReq().GetHead(LISTEN_ADDR);
    if (stTrapServer.strListenIP.empty()) {
        stTrapServer.strListenIP = UNKNOWN;
    } else {
        CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strListenIP));
    }

    // 参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strServerIP));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iPort, 0, HOST_PLUGIN_NUM_65535, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iVersion, 1, HOST_PLUGIN_NUM_3, vecExclude));

    mp_int32 iRet = m_host.RegTrapServer(stTrapServer);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "RegTrapServer faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "Register trap server succ.");
    }
    return iRet;
}
/* ------------------------------------------------------------
Description  : 解除trap
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 HostPlugin::UnRegTrapServer(CRequestMsg& req, CResponseMsg& rsp)
{
    LOGGUARD("");
    (mp_void) rsp;

    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    trap_server stTrapServer;

    COMMLOG(OS_LOG_INFO, "Begin unregister trap server succ.");
    GET_JSON_STRING(jv, REST_PARAM_HOST_IP, stTrapServer.strServerIP);
    GET_JSON_INT32(jv, REST_PARAM_HOST_PORT, stTrapServer.iPort);
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMPTYPE, stTrapServer.iVersion);

    // 参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamStringIsIP(stTrapServer.strServerIP));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iPort, 0, HOST_PLUGIN_NUM_65535, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stTrapServer.iVersion, 1, HOST_PLUGIN_NUM_3, vecExclude));

    mp_int32 iRet = m_host.UnRegTrapServer(stTrapServer);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "RegTrapServer faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "RegTrapServer success");
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  : 校验SNMP
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 HostPlugin::VerifySnmp(CRequestMsg& req, CResponseMsg& rsp)
{
    snmp_v3_param stParam;
    (mp_void) rsp;

    COMMLOG(OS_LOG_INFO, "Begin verify snmp.");
    // 从消息头中获取数据
    mp_string pHeadStr = req.GetHttpReq().GetHeadNoCheck(SNMP_PROTOCOL_USER);
    stParam.strSecurityName = CMpString::Trim(pHeadStr);

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(SNMP_AUTH_PW);
    stParam.strAuthPassword = CMpString::Trim(pHeadStr);

    pHeadStr = req.GetHttpReq().GetHeadNoCheck(SNMP_ENCRYPT_PW);
    stParam.strPrivPassword = CMpString::Trim(pHeadStr);

    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMP_AUTHTYPE, stParam.iAuthProtocol);
    GET_JSON_INT32(jv, REST_PARAM_HOST_SNMP_ENCRYPTYPE, stParam.iPrivProtocol);

    // 参数校验
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(stParam.iAuthProtocol, 1, HOST_PLUGIN_NUM_5, vecExclude));
    CHECK_FAIL_EX(CheckParamInteger32(stParam.iPrivProtocol, 1, HOST_PLUGIN_NUM_4, vecExclude));

    mp_int32 iRet = m_host.VerifySnmp(stParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "VerifySnmp faield, iRet %d.", iRet);
    } else {
        COMMLOG(OS_LOG_INFO, "VerifySnmp success");
    }

    stParam.strAuthPassword.replace(0, stParam.strAuthPassword.length(), "");
    stParam.strPrivPassword.replace(0, stParam.strPrivPassword.length(), "");
    return iRet;
}

/* ------------------------------------------------------------
Description  : ͨ通知连接DME business server
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : 无
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::ConnectDME(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin to connect DME business server.");
    mp_string dmeIP;
    mp_uint32 dmePort;
    mp_int32 dmeType;
    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_INT32(jv, SNMP_CONNECT_DME_TYPE, dmeType);
    GET_JSON_STRING(jv, SNMP_CONNECT_DME_IP, dmeIP);
    CHECK_FAIL_EX(CheckParamStringIsIP(dmeIP));
    GET_JSON_UINT32(jv, SNMP_CONNECT_DME_PORT, dmePort);
    CHECK_FAIL_EX(CheckParamInteger32(dmePort, 0, HOST_PLUGIN_NUM_65535));

    mp_int32 iRet = CheckEscapeCht(dmeIP);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Connect to dme server failed, ip is illegitimate, iRet %d.", iRet);
        return iRet;
    }

    MESSAGE_ROLE busiRole = static_cast<MESSAGE_ROLE>(dmeType);

    iRet = TCPClientHandler::GetInstance().Connect(dmeIP, dmePort, busiRole);
    mp_string tempip = CheckParamInvalidReplace(dmeIP);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Connect to dme server(%s:%u) failed, iRet %d.", tempip.c_str(), dmePort, iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Connect to dme server(%s:%u) succ.", tempip.c_str(), dmePort);
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 收集Agent&Plugin日志
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::CollectAgentLog(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_int32 iRet = m_host.CollectLog();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CollectLog faield, iRet %d.", iRet);
        rsp.SetHttpStatus(SC_INTERNAL_SERVER_ERROR);
    } else {
        COMMLOG(OS_LOG_INFO, "CollectAgentLog success, log id: %s, format: %s",
            m_host.GetLogId().c_str(), ZIP_SUFFIX.c_str());
        Json::Value& jValue = rsp.GetJsonValueRef();
        jValue[REST_PARAM_HOST_LOG_EXPORT_ID] = m_host.GetLogId();
        jValue[REST_PARAM_HOST_LOG_PACK_FORMAT] = ZIP_SUFFIX;
    }
    return iRet;
}

/* ------------------------------------------------------------
Description  : 收集Agent&Plugin日志状态
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::CollectAgentLogStauts(CRequestMsg& req, CResponseMsg& rsp)
{
    (mp_void) req;
    mp_uint32 collectStatus = m_host.GetCollectLogStatus();
    auto search = LogCollectStatusMap.find(collectStatus);
    if (search == LogCollectStatusMap.end()) {
        COMMLOG(OS_LOG_ERROR, "Unknown log collecting status: %d", collectStatus);
        return MP_FAILED;
    }
    auto colStatus = search->second;
    Json::Value& jValue = rsp.GetJsonValueRef();
    jValue[REST_PARAM_LOG_COLLECT_STATUS] = std::get<ITEM_COLLECT_STATUS>(colStatus);
    rsp.SetHttpStatus(std::get<ITEM_HTTP_CODE>(colStatus));
    COMMLOG(OS_LOG_INFO, std::get<ITEM_COLLECT_MSG>(colStatus));
    return std::get<ITEM_RET_CODE>(colStatus);
}

mp_int32 HostPlugin::CheckExportLogParams(const mp_string &strLogId, const mp_string &strMaxSize, CResponseMsg& rsp)
{
    mp_string strExclude("\\/:*?\"|;&$<>`!\n.");
    mp_string strInclude;
    mp_int32 lenEnd = 254;
    mp_int32 lenBeg = 1;

    mp_int32 iRet = CheckParamString(strLogId, lenBeg, lenEnd, strInclude, strExclude);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call CheckParamString failed, ret %d.", iRet);
        rsp.SetHttpStatus(SC_BAD_REQUEST);
        return ERROR_COMMON_INVALID_PARAM;
    }
    
    mp_string strFilePath = CPath::GetInstance().GetStmpPath()
        + mp_string(PATH_SEPARATOR) + m_host.FormatLogNameFromId(strLogId);
    iRet = CheckPathString(strFilePath);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Call CheckPathString failed, ret %d.", iRet);
        rsp.SetHttpStatus(SC_BAD_REQUEST);
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_int64 maxSize = -1;
    try {
        maxSize = std::stoll(strMaxSize);
        /* trans MiB to Bytes */
        maxSize = maxSize * NUM_1024 * NUM_1024;
    } catch (std::exception& e) {
        rsp.SetHttpStatus(SC_BAD_REQUEST);
        COMMLOG(OS_LOG_ERROR, "Request log size invalid: %s, exception: %s", strMaxSize.c_str(), e.what());
        return ERROR_COMMON_INVALID_PARAM;
    }

    mp_uint32 logSize = 0;
    if (CMpFile::FileSize(strFilePath.c_str(), logSize) != MP_SUCCESS) {
        rsp.SetHttpStatus(SC_INTERNAL_SERVER_ERROR);
        COMMLOG(OS_LOG_ERROR, "Get log file size failed, file: %s", strFilePath.c_str());
        return MP_FAILED;
    }

    if (maxSize < LOG_EXPORT_MIN || maxSize > LOG_EXPORT_MAX || logSize > maxSize) {
        rsp.SetHttpStatus(SC_BAD_REQUEST);
        COMMLOG(OS_LOG_ERROR,
            "Request log size is out of export range [%ld-%ld]. Request size: %ld, log size: %ld, error code: %ld",
            LOG_EXPORT_MIN, LOG_EXPORT_MAX, maxSize, logSize, ERROR_AGENT_EXPORT_LOG_SIZE_INVALID);
        return ERROR_AGENT_EXPORT_LOG_SIZE_INVALID;
    }

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 导出Agent日志
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::ExportAgentLog(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_string strLogId = req.GetURL().GetSpecialQueryParam(REST_PARAM_HOST_LOG_EXPORT_ID);
    mp_string strMaxSize = req.GetURL().GetSpecialQueryParam(REST_PARAM_HOST_LOG_EXPORT_MAXSIZE);
    /* check rest params */
    mp_int32 iRet = CheckExportLogParams(strLogId, strMaxSize, rsp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Check export log rest param failed, invalid parameters.");
        return iRet;
    }

    // 判断日志是否收集完成
    Json::Value& jValue = rsp.GetJsonValueRef();
    mp_uint32 collectStatus = m_host.GetCollectLogStatus();
    if (collectStatus != LOG_COMPLETED) {
        auto search = LogCollectStatusMap.find(collectStatus);
        if (search == LogCollectStatusMap.end()) {
            COMMLOG(OS_LOG_ERROR, "Unknown log collecting status: %d", collectStatus);
            return MP_FAILED;
        }
        auto colStatus = search->second;
        jValue[REST_PARAM_LOG_COLLECT_STATUS] = std::get<ITEM_COLLECT_STATUS>(colStatus);
        rsp.SetHttpStatus(std::get<ITEM_HTTP_CODE>(colStatus));
        COMMLOG(OS_LOG_INFO, std::get<ITEM_COLLECT_MSG>(colStatus));
        return std::get<ITEM_RET_CODE>(colStatus);
    }
    mp_string logFileName = m_host.FormatLogNameFromId(strLogId);
    mp_string logFilePath = CPath::GetInstance().GetStmpFilePath(m_host.FormatLogNameFromId(strLogId));
    jValue[REST_PARAM_ATTACHMENT_NAME] = logFileName;
    jValue[REST_PARAM_ATTACHMENT_PATH] = logFilePath;
    rsp.SetHttpType(CResponseMsg::RSP_ATTACHMENT_TYPE);
    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 HostPlugin::UpdateAgentLogLevel(CRequestMsg& req, CResponseMsg& rsp)
{
    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    mp_int32 level = -1;
    GET_JSON_INT32(jv, PARAM_KEY_LOG_LEVEL, level);
    return m_host.SetLogLevel(level);
}

EXTER_ATTACK mp_int32 HostPlugin::CleanAgentExportedLog(CRequestMsg& req, CResponseMsg& rsp)
{
    const Json::Value& jv = req.GetMsgBody().GetJsonValueRef();
    mp_string collectId;
    GET_JSON_STRING(jv, REST_PARAM_HOST_LOG_EXPORT_ID, collectId);
    return m_host.CleanLogPackage(collectId);
}

/* ------------------------------------------------------------
Description : query initiator info in host
Input       : reqMsg -- request command
            : rspMsg -- response result
Return      : MP_SUCCESS -- success
                NO MP_SUCCESS -- failed,return error code
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::GetInitiators(CDppMessage& reqMsg, CDppMessage& rspMsg)
{
    static const mp_string KEYINITIATORTYPE = "initiatorType";
    static const mp_string KEYINITIATOR = "Initiator";
    static const mp_string KEYINITIATORSINFO = "InitiatorsInfo";

    LOGGUARD("");

    initiator_info_t initInfo;
    Json::Value rspBody;
    Json::Value initiators;
    rspBody[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_HOST_GETINITIATOR_ACK;
    COMMLOG(OS_LOG_INFO, "Begin query initialtor infos from host.");
    mp_int32 iRet = m_host.GetInitiators(initInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get initiator info failed, iRet %d.", iRet);
        rspBody[MANAGECMD_KEY_ERRORCODE] = iRet;
    } else {
        for (vector<mp_string>::iterator iter = initInfo.fcs.begin(); iter != initInfo.fcs.end(); ++iter) {
            Json::Value jsonVal;
            jsonVal[KEYINITIATORTYPE] = 0;
            jsonVal[KEYINITIATOR] = *iter;
            initiators.append(std::move(jsonVal));
        }

        for (vector<mp_string>::iterator iter = initInfo.iscsis.begin(); iter != initInfo.iscsis.end(); ++iter) {
            Json::Value jsonVal;
            jsonVal[KEYINITIATORTYPE] = 1;
            jsonVal[KEYINITIATOR] = *iter;
            initiators.append(std::move(jsonVal));
            ClearString(*iter);
        }
        rspBody[MANAGECMD_KEY_ERRORCODE] = 0;
        COMMLOG(OS_LOG_INFO, "Query initialtor infos from host succ.");
    }
    rspBody[MANAGECMD_KEY_BODY] = std::move(initiators);
    rspMsg.SetMsgBody(rspBody);
    return iRet;
}

#ifdef WIN32
/* ------------------------------------------------------------
Description  : 设备上线
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::DeviceOnline(CRequestMsg& req, CResponseMsg& rsp)
{
    mp_int32 iRet;
    mp_string strDiskNum;
    (mp_void) rsp;

    COMMLOG(OS_LOG_INFO, "Begin online device.");
    const Json::Value& jReqValue = req.GetMsgBody().GetJsonValueRef();
    GET_JSON_STRING(jReqValue, REST_PARAM_HOST_DISK_NUM, strDiskNum);

    // 参数校验
    mp_int32 iDiskNum = atoi(strDiskNum.c_str());
    vector<mp_int32> vecExclude;
    CHECK_FAIL_EX(CheckParamInteger32(iDiskNum, 0, HOST_PLUGIN_NUM_255, vecExclude));

    iRet = m_host.DeviceOnline(strDiskNum);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Online device failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Online device succ.");
    return MP_SUCCESS;
}
/* ------------------------------------------------------------
Description  : 批量设备上线
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::DeviceBatchOnline(CRequestMsg& req, CResponseMsg& rsp)
{
    LOGGUARD("");
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iRetRst = MP_SUCCESS;

    COMMLOG(OS_LOG_INFO, "Begin batch online device.");
    const Json::Value& jReqValue = req.GetMsgBody().GetJsonValueRef();
    Json::Value& jRspValue = rsp.GetJsonValueRef();
    CHECK_JSON_VALUE(jReqValue, REST_PARAM_HOST_DISK_NUMS);
    CHECK_JSON_ARRAY(jReqValue[REST_PARAM_HOST_DISK_NUMS]);
    mp_uint32 uiSize = jReqValue[REST_PARAM_HOST_DISK_NUMS].size();

    vector<mp_int32> vecExclude;
    for (mp_uint32 i = 0; i < uiSize; i++) {
        mp_string strDiskNum;
        if (jReqValue[REST_PARAM_HOST_DISK_NUMS][i].isString()) {
            strDiskNum = jReqValue[REST_PARAM_HOST_DISK_NUMS][i].asString();
        }
        // 参数校验
        mp_int32 iDiskNum = atoi(strDiskNum.c_str());

        CHECK_FAIL_EX(CheckParamInteger32(iDiskNum, 0, HOST_PLUGIN_NUM_255, vecExclude));

        iRet = m_host.DeviceOnline(strDiskNum);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Online device failed, disk num %s, iRet %d.", strDiskNum.c_str(), iRet);
            Json::Value jv;
            jv[REST_PARAM_ERROR_CODE] = iRet;
            jv[REST_PARAM_HOST_DISK_NUM] = strDiskNum;
            jRspValue.append(jv);
            iRetRst = MP_FAILED;
        }
    }

    COMMLOG(OS_LOG_INFO, "End batch online device.");
    return iRetRst;
}

/* ------------------------------------------------------------
Description  : 查询磁盘分区
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
EXTER_ATTACK mp_int32 HostPlugin::QueryPartisions(CRequestMsg& req, CResponseMsg& rsp)
{
    COMMLOG(OS_LOG_INFO, "Begin query  partisions.");
    vector<partitisions_info_t> partisioninfos;
    vector<partitisions_info_t>::iterator iter;
    Json::Value& jRspValue = rsp.GetJsonValueRef();

    COMMLOG(OS_LOG_INFO, "Begin query partisions.");
    mp_int32 iRet = m_host.GetPartisions(partisioninfos);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query  partisions failed, iRet %d.", iRet);

        return iRet;
    }

    for (iter = partisioninfos.begin(); iter != partisioninfos.end(); ++iter) {
        Json::Value jv;
        jv[REST_PARAM_HOST_PARTISIONNAME] = iter->strPartitionName;
        jv[REST_PARAM_HOST_CAPACITY] = iter->lCapacity;
        jv[REST_PARAM_HOST_DISKNAME] = iter->strDiskNumber;
        jv[REST_RARAM_HOST_DEVICE_NAME] = iter->strVolName;
        jv[REST_PARAM_HOST_LBA_ADDR] = iter->strLba;

        jRspValue.append(jv);
    }

    COMMLOG(OS_LOG_INFO, "Query partisions succ.");

    return MP_SUCCESS;
}

#endif
