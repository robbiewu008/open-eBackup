/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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