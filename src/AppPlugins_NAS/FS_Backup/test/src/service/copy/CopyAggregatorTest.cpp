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
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "copy/CopyAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class CopyAggregatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<CopyAggregator> m_copyAggregator = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void CopyAggregatorTest::SetUp()
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

    m_copyAggregator = make_unique<CopyAggregator>(m_backupParams, m_aggregateQueue,
        m_writeQueue, m_readQueue, m_controlInfo, m_blockBufferMap);
}

void CopyAggregatorTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CopyAggregatorTest::SetUpTestCase()
{}

void CopyAggregatorTest::TearDownTestCase()
{}

TEST_F(CopyAggregatorTest, Start)
{
    m_copyAggregator->m_fileAggregator = make_unique<FileAggregator>(m_backupParams, m_writeQueue,
        m_readQueue, m_blockBufferMap, m_controlInfo);

    MOCKER_CPP(&CopyAggregator::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::FAILED));
    //EXPECT_EQ(m_copyAggregator->Start(), BackupRetCode::SUCCESS);

    unique_ptr<CopyAggregator> m_copyAggregator1 = make_unique<CopyAggregator>(m_backupParams,
        m_aggregateQueue, m_writeQueue, m_readQueue, m_controlInfo, m_blockBufferMap);

    MOCKER_CPP(&CopyAggregator::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED));

    //EXPECT_EQ(m_copyAggregator1->Start(), BackupRetCode::FAILED);
}

TEST_F(CopyAggregatorTest, Abort)
{
    MOCKER_CPP(&FileAggregator::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_copyAggregator->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(CopyAggregatorTest, GetStatus)
{
    m_copyAggregator->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(m_copyAggregator->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_copyAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_copyAggregator->m_abort = true;
    EXPECT_EQ(m_copyAggregator->GetStatus(), BackupPhaseStatus::ABORTED);

    m_copyAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_copyAggregator->m_abort = false;
    m_copyAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_copyAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_copyAggregator->GetStatus(), BackupPhaseStatus::FAILED);

    m_copyAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_copyAggregator->m_abort = false;
    m_copyAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_copyAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_copyAggregator->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(CopyAggregatorTest, IsAbort)
{
    m_copyAggregator->m_abort = false;
    m_copyAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_copyAggregator->IsAbort(), true);

    m_copyAggregator->m_abort = false;
    m_copyAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_copyAggregator->IsAbort(), false);
}

TEST_F(CopyAggregatorTest, IsComplete)
{
    m_copyAggregator->m_fileAggregator = make_unique<FileAggregator>(m_backupParams, m_writeQueue,
        m_readQueue, m_blockBufferMap, m_controlInfo);
    m_copyAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_copyAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_copyAggregator->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_copyAggregator->IsComplete(), true);

    m_copyAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_copyAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_copyAggregator->m_controlInfo->m_readProduce = 1;
    m_copyAggregator->m_fileAggregator->m_aggTaskProduce = 1;
    m_copyAggregator->m_fileAggregator->m_aggTaskConsume = 1;
    m_copyAggregator->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_copyAggregator->IsComplete(), true);

    m_copyAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_copyAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_copyAggregator->m_controlInfo->m_readProduce = 4;
    EXPECT_EQ(m_copyAggregator->IsComplete(), false);
}

TEST_F(CopyAggregatorTest, PushToWriter)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    m_copyAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_copyAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_copyAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&CopyAggregator::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&CopyAggregator::IsComplete)
            .stubs()
            .will(returnValue(true));
    m_copyAggregator->PushToWriter(fileHandle);
}

TEST_F(CopyAggregatorTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    m_copyAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_copyAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_copyAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&CopyAggregator::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&CopyAggregator::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&FileAggregator::Aggregate)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::PollAggregateTask)
            .stubs()
            .will(ignoreReturnValue());
    m_copyAggregator->ThreadFunc();

    unique_ptr<CopyAggregator> m_copyAggregator1 = make_unique<CopyAggregator>(m_backupParams,
        m_aggregateQueue, m_writeQueue, m_readQueue, m_controlInfo, m_blockBufferMap);
    m_copyAggregator1->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_copyAggregator1->m_controlInfo = make_shared<BackupControlInfo>();
    m_copyAggregator1->m_aggregateQueue->WaitAndPush(fileHandle);
    m_copyAggregator1->ThreadFunc();
}

TEST_F(CopyAggregatorTest, WaitForAggregatorStart)
{
    MOCKER_CPP(&FileAggregator::IsAggregateStarted)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&CopyAggregator::IsAbort)
            .stubs()
            .will(returnValue(true));
    m_copyAggregator->WaitForAggregatorStart();
}