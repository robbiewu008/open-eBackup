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
#include "taskmanager/TaskManager.h"
#include "common/ConfigXmlParse.h"
#include "common/CMpThread.h"
#include "common/Utils.h"
#include "taskmanager/TaskContext.h"
#include "common/DB.h"
#include "securecom/CryptAlg.h"
#include "taskmanager/TaskRedoFuncContainer.h"

#if (defined LINUX) && (!defined ENABLE_TSAN)
#include <gperftools/malloc_extension.h>
#endif

TaskManager* TaskManager::m_pTaskManager = NULL;
using std::ostringstream;
using std::vector;

TaskManager* TaskManager::GetInstance()
{
    if (NULL != m_pTaskManager) {
        return m_pTaskManager;
    }

    m_pTaskManager = new (std::nothrow) TaskManager();
    if (NULL == m_pTaskManager) {
        COMMLOG(OS_LOG_ERROR, "new TaskManager failed");
        return NULL;
    }
    COMMLOG(OS_LOG_DEBUG, "thread pool need to be initialized");
    return m_pTaskManager;
}

TaskManager::TaskManager()
{
    CMpThread::InitLock(&m_mapLock);
    mp_int32 threadSize = 1000;
    InitThreadPool(threadSize);
}

TaskManager::~TaskManager()
{
    CMpThread::DestroyLock(&m_mapLock);

    vector<Task*>::iterator iter = m_removeTasks.begin();
    while (iter != m_removeTasks.end()) {
        if (*iter != NULL) {
            delete *iter;
        }
        iter = m_removeTasks.erase(iter);
    }
}

mp_void TaskManager::InitThreadPool(mp_int32 threadPoolSize)
{
    m_threadPoolMax = (threadPoolSize > 0) ? threadPoolSize : THREADPOOL_MAX;
    COMMLOG(OS_LOG_INFO, "thread pool size is %d", m_threadPoolMax);
}

mp_int32 TaskManager::AddTask(const mp_string& taskID, Task* pTask)
{
    CThreadAutoLock tlock(&m_mapLock);
    if (ThreadNumCheck() == MP_FAILED) {
        COMMLOG(OS_LOG_ERROR, "threadpool is full, can not add task");
        return MP_FAILED;
    }

    std::map<mp_string, Task*>::iterator iter = m_taskMap.find(taskID);
    if (iter != m_taskMap.end()) {
        COMMLOG(OS_LOG_ERROR, "taskID is duplicated, can not add taskID %s", taskID.c_str());
        return MP_FAILED;
    }
    m_taskMap.insert(std::pair<mp_string, Task*>(taskID, pTask));
    COMMLOG(OS_LOG_INFO, "Add task %s successfully.", taskID.c_str());
    return MP_SUCCESS;
}

mp_void TaskManager::FindTask(const mp_string& taskID, Task*& pTask)
{
    CThreadAutoLock tlock(&m_mapLock);
    std::map<mp_string, Task*>::iterator iter = m_taskMap.find(taskID);
    if (iter == m_taskMap.end()) {
        pTask = NULL;
    } else {
        pTask = iter->second;
    }
}

Task* TaskManager::FindTaskEx(const mp_string& taskID, const mp_string& taskName)
{
    CThreadAutoLock tlock(&m_mapLock);
    std::map<mp_string, Task*>::iterator iterMap = m_taskMap.find(taskID);
    if (iterMap != m_taskMap.end()) {
        COMMLOG(OS_LOG_INFO, "taskID: %s, taskName: %s",
            iterMap->second->GetTaskID().c_str(), iterMap->second->GetTaskName().c_str());
        if (iterMap->second->GetTaskName() == taskName) {
            return iterMap->second;
        }
    }
    for (vector<Task*>::iterator iter = m_removeTasks.begin();
        iter != m_removeTasks.end(); ++iter) {
        COMMLOG(OS_LOG_INFO, "taskID: %s, taskName: %s",
            (*iter)->GetTaskID().c_str(), (*iter)->GetTaskName().c_str());
        if ((*iter)->GetTaskID() == taskID && (*iter)->GetTaskName() == taskName) {
            return *iter;
        }
    }
    return NULL;
}

mp_void TaskManager::RemoveTaskFromMap(const mp_string& taskID)
{
    CThreadAutoLock tlock(&m_mapLock);
    std::map<mp_string, Task*>::iterator iter = m_taskMap.find(taskID);
    if (iter != m_taskMap.end()) {
        Task* task = iter->second;
        m_taskMap.erase(iter);
        // 临时规避放到临时容器中，等待定时删除
        task->UpdateDelTime();
        m_removeTasks.push_back(task);
    }

    // 临时规避Agent的core问题，删除临时列表中过期的task
    static const mp_double REMOVE_TIME_OUT = 3600;
    mp_time nowTime;
    CMpTime::Now(nowTime);
    for (vector<Task*>::iterator iter = m_removeTasks.begin(); iter != m_removeTasks.end();) {
        Task* task = *iter;
        if (task == NULL) {
            iter = m_removeTasks.erase(iter);
            continue;
        }

        mp_double diff = CMpTime::Difftime(nowTime, task->GetDelTime());
        if (diff >= REMOVE_TIME_OUT) {
            iter = m_removeTasks.erase(iter);
            TaskContext::GetInstance()->RemoveTaskContext(task->GetTaskID());
            COMMLOG(OS_LOG_INFO, "remove task %s succ.", task->GetTaskID().c_str());
            delete task;
        } else {
            ++iter;
        }
    }

#if (defined LINUX) && (!defined ENABLE_TSAN)
    MallocExtension::instance()->ReleaseFreeMemory();
#endif

    COMMLOG(OS_LOG_INFO, "remove task list size %d.", m_removeTasks.size());
}

mp_int32 TaskManager::RunTask(const mp_string& taskID)
{
    Task* pTask = NULL;
    FindTask(taskID, pTask);
    if (NULL == pTask) {
        COMMLOG(OS_LOG_ERROR, "task id %s not found when running task", taskID.c_str());
        return ERROR_AGENT_INTERNAL_ERROR;
    }
    pTask->SetExpiration(0);

    thread_id_t tid;
    mp_int32 iRet = CMpThread::Create(&tid, RunTaskInThread, (mp_void*)pTask);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create task %s thread failed, ret %d.", taskID.c_str(), iRet);
        TaskContext::GetInstance()->RemoveTaskContext(taskID);
        return ERROR_AGENT_INTERNAL_ERROR;
    }
    pTask->SetThreadID(tid);

    COMMLOG(OS_LOG_INFO, "Create task %s thread successfully.", taskID.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskManager::RunTaskWithExpiration(const mp_string& taskID, mp_int64 taskExpiration)
{
    Task* pTask = NULL;
    FindTask(taskID, pTask);
    if (NULL == pTask) {
        COMMLOG(OS_LOG_ERROR, "task id %s not found when running task with expiration", taskID.c_str());
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    // set expiration
    COMMLOG(OS_LOG_DEBUG, "run task %s with expiration %u", taskID.c_str(), taskExpiration);
    mp_int64 expiration = (taskExpiration > 0) ? taskExpiration : DEFAULT_EXPIRATION;
    pTask->SetExpiration(expiration);

    // create a thread to run
    thread_id_t tid;
    mp_int32 iRet = CMpThread::Create(&tid, RunTaskInThread, (mp_void*)pTask);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create task %s thread failed, ret %d.", taskID.c_str(), iRet);
        return MP_FAILED;
    }
    pTask->SetThreadID(tid);
    COMMLOG(OS_LOG_INFO, "Create task %s thread success.", taskID.c_str());

    // create a thread to check expiration
    mp_int32 cRet = CMpThread::Create(&tid, CheckTaskExpiration, (mp_void*)pTask);
    if (MP_SUCCESS != cRet) {
        COMMLOG(OS_LOG_ERROR, "Create task %s thread failed, ret %d.", taskID.c_str(), cRet);
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Create task %s expiration thread success.", taskID.c_str());
    return MP_SUCCESS;
}

#ifdef WIN32
DWORD WINAPI TaskManager::RunTaskInThread(mp_void* pThis)
#else
mp_void* TaskManager::RunTaskInThread(mp_void* pThis)
#endif
{
    if (!pThis) {
        COMMLOG(OS_LOG_ERROR, "cast to task pointer failed.");
        CMPTHREAD_RETURN;
    }

    Task* pTask = static_cast<Task*>(pThis);
    if (pTask != NULL) {
        mp_string taskId = pTask->GetTaskID();
        COMMLOG(OS_LOG_INFO, "Begin to run task %s in thread.", taskId.c_str());
        Json::Value respMsg;
        (mp_void) pTask->RunTask(respMsg);
        COMMLOG(OS_LOG_INFO, "task %s have finished.", taskId.c_str());
        TaskManager::GetInstance()->RemoveTaskFromMap(taskId);
    } else {
        // 理论上不会到这个分支，如果出现也不会去delete 指针
        COMMLOG(OS_LOG_ERROR, "cast to task point failed.");
    }

    CMPTHREAD_RETURN;
}

#ifdef WIN32
DWORD WINAPI TaskManager::CheckTaskExpiration(void* pThis)
#else
mp_void* TaskManager::CheckTaskExpiration(void* pThis)
#endif
{
    Task* pTask = static_cast<Task*>(pThis);
    if (pTask != NULL) {
        COMMLOG(OS_LOG_DEBUG, "Begin to check task expiration in thread.");

        mp_int64 timeCount = pTask->GetExpiration();
        if (timeCount == 0) {
            COMMLOG(OS_LOG_INFO, "no expiration set");
            CMPTHREAD_RETURN;
        }

        while (timeCount > 0) {
            TaskStatus status = pTask->GetStatus();
            if (STATUS_ABORTED == status || STATUS_COMPLETED == status || STATUS_DELETED == status) {
                COMMLOG(OS_LOG_INFO, "task is finished in time");
                CMPTHREAD_RETURN;
            }

            DoSleep(CHECK_TASK_PERIOD);
            timeCount = timeCount - CHECK_TASK_PERIOD;
        }

        // kill task when time is expired
        (mp_void) pTask->CancelTask();
    } else {
        COMMLOG(OS_LOG_DEBUG, "cast to task point failed.");
    }

    CMPTHREAD_RETURN;
}

#ifdef WIN32
DWORD WINAPI TaskManager::RedoTaskInThread(void* pThis)
#else
mp_void* TaskManager::RedoTaskInThread(void* pThis)
#endif
{
    if (!pThis) {
        COMMLOG(OS_LOG_ERROR, "cast to task pointer failed.");
        CMPTHREAD_RETURN;
    }
    /*
    1.get task meta data from database
    2.initial task
    3.initial parameter
    4.redo with new thread
    */
    Task* pTask = static_cast<Task*>(pThis);
    if (pTask != NULL) {
        mp_string taskId = pTask->GetTaskID();
        COMMLOG(OS_LOG_INFO, "Begin to redo task %s in thread.", taskId.c_str());
        (void)pTask->Redo();
        COMMLOG(OS_LOG_INFO, "redo task %s have finished.", taskId.c_str());
        TaskManager::GetInstance()->RemoveTaskFromMap(taskId);
    } else {
        // 理论上不会到这个分支，如果出现也不会去delete 指针
        COMMLOG(OS_LOG_ERROR, "cast to task point failed.");
    }

    CMPTHREAD_RETURN;
}

mp_int32 TaskManager::CancelTask(const mp_string& taskID)
{
    Task* pTask = NULL;
    FindTask(taskID, pTask);
    if (NULL == pTask) {
        COMMLOG(OS_LOG_ERROR, "task id %s not found when deleting task", taskID.c_str());
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    COMMLOG(OS_LOG_INFO, "Begint to cancel task %s", taskID.c_str());
    mp_int32 iRet = pTask->CancelTask();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Cancel task %s failed, iRet %d.", taskID.c_str(), iRet);
        return iRet;
    }
    RemoveTaskFromMap(taskID);
    COMMLOG(OS_LOG_INFO, "Finish to cancel task %s", taskID.c_str());

    return MP_SUCCESS;
}

mp_int32 TaskManager::GetTaskStatusAndProgress(const mp_string& taskID, mp_string& status, mp_int32& progress)
{
    Task* pTask = NULL;
    FindTask(taskID, pTask);
    if (NULL == pTask) {
        COMMLOG(OS_LOG_WARN, "task id %s not found when getting task status", taskID.c_str());
        status = "NoExists";
        progress = 0;
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    COMMLOG(OS_LOG_DEBUG, "get task status, taskID %s.", taskID.c_str());
    // update status from file to memory
    mp_int32 iRet = pTask->RefreshTaskInfo();
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_WARN, "task id %s fresh task info failed.", taskID.c_str());
        return MP_FAILED;
    }

    TaskStatus enumStatus = pTask->GetStatus();
    progress = pTask->GetProgress();

    // return status from memory
    status = TransferStatus(enumStatus);
    return MP_SUCCESS;
}

mp_int32 TaskManager::GetTaskStatus(const mp_string& taskID, mp_string& status)
{
    Task* pTask = NULL;
    FindTask(taskID, pTask);
    if (NULL == pTask) {
        COMMLOG(OS_LOG_ERROR, "task id %s not found when getting task status", taskID.c_str());
        return ERROR_AGENT_INTERNAL_ERROR;
    }

    // return status from memory
    status = TransferStatus(pTask->GetStatus());
    return MP_SUCCESS;
}

mp_string TaskManager::TransferStatus(TaskStatus status)
{
    static const mp_int32 statusSize = 7;
    if (status >= 0 || status < statusSize) {
        static mp_string statusString[statusSize] = {
            "initial", "running", "completed", "failed", "deleting", "deleted", "abort"
        };
        return statusString[status];
    } else {
        return "";
    }
}

mp_int32 TaskManager::ThreadNumCheck()
{
    if (m_taskMap.size() >= m_threadPoolMax) {
        COMMLOG(OS_LOG_ERROR,
            "threadpool max number is reached, can not run new task, currentThreadSize = %d, threadPoolMax = %d",
            m_taskMap.size(),
            m_threadPoolMax);
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_DEBUG, "cureentThreadSize is %d", m_taskMap.size());
        return MP_SUCCESS;
    }
}

EXTER_ATTACK static mp_int32 DeleteExpireTasks(CDB &sqlDB, DbParamStream &dps)
{
    mp_string expireTime;
    if (CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_TASK_EXPIRE_DAYS, expireTime) !=
        MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "get task expireTime failed.");
        return MP_FAILED;
    }
    if (atoi(expireTime.c_str()) < 0) {
        COMMLOG(OS_LOG_ERROR, "invalid expiretime.");
        return MP_FAILED;
    }
    ostringstream delBuff;
    delBuff << "delete from " << JOBS << " where " << "startTime" << " < "
            << "date('now','-"<< expireTime <<" day')" << ";";
    return sqlDB.ExecSql(delBuff.str(), dps);
}

mp_int32 TaskManager::SaveTask(Task* task, const taskinfo_t& taskInfo)
{
    if (task != NULL && task->GetRedoFlag() == MP_FALSE) {
        INFOLOG("It's not redo task %s, don't insert into database.", task->GetTaskID().c_str());
        return MP_SUCCESS;
    }
    DbParamStream dps;
    mp_int32 iRet = DeleteExpireTasks(CDB::GetInstance(), dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Delete expire tasks failed, iRet=%d.", iRet);
    }

    COMMLOG(OS_LOG_INFO, "Begin save task info to db.");
    ostringstream insertBuff;
    insertBuff << "insert into " << JOBS << "(" << g_ID << "," << g_Status << "," << g_Step << ","
        << g_SubStepStatus << "," << g_SubStep << "," << g_ConnIP << "," << g_ConnPort << ","
        << g_InnerPID << "," << g_TaskType << "," << g_MsgBody << "," << "startTime" <<
        ") values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?,date('now'));";
    dps.Clear();
    dps << DbParam(taskInfo.taskID) << DbParam(taskInfo.status) << DbParam(taskInfo.step)
        << DbParam(taskInfo.subSetpStatus) << DbParam(taskInfo.subStep) << DbParam(taskInfo.connIp)
        << DbParam(taskInfo.connPort) << DbParam(taskInfo.innerPID) << DbParam(taskInfo.taskType);
    mp_string strEncryptBody;
    EncryptStr(taskInfo.msgBody, strEncryptBody);
    dps << DbParam(std::move(strEncryptBody));

    iRet = CDB::GetInstance().ExecSql(insertBuff.str(), dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Save task info failed, iRet=%d.", iRet);
        return iRet;
    }
    COMMLOG(OS_LOG_DEBUG, "Save task(%s) info success.", taskInfo.taskID.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskManager::QueryTaskInfo(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;

    DBReader readBuff;

    ostringstream buff;
    buff << "select ID, status, step, subStepStatus, subStep, connIp, connPort, innerPID, taskType, msgBody from "
         << JOBS << " where " << g_ID << " == ? and " << g_Step << " == ?";
    mp_string strSql = buff.str();

    DbParamStream dps;
    DbParam dp = taskID;
    dps << std::move(dp);
    dp = taskStep;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "db.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    if (iRowCount == 0) {
        COMMLOG(OS_LOG_WARN, "Task %s and step %s not exist.", taskID.c_str(), taskStep.c_str());
        return MP_SUCCESS;
    }

    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        mp_string strStatus = "";
        mp_string strSubStatus = "";
        mp_string strConnIp = "";
        mp_string strConnPort = "";
        mp_string strTaskType = "";
        mp_string strTaskEncBody = "";
        readBuff >> taskInfo.taskID;
        readBuff >> strStatus;
        readBuff >> taskInfo.step;
        readBuff >> strSubStatus;
        readBuff >> taskInfo.subStep;
        readBuff >> strConnIp;
        readBuff >> strConnPort;
        readBuff >> taskInfo.innerPID;
        readBuff >> strTaskType;
        readBuff >> strTaskEncBody;
        taskInfo.status = atoi(strStatus.c_str());
        taskInfo.subSetpStatus = atoi(strSubStatus.c_str());
        taskInfo.connIp = std::move(strConnIp);
        taskInfo.connPort = atoi(strConnPort.c_str());
        taskInfo.taskType = atoi(strTaskType.c_str());
        DecryptStr(strTaskEncBody, taskInfo.msgBody);
    }
    COMMLOG(OS_LOG_DEBUG, "Task %s query taskinfo succ.", taskID.c_str());
    return MP_SUCCESS;
}

mp_int32 TaskManager::UpdateSubStepTask(const mp_string& taskId,
    mp_int32 subStepStatus, const mp_string& subStep, const mp_string& taskName)
{
    DbParamStream dps;

    COMMLOG(
        OS_LOG_DEBUG, "Begin to update task %s(%s) status to [%d] .", taskId.c_str(), subStep.c_str(), subStepStatus);
    ostringstream updateBuff;
    updateBuff << "update " << JOBS << " set " << g_SubStepStatus << "= ?," << g_SubStep << "= ?"
               << " where " << g_ID << " == ? and " << g_Step << " == ?";
    mp_string strSql = updateBuff.str();

    DbParam dp = subStepStatus;
    dps << std::move(dp);
    dp = subStep;
    dps << std::move(dp);
    dp = taskId;
    dps << std::move(dp);
    dp = taskName;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update subStepTask info failed, iRet=%d.", iRet);
    }

    COMMLOG(OS_LOG_DEBUG, "Update task %s(step:%s) status[%d] succ.", taskId.c_str(), subStep.c_str(), subStepStatus);
    return iRet;
}

mp_int32 TaskManager::UpdateTaskStatus(const mp_string& taskId, mp_int32 status, const mp_string& step)
{
    DbParamStream dps;

    COMMLOG(OS_LOG_DEBUG, "Begin to update task %s(%s) status to [%d] .", taskId.c_str(), step.c_str(), status);
    ostringstream updateBuff;
    updateBuff << "update " << JOBS << " set " << g_Status << "= ?"
               << " where " << g_ID << " == ?"
               << " and " << g_Step << " == ?";
    mp_string strSql = updateBuff.str();

    DbParam dp = status;
    dps << std::move(dp);
    dp = taskId;
    dps << std::move(dp);
    dp = step;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().ExecSql(strSql, dps);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Update task %s status failed, iRet=%d.", taskId.c_str(), iRet);
    }
    COMMLOG(OS_LOG_DEBUG, "Update task %s(%s) status[%d] succ.", taskId.c_str(), step.c_str(), status);
    return iRet;
}

mp_int32 TaskManager::GetAllRunningFromDB(std::vector<taskinfo_t>& tasksInfo)
{
    DbParamStream dps;
    mp_int32 iRowCount = 0;
    mp_int32 iColCount = 0;
    DBReader readBuff;

    ostringstream buff;
    buff << "select " << g_ID << "," << g_Step << " from " << JOBS << " where " << g_Status << " == ?  and "
         << g_SubStepStatus << " == ?";
    mp_string strSql = buff.str();
    DbParam dp = STATUS_INPROGRESS;
    dps << std::move(dp);
    dp = STATUS_INPROGRESS;
    dps << std::move(dp);

    mp_int32 iRet = CDB::GetInstance().QueryTable(strSql, dps, readBuff, iRowCount, iColCount);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "sqlDB.QueryTable failed, iRet = %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Get %d running jobs from DB success.", iRowCount);
    tasksInfo.reserve(iRowCount);
    for (mp_int32 iRow = 1; iRow <= iRowCount; ++iRow) {
        taskinfo_t taskInfo;
        readBuff >> taskInfo.taskID;
        readBuff >> taskInfo.step;
        tasksInfo.emplace_back(taskInfo);
        COMMLOG(OS_LOG_DEBUG, "Task %s is still running, will do again.", taskInfo.taskID.c_str());
    }

    COMMLOG(OS_LOG_DEBUG, "Get %d running jobs from DB success.", tasksInfo.size());
    return MP_SUCCESS;
}

mp_int32 TaskManager::CreateRedoTask()
{
    // get all running jobs in db
    vector<taskinfo_t> tasksInfo;
    mp_int32 iRet = TaskManager::GetInstance()->GetAllRunningFromDB(tasksInfo);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    for (vector<taskinfo_t>::iterator iter = tasksInfo.begin(); iter != tasksInfo.end(); ++iter) {
        COMMLOG(OS_LOG_DEBUG, "ReCreate task %s step %s begin.", (*iter).taskID.c_str(), (*iter).step.c_str());
        RedoTaskNewFuncPtr func = TaskRedoFuncContainer::GetInstance().GetFunc((*iter).step);
        COMMLOG(OS_LOG_DEBUG, "get RedoTaskNewFuncPtr succ.");
        if (func == NULL) {
            UpdateTaskStatus((*iter).taskID, STATUS_FAILED, (*iter).step);
            COMMLOG(OS_LOG_ERROR, "get RedoTaskNewFuncPtr is null.");
            continue;
        }

        Task* task = func((*iter).taskID);
        if (!task) {
            return MP_FAILED;
        }

        COMMLOG(OS_LOG_DEBUG, "ReCreate task succ.");
        iRet = TaskManager::GetInstance()->AddTask((*iter).taskID, task);
        if (iRet != MP_SUCCESS) {
            delete task;
            task = NULL;
            TaskContext::GetInstance()->RemoveTaskContext((*iter).taskID);
            return iRet;
        }

        // run redo task
        task->SetExpiration(0);

        thread_id_t tid;
        iRet = CMpThread::Create(&tid, RedoTaskInThread, (mp_void*)task);
        if (MP_SUCCESS != iRet) {
            COMMLOG(OS_LOG_ERROR, "Create task %s thread failed, ret %d.", (*iter).taskID.c_str(), iRet);
            TaskContext::GetInstance()->RemoveTaskContext((*iter).taskID);
            delete task;
            task = NULL;
            return MP_FAILED;
        }
        task->SetThreadID(tid);

        COMMLOG(OS_LOG_DEBUG, "ReCreate task %s thread successfully.", (*iter).taskID.c_str());
    }

    return MP_SUCCESS;
}
