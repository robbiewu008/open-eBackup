#ifndef AGENT_VMWARENATIVE_BACKUP_CLOSEDISK_TASK
#define AGENT_VMWARENATIVE_BACKUP_CLOSEDISK_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupCloseDiskTask : public VMwareNativeTask {
public:
    VMwareNativeBackupCloseDiskTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupCloseDiskTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
