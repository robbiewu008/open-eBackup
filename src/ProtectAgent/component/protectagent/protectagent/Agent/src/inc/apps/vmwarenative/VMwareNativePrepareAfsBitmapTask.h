#ifndef AGENT_VMWARENATIVE_BACKUP_AFSBITMAP_TASK
#define AGENT_VMWARENATIVE_BACKUP_AFSBITMAP_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativePrepareAfsBitmapTask : public VMwareNativeTask {
public:
    VMwareNativePrepareAfsBitmapTask(const mp_string &taskID);
    virtual ~VMwareNativePrepareAfsBitmapTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
};

#endif
