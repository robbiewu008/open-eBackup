#ifndef AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK
#define AGENT_VMWARENATIVE_INIT_VDDKLIB_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeInitVddkLibTask : public VMwareNativeTask {
public:
    VMwareNativeInitVddkLibTask(const mp_string &taskID);
    virtual ~VMwareNativeInitVddkLibTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();

private:
    mp_int32 m_storageProtocol;
};

#endif
