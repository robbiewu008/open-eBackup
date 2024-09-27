#ifndef AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK
#define AGENT_ORACLE_NATIVE_LIVEMOUNT_TASK

#include "common/Types.h"
#include "apps/oraclenative/OracleNativeTask.h"

class OracleNativeLiveMTask : public OracleNativeTask {
public:
    OracleNativeLiveMTask(const mp_string& taskID);
    virtual ~OracleNativeLiveMTask();
    // initialize task step param by tcp channel
    mp_int32 InitTaskStep(const Json::Value& param);

private:
    mp_void CreateTaskStep();
};

#endif
