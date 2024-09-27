#ifndef AGENT_VMWARENATIVE_CLEANUP_VDDKLIB_TASK
#define AGENT_VMWARENATIVE_CLEANUP_VDDKLIB_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeCleanupVddkLibTask : public VMwareNativeTask {
public:
    VMwareNativeCleanupVddkLibTask(const mp_string &taskID);
    virtual ~VMwareNativeCleanupVddkLibTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();

private:
    mp_int32 m_storageProtocol;
};

#endif
