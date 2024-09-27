#ifndef AGENT_VMWARENATIVE_VMBACKUP_TASK
#define AGENT_VMWARENATIVE_VMBACKUP_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "common/Types.h"

class VMwareNativeVmBackupTask : public VMwareNativeTask {
public:
    VMwareNativeVmBackupTask(const mp_string &taskID);
    virtual ~VMwareNativeVmBackupTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
