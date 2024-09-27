#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_INIT_VMWAREDISKLIB = "TaskStepInitVMwareDiskLib";
class TaskStepInitVMwareDiskLib : public TaskStepVMwareNative {
public:
    TaskStepInitVMwareDiskLib(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepInitVMwareDiskLib();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
