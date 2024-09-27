/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeBackupCleanupTask.h
 * @author w00558987
 * @brief 提供VMwareNative高级备份资源清理功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#ifndef __AGENT_VMWARENATIVE_BACKUP_CLEANUP_TASK__
#define __AGENT_VMWARENATIVE_BACKUP_CLEANUP_TASK__
#define private public
#include "apps/vmwarenative/VMwareNativeBackupCleanupTask.h"
#include "gtest/gtest.h"
#include "stub.h"

class VMwareNativeBackupCleanupTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

private:
    Stub stub;
};

void VMwareNativeBackupCleanupTaskTest::SetUp()
{}

void VMwareNativeBackupCleanupTaskTest::TearDown()
{}

void VMwareNativeBackupCleanupTaskTest::SetUpTestCase()
{}

void VMwareNativeBackupCleanupTaskTest::TearDownTestCase()
{}

#endif
