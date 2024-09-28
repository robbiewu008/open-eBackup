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
#include <stdexcept>
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "dir/DirAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class DirAggregatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<DirAggregator> m_dirAggregator = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DirAggregatorTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    CommonParams commonParams {};
    commonParams.metaPath = "/xx-dir/";
    commonParams.jobId = "qqqqqqqqqq";
    commonParams.subJobId = "wwwwwwwwwwwwwww";
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    m_backupParams.commonParams = commonParams;

    m_dirAggregator = make_unique<DirAggregator>(m_backupParams, m_aggregateQueue,
        m_writeQueue, m_controlInfo);
}

void DirAggregatorTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DirAggregatorTest::SetUpTestCase()
{}

void DirAggregatorTest::TearDownTestCase()
{}

TEST_F(DirAggregatorTest, Start)
{
    MOCKER_CPP(&DirAggregator::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_dirAggregator->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DirAggregatorTest, Abort)
{
    EXPECT_EQ(m_dirAggregator->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DirAggregatorTest, GetStatus)
{
    m_dirAggregator->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(m_dirAggregator->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_dirAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_dirAggregator->m_abort = true;
    EXPECT_EQ(m_dirAggregator->GetStatus(), BackupPhaseStatus::ABORTED);

    m_dirAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_dirAggregator->m_abort = false;
    m_dirAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_dirAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_dirAggregator->GetStatus(), BackupPhaseStatus::FAILED);

    m_dirAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_dirAggregator->m_abort = false;
    m_dirAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_dirAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_dirAggregator->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(DirAggregatorTest, IsAbort)
{
    m_dirAggregator->m_abort = false;
    m_dirAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_dirAggregator->IsAbort(), true);

    m_dirAggregator->m_abort = false;
    m_dirAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_dirAggregator->IsAbort(), false);
}

TEST_F(DirAggregatorTest, IsComplete)
{
    m_dirAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_dirAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_dirAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_dirAggregator->m_controlInfo->m_readProduce = 1;
    m_dirAggregator->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_dirAggregator->IsComplete(), true);

    m_dirAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_dirAggregator->m_controlInfo->m_readProduce = 4;
    EXPECT_EQ(m_dirAggregator->IsComplete(), false);
}

TEST_F(DirAggregatorTest, PushToWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    //m_dirAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    //m_dirAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    //m_dirAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&DirAggregator::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&DirAggregator::IsComplete)
            .stubs()
            .will(returnValue(true));
    m_dirAggregator->PushToWriteQueue(fileHandle);
}

TEST_F(DirAggregatorTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    m_dirAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_dirAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_dirAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&DirAggregator::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&DirAggregator::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    m_dirAggregator->ThreadFunc();

    unique_ptr<DirAggregator> m_dirAggregator1 =
        make_unique<DirAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    m_dirAggregator1->m_aggregateQueue->WaitAndPush(fileHandle);
    MOCKER_CPP(&DirAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    m_dirAggregator1->ThreadFunc();
}
