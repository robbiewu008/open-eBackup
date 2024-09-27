#ifndef AGENT_BACKUP_STEP_CLEAR_DATABASE_H
#define AGENT_BACKUP_STEP_CLEAR_DATABASE_H

#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"

static const mp_string STEPNAME_CLEAR_DATABASE = "TaskStepClearDataBase";
class TaskStepClearDataBase : public TaskStepOracleNative {
public:
    TaskStepClearDataBase(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepClearDataBase();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Stop(const Json::Value& param);

private:
    mp_void BuildScriptParam(mp_string& strParam);
};

#endif
