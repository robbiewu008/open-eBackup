#ifndef AGENT_BACKUP_STEP_MOVE_DBF_ORACLE
#define AGENT_BACKUP_STEP_MOVE_DBF_ORACLE
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_NATIVE_MOVE_DBF = "TaskStepOracleNativeMoveDBF";
class TaskStepOracleNativeMoveDBF : public TaskStepOracleNative {
public:
    TaskStepOracleNativeMoveDBF(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeMoveDBF();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Stop();

private:
    mp_string recoverPath;
    mp_int32 recoverOrder;
    mp_int32 recoverNum;
    mp_int32 BuildScriptParam(mp_string& strParam);
};

#endif