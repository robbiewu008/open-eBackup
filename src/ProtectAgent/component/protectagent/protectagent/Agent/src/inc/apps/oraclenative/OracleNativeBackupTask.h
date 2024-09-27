#ifndef AGENT_ORACLE_NATIVE_BACKUP_TASK
#define AGENT_ORACLE_NATIVE_BACKUP_TASK

#include "apps/oraclenative/OracleNativeTask.h"
#include "common/Types.h"

class OracleNativeBackupTask : public OracleNativeTask {
public:
    OracleNativeBackupTask(const mp_string& taskID);
    virtual ~OracleNativeBackupTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);
    static Task *CreateRedoTask(const mp_string& taskId);

private:
    mp_int32 backupMode;
    mp_void CreateTaskStep();
};

#endif
