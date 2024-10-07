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
#include <iostream>
#include <memory>
#include "model/BlockBufferMap.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "addr_pri.h"
#include "BackupMgr.h"
#include "Backup.h"

using namespace std;
using namespace FS_Backup;

class BlockBufferMapTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
};

void BlockBufferMapTest::SetUp()
{}

void BlockBufferMapTest::TearDown()
{}

void BlockBufferMapTest::SetUpTestCase()
{}

void BlockBufferMapTest::TearDownTestCase()
{}


TEST_F(BlockBufferMapTest, BlockBufferMapOperationTest)
{
    BlockBufferMap bufferMap {};
    FileHandle fileHandle {};
    string filename1 = "filename1";
    string filename2 = "filename2";
    EXPECT_TRUE(bufferMap.Empty());
    bufferMap.Add(filename1, fileHandle);
    bufferMap.Add(filename2, fileHandle);
    bufferMap.Delete(filename1);
    bufferMap.Delete(filename2, fileHandle);
    EXPECT_NO_THROW(bufferMap.GetTotalBufferSize());
}

TEST_F(BlockBufferMapTest, BlockBufferMapQueueOperationTest)
{
    BlockBufferMapQueue  blockBufferMapQueue {};
    FileHandle fileHandle {};
    EXPECT_TRUE(blockBufferMapQueue.Empty());
    EXPECT_EQ(blockBufferMapQueue.Size(), 0);
    blockBufferMapQueue.Push(fileHandle);
    blockBufferMapQueue.GetAndPop();
    blockBufferMapQueue.Get(fileHandle);
    blockBufferMapQueue.GetAndPop(fileHandle);
    EXPECT_TRUE(blockBufferMapQueue.Empty());
}

