/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeLiveMount.cpp
 * @brief  Contains function declarations for TaskStepOracleNativeLiveMount
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/TaskStepOracleNativeLiveMount.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/UniqueId.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"

#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskContext.h"

TaskStepOracleNativeLiveMount::TaskStepOracleNativeLiveMount(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeLiveMount::~TaskStepOracleNativeLiveMount()
{
    ClearString(encKey);
}

mp_int32 TaskStepOracleNativeLiveMount::Init(const Json::Value& param)
{
    LOGGUARD("");
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    iRet = InitStorType(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    if (!param.isMember(keyDbParams)) {
        COMMLOG(OS_LOG_ERROR, "param have no backup param key %s.", keyDbParams.c_str());
        return MP_FAILED;
    }
    GET_JSON_INT32(param[keyDbParams], keyChannel, channel);
    CHECK_FAIL_EX(CheckParamInteger32(channel, 0, ORACLE_PLUGIN_MAX_CHANNEL));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyStartDB, startDB);
    CHECK_FAIL_EX(CheckParamInteger32(startDB, 0, 1));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverOrder, recoverOrder);
    CHECK_FAIL_EX(CheckParamInteger32(recoverOrder, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverNum, recoverNum);
    CHECK_FAIL_EX(CheckParamInteger32(recoverNum, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncAlgo, encAlgo);
    CHECK_FAIL_EX(CheckParamStringEnd(encAlgo, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncKey, encKey);
    CHECK_FAIL_EX(CheckParamStringEnd(encKey, 0, ORACLE_PLUGIN_MAX_STRING));
    if (!param[keyDbParams].isMember(keyPfileParams)) {
        COMMLOG(OS_LOG_ERROR, "param have no backup param key %s.", keyPfileParams.c_str());
        return MP_FAILED;
    }
    Json::Value pfileParams = param[keyDbParams][keyPfileParams];
    GET_JSON_STRING_OPTION(pfileParams, keyPfile, m_strpfileparam);
    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeLiveMount::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to start oracle with livemount, taskid %s.", m_taskId.c_str());
    m_stepStatus = STATUS_INPROGRESS;
    mp_int32 iRet = MP_FAILED;

    if (m_strpfileparam != "") {
        iRet = BuildPfileInfo();
    }

    mp_string scriptParams;
    iRet = BuildScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
    COMMLOG(OS_LOG_ERROR, "taskid %s, build restore script param, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_LIVEMOUNT, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, livemount database %s failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(),
            iRet,
            iNewRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_LIVEMOUNT,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, Start oracle with livemount failed, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
#endif
    (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    COMMLOG(OS_LOG_INFO, "Start oracle with livemount succ, taskid %s.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeLiveMount::BuildScriptParam(mp_string& strParam)
{
    std::ostringstream oss;
    oss << ORACLE_SCRIPTPARAM_INSTNAME << instName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBNAME << dbName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUUID << dbUUID << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUSERNAME << dbUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBPASSWORD << dbPwd << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMINSTANCE << asmInstance << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMUSERNAME << asmUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMPASSWOD << asmPwd << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERORDER << recoverOrder <<NODE_COLON
        << ORACLE_SCIPRTPARAM_CHANNEL << channel << NODE_COLON
        << ORACLE_SCIPRTPARAM_STORTYPE << storType << NODE_COLON
        << ORACLE_SCIPRTPARAM_DBTYPE << dbType << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERNUM << recoverNum << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCALGO << encAlgo << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCKEY << encKey << NODE_COLON
        << ORACLE_SCIPRTPARAM_PFIILEPID << m_strpfileuuid << NODE_COLON
        << ORACLE_SCRIPTPARAM_STARTDB << startDB;

    ClearString(dbPwd);
    ClearString(asmPwd);

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    
    strParam = oss.str();
    return MP_SUCCESS;
}

