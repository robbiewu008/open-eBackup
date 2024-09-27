/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file OracleNativeTask.cpp
 * @brief  Contains function declarations OracleNativeTask
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/oraclenative/OracleNativeTask.h"
#include "common/ConfigXmlParse.h"

#include <algorithm>
#include <vector>

#include "common/Utils.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/MessageHandler.h"
#include "taskmanager/TaskContext.h"

namespace {
const mp_int32 SLEEP_TIME_REPORT = 30000;
}
OracleNativeTask::OracleNativeTask(const mp_string& taskID) : Task(taskID)
{
    (mp_void)memset_s(&statusTid, sizeof(statusTid), 0, sizeof(statusTid));
    statusFlag = MP_FALSE;
    // oracle task support redo
    m_redoTask = MP_TRUE;

    if (MP_SUCCESS != CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_BACKUP_SECTION, CFG_PROGRESS_INTERVAL, m_progressInterval)) {
        COMMLOG(OS_LOG_ERROR, "get Progress Report Interval failed, set Default value %d.", SLEEP_TIME_REPORT);
        m_progressInterval = SLEEP_TIME_REPORT;
    }
}

OracleNativeTask::~OracleNativeTask()
{}

mp_void OracleNativeTask::RunTaskBefore()
{
    // start thread to get task progess
    COMMLOG(OS_LOG_DEBUG, "Task %s taskMode is %d.", m_taskId.c_str(), taskMode);
    if (taskMode != SYNCHRONOUS_TASK) {
        mp_int32 iRet = CMpThread::Create(&statusTid, RunGetProgressTask, (mp_void*)this);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Create get status and process thread failed, ret %d.", iRet);
        }
        COMMLOG(OS_LOG_DEBUG, "Start task %s thread %u to get status and progress.", m_taskId.c_str(), statusTid.os_id);
    }
}

mp_void OracleNativeTask::RunTaskAfter()
{
    // stop getting task progess thread
    statusFlag = MP_TRUE;
    if (taskMode != SYNCHRONOUS_TASK) {
        m_curStep = NULL;
        mp_int32 iRet = Task::ReportTaskStatus(this);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "report task %s status and progress failed, iRet %d.", m_taskId.c_str(), iRet);
        }
        SetReportStatus();
        iRet = CMpThread::WaitForEnd(&statusTid, NULL);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Task %s wait thread %u failed.", m_taskId.c_str(), statusTid.os_id);
        }
        COMMLOG(OS_LOG_DEBUG, "Task %s thread %u have stop.", m_taskId.c_str(), statusTid.os_id);
    }
    TaskContext::GetInstance()->RemoveTaskContext(m_taskId);
}

#ifdef WIN32
DWORD WINAPI OracleNativeTask::RunGetProgressTask(mp_void* pThis)
#else
mp_void* OracleNativeTask::RunGetProgressTask(mp_void* pThis)
#endif
{
    if (!pThis) {
        COMMLOG(OS_LOG_ERROR, "cast to OracleNativeTask pointer failed.");
        CMPTHREAD_RETURN;
    }

    OracleNativeTask* backupTask = static_cast<OracleNativeTask*>(pThis);
    if (backupTask != NULL) {
        COMMLOG(OS_LOG_INFO, "Begin to get OracleNativeTask %s in thread.", backupTask->GetTaskID().c_str());
        while (MP_TRUE) {
            if (backupTask->GetStatusFlag()) {
                COMMLOG(OS_LOG_INFO, "statusFlag is configured, exit get status task.");
                break;
            }

            mp_int32 iRet = backupTask->RefreshTaskInfo();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR,
                    "report task %s status and progress failed, iRet %d.",
                    backupTask->GetTaskID().c_str(), iRet);
            }

            // 上报进度
            iRet = Task::ReportTaskStatus(backupTask);
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR,
                    "report task %s status and progress failed, iRet %d.",
                    backupTask->GetTaskID().c_str(), iRet);
            }

            // if task status is belong to completed status, need to break query progress
            if (backupTask->IsCompleted() == MP_TRUE) {
                COMMLOG(OS_LOG_INFO,
                    "task %s is complete, status %d, exit get status and progess.",
                    backupTask->GetTaskID().c_str(), backupTask->GetStatus());
                backupTask->SetReportStatus();
                break;
            }

            DoSleep(backupTask->GetProgressInterval());
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "cast to OracleNativeTask point failed.");
    }

    CMPTHREAD_RETURN;
}

mp_bool OracleNativeTask::GetStatusFlag()
{
    return statusFlag;
}

