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
#include "FileAggregateTask.h"
#include "FSBackupUtils.h"
#include "Snowflake.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
#ifdef WIN32
    const string PATH_SEPERATOR = "\\";
#else
    const string PATH_SEPERATOR = "/";
#endif

    const string SQLITE_DIR = "sqlite";
    const string SQLITE_ALIAS_DIR = "sqlitealias";
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
    const uint32_t KB_IN_BYTES = 1024;
}

class FileAggregateTaskTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    shared_ptr<FileAggregateTask> m_fileAggregateTask = nullptr;
    shared_ptr<SqliteTask> m_sqliteTask = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);

    std::shared_ptr<std::vector<FileHandle>> m_fileHandleList = make_shared<vector<FileHandle>>();;
    std::string m_dirPath;
    std::shared_ptr<SQLiteDB> m_sqliteDb;
    std::string m_sqliteDBRootPath;
    std::string m_sqliteDBAliasPath;
    std::shared_ptr<Module::Snowflake> m_idGenerator;
    std::atomic<uint64_t> sqlTaskProduce;
    std::atomic<uint64_t> sqlTaskConsume;
    std::atomic<uint64_t> *sqlTaskConsumePtr;
    std::vector<BlobFileDetails> blobFileDetailsList {};
    std::vector<std::shared_ptr<BlobFileList>> blobFileList {};
};

void FileAggregateTaskTest::SetUp()
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
    commonParams.blockSize = 2;
    m_backupParams.commonParams = commonParams;

    m_idGenerator = make_shared<Snowflake>();
    m_sqliteDBRootPath = m_backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_DIR;
    m_sqliteDBAliasPath = m_backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_ALIAS_DIR
        + PATH_SEPERATOR + m_backupParams.commonParams.subJobId;
    m_sqliteDb = make_shared<SQLiteDB>(m_sqliteDBRootPath, m_sqliteDBAliasPath, m_idGenerator);

    m_sqliteTask = make_shared<SqliteTask>(SqliteEvent::INVALID_EVENT,
        m_idGenerator, sqlTaskConsumePtr, m_backupParams);

    m_fileAggregateTask = make_unique<FileAggregateTask>(
        AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX,
        m_backupParams, m_fileHandleList, m_dirPath, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_sqliteDb, m_sqliteDBRootPath, m_idGenerator, sqlTaskProduce,
        sqlTaskConsume, blobFileList);
}

void FileAggregateTaskTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void FileAggregateTaskTest::SetUpTestCase()
{}

void FileAggregateTaskTest::TearDownTestCase()
{}

TEST_F(FileAggregateTaskTest, Exec)
{
    MOCKER_CPP(&FileAggregateTask::DoAggrFilesAndCreateIndex)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregateTask->Exec();

    auto task1 = make_shared<FileAggregateTask>(
        AggregateEvent::UNAGGREGATE_FILES, m_backupParams,
        m_fileHandleList, m_dirPath, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_sqliteDb, m_sqliteDBRootPath, m_idGenerator, sqlTaskProduce,
        sqlTaskConsume, blobFileList);
    MOCKER_CPP(&FileAggregateTask::HandleUnaggregation)
            .stubs()
            .will(ignoreReturnValue());
    task1->Exec();

    auto task2 = make_shared<FileAggregateTask>(
        AggregateEvent::INVALID_EVENT, m_backupParams,
        m_fileHandleList, m_dirPath, m_writeQueue, m_blockBufferMap,
        m_controlInfo, m_sqliteDb, m_sqliteDBRootPath, m_idGenerator, sqlTaskProduce,
        sqlTaskConsume, blobFileList);
    task2->Exec();
}

static uint64_t GenerateId_Stub()
{
    uint64_t value = 0;
}

TEST_F(FileAggregateTaskTest, GetUniqueIdStr)
{
    MOCKER_CPP(&Snowflake::GenerateId)
            .stubs()
            .will(invoke(GenerateId_Stub));
    m_fileAggregateTask->GetUniqueIdStr();
}

TEST_F(FileAggregateTaskTest, GenerateArchiveFileName)
{
    m_fileAggregateTask->GenerateArchiveFileName();
}

TEST_F(FileAggregateTaskTest, PushArchiveFileToWriter)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_size = 1;

    m_fileAggregateTask->PushArchiveFileToWriter(fileHandle);

    fileHandle.m_file->m_size = 2;
    m_fileAggregateTask->PushArchiveFileToWriter(fileHandle);
}

TEST_F(FileAggregateTaskTest, DeleteFileList)
{
    std::string parentDir = "/d1";
    std::vector<string> blobFileList;
    std::string blobFileName = "file.dpa.emei.blob";
    blobFileList.push_back(blobFileName);
    m_fileAggregateTask->DeleteFileList(parentDir, blobFileList);
}

TEST_F(FileAggregateTaskTest, CreateFileHandles)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    uint8_t *buffer = new uint8_t[3]();
    fileHandle.m_file->m_size = 3;

    MOCKER_CPP(&FileAggregateTask::CreateNPushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregateTask->CreateFileHandles(buffer, fileHandle);

    fileHandle.m_file->m_size = 1;
    MOCKER_CPP(&FileAggregateTask::CreateNPushToWriteQueue)
            .stubs()
            .will(ignoreReturnValue());
    m_fileAggregateTask->CreateFileHandles(buffer, fileHandle);
}

TEST_F(FileAggregateTaskTest, CreateNPushToWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    uint8_t *buffer = new uint8_t[1]();
    fileHandle.m_file->m_size = 1;

    m_fileAggregateTask->CreateNPushToWriteQueue(buffer, fileHandle);  // 该函数会释放buffer

    fileHandle.m_file->m_size = 2;
    buffer = new uint8_t[2]();
    m_fileAggregateTask->CreateNPushToWriteQueue(buffer, fileHandle);
}

TEST_F(FileAggregateTaskTest, CreateNPushToWriteQueueSingleBlock)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    uint8_t *buffer = new uint8_t[1]();
    fileHandle.m_file->m_size = 1;

    m_fileAggregateTask->CreateNPushToWriteQueue(buffer, fileHandle, true);

    fileHandle.m_file->m_size = 5;
    buffer = new uint8_t[5]();
    m_fileAggregateTask->CreateNPushToWriteQueue(buffer, fileHandle, true);
}

TEST_F(FileAggregateTaskTest, CreateTaskForSqliteIndex)
{
	std::string archiveFileName = "File";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->m_dirName = "0";
    fileHandle.m_file->m_fileCount = 1;

    m_fileAggregateTask->m_dirPath = "0";
    std::shared_ptr<BlobFileList> blobFileList = make_shared<BlobFileList>();
    m_fileAggregateTask->m_blobFileList.push_back(blobFileList);
    m_fileAggregateTask->CreateTaskForSqliteIndex(fileHandle, archiveFileName);

    m_fileAggregateTask->m_inputFileHandleList = make_shared<vector<FileHandle>>();
    m_fileAggregateTask->m_inputFileHandleList->push_back(fileHandle);

	m_fileAggregateTask->CreateTaskForSqliteIndex(fileHandle, archiveFileName);
}

TEST_F(FileAggregateTaskTest, SqliteTaskExec)
{
    m_sqliteTask->Exec();

    MOCKER_CPP(&SqliteTask::DoCreateSqliteIndex)
            .stubs()
            .will(ignoreReturnValue());

    auto blobFileDetails = std::make_shared<BlobFileDetails>();
    blobFileDetails->m_dirPath = "0";
    m_sqliteTask = make_shared<SqliteTask>(SqliteEvent::SQL_TASK_PTR_CREATE_INDEX,
        m_idGenerator, sqlTaskConsumePtr, m_backupParams);
    m_sqliteTask->m_blobFileDetailsList.push_back(blobFileDetails);
    m_sqliteTask->Exec();
}

TEST_F(FileAggregateTaskTest, SqliteTaskGetUniqueIdStr)
{
    MOCKER_CPP(&Snowflake::GenerateId)
            .stubs()
            .will(invoke(GenerateId_Stub));
    m_sqliteTask->GetUniqueIdStr();
}

TEST_F(FileAggregateTaskTest, DoCreateSqliteIndex)
{
    std::string archiveFileName = "File";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_size = 5;

    auto blobFile = std::make_shared<BlobFileDetails>();
    std::vector<SmallFileDesc> t_fileDescList;
    blobFile->aggregatedFileSize = fileHandle.m_file->m_size;
    blobFile->archiveFileName = archiveFileName;
    blobFile->m_dirPath =  "d1";
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;
    blobFile->m_smallFileDescList = t_fileDescList;

    uint16_t index = 0;

    MOCKER_CPP(&FSBackupUtils::RecurseCreateDirectory)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&SqliteTask::CreateSqliteDb)
            .stubs()
            .will(returnValue(1));
    m_sqliteTask->DoCreateSqliteIndex(blobFile, index);
}

TEST_F(FileAggregateTaskTest, InsertIndexInfo)
{
    std::string archiveFileName = "File";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_size = 5;
    fileHandle.m_file->m_fileName = "/1.txt";

    sqlite3_stmt *sqlStmt = nullptr;
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr { nullptr };

    auto blobFile = std::make_shared<BlobFileDetails>();
    SmallFileDesc t_fileDesc;
    t_fileDesc.m_onlyFileName = fileHandle.m_file->m_onlyFileName;
    t_fileDesc.m_aggregateFileOffset = fileHandle.m_file->m_aggregateFileOffset;
    t_fileDesc.m_size = fileHandle.m_file->m_size;
    t_fileDesc.m_ctime = fileHandle.m_file->m_ctime;
    t_fileDesc.m_mtime = fileHandle.m_file->m_mtime;
    t_fileDesc.m_flag = fileHandle.m_file->m_flag;

    blobFile->m_smallFileDescList.push_back(t_fileDesc);
    blobFile->aggregatedFileSize = fileHandle.m_file->m_size;
    blobFile->archiveFileName = archiveFileName;
    blobFile->m_dirPath =  "d1";
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;

    MOCKER_CPP(&IndexDetails::InsertIndexInfo)
            .stubs()
            .will(returnValue(1));
    EXPECT_EQ(m_sqliteTask->InsertIndexInfo(blobFile, sqlStmt, sqlInfoPtr), 0);

    fileHandle.m_file->m_onlyFileName = "1.txt";
    t_fileDesc.m_onlyFileName = fileHandle.m_file->m_onlyFileName;
    blobFile->m_smallFileDescList.push_back(t_fileDesc);

    MOCKER_CPP(&IndexDetails::InsertIndexInfo)
            .stubs()
            .will(returnValue(1));
    EXPECT_EQ(m_sqliteTask->InsertIndexInfo(blobFile, sqlStmt, sqlInfoPtr), -1);
}

TEST_F(FileAggregateTaskTest, DoCreateSqliteIndexBlobList)
{
    auto blobFile = std::make_shared<BlobFileDetails>();
    blobFile->aggregatedFileSize = 4;
    blobFile->archiveFileName = "archiveFileName";
    blobFile->m_dirPath =  "/d1";
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;

    std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
    m_fileAggregateTask->m_blobFileList.push_back(tblobFileList);

    uint16_t hashIndex = FSBackupUtils::GetHashIndexForSqliteTask(blobFile->m_dirPath);
    //m_fileAggregateTask->m_blobFileList[hashIndex]->m_blobFileDetailsList.push_back(blobFile);

    MOCKER_CPP(&SqliteTask::DoCreateSqliteIndex)
            .stubs()
            .will(ignoreReturnValue());
    m_sqliteTask->DoCreateSqliteIndexBlobList();
}

TEST_F(FileAggregateTaskTest, HandleUnaggregation)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::LIBNFS, BackupIOEngine::LIBNFS);
    fileHandle.m_file->m_size = 5;
    fileHandle.m_file->m_fileName = "./1.txt";
    fileHandle.m_file->m_onlyFileName = "1.txt";
    fileHandle.m_file->m_aggregateFileOffset = 0;
    fileHandle.m_file->m_mode = 41471;

    m_fileAggregateTask->m_blobfileHandle = fileHandle;
    m_fileAggregateTask->m_blobfileHandle.m_block.m_buffer = nullptr;

    m_fileAggregateTask->HandleUnaggregation();

    m_fileAggregateTask->m_blobfileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_file->m_size + 1];

    m_fileAggregateTask->m_inputFileHandleList = make_shared<vector<FileHandle>>();
    m_fileAggregateTask->m_inputFileHandleList->push_back(fileHandle);

    MOCKER_CPP(&FileAggregateTask::CreateFileHandles)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_fileAggregateTask->HandleUnaggregation();

    m_fileAggregateTask->m_inputFileHandleList->clear();
    fileHandle.m_file->m_mode = 0;
    m_fileAggregateTask->m_inputFileHandleList->push_back(fileHandle);

    m_fileAggregateTask->HandleUnaggregation();

    m_fileAggregateTask->m_inputFileHandleList->clear();
    fileHandle.m_file->m_size = 0;
    m_fileAggregateTask->m_inputFileHandleList->push_back(fileHandle);

    m_fileAggregateTask->HandleUnaggregation();
}
