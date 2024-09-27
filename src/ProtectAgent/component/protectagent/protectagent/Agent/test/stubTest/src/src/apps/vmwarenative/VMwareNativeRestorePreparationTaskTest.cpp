#include "apps/vmwarenative/VMwareNativeRestorePreparationTaskTest.h"

TEST_F(VMwareNativeRestorePreparationTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, PrepareIscsiMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.PrepareIscsiMedia(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, PrepareNasMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    Json::Value param;
    // task.PrepareNasMedia(param);
}

TEST_F(VMwareNativeRestorePreparationTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeRestorePreparationTask task(taskID);
    task.CreateTaskStep();
}