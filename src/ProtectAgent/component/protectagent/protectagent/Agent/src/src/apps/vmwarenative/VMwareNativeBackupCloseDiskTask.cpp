#include "apps/vmwarenative/VMwareNativeBackupCloseDiskTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskStepScanDisk.h"
#include "apps/vmwarenative/TaskStepBackupCloseDisk.h"
#include "apps/vmwarenative/TaskStepPrepareTargetLun.h"
using namespace std;

VMwareNativeBackupCloseDiskTask::VMwareNativeBackupCloseDiskTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeBackupCloseDiskTask";
    CreateTaskStep();
}

VMwareNativeBackupCloseDiskTask::~VMwareNativeBackupCloseDiskTask()
{}

mp_int32 VMwareNativeBackupCloseDiskTask::InitTaskStep(const Json::Value &param)
{
    // trigger vm opendisk backup task step
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_BACKUP_CLOSEDISK)) {
        ERRLOG("Unable to init close disk backup, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Init close disk backup parameters successfully, task id '%s'.", m_taskId.c_str());

    return MP_SUCCESS;
}

mp_void VMwareNativeBackupCloseDiskTask::CreateTaskStep()
{
    ADD_TASKSTEP(TaskStepBackupCloseDisk, STEPNAME_BACKUP_CLOSEDISK, TASK_STEP_INTERVAL_HUNDERED, m_steps);
}
