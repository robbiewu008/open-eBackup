#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_UMOUNT_VMWARENASMEDIA_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_UMOUNT_VMWARENASMEDIA_H

#include "taskmanager/TaskStepPrepareNasMedia.h"

static const mp_string STEPNAME_UMOUNT_VMWARENASMEDIA = "TaskStepUmountVMwareNasMedia";
class TaskStepUmountVMwareNasMedia : public TaskStepPrepareNasMedia {
public:
    TaskStepUmountVMwareNasMedia(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepUmountVMwareNasMedia();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
    mp_void SetNasMediaParam(const mp_string &mountParam);

private:
    mp_string m_strParentTaskId;
    mp_int32 m_iStorageProtocol;
    mp_string m_strNasStorageIP;
    mp_string m_strNasSharePath;
    bool m_unmountNasOverFC = false;
};

#endif
