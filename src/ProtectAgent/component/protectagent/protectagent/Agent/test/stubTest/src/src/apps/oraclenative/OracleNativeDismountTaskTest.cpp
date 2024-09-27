#include "apps/oraclenative/OracleNativeDismountTaskTest.h"


TEST_F(OracleNativeDismountTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeDismountTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
}

TEST_F(OracleNativeDismountTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativeDismountTask task(taskID);
    task.CreateTaskStep();
}
