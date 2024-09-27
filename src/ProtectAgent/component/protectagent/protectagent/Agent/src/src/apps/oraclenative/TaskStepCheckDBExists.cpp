#include "apps/oraclenative/TaskStepCheckDBExists.h"

#include "apps/oracle/OracleDefines.h"
#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"

TaskStepCheckDBExists::TaskStepCheckDBExists(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepCheckDBExists::~TaskStepCheckDBExists()
{}

mp_int32 TaskStepCheckDBExists::Init(const Json::Value& param)
{
    LOGGUARD("");
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepCheckDBExists::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to check database %s exists.", dbName.c_str());
    mp_string scriptParams;
    BuildCheckDBScriptParam(scriptParams);

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_CHECKDB_STATUS, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, check database %s exists failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(),
            dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_CHECKDBSTASTUS,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_ORACLE_CHECKDBOPEN_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(
            OS_LOG_ERROR, "check database %s exists failed, ret %d.", dbName.c_str(), iRet);
        return iRet;
    }
#endif

    COMMLOG(OS_LOG_INFO, "taskid %s, check database %s exists succ.", m_taskId.c_str(), dbName.c_str());
    return MP_SUCCESS;
}

mp_void TaskStepCheckDBExists::BuildCheckDBScriptParam(mp_string& strParam)
{
    strParam = ORACLE_SCRIPTPARAM_INSTNAME + instName + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBNAME + dbName + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBUSERNAME + dbUser + NODE_COLON +
        ORACLE_SCRIPTPARAM_DBPASSWORD + dbPwd + NODE_COLON +
        ORACLE_SCIPRTPARAM_CHECKTYPE + "2";
    ClearString(dbPwd);
}

mp_int32 TaskStepCheckDBExists::Stop(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 TaskStepCheckDBExists::Cancel()
{
    return MP_SUCCESS;
}

