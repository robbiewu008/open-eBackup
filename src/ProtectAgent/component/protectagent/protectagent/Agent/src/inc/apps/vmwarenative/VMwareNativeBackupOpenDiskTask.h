#ifndef AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK
#define AGENT_VMWARENATIVE_BACKUP_OPENDISK_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupOpenDiskTask : public VMwareNativeTask {
public:
    VMwareNativeBackupOpenDiskTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupOpenDiskTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
