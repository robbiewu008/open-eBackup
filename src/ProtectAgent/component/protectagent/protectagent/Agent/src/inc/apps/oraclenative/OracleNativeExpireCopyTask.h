#ifndef AGENT_ORACLE_NATIVE_EXPIRE_COPY_TASK
#define AGENT_ORACLE_NATIVE_EXPIRE_COPY_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeExpireCopyTask : public OracleNativeTask {
public:
    OracleNativeExpireCopyTask(const mp_string& taskID);
    virtual ~OracleNativeExpireCopyTask();

    mp_int32 InitTaskStep(const Json::Value& param);
    static Task *CreateRedoTask(const mp_string& taskId);
private:
    mp_void CreateTaskStep();
};

#endif
