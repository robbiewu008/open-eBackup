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
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "delete/DeleteAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class DeleteAggregatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<DeleteAggregator> m_deleteAggregator = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DeleteAggregatorTest::SetUp()
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

    m_deleteAggregator = make_unique<DeleteAggregator>(m_backupParams, m_aggregateQueue,
        m_writeQueue, m_controlInfo);
}

void DeleteAggregatorTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DeleteAggregatorTest::SetUpTestCase()
{}

void DeleteAggregatorTest::TearDownTestCase()
{}

TEST_F(DeleteAggregatorTest, Start)
{
    MOCKER_CPP(&DeleteAggregator::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_deleteAggregator->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteAggregatorTest, Abort)
{
    EXPECT_EQ(m_deleteAggregator->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteAggregatorTest, GetStatus)
{
    m_deleteAggregator->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(m_deleteAggregator->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_deleteAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_deleteAggregator->m_abort = true;
    EXPECT_EQ(m_deleteAggregator->GetStatus(), BackupPhaseStatus::ABORTED);

    m_deleteAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_deleteAggregator->m_abort = false;
    m_deleteAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_deleteAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_deleteAggregator->GetStatus(), BackupPhaseStatus::FAILED);

    m_deleteAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_deleteAggregator->m_abort = false;
    m_deleteAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_deleteAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_deleteAggregator->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(DeleteAggregatorTest, IsAbort)
{
    m_deleteAggregator->m_abort = false;
    m_deleteAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_deleteAggregator->IsAbort(), true);

    m_deleteAggregator->m_abort = false;
    m_deleteAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_deleteAggregator->IsAbort(), false);
}

TEST_F(DeleteAggregatorTest, IsComplete)
{
    m_deleteAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_deleteAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_deleteAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_deleteAggregator->m_controlInfo->m_readProduce = 1;
    m_deleteAggregator->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_deleteAggregator->IsComplete(), true);

    m_deleteAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_deleteAggregator->m_controlInfo->m_readProduce = 4;
    EXPECT_EQ(m_deleteAggregator->IsComplete(), false);
}

TEST_F(DeleteAggregatorTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    m_deleteAggregator->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_deleteAggregator->m_controlInfo = make_shared<BackupControlInfo>();
    m_deleteAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&DeleteAggregator::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&DeleteAggregator::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true));
    m_deleteAggregator->ThreadFunc();

    unique_ptr<DeleteAggregator> m_deleteAggregator1 =
        make_unique<DeleteAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    m_deleteAggregator1->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_deleteAggregator1->m_controlInfo = make_shared<BackupControlInfo>();
    //m_deleteAggregator1->m_aggregateQueue->WaitAndPush(fileHandle);
    m_deleteAggregator1->ThreadFunc();

    unique_ptr<DeleteAggregator> m_deleteAggregator2 =
        make_unique<DeleteAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    m_deleteAggregator2->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_deleteAggregator2->m_controlInfo = make_shared<BackupControlInfo>();
    m_deleteAggregator2->m_aggregateQueue->WaitAndPush(fileHandle);
    m_deleteAggregator2->ThreadFunc();
}
