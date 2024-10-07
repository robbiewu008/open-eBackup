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
#include "stub.h"
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "PosixCopyReader.h"
#include "log/Log.h"
#include "ThreadPool.h"
#include "ThreadPoolFactory.h"
#include "BackupStructs.h"

using ::testing::_;
using testing::AllOf;
using ::testing::AnyNumber;
using ::testing::AtLeast;
using testing::ByMove;
using testing::DoAll;
using ::testing::Eq;
using ::testing::Field;
using ::testing::Ge;
using ::testing::Gt;
using testing::InitGoogleMock;
using ::testing::Invoke;
using ::testing::Ne;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::SetArgumentPointee;
using ::testing::Throw;

using namespace std;

class PosixCopyReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixCopyReader> m_posixCopyReader = nullptr;
    Stub stub;
};

void PosixCopyReaderTest::SetUp()
{
    BackupQueueConfig config { DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE };
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    ReaderParams copyReaderParams {};
    copyReaderParams.backupParams = params;
    copyReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_posixCopyReader = std::make_unique<PosixCopyReader>(copyReaderParams);
    m_posixCopyReader->m_jsPtr =
        make_shared<Module::JobScheduler>(*Module::ThreadPoolFactory::GetThreadPoolInstance("test", 2));
}

void PosixCopyReaderTest::TearDown() {}

void PosixCopyReaderTest::SetUpTestCase() {}

void PosixCopyReaderTest::TearDownTestCase() {}

static bool IsComplete_Stub_Suc()
{
    return true;
}

static bool IsComplete_Stub_Fail()
{
    return false;
}

static bool IsAbort_Stub_Suc()
{
    return true;
}

static bool IsAbort_Stub_Fail()
{
    return false;
}

static void ThreadFunc_Stub()
{
    return;
}

static void PollReadTask_Stub()
{
    return;
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete失败
 */
TEST_F(PosixCopyReaderTest, IsComplete_Fail)
{
    EXPECT_EQ(m_posixCopyReader->IsComplete(), false);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete成功
 */
TEST_F(PosixCopyReaderTest, IsComplete_True)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_controlReaderPhaseComplete = true;
    m_posixCopyReader->m_readQueue->Clear();
    m_posixCopyReader->m_timer.m_map.clear();
    controlInfo->m_noOfFilesRead = 1;
    controlInfo->m_noOfDirRead = 1;
    controlInfo->m_noOfFilesReadFailed = 1;
    controlInfo->m_skipFileCnt = 1;
    controlInfo->m_skipDirCnt = 1;
    controlInfo->m_unaggregatedFiles = 1;
    controlInfo->m_emptyFiles = 1;
    controlInfo->m_unaggregatedFaildFiles = 1;

    controlInfo->m_noOfFilesToBackup = 4;
    controlInfo->m_noOfDirToBackup = 2;
    controlInfo->m_unarchiveFiles = 2;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->IsComplete(), true);
}

/*
 * 用例名称：Start_Suc
 * 前置条件：无
 * check点：启动PosixCopyReader任务
 */
TEST_F(PosixCopyReaderTest, Start_Suc)
{
    // typedef void (*fptr)(PosixCopyReader*);
    // fptr PosixCopyReader_ThreadFunc = (fptr)(&PosixCopyReader::ThreadFunc);
    // stub.set(PosixCopyReader_ThreadFunc, ThreadFunc_Stub);

    // stub.set(ADDR(PosixCopyReader, PollReadTask), PollReadTask_Stub);
    // EXPECT_EQ(m_posixCopyReader->Start(), BackupRetCode::SUCCESS);
    // stub.reset(PosixCopyReader_ThreadFunc);
    // stub.reset(ADDR(PosixCopyReader, PollReadTask));
}

/*
 * 用例名称：Abort
 * 前置条件：无
 * check点：校验终止
 */
TEST_F(PosixCopyReaderTest, Abort)
{
    EXPECT_EQ(m_posixCopyReader->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：Destroy
 * 前置条件：无
 * check点：校验Destroy
 */
TEST_F(PosixCopyReaderTest, Destroy)
{
    m_posixCopyReader->m_threadDone = true;
    m_posixCopyReader->m_pollThreadDone = true;
    EXPECT_EQ(m_posixCopyReader->Destroy(), BackupRetCode::SUCCESS);

    m_posixCopyReader->m_pollThreadDone = false;
    EXPECT_EQ(m_posixCopyReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    m_posixCopyReader->m_threadDone = false;
    m_posixCopyReader->m_pollThreadDone = true;
    EXPECT_EQ(m_posixCopyReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);
}

/*
 * 用例名称：GetStatus_NotNeedPhaseCpl
 * 前置条件：无
 * check点：校验GetStatus无需解析完成
 */
TEST_F(PosixCopyReaderTest, GetStatus_NotNeedPhaseCpl)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->GetStatus(), BackupPhaseStatus::INPROGRESS);
}

/*
 * 用例名称：GetStatus_Abort
 * 前置条件：无
 * check点：校验GetStatus被终止
 */
TEST_F(PosixCopyReaderTest, GetStatus_Abort)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixCopyReader->m_abort = true;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->GetStatus(), BackupPhaseStatus::ABORTED);
}

/*
 * 用例名称：GetStatus_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixCopyReaderTest, GetStatus_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixCopyReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    controlInfo->m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->GetStatus(), BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE);
}

/*
 * 用例名称：GetStatus_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixCopyReaderTest, GetStatus_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixCopyReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：IsAbort_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixCopyReaderTest, IsAbort_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixCopyReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->IsAbort(), true);
}

/*
 * 用例名称：IsAbort_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixCopyReaderTest, IsAbort_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixCopyReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_posixCopyReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixCopyReader->IsAbort(), false);
}

static bool Put_Stub_Suc()
{
    return true;
}

static bool Put_Stub_Fail()
{
    return false;
}

/*
 * 用例名称：OpenFile_Suc
 * 前置条件：无
 * check点：校验OpenFile成功
 */
TEST_F(PosixCopyReaderTest, OpenFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->OpenFile(fileHandle), false);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadEmptyData_Suc
 * 前置条件：无
 * check点：校验ReadEmptyData成功
 */
TEST_F(PosixCopyReaderTest, ReadEmptyData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->ReadEmptyData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadEmptyData_Fail
 * 前置条件：无
 * check点：校验ReadEmptyData失败
 */
TEST_F(PosixCopyReaderTest, ReadEmptyData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixCopyReader->ReadEmptyData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}


/*
 * 用例名称：ReadSymlinkData_Suc
 * 前置条件：无
 * check点：校验ReadSymlinkData成功
 */
TEST_F(PosixCopyReaderTest, ReadSymlinkData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->ReadSymlinkData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadSymlinkData_Fail
 * 前置条件：无
 * check点：校验ReadSymlinkData失败
 */
TEST_F(PosixCopyReaderTest, ReadSymlinkData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixCopyReader->ReadSymlinkData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadNormalData_Suc
 * 前置条件：无
 * check点：校验ReadNormalData成功
 */
TEST_F(PosixCopyReaderTest, ReadNormalData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->ReadNormalData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadNormalData_Fail
 * 前置条件：无
 * check点：校验ReadNormalData失败
 */
TEST_F(PosixCopyReaderTest, ReadNormalData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixCopyReader->ReadNormalData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadData_Fail
 * 前置条件：无
 * check点：校验ReadData失败
 */
TEST_F(PosixCopyReaderTest, ReadData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 40960;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixCopyReader->ReadData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadData_Empty
 * 前置条件：无
 * check点：校验ReadData创建empty
 */
TEST_F(PosixCopyReaderTest, ReadData_Empty)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 0;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->ReadData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadData_Suc
 * 前置条件：无
 * check点：校验ReadData成功
 */
TEST_F(PosixCopyReaderTest, ReadData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 1;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->ReadData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Suc
 * 前置条件：无
 * check点：校验CloseFile成功
 */
TEST_F(PosixCopyReaderTest, CloseFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 1;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixCopyReader->CloseFile(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Fail
 * 前置条件：无
 * check点：校验CloseFile失败
 */
TEST_F(PosixCopyReaderTest, CloseFile_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixCopyReader->CloseFile(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadMeta_Suc
 * 前置条件：无
 * check点：校验ReadMeta成功
 */
TEST_F(PosixCopyReaderTest, ReadMeta_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_posixCopyReader->ReadMeta(fileHandle), 0);
}

/*
 * 用例名称：ProcessTimers_Suc
 * 前置条件：无
 * check点：校验ProcessTimers成功
 */
TEST_F(PosixCopyReaderTest, ProcessTimers_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    EXPECT_NE(m_posixCopyReader->ProcessTimers(), 0);
}

/*
 * 用例名称：ProcessReadEntries
 * 前置条件：无
 * check点：校验ProcessReadEntries成功
 */
TEST_F(PosixCopyReaderTest, ProcessReadEntries)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 16384;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));


    fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize + 1;
    fileHandle.m_file->m_srcState = FileDescState::INIT;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));

    fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize + 1;
    fileHandle.m_file->m_srcState = FileDescState::SRC_OPENED;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));

    fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize + 1;
    fileHandle.m_file->m_srcState = FileDescState::META_READED;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));

    fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize + 1;
    fileHandle.m_file->m_srcState = FileDescState::AGGREGATED;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));

    fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_posixCopyReader->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize + 1;
    fileHandle.m_file->m_srcState = FileDescState::READED;
    EXPECT_NO_THROW(m_posixCopyReader->ProcessReadEntries(fileHandle));
}


/*
 * 用例名称：BlockReadQueuePop
 * 前置条件：无
 * check点：校验BlockReadQueuePop
 */
TEST_F(PosixCopyReaderTest, BlockReadQueuePop)
{
    EXPECT_NO_THROW(m_posixCopyReader->BlockReadQueuePop());
}

static bool Get_Stub_Suc(){
    return true;
}

/*
 * 用例名称：PollReadTask
 * 前置条件：无
 * check点：校验PollReadTask
 */
TEST_F(PosixCopyReaderTest, PollReadTask)
{
    m_posixCopyReader->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_NO_THROW(m_posixCopyReader->PollReadTask());

    m_posixCopyReader->m_controlInfo->m_readPhaseComplete = false;
    stub.set(ADDR(Module::JobScheduler, Get), Get_Stub_Suc);
    EXPECT_NO_THROW(m_posixCopyReader->PollReadTask());
    stub.reset(ADDR(Module::JobScheduler, Get));
}

/*
 * 用例名称：HandleSuccessEvent
 * 前置条件：无
 * check点：校验HandleSuccessEvent
 */
TEST_F(PosixCopyReaderTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_posixCopyReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::CLOSE_SRC;
    EXPECT_NO_THROW(m_posixCopyReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::OPEN_SRC;
    EXPECT_NO_THROW(m_posixCopyReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::READ_DATA;
    EXPECT_NO_THROW(m_posixCopyReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_posixCopyReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::INVALID;
    EXPECT_NO_THROW(m_posixCopyReader->HandleSuccessEvent(taskPtr));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：无
 * check点：校验HandleFailedEvent
 */
TEST_F(PosixCopyReaderTest, HandleFailedEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_posixCopyReader->HandleFailedEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_retryCnt = DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_posixCopyReader->HandleFailedEvent(taskPtr));
}


/*
 * 用例名称：PushToReader
 * 前置条件：无
 * check点：校验PushToReader
 */
TEST_F(PosixCopyReaderTest, PushToReader)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_NO_THROW(m_posixCopyReader->PushToReader(fileHandle));
}

/*
 * 用例名称：DecomposeAndPush
 * 前置条件：无
 * check点：校验DecomposeAndPush
 */
TEST_F(PosixCopyReaderTest, DecomposeAndPush)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    uint64_t startOffset = 1;
    uint64_t totalSize = 10;
    uint64_t startSeq = 20;
    EXPECT_NO_THROW(m_posixCopyReader->DecomposeAndPush(fileHandle, startOffset, totalSize, startSeq));
}

/*
 * 用例名称：DecomposeAndPush2
 * 前置条件：无
 * check点：校验DecomposeAndPush
 */
TEST_F(PosixCopyReaderTest, DecomposeAndPush2)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_NO_THROW(m_posixCopyReader->DecomposeAndPush(fileHandle));
}
