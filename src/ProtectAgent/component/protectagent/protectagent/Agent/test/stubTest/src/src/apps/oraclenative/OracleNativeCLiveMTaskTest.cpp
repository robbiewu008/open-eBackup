#include "apps/oraclenative/OracleNativeCLiveMTaskTest.h"


TEST_F(OracleNativeCLiveMTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeCLiveMTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
}

TEST_F(OracleNativeCLiveMTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeCLiveMTask task(taskID);
    task.CreateTaskStep();
}
