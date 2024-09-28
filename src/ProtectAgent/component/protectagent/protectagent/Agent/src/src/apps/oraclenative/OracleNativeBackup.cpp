#include "apps/oraclenative/OracleNativeBackup.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "message/tcp/CDppMessage.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "apps/oracle/Oracle.h"
#include "securecom/RootCaller.h"
#include "common/CSystemExec.h"
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "apps/oraclenative/OracleNativeBackupTask.h"
#include "apps/oraclenative/OracleNativeCLiveMTask.h"
#include "apps/oraclenative/OracleNativeLiveMTask.h"
#include "apps/oraclenative/OracleNativeRestoreTask.h"
#include "apps/oraclenative/OracleNativeExpireCopyTask.h"
#include "apps/oraclenative/OracleNativeDismountTask.h"
#include "apps/oraclenative/OracleNativeInstRestoreTask.h"
#include "apps/oraclenative/OracleNativePrepareMediaTask.h"
#include "securecom/SecureUtils.h"

using namespace std;
namespace {
const mp_string KEYDATACAP = "datacap";
const mp_string KEYDATAINFO = "dataFile";
const mp_string KEYLOGCAP = "logcap";
const mp_string KEYLOGINFO = "logFile";
const mp_string KEYDBTYPE = "dbtype";
const mp_string KEYINSLST = "dbInsLst";
const mp_string KEYINSNAME = "InsName";
const mp_string KEYDBTYPEINFO = "dbType";
const mp_string KEYUSEDCAP = "usedCapacity";
const mp_string KEYALLCAP = "totalCapacity";
const mp_string KEYASM = "ASM";
const mp_string KEYASMDGNAME = "dgName";
const mp_string KEYFS = "fileSystem";
const mp_string KEYFSVGNAME = "vgName";
const mp_string KEYFSLVNAME = "lvName";
const mp_string KEYFSLVTYPE = "lvmType";
const mp_string KEYFSMOUNTPATH = "mountPath";
const mp_string KEYFSTYPE = "fileSystemType";
const mp_string KEYFILENAME = "FileName";
const mp_string KEYTABLESPACE = "Tablespace";
const mp_int32 ORACLE_DATA_BACKUP = 1;
const mp_int32 ORACLE_LOG_BACKUP = 2;

const mp_int32 CHECKMOUNTINTERVAL = 30000;
const mp_int32 CHECKMOUNTMAXNUM = 20;
}

OracleNativeBackup::OracleNativeBackup()
{
    HandlerStoreInfos[KEYDATACAP] = &OracleNativeBackup::DataCapHandler;
    HandlerStoreInfos[KEYLOGCAP] = &OracleNativeBackup::LogCapHandler;
    HandlerStoreInfos[KEYDBTYPE] = &OracleNativeBackup::DBTypeHandler;
    HandlerStoreInfos[KEYINSLST] = &OracleNativeBackup::DBInsLstHandler;
    m_bTExitCheckMount = true;
    m_tidCheckMount.os_id = 0;
}

OracleNativeBackup::~OracleNativeBackup()
{
    m_bTExitCheckMount = MP_TRUE;
    if (m_tidCheckMount.os_id != 0) {
        CMpThread::WaitForEnd(&m_tidCheckMount, NULL);
    }
}

/* ------------------------------------------------------------
Description  : get oracle list
Input        : lstOracleInstInfo -- oracle database list to be returned
Return       : MP_SUCCESS -- success
                NO MP_SUCCESS -- failed,return error code
Create By    : youlei 00412658
------------------------------------------------------------- */
mp_int32 OracleNativeBackup::GetDBStorInfo(const oracle_db_info_t& stDBInfo, Json::Value& dbInfo)
{
    mp_string strParam;
    vector<mp_string> vecResult;
    COMMLOG(OS_LOG_INFO, "Begin get oracle storage information.");
    BuildQueryStorageInfoScriptParam(stDBInfo, strParam);

#ifdef WIN32
    mp_int32 iRet = SecureCom::SysExecScript(WIN_ORACLE_STORAGEINFO, strParam, &vecResult);
    ClearString(strParam);
    if (iRet != MP_SUCCESS) {
        mp_int32 iNewRet = ErrorCode::GetInstance().GetErrorCode(iRet);
        ERRLOG("Get oracle %s(%s) storage info failed, iRet %d, tranformed return code is %d",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(),  iRet, iNewRet);
        return iNewRet;
    }
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_STORAGEINFO, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get oracle %s(%s) storage info failed, iRet %d.",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }
#endif

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The result of get oracle storage info is empty.");
        return ERROR_COMMON_OPER_FAILED;
    }

    iRet = AnalyseStorInfoScriptRst(vecResult, dbInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Analyse oracle %s(%s) storage info from script failed, iRet %d.",
            stDBInfo.strInstName.c_str(), stDBInfo.strDBName.c_str(), iRet);
        return iRet;
    }

    dbInfo[PARAM_KEY_INSTNAME] = stDBInfo.strInstName;
    dbInfo[PARAM_KEY_DBNAME] = stDBInfo.strDBName;
    COMMLOG(OS_LOG_INFO, "Get oracle storage info succ");
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::QueryBackupLevel(const Json::Value& reqBody, Json::Value& rspBody)
{
    mp_string taskId;
    GET_JSON_STRING(reqBody, KEY_TASKID, taskId);
    rspBody[MANAGECMD_KEY_TASKID] = taskId;
    COMMLOG(OS_LOG_INFO, "begin QueryBackupLevel, taskid:%s", taskId.c_str());

    static const mp_int32 maxLevel = 2;
    mp_int32 level = 0;
    GET_JSON_INT32(reqBody, "level", level);
    CHECK_FAIL_EX(CheckParamInteger32(level, 0, maxLevel));
    mp_string dbUUID;
    GET_JSON_STRING(reqBody, "dbUUID", dbUUID);
    CHECK_FAIL_EX(CheckParamStringEnd(dbUUID, 0, ORACLE_PLUGIN_MAX_UUID));
    oracle_db_info_t stDBInfo;
    GET_JSON_STRING(reqBody, PARAM_KEY_DBNAME, stDBInfo.strDBName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBName, 0, ORACLE_PLUGIN_MAX_DBNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBName));
    GET_JSON_STRING(reqBody, PARAM_KEY_INSTNAME, stDBInfo.strInstName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strInstName, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strInstName));
    GET_JSON_STRING(reqBody, PARAM_KEY_DBUSER, stDBInfo.strDBUsername);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBUsername, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBUsername));
    GET_JSON_STRING(reqBody, PARAM_KEY_DBPWD, stDBInfo.strDBPassword);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strDBPassword, 0, ORACLE_PLUGIN_MAX_STRING));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strDBPassword));
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMINST, stDBInfo.strASMInstance);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMInstance, 0, ORACLE_PLUGIN_MAX_DBINSTANCENAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMInstance));
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMUSER, stDBInfo.strASMUserName);
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMUserName, 0, ORACLE_PLUGIN_MAX_NSERNAME));
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMUserName));
    GET_JSON_STRING(reqBody, PARAM_KEY_ASMPWD, stDBInfo.strASMPassword);
    CHECK_FAIL_EX(CheckParamValid(stDBInfo.strASMPassword));
    CHECK_FAIL_EX(CheckParamStringEnd(stDBInfo.strASMPassword, 0, ORACLE_PLUGIN_MAX_STRING));

    mp_string dataPath;
    TaskStepOracleNative t_step("", "", "", 0, 0);
    t_step.GetBackupParam(stDBInfo.strDBName + STR_DASH + dbUUID + STR_DASH + taskId, "dataPath", dataPath);

    std::ostringstream oss;
    oss << ORACLE_SCRIPTPARAM_INSTNAME << stDBInfo.strInstName << NODE_COLON <<
        ORACLE_SCRIPTPARAM_DBNAME << stDBInfo.strDBName << NODE_COLON <<
        ORACLE_SCRIPTPARAM_DBUSERNAME << stDBInfo.strDBUsername << NODE_COLON <<
        ORACLE_SCRIPTPARAM_DBPASSWORD << stDBInfo.strDBPassword << NODE_COLON <<
        ORACLE_SCRIPTPARAM_ASMINSTANCE << stDBInfo.strASMInstance << NODE_COLON <<
        ORACLE_SCRIPTPARAM_ASMUSERNAME << stDBInfo.strASMUserName << NODE_COLON <<
        ORACLE_SCRIPTPARAM_ASMPASSWOD  << stDBInfo.strASMPassword  << NODE_COLON <<
        ORACLE_SCIPRTPARAM_BACKUPLEVEL << level << NODE_COLON << SCIPRTPARAM_BACKUPPATH << dataPath;
    ClearString(stDBInfo.strDBPassword);
    ClearString(stDBInfo.strASMPassword);
    return RunBackuplevelScript(rspBody, oss.str());
}

mp_int32 OracleNativeBackup::RunBackuplevelScript(Json::Value& rspBody, const mp_string& strParam)
{
    vector<mp_string> vecResult;
#ifdef WIN32
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_QUERYBACKUPLEVEL, strParam, &vecResult);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_SCRIPT_EXEC_FAILED);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "query backup level failed, iRet %d.", iRet);
        return iRet;
    }
#endif
    if (!vecResult.empty()) {
        vector<mp_string> backupRstInfo;
        CMpString::StrSplit(backupRstInfo, vecResult.front(), CHAR_SEMICOLON);
        if (!backupRstInfo.empty()) {
            rspBody["level"] = atoint32(backupRstInfo.back().c_str());
            return MP_SUCCESS;
        }
    }
    return ERROR_COMMON_OPER_FAILED;
}


#ifdef WIN32
DWORD WINAPI OracleNativeBackup::CheckMountTaskInThread(void* pThis)
#else
mp_void* OracleNativeBackup::CheckMountTaskInThread(void* pThis)
#endif
{
    OracleNativeBackup* pOracleNativeBackup = static_cast<OracleNativeBackup*>(pThis);
    if (!pOracleNativeBackup) {
        COMMLOG(OS_LOG_ERROR, "convert to OracleNativeBackup failed.");
        CMPTHREAD_RETURN;
    }

    COMMLOG(OS_LOG_INFO, "Start CheckMountTask thread.");
    while (!pOracleNativeBackup->GetCMExitFlag()) {
        pOracleNativeBackup->CheckMount();
        CMpTime::DoSleep(CHECKMOUNTINTERVAL);
    }
    COMMLOG(OS_LOG_INFO, "Finish CheckMountTask thread.");

    CMPTHREAD_RETURN;
}

mp_int32 OracleNativeBackup::CreateCheckMountTask()
{
    m_bTExitCheckMount = false;
    return CMpThread::Create(&m_tidCheckMount, CheckMountTaskInThread, this);
}

bool OracleNativeBackup::GetCMExitFlag()
{
    return m_bTExitCheckMount;
}

mp_int32 OracleNativeBackup::CheckMount()
{
    COMMLOG(OS_LOG_DEBUG, "begin CheckMount.");
    std::map<mp_string, std::set<mp_string> > mapMountInfo;
    QueryMountPointInfo(mapMountInfo);
    std::map<mp_string, std::set<mp_string> >::iterator it = mapMountInfo.begin();
    for (; it != mapMountInfo.end(); ++it) {
        Task* pTask = NULL;
        TaskManager::GetInstance()->FindTask(it->first, pTask);
        if (pTask == NULL) {
            continue;
        }

        bool bDisconnect = false;
        for (std::set<mp_string>::iterator iter = it->second.begin(); iter != it->second.end(); ++iter) {
            if (CSocket::CheckHostLinkStatus("", *iter) != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "unable connect to (%s).", (*iter).c_str());
                bDisconnect = true;
                break;
            }
        }
        if (bDisconnect) {
            mp_int32 disconnectNum = pTask->GetDisconnectNum();
            pTask->setDisconnectNum(++disconnectNum);
            COMMLOG(OS_LOG_ERROR, "The task(%s) is disconnected, times: %d.", it->first.c_str(), disconnectNum);
            if (disconnectNum >= CHECKMOUNTMAXNUM) {
                COMMLOG(OS_LOG_ERROR, "The task(%s) does not respond for a long time.", it->first.c_str());
                TaskManager::GetInstance()->CancelTask(it->first);
            }
        } else {
            pTask->setDisconnectNum(0);
        }
    }
    mp_int32 ret = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "end CheckMount.");
    return ret;
}

void OracleNativeBackup::QueryMountPointInfo(std::map<mp_string, std::set<mp_string> >& mapMountInfo)
{
    mapMountInfo.clear();

    std::ostringstream buff;
    DbParamStream dps;
    DBReader readBuff;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;

    buff << "select " << g_ParamKey << ", " << g_ParamValue << " from " << g_BackupParam;
    mp_string strSql = buff.str();

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Query backup parameters failed, iRet %d.", iRet);
        return;
    }
    mp_string paramKey;
    mp_string paramValue;
    for (int i = 0; i < iRowCount; i++) {
        readBuff >> paramKey;
        readBuff >> paramValue;
        if (paramKey != "mountIPs") {
            continue;
        }
        std::set<mp_string> setIP;
        std::vector<mp_string> vecTmp;
        CMpString::StrSplit(vecTmp, paramValue, CHAR_SEMICOLON);
        mp_string taskID = vecTmp.front();
        COMMLOG(OS_LOG_DEBUG, "taskID [%s] MountIPs is %s.", taskID.c_str(), paramValue.c_str());
        mp_int32 size = vecTmp.size();
        for (size_t i = 1; i < size; i++) {
            if (vecTmp.at(i).empty()) {
                continue;
            }
            setIP.insert(vecTmp.at(i));
        }
        mapMountInfo[taskID] = std::move(setIP);
    }
}


mp_int32 OracleNativeBackup::BackupData(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin backup oracle data");
    return BackupOracle(msgBody, connIp, connPort, ORACLE_DATA_BACKUP, taskId);
}

mp_int32 OracleNativeBackup::BackupLog(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin backup oracle log.");
    return BackupOracle(msgBody, connIp, connPort, ORACLE_LOG_BACKUP, taskId);
}

mp_int32 OracleNativeBackup::BackupOracle(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_int32 backupMode, mp_string& taskId)
{
    mp_int32 iRet = GetTaskidByReq(msgBody, taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get taskid by Req failed, iRet %d.", iRet);
        return iRet;
    }

    // set backup type: data(1) or log(2)
    TaskContext::GetInstance()->SetValueInt32(taskId, KEY_BACKUP_MODE, backupMode);

    iRet = CreateTask<OracleNativeBackupTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS; // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeBackupTask failed, iRet %d.", iRet);
        return iRet;
    }

    if (backupMode == ORACLE_DATA_BACKUP) {
        TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_BACKUP_DATA);
    } else {
        TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_BACKUP_LOG);
    }
    COMMLOG(OS_LOG_DEBUG, "begin to run backup task %s.", taskId.c_str());
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::RestoreDB(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    // 传下来的taskid为空
    COMMLOG(OS_LOG_INFO, "Begin restore oracle");
    mp_int32 iRet = CreateTask<OracleNativeRestoreTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS;  // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeRestoreTask failed, iRet %d.", iRet);
        return iRet;
    }

    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_RESTORE);
    COMMLOG(OS_LOG_DEBUG, "begin to run restore task %s.", taskId.c_str());
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::LiveMount(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin start oracle livemount.");
    mp_int32 iRet = CreateTask<OracleNativeLiveMTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS;  // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeLiveMTask failed, iRet %d.", iRet);
        return iRet;
    }

    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_LIVEMOUNT);
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::CancelLiveMount(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin cancel oracle livemount.");
    mp_int32 iRet = CreateTask<OracleNativeCLiveMTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS;  // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeCLiveMTask failed, iRet %d.", iRet);
        return iRet;
    }

    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_CLIVEMOUNT);
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::InstanceRestore(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin to excute instance restore.");
    mp_int32 iRet = CreateTask<OracleNativeInstRestoreTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS;  // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeInstRestoreTask failed, iRet %d.", iRet);
        return iRet;
    }

    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_INSTANT_RESTORE);
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::ExpireCopy(
    const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    COMMLOG(OS_LOG_INFO, "Begin expire oracle copy data.");
    mp_int32 iRet = CreateTask<OracleNativeExpireCopyTask>(msgBody, connIp, connPort, taskId);
    if (iRet == MP_TASK_RUNNING) {
        return MP_SUCCESS;  // task is running, return 0 inform dme wait
    }
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "create OracleNativeExpireCopyTask failed, iRet %d.", iRet);
        return iRet;
    }

    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_TASK_CMDNO, MANAGE_CMD_NO_ORACLE_EXPIRE_COPY);
    return TaskManager::GetInstance()->RunTask(taskId);
}

mp_int32 OracleNativeBackup::PrepareMedia(const mp_string& msgBody, mp_int32 taskType, mp_int32 storType,
    Json::Value& respMsg)
{
    mp_string taskId;
    mp_int32 iRet = GetTaskidByReq(msgBody, taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    TaskContext::GetInstance()->SetValueInt32(taskId, KEY_TASKTYPE, taskType);
    TaskContext::GetInstance()->SetValueInt32(taskId, KEY_STORAGE_TYPE, storType);

    COMMLOG(
        OS_LOG_INFO, "Task(%s) begin to prepare media, taskType %d, storType %d", taskId.c_str(), taskType, storType);
    return RunSyncTask<OracleNativePrepareMediaTask>(msgBody, taskId, respMsg);
}

mp_int32 OracleNativeBackup::DisMountMedium(const mp_string& msgBody, mp_int32 taskType)
{
    mp_string taskId;
    mp_int32 iRet = GetTaskidByReq(msgBody, taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    TaskContext::GetInstance()->SetValueInt32(taskId, KEY_TASKTYPE, taskType);

    COMMLOG(OS_LOG_INFO, "Task(%s) Begin to dismount backup or restore medium, taskType %d", taskId.c_str(), taskType);
    Json::Value respMsg;
    return RunSyncTask<OracleNativeDismountTask>(msgBody, taskId, respMsg);
}

mp_void OracleNativeBackup::BuildQueryStorageTypeScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBPASSWORD) + stDBInfo.strDBPassword + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ORACLE_HOME) + stDBInfo.strOracleHome;
}

mp_void OracleNativeBackup::BuildQueryStorageInfoScriptParam(const oracle_db_info_t& stDBInfo, mp_string& strParam)
{
    strParam = mp_string(ORACLE_SCRIPTPARAM_INSTNAME) + stDBInfo.strInstName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBNAME) + stDBInfo.strDBName + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_DBUSERNAME) + stDBInfo.strDBUsername + mp_string(NODE_COLON) +
               ORACLE_SCRIPTPARAM_DBPASSWORD + stDBInfo.strDBPassword + NODE_COLON +
               mp_string(ORACLE_SCRIPTPARAM_ASMINSTANCE) + stDBInfo.strASMInstance + mp_string(NODE_COLON) +
               mp_string(ORACLE_SCRIPTPARAM_ASMUSERNAME) + stDBInfo.strASMUserName + NODE_COLON +
               ORACLE_SCRIPTPARAM_ASMPASSWOD + stDBInfo.strASMPassword;
}

mp_int32 OracleNativeBackup::AnalyseStorInfoScriptRst(vector<mp_string>& vecResult, Json::Value& dbInfo)
{
    Json::Value dataInfo;
    Json::Value logInfo;
    dbInfo[KEYDATAINFO] = std::move(dataInfo);
    dbInfo[KEYLOGINFO] = std::move(logInfo);

    for (vector<mp_string>::iterator iter = vecResult.begin(); iter != vecResult.end(); ++iter) {
        vector<mp_string> storInfos;
        CMpString::StrSplit(storInfos, *iter, CHAR_SEMICOLON);

        if (storInfos.empty()) {
            COMMLOG(OS_LOG_ERROR, "split string failed, %s.", iter->c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        // storInfos[0]是返回结果文件的key，用于区分执行什么内容的解析
        if (HandlerStoreInfos.find(storInfos[0]) == HandlerStoreInfos.end()) {
            COMMLOG(OS_LOG_ERROR, "storage info have %s no hander.", storInfos[0].c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        FUNC_ANALYSEINFO func = HandlerStoreInfos[storInfos[0]];
        if (func == NULL) {
            COMMLOG(OS_LOG_ERROR, "storage info %s hander is NULL.", storInfos[0].c_str());
            return ERROR_COMMON_OPER_FAILED;
        }

        mp_int32 ret = (this->*func)(storInfos, dbInfo);
        if (ret != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "handle %s storage info failed, ret %d.", storInfos[0].c_str(), ret);
            return ERROR_COMMON_OPER_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::DataCapHandler(const vector<mp_string>& storInfos, Json::Value& dbInfo)
{
    static const mp_int32 numCapacity = 3;
    if (storInfos.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "data capcatiy information have incorrect number, %d.", storInfos.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    static const mp_int32 idxUsed = 1;
    static const mp_int32 idxTotal = 2;
    dbInfo[KEYDATAINFO][KEYUSEDCAP] = atoi(storInfos[idxUsed].c_str());
    dbInfo[KEYDATAINFO][KEYALLCAP] = atoi(storInfos[idxTotal].c_str());
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::LogCapHandler(const vector<mp_string>& storInfos, Json::Value& dbInfo)
{
    static const mp_int32 numCapacity = 3;
    if (storInfos.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "log capcatiy information have incorrect number, %d.", storInfos.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    static const mp_int32 idxUsed = 1;
    static const mp_int32 idxTotal = 2;
    dbInfo[KEYLOGINFO][KEYUSEDCAP] = atoi(storInfos[idxUsed].c_str());
    dbInfo[KEYLOGINFO][KEYALLCAP] = atoi(storInfos[idxTotal].c_str());
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::DBTypeHandler(const vector<mp_string>& storInfos, Json::Value& dbInfo)
{
    static const mp_int32 numCapacity = 2;
    if (storInfos.size() != numCapacity) {
        COMMLOG(OS_LOG_ERROR, "dbType information have incorrect number, %d.", storInfos.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    static const mp_int32 idxType = 1;
    dbInfo[KEYDBTYPEINFO] = atoi(storInfos[idxType].c_str());
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::DBInsLstHandler(const vector<mp_string>& storInfos, Json::Value& dbInfo)
{
    static const mp_int32 numCapacity = 2;
    if (storInfos.size() < numCapacity) {
        COMMLOG(OS_LOG_ERROR, "dbInsLst information have incorrect number, %d.", storInfos.size());
        return ERROR_COMMON_OPER_FAILED;
    }

    static const mp_int32 idxType = 1;
    Json::Value dbInsInfo;
    for (int i = idxType; i < storInfos.size(); i++) {
        dbInsInfo[KEYINSNAME] = storInfos[i];
        dbInfo[KEYINSLST].append(std::move(dbInsInfo));
    }

    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::GetTaskidByReq(const mp_string& msgBody, mp_string& taskid)
{
    Json::Value backupParam;
    mp_string strBuffer;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(msgBody, backupParam);
    if (iRet != MP_SUCCESS) {
        (mp_void)WipeSensitiveForJsonData(msgBody, strBuffer);
        COMMLOG(OS_LOG_ERROR, "dpp message string %s.", strBuffer.c_str());
        return iRet;
    }

    if (!backupParam.isMember(MANAGECMD_KEY_BODY)) {
        (mp_void)WipeSensitiveForJsonData(msgBody, strBuffer);
        COMMLOG(OS_LOG_ERROR, "dpp message string have no body, %s.", strBuffer.c_str());
        return iRet;
    }
    Json::Value bodyParam = backupParam[MANAGECMD_KEY_BODY];
    GET_JSON_STRING(bodyParam, KEY_TASKID, taskid);
    CHECK_FAIL_EX(CheckParamStringEnd(taskid, 0, MAX_TASKID_LEN));
    COMMLOG(OS_LOG_DEBUG, "get taskid is %s", taskid.c_str());
    return MP_SUCCESS;
}

mp_int32 OracleNativeBackup::StopTask(const mp_string& msgBody, mp_string& taskId)
{
    mp_int32 iRet = GetTaskidByReq(msgBody, taskId);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get taskid by Req failed, iRet %d.", taskId.c_str(), iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_INFO, "Begin to stop task %s.", taskId.c_str());

    return TaskManager::GetInstance()->CancelTask(taskId);
}
