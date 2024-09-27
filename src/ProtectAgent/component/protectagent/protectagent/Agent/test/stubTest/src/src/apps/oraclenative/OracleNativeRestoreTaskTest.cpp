#include "apps/oraclenative/OracleNativeRestoreTaskTest.h"


using namespace std;
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 InitTaskStepParamTest(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName)
{
    return MP_SUCCESS;
}
};

TEST_F(OracleNativeRestoreTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeRestoreTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
    stub.set(ADDR(Task, InitTaskStepParam), InitTaskStepParamTest);
    task.InitTaskStep(param);
    stub.reset(ADDR(Task, InitTaskStepParam));
}

TEST_F(OracleNativeRestoreTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeRestoreTask task(taskID);
    task.CreateTaskStep();
}
