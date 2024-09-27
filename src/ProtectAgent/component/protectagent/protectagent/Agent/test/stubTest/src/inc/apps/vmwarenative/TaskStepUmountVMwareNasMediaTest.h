/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file TaskStepUmountVMwareNasMedia.h
 * @author w00558987
 * @brief 提供VMwareNative高级备份特性 NAS文件系统Umount功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#ifndef __AGENT_BACKUP_STEP_VMWARENATIVE_UMOUNT_VMWARENASMEDIA_H__
#define __AGENT_BACKUP_STEP_VMWARENATIVE_UMOUNT_VMWARENASMEDIA_H__

#include "apps/vmwarenative/TaskStepUmountVMwareNasMedia.h"
#include "gtest/gtest.h"
#include "stub.h"

class TaskStepUmountVMwareNasMediaTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void TaskStepUmountVMwareNasMediaTest::SetUp()
{}

void TaskStepUmountVMwareNasMediaTest::TearDown()
{}

void TaskStepUmountVMwareNasMediaTest::SetUpTestCase()
{}

void TaskStepUmountVMwareNasMediaTest::TearDownTestCase()
{}

#endif
