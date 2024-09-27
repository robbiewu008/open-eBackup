#ifndef AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_H
#define AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_H
#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"

static const mp_string STEPNAME_NATIVE_EXPIRECOPY = "TaskStepOracleNativeExpireCopy";
class TaskStepOracleNativeExpireCopy : public TaskStepOracleNative {
public:
    TaskStepOracleNativeExpireCopy(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeExpireCopy();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    mp_string maxTime;
    mp_string minTime;
    mp_string maxScn;
    mp_string minScn;
    mp_int32 BuildScriptParam(mp_string& strParam);
};

#endif
