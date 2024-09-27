#include "apps/oraclenative/TaskStepOracleNativeDismountTest.h"
#include "apps/oraclenative/TaskStepOracleNativeDismount.h"
#include "gtest/gtest.h"
#include "stub.h"
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

TEST_F(TaskStepOracleNativeDismountTest, DismountInit)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeDismount dataBase(id, taskId, name, ratio , order);
    dataBase.Init(param);
    param["taskType"] = 1;
    param["params"] = "21";
    param["storage"] = "11";
    dataBase.Init(param);
}

TEST_F(TaskStepOracleNativeDismountTest, ParseVolumeParameterTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeDismount dataBase(id, taskId, name, ratio , order);
    dataBase.ParseVolumeParameter(param);
    param["dataVolumes"] = "11";
    dataBase.ParseVolumeParameter(param);
    Json::Value paramEE;
    param["logVolumes"].append(paramEE);
    dataBase.ParseVolumeParameter(param);
}

TEST_F(TaskStepOracleNativeDismountTest, RunTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeDismount dataBase(id, taskId, name, ratio , order);
    dataBase.Run();
}

TEST_F(TaskStepOracleNativeDismountTest, BuildScriptParamTest)
{
    DoGetJsonStringTest();
    Json::Value param;
    mp_string id;
    mp_string taskId;
    mp_string name;
    mp_int32 ratio;
    mp_int32 order;
    TaskStepOracleNativeDismount dataBase(id, taskId, name, ratio , order);
    dataBase.BuildScriptParam(name);
    mp_string strParam;
    dataBase.ipList.push_back("111");
    dataBase.BuildScriptParam(strParam);
}