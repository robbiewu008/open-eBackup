/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include <stdio.h>
#include <unistd.h>
#include <iostream>
#include <memory>
#include <shared_mutex>
#include "config_reader/ConfigIniReader.h"
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "addr_pri.h"
#include "BackupStructs.h"
#include "BackupTimer.h"

using namespace std;
namespace {
    constexpr int RETRY_TIME_MILLISENCOND = 1000;
}

class BackupTimerTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void BackupTimerTest::SetUp()
{}

void BackupTimerTest::TearDown()
{}

void BackupTimerTest::SetUpTestCase()
{}

void BackupTimerTest::TearDownTestCase()
{}

TEST_F(BackupTimerTest, Insert) {
    BackupTimer backupTimerTest;
    FileHandle fileHandle1;
    std::vector<FileHandle> fileHandles;
    int64_t retryTimeInterval1 = 1;
    fileHandle1.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle1.m_file->m_fileName = "1.txt";

    FileHandle fileHandle2;
    int64_t retryTimeInterval2 = 2;
    fileHandle2.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle2.m_file->m_fileName = "2.txt";

    backupTimerTest.Insert(fileHandle1, retryTimeInterval1 * RETRY_TIME_MILLISENCOND);
    backupTimerTest.Insert(fileHandle2, retryTimeInterval2 * RETRY_TIME_MILLISENCOND);

    sleep(retryTimeInterval1);
    int time = backupTimerTest.GetExpiredEventAndTime(fileHandles);
    EXPECT_EQ(fileHandles.size(), 1);
    EXPECT_EQ(fileHandles[0].m_file->m_fileName, "1.txt");

    fileHandles.resize(0);
    sleep(retryTimeInterval2 - retryTimeInterval1);
    time = backupTimerTest.GetExpiredEventAndTime(fileHandles);
    EXPECT_EQ(fileHandles.size(), 1);
    EXPECT_EQ(fileHandles[0].m_file->m_fileName, "2.txt");
}
