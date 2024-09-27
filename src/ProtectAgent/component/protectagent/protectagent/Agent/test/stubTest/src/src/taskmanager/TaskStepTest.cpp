#include "taskmanager/TaskStepTest.h"
#include "taskmanager/TaskStep.h"
#include "securecom/RootCaller.h"
namespace {
mp_void LogTest() {}
#define DoGetJsonStringTest() do { \
    stub1.set(ADDR(CLogger, Log), LogTest); \
} while (0)
mp_int32 GetBackupParamS(const mp_string& paramID, const mp_string& paramKey, mp_string& paramValue)
{
    return MP_SUCCESS;
}

mp_int32 ReadResultFileTest(mp_void* pThis, mp_int32 iCommandID, const mp_string& strUniqueID, std::vector<mp_string> pvecResult[])
{
    return MP_SUCCESS;
}

mp_int32 ReadResultFileTest1(mp_void* pThis, mp_int32 iCommandID, const mp_string& strUniqueID, std::vector<mp_string> pvecResult[])
{
    mp_string str;
    (*pvecResult).push_back(str);
    return MP_SUCCESS;
}
mp_int32 QueryTableF(
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    return MP_FAILED;
}
mp_int32 QueryTableS(
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    return MP_FAILED;
}
mp_int32 ExecTest(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 GetParamCountTest(const mp_string& paramID, const mp_string& paramKey, mp_int32 &num)
{
    return MP_SUCCESS;
}

mp_int32 GetParamCountTest1(const mp_string& paramID, const mp_string& paramKey, mp_int32 &num)
{
    num = 1;
    return MP_SUCCESS;
}
mp_int32 ExecSqlTest(const mp_string& strSql, DbParamStream& dps)
{
    return MP_SUCCESS;
}
mp_int32 ExecSqlFailTest(const mp_string& strSql, DbParamStream& dps)
{
    return MP_FAILED;
}

mp_int32 StubCMpThreadWaitForEndEq0(thread_id_t* id, mp_void** retValue)
{
    return 0;
}
FILE* StubPopenNull(mp_void* pthis)
{
    FILE* pStream = nullptr;
    return pStream;
}

static mp_int32 StubExecSystemWithEcho(const mp_string& strCommand, std::vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    return MP_SUCCESS;
}
}

class TaskStepSub : public TaskStep {
public:
    TaskStepSub(const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order) :
        TaskStep(id, taskId, name, ratio, order){}
    virtual mp_int32 Init(const Json::Value& param) {};
    virtual mp_int32 Run() {};
    virtual mp_int32 Cancel() {};
    virtual mp_int32 Cancel(Json::Value& respParam) {};
    virtual mp_int32 Stop(const Json::Value& param) {};
    virtual mp_int32 Update(const Json::Value& param) {};
    virtual mp_int32 Update(Json::Value& param, Json::Value& respParam) {};
    virtual mp_int32 Finish(const Json::Value& param) {};
    virtual mp_int32 Finish(Json::Value& param, Json::Value& respParam) {};
    virtual mp_int32 Redo(mp_string& innerPID) {};
};

TEST_F(TaskStepTest, WaitScriptProcessFinishTest)
{
    DoGetJsonStringTest();
    Stub stub;
    stub.set(&CMpThread::WaitForEnd, StubCMpThreadWaitForEndEq0);
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    mp_string strUniqueID;
    stub.set(ADDR(CRootCaller, ReadResultFile), ReadResultFileTest);
    EXPECT_EQ(MP_FAILED, taskaa.WaitScriptProcessFinish(strUniqueID));

    stub.set(ADDR(CRootCaller, ReadResultFile), ReadResultFileTest1);
    stub.set(ADDR(CSystemExec, ExecSystemWithEcho), StubExecSystemWithEcho);
    EXPECT_EQ(MP_SUCCESS, taskaa.WaitScriptProcessFinish(strUniqueID));
}

TEST_F(TaskStepTest, CheckScriptResultTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    mp_string strUniqueID;
    taskaa.CheckScriptResult(strUniqueID);
    stub.set(ADDR(CRootCaller, ReadResultFile), ReadResultFileTest);
    taskaa.CheckScriptResult(strUniqueID);
    stub.reset(ADDR(CRootCaller, ReadResultFile));
    taskaa.CheckScriptResult(strUniqueID);
    stub.set(ADDR(CRootCaller, ReadResultFile), ReadResultFileTest1);
    taskaa.CheckScriptResult(strUniqueID);
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
}

TEST_F(TaskStepTest, UpdateParamTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    mp_string paramID;
    mp_string paramKey;
    mp_string paramValue;
    taskaa.UpdateParam(paramID, paramKey, paramValue);
    stub.set(ADDR(TaskStep, GetParamCount), GetParamCountTest);
    taskaa.UpdateParam(paramID, paramKey, paramValue);
    stub.reset(ADDR(TaskStep, GetParamCount));
    //stub.set(ADDR(TaskStep, GetParamCount), GetParamCountTest1);
    stub.set(ADDR(CDB, ExecSql), ExecSqlFailTest);
    taskaa.UpdateParam(paramID, paramKey, paramValue);
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    stub.reset(ADDR(CRootCaller, ReadResultFile));
    stub.set(ADDR(CRootCaller, ReadResultFile), ReadResultFileTest1);
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
    taskaa.UpdateParam(paramID, paramKey, paramValue);
}

TEST_F(TaskStepTest, RemoveParamTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    stub.set(ADDR(CDB, ExecSql), ExecSqlFailTest);
    taskaa.RemoveParam(id);
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    taskaa.RemoveParam(id);
}

TEST_F(TaskStepTest, GetBackupParamTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    mp_string paramID;
    mp_string paramKey;
    mp_string paramValue;
    stub.set(ADDR(CDB, QueryTable), QueryTableF);
    taskaa.GetBackupParam(paramID, paramKey, paramValue);

    stub.reset(ADDR(CDB, QueryTable));
    stub.set(ADDR(CDB, QueryTable), QueryTableS);
    taskaa.GetBackupParam(paramID, paramKey, paramValue);
}

TEST_F(TaskStepTest, BuildDataLogPathTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    std::ostringstream oss;
    mp_string paramID;
    taskaa.BuildDataLogPath(oss, paramID);
    stub.set(ADDR(TaskStepSub, GetBackupParam), GetBackupParamS);
    taskaa.BuildDataLogPath(oss, paramID);
}
/*
TEST_F(TaskStepTest, UpdateInnerPIDCallBackTest)
{
    DoGetJsonStringTest();
    Stub stub;
    mp_string id;
    const mp_string taskId;
    const mp_string name;
    mp_int32 ratio; 
    mp_int32 order;
    TaskStepSub taskaa(id, taskId, name, ratio, order);
    void* pPointer;
    mp_string innerPID;
    taskaa.UpdateInnerPIDCallBack(pPointer, innerPID);
    pPointer = &taskaa;
    stub.set(ADDR(CDB, ExecSql), ExecSqlFailTest);
    taskaa.UpdateInnerPIDCallBack(pPointer, innerPID);
    // stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    // taskaa.UpdateInnerPIDCallBack(pPointer, innerPID);
}
*/