#include "apps/oraclenative/OracleNativeInstRestoreTaskTest.h"

#define private public
namespace {
mp_int32 InitTaskStepParamTest(const Json::Value& param, const mp_string& paramKey, const mp_string& stepName)
{
    return MP_SUCCESS;
}
};

TEST_F(OracleNativeInstRestoreTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeInstRestoreTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
    stub.set(ADDR(Task, InitTaskStepParam), InitTaskStepParamTest);
    task.InitTaskStep(param);
    stub.reset(ADDR(Task, InitTaskStepParam));
}

TEST_F(OracleNativeInstRestoreTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeInstRestoreTask task(taskID);
    task.CreateTaskStep();
}
