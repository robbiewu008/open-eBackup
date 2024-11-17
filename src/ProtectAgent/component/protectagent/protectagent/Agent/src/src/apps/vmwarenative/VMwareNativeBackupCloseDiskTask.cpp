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
