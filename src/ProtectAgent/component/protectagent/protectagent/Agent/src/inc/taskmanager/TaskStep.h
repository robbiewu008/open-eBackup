#ifndef _AGENT_TASKSTEP_H_
#define _AGENT_TASKSTEP_H_

#include <sstream>
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "common/DB.h"
#include "common/Utils.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "common/Utils.h"

static const mp_string SCIPRTPARAM_BACKUPPATH = "DataPath=";
static const mp_string SCIPRTPARAM_ARCHIVEPATH = "LogPath=";
static const mp_string SCIPRTPARAM_METADATAPATH = "MetaDataPath=";

const mp_int32 SLEEP_10000_MS  = 10000;

typedef mp_int32 (*pFunc)(mp_string);

typedef enum taskStatus_t {
    STATUS_NO_EXISTS = 0,
    STATUS_INITIAL,
    STATUS_INPROGRESS,
    STATUS_INPROGRESS_MOVING_DBF,
    STATUS_FAILED,
    STATUS_COMPLETED,
    STATUS_DELETING,
    STATUS_DELETED,
    STATUS_UPDATING,
    STATUS_ABORTED
} TaskStatus;

typedef enum oracleNative_taskType_t {
    ORACLENATIVE_TASK_DATA_BACKUP = 1,
    ORACLENATIVE_TASK_LOG_BACKUP,
    ORACLENATIVE_TASK_RESTORE,
    ORACLENATIVE_TASK_LIVEMOUNT,
    ORACLENATIVE_TASK_INSTANT_RESTORE,
    ORACLENATIVE_TASK_DELETE
} OracleNativeTaskType;

#define CHECK_PARAM_EXISTS(params, key)                                                                                \
    {                                                                                                                  \
        if (!params.isObject() || !params.isMember(key)) {                                                             \
            COMMLOG(OS_LOG_ERROR, "param have no backup param key %s.", key.c_str());                                  \
            return MP_FAILED;                                                                                          \
        }                                                                                                              \
    }

class TaskStep {
public:
    TaskStep(const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order)
    {
        m_stepId = id;
        m_taskId = taskId;
        m_stepName = name;
        m_stepStatus = STATUS_INITIAL;
        m_stepProgress = 0;
        m_stepExpiration = 0;
        m_stepSpeed = 0;
        m_backupSize = 0;
        m_ratio = ratio;
        m_order = order;
    }

    virtual ~TaskStep()
    {
        // delete extend taskstep
        for (int i = 0; i < m_clearSteps.size(); ++i) {
            if (m_clearSteps[i] != NULL) {
                delete m_clearSteps[i];
            }
        }
        m_clearSteps.clear();
        m_respMsg.clear();
    }

    virtual mp_int32 Init(const Json::Value& param) = 0;
    virtual mp_int32 Run() = 0;
    virtual mp_int32 Cancel() = 0;
    virtual mp_int32 Cancel(Json::Value& respParam) = 0;
    virtual mp_int32 Stop(const Json::Value& param) = 0;
    virtual mp_int32 Update(const Json::Value& param) = 0;
    virtual mp_int32 Update(Json::Value& param, Json::Value& respParam) = 0;
    virtual mp_int32 Finish(const Json::Value& param) = 0;
    virtual mp_int32 Finish(Json::Value& param, Json::Value& respParam) = 0;

    // redo, check script pid, and wait script complelted
    // check result file: succ,failed,no exists(failed)
    virtual mp_int32 Redo(mp_string& innerPID) = 0;

    // try to refresh task step information, .eg
    // oracle: query information by rman with script
    // vmware: do nothing
    virtual mp_int32 RefreshStepInfo()
    {
        return MP_SUCCESS;
    }

    mp_string GetTaskId()
    {
        return m_taskId;
    }

    mp_string GetStepId()
    {
        return m_stepId;
    }

    mp_string GetStepName()
    {
        return m_stepName;
    }

    void SetInnerPID(const mp_string& innerPID)
    {
        m_taskStepInnerPID = innerPID;
    }

    mp_string GetInnerPID()
    {
        return m_taskStepInnerPID;
    }

    mp_void SetProgress(mp_int32 progress)
    {
        m_stepProgress = progress;
    }

    mp_int32 GetProgress()
    {
        return m_stepProgress;
    }

    TaskStatus GetStatus()
    {
        return m_stepStatus;
    }

    mp_int32 GetSpeed()
    {
        return m_stepSpeed;
    }

    mp_int32 GetBackupSize()
    {
        return m_backupSize;
    }

    mp_void SetExpiration(mp_int64 expiration)
    {
        m_stepExpiration = expiration;
    }

    mp_int64 GetExpiration()
    {
        return m_stepExpiration;
    }

    mp_int32 GetRatio()
    {
        return m_ratio;
    }

    mp_int32 GetOrder()
    {
        return m_order;
    }

    mp_void AddClearStep(TaskStep *clearStep)
    {
        m_clearSteps.push_back(clearStep);
    }

    std::vector<TaskStep*> &GetClearSteps()
    {
        return m_clearSteps;
    }

    mp_void SetParamKey(const mp_string& paramKey)
    {
        m_stepParamKey = paramKey;
    }

    mp_string GetParamKey()
    {
        return m_stepParamKey;
    }

    std::vector<std::pair<mp_string, mp_uint32> > &GetWarnInfo()
    {
        return m_vecWarnInfo;
    }
#ifndef WIN32
    mp_int32 WaitScriptProcessFinish(mp_string& strUniqueID)
    {
        CRootCaller rootCaller;
        std::vector<mp_string> vecResult;
        mp_int32 iRet = rootCaller.ReadResultFile(ROOT_COMMAND_SCRIPT_PID, strUniqueID, &vecResult);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }

        if (vecResult.size() != 1) {
            COMMLOG(OS_LOG_ERROR, "Get Script PID failed.");
            return MP_FAILED;
        }
        
        // get script PID ,if exist
        mp_string scrpitPID = vecResult.front();
        CHECK_FAIL_EX(CheckCmdDelimiter(scrpitPID));
        while (true) {
            // check if script process exist
            std::vector<mp_string> vecResult;
            mp_string strCmd = "ps -aef | grep -v grep | awk '{print $2}' | grep '" + scrpitPID + "$'";
            iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecResult);
            if (iRet != MP_SUCCESS) {
                ERRLOG("Exec system cmd failed,iRet is %d.", iRet);
                return iRet;
            }
            if (vecResult.empty()) {
                COMMLOG(OS_LOG_INFO, "Check script process(%s) is empty, will check result file.", scrpitPID.c_str());
                break;
            }
            COMMLOG(OS_LOG_DEBUG, "Script process(%s) still exists, continue wait.", scrpitPID.c_str());
            DoSleep(SLEEP_10000_MS);
        }
        
        return MP_SUCCESS;
    }
#endif

    mp_int32 CheckScriptResult(mp_string& strUniqueID)
    {
        CRootCaller rootCaller;
        std::vector<mp_string> vecResult;
        mp_int32 iRet = rootCaller.ReadResultFile(ROOT_COMMAND_THIRDPARTY, strUniqueID, &vecResult);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }

        if (vecResult.size() != 1) {
            COMMLOG(OS_LOG_ERROR, "Get Script result failed.");
            return MP_FAILED;
        }
        
        // get script result , if exist and result is success
        mp_string scrpitResult = vecResult.front();
        if (scrpitResult == "Success") {
            return MP_SUCCESS;
        }
        COMMLOG(OS_LOG_ERROR, "Get Script result is %s.", scrpitResult.c_str());
        return MP_FAILED;
    }

    // record parameter into database for backup or restore
    mp_int32 UpdateParam(const mp_string& paramID, const mp_string& paramKey, const mp_string& paramValue)
    {
        DbParamStream dps;
        mp_string strSql;

        mp_int32 rowNum;
        mp_int32 iRet = GetParamCount(paramID, paramKey, rowNum);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }

        std::ostringstream insertBuff;
        if (rowNum >= 1) {
            COMMLOG(OS_LOG_DEBUG, "paramID=%s,key=%s is exists.", paramID.c_str(), paramKey.c_str());
            insertBuff << "update " << g_BackupParam << " set " << g_ParamValue << "= ?"
                       << " where " << g_ParamID << "== ? and " << g_ParamKey << "== ?";
            strSql = insertBuff.str();

            DbParam dp = paramValue;
            dps << std::move(dp);
            dp = paramID;
            dps << std::move(dp);
            dp = paramKey;
            dps << std::move(dp);
        } else {
            COMMLOG(OS_LOG_DEBUG, "paramID=%s,key=%s is not exists.", paramID.c_str(), paramKey.c_str());
            insertBuff << "insert into " << g_BackupParam << "(" << g_ParamID << "," << g_ParamKey << ","
                       << g_ParamValue << ") values(?,?,?);";
            strSql = insertBuff.str();

            DbParam dp = paramID;
            dps << std::move(dp);
            dp = paramKey;
            dps << std::move(dp);
            dp = paramValue;
            dps << std::move(dp);
        }

        iRet = CDB::GetInstance().ExecSql(strSql, dps);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "update backup parameter failed, iRet=%d.", iRet);
            return iRet;
        }

        COMMLOG(OS_LOG_DEBUG, "update ID=%s,key=%s,value=%s.", paramID.c_str(), paramKey.c_str(), paramValue.c_str());
        return iRet;
    }

    // record parameter into database for backup or restore
    mp_int32 RemoveParam(const mp_string& paramID)
    {
        DbParamStream dps;

        std::ostringstream removeStream;
        removeStream << "delete from " << g_BackupParam << " where " << g_ParamID << "== ?";
        DbParam dp = paramID;
        dps << std::move(dp);

        mp_int32 iRet = CDB::GetInstance().ExecSql(removeStream.str(), dps);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "remove backup parameter failed, paramID=%s,iRet=%d.", paramID.c_str(), iRet);
            return iRet;
        }

        COMMLOG(OS_LOG_DEBUG, "remove backup param %s succ.", paramID.c_str());
        return iRet;
    }

    mp_int32 GetBackupParam(const mp_string& paramID, const mp_string& paramKey, mp_string& paramValue)
    {
        std::ostringstream buff;
        DbParamStream dps;
        DBReader readBuff;
        mp_int32 iRowCount = 0;
        mp_int32 iColCount = 0;
        mp_string strSql;

        buff << "select " << g_ParamValue << " from " << g_BackupParam << " where " << g_ParamID << " == ? and "
             << g_ParamKey << " == ?";
        strSql = buff.str();
        DbParam dp = paramID;
        dps << std::move(dp);
        dp = paramKey;
        dps << std::move(dp);

        mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Query backup parameters failed, iRet %d.", iRet);
            return iRet;
        }

        if (iRowCount != 1) {
            COMMLOG(OS_LOG_ERROR, "rowcount=%d invalid, id=%s, key=%s.", iRowCount, paramID.c_str(), paramKey.c_str());
            return MP_FAILED;
        }

        readBuff >> paramValue;
        COMMLOG(OS_LOG_DEBUG, "Get ID=%s,key=%s,value=%s.", paramID.c_str(), paramKey.c_str(), paramValue.c_str());
        return MP_SUCCESS;
    }

    mp_int32 BuildDataLogPath(std::ostringstream& oss, const mp_string& paramID)
    {
        // NAS: get datapath and log path from db, which is writed by prepare step
        mp_string dataPath;
        mp_int32 iRet = GetBackupParam(paramID, "dataPath", dataPath);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Query dataPath failed, id=%s, iRet %d", paramID.c_str(), iRet);
            return iRet;
        }
        oss << NODE_COLON << SCIPRTPARAM_BACKUPPATH << dataPath;

        mp_string metaPath;
        iRet = GetBackupParam(paramID, "metaPath", metaPath);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Query metaPath failed, id=%s, iRet %d", paramID.c_str(), iRet);
            return iRet;
        }
        oss << NODE_COLON << SCIPRTPARAM_METADATAPATH << metaPath;

        mp_string logPath;
        iRet = GetBackupParam(paramID, "logPath", logPath);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Query logPath failed, id=%s, iRet %d", paramID.c_str(), iRet);
            return iRet;
        }
        oss << NODE_COLON << SCIPRTPARAM_ARCHIVEPATH << logPath;
        return MP_SUCCESS;
    }

    void GetRespMsg(Json::Value &respMsg)
    {
        respMsg = m_respMsg;
    }

protected:
    mp_string m_stepId;
    mp_string m_taskId;
    mp_string m_stepName;
    TaskStatus m_stepStatus;
    mp_int32 m_stepProgress;
    mp_int32 m_stepSpeed;
    mp_int32 m_backupSize;
    mp_int64 m_stepExpiration;
    // task step ratio, for computing the progress
    mp_int32 m_ratio;
    thread_lock_t m_memStatusLock;
    thread_lock_t m_memProgressLock;
    // task step sort order
    mp_int32 m_order;
    mp_string m_taskStepInnerPID;
    // when running task step failed, some clear resource task step may be need to excute
    std::vector<TaskStep*> m_clearSteps;
    mp_string m_stepParamKey;
    /*
    m_vecWarnInfo:
    When the whole step executed successfully, but should indicate warnings to user.
    */
    std::vector<std::pair<mp_string, mp_uint32> >  m_vecWarnInfo;
    Json::Value m_respMsg;

    // update task job's pids
    static mp_int32 UpdateInnerPIDCallBack(void* pPointer, const mp_string& innerPID)
    {
        TaskStep* pStep = static_cast<TaskStep*>(pPointer);
        if (pStep == NULL) {
            return MP_FAILED;
        }
        mp_string taskId = pStep->GetTaskId();
        mp_string subStepName = pStep->GetStepName();
        pStep->SetInnerPID(innerPID);

        COMMLOG(OS_LOG_DEBUG, "Get taskid is %s, stepName is %s, innerPID %s.",
            taskId.c_str(), subStepName.c_str(), innerPID.c_str());
        
        DbParamStream dps;
        std::ostringstream updateBuff;
        updateBuff << "update " << JOBS << " set " << g_InnerPID << "= ?"
                   << " where " << g_ID << " == ?" << " and " << g_SubStep << " == ?";
        mp_string strSql = updateBuff.str();
        DbParam dp = innerPID;
        dps << std::move(dp);
        dp = taskId;
        dps << std::move(dp);
        dp = subStepName;
        dps << std::move(dp);

        mp_int32 iRet = CDB::GetInstance().ExecSql(strSql, dps);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Update Inner PID failed, iRet=%d.", iRet);
        }
        COMMLOG(OS_LOG_DEBUG, "update taskid  %s innerPID to %s.", taskId.c_str(), innerPID.c_str());

        return iRet;
    }

private:
    mp_int32 GetParamCount(const mp_string& paramID, const mp_string& paramKey, mp_int32 &num)
    {
        std::ostringstream buff;
        DbParamStream dps;
        DBReader readBuff;
        mp_int32 iRowCount = 0;
        mp_int32 iColCount = 0;
        mp_string strSql;

        buff << "select count(*) from " << g_BackupParam << " where " << g_ParamID << " == ? and " << g_ParamKey
             << " == ?";
        strSql = buff.str();
        DbParam dp = paramID;
        dps << std::move(dp);
        dp = paramKey;
        dps << std::move(dp);

        mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Query backup parameters failed, iRet %d.", iRet);
            return iRet;
        }

        for (mp_int32 iRowCurr = 0; iRowCurr < iRowCount; ++iRowCurr) {
            mp_string strRowNum;
            readBuff >> strRowNum;
            num = atoi(strRowNum.c_str());
        }
        return MP_SUCCESS;
    }
};

#endif  // _AGENT_TASKSTEP_H_
