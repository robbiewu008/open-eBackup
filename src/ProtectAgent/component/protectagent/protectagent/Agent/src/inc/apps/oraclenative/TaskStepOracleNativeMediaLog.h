#ifndef AGENT_BACKUP_STEP_PREPARE_MEDIA_LOG_H
#define AGENT_BACKUP_STEP_PREPARE_MEDIA_LOG_H

#include "apps/oraclenative/TaskStepOracleNativeMedia.h"
#include "common/Types.h"

static const mp_string STEPNAME_PREPAREMEDIA_LOG = "TaskStepOracleNativeMediaLog";

class TaskStepOracleNativeMediaLog : public TaskStepOracleNativeMedia {
public:
    TaskStepOracleNativeMediaLog(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeMediaLog();
    mp_int32 Init(const Json::Value& param);

protected:
    mp_int32 InitialVolumes(mp_string& diskList);
    mp_int32 InitMountPath();

private:
    mp_int32 PrepareMediumInfo();
};

#endif
