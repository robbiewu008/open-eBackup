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
#include "apps/oraclenative/OracleNativeRestoreTask.h"

#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskStepPreSufScript.h"
#include "apps/oraclenative/TaskStepCheckDBClose.h"
#include "apps/oraclenative/TaskStepClearDataBase.h"
#include "apps/oraclenative/TaskStepOracleNativeRestore.h"

OracleNativeRestoreTask::OracleNativeRestoreTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeRestoreTask";
    statusFlag = MP_FALSE;
    CreateTaskStep();
}

OracleNativeRestoreTask::~OracleNativeRestoreTask()
{}

mp_int32 OracleNativeRestoreTask::InitTaskStep(const Json::Value& param)
{
    // check database valid
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CHECK_DBCLOSE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init close DB failed, iRet %d.", iRet);
        return iRet;
    }

    // prefix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_PERSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init prescript failed, iRet %d.", iRet);
        return iRet;
    }

    // rman
    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVERESTORE);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init restore failed, iRet %d.", iRet);
        return iRet;
    }

    // suffix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_SUFSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativeInstRestoreTask init suff script failed, iRet %d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*
step
1.check database status
2.exec pre script
3.exec restore
4.exec suf script
*/
mp_void OracleNativeRestoreTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFive = 5;
    mp_int32 iStepEightyFive = 85;
    ADD_TASKSTEP(TaskStepCheckDBClose, STEPNAME_CHECK_DBCLOSE, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepPreScript, STEPNAME_PERSCRIPT, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeRestore, STEPNAME_NATIVERESTORE, iStepEightyFive, m_steps);
    ADD_TASKSTEP(TaskStepPostScript, STEPNAME_SUFSCRIPT, iStepFive, m_steps);

    ADD_CLEARSTEP(STEPNAME_NATIVERESTORE, TaskStepClearDataBase, STEPNAME_CLEAR_DATABASE, "");
    ADD_CLEARSTEP(STEPNAME_NATIVERESTORE, TaskStepFailPostScript, STEPNAME_FAILEDSCRIPT, "scripts");
}
