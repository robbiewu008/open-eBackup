#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oracle/OracleDefines.h"
#include "securecom/SecureUtils.h"

TaskStepOracleNativeExpireCopy::TaskStepOracleNativeExpireCopy(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order)
{}

TaskStepOracleNativeExpireCopy::~TaskStepOracleNativeExpireCopy()
{}

mp_int32 TaskStepOracleNativeExpireCopy::Init(const Json::Value& param)
{
    LOGGUARD("");
    // initial db parameters
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    // initial copy parameters
    static const mp_string keyCopyInfo = "copyInfo";
    if (!param.isMember(keyCopyInfo)) {
        COMMLOG(OS_LOG_ERROR, "param have no copy param key %s.", keyCopyInfo.c_str());
        return MP_FAILED;
    }

    static const mp_string keyMaxScn = "logmaxscn";
    GET_JSON_STRING(param[keyCopyInfo], keyMaxScn, maxScn);
    CHECK_FAIL_EX(CheckParamStringEnd(maxScn, 0, ORACLE_PLUGIN_MAX_STRING));
    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeExpireCopy::Run()
{
    COMMLOG(OS_LOG_INFO, "Begin to expire oracle copy data by rman, taskid %s.", m_taskId.c_str());
    m_stepStatus = STATUS_INPROGRESS;
    mp_string scriptParams;
    mp_int32 iRet = BuildScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, build expire copy task script param, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_EXPIRE_COPY, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, expire database %s copy failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(),
            dbName.c_str(),
            iRet,
            iNewRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_EXPIRE_COPY,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    ClearString(scriptParams);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    if (iRet != MP_SUCCESS) {
        COMMLOG(
            OS_LOG_ERROR, "taskid %s, expire database %s copy failed, ret %d.", m_taskId.c_str(), dbName.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
#endif
    (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    COMMLOG(OS_LOG_INFO, "Restore oracle by rman succ, taskid %s.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeExpireCopy::BuildScriptParam(mp_string& strParam)
{
    static const mp_string keyMaxTime = "MaxTime=";
    static const mp_string keyMinTime = "MinTime=";
    static const mp_string keyMaxScn = "MaxScn=";
    static const mp_string keyMinScn = "MinScn=";

    std::ostringstream oss;
    oss << ORACLE_SCRIPTPARAM_INSTNAME << instName << NODE_COLON << ORACLE_SCRIPTPARAM_DBNAME << dbName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUUID << dbUUID << NODE_COLON << ORACLE_SCRIPTPARAM_DBUSERNAME << dbUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBPASSWORD << dbPwd << NODE_COLON << keyMaxTime << maxTime << NODE_COLON << keyMinTime
        << minTime << NODE_COLON << keyMaxScn << maxScn << NODE_COLON << keyMinScn << minScn;
    ClearString(dbPwd);

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    strParam = oss.str();
    return MP_SUCCESS;
}
