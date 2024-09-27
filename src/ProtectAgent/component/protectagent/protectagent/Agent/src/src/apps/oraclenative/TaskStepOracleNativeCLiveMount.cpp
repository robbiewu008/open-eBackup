/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeCLiveMount.cpp
 * @brief  Contains function declarations for TaskStepOracleNativeCLiveMount
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/TaskStepOracleNativeCLiveMount.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskContext.h"
#include "securecom/SecureUtils.h"

TaskStepOracleNativeCLiveMount::TaskStepOracleNativeCLiveMount(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeCLiveMount::~TaskStepOracleNativeCLiveMount()
{}

mp_int32 TaskStepOracleNativeCLiveMount::Init(const Json::Value& param)
{
    LOGGUARD("");
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeCLiveMount::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to cancel oracle livemount, taskid %s.", m_taskId.c_str());
    mp_string scriptParams;
    BuildScriptParam(scriptParams);

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_CANCEL_LIVEMOUNT, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, cancel livemount database %s failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_CLIVEMOUNT,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, cancle oracle livemount failed, ret %d.", m_taskId.c_str(), iRet);
        return iRet;
    }
#endif
    COMMLOG(OS_LOG_INFO, "Cancel oracle livemount succ, taskid %s.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_void TaskStepOracleNativeCLiveMount::BuildScriptParam(mp_string& strParam)
{
    std::ostringstream oss;
    oss <<ORACLE_SCRIPTPARAM_INSTNAME <<instName <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBNAME <<dbName <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBUSERNAME <<dbUser <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_DBPASSWORD <<dbPwd <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMINSTANCE <<asmInstance <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMUSERNAME <<asmUser <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ASMPASSWOD <<asmPwd <<NODE_COLON
        <<ORACLE_SCRIPTPARAM_ORACLE_HOME <<NODE_COLON;
    strParam = oss.str();

    ClearString(dbPwd);
    ClearString(asmPwd);
}
