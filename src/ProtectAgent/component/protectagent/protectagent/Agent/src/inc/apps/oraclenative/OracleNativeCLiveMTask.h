#ifndef AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK
#define AGENT_ORACLE_NATIVE_CANCEL_LIVEMOUNT_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeCLiveMTask : public OracleNativeTask {
public:
    OracleNativeCLiveMTask(const mp_string& taskID);
    virtual ~OracleNativeCLiveMTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
