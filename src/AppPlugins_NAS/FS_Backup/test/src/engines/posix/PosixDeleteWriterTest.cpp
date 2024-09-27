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
#include "PosixDeleteWriter.h"
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

class PosixDeleteWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixDeleteWriter> m_posixDeleteWriter = nullptr;
    Stub stub;
};

void PosixDeleteWriterTest::SetUp()
{
    BackupQueueConfig config { DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE };
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    WriterParams deleteWriterParams {};
    deleteWriterParams.backupParams = params;
    deleteWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_posixDeleteWriter = std::make_unique<PosixDeleteWriter>(deleteWriterParams);
    m_posixDeleteWriter->m_jsPtr =
        make_shared<Module::JobScheduler>(*Module::ThreadPoolFactory::GetThreadPoolInstance("test", 2));
}

void PosixDeleteWriterTest::TearDown() {}

void PosixDeleteWriterTest::SetUpTestCase() {}

void PosixDeleteWriterTest::TearDownTestCase() {}


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

static bool Put_Stub_Suc()
{
    return true;
}

static bool Put_Stub_Fail()
{
    return false;
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete
 */
TEST_F(PosixDeleteWriterTest, IsComplete)
{
    // EXPECT_EQ(m_posixDeleteWriter->IsComplete(), false);
}

/*
 * 用例名称：Abort
 * 前置条件：无
 * check点：校验Abort
 */
TEST_F(PosixDeleteWriterTest, Abort)
{
    EXPECT_EQ(m_posixDeleteWriter->Abort(), BackupRetCode::SUCCESS);
}

/*
 * 用例名称：GetStatus_NotComplete
 * 前置条件：无
 * check点：校验GetStatus被未完成
 */
TEST_F(PosixDeleteWriterTest, GetStatus_NotComplete)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = false;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);
}

/*
 * 用例名称：GetStatus_Abort
 * 前置条件：无
 * check点：校验GetStatus被终止
 */
TEST_F(PosixDeleteWriterTest, GetStatus_Abort)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_posixDeleteWriter->m_abort = true;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->GetStatus(), BackupPhaseStatus::ABORTED);
}

/*
 * 用例名称：GetStatus_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixDeleteWriterTest, GetStatus_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_posixDeleteWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    controlInfo->m_backupFailReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->GetStatus(), BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE);
}

/*
 * 用例名称：GetStatus_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixDeleteWriterTest, GetStatus_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_writePhaseComplete = true;
    m_posixDeleteWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}


/*
 * 用例名称：WriteData_Fail
 * 前置条件：无
 * check点：校验WriteData失败
 */
TEST_F(PosixDeleteWriterTest, WriteData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    m_posixDeleteWriter->m_jsPtr = make_shared<Module::JobScheduler>(
        *Module::ThreadPoolFactory::GetThreadPoolInstance(
        m_posixDeleteWriter->m_threadPoolKey, m_posixDeleteWriter->m_dstAdvParams->threadNum));
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_posixDeleteWriter->WriteData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteData_Suc
 * 前置条件：无
 * check点：校验WriteData成功
 */
TEST_F(PosixDeleteWriterTest, WriteData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    m_posixDeleteWriter->m_jsPtr = make_shared<Module::JobScheduler>(
        *Module::ThreadPoolFactory::GetThreadPoolInstance(
        m_posixDeleteWriter->m_threadPoolKey, m_posixDeleteWriter->m_dstAdvParams->threadNum));
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_posixDeleteWriter->WriteData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile
 * 前置条件：无
 * check点：校验CloseFile成功
 */
TEST_F(PosixDeleteWriterTest, CloseFile)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_posixDeleteWriter->CloseFile(fileHandle), 0);
}

/*
 * 用例名称：IsAbort_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixDeleteWriterTest, IsAbort_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixDeleteWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->IsAbort(), true);
}

/*
 * 用例名称：IsAbort_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixDeleteWriterTest, IsAbort_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    m_posixDeleteWriter->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->IsAbort(), false);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete失败
 */
TEST_F(PosixDeleteWriterTest, IsComplete_Fail)
{
    EXPECT_EQ(m_posixDeleteWriter->IsComplete(), false);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete成功
 */
TEST_F(PosixDeleteWriterTest, IsComplete_True)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_aggregatePhaseComplete = true;
    m_posixDeleteWriter->m_writeQueue->Clear();
    m_posixDeleteWriter->m_timer.m_map.clear();
    m_posixDeleteWriter->m_writeTaskProduce = 0;
    m_posixDeleteWriter->m_writeTaskConsume = 0;
    m_posixDeleteWriter->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteWriter->IsComplete(), true);
}

/*
 * 用例名称：ProcessTimers
 * 前置条件：无
 * check点：校验ProcessTimers
 */
// TEST_F(PosixDeleteWriterTest, ProcessTimers)
// {
//     FileHandle fileHandle;
//     fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
//     m_posixDeleteWriter->m_timer.m_map.insert(pair<int64_t, FileHandle>(1, fileHandle));
//     EXPECT_NE(m_posixDeleteWriter->ProcessTimers(), 0);
// }

/*
 * 用例名称：OpenFile
 * 前置条件：无
 * check点：校验OpenFile
 */
TEST_F(PosixDeleteWriterTest, OpenFile)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_posixDeleteWriter->OpenFile(fileHandle), 0);
}

/*
 * 用例名称：WriteMeta
 * 前置条件：无
 * check点：校验WriteMeta
 */
TEST_F(PosixDeleteWriterTest, WriteMeta)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_posixDeleteWriter->WriteMeta(fileHandle), 0);
}


/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：校验ThreadFunc
 */
TEST_F(PosixDeleteWriterTest, ThreadFunc)
{
    stub.set(ADDR(PosixDeleteWriter, IsComplete), IsComplete_Stub_Suc);
    EXPECT_NO_THROW(m_posixDeleteWriter->ThreadFunc());
    stub.reset(ADDR(PosixDeleteWriter, IsComplete));

    stub.set(ADDR(PosixDeleteWriter, IsComplete), IsComplete_Stub_Fail);
    stub.set(ADDR(PosixDeleteWriter, IsAbort), IsAbort_Stub_Suc);
    EXPECT_NO_THROW(m_posixDeleteWriter->ThreadFunc());
    stub.reset(ADDR(PosixDeleteWriter, IsAbort));
    stub.reset(ADDR(PosixDeleteWriter, IsComplete));
}

/*
 * 用例名称：PollWriteTask
 * 前置条件：无
 * check点：校验PollWriteTask
 */
TEST_F(PosixDeleteWriterTest, PollWriteTask)
{
    m_posixDeleteWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_NO_THROW(m_posixDeleteWriter->PollWriteTask());
}


/*
 * 用例名称：HandleSuccessEvent
 * 前置条件：无
 * check点：校验HandleSuccessEvent
 */
TEST_F(PosixDeleteWriterTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->SetFlag(IS_DIR);
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_posixDeleteWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->ClearFlag(IS_DIR);
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::INIT;
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_posixDeleteWriter->HandleSuccessEvent(taskPtr));
}

static bool IsCriticalError_Stub()
{
    return true;
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：无
 * check点：校验HandleFailedEvent
 */
// TEST_F(PosixDeleteWriterTest, HandleFailedEvent)
// {
//     FileHandle fileHandle;
//     fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
//     shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
//     taskPtr->m_fileHandle = fileHandle;
//     EXPECT_NO_THROW(m_posixDeleteWriter->HandleFailedEvent(taskPtr));

//     fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
//     fileHandle.m_retryCnt= DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
//     taskPtr = make_shared<PosixServiceTask>();
//     taskPtr->m_fileHandle = fileHandle;
//     EXPECT_NO_THROW(m_posixDeleteWriter->HandleFailedEvent(taskPtr));

//     fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
//     fileHandle.m_retryCnt= DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
//     fileHandle.m_file->SetFlag(IS_DIR);
//     taskPtr = make_shared<PosixServiceTask>();
//     taskPtr->m_fileHandle = fileHandle;
//     EXPECT_NO_THROW(m_posixDeleteWriter->HandleFailedEvent(taskPtr));

//     fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
//     fileHandle.m_retryCnt= DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
//     fileHandle.m_file->SetFlag(IS_DIR);
//     taskPtr = make_shared<PosixServiceTask>();
//     taskPtr->m_fileHandle = fileHandle;
//     stub.set(ADDR(PosixServiceTask, IsCriticalError), IsCriticalError_Stub);
//     EXPECT_NO_THROW(m_posixDeleteWriter->HandleFailedEvent(taskPtr));
//     stub.reset(ADDR(PosixServiceTask, IsCriticalError));
// }