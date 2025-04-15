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
#include "apps/vmwarenative/VMwareNativeVmRestoreTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "apps/vmwarenative/TaskStepRestoreDataBlock.h"
#include "apps/vmwarenative/TaskStepPrepareTargetLun.h"

#include "common/JsonUtils.h"
#include "common/Utils.h"

using namespace std;

VMwareNativeVmRestoreTask::VMwareNativeVmRestoreTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeVmRestoreTask";
    // obtain backend storage type of current task: 0-nas, 1-iscsi
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_STORAGE_PROTOCOL, m_iStorageProtocol);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Task(%s) get backend storage type failed.", m_taskId.c_str());
    }
    CreateTaskStep();
}

VMwareNativeVmRestoreTask::~VMwareNativeVmRestoreTask()
{}

mp_int32 VMwareNativeVmRestoreTask::InitTaskStep(const Json::Value &param)
{
    // trigger target lun discovery task step
    if (m_iStorageProtocol == VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI) {
        if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_PREPARE_TARGETLUN)) {
            ERRLOG("Init target lun path failed, task id '%s'.", m_taskId.c_str());
            return MP_FAILED;
        }
    }

    // trigger vm disk Restore task step
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_RESTORE_DATABLOCK)) {
        ERRLOG("Unable to init disk data block restore, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    INFOLOG("Init disk data block restore parameters successfully, task id '%s'.", m_taskId.c_str());

    return MP_SUCCESS;
}

mp_void VMwareNativeVmRestoreTask::CreateTaskStep()
{
    if (m_iStorageProtocol == VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI) {
        ADD_TASKSTEP(TaskStepPrepareTargetLun, STEPNAME_PREPARE_TARGETLUN, TASK_STEP_INTERVAL_TEN, m_steps);
    }
    ADD_TASKSTEP(TaskStepRestoreDataBlock, STEPNAME_RESTORE_DATABLOCK, TASK_STEP_INTERVAL_NINETY, m_steps);
}
