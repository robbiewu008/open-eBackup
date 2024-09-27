#ifndef AGENT_ORACLE_NATIVE_INST_RESTORE_TASK
#define AGENT_ORACLE_NATIVE_INST_RESTORE_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeInstRestoreTask : public OracleNativeTask {
public:
    OracleNativeInstRestoreTask(const mp_string& taskID);
    virtual ~OracleNativeInstRestoreTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
