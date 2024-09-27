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
#include "PosixCopyWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "PosixUtils.h"
#include "stub.h"

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

class PosixCopyWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixCopyWriter> m_PosixCopyWriter = nullptr;
    Stub stub;
};

void PosixCopyWriterTest::SetUp()
{
    BackupQueueConfig config { DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE };
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    WriterParams copyWriterParams {};
    copyWriterParams.backupParams = params;
    copyWriterParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_PosixCopyWriter = std::make_unique<PosixCopyWriter>(copyWriterParams);
    m_PosixCopyWriter->m_jsPtr =
        make_shared<Module::JobScheduler>(*Module::ThreadPoolFactory::GetThreadPoolInstance("test", 2));
}

void PosixCopyWriterTest::TearDown() {}

void PosixCopyWriterTest::SetUpTestCase() {}

void PosixCopyWriterTest::TearDownTestCase() {}

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

/*
 * 用例名称：Abort
 * 前置条件：无
 * check点：校验Abort
 */
TEST_F(PosixCopyWriterTest, Abort)
{
    EXPECT_EQ(m_PosixCopyWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：Destroy
 * 前置条件：无
 * check点：校验Destroy
 */
TEST_F(PosixCopyWriterTest, Destroy)
{
    m_PosixCopyWriter->m_threadDone = true;
    m_PosixCopyWriter->m_pollThreadDone = true;
    EXPECT_EQ(m_PosixCopyWriter->Destroy(), BackupRetCode::SUCCESS);

    m_PosixCopyWriter->m_pollThreadDone = false;
    EXPECT_EQ(m_PosixCopyWriter->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    m_PosixCopyWriter->m_threadDone = false;
    m_PosixCopyWriter->m_pollThreadDone = true;
    EXPECT_EQ(m_PosixCopyWriter->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);
}

/*
 * 用例名称：GetStatus_NotComplete
 * 前置条件：无
 * check点：校验GetStatus被未完成
 */
TEST_F(PosixCopyWriterTest, GetStatus_NotComplete)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = false;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);
}

/*
 * 用例名称：GetStatus_Abort
 * 前置条件：无
 * check点：校验GetStatus被终止
 */
TEST_F(PosixCopyWriterTest, GetStatus_Abort)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_PosixCopyWriter->m_abort = true;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->GetStatus(), BackupPhaseStatus::ABORTED);
}

/*
 * 用例名称：GetStatus_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixCopyWriterTest, GetStatus_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_PosixCopyWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    controlInfo->m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->GetStatus(), BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE);
}

/*
 * 用例名称：GetStatus_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixCopyWriterTest, GetStatus_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_PosixCopyWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete失败
 */
TEST_F(PosixCopyWriterTest, IsComplete_Fail)
{
    EXPECT_EQ(m_PosixCopyWriter->IsComplete(), false);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete成功
 */
TEST_F(PosixCopyWriterTest, IsComplete_True)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_aggregatePhaseComplete = true;
    m_PosixCopyWriter->m_writeQueue->Clear();
    m_PosixCopyWriter->m_writeCache.clear();
    m_PosixCopyWriter->m_timer.m_map.clear();
    m_PosixCopyWriter->m_writeTaskProduce = 0;
    m_PosixCopyWriter->m_writeTaskConsume = 0;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->IsComplete(), true);
}

/*
 * 用例名称：IsAbort_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixCopyWriterTest, IsAbort_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_PosixCopyWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->IsAbort(), true);
}

/*
 * 用例名称：IsAbort_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixCopyWriterTest, IsAbort_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_PosixCopyWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_PosixCopyWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_PosixCopyWriter->IsAbort(), false);
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
TEST_F(PosixCopyWriterTest, OpenFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixCopyWriter->OpenFile(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：OpenFile_Fail
 * 前置条件：无
 * check点：校验OpenFile失败
 */
TEST_F(PosixCopyWriterTest, OpenFile_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixCopyWriter->OpenFile(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteMeta_Suc
 * 前置条件：无
 * check点：校验WriteMeta成功
 */
TEST_F(PosixCopyWriterTest, WriteMeta_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixCopyWriter->WriteMeta(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteMeta_Fail
 * 前置条件：无
 * check点：校验WriteMeta失败
 */
TEST_F(PosixCopyWriterTest, WriteMeta_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixCopyWriter->WriteMeta(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteData_Fail
 * 前置条件：无
 * check点：校验WriteData失败
 */
TEST_F(PosixCopyWriterTest, WriteData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixCopyWriter->WriteData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteData_Suc
 * 前置条件：无
 * check点：校验WriteData成功
 */
TEST_F(PosixCopyWriterTest, WriteData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixCopyWriter->WriteData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Suc
 * 前置条件：无
 * check点：校验CloseFile成功
 */
TEST_F(PosixCopyWriterTest, CloseFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 1;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixCopyWriter->CloseFile(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Fail
 * 前置条件：无
 * check点：校验CloseFile失败
 */
TEST_F(PosixCopyWriterTest, CloseFile_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixCopyWriter->CloseFile(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CreateDir
 * 前置条件：无
 * check点：校验CreateDir
 */
TEST_F(PosixCopyWriterTest, CreateDir)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_NO_THROW(m_PosixCopyWriter->CreateDir(fileHandle));
    stub.reset(ADDR(Module::JobScheduler, Put));

    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_NO_THROW(m_PosixCopyWriter->CreateDir(fileHandle));
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ClearWriteCache
 * 前置条件：无
 * check点：校验ClearWriteCache
 */
TEST_F(PosixCopyWriterTest, ClearWriteCache)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_PosixCopyWriter->m_writeCache.clear();
    EXPECT_NO_THROW(m_PosixCopyWriter->ClearWriteCache());

    fileHandle.m_file->m_dstState = FileDescState::LSTAT;
    std::vector<FileHandle> files;
    files.push_back(fileHandle);
    m_PosixCopyWriter->m_writeCache.insert(pair<string, std::vector<FileHandle>>("test1", files));
    EXPECT_NO_THROW(m_PosixCopyWriter->ClearWriteCache());

    fileHandle.m_file->m_dstState = FileDescState::INIT;
    files.clear();
    files.push_back(fileHandle);
    m_PosixCopyWriter->m_writeCache.clear();
    m_PosixCopyWriter->m_writeCache.insert(pair<string, std::vector<FileHandle>>("test2", files));
    EXPECT_NO_THROW(m_PosixCopyWriter->ClearWriteCache());
}

/*
 * 用例名称：InsertWriteCache
 * 前置条件：无
 * check点：校验InsertWriteCache
 */
TEST_F(PosixCopyWriterTest, InsertWriteCache)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_NO_THROW(m_PosixCopyWriter->InsertWriteCache(fileHandle));
}

/*
 * 用例名称：InsertWriteCache
 * 前置条件：无
 * check点：校验InsertWriteCache
 */
TEST_F(PosixCopyWriterTest, ProcessWriteEntries)
{
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 16384;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 0;
    m_PosixCopyWriter->m_params.blockSize = 10;
    fileHandle.m_file->m_dstState = FileDescState::INIT;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 11;
    m_PosixCopyWriter->m_params.blockSize = 10;
    fileHandle.m_file->m_dstState = FileDescState::INIT;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dstState = FileDescState::PARTIAL_WRITED;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dstState = FileDescState::WRITED;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_dstState = FileDescState::DST_CLOSED;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteEntries(fileHandle));
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：IsOpenBlock
 * 前置条件：无
 * check点：校验IsOpenBlock
 */
TEST_F(PosixCopyWriterTest, IsOpenBlock)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_PosixCopyWriter->IsOpenBlock(fileHandle), true);
}

/*
 * 用例名称：ProcessTimers
 * 前置条件：无
 * check点：校验ProcessTimers
 */
TEST_F(PosixCopyWriterTest, ProcessTimers)
{
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessTimers());
}

/*
 * 用例名称：PollWriteTask
 * 前置条件：无
 * check点：校验PollWriteTask
 */
TEST_F(PosixCopyWriterTest, PollWriteTask)
{
    m_PosixCopyWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_NO_THROW(m_PosixCopyWriter->PollWriteTask());
}

/*
 * 用例名称：ProcessWriteData
 * 前置条件：无
 * check点：校验ProcessWriteData
 */
TEST_F(PosixCopyWriterTest, ProcessWriteData)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteData(fileHandle));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 0;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->m_size = 0;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteData(fileHandle));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 0;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->m_size = 0;
    m_PosixCopyWriter->m_params.writeMeta = false;
    EXPECT_NO_THROW(m_PosixCopyWriter->ProcessWriteData(fileHandle));
}

/*
 * 用例名称：HandleSuccessEvent
 * 前置条件：无
 * check点：校验HandleSuccessEvent
 */
TEST_F(PosixCopyWriterTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::END;
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::OPEN_DST;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::WRITE_SKIP;
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::WRITE_META;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::END;
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::WRITE_META;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::END;
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::CLOSE_DST;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixCopyWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::INVALID;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleSuccessEvent(taskPtr));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：无
 * check点：校验HandleFailedEvent
 */
TEST_F(PosixCopyWriterTest, HandleFailedEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleFailedEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_retryCnt= DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_PosixCopyWriter->HandleFailedEvent(taskPtr));
}