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
#include "apps/oraclenative/TaskStepOracleNativeInstRestore.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/UniqueId.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskContext.h"
#include "securecom/SecureUtils.h"

TaskStepOracleNativeInstRestore::TaskStepOracleNativeInstRestore(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order),
    m_irecoverTarget(0)
{}

TaskStepOracleNativeInstRestore::~TaskStepOracleNativeInstRestore()
{
    ClearString(encKey);
}

mp_int32 TaskStepOracleNativeInstRestore::Init(const Json::Value& param)
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
    GET_JSON_UINT64(param[keyDbParams], keyPitTime, pitTime);
    GET_JSON_UINT64(param[keyDbParams], keyPitScn, pitSCN);
    GET_JSON_INT32(param[keyDbParams], keyRecoverNum, recoverNum);
    CHECK_FAIL_EX(CheckParamInteger32(recoverNum, 0, ORACLE_PLUGIN_MAX_INT32));
    GET_JSON_INT32(param[keyDbParams], keyRecoverOrder, recoverOrder);
    CHECK_FAIL_EX(CheckParamInteger32(recoverOrder, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_STRING_OPTION(param[keyDbParams], keyRecoverPath, recoverPath);
    CHECK_FAIL_EX(CheckPathString(recoverPath));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverTarget, m_irecoverTarget);
    CHECK_FAIL_EX(CheckParamInteger32(m_irecoverTarget, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
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

mp_int32 TaskStepOracleNativeInstRestore::Cancel()
{
    mp_string strParam;
    mp_int32 RestoreType = 3;
    BuildStopRmanTaskScriptParam(strParam, RestoreType);
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to cancel taskStep(%s)...", m_taskId.c_str(), m_stepName.c_str());
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_STOPRMANTASK, strParam, NULL);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run stop Insrestore script failed, iRet %d, taskid %s", iRet, m_taskId.c_str());
        return iRet;
    }

    return MP_SUCCESS;
}
mp_int32 TaskStepOracleNativeInstRestore::Run()
{
    static const int OTHER_AGENT_RESTORE = 2;
    COMMLOG(OS_LOG_INFO, "Begin to start oracle with instance restore, taskid %s.", m_taskId.c_str());
    m_stepStatus = STATUS_INPROGRESS;

    mp_int32 iRet = MP_FAILED;
    if (m_irecoverTarget == OTHER_AGENT_RESTORE) {
        if (m_strpfileparam != "") {
            iRet = BuildPfileInfo();
        }
    }
    
    mp_string scriptParams;
    iRet = BuildScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, build instance restore script param, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_INSTRESTORE, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, instance restore database %s failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(),
            iRet,
            iNewRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_INSTRESTORE,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_RESTORE_DATABASE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, Start oracle with instance restore failed, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
#endif

    // parameter don't deleted, move dbf will use it
    COMMLOG(OS_LOG_INFO, "Start oracle with instance restore succ, taskid %s.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeInstRestore::BuildScriptParam(mp_string& strParam)
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
        << ORACLE_SCIPRTPARAM_CHANNEL << channel << NODE_COLON
        << ORACLE_SCIPRTPARAM_PITTIME << pitTime << NODE_COLON
        << ORACLE_SCIPRTPARAM_PITSCN << pitSCN << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERPATH << recoverPath << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERORDER << recoverOrder << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERNUM << recoverNum << NODE_COLON
        << ORACLE_SCIPRTPARAM_STORTYPE << storType << NODE_COLON
        << ORACLE_SCIPRTPARAM_DBTYPE << dbType << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCALGO << encAlgo << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCKEY << encKey << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERTARGET << m_irecoverTarget << NODE_COLON
        << ORACLE_SCIPRTPARAM_PFIILEPID << m_strpfileuuid;
    
    ClearString(dbPwd);
    ClearString(asmPwd);

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    strParam = oss.str();
    return MP_SUCCESS;
}

