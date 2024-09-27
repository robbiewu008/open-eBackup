#ifndef AGENT_BACKUP_STEP_CHECK_ORACLE_CLOSE_H
#define AGENT_BACKUP_STEP_CHECK_ORACLE_CLOSE_H

#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"

static const mp_string STEPNAME_CHECK_DBCLOSE = "TaskStepCheckDBClose";
class TaskStepCheckDBClose : public TaskStepOracleNative {
public:
    TaskStepCheckDBClose(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepCheckDBClose();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Stop(const Json::Value& param);
    
private:
    mp_void BuildCheckDBScriptParam(mp_string& strParam);
};

#endif
