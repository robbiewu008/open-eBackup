/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeDismount.cpp
 * @brief  The implemention about TaskStepOracleNativeDismount
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"

#include <vector>
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "device/BackupVolume.h"
#include "apps/oracle/OracleDefines.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
#include "taskmanager/TaskContext.h"
#include "securecom/SecureUtils.h"
using std::vector;

TaskStepOracleNativeDismount::TaskStepOracleNativeDismount(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{
    storType = ORA_STORTYPE_NAS;
    taskType = -1;
}

TaskStepOracleNativeDismount::~TaskStepOracleNativeDismount()
{
    ClearString(authUser);
    ClearString(authKey);
}

mp_int32 TaskStepOracleNativeDismount::Init(const Json::Value& param)
{
    LOGGUARD("");

    // initial db parameters
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // inti clean task type, 0-clean, 1-dismount
    GET_JSON_INT32(param, keyTaskType, taskType);
    if (param.isMember(keyDbParams)) {
        GET_JSON_STRING_OPTION(param[keyDbParams], keyRecoverPath, recoverPath);
        COMMLOG(OS_LOG_INFO, "Get recover path is %s.", recoverPath.c_str());
    }

    // init storage info
    if (!param.isMember(keyStorage)) {
        COMMLOG(OS_LOG_ERROR, "dismount parameter have no key storage.");
        return ERROR_COMMON_INVALID_PARAM;
    }

    Json::Value mediaInfo = param[keyStorage];
    if (!mediaInfo.isMember(keyStorType)) {
        COMMLOG(OS_LOG_WARN, "dpp message have no key storage[storType].");
        storType = ORA_STORTYPE_FC;
    } else {
        GET_JSON_INT32(mediaInfo, keyStorType, storType);
        CheckParamInteger32(storType, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
    }

    GET_JSON_STRING(mediaInfo, "authUser", authUser);
    CHECK_FAIL_EX(CheckParamStringEnd(authUser, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING(mediaInfo, "authKey", authKey);
    CHECK_FAIL_EX(CheckParamStringEnd(authKey, 0, ORACLE_PLUGIN_MAX_STRING));

    return ParseVolumeParameter(param);
}

mp_int32 TaskStepOracleNativeDismount::ParseVolumeParameter(const Json::Value& param)
{
    // initial data volumes
    if (param.isMember(keyDataVol)) {
        std::vector<Json::Value> jsonDataVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyDataVol, jsonDataVols);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Input param has no key:%s.", keyDataVol.c_str());
            return iRet;
        }
        if (jsonDataVols.size() == 0) {
            COMMLOG(OS_LOG_ERROR, "have no data volumes.");
            return ERROR_COMMON_INVALID_PARAM;
        }
        GET_JSON_STRING(jsonDataVols.front(), "mediumID", dataSharePath);
    }

    // initial log volumes
    if (param.isMember(keyLogVol)) {
        std::vector<Json::Value> jsonLogVols;
        mp_int32 iRet = CJsonUtils::GetJsonArrayJson(param, keyLogVol, jsonLogVols);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Input param has no key:%s.", keyLogVol.c_str());
            return iRet;
        }
        if (jsonLogVols.size() == 0) {
            COMMLOG(OS_LOG_ERROR, "have no log volumes.");
            return ERROR_COMMON_INVALID_PARAM;
        }
        GET_JSON_STRING(jsonLogVols.front(), "mediumID", logSharePath);
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeDismount::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to dismount oracle %s medium", dbName.c_str());
    m_stepStatus = STATUS_INPROGRESS;
    mp_string scriptParams;
    mp_int32 iRet = BuildScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "build dismount oracle %s medium script param, ret %d.", dbName.c_str(), iRet);
        return iRet;
    }

#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_DISMOUNT_MEDIUM, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "dismount oracle %s medium failed, ret %d, tranformed return code is %d",
            dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_DISMOUNT_MEDIUM,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_DISMOUNT_BACKUP_MEDIUM_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "dismount oracle %s medium failed, ret %d.", dbName.c_str(), iRet);
        return iRet;
    }
#endif

    COMMLOG(OS_LOG_INFO, "Dismount oracle %s medium succ", dbName.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeDismount::BuildScriptParam(mp_string& strParam)
{
    std::ostringstream oss;
    oss << keyDataShareMountPath << dataSharePath << NODE_COLON <<
        keyLogShareMountPath << logSharePath << NODE_COLON <<
        ORACLE_SCRIPTPARAM_DBNAME << dbName << NODE_COLON <<
        ORACLE_SCRIPTPARAM_DBUUID << dbUUID;

    strParam = oss.str();
    mp_int32 ret = MP_SUCCESS;
    return ret;
}
