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
#include <iostream>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "FileAggregator.h"
#include "FileAggregateTask.h"
#include "Snowflake.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
    const uint32_t KB_IN_BYTES = 1024;
}

class FileAggregatorTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<FileAggregator> m_fileAggregator = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void FileAggregatorTest::SetUp()
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
    commonParams.subJobId = "wwwwwwwwwwwwwww";
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    commonParams.maxAggregateFileSize = 10;
    commonParams.maxFileSizeToAggregate = 10;
    std::vector<std::string> aggregateFileSet;
    std::string file = "./1.txt";
    aggregateFileSet.push_back(file);
    commonParams.aggregateFileSet = aggregateFileSet;
    m_backupParams.commonParams = commonParams;

    m_fileAggregator = make_unique<FileAggregator>(m_backupParams, m_writeQueue, m_readQueue,
        m_blockBufferMap, m_controlInfo);
}

void FileAggregatorTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void FileAggregatorTest::SetUpTestCase()
{}

void FileAggregatorTest::TearDownTestCase()
{}

TEST_F(FileAggregatorTest, Start)
{
    EXPECT_EQ(m_fileAggregator->Start(), BackupRetCode::SUCCESS);

    m_backupParams.commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    unique_ptr<FileAggregator> m_fileAggregator1 = make_unique<FileAggregator>(m_backupParams,
        m_writeQueue, m_readQueue, m_blockBufferMap, m_controlInfo);

    EXPECT_EQ(m_fileAggregator1->Start(), BackupRetCode::SUCCESS);
}

TEST_F(FileAggregatorTest, Abort)
{
    EXPECT_EQ(m_fileAggregator->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(FileAggregatorTest, IsAbort)
{
    m_fileAggregator->m_abort = false;
    m_fileAggregator->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_fileAggregator->IsAbort(), true);

    m_fileAggregator->m_abort = false;
    m_fileAggregator->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_fileAggregator->IsAbort(), false);
}

TEST_F(FileAggregatorTest, PollAggregateTask)
{
    m_fileAggregator->m_started = false;
    m_fileAggregator->PollAggregateTask();

    m_fileAggregator->m_started = true;
    m_fileAggregator->m_jsPtr = make_shared<Module::JobScheduler>(
        *Module::ThreadPoolFactory::GetThreadPoolInstance(m_fileAggregator->m_threadPoolKey, 1));
    MOCKER_CPP(&Module::JobScheduler::Empty)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&Module::JobScheduler::Get)
            .stubs()
            .will(returnValue(true));
    m_fileAggregator->PollAggregateTask();
}

TEST_F(FileAggregatorTest, HandleSuccessEvent)
{
    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    std::shared_ptr<Snowflake> idGenerator = make_shared<Snowflake>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";
    fileHandle.m_block.m_seq = 1;

    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";

    std::atomic<uint64_t> sqlTaskProduce;
    std::atomic<uint64_t> sqlTaskConsume;
    std::vector<std::shared_ptr<BlobFileList>> blobFileList {}; {};
    auto task = make_shared<FileAggregateTask>(
        AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX, m_backupParams,
        aggregateDirInfo->m_aggregatedFiles, aggregateDirInfo->m_dirName, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_fileAggregator->m_sqliteDb, m_fileAggregator->m_sqliteDBRootPath, idGenerator,
        sqlTaskProduce, sqlTaskConsume, blobFileList);
    task->m_outputFileHandleList.push_back(fileHandle);

    MOCKER_CPP(&FileAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->HandleSuccessEvent(task);

    auto task1 = make_shared<FileAggregateTask>(
        AggregateEvent::UNAGGREGATE_FILES, m_backupParams,
        aggregateDirInfo->m_aggregatedFiles, aggregateDirInfo->m_dirName, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_fileAggregator->m_sqliteDb, m_fileAggregator->m_sqliteDBRootPath, idGenerator,
        sqlTaskProduce, sqlTaskConsume, blobFileList);
    task1->m_blobfileHandle.m_file =
        std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    task1->m_isDeleteBlobFile = true;
    task1->m_outputFileHandleList.push_back(fileHandle);

    m_fileAggregator->HandleSuccessEvent(task1);
}

TEST_F(FileAggregatorTest, HandleFailureEvent)
{
    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    std::shared_ptr<Snowflake> idGenerator = make_shared<Snowflake>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";

    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";
    std::atomic<uint64_t> sqlTaskProduce;
    std::atomic<uint64_t> sqlTaskConsume;
    std::vector<std::shared_ptr<BlobFileList>> blobFileList {};
    auto task = make_shared<FileAggregateTask>(
        AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX, m_backupParams,
        aggregateDirInfo->m_aggregatedFiles, aggregateDirInfo->m_dirName, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_fileAggregator->m_sqliteDb, m_fileAggregator->m_sqliteDBRootPath, idGenerator,
        sqlTaskProduce, sqlTaskConsume, blobFileList);

    m_fileAggregator->HandleFailureEvent(task);

    auto task1 = make_shared<FileAggregateTask>(
        AggregateEvent::UNAGGREGATE_FILES, m_backupParams,
        aggregateDirInfo->m_aggregatedFiles, aggregateDirInfo->m_dirName, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_fileAggregator->m_sqliteDb, m_fileAggregator->m_sqliteDBRootPath, idGenerator,
        sqlTaskProduce, sqlTaskConsume, blobFileList);
    task1->m_blobfileHandle.m_file =
        std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    task1->m_isDeleteBlobFile = true;

    m_fileAggregator->HandleFailureEvent(task1);
    string archiveFileName = "";
    std::vector<BlobFileDetails> blobFileDetailsList {};
    std::atomic<uint64_t> *sqlTaskConsume1;
    auto task2 = make_shared<SqliteTask>(
        SqliteEvent::SQL_TASK_PTR_CREATE_INDEX, idGenerator, sqlTaskConsume1, m_backupParams);

    m_fileAggregator->SqlHandleSuccessEvent(task2);
}

TEST_F(FileAggregatorTest, PushToWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";

    MOCKER_CPP(&FileAggregator::IsAbort)
            .stubs()
            .will(returnValue(true));
    m_fileAggregator->PushToWriteQueue(fileHandle);
}

TEST_F(FileAggregatorTest, CreateAggregateTask)
{
    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";

    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";

    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    m_fileAggregator->CreateAggregateTask(aggregateDirInfo);

    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";

    m_fileAggregator->CreateAggregateTask(aggregateDirInfo);
}


TEST_F(FileAggregatorTest, CreateSqliteIndexTask)
{
    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_nonAggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";
    aggregateDirInfo->m_totalFilesCount = 1;
    aggregateDirInfo->m_readFailedFilesCount = 1;

    MOCKER_CPP(&FileAggregator::CreateTaskForSqliteIndex)
            .stubs()
            .will(ignoreReturnValue());

    aggregateDirInfo->m_nonAggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";

    m_fileAggregator->CreateSqliteIndexTask(aggregateDirInfo);
}


TEST_F(FileAggregatorTest, CreateSqliteIndexTaskForEmptyDir)
{
    std::string dirName = "d1";
    MOCKER_CPP(&FileAggregator::CreateTaskForSqliteIndex)
            .stubs()
            .will(ignoreReturnValue());

    m_fileAggregator->CreateSqliteIndexTaskForEmptyDir(dirName);
}

TEST_F(FileAggregatorTest, CheckAndCreateAggregateTask)
{
    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_dirName = "d1";
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    MOCKER_CPP(&FileAggregator::CreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::CreateSqliteIndexTask)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->CheckAndCreateAggregateTask(aggregateDirInfo);

    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 50;
    aggregateDirInfo->m_archiveFileSize = 20 * KB_IN_BYTES;

    m_fileAggregator->CheckAndCreateAggregateTask(aggregateDirInfo);
}

TEST_F(FileAggregatorTest, CreateSqliteIndexTaskForDir)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "./1.txt";

    MOCKER_CPP(&FileAggregator::CreateTaskForSqliteIndex)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->CreateSqliteIndexTaskForDir(fileHandle);

    m_fileAggregator->CreateSqliteIndexTaskForDir(fileHandle);
}

TEST_F(FileAggregatorTest, CreateTaskForSqliteIndex)
{
	std::shared_ptr<std::vector<FileHandle>> fileHandleList = make_shared<vector<FileHandle>>();
    std::string dirName = "0";
	std::string archiveFileName = "File";
	uint64_t fileSize = 10;

    std::shared_ptr<BlobFileList> blobFileList = make_shared<BlobFileList>();
    m_fileAggregator->m_blobFileList.push_back(blobFileList);
    m_fileAggregator->CreateTaskForSqliteIndex(fileHandleList, dirName, archiveFileName, fileSize);

    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "0";
    fileHandle.m_file->m_fileCount = 1;
	fileHandleList->push_back(fileHandle);
	m_fileAggregator->CreateTaskForSqliteIndex(fileHandleList, dirName, archiveFileName, fileSize);
}

TEST_F(FileAggregatorTest, CreateSqliteIndexForAllParentDirs)
{
    std::string parentDir = "/";
    MOCKER_CPP(&lstat)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    MOCKER_CPP(&FileAggregator::CreateSqliteIndexTaskForDir)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->CreateSqliteIndexForAllParentDirs(parentDir);

    m_fileAggregator->CreateSqliteIndexForAllParentDirs(parentDir);
}

TEST_F(FileAggregatorTest, HandleAggrFileSet)
{
    MOCKER_CPP(&FileAggregator::CreateSqliteIndexForAllParentDirs)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->HandleAggrFileSet();
}

TEST_F(FileAggregatorTest, ProcessEmptyFileReadCompletion)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "./1.txt";
    fileHandle.m_file->m_dirName = ".";

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = ".";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);
    MOCKER_CPP(&FileAggregator::CheckAndCreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessEmptyFileReadCompletion(fileHandle);

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    m_fileAggregator->ProcessEmptyFileReadCompletion(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessReadFailedFiles)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "./1.txt";
    fileHandle.m_file->m_dirName = ".";

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = ".";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);
    MOCKER_CPP(&FileAggregator::CheckAndCreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessReadFailedFiles(fileHandle);

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    m_fileAggregator->ProcessReadFailedFiles(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessAggrFileReadCompletion)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "d2";
    fileHandle.m_file->m_dirName = "d2";

    m_fileAggregator->ProcessAggrFileReadCompletion(fileHandle);

    m_fileAggregator->m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d2";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;
    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);

    MOCKER_CPP(&FileAggregator::CheckAndCreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessAggrFileReadCompletion(fileHandle);

    fileHandle.m_file->m_fileName = "d2";
    fileHandle.m_file->m_dirName = "d2";
    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    m_fileAggregator->ProcessAggrFileReadCompletion(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessNonAggrFileReadCompletion)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "d1";
    fileHandle.m_file->m_dirName = "d1";

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_nonAggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_nonAggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);
    MOCKER_CPP(&FileAggregator::CheckAndCreateAggregateTask)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    m_fileAggregator->ProcessNonAggrFileRead(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessDirectory)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "./1.txt";
    fileHandle.m_file->m_dirName = ".";
    fileHandle.m_file->m_fileCount = 0;

    MOCKER_CPP(&FileAggregator::CreateSqliteIndexTaskForEmptyDir)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::CreateSqliteIndexTaskForDir)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessDirectory(fileHandle);

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d1";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    fileHandle.m_file->m_fileCount = 1;
    fileHandle.m_file->m_dirName = "d1";
    fileHandle.m_file->m_fileName = "d1";
    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    MOCKER_CPP(&FileAggregator::CreateSqliteIndexForAllParentDirs)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessDirectory(fileHandle);

    aggregateDirInfo->m_dirName = ".";
    m_backupParams.backupType = BackupType::FILE_LEVEL_RESTORE;

    unique_ptr<FileAggregator> m_fileAggregator1 = make_unique<FileAggregator>(m_backupParams,
        m_writeQueue, m_readQueue, m_blockBufferMap, m_controlInfo);
    m_fileAggregator1->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);

    MOCKER_CPP(&SQLiteDB::PrepareDb)
            .stubs()
            .will(returnValue(Module::FAILED));
    fileHandle.m_file->m_fileName = ".";
    m_fileAggregator1->ProcessDirectory(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessAggBackup)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);

    MOCKER_CPP(&FileAggregator::ProcessReadFailedFiles)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggBackup(fileHandle);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->m_mode = 4480;
    MOCKER_CPP(&FileAggregator::ProcessNonAggrFileRead)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessAggBackup(fileHandle);

    fileHandle.m_file->m_size = 0;
    fileHandle.m_file->m_mode = 41471;
    MOCKER_CPP(&FileAggregator::ProcessEmptyFileReadCompletion)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggBackup(fileHandle);

    fileHandle.m_file->m_size = 1;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    MOCKER_CPP(&FileAggregator::ProcessAggrFileReadCompletion)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggBackup(fileHandle);

    fileHandle.m_file->m_size = 20 * KB_IN_BYTES;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    MOCKER_CPP(&FileAggregator::ProcessNonAggrFileRead)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggBackup(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    m_fileAggregator->ProcessFile(fileHandle);

    m_backupParams.backupType = BackupType::FILE_LEVEL_RESTORE;
    unique_ptr<FileAggregator> m_fileAggregator1 = make_unique<FileAggregator>(m_backupParams,
        m_writeQueue, m_readQueue, m_blockBufferMap, m_controlInfo);

    m_fileAggregator1->ProcessFile(fileHandle);

    m_backupParams.backupType = BackupType::UNKNOWN_TYPE;
    unique_ptr<FileAggregator> m_fileAggregator2 = make_unique<FileAggregator>(m_backupParams,
        m_writeQueue, m_readQueue, m_blockBufferMap, m_controlInfo);

    m_fileAggregator2->ProcessFile(fileHandle);
}

TEST_F(FileAggregatorTest, Aggregate)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->SetFlag(IS_DIR);

    m_fileAggregator->m_started = true;
    m_fileAggregator->m_isAggregateFileSetProcessed = false;

    MOCKER_CPP(&FileAggregator::HandleAggrFileSet)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::ProcessDirectory)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->Aggregate(fileHandle);

    fileHandle.m_file->ClearFlag(IS_DIR);
    m_fileAggregator->Aggregate(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessAggRestoreEmptyFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "d3";
    fileHandle.m_file->m_dirName = "d3";

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d3";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);

    m_fileAggregator->ProcessAggRestoreEmptyFile(fileHandle);

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_dirName, aggregateDirInfo);

    MOCKER_CPP(&FileAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::CleanUpTheDirMap)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestoreEmptyFile(fileHandle);
}

TEST_F(FileAggregatorTest, FillAndPushToReadQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    const string aggfileName = "aggrFileName";
    uint64_t aggFileSize = 10;
    shared_ptr<AggregateDirInfo> aggDirInfo = make_shared<AggregateDirInfo>();
    uint32_t numOfNormalFiles = 1;

     MOCKER_CPP(&FileAggregator::PushToReadQueue)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::AddNormalnGenFileToAggregateFileMap)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->FillAndPushToReadQueue(fileHandle, aggfileName,
        aggFileSize, aggDirInfo, numOfNormalFiles);
}

TEST_F(FileAggregatorTest, FillAndPushToReadQueueOnly)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    const string aggfileName = "aggrFileName";
    shared_ptr<AggregateDirInfo> aggDirInfo = make_shared<AggregateDirInfo>();
    shared_ptr<AggregateNormalInfo> aggFileInfo = make_shared<AggregateNormalInfo>();

     MOCKER_CPP(&FileAggregator::PushToReadQueue)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->FillAndPushToReadQueueOnly(fileHandle, aggfileName,
        aggFileInfo, aggDirInfo);
}

TEST_F(FileAggregatorTest, AddNormalnGenFileToAggregateFileMap)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    FileHandle zipfileHandle {};
    zipfileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    zipfileHandle.m_file->m_onlyFileName = "1.txt";
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    uint32_t numOfNormalFiles = 1;

    m_fileAggregator->AddNormalnGenFileToAggregateFileMap(fileHandle, zipfileHandle,
        aggregateDirInfo, numOfNormalFiles);

    shared_ptr<AggregateNormalInfo> aggFileInfo = make_shared<AggregateNormalInfo>();
    aggregateDirInfo->m_aggFileMap.Insert(zipfileHandle.m_file->m_onlyFileName, aggFileInfo);

    m_fileAggregator->AddNormalnGenFileToAggregateFileMap(fileHandle, zipfileHandle,
        aggregateDirInfo, numOfNormalFiles);
}

TEST_F(FileAggregatorTest, PushToReadQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    m_fileAggregator->PushToReadQueue(fileHandle);
}

TEST_F(FileAggregatorTest, CreateUnAggregateTask)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    std::shared_ptr<AggregateNormalInfo> aggFileInfo = make_shared<AggregateNormalInfo>();
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
    bool isAllFilesRcvd = true;
    aggFileInfo->aggregatedfileHandle = fileHandle;

    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    m_fileAggregator->CreateUnAggregateTask(aggFileInfo, aggregateDirInfo, isAllFilesRcvd);

    aggFileInfo->normalFileHandles->push_back(fileHandle);
    MOCKER_CPP(&FileAggregator::CleanUpTheDirMap)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->CreateUnAggregateTask(aggFileInfo, aggregateDirInfo, isAllFilesRcvd);

    isAllFilesRcvd = false;
    aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
    aggFileInfo->normalFileHandles->push_back(fileHandle);
    m_fileAggregator->CreateUnAggregateTask(aggFileInfo, aggregateDirInfo, isAllFilesRcvd);
}

TEST_F(FileAggregatorTest, FillTheInfoMap)
{
    AggSqlRestoreInfo aggSqlRestoreInfo;
    std::shared_ptr<AggRestoreInfo> aggRestoreInfo = make_shared<AggRestoreInfo>();
    std::vector<AggSqlRestoreInfo> vecNormalFiles {};
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    std::string aggFileName = "1.txt";

    aggSqlRestoreInfo.normalFileName = aggFileName;
    aggSqlRestoreInfo.normalFileOffset = 0;
    vecNormalFiles.push_back(aggSqlRestoreInfo);
    uint32_t numOfNormalFiles = 0;
    m_fileAggregator->FillTheInfoMap(vecNormalFiles, aggregateDirInfo, aggFileName, numOfNormalFiles);
    EXPECT_EQ(numOfNormalFiles, 1);
    aggRestoreInfo->blobFileName = aggFileName;
    aggRestoreInfo->normalFileOffset = 0;
    aggregateDirInfo->m_InfoMap.Insert(aggFileName, aggRestoreInfo);
    numOfNormalFiles = 0;
    m_fileAggregator->FillTheInfoMap(vecNormalFiles, aggregateDirInfo, aggFileName, numOfNormalFiles);
    EXPECT_EQ(numOfNormalFiles, 0);
}

TEST_F(FileAggregatorTest, ProcessAggRestoreNonAggrFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "d";
    fileHandle.m_file->m_dirName = "d";

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;

    m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);
    MOCKER_CPP(&FileAggregator::CleanUpTheDirMap)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestoreNonAggrFile(fileHandle);

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    m_fileAggregator->ProcessAggRestoreNonAggrFile(fileHandle);
}


TEST_F(FileAggregatorTest, CleanUpTheDirMap)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "d";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;
    aggregateDirInfo->m_genFilesCount = 5;

    std::string zipFileName = "aggFileName";
    std::shared_ptr<AggregateNormalInfo> aggFileInfo = make_shared<AggregateNormalInfo>();
    aggFileInfo->aggregatedfileHandle = fileHandle;

    aggregateDirInfo->m_aggFileMap.Insert(zipFileName, aggFileInfo);

    MOCKER_CPP(&FileAggregator::CleanTheBlobFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->CleanUpTheDirMap(aggregateDirInfo);
}

TEST_F(FileAggregatorTest, CleanTheBlobFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);

    std::shared_ptr<AggregateNormalInfo> aggFileInfo = make_shared<AggregateNormalInfo>();
    aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
    aggFileInfo->aggregatedfileHandle = fileHandle;

    MOCKER_CPP(&FileDesc::GetUnAggTaskCnt)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    m_fileAggregator->CleanTheBlobFile(aggFileInfo);
    m_fileAggregator->CleanTheBlobFile(aggFileInfo);

    aggFileInfo->normalFileHandles->push_back(fileHandle);
    m_fileAggregator->CleanTheBlobFile(aggFileInfo);
}

TEST_F(FileAggregatorTest, ProcessAggRestore)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_mode = 4480;

    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&FileAggregator::ProcessAggRestoreNonAggrFile)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::PushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);

    fileHandle.m_file->m_mode = 0777;
    fileHandle.m_file->m_size = 0;
    MOCKER_CPP(&FileAggregator::ProcessAggRestoreEmptyFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);

    fileHandle.m_file->m_size = 4 * KB_IN_BYTES;
    fileHandle.m_file->SetSrcState(FileDescState::AGGREGATED);
    MOCKER_CPP(&FileAggregator::ProcessAggRestoreNormalFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);

    fileHandle.m_file->m_size = 20 * KB_IN_BYTES;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    MOCKER_CPP(&FileAggregator::ProcessAggRestoreBlobFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);

    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    MOCKER_CPP(&FileAggregator::ProcessAggRestoreReadFailedBlobFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    m_fileAggregator->ProcessAggRestore(fileHandle);

    MOCKER_CPP(&FileAggregator::ProcessAggRestoreNonAggrFile)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestore(fileHandle);
}

TEST_F(FileAggregatorTest, ProcessAggRestoreNormalFile)
{ 
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_onlyFileName = "a.txt";
    fileHandle.m_file->m_fileName = "dir/a.txt";
    fileHandle.m_file->m_dirName = "dir";

    m_fileAggregator->ProcessAggRestoreNormalFile(fileHandle);

    shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();
    aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    aggregateDirInfo->m_dirName = "dir";
    aggregateDirInfo->m_readCompletedFilesCount = 10;
    aggregateDirInfo->m_totalFilesCount = 5;
    aggregateDirInfo->m_genFilesCount = 5;

    //m_fileAggregator->m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName);

    m_fileAggregator->m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);

    MOCKER_CPP(&FileAggregator::NormalFileInFileMap)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&IndexDetails::QueryNormalFilesByName)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&FileAggregator::FillTheInfoMap)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::FillAndPushToReadQueue)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileAggregator::CleanUpTheDirMap)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregator->ProcessAggRestoreNormalFile(fileHandle);
}

TEST_F(FileAggregatorTest, NormalFileInFileMap)
{
    std::string zipFileName = "aggFile";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = make_shared<AggregateDirInfo>();

    m_fileAggregator->NormalFileInFileMap(zipFileName, fileHandle, aggregateDirInfo);
}

static std::shared_ptr<SQLiteCoreInfo> PrepareDB_Stub()
{
    std::shared_ptr<SQLiteCoreInfo> ptr = nullptr;
    return ptr;
}

static std::shared_ptr<SQLiteCoreInfo> PrepareDB_Stub1()
{
    sqlite3* pDB;
    std::string dbFile = "SQLITE/copymetadata.sqlite";
    std::shared_ptr<SQLiteCoreInfo> ptr = std::make_shared<SQLiteCoreInfo>(pDB, dbFile);
    return ptr;
}

TEST_F(FileAggregatorTest, IsBlobListEmpty)
{
    std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
    m_fileAggregator->m_blobFileList.push_back(tblobFileList);

    EXPECT_EQ(m_fileAggregator->IsBlobListEmpty(), true);

    BlobFileDetails tblobFile;
    tblobFile.aggregatedFileSize = 4;
    tblobFile.archiveFileName = "archiveFileName";
    tblobFile.m_dirPath =  "/d1";
    tblobFile.m_sqliteDb = m_fileAggregator->m_sqliteDb;
    tblobFile.m_sqliteDBRootPath = m_fileAggregator->m_sqliteDBRootPath;

    //uint16_t hashIndex = FSBackupUtils::GetHashIndexForSqliteTask(tblobFile.m_dirPath);
    //m_fileAggregator->m_blobFileList[hashIndex]->m_blobFileDetailsList.push_back(tblobFile);
//
    //EXPECT_EQ(m_fileAggregator->IsBlobListEmpty(), false);
}

TEST_F(FileAggregatorTest, PrintSqliteTaskDistribution)
{
    std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
    m_fileAggregator->m_blobFileList.push_back(tblobFileList);

    m_fileAggregator->PrintSqliteTaskDistribution();
}

TEST_F(FileAggregatorTest, CanAcceptMoreWork)
{
    std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
    m_fileAggregator->m_blobFileList.push_back(tblobFileList);

    EXPECT_EQ(m_fileAggregator->CanAcceptMoreWork(), true);
}
