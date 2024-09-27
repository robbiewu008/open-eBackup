#ifndef AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK
#define AGENT_VMWARENATIVE_RESTORE_PREPARATION_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeRestorePreparationTask : public VMwareNativeTask {
public:
    VMwareNativeRestorePreparationTask(const mp_string &taskID);
    virtual ~VMwareNativeRestorePreparationTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
    mp_int32 PrepareNasMedia(const Json::Value &param);
    mp_int32 PrepareIscsiMedia(const Json::Value &param);

private:
    mp_int32 m_storageProtocol;
    mp_string m_diskType;
};

#endif
