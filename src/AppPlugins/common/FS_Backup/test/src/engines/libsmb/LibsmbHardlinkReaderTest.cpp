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
#include <shared_mutex>
#include "config_reader/ConfigIniReader.h"
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbHardlinkReader.h"

using namespace std;

class LibsmbHardlinkReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<LibsmbHardlinkReader> m_libsmbHardlinkReader = nullptr;
};

void LibsmbHardlinkReaderTest::SetUp()
{
    BackupQueueConfig config {100, 1 * 1024 * 1024};;
    BackupParams params;
    params.srcEngine = BackupIOEngine::LIBSMB;
    params.dstEngine = BackupIOEngine::LIBSMB;
    params.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    ReaderParams hardlinkReaderParams {};
    hardlinkReaderParams.backupParams = params;
    hardlinkReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libsmbHardlinkReader = std::make_unique<LibsmbHardlinkReader>(hardlinkReaderParams);
}

void LibsmbHardlinkReaderTest::TearDown()
{}

void LibsmbHardlinkReaderTest::SetUpTestCase()
{}

void LibsmbHardlinkReaderTest::TearDownTestCase()
{}

/*
* 用例名称：输入参数，测试ArchiveHardlinkReader的启动
* 前置条件：文件读队列为空
* check点：ArchiveHardlinkReader启动正常
*/
TEST_F(LibsmbHardlinkReaderTest, IsReaderRequestReachThreshold)
{
    // 没有计数
    EXPECT_EQ(m_libsmbHardlinkReader->IsReaderRequestReachThreshold(), false);
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
    fileHandle.m_file->m_fileName = "hardlink_test.txt";
    fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
    fileHandle.m_file->m_size = 200 * 1024 * 1024; // 200M
    fileHandle.m_block.m_size = 200 * 1024 * 1024; // 200M
    m_libsmbHardlinkReader->m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    EXPECT_EQ(m_libsmbHardlinkReader->IsReaderRequestReachThreshold(), true);
    m_libsmbHardlinkReader->m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
    for (int i = 0; i < 100; ++i) {
        m_libsmbHardlinkReader->m_pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::SENT);
    }
    m_libsmbHardlinkReader->m_pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::RECVD);
    m_libsmbHardlinkReader->m_pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::SENT);
    m_libsmbHardlinkReader->m_pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD);
    EXPECT_EQ(m_libsmbHardlinkReader->IsReaderRequestReachThreshold(), false);
    for (int i = 0; i < 200; ++i) {
        m_libsmbHardlinkReader->m_pktStats->Increment(PKT_TYPE::READ, PKT_COUNTER::SENT);
    }
    EXPECT_EQ(m_libsmbHardlinkReader->IsReaderRequestReachThreshold(), true);
}
