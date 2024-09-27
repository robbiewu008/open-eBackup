#include "taskmanager/TaskStepPrepareNasMediaTest.h"
#include "taskmanager/TaskStepPrepareNasMedia.h"
#include "common/ErrorCode.h"
#include <vector>

static mp_void StubCLoggerLog(mp_void){
    return;
}


TEST_F(TaskStepPrepareNasMediaTest, InitTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);

    Json::Value param;
    a.Init(param);
}

TEST_F(TaskStepPrepareNasMediaTest, RunTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    a.Run();
}

TEST_F(TaskStepPrepareNasMediaTest, CancelTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    a.Cancel();
}

TEST_F(TaskStepPrepareNasMediaTest, RedoTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    mp_string innerPID;
    a.Redo(innerPID);
}

TEST_F(TaskStepPrepareNasMediaTest, StopTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    Json::Value param;
    a.Stop(param);
}

TEST_F(TaskStepPrepareNasMediaTest, UpdateTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    Json::Value param;
    a.Update(param);
}

TEST_F(TaskStepPrepareNasMediaTest, FinishTest)
{
    mp_string id = "";
    mp_string taskId = "";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    Json::Value param;
    a.Finish(param);
}

mp_int32 ExecTest(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_int32 ExecTest_FAILED(mp_int32 iCommandID, mp_string strParam, std::vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_FAILED;
}

TEST_F(TaskStepPrepareNasMediaTest, MountNasMediaTest)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id = "";
    mp_string taskId = "123";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    mp_string scriptParam = "abcd";
    std::vector<mp_string> vecRst;
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    stub.set(ADDR(CRootCaller, Exec), ExecTest); 
    a.MountNasMedia(scriptParam, vecRst);
}


TEST_F(TaskStepPrepareNasMediaTest, UmountNasMedia)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id = "";
    mp_string taskId = "123";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    mp_string scriptParam = "abcd";
    std::vector<mp_string> vecRst;
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    stub.set(ADDR(CRootCaller, Exec), ExecTest); 
    a.UmountNasMedia(scriptParam, vecRst);
}

/*
* 用例名称：检查Dataturbo链路状态
* 观测点：1、IP为空时，返回 ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT
*        2、脚本返回失败，返回 ERR_CREATE_DATA_TURBO_LINK
*        3、脚本返回成功，返回 MP_SUCCESS
*/
TEST_F(TaskStepPrepareNasMediaTest, CheckAndCreateDataturboLink)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id = "";
    mp_string taskId = "123";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    DataturboMountParam param;
    param.authPwd = "test";
    param.authUser = "test";
    param.storageName = "test";
    stub.set(ADDR(CRootCaller, Exec), ExecTest);
    mp_int32 iRet = a.CheckAndCreateDataturboLink(param);
    EXPECT_EQ(ERR_NOT_CONFIG_DATA_TURBO_LOGIC_PORT, iRet);

    param.vecDataturboIP.push_back("1.2.3.4");
    iRet = a.CheckAndCreateDataturboLink(param);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(CRootCaller, Exec), ExecTest_FAILED);
    iRet = a.CheckAndCreateDataturboLink(param);
    EXPECT_EQ(ERR_CREATE_DATA_TURBO_LINK, iRet);

    stub.reset(ADDR(CRootCaller, Exec));
}

mp_int32 Stub_CheckAndCreateDataturboLink(const DataturboMountParam &param)
{
    return MP_SUCCESS;
}

mp_int32 Stub_CheckAndCreateDataturboLinkFailed(const DataturboMountParam &param)
{
    return MP_FAILED;
}

/*
* 用例名称：挂载Dataturbo
* 观测点：1、链路检查失败，返回失败
*        2、脚本返回失败，返回MP_FAILED
*        3、脚本返回成功，返回MP_SUCCESS
*/
TEST_F(TaskStepPrepareNasMediaTest, MountDataturboMedia)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_string id = "";
    mp_string taskId = "123";
    mp_string name = "";
    mp_int32 ratio = 0;
    mp_int32 order = 0;
    stub.set(ADDR(TaskStepPrepareNasMedia, CheckAndCreateDataturboLink), Stub_CheckAndCreateDataturboLink);
    TaskStepPrepareNasMedia a(id, taskId, name, ratio, order);
    mp_string scriptParam = "abcd";
    DataturboMountParam param;
    std::vector<mp_string> vecRst;
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    vecRst.push_back(scriptParam);
    stub.set(ADDR(CRootCaller, Exec), ExecTest); 
    mp_int32 iRet = a.MountDataturboMedia(scriptParam, vecRst, param);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(TaskStepPrepareNasMedia, CheckAndCreateDataturboLink), Stub_CheckAndCreateDataturboLinkFailed);
    iRet = a.MountDataturboMedia(scriptParam, vecRst, param);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CRootCaller, Exec), ExecTest_FAILED); 
    iRet = a.MountDataturboMedia(scriptParam, vecRst, param);
    EXPECT_EQ(MP_FAILED, iRet); 
    stub.reset(ADDR(CRootCaller, Exec));
    stub.reset(ADDR(TaskStepPrepareNasMedia, CheckAndCreateDataturboLink));
}
