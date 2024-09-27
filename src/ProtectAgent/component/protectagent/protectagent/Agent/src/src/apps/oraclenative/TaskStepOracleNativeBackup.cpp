/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeBackup.cpp
 * @brief  The implemention about oracle backup task step
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/TaskStepOracleNativeBackup.h"
#include <sstream>
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "apps/oracle/OracleDefines.h"
#include "securecom/SecureUtils.h"

using namespace std;
namespace {
const mp_string KEYDATABACKUPRST = "databackuprst";
const mp_string KEYLOGBACKUPRST = "logbackuprst";
const mp_string KEYRESETLOGSCHANGE = "resetlogs_change";
const mp_string KEYDBSCN = "dbscn";
const mp_string KEYDBBACKTIME = "dbbacktime";
const mp_string KEYLOGMINSCN = "logminscn";
const mp_string KEYLOGMAXSCN = "logmaxscn";
const mp_string KEYLOGBACKMINTIME = "logbackmintime";
const mp_string KEYLOGBACKMAXTIME = "logbackmaxtime";
const mp_string KEYHISTORYSPEED = "taskavgspeed";
const mp_string KEYBACKUPSIZE = "backupsize";
const mp_string KEYEXTENDPARAM = "extendparam";
const mp_string KEYRESTOREFIEL = "pfile";
const mp_string KEYBACKUPLEVEL = "BackupLevel";
const mp_string KEYFILELIST = "filelist";
const mp_int32 VALUE_CHANNEL_INIT = 4;
}

TaskStepOracleNativeBackup::TaskStepOracleNativeBackup(
    const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    : TaskStepOracleNative(id, taskId, name, ratio, order),
      channel(VALUE_CHANNEL_INIT), level(0), limitSpeed(0), truncateLog(0), storType(0), archiveLogKeepDays(0)
{
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("RUNNING", STATUS_INPROGRESS));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("COMPLETED", STATUS_COMPLETED));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("RUNNING WITH WARNINGS", STATUS_FAILED));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("RUNNING WITH ERRORS", STATUS_FAILED));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("COMPLETED WITH WARNINGS", STATUS_FAILED));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("COMPLETED WITH ERRORS", STATUS_FAILED));
    m_mapRmanTaskStatus.insert(std::pair<mp_string, mp_int32>("FAILED", STATUS_FAILED));

    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYDATABACKUPRST, &TaskStepOracleNativeBackup::DataBackupHandler));
    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYLOGBACKUPRST, &TaskStepOracleNativeBackup::LogBackupHandler));
    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYHISTORYSPEED,
                                                   &TaskStepOracleNativeBackup::HistorySpeedBackupHandler));
    HandlerBackupRsts.insert(std::pair<mp_string, FUNC_ANALYSEBACKINFO>(
        KEYBACKUPSIZE, &TaskStepOracleNativeBackup::BackupSizeBackupHandler));
    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYBACKUPLEVEL,
                                                   &TaskStepOracleNativeBackup::BackupLevelBackupHandler));
    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYRESTOREFIEL,
                                                   &TaskStepOracleNativeBackup::ExtendParamBackupHandler));
    HandlerExtendParam.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYRESTOREFIEL,
                                                   &TaskStepOracleNativeBackup::PfileParamBackupHandler));
    HandlerBackupRsts.insert(
        std::pair<mp_string, FUNC_ANALYSEBACKINFO>(KEYFILELIST, &TaskStepOracleNativeBackup::FilelistHandler));
}

TaskStepOracleNativeBackup::~TaskStepOracleNativeBackup()
{
    ClearString(encKey);
}

mp_int32 TaskStepOracleNativeBackup::Init(const Json::Value& param)
{
    LOGGUARD("");
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to init backup params.", m_taskId.c_str());
    mp_int32 iRet = InitialDBInfo(param);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    if (!param.isMember(keyStorInfo)) {
        COMMLOG(OS_LOG_ERROR, "dpp message have no key storage.");
        return MP_FAILED;
    }
    if (!param[keyStorInfo].isMember(keyStorType)) {
        COMMLOG(OS_LOG_WARN, "dpp message have no key \'storType\'.");
        storType = ORA_STORTYPE_FC;
    } else {
        GET_JSON_INT32(param[keyStorInfo], keyStorType, storType);
        CheckParamInteger32(storType, ORA_STORTYPE_NAS, ORA_STORTYPE_FC);
    }

    mp_int32 bakcupMode;
    iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_BACKUP_MODE, bakcupMode);
    if (iRet != MP_SUCCESS) {
        (mp_void) RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncAlgo, encAlgo);
    CHECK_FAIL_EX(CheckParamStringEnd(encAlgo, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_STRING_OPTION(param[keyDbParams], g_EncKey, encKey);
    CHECK_FAIL_EX(CheckParamStringEnd(encKey, 0, ORACLE_PLUGIN_MAX_STRING));
    GET_JSON_INT32(param[keyDbParams], keyChannel, channel);
    CHECK_FAIL_EX(CheckParamInteger32(channel, 0, ORACLE_PLUGIN_MAX_CHANNEL));
    GET_JSON_INT32_OPTION(param[keyDbParams], keyLimitSpeed, limitSpeed);
    CHECK_FAIL_EX(CheckParamInteger32(limitSpeed, 0, ORACLE_PLUGIN_MAX_INT32));
    if (bakcupMode == 1) {
        GET_JSON_INT32_OPTION(param[keyDbParams], keyLevel, level);
        CHECK_FAIL_EX(CheckParamInteger32(level, 0, ORACLE_PLUGIN_MAX_LEVEL));
        if (param[keyDbParams].isMember(keyArchiveLogKeepDays)) {
            GET_JSON_INT32(param[keyDbParams], keyArchiveLogKeepDays, archiveLogKeepDays);
            CHECK_FAIL_EX(CheckParamInteger32(archiveLogKeepDays, -1, ORACLE_PLUGIN_MAX_INT32));
        }
    } else {
        GET_JSON_INT32(param[keyDbParams], keyTruncateLog, truncateLog);
        CHECK_FAIL_EX(CheckParamInteger32(truncateLog, 0, 1));
    }
    m_stepStatus = STATUS_INITIAL;
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::Run()
{
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to backup oracle by rman...", m_taskId.c_str());

    mp_int32 bakcupMode;
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_BACKUP_MODE, bakcupMode);
    if (iRet != MP_SUCCESS) {
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

    UpdateOracleDbInfo();
    mp_string strParam;
    iRet = BuildRmanBackupScriptParam(strParam);
    if (iRet != MP_SUCCESS) {
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

    std::vector<mp_string> vecResult;
    iRet = RunExecScript(bakcupMode, strParam, vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        (mp_void)RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
        return iRet;
    }

    Json::Value backupRst;
    iRet = AnalyseOracleBackupRst(vecResult, backupRst);
    (mp_void) RemoveParam(dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    
    COMMLOG(OS_LOG_INFO, "task(%s) backup oracle success.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::RunExecScript(
    int bakcupMode, const mp_string& strParam, std::vector<mp_string>& vecResult)
{
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    if (bakcupMode == 1) {
        iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVEBACKUP_DATA, strParam, &vecResult);
    } else {
        iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVEBACKUP_LOG, strParam, &vecResult);
    }
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, backup database %s failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(),
            dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    if (bakcupMode == 1) {
        iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPDATA,
            strParam, &vecResult, UpdateInnerPIDCallBack, this);
        TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    } else {
        iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPLOG,
            strParam, &vecResult, UpdateInnerPIDCallBack, this);
        TRANSFORM_RETURN_CODE(iRet, ERROR_AGENT_INTERNAL_ERROR);
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run rman script failed, iRet %d", iRet);
        return iRet;
    }
#endif
    return iRet;
}

mp_int32 TaskStepOracleNativeBackup::Redo(mp_string& innerPID)
{
    COMMLOG(OS_LOG_INFO, "Redo task(%s)(%s) backup oracle begin.", m_taskId.c_str(), innerPID.c_str());
    // check innerPID
    if (innerPID.empty()) {
        COMMLOG(OS_LOG_ERROR, "innerPID is empty.");
        return MP_FAILED;
    }
    
#ifdef WIN32
    mp_string strParam;
    mp_int32 iRet = BuildRmanBackupScriptParam(strParam);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    std::vector<mp_string> vecResult;
    iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVEBACKUP_DATA, strParam, &vecResult);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, get database %s backup status failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(),
            dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    // wait script finish
    mp_int32 iRet = WaitScriptProcessFinish(innerPID);
    if (iRet != MP_SUCCESS && iRet != ERROR_FILESYSTEM_NO_SPACE) {
        return iRet;
    }

    // readresult
    std::vector<mp_string> vecResult;
    CRootCaller rootCaller;
    iRet = rootCaller.ReadResultFile(ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPDATA, innerPID, &vecResult);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
#endif

    Json::Value backupRst;
    iRet = AnalyseOracleBackupRst(vecResult, backupRst);
    if (iRet != MP_SUCCESS) {
        return ERROR_COMMON_OPER_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Redo task(%s) backup oracle success.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::RefreshStepInfo()
{
    // get speed and progress, need use a script to find the
    mp_string strParam;
    std::vector<mp_string> vecResult;
    COMMLOG(OS_LOG_INFO, "Begin query backup task(%s) speed and progress.", m_taskId.c_str());
    BuildQueryRmanTaskStatusParam(strParam, "0");
#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVE_BACKUPSTATUS, strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        ERRLOG("taskid %s, get database %s backup status failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(), dbName.c_str(), iRet, iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_BACKUPSTATUS, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run query rman script failed, iRet %d", iRet);
        return iRet;
    }
#endif
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of query rman speed and progress is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = AnalyseQueryRmanStatusScriptRst(vecResult, m_stepSpeed, m_stepProgress);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Analyse backup speed failed, iRet %d", iRet);
        return iRet;
    }
    {
        std::vector<mp_string> vecTmp;
        CMpString::StrSplit(vecTmp, vecResult.front(), CHAR_SEMICOLON);
        static const mp_int32 nBackupSizeIndex = 5;
        if (vecTmp.size() < nBackupSizeIndex + 1) {
            COMMLOG(OS_LOG_ERROR, "Analyse backup size failed, vecResult size=%d", vecTmp.size());
        } else {
            m_backupSize = atoi(vecTmp.at(nBackupSizeIndex).c_str());
        }
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::BuildRmanBackupScriptParam(mp_string& strParam)
{
    std::ostringstream oss;
    oss << ORACLE_SCRIPTPARAM_DBNAME << dbName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUUID << dbUUID << NODE_COLON
        << ORACLE_SCRIPTPARAM_INSTNAME << instName << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBUSERNAME << dbUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_DBPASSWORD << dbPwd << NODE_COLON
        << ORACLE_SCRIPTPARAM_ORACLE_HOME << oracleHome << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMUSERNAME << asmUser << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMPASSWOD << asmPwd << NODE_COLON
        << ORACLE_SCRIPTPARAM_ASMINSTANCE << asmInstance << NODE_COLON
        << ORACLE_SCIPRTPARAM_CHANNEL << channel << NODE_COLON
        << ORACLE_SCIPRTPARAM_BACKUPQOS << limitSpeed << NODE_COLON
        << ORACLE_SCIPRTPARAM_BACKUPLEVEL << level << NODE_COLON
        << ORACLE_SCIPRTPARAM_TRUNCATE_LOG << truncateLog << NODE_COLON
        << ORACLE_SCIPRTPARAM_STORTYPE << storType << NODE_COLON
        << ORACLE_SCIPRTPARAM_DBTYPE << dbType << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCALGO << encAlgo << NODE_COLON
        << ORACLE_SCIPRTPARAM_ENCKEY << encKey << NODE_COLON
        << ORACLE_SCIPRTPARAM_ARCHIVELOG_KEEPDAYS << archiveLogKeepDays;

    mp_int32 iRet = BuildDataLogPath(oss, dbName + STR_DASH + dbUUID + STR_DASH + m_taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    strParam = oss.str();

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::DataBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 numCapacity = 4;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "data backup information have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    mp_int32 i = 1;
    backupRst[KEYDATABACKUPRST][KEYDBSCN] = backupRstInfo[i++];
    backupRst[KEYDATABACKUPRST][KEYDBBACKTIME] = backupRstInfo[i++];
    backupRst[KEYDATABACKUPRST][KEYRESETLOGSCHANGE] = backupRstInfo[i++];
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::LogBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 logInfoNum = 5;
    if ((backupRstInfo.size() - 1) % logInfoNum != 0) {
        COMMLOG(OS_LOG_ERROR, "log scn have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    mp_int32 i = 1;
    while (i < backupRstInfo.size()) {
        Json::Value logItem;
        logItem[KEYLOGMINSCN] = backupRstInfo[i++];
        logItem[KEYLOGBACKMINTIME] = backupRstInfo[i++];
        logItem[KEYLOGMAXSCN] = backupRstInfo[i++];
        logItem[KEYLOGBACKMAXTIME] = backupRstInfo[i++];
        logItem[KEYRESETLOGSCHANGE] = backupRstInfo[i++];
        backupRst[KEYLOGBACKUPRST].append(std::move(logItem));
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::HistorySpeedBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 numCapacity = 2;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "taskavgspeed have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    static const mp_int32 idxhistoryspeed = 1;
    backupRst[KEYHISTORYSPEED] = ConvertBackupSpeed(backupRstInfo[idxhistoryspeed]);
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::BackupSizeBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 numCapacity = 2;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "backupsize have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    backupRst[KEYBACKUPSIZE] = atoi(backupRstInfo.back().c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::PfileParamBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 numCapacity = 2;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "pfile incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    static const mp_int32 idxpfileparam = 1;
    mp_string tempfilename = backupRstInfo[idxpfileparam];
    Json::Value pfileValues;
    std::vector<mp_string> pfileparamItems;
    CIPCFile::ReadResult(tempfilename, pfileparamItems);
    CRootCaller rootCaller;
    rootCaller.RemoveFile(tempfilename);
    for (vector<mp_string>::iterator iter = pfileparamItems.begin(); iter != pfileparamItems.end(); ++iter) {
        std::vector<mp_string> ItemValue;
        CMpString::StrSplit(ItemValue, *iter, '=');
        mp_int32 size = ItemValue.size();
        static const mp_int32 pfileparamsminsize = 2;
        if (size < pfileparamsminsize) {
            COMMLOG(OS_LOG_ERROR, "pfile param split string failed, %s.", iter->c_str());
            return ERROR_COMMON_OPER_FAILED;
        }
        std::ostringstream oss;
        mp_string Values;
        for (mp_int32 i = 1; i < size - 1; i++) {
            oss << ItemValue[i] << STR_EQUAL;
        }
        oss << ItemValue[size - 1];
        Values = oss.str();
        pfileValues[ItemValue[0]] = Values;
    }
    
    backupRst[backupRstInfo[0]] = pfileValues.toStyledString();
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::ExtendParamBackupHandler(
    const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst)
{
    static const mp_int32 numCapacity = 2;
    static const mp_int32 keyindex = 0;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "Extendparam have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    if (HandlerExtendParam.find(backupRstInfo[keyindex]) == HandlerExtendParam.end()) {
        COMMLOG(OS_LOG_ERROR, "Extendparam info have %s no hander.", backupRstInfo[keyindex].c_str());
        return ERROR_COMMON_OPER_FAILED;
    }
    FUNC_ANALYSEBACKINFO func = HandlerExtendParam[backupRstInfo[keyindex]];
    Json::Value ItemInfo;
    mp_int32 ret = (this->*func)(backupRstInfo, ItemInfo);
    if (ret != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "handle %s Extendparam info failed, ret %d.", backupRstInfo[keyindex].c_str(), ret);
        return ERROR_COMMON_OPER_FAILED;
    }

    backupRst[KEYEXTENDPARAM].append(std::move(ItemInfo));
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::AnalyseOracleBackupRst(vector<mp_string>& vecResult, Json::Value& backupRst)
{
    Json::Value dataBackupRst;
    Json::Value logBackupRst;
    backupRst[KEYDATABACKUPRST] = std::move(dataBackupRst);
    backupRst[KEYLOGBACKUPRST] = std::move(logBackupRst);
    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of backup info is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        vector<mp_string> backupRstInfo;
        CMpString::StrSplit(backupRstInfo, *iter, CHAR_SEMICOLON);

        if (backupRstInfo.empty()) {
            COMMLOG(OS_LOG_ERROR, "split string failed, %s.", iter->c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        // backupRstInfo[0]是返回结果文件的key，用于区分执行什么内容的解析
        if (HandlerBackupRsts.find(backupRstInfo[0]) == HandlerBackupRsts.end()) {
            COMMLOG(OS_LOG_ERROR, "backup info have %s no hander.", backupRstInfo[0].c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        FUNC_ANALYSEBACKINFO func = HandlerBackupRsts[backupRstInfo[0]];
        if (func == NULL) {
            COMMLOG(OS_LOG_ERROR, "bakcup info %s hander is NULL.", backupRstInfo[0].c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        mp_int32 ret = (this->*func)(backupRstInfo, backupRst);
        if (ret != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "handle %s backup info failed, ret %d.", backupRstInfo[0].c_str(), ret);
            return ERROR_COMMON_OPER_FAILED;
        }
    }

    Task* pTask = TaskManager::GetInstance()->FindTaskEx(m_taskId, "OracleNativeBackupTask");
    if (pTask != NULL) {
        pTask->SetTaskResult(backupRst);
    }
    TaskContext::GetInstance()->SetJsonValue(m_taskId, KEY_TASK_RESULT, backupRst);
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::Cancel()
{
    mp_string strParam;
    mp_int32 bakcupMode = 1;
    TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_BACKUP_MODE, bakcupMode);
    BuildStopRmanTaskScriptParam(strParam, bakcupMode);
    COMMLOG(OS_LOG_INFO, "Task(%s) begin to cancel taskStep(%s)...", m_taskId.c_str(), m_stepName.c_str());
#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_ORACLENATIVE_BACKUPSTATUS, strParam, NULL);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "taskid %s, stop %s backup task failed, ret %d, tranformed return code is %d",
            m_taskId.c_str(),
            dbName.c_str(),
            iRet,
            iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_ORACLENATIVE_STOPRMANTASK,
        strParam, NULL, UpdateInnerPIDCallBack, this);
    ClearString(strParam);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run stop backup script failed, iRet %d, taskid %s", iRet, m_taskId.c_str());
        return iRet;
    }
#endif

    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::BackupLevelBackupHandler(const std::vector<mp_string> &backupRstInfo,
    Json::Value &backupRst)
{
    static const mp_int32 numCapacity = 2;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "backuplevel have incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    static const mp_int32 idxbackuplevel = 1;
    backupRst[KEYBACKUPLEVEL] = atoi(backupRstInfo[idxbackuplevel].c_str());
    return MP_SUCCESS;
}

mp_int32 TaskStepOracleNativeBackup::FilelistHandler(const std::vector<mp_string> &backupRstInfo,
    Json::Value &backupRst)
{
    COMMLOG(OS_LOG_INFO, "Start get filelist info .");
    static const mp_int32 numCapacity = 2;
    if (backupRstInfo.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "filelist incorrect number, %d.", backupRstInfo.size());
        return ERROR_COMMON_OPER_FAILED;
    }
    std::vector<mp_string> filelistItems;
    Json::Value filelist;
    static const mp_int32 idxfilelistparam = 1;
    mp_string tempfilename = backupRstInfo[idxfilelistparam];
    CIPCFile::ReadResult(tempfilename, filelistItems);
    CRootCaller rootCaller;
    rootCaller.RemoveFile(tempfilename);
    mp_int32 size = filelistItems.size();
    if (size == 0) {
        COMMLOG(OS_LOG_ERROR, "filelistItems is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }
    for (int i = 0; i < size; ++i) {
        Json::Value file;
        std::vector<mp_string> ItemValue;
        CMpString::StrSplit(ItemValue, filelistItems.at(i), CHAR_SEMICOLON);
        mp_int32 size = ItemValue.size();
        static const mp_int32 filelistparamsminsize = 2;
        if (size < filelistparamsminsize) {
            COMMLOG(OS_LOG_ERROR, "filelist param split string failed, %s.", filelistItems.at(i).c_str());
            return ERROR_COMMON_OPER_FAILED;
        }
        mp_string filepath = ItemValue.at(0);
        mp_int64 filesize = atoll(ItemValue.at(1).c_str());
        file["name"] = filepath;
        file["size"] = static_cast<Json::UInt64>(filesize);
        filelist.append(std::move(file));
    }
    backupRst[KEYFILELIST] = std::move(filelist);
    return MP_SUCCESS;
}

void TaskStepOracleNativeBackup::UpdateOracleDbInfo()
{
    DbParamStream dps;
    DbParam param;

    std::ostringstream removeStream;
    removeStream << "delete from " << g_OracleDbInfo << " where " << g_OracleDBInfo_DbName << "== ?";
    param = dbName;
    dps << std::move(param);
    mp_int32 iRet = CDB::GetInstance().ExecSql(removeStream.str(), dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "remove oracle db info failed, dbName=%s,iRet=%d.", dbName.c_str(), iRet);
        return;
    }

    std::ostringstream insertStream;
    insertStream << "insert into " << g_OracleDbInfo << "(" << g_OracleDBInfo_DbName << ","
        << g_OracleDBInfo_DbInstance << "," << g_OracleDBInfo_DbUser << "," << g_OracleDBInfo_DbPassword << ","
        << g_OracleDBInfo_ASMInstance << "," << g_OracleDBInfo_ASMUser << "," << g_OracleDBInfo_ASMPassword
        << ") values(?,?,?,?,?,?,?);";
    param = dbName;
    dps << std::move(param);
    param = instName;
    dps << std::move(param);
    param = dbUser;
    dps << std::move(param);
    mp_string strEncrypt;
    EncryptStr(dbPwd, strEncrypt);
    param = strEncrypt;
    dps << std::move(param);
    param = asmInstance;
    dps << std::move(param);
    param = asmUser;
    dps << std::move(param);
    EncryptStr(asmPwd, strEncrypt);
    param = strEncrypt;
    dps << std::move(param);
    iRet = CDB::GetInstance().ExecSql(insertStream.str(), dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "insert oracle db info failed, iRet=%d.", iRet);
    }
    return;
}