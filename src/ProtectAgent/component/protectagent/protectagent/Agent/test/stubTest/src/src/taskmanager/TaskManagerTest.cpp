#include "taskmanager/TaskManagerTest.h"
#include "taskmanager/TaskManager.h"
#include "taskmanager/Task.h"

namespace {
mp_int32 StubCConfigXmlParserGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32Return); \
} while (0)
static int g_iSybaseJsonStringCounter = 0;
static mp_int32 SybaseStubJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    if (g_iSybaseJsonStringCounter++ == 0)
    {
        strValue = "";
        return MP_SUCCESS;
    }
    strValue = "test123";
    
    return MP_SUCCESS;
}
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
static bool  stub_return_bool_StopResouceGroup(void)
{
    return MP_TRUE;
}
mp_int32 CreateTest(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_FAILED;
}

mp_int32 CreateTestSuc(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 CancelTaskTest()
{
    return MP_FAILED;
}

mp_int32 CancelTaskTestSuc()
{
    return MP_SUCCESS;
}
}

mp_int32 RefreshTaskInfoTest()
{
    return MP_FAILED;
}

mp_int32 ExecSqlTest(const mp_string& strSql, DbParamStream& dps)
{
    return MP_SUCCESS;
}

mp_int32 ExecSqlTestFail(const mp_string& strSql, DbParamStream& dps)
{
    return MP_FAILED;
}

mp_int32 QueryTableTest(
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    iRowCount = 1;
    return MP_SUCCESS;
}

mp_int32 GetAllRunningFromDBTest(std::vector<taskinfo_t>& tasksInfo)
{
    // taskinfo_t aa;
    // tasksInfo.push_back(aa);
    return MP_SUCCESS;
}

mp_int32 ConvertStringtoJsonTest(const mp_string& rawBuffer, Json::Value& jsValue)
{
    Json::Value bodyjson;
    bodyjson["taskId"] = "1111";
    jsValue["body"] = bodyjson;

    return MP_SUCCESS;
}

mp_int32 ConvertStringtoJsonjsValueTest(const mp_string& rawBuffer, Json::Value& jsValue)
{
    jsValue["body"] = "aasdada";
    return MP_SUCCESS;
}

mp_int32 QueryTaskInfoTest(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
    return MP_SUCCESS;
}

mp_int32 QueryTaskInfoTestFailed(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
 //   taskInfo.taskID = "aaa";
 //   taskInfo.status = 4;
    return MP_FAILED;
}

mp_int32 QueryTaskInfoTestCompleted(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
    taskInfo.taskID = "aaa";
    taskInfo.status = 5;
    return MP_SUCCESS;
}

mp_int32 QueryTaskInfoTestCompletedbb(const mp_string& taskID, const mp_string& taskStep, taskinfo_t& taskInfo)
{
    taskInfo.taskID = "aaa";
    taskInfo.status = 6;
    return MP_SUCCESS;
}

mp_int32 InitialTaskParamTest(
    const mp_string& msgBody, Json::Value& bodyParam, const mp_string &connIp, mp_uint16 connPort, mp_string& taskId)
{
    return MP_SUCCESS;
}

class TaskSub : public Task
{
public:
    TaskSub(const mp_string& taskID):Task(taskID){}
    virtual mp_void CreateTaskStep() { return;}
    virtual mp_void RunTaskBefore() { return;}
    virtual mp_void RunTaskAfter() { return;}
    virtual mp_int32 RefreshTaskInfo() { return MP_SUCCESS; }
};

TEST_F(TaskManagerTest, RunTaskTest)
{
    DoGetJsonStringTest();
    TaskManager* manager = TaskManager::GetInstance();
    stub.set(ADDR(CMpThread, Create), CreateTestSuc);
    EXPECT_EQ(ERROR_AGENT_INTERNAL_ERROR, manager->RunTask("111"));
}

TEST_F(TaskManagerTest, CancelTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    Stub stubet;
    manager->CancelTask(taskID);
    TaskSub aataks(taskID);
    EXPECT_EQ(MP_SUCCESS, manager->AddTask(taskID, &aataks));
    manager->CancelTask(taskID);
    // stubet.set(ADDR(Task, CancelTask), CancelTaskTestSuc);
    // manager->CancelTask(taskID);
}

TEST_F(TaskManagerTest, GetTaskStatusAndProgressTest)
{
    LogTest();
    DoGetJsonStringTest();
    Stub stubaa;
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111212121";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    manager->GetTaskStatusAndProgress(taskID, status, progress);
    TaskSub aataks(taskID);
    EXPECT_EQ(MP_SUCCESS, manager->AddTask(taskID, &aataks));
    manager->GetTaskStatusAndProgress(taskID, status, progress);
    // stubaa.set(ADDR(Task, RefreshTaskInfo), RefreshTaskInfoTest);
    // manager->GetTaskStatusAndProgress(taskID, status, progress);
}

TEST_F(TaskManagerTest, GetTaskStatusTest)
{
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    manager->GetTaskStatus(taskID, status);
    TaskSub aataks(taskID);
    EXPECT_EQ(MP_SUCCESS, manager->AddTask(taskID, &aataks));
    EXPECT_EQ(MP_SUCCESS, manager->GetTaskStatus(taskID, status));
}

TEST_F(TaskManagerTest, InitThreadPoolTest)
{
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    manager->InitThreadPool(progress);
}

TEST_F(TaskManagerTest, AddTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    TaskSub aataks(taskID);
    EXPECT_EQ(MP_SUCCESS, manager->AddTask(taskID, &aataks));
    EXPECT_EQ(MP_FAILED, manager->AddTask(taskID, &aataks));
}

TEST_F(TaskManagerTest, FindTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    Task *pTask = NULL;
    manager->FindTask(taskID, pTask);
    manager->FindTask(taskID, pTask);
}

TEST_F(TaskManagerTest, SaveTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    mp_int64 taskExpiration;
    mp_string status;
    mp_int32 progress;
    TaskSub aataks(taskID);
    taskinfo_t taskInfo;
    manager->SaveTask(&aataks, taskInfo);
    TaskSub *taskaa = new TaskSub(taskID);
    taskaa->m_redoTask = MP_FALSE;
    manager->SaveTask(taskaa, taskInfo);
}

TEST_F(TaskManagerTest, UpdateSubStepTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);

    mp_int32 subStepStatus;
    mp_string subStep;
    mp_string taskName;
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    EXPECT_EQ(MP_SUCCESS, manager->UpdateSubStepTask(taskID, subStepStatus, subStep, taskName));
    stub.set(ADDR(CDB, ExecSql), ExecSqlTestFail);
    EXPECT_EQ(MP_FAILED, manager->UpdateSubStepTask(taskID, subStepStatus, subStep, taskName));
}

TEST_F(TaskManagerTest, UpdateTaskStatusTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);

    mp_int32 subStepStatus;
    mp_string subStep;
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    EXPECT_EQ(MP_SUCCESS, manager->UpdateTaskStatus(taskID, subStepStatus, subStep));
    stub.set(ADDR(CDB, ExecSql), ExecSqlTestFail);
    EXPECT_EQ(MP_FAILED, manager->UpdateTaskStatus(taskID, subStepStatus, subStep));
}

TEST_F(TaskManagerTest, QueryTaskInfoTest)
{
    LogTest();
    Stub stub11;
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);

    mp_int32 subStepStatus;
    mp_string subStep;
    taskinfo_t taskInfo;
    stub11.set(ADDR(CDB, ExecSql), ExecSqlTest);
    EXPECT_EQ(MP_FAILED, manager->QueryTaskInfo(taskID, subStep, taskInfo));
    stub11.set(ADDR(CDB, ExecSql), ExecSqlTestFail);
    // stub11.set(ADDR(CDB, QueryTable), QueryTableTest);
    manager->QueryTaskInfo(taskID, subStep, taskInfo);
    //EXPECT_EQ(MP_FAILED, manager->QueryTaskInfo(taskID, subStep, taskInfo));
}

TEST_F(TaskManagerTest, RemoveTaskFromMapTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    manager->RemoveTaskFromMap(taskID);
    TaskSub *aaaaataks = nullptr;
    EXPECT_EQ(MP_SUCCESS, manager->AddTask(taskID, &aataks));
    EXPECT_EQ(MP_SUCCESS, manager->AddTask("222", aaaaataks));
    manager->RemoveTaskFromMap(taskID);
    manager->RemoveTaskFromMap(taskID);
}

TEST_F(TaskManagerTest, GetAllRunningFromDBTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    manager->RemoveTaskFromMap(taskID);
    TaskSub *aaataks = nullptr;
    std::vector<taskinfo_t> tasksInfo;
    EXPECT_EQ(MP_FAILED, manager->GetAllRunningFromDB(tasksInfo));
}

TEST_F(TaskManagerTest, GetAllRunningFromDBTest2)
{
    LogTest();
    Stub stub11;
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    manager->RemoveTaskFromMap(taskID);
    TaskSub *aaataks = nullptr;
    std::vector<taskinfo_t> tasksInfo;
    stub11.set(ADDR(CDB, QueryTable), QueryTableTest);
    //EXPECT_EQ(MP_SUCCESS, manager->GetAllRunningFromDB(tasksInfo));
}

TEST_F(TaskManagerTest, CreateRedoTaskTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    manager->RemoveTaskFromMap(taskID);
    TaskSub *aaataks = nullptr;
    std::vector<taskinfo_t> tasksInfo;
    EXPECT_EQ(MP_FAILED, manager->CreateRedoTask());
    stub.set(ADDR(TaskManager, GetAllRunningFromDB), GetAllRunningFromDBTest);
    manager->CreateRedoTask();
    // taskinfo_t taskss;
    // tasksInfo.push_back(taskss);
    // EXPECT_EQ(MP_SUCCESS, manager->CreateRedoTask());
}

TEST_F(TaskManagerTest, RunTaskInThreadTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    mp_void* pThis = nullptr;
    //manager->RunTaskInThread(pThis);
    pThis = &aataks;
    manager->RunTaskInThread(pThis);
}

TEST_F(TaskManagerTest, CheckTaskExpirationTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    mp_void* pThis = nullptr;
    manager->CheckTaskExpiration(pThis);
    pThis = &aataks;
    aataks.m_taskExpiration = 0;
    manager->CheckTaskExpiration(pThis);
    aataks.m_taskExpiration = 1;
    manager->CheckTaskExpiration(pThis);
}


TEST_F(TaskManagerTest, RedoTaskInThreadTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    TaskManager* manager = TaskManager::GetInstance();
    mp_string taskID = "111";
    TaskSub aataks(taskID);
    mp_void* pThis = nullptr;
    manager->RedoTaskInThread(pThis);
    pThis = &aataks;
    manager->RedoTaskInThread(pThis);
}

TEST_F(TaskManagerTest, InitialTaskParamTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    mp_string msgBody;
    Json::Value bodyParam;
    mp_string connIp;
    mp_uint16 connPort; 
    mp_string taskId;
    InitialTaskParam(msgBody, bodyParam, connIp, connPort, taskId);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), ConvertStringtoJsonTest);
    EXPECT_EQ(MP_SUCCESS, InitialTaskParam(msgBody, bodyParam, connIp, connPort, taskId));
    stub.reset(ConvertStringtoJsonTest);
    InitialTaskParam(msgBody, bodyParam, connIp, connPort, taskId);
    stub.set((bool (Json::Value::*)(const char *) const)ADDR(Json::Value, isMember), stub_return_bool_StopResouceGroup);
    stub.set(ADDR(CJsonUtils, GetJsonString), SybaseStubJsonString);
    InitialTaskParam(msgBody, bodyParam, connIp, connPort, taskId);
}
/*
TEST_F(TaskManagerTest, CheckTaskInfoTest)
{
    LogTest();
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    bool isSync = MP_FALSE;
    mp_string tskName;
    mp_string taskId = "111";
    mp_int32 iCheckResult;
    EXPECT_EQ(MP_FALSE, CheckTaskInfo(isSync, tskName, taskId, iCheckResult));
    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTest);
    EXPECT_EQ(MP_TRUE, CheckTaskInfo(isSync, tskName, taskId, iCheckResult));
    stub.reset(QueryTaskInfoTest);

    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTestFailed);
    EXPECT_EQ(MP_FALSE, CheckTaskInfo(isSync, tskName, taskId, iCheckResult));
    stub.reset(QueryTaskInfoTestFailed);

    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTestCompleted);
    EXPECT_EQ(MP_TRUE, CheckTaskInfo(isSync, tskName, taskId, iCheckResult));
    stub.reset(QueryTaskInfoTestCompleted);
    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTestCompletedbb);
    EXPECT_EQ(MP_TRUE, CheckTaskInfo(isSync, tskName, taskId, iCheckResult));
    stub.reset(QueryTaskInfoTestCompletedbb);
}
*/
TEST_F(TaskManagerTest, CreateTaskTest)
{
    LogTest();
    Stub stub;
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    mp_string msgBody;
    mp_string connIp;
    mp_uint16 connPort;
    mp_string taskId;
    CreateTask<TaskSub>(msgBody, connIp, connPort, taskId);
    stub.set(InitialTaskParam, InitialTaskParamTest);
    CreateTask<TaskSub>(msgBody, connIp, connPort, taskId);
    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTest);
    CreateTask<TaskSub>(msgBody, connIp, connPort, taskId);
}

TEST_F(TaskManagerTest, RunSyncTaskTest)
{
    LogTest();
    Stub stub;
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    mp_string msgBody;
    mp_string taskId;
    Json::Value respMsg;
    RunSyncTask<TaskSub>(msgBody, taskId, respMsg);
    stub.set(ADDR(TaskManager, QueryTaskInfo), QueryTaskInfoTest);
    RunSyncTask<TaskSub>(msgBody, taskId, respMsg);
    stub.set(ADDR(CJsonUtils, ConvertStringtoJson), ConvertStringtoJsonTest);
    RunSyncTask<TaskSub>(msgBody, taskId, respMsg);
}

TEST_F(TaskManagerTest, DeleteExpireTasksTest)
{
       LogTest();
    Stub stub;
    DoGetJsonStringTest();
    TaskManager::m_pTaskManager = nullptr;
    mp_string msgBody;
    mp_string taskId;
    // DeleteExpireTasks(sqlDB, dps);
}