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
#include "apps/oraclenative/OracleNativePrepareMediaTask.h"

#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "apps/oracle/OracleDefines.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskStepScanDisk.h"
#include "apps/oraclenative/TaskStepOracleNativeNasMedia.h"
#include "apps/oraclenative/TaskStepOracleNativeMediaData.h"
#include "apps/oraclenative/TaskStepOracleNativeMediaLog.h"

OracleNativePrepareMediaTask::OracleNativePrepareMediaTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativePrepareMediaTask";
    // get tasktype: 1-databackup, 2-logbackup, 3-restore, 4-livemount, 5-instance recovery, 6-delete
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_TASKTYPE, taskType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task(%s) Get taskType failed.", m_taskId.c_str());
    }
    // storType: 0-nas, 1-iscsi
    iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_STORAGE_TYPE, storType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task(%s) Get storType failed.", m_taskId.c_str());
    }

    statusFlag = MP_FALSE;
    CreateTaskStep();
    taskMode = SYNCHRONOUS_TASK;
}

OracleNativePrepareMediaTask::~OracleNativePrepareMediaTask()
{}

mp_int32 OracleNativePrepareMediaTask::InitTaskStep(const Json::Value& param)
{
    LOGGUARD("");
    mp_int32 iRet = 0;
    iRet = InitTaskStepParam(param, "", STEPNAME_PREPARE_ORACLENASMEDIA);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "OracleNativePrepareMediaTask init media failed, iRet %d.", iRet);
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 OracleNativePrepareMediaTask::InitPrepareMedia(const Json::Value& param)
{
    mp_int32 iRet = 0;
    // prepare backup data media
    if ((taskType != ORACLENATIVE_TASK_LOG_BACKUP) && (taskType != ORACLENATIVE_TASK_DELETE)) {
        iRet = InitTaskStepParam(param, "", STEPNAME_PREPAREMEDIA_DATA);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Init TaskStepOracleNativeMedia Data failed, ret=%d.", iRet);
            return iRet;
        }
    }

    // prepare backup log media
    iRet = InitTaskStepParam(param, "", STEPNAME_PREPAREMEDIA_LOG);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init TaskStepOracleNativeMedia Log failed, ret=%d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

mp_void OracleNativePrepareMediaTask::CreateTaskStep()
{
    LOGGUARD("");
    if ((storType == ORA_STORTYPE_ISCSI) || (storType == ORA_STORTYPE_FC)) {
        return;
    } else {
        mp_int32 iNum = 100;
        ADD_TASKSTEP(TaskStepOracleNativeNasMedia, STEPNAME_PREPARE_ORACLENASMEDIA, iNum, m_steps);
    }
}
