/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Task.h
 * @brief  Contains function declarations Task
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */
#ifndef _AGENT_TASK_H_
#define _AGENT_TASK_H_
#include <vector>
#include <algorithm>

#include "common/CMpThread.h"
#include "common/Log.h"
#include "common/Types.h"
#include "common/Uuid.h"
#include "taskmanager/TaskStep.h"
#include "message/tcp/CDppMessage.h"

// add taskstep to step list
#define ADD_TASKSTEP(clsname, name, ratio, steplist)                                                                   \
    do {                                                                                                               \
        mp_int32 iRet = InitialSubStep<clsname>(name, ratio);                                                          \
        if (iRet != MP_SUCCESS) {                                                                                      \
            COMMLOG(OS_LOG_ERROR, "initial %s failed, iRet %d.", #clsname, iRet);                                      \
            steplist.clear();                                                                                          \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

// add extend step of task step
#define ADD_CLEARSTEP(stepName, eStepCls, eStepName, stepParamKey)                                                     \
    do {                                                                                                               \
        mp_int32 iRet = AddClearStep<eStepCls>(stepName, eStepName, stepParamKey);                                     \
        if (iRet != MP_SUCCESS) {                                                                                      \
            COMMLOG(OS_LOG_ERROR, "add extend eStep %s to %s failed, iRet %d.", #eStepCls, stepName, iRet);            \
            return;                                                                                                    \
        }                                                                                                              \
    } while (0)

// sort task step
struct CompareStep {
    bool operator()(TaskStep* step1, TaskStep* step2)
    {
        if (step1 == nullptr || step2 == nullptr) {
            return false;
        }
        return step1->GetOrder() < step2->GetOrder();
    }
};

// task mode, asynchronous or synchronous
typedef enum { ASYNCHRONOUS_TASK = 0, SYNCHRONOUS_TASK } TaskMode;

typedef struct taskinfo {
    mp_string taskID;
    mp_int32 status;
    mp_string step;
    mp_int32 subSetpStatus;
    mp_string subStep;
    mp_string connIp;
    mp_uint32 connPort;
    mp_string innerPID;
    mp_int32 taskType;
    mp_string msgBody;
    mp_string startTime;
    taskinfo()
    {
        status = 0;
        subSetpStatus = 0;
        connIp = "";
        connPort = 0;
        taskType = 0;
        msgBody = "";
        startTime = "";
    }
} taskinfo_t;

class Task {
public:
    Task(const mp_string& taskID);
    virtual ~Task();
    virtual mp_int32 InitTaskStep(const Json::Value& param);
    virtual mp_int32 RunTask(Json::Value &respMsg);
    virtual mp_int32 RunSyncTask();
    virtual mp_int32 CancelTask();
    virtual mp_int32 CancelTask(Json::Value &respParam);
    virtual mp_int32 RefreshTaskInfo();
    // 部分场景下，runTask会启动一个task，后续需要通过定时更新备份参数方式驱动任务执行，如VMWare备份实现
    virtual mp_int32 UpdateTask(const Json::Value& param);
    virtual mp_int32 UpdateTask(Json::Value& param, Json::Value& respBody);
    // 部分场景下，runTask会启动一个task，并保存运行状态，等待微服务通知任务完成
    virtual mp_int32 FinishTask(const Json::Value& param);
    virtual mp_int32 FinishTask(Json::Value& param, Json::Value& respBody);

    mp_string GetTaskID();
    mp_string GetTaskName();
    mp_int32 GetProgress();
    TaskStatus GetStatus();
    mp_int32 GetSpeed();
    mp_int32 GetBackupSize();
    mp_void SetExpiration(mp_int64 expiration);
    mp_int64 GetExpiration();
    thread_id_t GetThreadID();
    mp_void SetThreadID(thread_id_t threadId);
    TaskMode GetTaskMode();
    mp_void SetRedoTaskFalg(mp_bool redoTask);
    mp_bool GetRedoTaskFlag();
    mp_int32 GetErrCode();
    std::vector<mp_string> GetErrParams();
    std::vector<mp_string> GetErrDetails();
    mp_bool IsCompleted();
    mp_void SetReportStatus();
    mp_bool GetReportStatus();
    mp_void UpdateDelTime();
    mp_time GetDelTime();
    mp_int32 GetDisconnectNum();
    void setDisconnectNum(mp_int32 num);

    // initial task step, including status and parameter
    mp_int32 Redo();
    mp_bool GetRedoFlag();

    std::vector<std::pair<mp_string, mp_uint32> > &GetSubWarnInfo();
    void SetSubWarnInfo(const std::vector<std::pair<mp_string, mp_uint32> > &logLabel);
    void SetTaskResult(const Json::Value& result);
    Json::Value GetTaskResult();

    static mp_int32 ReportTaskStatus(Task* task);
    static mp_int32 ConstructAndSendMsg(Task* task, const mp_string &connIp, mp_uint16 connPort);
    static mp_bool SendStatusMsg(CDppMessage *rspMsg);

protected:
    // task support to redo
    mp_bool m_redoTask;
    mp_string m_taskId;
    mp_string m_taskName;
    volatile TaskStatus m_taskStatus;
    mp_int32 m_taskProgress;
    mp_int32 m_taskSpeed;
    mp_int32 m_backupSize;
    mp_int64 m_taskExpiration;
    volatile mp_bool m_ExitFlag;
    mp_int32 m_allStepRation;
    volatile mp_int32 m_finStepRation;
    thread_lock_t m_taskStepLock;
    TaskMode taskMode;
    mp_bool m_isReported;
    mp_time m_delTime;
    std::vector<std::pair<mp_string, mp_uint32> > m_subStepWarnInfo;
    Json::Value m_taskResult;
    mp_int32 m_disconnectNum;

    // operation object
    mp_string operObject;
    // operation command, 0:backupend task; other: send command
    mp_uint32 operCmd;
    // operation exclusive
    std::vector<mp_uint32> exCmds;

    // task step list, when running task, run the taskstep one by one
    std::vector<TaskStep*> m_steps;
    TaskStep* m_curStep;
    thread_id_t m_threadId;

    // task errorcode, ready report
    mp_int32 errorCode;
    std::vector<mp_string> errParams;
    std::vector<mp_string> errDetails;

    virtual mp_void CreateTaskStep() = 0;
    mp_int32 InitTaskStepParam(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName);

    // do something before run task, .eg start thread to get process
    virtual mp_void RunTaskBefore() = 0;
    // do something after task finish, .eg stop get process thread
    virtual mp_void RunTaskAfter() = 0;

    // add taskstep to step list
    template<typename T>
    mp_int32 InitialSubStep(const mp_string& name, mp_int32 ratio)
    {
        mp_string uuidStr;
        mp_int32 iRet = CUuidNum::GetUuidStandardStr(uuidStr);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get taskstep uuid failed, iRet %d.", iRet);
            return iRet;
        }

        // sort by adding order
        mp_int32 order = static_cast<mp_int32>(m_steps.size() + 1);
        T* t = new (std::nothrow) T(uuidStr, m_taskId, name, ratio, order);
        if (t == NULL) {
            COMMLOG(OS_LOG_ERROR, "new step %s failed.", name.c_str());
            return MP_FAILED;
        }
        m_steps.push_back(t);

        // sort when add taskstep, reduce calling complexity
        std::sort(m_steps.begin(), m_steps.end(), CompareStep());

        COMMLOG(OS_LOG_INFO, "initial task step name %s succ.", name.c_str());
        return MP_SUCCESS;
    }

    // add extend taskstep of taskstep
    template<typename T>
    mp_int32 AddClearStep(const mp_string& name, const mp_string& eStepName, const mp_string& stepParamKey)
    {
        mp_string uuidStr;
        mp_int32 iRet = CUuidNum::GetUuidStandardStr(uuidStr);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Get extenad step uuid failed, iRet %d.", iRet);
            return iRet;
        }

        // Get step by name
        TaskStep* step = NULL;
        for (std::vector<TaskStep*>::iterator iter = m_steps.begin(); iter != m_steps.end(); ++iter) {
            if ((*iter)->GetStepName() == name) {
                step = *iter;
            }
        }
        if (step == NULL) {
            COMMLOG(OS_LOG_ERROR, "Get step by name %s failed.", name.c_str());
            return MP_FAILED;
        }

        // sort by adding order
        T* t = new (std::nothrow) T(uuidStr, m_taskId, eStepName, 0, 0);
        if (t == NULL) {
            COMMLOG(OS_LOG_ERROR, "new extend step %s failed.", eStepName.c_str());
            return MP_FAILED;
        }
        t->SetParamKey(stepParamKey);

        step->AddClearStep(t);
        COMMLOG(OS_LOG_INFO, "add extend step %s to step %s succ.", eStepName.c_str(), name.c_str());
        return MP_SUCCESS;
    }

    inline mp_void ConfigCurrentStep(TaskStep* taskStep)
    {
        CThreadAutoLock lock(&m_taskStepLock);
        m_curStep = taskStep;
    }

private:
    mp_int32 RedoTaskStep(TaskStep* taskStep, mp_bool& executeFlag, mp_bool& continueRun,
        const mp_string& curRunningStep, mp_string& innerPID);
    mp_int32 DoTaskStep(TaskStep* taskStep);
    void PostSubStepWarnInfo();
    mp_int32 UpdateTaskStatus(mp_string taskId, mp_int32 status, mp_string step);
    mp_int32 UpdateSubStepTask(const mp_string& taskId,
        mp_int32 subStepStatus, const mp_string& subStep, const mp_string& taskName);
};

#endif  // _AGENT_TASK_H_
