#include "apps/oraclenative/TaskStepOracleNativeInstRestoreTest.h"

using namespace std;
namespace {
mp_int32 ExecTesta(mp_void* pThis, mp_int32 iCommandID, mp_string strParam, vector<mp_string> pvecResult[],
    mp_int32 (*cb)(void*, const mp_string&), void* pTaskStep)
{
    return MP_SUCCESS;
}

mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

};

TEST_F(TaskStepOracleNativeInstRestoreTest, DismountRestoreInit)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeInstRestore dataBase(id, taskId, name, ratio , order);
    dataBase.Init(param);
    param["taskType"] = 1;
    param["params"] = "21";
    param["storage"] = "11";
    dataBase.Init(param);
}

TEST_F(TaskStepOracleNativeInstRestoreTest, DismountCancel)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeInstRestore dataBase(id, taskId, name, ratio , order);
    dataBase.Cancel();
}

TEST_F(TaskStepOracleNativeInstRestoreTest, RunTest)
{
    DoGetJsonStringTest();
    stub.set(ADDR(CRootCaller, Exec), ExecTesta);
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeInstRestore dataBase(id, taskId, name, ratio , order);
    dataBase.Run();
}

TEST_F(TaskStepOracleNativeInstRestoreTest, BuildScriptParamTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeInstRestore dataBase(id, taskId, name, ratio , order);
    dataBase.BuildScriptParam(name);
    mp_string strParam;

    dataBase.BuildScriptParam(strParam);
}