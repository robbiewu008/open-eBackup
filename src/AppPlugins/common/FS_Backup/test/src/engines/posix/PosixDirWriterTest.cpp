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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include "log/Log.h"
#include "PosixDirWriter.h"
#include "interface/PosixConstants.h"
#include "ThreadPoolFactory.h"
#include "PosixServiceTask.h"
#include "PosixUtils.h"

using ::testing::_;
using ::testing::AtLeast;
using testing::DoAll;
using testing::InitGoogleMock;
using ::testing::Return;
using ::testing::Sequence;
using ::testing::SetArgReferee;
using ::testing::Throw;

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int RETRY_TIME_MILLISENCOND = 1000;
}

class PosixDirWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    unique_ptr<PosixDirWriter> m_DirWriter = nullptr;
};

void PosixDirWriterTest::SetUp()
{
    BackupQueueConfig config;
    config.maxMemorySize = 1;
    config.maxMemorySize = 2;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    HostBackupAdvanceParams posixBackupAdvanceParams;
    posixBackupAdvanceParams.dataPath = "/ll";
    params.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    params.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);

    WriterParams dirWriterParams;
    dirWriterParams.backupParams = params;
    dirWriterParams.writeQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    dirWriterParams.controlInfo = make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = make_shared<BlockBufferMap>();

    m_DirWriter = make_unique<PosixDirWriter>(dirWriterParams);
}

void PosixDirWriterTest::TearDown()
{}

void PosixDirWriterTest::SetUpTestCase()
{}

void PosixDirWriterTest::TearDownTestCase()
{}

static bool Stub_Put_False(void* obj)
{
    return false;
}
/*
* 用例名称：WriteMeta
* 前置条件：无
* check点：写入Meta数据失败
*/
// TEST_F(PosixDirWriterTest, WriteMeta)
// {
//     FileHandle fileHandle;
//     BlockBuffer blockBuffer;
//     blockBuffer.m_size = 1;
//     fileHandle.m_block = blockBuffer;
//     fileHandle.m_retryCnt= 1;

//     Module::ThreadPool threadPool;
//     threadPool.m_threadNum = 1;
//     m_DirWriter->m_jsPtr = make_shared<Module::JobScheduler>();
//     stub.set(ADDR(Module::JobScheduler, Put), Stub_Put_False);
//     int ret = m_DirWriter->WriteMeta(fileHandle);
//     EXPECT_EQ(ret, FAILED);
// }

static int  Stub_GetExpiredEventAndTime_fileHandles(void* obj, vector<FileHandle> &fileHandles)
{
    FileHandle fileHandle;
    // FileDesc fileDesc;
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = make_shared<FileDesc>(srcIoEngine, dstIoEngine);

    BlockBuffer blockBuffer;
    blockBuffer.m_size = 1;
    fileHandle.m_block = blockBuffer;
    fileHandle.m_retryCnt= 1;
    fileHandles.push_back(fileHandle);
    fileHandles.push_back(fileHandle);
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
 * 用例名称：HandleSuccessEvent
 * 前置条件：无
 * check点：校验HandleSuccessEvent
 */
TEST_F(PosixDirWriterTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_DirWriter->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_DirWriter->m_params.blockSize + 1;
    taskPtr = make_shared<PosixServiceTask>();
    fileHandle.m_file->m_dstState = FileDescState::INIT;
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event = HostEvent::OPEN_DST;
    EXPECT_NO_THROW(m_DirWriter->HandleSuccessEvent(taskPtr));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：无
 * check点：校验HandleFailedEvent
 */
TEST_F(PosixDirWriterTest, HandleFailedEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_DirWriter->HandleFailedEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_retryCnt= DEFAULT_ERROR_SINGLE_FILE_CNT + 1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_DirWriter->HandleFailedEvent(taskPtr));
}

/*
 * 用例名称：WriteMeta_Suc
 * 前置条件：无
 * check点：校验WriteMeta成功
 */
TEST_F(PosixDirWriterTest, WriteMeta_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_DirWriter->m_jsPtr = make_shared<Module::JobScheduler>(
        *Module::ThreadPoolFactory::GetThreadPoolInstance(
        m_DirWriter->m_threadPoolKey, m_DirWriter->m_dstAdvParams->threadNum));
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_DirWriter->WriteMeta(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：WriteMeta_Fail
 * 前置条件：无
 * check点：校验WriteMeta失败
 */
TEST_F(PosixDirWriterTest, WriteMeta_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    m_DirWriter->m_jsPtr = make_shared<Module::JobScheduler>(
        *Module::ThreadPoolFactory::GetThreadPoolInstance(
        m_DirWriter->m_threadPoolKey, m_DirWriter->m_dstAdvParams->threadNum));
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_DirWriter->WriteMeta(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
* 用例名称：ProcessTimers
* 前置条件：无
* check点：校验ProcessTimers
*/
TEST_F(PosixDirWriterTest, ProcessTimers)
{
    // stub.set((int64_t(BackupTimer::*)(vector<FileHandle> &))ADDR(BackupTimer,GetExpiredEventAndTime), Stub_GetExpiredEventAndTime_fileHandles);
    EXPECT_NO_THROW(m_DirWriter->ProcessTimers());
}
/*
* 用例名称：Abort
* 前置条件：无
* check点：判断是否中断成功
*/
TEST_F(PosixDirWriterTest, Abort)
{
    BackupRetCode ret = m_DirWriter->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

static bool Stub_PosixDirReader_True(void* obj)
{
    return true;
}

static bool Stub_PosixDirReader_False(void* obj)
{
    return false;
}
/*
* 用例名称：ThreadFunc
* 前置条件：无
* check点：线程正常执行完
*/
TEST_F(PosixDirWriterTest, ThreadFunc)
{
    stub.set(ADDR(PosixDirWriter, IsComplete), Stub_PosixDirReader_True);
    EXPECT_NO_THROW(m_DirWriter->ThreadFunc());
    stub.reset(ADDR(PosixDirWriter, IsComplete));
}

/*
* 用例名称：ThreadFunc
* 前置条件：无
* check点：线程正常中断
*/
TEST_F(PosixDirWriterTest, ThreadFunc2)
{
    stub.set(ADDR(PosixDirWriter, IsComplete), Stub_PosixDirReader_False);
    stub.set(ADDR(PosixDirWriter, IsAbort), Stub_PosixDirReader_True);
    EXPECT_NO_THROW(m_DirWriter->ThreadFunc());
    stub.reset(ADDR(PosixDirWriter, IsComplete));
    stub.reset(ADDR(PosixDirWriter, IsAbort));
}

static bool Stub_WaitAndPop_True(void* obj, FileHandle& fileHandle, uint32_t timeOut)
{
    BackupIOEngine srcIoEngine;
    BackupIOEngine dstIoEngine;
    fileHandle.m_file = make_shared<FileDesc>(srcIoEngine, dstIoEngine);
    fileHandle.m_file->m_dirName = "/mm";
    return false;
}

static void Stub_WaitAndPush_True(void* obj, FileHandle fileHandle)
{
    fileHandle.m_file->m_dirName = "/mm";
    return;
}

/*
* 用例名称：OpenFile
* 前置条件：无
* check点：打开文件，并去去文件中的数据
*/
TEST_F(PosixDirWriterTest, OpenFile)
{
    FileHandle fileHandle;
    BlockBuffer blockBuffer;
    blockBuffer.m_size = 1;
    fileHandle.m_block = blockBuffer;
    fileHandle.m_retryCnt= 1;
    int ret = m_DirWriter->OpenFile(fileHandle);
    ret = m_DirWriter->WriteData(fileHandle);
    ret = m_DirWriter->CloseFile(fileHandle);
    EXPECT_EQ(ret, 0);
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行失败
*/
TEST_F(PosixDirWriterTest, IsComplete)
{
    bool ret = m_DirWriter->IsComplete();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行成功
*/
TEST_F(PosixDirWriterTest, IsComplete2)
{
    BackupQueueConfig config;
    config.maxMemorySize = 1;
    config.maxMemorySize = 2;

    m_DirWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_DirWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_DirWriter->m_writeTaskProduce = 0;
    m_DirWriter->m_writeTaskConsume = 0;

    m_DirWriter->m_controlInfo->m_noOfDirCopied = 0;
    m_DirWriter->m_controlInfo->m_noOfDirFailed = 0;
    m_DirWriter->m_controlInfo->m_controlFileReaderProduce = 0;
    bool ret = m_DirWriter->IsComplete();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程成功终止
*/
TEST_F(PosixDirWriterTest, IsAbort)
{
    m_DirWriter->m_abort = true;
    bool ret = m_DirWriter->IsAbort();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程终止失败
*/
TEST_F(PosixDirWriterTest, IsAbort2)
{
    m_DirWriter->m_abort = false;
    m_DirWriter->m_controlInfo->m_failed = false;
    m_DirWriter->m_controlInfo->m_controlReaderFailed = false;
    bool ret = m_DirWriter->IsAbort();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirWriterTest, GetStatus)
{
    m_DirWriter->m_controlInfo->m_writePhaseComplete = false;
    BackupPhaseStatus ret = m_DirWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirWriterTest, GetStatus2)
{
    m_DirWriter->m_controlInfo->m_writePhaseComplete = true;
    m_DirWriter->m_abort = true;
    BackupPhaseStatus ret = m_DirWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirWriterTest, GetStatus3)
{
    m_DirWriter->m_controlInfo->m_writePhaseComplete = true;
    m_DirWriter->m_abort = false;
    m_DirWriter->m_controlInfo->m_failed = true;
    BackupPhaseStatus ret = m_DirWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirWriterTest, GetStatus4)
{
    m_DirWriter->m_controlInfo->m_writePhaseComplete = true;
    m_DirWriter->m_abort = false;
    m_DirWriter->m_controlInfo->m_failed = false;
    m_DirWriter->m_controlInfo->m_controlReaderFailed = false;
    BackupPhaseStatus ret = m_DirWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}
