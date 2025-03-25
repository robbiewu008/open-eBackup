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
#include "apps/oraclenative/OracleNativeCLiveMTask.h"
#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oraclenative/TaskStepOracleNativeCLiveMount.h"
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"

OracleNativeCLiveMTask::OracleNativeCLiveMTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeCLiveMTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeCLiveMTask::~OracleNativeCLiveMTask()
{}

mp_int32 OracleNativeCLiveMTask::InitTaskStep(const Json::Value& param)
{
    // stop database
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CANCEL_LIVEMOUNT);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    iRet = InitTaskStepParam(param, "", STEPNAME_ORACLE_NATIVEDISMOUNT);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    return MP_SUCCESS;
}

/*
cancel database LiveMount
1.close database
2.cancel database LiveMount backup Media
1)fs
umount fs
deactive VG
export VG
2)ASM
dismount asm diskgroup
remove raw device
*/
mp_void OracleNativeCLiveMTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFivty = 50;
    ADD_TASKSTEP(TaskStepOracleNativeCLiveMount, STEPNAME_CANCEL_LIVEMOUNT, iStepFivty, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeDismount, STEPNAME_ORACLE_NATIVEDISMOUNT, iStepFivty, m_steps);
}
