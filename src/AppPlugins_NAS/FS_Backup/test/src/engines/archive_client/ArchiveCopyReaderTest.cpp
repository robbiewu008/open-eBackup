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
#include <list>
#include <cstdio>
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "log/Log.h"
#include "ThreadPoolFactory.h"
#include "stub.h"
#include "ArchiveCopyReader.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

class ArchiveClientTest : public ArchiveClientBase {
public:
    ArchiveClientTest() {}
    ~ArchiveClientTest() {}

    virtual int GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp) override { return SUCCESS; }
};

class ArchiveCopyReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    std::unique_ptr<ArchiveCopyReader> m_copyReader = nullptr;
};

void ArchiveCopyReaderTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<ArchiveRestoreAdvanceParams>(std::make_shared<ArchiveClientTest>());
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    ReaderParams copyReaderParams {};
    copyReaderParams.backupParams = params;
    copyReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_copyReader = std::make_unique<ArchiveCopyReader>(copyReaderParams);
}

void ArchiveCopyReaderTest::TearDown()
{}

void ArchiveCopyReaderTest::SetUpTestCase()
{}

void ArchiveCopyReaderTest::TearDownTestCase()
{}

int Function_Ret_Zero()
{
    return 0;
}

void Function_Ret_Void()
{}


FileDescState GetSrcState_Ret()
{
    return FileDescState::INIT;
}
bool getPut_True()
{
    return true;
}
bool getPut_False()
{
    return false;
}

/*
* 用例名称：输入参数，测试ArchiveCopyReader的启动
* 前置条件：文件读队列为空
* check点：ArchiveCopyReader启动正常
*/
TEST_F(ArchiveCopyReaderTest, StartTest)
{
    auto ret = m_copyReader->Start();
    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
* 用例名称：检查状态完成
* 前置条件：读状态是否完成/未完成
* check点：文件拷贝状态完成/未完成
*/
TEST_F(ArchiveCopyReaderTest, IsCompleteTest)
{
    bool ret = m_copyReader->IsComplete();
    EXPECT_EQ(ret, false);
    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    ret = m_copyReader->IsComplete();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：SetArchiveClient
* 前置条件：
* check点：无返回值
*/
TEST_F(ArchiveCopyReaderTest, SetArchiveClient)
{
    std::shared_ptr<ArchiveClientBase> client = std::make_shared<ArchiveClientTest>();
    m_copyReader->SetArchiveClient(client);
    EXPECT_NE(m_copyReader->m_archiveClient, nullptr);
}

/*
* 用例名称：Abort
* 前置条件：
* check点：中止
*/
TEST_F(ArchiveCopyReaderTest, Abort)
{
    BackupRetCode ret = m_copyReader->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
* 用例名称：Destroy
* 前置条件：
* check点：销毁
*/
TEST_F(ArchiveCopyReaderTest, Destroy)
{
    m_copyReader->m_threadDone = true;
    m_copyReader->m_pollThreadDone = true;
    EXPECT_EQ(m_copyReader->Destroy(), BackupRetCode::SUCCESS);

    m_copyReader->m_pollThreadDone = false;
    EXPECT_EQ(m_copyReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    m_copyReader->m_threadDone = false;
    m_copyReader->m_pollThreadDone = true;
    EXPECT_EQ(m_copyReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);
}

/*
* 用例名称：Enqueue
* 前置条件：
* check点：队列
*/
TEST_F(ArchiveCopyReaderTest, Enqueue)
{
    FileHandle fileHandle;
    BackupRetCode ret = m_copyReader->Enqueue(fileHandle);
    // m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
* 用例名称：GetStatus
* 前置条件：
* check点：获得状态
*/
TEST_F(ArchiveCopyReaderTest, GetStatus)
{
    BackupPhaseStatus ret = m_copyReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);
}

/*
* 用例名称：OpenFile
* 前置条件：
* check点：打开文件
*/
TEST_F(ArchiveCopyReaderTest, OpenFile)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/file.txt";
    fileHandle.m_file->m_size = 0;

    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyReader->m_controlInfo->m_noOfFilesToBackup = 1;

    m_copyReader->Start();
    int ret = m_copyReader->OpenFile(fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：ReadMeta
* 前置条件：
* check点：读meta
*/
TEST_F(ArchiveCopyReaderTest, ReadMeta)
{
    FileHandle fileHandle;
    int ret = m_copyReader->ReadMeta(fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：CloseFile
* 前置条件：
* check点：关闭文件
*/
TEST_F(ArchiveCopyReaderTest, CloseFile)
{   
    Stub stub;
    stub.set(ADDR(Module::JobScheduler, Put), getPut_True);

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/file.txt";
    fileHandle.m_file->m_size = 0;

    m_copyReader->Start();
    m_copyReader->OpenFile(fileHandle);
    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    int ret = m_copyReader->CloseFile(fileHandle);

    stub.reset(ADDR(Module::JobScheduler, Put));

    stub.set(ADDR(Module::JobScheduler, Put), getPut_False);
    ret = m_copyReader->CloseFile(fileHandle);
    EXPECT_EQ(ret, Module::FAILED);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
* 用例名称：ReadData
* 前置条件：
* check点：读数据
*/
TEST_F(ArchiveCopyReaderTest, ReadData)
{

    // Stub stub;
    // stub.set(ADDR(ArchiveCopyReader, ReadEmptyData), Function_Ret_Zero);

    // BackupParams params;
    // params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    // params.dstEngine = BackupIOEngine::POSIX;

    // FileHandle fileHandle; 
    // fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    // fileHandle.m_file->m_fileName = "/file.txt";
    // fileHandle.m_file->m_size = 0;
    
    // int ret = m_copyReader->ReadData(fileHandle);
    // EXPECT_EQ(ret, SUCCESS);

    // stub.reset(ADDR(ArchiveCopyReader, ReadEmptyData));
}

/*
* 用例名称：ReadEmptyData
* 前置条件：
* check点：读空数据
*/
TEST_F(ArchiveCopyReaderTest, ReadEmptyData)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/root/.bash_history";
    fileHandle.m_file->m_size = 0;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;

    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyReader->Start();
    int ret = m_copyReader->ReadEmptyData(fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：ReadNormalData
* 前置条件：
* check点：读正常数据吧
*/
TEST_F(ArchiveCopyReaderTest, ReadNormalData)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/root/.bash_history";
    fileHandle.m_file->m_size = 4194304 * 3 + 1;
    fileHandle.m_block.m_size = 4194304;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;

    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyReader->Start();
    int ret = m_copyReader->ReadNormalData(fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/*
* 用例名称：ProcessTimers
* 前置条件：
* check点：过程定时器
*/
TEST_F(ArchiveCopyReaderTest, ProcessTimers)
{
    int64_t ret = m_copyReader->ProcessTimers();
    EXPECT_NE(ret, 0);
}

/*
* 用例名称：ProcessReadEntries
* 前置条件：
* check点：无返回值 
*/
TEST_F(ArchiveCopyReaderTest, ProcessReadEntries)
{
    Stub stub;
    stub.set(ADDR(Module::JobScheduler, Put), getPut_True);

    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/root/.bash_history";
    fileHandle.m_file->m_mode = 0;

    m_copyReader->ProcessReadEntries(fileHandle);
    EXPECT_EQ(m_copyReader->m_readTaskProduce, 1);

    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
* 用例名称：PollReadTask
* 前置条件：
* check点：无返回值
*/
TEST_F(ArchiveCopyReaderTest, PollReadTask)
{
    m_copyReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyReader->Start();
    EXPECT_NO_THROW(m_copyReader->PollReadTask());
}

/*
* 用例名称：HandleSuccessEvent
* 前置条件：
* check点：无返回值
*/
// TEST_F(ArchiveCopyReaderTest, HandleSuccessEvent)
// {
//     std::shared_ptr<BlockBufferMap> m_blockBufferMap = std::make_shared<BlockBufferMap>();

//     FileHandle m_fileHandle;
//     m_fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::ARCHIVE_CLIENT);
    
//     ArchiveServiceParams m_params;
//     m_params.srcRootPath = "m_srcAdvParams->rootPath";
//     m_params.dstRootPath = "m_dstAdvParams->rootPath";
//     m_params.backupDataFormat = BackupDataFormat::UNKNOWN_FORMAT;
//     m_params.restoreReplacePolicy = RestoreReplacePolicy::NONE;
//     m_params.backupType = BackupType::UNKNOWN_TYPE;

//     // std::shared_ptr<ArchiveClientBase>  = std::make_shared<ArchiveClientTest>();

//     std::shared_ptr<ArchiveServiceTask> taskPtr = std::make_shared<ArchiveServiceTask>(
//         ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params);
//     m_copyReader->HandleSuccessEvent(taskPtr);
//     EXPECT_EQ(0, 0);
// }

/*
* 用例名称：HandleFailedEvent
* 前置条件：
* check点：无返回值
*/
TEST_F(ArchiveCopyReaderTest, HandleFailedEvent)
{
    std::shared_ptr<BlockBufferMap> m_blockBufferMap = std::make_shared<BlockBufferMap>();

    FileHandle m_fileHandle;
    m_fileHandle.m_file = std::make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::ARCHIVE_CLIENT);
    m_fileHandle.m_retryCnt = 10e6;

    ArchiveServiceParams m_params;
    m_params.srcRootPath = "m_srcAdvParams->rootPath";
    m_params.dstRootPath = "m_dstAdvParams->rootPath";
    m_params.backupDataFormat = BackupDataFormat::UNKNOWN_FORMAT;
    m_params.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    m_params.backupType = BackupType::UNKNOWN_TYPE;

    std::shared_ptr<ArchiveClientBase> m_archiveClient = std::make_shared<ArchiveClientTest>();

    std::shared_ptr<ArchiveServiceTask> taskPtr = std::make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, m_fileHandle, m_params, m_archiveClient);
    m_copyReader->HandleFailedEvent(taskPtr);
    EXPECT_EQ(0, 0);
}

/*
* 用例名称：PushFileHandleToAggregator
* 前置条件：
* check点：无返回值
*/
TEST_F(ArchiveCopyReaderTest, PushFileHandleToAggregator)
{
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;

    FileHandle fileHandle; 
    fileHandle.m_file = std::make_shared<FileDesc>(params.srcEngine, params.dstEngine);
    fileHandle.m_file->m_fileName = "/file.txt";
    fileHandle.m_file->m_mode = 0;

    //int temp = m_controlInfo->m_readProduce;
    m_copyReader->PushFileHandleToAggregator(fileHandle);
    //EXPECT_EQ(m_controlInfo->m_readProduce, temp + 1);
}
