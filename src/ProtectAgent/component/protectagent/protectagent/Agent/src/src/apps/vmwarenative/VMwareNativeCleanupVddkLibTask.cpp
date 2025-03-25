/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeCleanupVddkLibTask.cpp
 * @author w00558987
 * @brief 备份/恢复任务Cleanup VDDK
 * @version 0.1
 * @date 2021-01-16
 *
 */

#include "apps/vmwarenative/VMwareNativeCleanupVddkLibTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "apps/vmwarenative/TaskStepCleanUpVMwareDiskLib.h"

VMwareNativeCleanupVddkLibTask::VMwareNativeCleanupVddkLibTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeCleanupVddkLibTask";
    CreateTaskStep();
}

VMwareNativeCleanupVddkLibTask::~VMwareNativeCleanupVddkLibTask()
{}

mp_int32 VMwareNativeCleanupVddkLibTask::InitTaskStep(const Json::Value &param)
{
    mp_int32 iRet = MP_FAILED;

    // trigger VDDK lib init task step
    iRet = InitTaskStepParam(param, "", STEPNAME_CLEANUP_VMWAREDISKLIB);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Cleanup VDDK lib failed, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    return iRet;
}

mp_void VMwareNativeCleanupVddkLibTask::CreateTaskStep()
{
    ADD_TASKSTEP(TaskStepCleanUpVMwareDiskLib, STEPNAME_CLEANUP_VMWAREDISKLIB, TASK_STEP_INTERVAL_HUNDERED, m_steps);
}