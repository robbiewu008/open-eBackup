#ifndef __AGENT_TASKSTEP_BACKUP_PREPARATION_H__
#define __AGENT_TASKSTEP_BACKUP_PREPARATION_H__

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_BACKUP_PREPARATION = "TaskStepBackupPreparation";
class TaskStepBackupPreparation : public TaskStepVMwareNative {
public:
    TaskStepBackupPreparation(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepBackupPreparation();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
