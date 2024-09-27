#ifndef AGENT_VMWARENATIVE_VMRESTORE_TASK
#define AGENT_VMWARENATIVE_VMRESTORE_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"
#include "common/Types.h"

class VMwareNativeVmRestoreTask : public VMwareNativeTask {
public:
    VMwareNativeVmRestoreTask(const mp_string &taskID);
    virtual ~VMwareNativeVmRestoreTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
