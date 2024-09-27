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
#include "taskmanager/TaskTest.h"
#include "taskmanager/Task.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskManager.h"
class TaskSub : public Task
{
public:
    TaskSub(const mp_string& taskID):Task(taskID){}
    virtual mp_void CreateTaskStep() { return;}
    virtual mp_void RunTaskBefore() { return;}
    virtual mp_void RunTaskAfter() { return;}
};

class TaskStepSub : public TaskStep {
public:
    TaskStepSub(const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order) :
        TaskStep(id, taskId, name, ratio, order){}
    virtual mp_int32 Init(const Json::Value& param) {};
    virtual mp_int32 Run() { MP_SUCCESS; };
    virtual mp_int32 Cancel() {};
    virtual mp_int32 Cancel(Json::Value &respParam) {};
    virtual mp_int32 Stop(const Json::Value& param) {};
    virtual mp_int32 Update(const Json::Value& param) {};
    virtual mp_int32 Update(Json::Value& param, Json::Value& respParam) {};
    virtual mp_int32 Finish(const Json::Value& param) {};
    virtual mp_int32 Finish(Json::Value& param, Json::Value& respParam) {};
    virtual mp_int32 Redo(mp_string& innerPID) {};
    virtual mp_int32 RefreshStepInfo() {};
};

namespace {
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 QueryTableTest(
    mp_string strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    mp_string tmp("11");
    readBuff << tmp;
    readBuff << tmp;
    readBuff << tmp;
    readBuff << tmp;
    readBuff << tmp;
    readBuff << tmp;
    readBuff << tmp;
    iRowCount = 0;
    iColCount = 0;
    return MP_SUCCESS;
}
}
mp_int32 RunErrTest() { MP_FAILED; };

mp_int32 ReadResultTest(const mp_string& strFileName, std::vector<mp_string>& vecRlt)
{
    return MP_SUCCESS;
}

mp_int32 UpdateSubStepTaskTest(mp_void* pThis, const mp_string& taskId, mp_int32 subStepStatus,
    const mp_string& subStep, const mp_string& taskName)
{
    return MP_SUCCESS;
}

std::vector<TaskStep*> &GetClearStepsTest()
{
    std::vector<TaskStep*> m_clearSteps;
    std::string taskId;
    mp_string id;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepSub *stepsub = new TaskStepSub(id, taskId, name, ratio, order);
    m_clearSteps.push_back(stepsub);
    return m_clearSteps;
}

TEST_F(TaskTest, InitTaskStepTest)
{
    std::string taskId;
    TaskSub taskaa(taskId);

    Json::Value param;
    taskaa.InitTaskStep(param);
}

TEST_F(TaskTest, TaskRunTaskTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    Json::Value respMsg;
    TaskSub taskaa(taskId);
    taskaa.RunTask(respMsg);

    mp_int32 ratio;
    mp_int32 order;
    mp_string id;
    mp_string name;
    TaskStepSub *stepsub = new TaskStepSub(id, taskId, name, ratio, order);
    taskaa.m_steps.push_back(stepsub);
    taskaa.m_ExitFlag = MP_TRUE;
    taskaa.RunTask(respMsg);
}

TEST_F(TaskTest, RedoTaskStepTest)
{
    DoGetJsonStringTest();
    std::string taskId = "111";
    TaskSub taskaa(taskId);
    mp_string id;
    mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub stepsub(id, taskId, name, ratio, order);
    mp_bool executeFlag = 1;
    mp_bool continueRun = 1;
    mp_string curRunningStep = "sraeas";
    mp_string innerPID = "321";
    taskaa.RedoTaskStep(&stepsub, executeFlag, continueRun, curRunningStep, innerPID);
    executeFlag = 0;
    taskaa.RedoTaskStep(&stepsub, executeFlag, continueRun, curRunningStep, innerPID);
    Stub stub;
    stub.set(ADDR(CIPCFile, ReadResult), ReadResultTest);
    taskaa.RedoTaskStep(&stepsub, executeFlag, continueRun, curRunningStep, innerPID);
}

TEST_F(TaskTest, RedoTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.Redo();

    mp_string id;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepSub *stepsub = new TaskStepSub(id, taskId, name, ratio, order);
    taskaa.m_steps.push_back(stepsub);
    taskaa.Redo();
}

TEST_F(TaskTest, PostSubStepWarnInfoTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.PostSubStepWarnInfo();
}

TEST_F(TaskTest, DoTaskStepTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);

    mp_string id;
    mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub stepsub(id, taskId, name, ratio, order);
    taskaa.DoTaskStep(&stepsub);
    Stub stub;
    stub.set(ADDR(TaskManager, UpdateSubStepTask), UpdateSubStepTaskTest);
    taskaa.DoTaskStep(&stepsub);
    // stub.set(ADDR(TaskStep, GetClearSteps), GetClearStepsTest);
    taskaa.DoTaskStep(&stepsub);
}

TEST_F(TaskTest, RunSyncTaskTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.RunSyncTask();
    mp_int32 ratio;
    mp_int32 order;
    mp_string id;
    mp_string name;
    TaskStepSub *stepsub = new TaskStepSub(id, taskId, name, ratio, order);
    taskaa.m_steps.push_back(stepsub);

    taskaa.RunSyncTask();
}

TEST_F(TaskTest, RefreshTaskInfoTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub taskaa(taskId);
    taskaa.RefreshTaskInfo();
    taskaa.m_taskStatus = STATUS_INPROGRESS;
    // taskaa.m_curStep = nullptr;
    // taskaa.RefreshTaskInfo();
    taskaa.m_curStep = &taskSub;
    taskaa.RefreshTaskInfo();
}

TEST_F(TaskTest, UpdateTaskTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    taskaa.UpdateTask(param);
    taskaa.m_taskStatus = STATUS_DELETING;
    taskaa.UpdateTask(param);
    taskaa.m_taskStatus = STATUS_NO_EXISTS;
    taskaa.UpdateTask(param);
}

TEST_F(TaskTest, UpdateTaskJsonTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    Json::Value body;
    taskaa.UpdateTask(param, body);
    taskaa.m_taskStatus = STATUS_DELETING;
    taskaa.UpdateTask(param, body);
    taskaa.m_taskStatus = STATUS_NO_EXISTS;
    taskaa.UpdateTask(param, body);
}

TEST_F(TaskTest, FinishTaskTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    taskaa.FinishTask(param);
    taskaa.m_taskStatus = STATUS_DELETING;
    taskaa.FinishTask(param);
    taskaa.m_taskStatus = STATUS_INPROGRESS;
    taskaa.FinishTask(param);
}

TEST_F(TaskTest, FinishTaskTestJson)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    Json::Value body;
    taskaa.FinishTask(param, body);
    taskaa.m_taskStatus = STATUS_DELETING;
    taskaa.FinishTask(param, body);
    taskaa.m_taskStatus = STATUS_INPROGRESS;
    taskaa.FinishTask(param, body);
}

TEST_F(TaskTest, GetProgressTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    taskaa.GetProgress();
}

TEST_F(TaskTest, GetSpeedTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetSpeed();
}

TEST_F(TaskTest, GetThreadIDTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetThreadID();
}

TEST_F(TaskTest, GetTaskModeTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetTaskMode();
}

TEST_F(TaskTest, GetErrCodeTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetErrCode();
}

TEST_F(TaskTest, GetErrParamsTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetErrParams();
}

TEST_F(TaskTest, GetErrDetailsTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetErrDetails();
}

TEST_F(TaskTest, InitTaskStepParamTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    Json::Value param;
    mp_string paramKey;
    mp_string stepName;
    taskaa.InitTaskStepParam(param, paramKey, stepName);
    mp_int32 ratio;
    mp_int32 order;
    mp_string id;
    mp_string name;
    TaskStepSub *stepsub = new TaskStepSub(id, taskId, name, ratio, order);
    taskaa.m_steps.push_back(stepsub);
    taskaa.InitTaskStepParam(param, paramKey, stepName);
}

TEST_F(TaskTest, IsCompletedTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.IsCompleted();
}

TEST_F(TaskTest, SetReportStatusTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.SetReportStatus();
}

TEST_F(TaskTest, GetReportStatusTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetReportStatus();
}

TEST_F(TaskTest, GetSubWarnInfoTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    taskaa.GetSubWarnInfo();
}

TEST_F(TaskTest, SetSubWarnInfoTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    std::vector<std::pair<mp_string, mp_uint32>> logLabel;
    taskaa.SetSubWarnInfo(logLabel);
}

TEST_F(TaskTest, ReportTaskStatusTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    // TaskSub *taskaaa;
    TaskSub taskaa(taskId);
    std::vector<std::pair<mp_string, mp_uint32>> logLabel;
    // taskaa.ReportTaskStatus(taskaaa);
    taskaa.ReportTaskStatus(&taskaa);
}

TEST_F(TaskTest, ConstructAndSendMsgTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    mp_string connIp;
    mp_uint16 connPort;
    taskaa.ConstructAndSendMsg(&taskaa, connIp, connPort);
}

TEST_F(TaskTest, SendStatusMsgTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    CDppMessage rspMsg;
    taskaa.SendStatusMsg(&rspMsg);
}

TEST_F(TaskTest, InitialSubStepTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub taskaa(taskId);
    mp_string name;
    mp_int32 ratio;
    EXPECT_EQ(MP_SUCCESS, taskaa.InitialSubStep<TaskStepLinkTarget>(name, ratio));
}

TEST_F(TaskTest, AddClearStepTest)
{
    DoGetJsonStringTest();
    std::string taskId;
    TaskSub *taskaa = new TaskSub(taskId);
    mp_string name = "aaa";
    mp_string eStepName;
    mp_string stepParamKey;
    EXPECT_EQ(MP_FAILED, taskaa->AddClearStep<TaskStepLinkTarget>(name, eStepName, stepParamKey));

    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    taskaa->m_steps.push_back(&taskSub);
    taskaa->AddClearStep<TaskStepLinkTarget>(name, eStepName, stepParamKey);
}

TEST_F(TaskTest, CancelTaskTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    taskaa->CancelTask();
    taskaa->m_taskStatus = STATUS_DELETING;
    taskaa->CancelTask();
    taskaa->m_taskStatus = STATUS_INPROGRESS;
    taskaa->CancelTask();
}

TEST_F(TaskTest, CancelTaskJsonTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    Json::Value respParam;
    taskaa->CancelTask(respParam);
    taskaa->m_taskStatus = STATUS_DELETING;
    taskaa->CancelTask(respParam);
    taskaa->m_taskStatus = STATUS_INPROGRESS;
    taskaa->CancelTask(respParam);
}

TEST_F(TaskTest, GetTaskNameTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    taskaa->GetTaskName();
}

TEST_F(TaskTest, SetExpirationTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    mp_int64 expiration;
    taskaa->SetExpiration(expiration);
}

TEST_F(TaskTest, SetThreadIDTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    thread_id_t threadId;
    taskaa->SetThreadID(threadId);
}

TEST_F(TaskTest, GetDisconnectNumTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    taskaa->GetDisconnectNum();
}

TEST_F(TaskTest, setDisconnectNumTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    mp_int32 threadId;
    taskaa->setDisconnectNum(threadId);
}

TEST_F(TaskTest, SetTaskResultTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    Json::Value result;
    taskaa->SetTaskResult(result);
}

TEST_F(TaskTest, GetTaskResultTest)
{
    DoGetJsonStringTest();
    Stub stb;
    std::string taskId;
    mp_string name = "aaa";
    mp_string id;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskSub(id, taskId, name, ratio, order);
    TaskSub *taskaa = new TaskSub(taskId);
    taskaa->GetTaskResult();
}