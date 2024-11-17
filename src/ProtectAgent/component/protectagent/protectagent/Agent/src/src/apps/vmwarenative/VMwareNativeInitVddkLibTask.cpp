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