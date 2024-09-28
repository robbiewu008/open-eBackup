#include "apps/vmwarenative/VMwareNativeInitVddkLibTask.h"

#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskStepScanDisk.h"
#include "apps/vmwarenative/TaskStepPrepareVMwareNasMedia.h"
#include "apps/vmwarenative/TaskStepInitVMwareDiskLib.h"

using namespace std;
VMwareNativeInitVddkLibTask::VMwareNativeInitVddkLibTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeInitVddkLibTask";
    CreateTaskStep();
}

VMwareNativeInitVddkLibTask::~VMwareNativeInitVddkLibTask()
{}

mp_int32 VMwareNativeInitVddkLibTask::InitTaskStep(const Json::Value &param)
{
    mp_int32 iRet = MP_FAILED;

    // trigger VDDK lib init task step
    iRet = InitTaskStepParam(param, "", STEPNAME_INIT_VMWAREDISKLIB);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init VDDK lib failed, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    return iRet;
}

mp_void VMwareNativeInitVddkLibTask::CreateTaskStep()
{
    ADD_TASKSTEP(TaskStepInitVMwareDiskLib, STEPNAME_INIT_VMWAREDISKLIB, TASK_STEP_INTERVAL_HUNDERED, m_steps);
}
