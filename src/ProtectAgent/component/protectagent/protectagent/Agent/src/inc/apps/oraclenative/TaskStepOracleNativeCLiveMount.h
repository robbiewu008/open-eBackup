#ifndef AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#define AGENT_BACKUP_STEP_CANCEL_LIVEMOUNT_ORACLE
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_CANCEL_LIVEMOUNT = "TaskStepCancelLiveMount";
class TaskStepOracleNativeCLiveMount : public TaskStepOracleNative {
public:
    TaskStepOracleNativeCLiveMount(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeCLiveMount();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    mp_void BuildScriptParam(mp_string& strParam);
};

#endif
