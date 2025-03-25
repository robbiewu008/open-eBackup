/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Task.cpp
 * @brief  The implemention about task operations
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "taskmanager/Task.h"

#include <vector>
#include <algorithm>
#include "common/ErrorCode.h"
#include "common/CMpThread.h"
#include "common/JsonUtils.h"
#include "taskmanager/TaskContext.h"
#include "taskmanager/TaskManager.h"
#include "securecom/CryptAlg.h"
#include "message/tcp/MessageHandler.h"

using namespace std;
namespace {
    const mp_int32 TASK_COMPLETE = 100;
}
Task::Task(const mp_string& taskID)
{
    m_taskId = taskID;
    m_taskName = "Task";
    m_taskStatus = STATUS_INITIAL;
    m_taskProgress = 0;
    m_taskExpiration = 0;
    m_taskSpeed = 0;
    m_backupSize = 0;
    m_ExitFlag = MP_FALSE;
    taskMode = ASYNCHRONOUS_TASK;

    m_disconnectNum = 0;
    m_finStepRation = 0;
    m_allStepRation = TASK_COMPLETE;
    errorCode = MP_SUCCESS;
    m_curStep = NULL;
    m_isReported = MP_FALSE;
    m_redoTask = MP_FALSE;
    (mp_void) memset_s(&m_threadId, sizeof(m_threadId), 0, sizeof(m_threadId));
    CMpThread::InitLock(&m_taskStepLock);
}

Task::~Task()
{
    if (m_threadId.os_id != 0) {
        CMpThread::WaitForEnd(&m_threadId, NULL);
    }
    for (vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
        if (*iter) {
            delete *iter;
        }
    }
    m_steps.clear();
    CMpThread::DestroyLock(&m_taskStepLock);
}

mp_int32 Task::InitTaskStep(const Json::Value& param)
{
    return MP_SUCCESS;
}

mp_int32 Task::RedoTaskStep(TaskStep* taskStep, mp_bool& executeFlag, mp_bool& continueRun,
    const mp_string& curRunningStep, mp_string& innerPID)
{
    mp_int32 iRet = MP_FALSE;
    continueRun = MP_FALSE;
    if (!executeFlag) {
        if (taskStep->GetStepName() == curRunningStep) {
            executeFlag = MP_TRUE;
            iRet = TaskManager::GetInstance()->UpdateSubStepTask(
                m_taskId, STATUS_INPROGRESS, taskStep->GetStepName(), m_taskName);
            if (iRet != MP_SUCCESS) {
                return ERROR_AGENT_INTERNAL_ERROR;
            }
            iRet = taskStep->Redo(innerPID);
        } else {
            continueRun = MP_TRUE;
        }
    } else {
        iRet = TaskManager::GetInstance()->UpdateSubStepTask(
            m_taskId, STATUS_INPROGRESS, taskStep->GetStepName(), m_taskName);
        if (iRet != MP_SUCCESS) {
            return ERROR_AGENT_INTERNAL_ERROR;
        }
        iRet = taskStep->Run();
    }
    CIPCFile::ReadResult(RESULT_ERRDETAIL_FILE + taskStep->GetInnerPID(), errDetails);
    CRootCaller rootCaller;
    rootCaller.RemoveFile(RESULT_ERRDETAIL_FILE + taskStep->GetInnerPID());

    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "Execute task %s(%s) failed, ret %d.",
            m_taskId.c_str(),
            taskStep->GetStepName().c_str(),
            iRet);
        m_taskStatus = STATUS_FAILED;
        // wgt update task status=failed
        mp_int32 iRet1 = TaskManager::GetInstance()->UpdateTaskStatus(m_taskId, STATUS_FAILED, m_taskName);
        if (iRet1 != MP_SUCCESS) {
            return ERROR_AGENT_INTERNAL_ERROR;
        }
        return iRet;
    }

    iRet = TaskManager::GetInstance()->UpdateSubStepTask(
        m_taskId, STATUS_COMPLETED, taskStep->GetStepName(), m_taskName);
    if (iRet != MP_SUCCESS) {
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    return MP_SUCCESS;
}

// current running taskstep
//      null: all will run
//      not null: before current running taskstep, continue
//      not null: current running taskstep, monitor script process, excute taskstep redo
//      not null: after current running taskstep, excute taskstep run
mp_int32 Task::Redo()
{
    mp_string curRunningStep;
    mp_string innerPID;
    TaskContext::GetInstance()->GetValueString(m_taskId, g_CUR_RUNNING_STEP, curRunningStep);
    TaskContext::GetInstance()->GetValueString(m_taskId, g_CUR_INNERPID, innerPID);

    RunTaskBefore();
    mp_bool executeFlag = MP_FALSE;
    m_taskStatus = STATUS_INPROGRESS;
    mp_int32 iRet = MP_FAILED;
    for (vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
        if (m_ExitFlag == MP_TRUE) {
            COMMLOG(OS_LOG_ERROR, "ExitFlag is configured, exit task running.");
            iRet = MP_FAILED;
            goto RUNAFTER;
        }

        if (*iter == NULL) {
            COMMLOG(OS_LOG_ERROR, "Unexpected exception, task step is NULL.");
            iRet = ERROR_AGENT_INTERNAL_ERROR;
            goto RUNAFTER;
        }

        ConfigCurrentStep(*iter);
        COMMLOG(OS_LOG_INFO, "task(%s) begin to run %s", m_taskId.c_str(), (*iter)->GetStepName().c_str());

        mp_bool continueRun = MP_FALSE;
        iRet = RedoTaskStep(*iter, executeFlag, continueRun, curRunningStep, innerPID);
        if (iRet != MP_SUCCESS) {
            goto RUNAFTER;
        }

        if (continueRun == MP_TRUE) {
            continue;
        }

        // set progress
        m_finStepRation += (*iter)->GetRatio();
        m_taskProgress = m_finStepRation;
    }

    COMMLOG(OS_LOG_DEBUG, "Rerun task(%s) finished", m_taskId.c_str());

    m_taskStatus = STATUS_COMPLETED;
    m_taskProgress = TASK_COMPLETE;
    ConfigCurrentStep(NULL);

    // update task completed
    iRet = TaskManager::GetInstance()->UpdateTaskStatus(m_taskId, STATUS_COMPLETED, m_taskName);
    if (iRet != MP_SUCCESS) {
        goto RUNAFTER;
    }

RUNAFTER:
    errorCode = iRet;
    TaskContext::GetInstance()->SetValueInt32(m_taskId, KEY_ERRCODE, errorCode);
    RunTaskAfter();
    return iRet;
}

mp_bool Task::GetRedoFlag()
{
    return m_redoTask;
}

void Task::PostSubStepWarnInfo()
{
    static const int SIZE_WARN_INFO = 2;
    std::vector<mp_string> tmpVec;
    CIPCFile::ReadResult(RESULT_WARNINFO_FILE, tmpVec);
    CRootCaller rootCaller;
    rootCaller.RemoveFile(RESULT_WARNINFO_FILE);
    for (vector<mp_string>::iterator iter = tmpVec.begin(); iter != tmpVec.end(); ++iter) {
        vector<mp_string> infoVec;
        CMpString::StrSplit(infoVec, *iter, CHAR_SEMICOLON);
        if (infoVec.size() == SIZE_WARN_INFO) {
            COMMLOG(OS_LOG_INFO, "sub step warn info,%s:%s", infoVec.front().c_str(), infoVec.back().c_str());
            std::pair<mp_string, mp_uint32> tempPair(infoVec.front(),
                ErrorCode::GetInstance().GetErrorCode(atoi(infoVec.back().c_str())));
            m_subStepWarnInfo.emplace_back(tempPair);
        }
    }
    if (!m_subStepWarnInfo.empty()) {
        Task::ReportTaskStatus(this);
        m_subStepWarnInfo.clear();
    }
}

mp_int32 Task::DoTaskStep(TaskStep* taskStep)
{
    COMMLOG(OS_LOG_INFO, "task(%s) begin to run %s.", m_taskId.c_str(), taskStep->GetStepName().c_str());
    // update task step status, status=running
    mp_int32 iRet = UpdateSubStepTask(
        m_taskId, STATUS_INPROGRESS, taskStep->GetStepName(), m_taskName);
    if (iRet != MP_SUCCESS) {
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    iRet = taskStep->Run();
    CIPCFile::ReadResult(RESULT_ERRDETAIL_FILE + taskStep->GetInnerPID(), errDetails);
    CRootCaller rootCaller;
    rootCaller.RemoveFile(RESULT_ERRDETAIL_FILE + taskStep->GetInnerPID());
    m_subStepWarnInfo = taskStep->GetWarnInfo();
    PostSubStepWarnInfo();
    if (iRet == MP_SUCCESS) {
        // update task step status, status=completed
        iRet = UpdateSubStepTask(
            m_taskId, STATUS_COMPLETED, taskStep->GetStepName(), m_taskName);
        if (iRet != MP_SUCCESS) {
            return ERROR_AGENT_INTERNAL_ERROR;
        }
        return MP_SUCCESS;
    }

    COMMLOG(OS_LOG_ERROR, "Execute task %s(%s) fail,ret %d.", m_taskId.c_str(), taskStep->GetStepName().c_str(), iRet);
    // run clear task step
    vector<TaskStep*> clearSteps = taskStep->GetClearSteps();
    if (clearSteps.size() > 0) {
        for (int i = 0; i < clearSteps.size(); ++i) {
            mp_int32 iRet1 = clearSteps[i]->Run();
            // just excute, don't exit
            mp_uint32 tempProgress = GetProgress();
            m_taskStatus = STATUS_INPROGRESS;
            mp_uint32 tempProgressValue = 99;
            m_taskProgress = (tempProgress < tempProgressValue) ? tempProgress : tempProgressValue;
            m_subStepWarnInfo = clearSteps[i]->GetWarnInfo();
            if (!m_subStepWarnInfo.empty()) {
                Task::ReportTaskStatus(this);
                m_subStepWarnInfo.clear();
            }
            m_taskProgress = tempProgress;
            m_taskStatus = STATUS_FAILED;
        }
    }

    // wgt update task status=failed
    mp_int32 iRet1 = UpdateTaskStatus(m_taskId, STATUS_FAILED, m_taskName);
    if (iRet1 != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "update task %s status failed, ret %d.", taskStep->GetStepName().c_str(), iRet1);
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    errorCode = iRet;
    TaskContext::GetInstance()->SetValueInt32(m_taskId, KEY_ERRCODE, errorCode);
    m_taskStatus = STATUS_FAILED;
    return iRet;
}

// 顺序执行任务中的步骤
mp_int32 Task::RunTask(Json::Value &respMsg)
{
    mp_int32 iRet = MP_FAILED;
    RunTaskBefore();

    m_taskStatus = STATUS_INPROGRESS;
    iRet = UpdateTaskStatus(m_taskId, m_taskStatus, m_taskName);
    if (iRet != MP_SUCCESS) {
        iRet = ERROR_AGENT_INTERNAL_ERROR;
        goto RUNAFTER;
    }
    for (vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
        if (m_ExitFlag == MP_TRUE) {
            COMMLOG(OS_LOG_ERROR, "ExitFlag is configured in runtask, exit task id: %s.", m_taskId.c_str());
            m_taskStatus = STATUS_ABORTED;
            iRet = MP_FAILED;
            goto RUNAFTER;
        }

        if (*iter == NULL) {
            COMMLOG(OS_LOG_ERROR, "Unexpected exception, task step is NULL.");
            iRet = ERROR_AGENT_INTERNAL_ERROR;
            m_taskStatus = STATUS_FAILED;
            goto RUNAFTER;
        }

        ConfigCurrentStep(*iter);

        iRet = DoTaskStep(*iter);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Excute task %s failed, iRet=%d.", (*iter)->GetStepName().c_str(), iRet);
            goto RUNAFTER;
        }

        Json::Value respMsgTmp;
        (*iter)->GetRespMsg(respMsgTmp);
        if (!respMsgTmp.empty()) {
            respMsg = std::move(respMsgTmp);
        }

        // set progress
        m_finStepRation += (*iter)->GetRatio();
        m_taskProgress = m_finStepRation;
    }
    COMMLOG(OS_LOG_DEBUG, "run task(%s) finished", m_taskId.c_str());

    m_taskStatus = STATUS_COMPLETED;
    m_taskProgress = TASK_COMPLETE;
    ConfigCurrentStep(NULL);

    // update task completed
    iRet = UpdateTaskStatus(m_taskId, STATUS_COMPLETED, m_taskName);
    if (iRet != MP_SUCCESS) {
        iRet = ERROR_AGENT_INTERNAL_ERROR;
        goto RUNAFTER;
    }

RUNAFTER:
    errorCode = iRet;
    RunTaskAfter();
    return iRet;
}

mp_int32 Task::RunSyncTask()
{
    // check
    for (vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
        TaskStep* taskStep = *iter;
        COMMLOG(OS_LOG_INFO, "task(%s) begin to run %s", m_taskId.c_str(), taskStep->GetStepName().c_str());
        mp_int32 iRet = taskStep->Run();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR,
                "Execute sync task %s(%s) failed, ret %d.",
                taskStep->GetStepName().c_str(),
                taskStep->GetStepId().c_str(),
                iRet);
            TaskContext::GetInstance()->RemoveTaskContext(m_taskId);
            return iRet;
        }
    }
    TaskContext::GetInstance()->RemoveTaskContext(m_taskId);
    COMMLOG(OS_LOG_DEBUG, "run task(%s) finished", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 Task::CancelTask()
{
    COMMLOG(OS_LOG_INFO, "begin to cancel task %s.", m_taskId.c_str());
    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    if (m_taskStatus != STATUS_INPROGRESS) {
        COMMLOG(OS_LOG_ERROR, "task status is not running.");
        return MP_FAILED;
    }

    // set exit flag
    m_ExitFlag = MP_TRUE;

    // begin to cancel
    m_taskStatus = STATUS_FAILED;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Cancel();
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Cancel taskstep failed, iRet=%d.", iRet);
            return MP_FAILED;
        }
        m_taskStatus = STATUS_FAILED;
    } else {
        m_taskStatus = STATUS_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "cancel task %s succ.", m_taskId.c_str());
    m_taskProgress = TASK_COMPLETE;
    return MP_SUCCESS;
}

mp_int32 Task::CancelTask(Json::Value &respParam)
{
    COMMLOG(OS_LOG_INFO, "begin to cancel task %s.", m_taskId.c_str());
    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    if (m_taskStatus != STATUS_INPROGRESS) {
        COMMLOG(OS_LOG_ERROR, "task status is not running.");
        return MP_FAILED;
    }

    // set exit flag
    m_ExitFlag = MP_TRUE;

    // begin to cancel
    m_taskStatus = STATUS_FAILED;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Cancel(respParam);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Cancel taskstep failed, iRet=%d.", iRet);
            return MP_FAILED;
        }
        m_taskStatus = STATUS_FAILED;
    } else {
        m_taskStatus = STATUS_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "cancel task %s succ.", m_taskId.c_str());
    m_taskProgress = TASK_COMPLETE;
    return MP_SUCCESS;
}

mp_int32 Task::RefreshTaskInfo()
{
    static const mp_int32 DONO_RATIO = 100;

    if (m_taskStatus == STATUS_INPROGRESS) {
        CMpThread::Lock(&m_taskStepLock);
        TaskStep* taskStep = m_curStep;
        CMpThread::Unlock(&m_taskStepLock);

        // if task is running, need to count progress by task step
        if (taskStep != NULL) {
            mp_int32 iRet = taskStep->RefreshStepInfo();
            if (iRet != MP_SUCCESS) {
                COMMLOG(OS_LOG_WARN,
                    "task(%s) step(%s) Refresh StepInfo failed, ret is %d",
                    m_taskId.c_str(),
                    taskStep->GetStepName().c_str(),
                    iRet);
                return iRet;
            }
            mp_int32 tmpProgress = (m_finStepRation + (taskStep->GetProgress() * taskStep->GetRatio()) / DONO_RATIO);
            tmpProgress = (tmpProgress > DONO_RATIO) ? DONO_RATIO : tmpProgress;
            COMMLOG(OS_LOG_DEBUG, "taskstep(%s) get progress is %d", taskStep->GetStepName().c_str(), tmpProgress);
            // 防止出现进度变小的情况
            m_taskProgress = (tmpProgress < m_taskProgress) ? m_taskProgress : tmpProgress;
            m_taskSpeed = taskStep->GetSpeed();
            m_backupSize = taskStep->GetBackupSize();
        } else {
            // 理论上不会出现该场景
            COMMLOG(OS_LOG_WARN, "cancel task %s succ.", m_taskId.c_str());
        }
    } else {
        m_taskSpeed = 0;
    }

    return MP_SUCCESS;
}

mp_int32 Task::UpdateTask(const Json::Value& param)
{
    COMMLOG(OS_LOG_INFO, "begin to update task %s.", m_taskId.c_str());
    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    // begin to udpate
    m_taskStatus = STATUS_INPROGRESS;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Update(param);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "update taskstep failed, iRet=%d.", iRet);
            return iRet;
        }
    }
    m_taskStatus = STATUS_INPROGRESS;

    COMMLOG(OS_LOG_INFO, "update task %s succ.", m_taskId.c_str());
    m_taskProgress = TASK_COMPLETE;
    return MP_SUCCESS;
}
mp_int32 Task::UpdateTask(Json::Value& param, Json::Value& respBody)
{
    COMMLOG(OS_LOG_INFO, "begin to update task %s.", m_taskId.c_str());
    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    // begin to udpate
    m_taskStatus = STATUS_INPROGRESS;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Update(param, respBody);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "update taskstep failed, iRet=%d.", iRet);
            return iRet;
        }
    }
    m_taskStatus = STATUS_INPROGRESS;

    COMMLOG(OS_LOG_INFO, "update task %s succ.", m_taskId.c_str());
    m_taskProgress = TASK_COMPLETE;
    return MP_SUCCESS;
}

mp_int32 Task::FinishTask(const Json::Value& param)
{
    COMMLOG(OS_LOG_INFO, "begin to finish task %s, task status %d.", m_taskId.c_str(), m_taskStatus);

    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    if (m_taskStatus != STATUS_INPROGRESS) {
        COMMLOG(OS_LOG_ERROR, "task status is not running.");
        return MP_FAILED;
    }

    // set exit flag
    m_ExitFlag = MP_TRUE;

    // begin to finish
    m_taskStatus = STATUS_DELETING;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Finish(param);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "finish taskstep failed, iRet=%d.", iRet);
            return MP_FAILED;
        }
    }

    m_taskStatus = STATUS_COMPLETED;
    m_taskProgress = TASK_COMPLETE;
    COMMLOG(OS_LOG_INFO, "finish task %s succ.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 Task::FinishTask(Json::Value& param, Json::Value& respParam)
{
    COMMLOG(OS_LOG_INFO, "begin to finish task %s, task status %d.", m_taskId.c_str(), m_taskStatus);

    // check status
    if (m_taskStatus == STATUS_DELETING) {
        COMMLOG(OS_LOG_ERROR, "task status is deleting.");
        return MP_FAILED;
    }

    if (m_taskStatus != STATUS_INPROGRESS) {
        COMMLOG(OS_LOG_ERROR, "task status is not running.");
        return MP_FAILED;
    }

    // set exit flag
    m_ExitFlag = MP_TRUE;

    // begin to finish
    m_taskStatus = STATUS_DELETING;

    if (m_curStep != NULL) {
        mp_int32 iRet = m_curStep->Finish(param, respParam);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "finish taskstep failed, iRet=%d.", iRet);
            return iRet;
        }
    }

    m_taskStatus = STATUS_COMPLETED;
    m_taskProgress = TASK_COMPLETE;
    COMMLOG(OS_LOG_INFO, "finish task %s succ.", m_taskId.c_str());
    return MP_SUCCESS;
}

mp_string Task::GetTaskID()
{
    return m_taskId;
}

mp_string Task::GetTaskName()
{
    return m_taskName;
}

mp_int32 Task::GetProgress()
{
    return m_taskProgress;
}

TaskStatus Task::GetStatus()
{
    return m_taskStatus;
}

mp_int32 Task::GetSpeed()
{
    return m_taskSpeed;
}

mp_int32 Task::GetBackupSize()
{
    return m_backupSize;
}

void Task::SetExpiration(mp_int64 expiration)
{
    m_taskExpiration = expiration;
}

mp_int64 Task::GetExpiration()
{
    return m_taskExpiration;
}

thread_id_t Task::GetThreadID()
{
    return m_threadId;
}

mp_void Task::SetThreadID(thread_id_t threadId)
{
    m_threadId = threadId;
}

TaskMode Task::GetTaskMode()
{
    return taskMode;
}

mp_int32 Task::GetErrCode()
{
    return errorCode;
}

std::vector<mp_string> Task::GetErrParams()
{
    return errParams;
}

std::vector<mp_string> Task::GetErrDetails()
{
    return errDetails;
}

mp_int32 Task::InitTaskStepParam(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName)
{
    TaskStep* step = NULL;
    for (vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
        if (stepName == (*iter)->GetStepName()) {
            step = *iter;
        }
    }

    if (step == NULL) {
        COMMLOG(OS_LOG_ERROR, "task %s have no task step %s.", this->m_taskName.c_str(), stepName.c_str());
        return MP_FAILED;
    }

    Json::Value stepParam = param;
    if (!paramKey.empty()) {
        if (!param.isObject() || !param.isMember(paramKey)) {
            COMMLOG(OS_LOG_ERROR, "param have no scsi key %s.", paramKey.c_str());
            return MP_FAILED;
        } else {
            stepParam = param[paramKey];
        }
    }

    mp_int32 iRet = step->Init(stepParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "task step %s(%s) initialize %s failed, iRet %d.",
            step->GetStepName().c_str(),
            step->GetStepId().c_str(),
            paramKey.c_str(),
            iRet);
        return iRet;
    }

    // extend task isn't null, need to init extend param
    if (step->GetClearSteps().size() == 0) {
        return MP_SUCCESS;
    }

    vector<TaskStep*> &exStep = step->GetClearSteps();
    for (int i = 0; i < exStep.size(); ++i) {
        mp_string paramKey = exStep[i]->GetParamKey();
        mp_bool flag = !paramKey.empty() && stepParam.isObject() && stepParam.isMember(paramKey);
        if (flag) {
            iRet = exStep[i]->Init(stepParam[paramKey]);
        } else {
            iRet = exStep[i]->Init(stepParam);
        }
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "clear step %s failed, iRet %d.", exStep[i]->GetStepName().c_str(), iRet);
            COMMLOG(OS_LOG_ERROR, "step id(%s) param %s", exStep[i]->GetStepId().c_str(), paramKey.c_str());
            return iRet;
        }
    }
    return MP_SUCCESS;
}

mp_bool Task::IsCompleted()
{
    static const TaskStatus COMPLETED_STATUS[] = {STATUS_COMPLETED, STATUS_FAILED, STATUS_DELETED, STATUS_ABORTED};
    // 任务结束
    mp_int32 idx = sizeof(COMPLETED_STATUS) / sizeof(TaskStatus);
    const TaskStatus* findStatus = std::find(COMPLETED_STATUS, COMPLETED_STATUS + idx, m_taskStatus);
    return (findStatus != COMPLETED_STATUS + idx);
}

mp_void Task::SetReportStatus()
{
    m_isReported = MP_TRUE;
}

mp_bool Task::GetReportStatus()
{
    return m_isReported;
}

mp_void Task::UpdateDelTime()
{
    CMpTime::Now(m_delTime);
}

mp_time Task::GetDelTime()
{
    return m_delTime;
}

mp_int32 Task::GetDisconnectNum()
{
    return m_disconnectNum;
}

void Task::setDisconnectNum(mp_int32 num)
{
    m_disconnectNum = num;
}

std::vector<std::pair<mp_string, mp_uint32> > &Task::GetSubWarnInfo()
{
    return m_subStepWarnInfo;
}

void Task::SetSubWarnInfo(const std::vector<std::pair<mp_string, mp_uint32> > &logLabel)
{
    m_subStepWarnInfo = logLabel;
}

void Task::SetTaskResult(const Json::Value& result)
{
    m_taskResult = result;
}

Json::Value Task::GetTaskResult()
{
    return m_taskResult;
}

mp_int32 Task::ReportTaskStatus(Task* task)
{
    if (task == NULL) {
        COMMLOG(OS_LOG_ERROR, "task is null.");
        return MP_FAILED;
    }

    if (task->GetReportStatus() == MP_TRUE) {
        COMMLOG(OS_LOG_INFO, "task %s have reported status, don't report again.", task->GetTaskID().c_str());
        return MP_SUCCESS;
    }

    mp_string connIp = "";
    mp_uint16 connPort = 0;
    COMMLOG(OS_LOG_DEBUG, "task %s begin to report task details", task->GetTaskID().c_str());

    // connection ip address: reporting data
    mp_int32 iRet = TaskContext::GetInstance()->GetValueString(task->GetTaskID(), KEY_CONNECTION_IP, connIp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "task context %s have not key %s value.",
            task->GetTaskID().c_str(),
            KEY_CONNECTION_IP.c_str());
    }

    // connection ip port: reporting data
    mp_uint32 uiTmp = 0;
    iRet = TaskContext::GetInstance()->GetValueUInt32(task->GetTaskID(), KEY_CONNECTION_PORT, uiTmp);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR,
            "task context %s have not key %s value.",
            task->GetTaskID().c_str(),
            KEY_CONNECTION_PORT.c_str());
    }
    connPort = static_cast<mp_uint16>(uiTmp);

    return ConstructAndSendMsg(task, connIp, connPort);
}

mp_int32 Task::ConstructAndSendMsg(Task* task, const mp_string &connIp, mp_uint16 connPort)
{
    // 上报的消息放到response的消息中
    CDppMessage* rspMsg = NULL;
    NEW_CATCH_RETURN_FAILED(rspMsg, CDppMessage);
    rspMsg->InitMsgHead(MSG_DATA_TYPE_MANAGE, 0, 0);
    rspMsg->SetMsgSrc(ROLE_HOST_AGENT);
    rspMsg->SetMsgTgt(ROLE_EBK_APP);
    rspMsg->SetLinkInfo(connIp, connPort);

    Json::Value msgVal;
    msgVal[MANAGECMD_KEY_CMDNO] = MANAGE_CMD_NO_TASK_REPORT_PROGRESS;
    Json::Value msgBody;
    msgBody[PARAM_KEY_TASKID] = task->GetTaskID();
    msgBody[PARAM_KEY_TASKSTATUS] = task->GetStatus();
    msgBody[PARAM_KEY_TASKDESC] = "";
    msgBody[PARAM_KEY_TASKSPEED] = task->GetSpeed();
    msgBody[PARAM_KEY_TASKPROGRESS] = task->GetProgress();
    msgBody[PARAM_KEY_BACKUP_SIZE] = task->GetBackupSize();
    std::vector<mp_string> errParams = task->GetErrParams();
    for (std::vector<mp_string>::iterator iter = errParams.begin(); iter != errParams.end(); ++iter) {
        msgBody[PARAM_KEY_LOGPARAMS].append(*iter);
    }
    if (msgBody[PARAM_KEY_TASKSTATUS] == STATUS_FAILED) {
        std::vector<mp_string> errDetails = task->GetErrDetails();
        for (std::vector<mp_string>::iterator iter = errDetails.begin(); iter != errDetails.end(); ++iter) {
            msgBody[PARAM_KEY_LOGDETAILINFO].append(*iter);
        }
    }
    Json::Value taskResult;
    if (TaskContext::GetInstance()->GetValueJson(task->GetTaskID(), KEY_TASK_RESULT, taskResult) == MP_SUCCESS) {
        msgBody[PARAM_KEY_TASKRESULT] = std::move(taskResult);
    }

    std::vector<std::pair<mp_string, mp_uint32> > tempLogLabel = task->GetSubWarnInfo();
    if (tempLogLabel.empty()) {
        std::pair<mp_string, mp_uint32> tempPair("", task->GetErrCode());
        tempLogLabel.emplace_back(tempPair);
    }

    for (size_t i = 0; i < tempLogLabel.size(); i++) {
        msgBody[PARAM_KEY_LOGLABEL] = tempLogLabel[i].first;
        msgBody[PARAM_KEY_LOGDETAIL] = tempLogLabel[i].second;
        msgVal[MANAGECMD_KEY_BODY] = std::move(msgBody);
        rspMsg->SetMsgBody(msgVal);
        if (!SendStatusMsg(rspMsg)) {
            delete rspMsg;
            return MP_FAILED;
        }
    }
    if (rspMsg) {
        delete rspMsg;
    }
    return MP_SUCCESS;
}

mp_bool Task::SendStatusMsg(CDppMessage *rspMsg)
{
    CDppMessage* reqMsgFirst = NULL;
    CDppMessage* reqMsgSecond = NULL;
    NEW_CATCH(reqMsgFirst, CDppMessage);
    NEW_CATCH(reqMsgSecond, CDppMessage);
    if (!reqMsgFirst || !reqMsgSecond) {
        COMMLOG(OS_LOG_ERROR, "New CDppMessage failed");
        return MP_FALSE;
    }

    reqMsgFirst->CloneMsg(*rspMsg);
    (*reqMsgSecond) = (*rspMsg);
    message_pair_t msgPair(*reqMsgFirst, *reqMsgSecond);
    MessageHandler::GetInstance().PushRspMsg(msgPair);
    return MP_TRUE;
}

mp_int32 Task::UpdateTaskStatus(mp_string taskId, mp_int32 status, mp_string step)
{
    if (GetRedoFlag() == MP_TRUE) {
        return TaskManager::GetInstance()->UpdateTaskStatus(m_taskId, status, m_taskName);
    }
    return MP_SUCCESS;
}

mp_int32 Task::UpdateSubStepTask(const mp_string& taskId,
    mp_int32 subStepStatus, const mp_string& subStep, const mp_string& taskName)
{
    if (GetRedoFlag() == MP_TRUE) {
        return TaskManager::GetInstance()->UpdateSubStepTask(taskId, subStepStatus, subStep, taskName);
    }
    return MP_SUCCESS;
}