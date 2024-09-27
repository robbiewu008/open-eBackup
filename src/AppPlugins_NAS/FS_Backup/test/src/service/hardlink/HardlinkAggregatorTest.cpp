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
#include "hardlink/HardlinkAggregator.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
    const uint16_t MAX_JOB_SCHDULER_TEST = 1;
}

class HardlinkAggregatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<HardlinkAggregator> m_hardlinkAggregator = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<HardLinkMap> m_hardlinkMap                 = make_shared<HardLinkMap>();
};

void HardlinkAggregatorTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_INC;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    CommonParams commonParams {};
    commonParams.metaPath = "/xx-dir/";
    commonParams.jobId = "qqqqqqqqqq";
    commonParams.subJobId = "subJobId";
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    m_backupParams.commonParams = commonParams;

    m_hardlinkAggregator =make_unique<HardlinkAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
}

void HardlinkAggregatorTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void HardlinkAggregatorTest::SetUpTestCase()
{}

void HardlinkAggregatorTest::TearDownTestCase()
{}

static shared_ptr<AggregateHardlinkDirInfo> GetDirInfo_Stub1()
{
    shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = nullptr;
    return aggregateDirInfo;
}

static shared_ptr<AggregateHardlinkDirInfo> GetDirInfo_Stub2()
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";
    fileHandle.m_file->m_fileCount = 1;

    shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = make_shared<AggregateHardlinkDirInfo>();
    aggregateDirInfo->m_hardlinkFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_dirName = fileHandle.m_file->m_dirName;
    aggregateDirInfo->m_totalHardlinkFilesCount = fileHandle.m_file->m_fileCount;
    aggregateDirInfo->m_readCompletedFilesCount = 1;
    aggregateDirInfo->m_hardlinkFiles->push_back(fileHandle);
    return aggregateDirInfo;
}

TEST_F(HardlinkAggregatorTest, Abort)
{
    EXPECT_EQ(m_hardlinkAggregator->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkAggregatorTest, GetStatus)
{
    m_hardlinkAggregator->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_EQ(m_hardlinkAggregator->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_hardlinkAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_hardlinkAggregator->m_abort = true;
    EXPECT_EQ(m_hardlinkAggregator->GetStatus(), BackupPhaseStatus::ABORTED);

    m_hardlinkAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_hardlinkAggregator->m_abort = false;
    m_hardlinkAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_hardlinkAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_hardlinkAggregator->GetStatus(), BackupPhaseStatus::FAILED);

    m_hardlinkAggregator->m_controlInfo->m_aggregatePhaseComplete = true;
    m_hardlinkAggregator->m_abort = false;
    m_hardlinkAggregator->m_controlInfo->m_controlReaderFailed = false;
    m_hardlinkAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_hardlinkAggregator->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(HardlinkAggregatorTest, IsAbort)
{
    m_hardlinkAggregator->m_abort = false;
    m_hardlinkAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_hardlinkAggregator->IsAbort(), true);

    m_hardlinkAggregator->m_abort = false;
    m_hardlinkAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_hardlinkAggregator->IsAbort(), false);
}

TEST_F(HardlinkAggregatorTest, IsComplete)
{
    m_hardlinkAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_hardlinkAggregator->m_controlInfo->m_readProduce = 1;
    m_hardlinkAggregator->m_aggTaskProduce = 1;
    m_hardlinkAggregator->m_aggTaskConsume = 1;
    m_hardlinkAggregator->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_hardlinkAggregator->IsComplete(), true);

    m_hardlinkAggregator->m_controlInfo->m_aggregateConsume = 1;
    m_hardlinkAggregator->m_controlInfo->m_readProduce = 4;
    EXPECT_EQ(m_hardlinkAggregator->IsComplete(), false);
}

TEST_F(HardlinkAggregatorTest, HandleSuccessEvent)
{
    shared_ptr<SqliteTask> taskPtr {};
    EXPECT_NO_THROW(m_hardlinkAggregator->HandleSuccessEvent(taskPtr));
}

TEST_F(HardlinkAggregatorTest, HandleFailureEvent)
{
    shared_ptr<SqliteTask> taskPtr {};
    EXPECT_NO_THROW(m_hardlinkAggregator->HandleFailureEvent(taskPtr));
}

TEST_F(HardlinkAggregatorTest, PollHardlinkAggregateTask)
{
    for (uint16_t i = 0; i < MAX_JOB_SCHDULER_TEST; i++) {
        std::shared_ptr<Module::JobScheduler> t_jsPtr = make_shared<Module::JobScheduler>(
            *Module::ThreadPoolFactory::GetThreadPoolInstance(m_hardlinkAggregator->m_threadPoolKey, 1));
        m_hardlinkAggregator->m_jsPtr.push_back(t_jsPtr);
    }

    m_hardlinkAggregator->PollHardlinkAggregateTask();
    MOCKER_CPP(&Module::JobScheduler::Empty)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&Module::JobScheduler::Get)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_hardlinkAggregator->PollHardlinkAggregateTask());
}

TEST_F(HardlinkAggregatorTest, PushToWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";
    EXPECT_NO_THROW(m_hardlinkAggregator->PushToWriteQueue(fileHandle));
}

TEST_F(HardlinkAggregatorTest, CreateSqliteIndexTask)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "0";
    fileHandle.m_file->m_fileCount = 1;

    shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = make_shared<AggregateHardlinkDirInfo>();
    aggregateDirInfo->m_hardlinkFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_dirName = fileHandle.m_file->m_dirName;
    aggregateDirInfo->m_totalHardlinkFilesCount = fileHandle.m_file->m_fileCount;
    aggregateDirInfo->m_readCompletedFilesCount = 1;
    aggregateDirInfo->m_hardlinkFiles->push_back(fileHandle);

    std::shared_ptr<BlobFileList> blobFileList = make_shared<BlobFileList>();
    m_hardlinkAggregator->m_blobFileList.push_back(blobFileList);
    m_hardlinkAggregator->CreateSqliteIndexTask(aggregateDirInfo);
    EXPECT_TRUE(aggregateDirInfo->m_hardlinkFiles->empty());
}

TEST_F(HardlinkAggregatorTest, CheckAndCreateAggregateTask)
{
    std::string dirName = "d1";
    m_hardlinkAggregator->CheckAndCreateAggregateTask(dirName);

    MOCKER_CPP(&AggregateHardlinkDirMap::GetDirInfo)
            .stubs()
            .will(invoke(GetDirInfo_Stub2));
    MOCKER_CPP(&HardlinkAggregator::CreateSqliteIndexTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    EXPECT_NO_THROW(m_hardlinkAggregator->CheckAndCreateAggregateTask(dirName));
}

TEST_F(HardlinkAggregatorTest, HandleHardlinkAggregate)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_dirName = "d1";

    m_hardlinkAggregator->m_aggregateHardlinkMap.Exist(fileHandle.m_file->m_dirName);

    MOCKER_CPP(&AggregateHardlinkDirMap::Exist)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&HardlinkAggregator::CheckAndCreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    EXPECT_NO_THROW(m_hardlinkAggregator->HandleHardlinkAggregate(fileHandle));

    MOCKER_CPP(&AggregateHardlinkDirMap::GetDirInfo)
            .stubs()
            .will(invoke(GetDirInfo_Stub1))
            .then(invoke(GetDirInfo_Stub2));
    EXPECT_NO_THROW(m_hardlinkAggregator->HandleHardlinkAggregate(fileHandle));
    EXPECT_NO_THROW(m_hardlinkAggregator->HandleHardlinkAggregate(fileHandle));
}

TEST_F(HardlinkAggregatorTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";

    m_hardlinkAggregator->m_aggregateQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&HardlinkAggregator::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HardlinkAggregator::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_hardlinkAggregator->ThreadFunc());

    unique_ptr<HardlinkAggregator> m_hardlinkAggregator1 =
        make_unique<HardlinkAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
    m_hardlinkAggregator1->m_aggregateQueue->WaitAndPush(fileHandle);
    MOCKER_CPP(&HardlinkAggregator::HandleHardlinkAggregate)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&HardlinkAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&HardlinkAggregator::PollHardlinkAggregateTask)
            .stubs()
            .will(ignoreReturnValue());
     for (uint16_t i = 0; i < MAX_JOB_SCHDULER; i++) {
        std::shared_ptr<Module::JobScheduler> t_jsPtr = make_shared<Module::JobScheduler>(
            *Module::ThreadPoolFactory::GetThreadPoolInstance(m_hardlinkAggregator1->m_threadPoolKey, 1));
        m_hardlinkAggregator1->m_jsPtr.push_back(t_jsPtr);
    }
    EXPECT_NO_THROW(m_hardlinkAggregator1->ThreadFunc());
}