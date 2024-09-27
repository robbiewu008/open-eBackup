#include "apps/oraclenative/OracleNativePrepareMediaTaskTest.h"


TEST_F(OracleNativePrepareMediaTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.InitPrepareMedia(param);
}

TEST_F(OracleNativePrepareMediaTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    task.CreateTaskStep();
}

TEST_F(OracleNativePrepareMediaTaskTest, InitTaskStepStubA)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.InitTaskStep(param);
}


TEST_F(OracleNativePrepareMediaTaskTest, InitPrepareMediaStub)
{
    mp_string taskID = "1";
    OracleNativePrepareMediaTask task(taskID);
    Json::Value param;
    task.taskType = 0;
    task.InitPrepareMedia(param);
}