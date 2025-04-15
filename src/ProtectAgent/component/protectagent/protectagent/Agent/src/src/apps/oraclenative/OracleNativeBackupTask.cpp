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
#include "apps/oraclenative/OracleNativeBackupTask.h"

#include <algorithm>
#include <vector>
#include "common/Log.h"
#include "common/Utils.h"
#include "common/JsonUtils.h"
#include "message/tcp/CDppMessage.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "taskmanager/TaskStepPreSufScript.h"
#include "apps/oraclenative/TaskStepOracleNativeBackup.h"
#include "apps/oraclenative/TaskStepCheckDBOpen.h"

OracleNativeBackupTask::OracleNativeBackupTask(const mp_string& taskID) : OracleNativeTask(taskID)
{
    m_taskName = "OracleNativeBackupTask";
    // set backup type: data(1) or log(2)
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_BACKUP_MODE, backupMode);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_INFO, "Get backup mode failed.");
    }

    if (backupMode == 1) {
        operCmd = MANAGE_CMD_NO_ORACLE_BACKUP_DATA;
    } else {
        operCmd = MANAGE_CMD_NO_ORACLE_BACKUP_LOG;
    }
    CreateTaskStep();
}

OracleNativeBackupTask::~OracleNativeBackupTask()
{}

mp_int32 OracleNativeBackupTask::InitTaskStep(const Json::Value& param)
{
    LOGGUARD("");
    // check db open
    mp_int32 iRet = InitTaskStepParam(param, "", STEPNAME_CHECK_ORACLE_OPEN);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init check db status failed, ret=%d.", iRet);
        return iRet;
    }

    static const mp_string KEY_SCRIPTS = "scripts";
    // prefix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_PERSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init preScript failed, ret=%d.", iRet);
        return iRet;
    }

    // backup parameters
    iRet = InitTaskStepParam(param, "", STEPNAME_NATIVEBACKUP);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init backup parameters failed, ret=%d.", iRet);
        return iRet;
    }

    // suffix script
    iRet = InitTaskStepParam(param, KEY_SCRIPTS, STEPNAME_SUFSCRIPT);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init suffix script failed, ret=%d.", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*
step
1.checkdbstatus
2.exec pre script
3.exec backup
4.exec suf script
*/
mp_void OracleNativeBackupTask::CreateTaskStep()
{
    LOGGUARD("");
    mp_int32 iStepFive = 5;
    mp_int32 iStepEightyFive = 85;
    ADD_TASKSTEP(TaskStepCheckDBOpen, STEPNAME_CHECK_ORACLE_OPEN, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepPreScript, STEPNAME_PERSCRIPT, iStepFive, m_steps);
    ADD_TASKSTEP(TaskStepOracleNativeBackup, STEPNAME_NATIVEBACKUP, iStepEightyFive, m_steps);
    ADD_TASKSTEP(TaskStepPostScript, STEPNAME_SUFSCRIPT, iStepFive, m_steps);

    ADD_CLEARSTEP(STEPNAME_NATIVEBACKUP, TaskStepFailPostScript, STEPNAME_FAILEDSCRIPT, "scripts");
}

Task *OracleNativeBackupTask::CreateRedoTask(const mp_string& taskId)
{
    // get param from database
    taskinfo_t taskInfo;
    mp_int32 iRet = TaskManager::GetInstance()->QueryTaskInfo(taskId, "OracleNativeBackupTask", taskInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task %s get OracleNativeBackupTask info failed.", taskId.c_str());
        return NULL;
    }
    COMMLOG(OS_LOG_INFO, "Task %s get OracleNativeBackupTask info succ.", taskId.c_str());

    // set backup input params
    TaskContext::GetInstance()->SetJsonValue(taskId, KEY_INPUTPARAM, taskInfo.msgBody);
    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueString(taskId, KEY_CONNECTION_IP, taskInfo.connIp);
    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_CONNECTION_PORT, taskInfo.connPort);
    TaskContext::GetInstance()->SetValueString(taskId, g_CUR_RUNNING_STEP, taskInfo.subStep);
    TaskContext::GetInstance()->SetValueString(taskId, g_CUR_INNERPID, taskInfo.innerPID);
    TaskContext::GetInstance()->SetValueInt32(taskId, KEY_BACKUP_MODE, taskInfo.taskType);
    COMMLOG(OS_LOG_INFO, "Task %s set parameters succ.", taskId.c_str());

    OracleNativeBackupTask *task = new (std::nothrow) OracleNativeBackupTask(taskId);
    if (!task) {
        COMMLOG(OS_LOG_ERROR, "New task failed.");
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return NULL;
    }

    Json::Value params;
    iRet = CJsonUtils::ConvertStringtoJson(taskInfo.msgBody, params);
    if (iRet != MP_SUCCESS) {
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        delete task;
        return NULL;
    }

    if (!params.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "CreateRedoTask: dpp message string have no body, %s.", taskId.c_str());
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return NULL;
    }
    
    Json::Value bodyParam = params[MANAGECMD_KEY_BODY];
    task->InitTaskStep(bodyParam);
    if (iRet != MP_SUCCESS) {
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        delete task;
        return NULL;
    }
    
    COMMLOG(OS_LOG_INFO, "Create redo task %s succ.", taskId.c_str());
    return task;
}
