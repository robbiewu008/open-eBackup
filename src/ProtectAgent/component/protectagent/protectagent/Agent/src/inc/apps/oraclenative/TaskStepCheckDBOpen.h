#ifndef AGENT_BACKUP_STEP_CHECK_ORACLE_OPEN_H
#define AGENT_BACKUP_STEP_CHECK_ORACLE_OPEN_H

#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_CHECK_ORACLE_OPEN = "TaskStepCheckDBOpen";
class TaskStepCheckDBOpen : public TaskStepOracleNative {
public:
    TaskStepCheckDBOpen(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepCheckDBOpen();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Stop(const Json::Value& param);

private:
    mp_void BuildCheckDBScriptParam(mp_string& strParam);
};

#endif
