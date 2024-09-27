/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file FileSearcher.cpp
 * @brief  The implemention about single instance, manage all tasks
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef _AGENT_TASKMANAGER_H_
#define _AGENT_TASKMANAGER_H_

#include <map>
#include <vector>

#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "common/JsonUtils.h"
#include "taskmanager/Task.h"
#include "taskmanager/TaskContext.h"
#include "message/tcp/CDppMessage.h"
namespace {
const mp_int64 DEFAULT_EXPIRATION = 604800000;  // 1000 * 3600 * 24 * 7
const mp_int32 THREADPOOL_MAX = 1000;
const mp_int32 CHECK_TASK_PERIOD = 1000;
const mp_int32 MP_NOTEXIST = -3;
const mp_int32 ERASE_PLACE = 2;
}

class TaskManager {
public:
    static TaskManager* GetInstance();
    mp_int32 RunTask(const mp_string& taskID);
    mp_int32 RunTaskWithExpiration(const mp_string& taskID, mp_int64 taskExpiration);
    mp_int32 CancelTask(const mp_string& taskID);
    mp_int32 GetTaskStatusAndProgress(const mp_string& taskID, mp_string& status, mp_int32& progress);
    mp_int32 GetTaskStatus(const mp_string& taskID, mp_string& status);
    mp_void InitThreadPool(mp_int32 threadPoolMax);
    mp_int32 AddTask(const mp_string& taskID, Task* newTask);
    mp_void FindTask(const mp_string& taskID, Task*& pTask);
    Task* FindTaskEx(const mp_string& taskID, const mp_string& taskName);
    mp_int32 SaveTask(Task* task, const taskinfo_t& taskInfo);
    mp_int32 UpdateSubStepTask(const mp_string& taskId,
        mp_int32 subStepStatus, const mp_string& subStep, const mp_string& taskName);
    mp_int32 UpdateTaskStatus(const mp_string& taskId, mp_int32 status, const mp_string& step);
    mp_int32 QueryTaskInfo(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo);
    mp_void RemoveTaskFromMap(const mp_string& taskID);
    mp_int32 GetAllRunningFromDB(std::vector<taskinfo_t>& tasksInfo);
    mp_int32 CreateRedoTask();

private:
    TaskManager();
    ~TaskManager();

    mp_int32 ThreadNumCheck();

    mp_string TransferStatus(TaskStatus status);
#ifdef WIN32
    static DWORD WINAPI RunTaskInThread(mp_void* pThis);
    static DWORD WINAPI CheckTaskExpiration(mp_void* pThis);
    static DWORD RedoTaskInThread(void* pThis);
#else
    static mp_void* RunTaskInThread(mp_void* pThis);
    static mp_void* CheckTaskExpiration(mp_void* pThis);
    static mp_void* RedoTaskInThread(void* pThis);
#endif

private:
    static TaskManager* m_pTaskManager;
    // wgt: 任务执行前加入，执行完成后删除，只存在运行中的task， 同步异步任务都需要处理
    std::map<mp_string, Task*> m_taskMap;
    std::vector<Task*> m_removeTasks;
    thread_lock_t m_mapLock;
    mp_uint64 m_threadPoolMax;
};

static mp_int32 InitialTaskParam(
    const mp_string& msgBody, Json::Value& bodyParam, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    // need to initial backup step param
    Json::Value params;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(msgBody, params);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "convert string to json dpp message failed, taskId[%s]", taskId.c_str());
        return iRet;
    }

    if (!params.isObject() || !params.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "dpp message string have no body.");
        return MP_FAILED;
    }

    bodyParam = params[MANAGECMD_KEY_BODY];
    GET_JSON_STRING(bodyParam, KEY_TASKID, taskId);
    CHECK_FAIL_EX(CheckParamStringEnd(taskId, 0, MAX_TASKID_LEN));

    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueString(taskId, KEY_CONNECTION_IP, connIp);
    // connection ip address: reporting data
    TaskContext::GetInstance()->SetValueUInt32(taskId, KEY_CONNECTION_PORT, connPort);
    return MP_SUCCESS;
}

static mp_bool CheckTaskInfo(bool isSync, const mp_string& tskName, const mp_string& taskId, mp_int32& iCheckResult)
{
    taskinfo_t taskInfo;
    COMMLOG(OS_LOG_INFO, "task %s run sync task is %s.", taskId.c_str(), tskName.c_str());
    mp_int32 iRet = TaskManager::GetInstance()->QueryTaskInfo(taskId, tskName, taskInfo);
    if (iRet != MP_SUCCESS) {
        iCheckResult = iRet;
        return MP_FALSE;
    }
    if (taskInfo.taskID != "") {
        if (taskInfo.status == STATUS_FAILED) {
            COMMLOG(OS_LOG_ERROR, "Task %s already failed before.", taskId.c_str());
            TaskContext::GetInstance()->GetValueInt32(taskId, KEY_ERRCODE, iCheckResult);
            return MP_FALSE;
        } else if (taskInfo.status == STATUS_COMPLETED) {
            COMMLOG(OS_LOG_INFO, "Task %s already succ before.", taskId.c_str());
            iCheckResult = isSync ? MP_SUCCESS : MP_TASK_COMPLETE;
            return MP_FALSE;
        } else {
            COMMLOG(OS_LOG_INFO, "Task %s is running now.", taskId.c_str());
            iCheckResult = isSync ? MP_TASK_COMPLETE : MP_TASK_RUNNING;
            return MP_FALSE;
        }
    }

    return MP_TRUE;
}

template<typename T>
mp_int32 CreateTask(const mp_string& msgBody, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    Json::Value bodyParam;
    mp_int32 iRet = InitialTaskParam(msgBody, bodyParam, connIp, connPort, taskId);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }

    mp_int32 taskType = 0;
    taskinfo_t tskInfo;
    /*
    check task status by taskid and class name, query data from database
    exist
        return db result
    no exist
        execute normally
    */
    mp_string tskName = typeid(T).name();
    tskName = tskName.erase(0, ERASE_PLACE);
    mp_int32 iCheckResult = MP_FAILED;
    if (CheckTaskInfo(false, tskName, taskId, iCheckResult) != MP_TRUE) {
        return iCheckResult;
    }

    // wgt save task, failed record log for not support redo
    T* task = new (std::nothrow) T(taskId);
    if (!task) {
        COMMLOG(OS_LOG_ERROR, "New task failed.");
        goto FIN_FLAG;
    }

    // there is tasktype only when backup task, so, don't handle the task type
    (mp_void) TaskContext::GetInstance()->GetValueInt32(taskId, KEY_BACKUP_MODE, taskType);

    // wgt save task parameter
    tskInfo.taskID = taskId;
    tskInfo.status = STATUS_INPROGRESS;
    tskInfo.step = std::move(tskName);
    tskInfo.msgBody = msgBody;
    tskInfo.connIp = connIp;
    tskInfo.connPort = connPort;
    tskInfo.taskType = taskType;

    iRet = TaskManager::GetInstance()->SaveTask(task, tskInfo);
    if (iRet != MP_SUCCESS) {
        goto FIN_FLAG;
    }

    // initial tasksetp by request message
    iRet = task->InitTaskStep(bodyParam);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init task step failed ret=%d.", iRet);
        goto FIN_FLAG;
    }

    iRet = TaskManager::GetInstance()->AddTask(taskId, task);
    if (iRet != MP_SUCCESS) {
        goto FIN_FLAG;
    }

    iRet = MP_SUCCESS;

FIN_FLAG:
    if (iRet != MP_SUCCESS) {
        if (task) {
            delete task;
        }
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
    }
    return iRet;
}

template<typename T>
mp_int32 RunSyncSubTask(const mp_string &taskId, const mp_string &taskName, taskinfo_t& taskInfo,
                        const mp_string &msgBody, T* task)
{
    mp_int32 iRet = TaskManager::GetInstance()->QueryTaskInfo(taskId, taskName, taskInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Task %s get task info failed.", taskId.c_str());
        return iRet;
    }
    if (taskInfo.taskID.empty()) {
        // wgt save task parameter
        taskInfo.taskID = taskId;
        taskInfo.status = STATUS_INITIAL;
        taskInfo.step = taskName;
        taskInfo.msgBody = msgBody;
        iRet = TaskManager::GetInstance()->SaveTask(task, taskInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Task %s save task info failed.", taskId.c_str());
            return iRet;
        }
    }
    return MP_SUCCESS;
}
/*
run synchronous task directly, don't add it into task list, so reexcute isn't supported
check task status by taskid and class name, query data from database
exist
    return db result
no exist
    execute normally
*/
template<typename T>
mp_int32 RunSyncTask(const mp_string& msgBody, mp_string& taskId, Json::Value &respMsg)
{
    mp_string taskName = typeid(T).name();
    COMMLOG(OS_LOG_INFO, "task %s run sync task is %s.", taskId.c_str(), taskName.c_str());
    mp_int32 iPlace = 2;
    taskName = taskName.erase(0, iPlace);
    // need to initial backup step param
    Json::Value params;
    mp_int32 iRet = CJsonUtils::ConvertStringtoJson(msgBody, params);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Run sync task(%s) failed.", taskId.c_str());
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return iRet;
    }
    if (!params.isObject() || !params.isMember(MANAGECMD_KEY_BODY)) {
        COMMLOG(OS_LOG_ERROR, "RunSyncTask: dpp message string have no body, %s.", taskId.c_str());
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return iRet;
    }

    // wgt save task, failed record log for not support redo
    T* task = new (std::nothrow) T(taskId);
    if (!task) {
        COMMLOG(OS_LOG_ERROR, "New sychronous task(%s) failed.", taskId.c_str());
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
        return MP_FAILED;
    }
    taskinfo_t taskInfo;
    iRet = RunSyncSubTask(taskId, taskName, taskInfo, msgBody, task);
    if (iRet != MP_SUCCESS) {
        delete task;
        task = NULL;
        COMMLOG(OS_LOG_ERROR, "Task %s save task info failed.", taskId.c_str());
        return iRet;
    }
    // initial taskstep by request message
    iRet = task->InitTaskStep(params[MANAGECMD_KEY_BODY]);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Init sync task step failed ret=%d.", iRet);
        TaskContext::GetInstance()->RemoveTaskContext(taskId);
    } else {
        iRet = task->RunTask(respMsg);
    }
    INFOLOG("RunSyncTask respMsg.");
    delete task;
    task = NULL;
    return iRet;
}
#endif  // _AGENT_TASKMANAGER_H_
