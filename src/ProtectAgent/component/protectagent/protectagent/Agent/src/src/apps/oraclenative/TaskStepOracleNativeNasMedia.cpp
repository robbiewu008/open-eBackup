/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeNasMedia.cpp
 * @brief  Contains function declarations for TaskStepOracleNativeNasMedia
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/TaskStepOracleNativeNasMedia.h"

#include <vector>
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/DB.h"
#include "common/JsonUtils.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "common/MpString.h"
#include "common/Ip.h"
#include "apps/oracle/OracleDefines.h"
#include "host/host.h"
#include "message/tcp/CDppMessage.h"

using std::vector;
namespace {
const mp_int32 MAX_IP_NUM = 10;
};

TaskStepOracleNativeNasMedia::TaskStepOracleNativeNasMedia(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepPrepareNasMedia(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeNasMedia::~TaskStepOracleNativeNasMedia()
{
    ClearString(m_authUser);
    ClearString(m_authKey);
}

mp_int32 TaskStepOracleNativeNasMedia::Init(const Json::Value& param)
{
    mp_int32 iRet = InitPara(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    // parse db info
    if (!param.isMember(keyDbInfos)) {
        COMMLOG(OS_LOG_ERROR, "Input param has no key:%s", keyDbInfos.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    GET_JSON_STRING(param[keyDbInfos], "dbName", m_dbName);
    CHECK_FAIL_EX(CheckParamStringEnd(m_dbName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    GET_JSON_STRING(param[keyDbInfos], "dbUUID", m_dbUUID);
    CHECK_FAIL_EX(CheckParamStringEnd(m_dbUUID, 0, ORACLE_PLUGIN_MAX_UUID));
    // only data backup and log backup need init share path
    if (param.isMember(keyLogVol)) {
        std::vector<Json::Value> jsonLogVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyLogVol, jsonLogVols);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get log volume(%s) failed.", keyLogVol.c_str());
            return iRet;
        }
        GET_JSON_STRING(jsonLogVols.front(), "mediumID", m_logSharePath);
        CHECK_FAIL_EX(CheckParamStringEnd(m_logSharePath, 0, ORACLE_PLUGIN_MAX_STRING));
    }
    if (param.isMember(keyDataVol)) {
        std::vector<Json::Value> jsonDataVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyDataVol, jsonDataVols);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get data volume(%s) failed.", keyDataVol.c_str());
            return iRet;
        }
        GET_JSON_STRING(jsonDataVols.front(), "mediumID", m_dataSharePath);
        CHECK_FAIL_EX(CheckParamStringEnd(m_dataSharePath, 0, ORACLE_PLUGIN_MAX_STRING));
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::InitPara(const Json::Value& param)
{
    GET_JSON_INT32(param, keyHostRole, m_hostRole);
    CHECK_FAIL_EX(CheckParamInteger32(m_hostRole, 0, ORACLE_PLUGIN_MAX_HOSTROLE));
    GET_JSON_INT32(param, keyTaskType, m_taskType);
    if (param.isMember("isSrcDedup")) {
        GET_JSON_INT32(param, "isSrcDedup", m_isSrcDedup);
    }
    if (param.isMember("isLinkEncry")) {
        GET_JSON_INT32(param, "isLinkEncry", m_isLinkEncry);
    }
    CHECK_FAIL_EX(CheckParamInteger32(m_taskType, 0, ORACLE_PLUGIN_MAX_TASKTYPE));
    if (!param.isMember(keyStorage)) {
        COMMLOG(OS_LOG_ERROR, "Input param has no key:%s.", keyStorage.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    const Json::Value& jsonStor = param[keyStorage];
    if (jsonStor.isArray()) {
        COMMLOG(OS_LOG_ERROR, "%s is array.", keyStorage.c_str());
        return ERROR_COMMON_INVALID_PARAM;
    }
    GET_JSON_STRING(jsonStor, "authUser", m_authUser);
    CHECK_FAIL_EX(CheckParamStringEnd(m_authUser, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING(jsonStor, "authKey", m_authKey);
    CHECK_FAIL_EX(CheckParamStringEnd(m_authKey, 0, ORACLE_PLUGIN_MAX_STRING));

    return GetStorageIP(jsonStor);
}

mp_int32 TaskStepOracleNativeNasMedia::GetStorageIP(const Json::Value& jsonStor)
{
    if (jsonStor.isMember("dataOwnerIps")) {
        GET_JSON_ARRAY_STRING(jsonStor, "dataOwnerIps", m_VecDataOwnerIps);
    }
    if (jsonStor.isMember("dataOtherIps")) {
        GET_JSON_ARRAY_STRING(jsonStor, "dataOtherIps", m_VecDataOtherIps);
    }
    if (jsonStor.isMember("logOwnerIps")) {
        GET_JSON_ARRAY_STRING(jsonStor, "logOwnerIps", m_VecLogOwnerIps);
    }
    if (jsonStor.isMember("logOtherIps")) {
        GET_JSON_ARRAY_STRING(jsonStor, "logOtherIps", m_VecLogOtherIps);
    }
    if (jsonStor.isMember("dataturboIps")) {
        GET_JSON_ARRAY_STRING(jsonStor, "dataturboIps", m_VecDataturboIps);
    }

    // Keep only the first 10 elements
    if (m_VecDataOwnerIps.size() > MAX_IP_NUM) {
        m_VecDataOwnerIps.erase(m_VecDataOwnerIps.begin() + MAX_IP_NUM, m_VecDataOwnerIps.end());
    }
    if (m_VecDataOtherIps.size() > MAX_IP_NUM) {
        m_VecDataOtherIps.erase(m_VecDataOtherIps.begin() + MAX_IP_NUM, m_VecDataOtherIps.end());
    }
    if (m_VecLogOwnerIps.size() > MAX_IP_NUM) {
        m_VecLogOwnerIps.erase(m_VecLogOwnerIps.begin() + MAX_IP_NUM, m_VecLogOwnerIps.end());
    }
    if (m_VecLogOtherIps.size() > MAX_IP_NUM) {
        m_VecLogOtherIps.erase(m_VecLogOtherIps.begin() + MAX_IP_NUM, m_VecLogOtherIps.end());
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::Run()
{
    vector<mp_string> vecRst;
    mp_int32 iRet = MountStorageNasMedia(vecRst);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Mount Storage Nas Media Failed.");
        return MP_FAILED;
    }

    const mp_size resultNum = 3;
    if (vecRst.size() != resultNum) {
        COMMLOG(OS_LOG_ERROR, "Prepare oracle nas medium failed, no correct result, num=%u.", vecRst.size());
        return MP_FAILED;
    }

    iRet = UpdateDatabaseParam(vecRst);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update database param failed.");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

void TaskStepOracleNativeNasMedia::SetLogInfo(
    const mp_string& label, const mp_int32& errorCode, const std::vector<std::string>& errorParams)
{
    mp_string hostIP;
    mp_string strPort;
    if (CIP::GetListenIPAndPort(hostIP, strPort) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get Agent listen IP and port failed.");
        return;
    }
    m_respMsg[PARAM_KEY_LOGLABEL] = label;
    m_respMsg[PARAM_KEY_LOGLABELPARAM].append(hostIP);
    m_respMsg[PARAM_KEY_LOGDETAIL] = CMpString::to_string(errorCode);
    for (size_t i = 0; i < errorParams.size(); i++) {
        m_respMsg[PARAM_KEY_LOGPARAMS].append(errorParams[i]);
    }
}

mp_int32 TaskStepOracleNativeNasMedia::MountStorageNasMedia(std::vector<mp_string> &vecRst)
{
    std::ostringstream ossParam;
    mp_string serviceType = "database";
    COMMLOG(OS_LOG_INFO, "m_isSrcDedup value is %d", m_isSrcDedup);
    mp_int32 iRet;
    if (m_isSrcDedup == 1) {
        DataturboMountParam param;
        CHost host;
        mp_int32 iRet = host.GetHostSN(param.storageName); // 使用HostSN做Dataturbo链接对象的storage_name
        if (iRet != MP_SUCCESS) {
            ERRLOG("GetHostSN failed, iRet %d.", iRet);
            return MP_FAILED;
        }
        ossParam << SERVICE_TYPE << STR_EQUAL << serviceType << NODE_COLON << keyDataShareMountPath << m_dataSharePath
            << NODE_COLON << keyLogShareMountPath << m_logSharePath << NODE_COLON << keyScriptParamDataturboName
            << param.storageName << NODE_COLON << ORACLE_SCRIPTPARAM_DBUUID << m_dbUUID << NODE_COLON;
        param.authUser = m_authUser;
        param.authPwd = m_authKey;
        param.vecDataturboIP = m_VecDataturboIps;
        iRet = MountDataturboMedia(ossParam.str(), vecRst, param);
        if (MP_SUCCESS == iRet) {
            COMMLOG(OS_LOG_INFO, "Mount Backup Storage meida through dataturbo protocol successfull!");
            return MP_SUCCESS;
        }
        std::vector<std::string> errorParams;
        if (iRet == ERR_MOUNT_DATA_TURBO_FILE_SYSTEM) {
            errorParams.push_back(m_logSharePath);
        }
        SetLogInfo(DATATURBO_FAILED_LABEL, iRet, errorParams);
        COMMLOG(OS_LOG_WARN, "Mount Backup Storage meida through dataturbo protocol failed!");
    }
    ossParam.clear();
    ossParam << SERVICE_TYPE << STR_EQUAL << serviceType << NODE_COLON << keyDataShareMountPath << m_dataSharePath
        << NODE_COLON << keyLogShareMountPath << m_logSharePath<< NODE_COLON
        << ORACLE_SCRIPTPARAM_DBNAME << m_dbName << NODE_COLON << ORACLE_SCRIPTPARAM_DBUUID << m_dbUUID << NODE_COLON
        << keyScriptParamDataOwnerIP << CMpString::StrJoin(m_VecDataOwnerIps, NODE_SEMICOLON) << NODE_COLON
        << keyScriptParamDataOtherIP << CMpString::StrJoin(m_VecDataOtherIps, NODE_SEMICOLON) << NODE_COLON
        << keyScriptParamLogOwnerIP << CMpString::StrJoin(m_VecLogOwnerIps, NODE_SEMICOLON) << NODE_COLON
        << keyScriptParamLogOtherIP << CMpString::StrJoin(m_VecLogOtherIps, NODE_SEMICOLON) << NODE_COLON
        << keyScriptParamAuthUser << m_authUser << NODE_COLON << keyScriptParamAuthKey << m_authKey << NODE_COLON
        << keyScriptParamLinkEncry << m_isLinkEncry << NODE_COLON;

    iRet = MountNasMedia(ossParam.str(), vecRst);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Mount Backup Media through NFS protocol successful!");
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::UpdateDatabaseParam(const std::vector<mp_string> &vecRst)
{
    mp_int32 iRet;
    // the nas mount path is need to write into database for backup process
    mp_string paramID = m_dbName + STR_DASH + m_dbUUID + STR_DASH + m_taskId;
    mp_size idx = 0;
    iRet = UpdateParam(paramID, "dataPath", vecRst[idx]);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save data nas path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }

    iRet = UpdateParam(paramID, "metaPath", vecRst[idx]);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save metadata nas path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }

    iRet = UpdateParam(paramID, "logPath", vecRst[++idx]);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save log nas path failed, iRet=%d.", iRet);
        return MP_FAILED;
    }

    iRet = UpdateParam(paramID, "mountIPs", m_taskId + NODE_SEMICOLON + vecRst[++idx]);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "save mountPoint failed, iRet=%d.", iRet);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::Cancel()
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::Stop(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::Update(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeNasMedia::Finish(const Json::Value& param)
{
    return MP_SUCCESS;
}

