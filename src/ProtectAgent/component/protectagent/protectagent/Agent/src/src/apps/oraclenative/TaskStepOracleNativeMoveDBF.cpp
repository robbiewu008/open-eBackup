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
#include "apps/oraclenative/TaskStepOracleNativeMoveDBF.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskContext.h"
#include "securecom/SecureUtils.h"

TaskStepOracleNativeMoveDBF::TaskStepOracleNativeMoveDBF(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeMoveDBF::~TaskStepOracleNativeMoveDBF()
{}

mp_int32 TaskStepOracleNativeMoveDBF::Init(const Json::Value& param)
{
    LOGGUARD("");
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    if (!param.isMember(keyDbParams)) {
        COMMLOG(OS_LOG_ERROR, "param have no backup param key %s.", keyDbParams.c_str());
        return MP_FAILED;
    }

    GET_JSON_INT32(param[keyDbParams], keyRecoverOrder, recoverOrder);
    CHECK_FAIL_EX(CheckParamInteger32(recoverOrder, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_INT32(param[keyDbParams], keyRecoverNum, recoverNum);
    CHECK_FAIL_EX(CheckParamInteger32(recoverNum, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_STRING_OPTION(param[keyDbParams], keyRecoverPath, recoverPath);

    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMoveDBF::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to moving dbf online, taskid %s.", m_taskId.c_str());
    m_stepStatus = STATUS_INPROGRESS_MOVING_DBF;
    mp_string scriptParams;
    mp_int32 iRet = BuildScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, build moving dbf online script param, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_MOVEDBF, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, move database %s files failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(),
            iRet,
            iNewRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_MOVEDBF,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_RESTORE_DATABASE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, Begin to moving dbf online failed, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
#endif
    (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    COMMLOG(OS_LOG_INFO, "Moving database %s dbf online succ, taskid %s.", dbName.c_str(), m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMoveDBF::BuildScriptParam(mp_string& strParam)
{
    std::ostringstream oss;
    oss <<ORACLE_SCRIPTPARAM_INSTNAME <<instName <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBNAME <<dbName <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBUSERNAME <<dbUser <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBPASSWORD <<dbPwd <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMINSTANCE <<asmInstance <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMUSERNAME <<asmUser <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMPASSWOD <<asmPwd <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ORACLE_HOME <<NODE_COLON
        <<ORACLE_SCIPRTPARAM_RECOVERORDER <<recoverOrder << NODE_COLON
        <<ORACLE_SCIPRTPARAM_RECOVERPATH <<recoverPath << NODE_COLON
        <<ORACLE_SCIPRTPARAM_RECOVERNUM <<recoverNum << NODE_COLON
        <<ORACLE_SCIPRTPARAM_DBTYPE <<dbType;
    
    ClearString(dbPwd);
    ClearString(asmPwd);

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    
    strParam = oss.str();
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeMoveDBF::Stop()
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}
