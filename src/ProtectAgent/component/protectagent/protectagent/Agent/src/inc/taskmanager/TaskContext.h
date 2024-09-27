#ifndef _AGENT_TASKCONTEXT_H_
#define _AGENT_TASKCONTEXT_H_

#include <map>
#include <vector>
#include "common/Types.h"
#include "common/Log.h"
#include "jsoncpp/include/json/json.h"
#include "jsoncpp/include/json/value.h"
#include "common/CMpThread.h"

static const mp_string KEY_TASKID = "taskId";
static const mp_string KEY_PARENT_TASKID = "parentTaskId";
static const mp_string KEY_ERRCODE = "errcode";
static const mp_string KEY_ERRMSG = "errMsg";
static const mp_string KEY_INPUTPARAM = "inputParams";
// 0:data 1:log
static const mp_string KEY_BACKUP_MODE = "backupmode";
// task cmd number
static const mp_string KEY_TASK_CMDNO = "task_cmdno";
// connection ip, for reportting message
static const mp_string KEY_CONNECTION_IP = "connip";
// connection ip port, for reportting message
static const mp_string KEY_CONNECTION_PORT = "connport";
// task result
static const mp_string KEY_TASK_RESULT = "task_result";
// oracle store detail
static const mp_string KEY_ORACLE_STORE_DETAIL = "store_detail_oracle";
// taskType
static const mp_string KEY_TASKTYPE = "taskType";
// hostRole
static const mp_string KEY_HOSTROLE = "hostRole";
// storage type: 0-nas, 1-iscsi
static const mp_string KEY_STORAGE_TYPE = "storage_type";
static const mp_string KEY_STORAGE_PROTOCOL = "storage_protocol";
// disk type: 0-normal, 1-rdm
static const mp_string KEY_DISK_TYPE = "disk_type";

static const mp_string STR_CMD_KEY = "cmd";
static const mp_int32 STR_VMWARENATIVE_FINISH_DISK_BACKUP_CMD_VALUE = 0x040C;
static const mp_int32 STR_VMWARENATIVE_FINISH_DISK_RECOVERY_CMD_VALUE = 0x0418;
static const mp_string KEY_REDOTASK_FLAG = "redotask_flag";

static const mp_string g_CUR_RUNNING_STEP = "cur_running_step";
static const mp_string g_CUR_INNERPID = "cur_innerpid";

static const mp_string KEY_APIINVOKE_TIMESTAMP = "invokedTimestamp";
static const mp_int32 KEY_APIINVOKE_TIMEINTERVAL_VALUE = 300;

class TaskContext {
public:
    static TaskContext* GetInstance();
    mp_void SetJsonValue(const mp_string& taskID, const mp_string& strKey, const Json::Value& jsonVal);
    mp_void SetValueString(const mp_string& taskID, const mp_string& strKey, const mp_string& strValue);
    mp_void SetValueInt32(const mp_string& taskID, const mp_string& strKey, const mp_int32& iValue);
    mp_void SetValueUInt32(const mp_string& taskID, const mp_string& strKey, const mp_uint32& iValue);
    mp_int32 GetValueString(const mp_string& taskID, const mp_string& strKey, mp_string& strValue);
    mp_int32 GetValueInt32(const mp_string& taskID, const mp_string& strKey, mp_int32& iValue);
    mp_int32 GetValueUInt32(const mp_string& taskID, const mp_string& strKey, mp_uint32& uiValue);
    mp_int32 GetValueVector(const mp_string& taskID, const mp_string& strKey, std::vector<mp_string>& vecValue);
    mp_int32 GetValueJson(const mp_string& taskID, const mp_string& strKey, Json::Value& jsValue);
    mp_int32 GetValueStringOption(const mp_string& taskID, const mp_string& strKey, mp_string& strValue);
    mp_int32 GetValueInt32Option(const mp_string& taskID, const mp_string& strKey, mp_int32& iValue);
    mp_int32 GetValueUInt32Option(const mp_string& taskID, const mp_string& strKey, mp_uint32& uiValue);
    mp_int32 GetValueVectorOption(const mp_string& taskID, const mp_string& strKey, std::vector<mp_string>& vecValue);

    mp_void RemoveTaskContext(const mp_string& taskID);

public:
    // Json::Value m_taskContextJsonDate;
    std::map<mp_string, Json::Value> m_taskContextJsonDate;

private:
    TaskContext();
    ~TaskContext();

private:
    static TaskContext* m_pTaskContext;
    thread_lock_t m_taskcontextLock;
};

#endif  // _AGENT_TASKCONTEXT_H_