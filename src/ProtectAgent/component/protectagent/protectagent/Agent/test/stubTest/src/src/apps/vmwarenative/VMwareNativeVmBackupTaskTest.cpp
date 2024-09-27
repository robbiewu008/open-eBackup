#include "apps/vmwarenative/VMwareNativeVmBackupTaskTest.h"

TEST_F(VMwareNativeVmBackupTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeVmBackupTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeVmBackupTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeVmBackupTask task(taskID);
    task.CreateTaskStep();
}