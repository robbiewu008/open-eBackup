/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeBackupCleanupTask.cpp
 * @author w00558987
 * @brief 提供VMwareNative高级备份资源清理功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#include "apps/vmwarenative/VMwareNativeBackupCleanupTaskTest.h"

TEST_F(VMwareNativeBackupCleanupTaskTest, InitTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupCleanupTask task(taskID);
    Json::Value param;
    // task.InitTaskStep(param);
}

TEST_F(VMwareNativeBackupCleanupTaskTest, UmountBackendNasMediaStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupCleanupTask task(taskID);
    Json::Value param;
    // task.UmountBackendNasMedia(param);
}

TEST_F(VMwareNativeBackupCleanupTaskTest, CreateTaskStepStub)
{
    mp_string taskID = "1";
    VMwareNativeBackupCleanupTask task(taskID);
    task.CreateTaskStep();
}