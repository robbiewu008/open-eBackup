#include "apps/vmwarenative/VMwareNativeVmRestoreTaskTest.h"

TEST_F(VMwareNativeVmRestoreTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeVmRestoreTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeVmRestoreTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeVmRestoreTask task(taskID);
    task.CreateTaskStep();
}