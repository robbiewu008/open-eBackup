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
#include "apps/oraclenative/TaskStepOracleNativeRestore.h"

#include <sstream>
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/UniqueId.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "securecom/SecureUtils.h"

namespace {
const mp_string KEYHISTORYSPEED = "taskavgspeed";
}
TaskStepOracleNativeRestore::TaskStepOracleNativeRestore(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order),
    m_iRestoreBy(0),
    m_iHistorySpeed(0),
    recoverNum(0)
{}

TaskStepOracleNativeRestore::~TaskStepOracleNativeRestore()
{
    ClearString(encKey);
}

mp_int32 TaskStepOracleNativeRestore::Init(const Json::Value& param)
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
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverOrder, recoverOrder);
    CHECK_FAIL_EX(CheckParamInteger32(recoverOrder, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverTarget, recoverTarget);
    CHECK_FAIL_EX(CheckParamInteger32(recoverTarget, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_STRING_OPTION(param[keyDbParams], keyRecoverPath, recoverPath);
    CHECK_FAIL_EX(CheckPathString(recoverPath));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRecoverNum, recoverNum);
    CHECK_FAIL_EX(CheckParamInteger32(recoverNum, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncAlgo, encAlgo);
    CHECK_FAIL_EX(CheckParamStringEnd(encAlgo, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncKey, encKey);
    CHECK_FAIL_EX(CheckParamStringEnd(encKey, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyRestoreBy, m_iRestoreBy);
    CHECK_FAIL_EX(CheckParamInteger32(m_iRestoreBy, 0, ORACLE_PLUGIN_MAX_INTGENERAL));
    if (!param[keyDbParams].isMember(keyPfileParams)) {
        COMMLOG(OS_LOG_ERROR, "param have no backup param key %s.", keyPfileParams.c_str());
        return MP_FAILED;
    }
     
    Json::Value pfileParams = param[keyDbParams][keyPfileParams];
    GET_JSON_STRING_OPTION(pfileParams, keyPfile, m_strpfileparam);

    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}
mp_int32 TaskStepOracleNativeRestore::Cancel()
{
    mp_string strParam;
    mp_int32 RestoreType = 2;
    BuildStopRmanTaskScriptParam(strParam, RestoreType);
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to cancel taskStep(%s)...", m_taskId.c_str(), m_stepName.c_str());
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_STOPRMANTASK,
        strParam, NULL, UpdateInnerPIDCallBack, this);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run stop restore script failed, iRet %d, taskid %s", iRet, m_taskId.c_str());
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeRestore::Run()
{
    static const int OTHER_AGENT_RESTORE = 2;

    COMMLOG(OS_LOG_INFO, "Begin to restore oracle by rman, taskid %s.", m_taskId.c_str());

    mp_int32 iRet = MP_FAILED;
    if (recoverTarget == OTHER_AGENT_RESTORE) {
        if (m_strpfileparam != "") {
            iRet = BuildPfileInfo();
        }
    }
    mp_string scriptParams;
    iRet = BuildRestoreScriptParam(scriptParams);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s, build restore script param, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "taskid %s, build restore script param is %s.", m_taskId.c_str(), scriptParams.c_str());
#ifdef WIN32
    iRet = SecureCom::SysExecScript(WIN_ORACLE_NATIVE_RESTORE, scriptParams, NULL);
    ClearString(scriptParams);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "taskid %s, restore database %s failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(), iRet, iNewRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVERESTORE,
        scriptParams, NULL, UpdateInnerPIDCallBack, this);
    TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_RESTORE_DATABASE_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "taskid %s restore failed, ret %d.", m_taskId.c_str(), iRet);
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
#endif
    Json::Value backupRst;
    backupRst[KEYHISTORYSPEED] = m_iHistorySpeed;
    Task* pTask = TaskManager::GetInstance()->FindTaskEx(m_taskId, "OracleNativeRestoreTask");
    if (pTask != NULL) {
        pTask->SetTaskResult(backupRst);
    }
    TaskContext::GetInstance()->SetJsonValue(m_taskId, KEY_TASK_RESULT, backupRst);
    (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    COMMLOG(OS_LOG_INFO, "Restore oracle by rman succ, taskid %s.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeRestore::BuildRestoreScriptParam(mp_string& strParam)
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
        << ORACLE_SCIPRTPARAM_RECOVERTARGET << recoverTarget << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERPATH << recoverPath << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERORDER << recoverOrder << NODE_COLON
        << ORACLE_SCIPRTPARAM_RECOVERNUM << recoverNum << NODE_COLON
        << ORACLE_SCIPRTPARAM_STORTYPE << storType << NODE_COLON
        << ORACLE_SCIPRTPARAM_DBTYPE << dbType << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCALGO << encAlgo << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCKEY << encKey << NODE_COLON
        << ORACLE_SCIPRTPARAM_PFIILEPID << m_strpfileuuid << NODE_COLON
        << ORACLE_SCIPRTPARAM_RESTOREBY << m_iRestoreBy;
    strParam = oss.str();

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    strParam = oss.str();
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeRestore::RefreshStepInfo()
{
    mp_string strParam;
    std::vector<mp_string> vecResult;
    COMMLOG(OS_LOG_INFO, "Begin query restore task(%s) status and progress.", m_taskId.c_str());
    BuildQueryRmanTaskStatusParam(strParam, "1");
    strParam += mp_string(NODE_COLON) + mp_string(ORACLE_SCIPRTPARAM_TASKTYPE) + mp_string(ORACLE_TASK_RESTORE);

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVE_BACKUPSTATUS, strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR, "taskid %s, get database %s backup status failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(), iRet, iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPSTATUS,
        strParam, &vecResult);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run query rman script failed, iRet %d", iRet);
        return iRet;
    }
#endif

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of query rman status and progress is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = AnalyseQueryRmanStatusScriptRst(vecResult, m_stepSpeed, m_stepProgress);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse restore status failed, iRet %d", iRet);
        return iRet;
    }

    iRet = GetHistorySpeed(vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse restore Historyspeed failed, iRet %d", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG,
        "The analyse result of script is:status(%d), speed(%d), progress(%d).",
        m_stepStatus,
        m_stepSpeed,
        m_stepProgress);
    return MP_SUCCESS;
}


mp_int32 TaskStepOracleNativeRestore::GetHistorySpeed(const std::vector<mp_string>& vecResult)
{
    static const mp_int32 ihistorySpeedidex = 4;
    static const mp_int32 iparaSize = 5;
    for (std::vector<mp_string>::const_iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        std::vector<mp_string> restoreRstInfo;
        CMpString::StrSplit(restoreRstInfo, *iter, CHAR_SEMICOLON);
        if (restoreRstInfo.size() < iparaSize) {
            COMMLOG(OS_LOG_WARN, "restore speed size is not right");
            return ERROR_COMMON_OPER_FAILED;
        }
        mp_int32 historySpeed = ConvertBackupSpeed(restoreRstInfo[ihistorySpeedidex]);
        if (historySpeed != 0) {
            m_iHistorySpeed = historySpeed;
        }
    }
    if (m_stepSpeed == 0 && m_iHistorySpeed != 0) {
        m_stepSpeed = m_iHistorySpeed;
    }
    return MP_SUCCESS;
}
