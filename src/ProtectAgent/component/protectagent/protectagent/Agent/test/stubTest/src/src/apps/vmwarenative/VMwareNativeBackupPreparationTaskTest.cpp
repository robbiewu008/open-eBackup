#include "apps/vmwarenative/VMwareNativeBackupPreparationTaskTest.h"

TEST_F(VMwareNativeBackupPreparationTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupPreparationTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeBackupPreparationTaskTest, PrepareIscsiMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupPreparationTask task(taskID);
    Json::Value param;
    // task.PrepareIscsiMedia(param);
}

TEST_F(VMwareNativeBackupPreparationTaskTest, PrepareNasMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupPreparationTask task(taskID);
    Json::Value param;
    // task.PrepareNasMedia(param);
}

TEST_F(VMwareNativeBackupPreparationTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupPreparationTask task(taskID);
    task.CreateTaskStep();
}