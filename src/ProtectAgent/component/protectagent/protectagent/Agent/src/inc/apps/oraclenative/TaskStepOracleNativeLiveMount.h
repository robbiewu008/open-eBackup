#ifndef AGENT_BACKUP_STEP_LIVEMOUNT_ORACLE
#define AGENT_BACKUP_STEP_LIVEMOUNT_ORACLE
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_NATIVE_LIVEMOUNT = "TaskStepOracleNativeLiveMount";
class TaskStepOracleNativeLiveMount : public TaskStepOracleNative {
public:
    TaskStepOracleNativeLiveMount(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeLiveMount();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    mp_int32 BuildScriptParam(mp_string& strParam);

private:
    mp_int32 channel;
    mp_int32 startDB;
    mp_int32 recoverOrder;
    mp_int32 recoverNum;
    mp_string encAlgo;
    mp_string encKey;
};

#endif
