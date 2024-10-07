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
#include "PosixHardlinkReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
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
    const int OOM_SLEEP_SECOND = 1;
    const int RETRY_TIME_MILLISENCOND = 1000;
    const uint32_t INVALID_MEMORY = static_cast<uint32_t>(-1);
}

class PosixHardlinkReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    unique_ptr<PosixHardlinkReader> m_PosixHardlinkReader = nullptr;
};

void PosixHardlinkReaderTest::SetUp()
{
    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    HostBackupAdvanceParams posixBackupAdvanceParams;
    posixBackupAdvanceParams.dataPath = "/ll";
    params.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    params.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);

    ReaderParams hardlinkReaderParams;
    hardlinkReaderParams.backupParams = params;
    hardlinkReaderParams.readQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.aggregateQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.controlInfo = make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = make_shared<BlockBufferMap>();

    m_PosixHardlinkReader = make_unique<PosixHardlinkReader>(hardlinkReaderParams);
    m_PosixHardlinkReader->m_jsPtr =
        make_shared<Module::JobScheduler>(*Module::ThreadPoolFactory::GetThreadPoolInstance("test", 2));
}

void PosixHardlinkReaderTest::TearDown()
{}

void PosixHardlinkReaderTest::SetUpTestCase()
{}

void PosixHardlinkReaderTest::TearDownTestCase()
{}

/*
* 用例名称：Abort
* 前置条件：无
* check点：判断是否中断成功
*/
TEST_F(PosixHardlinkReaderTest, Abort)
{
    BackupRetCode ret = m_PosixHardlinkReader->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

/*
 * 用例名称：Destroy
 * 前置条件：无
 * check点：校验Destroy
 */
TEST_F(PosixHardlinkReaderTest, Destroy)
{
    m_PosixHardlinkReader->m_threadDone = true;
    m_PosixHardlinkReader->m_pollThreadDone = true;
    EXPECT_EQ(m_PosixHardlinkReader->Destroy(), BackupRetCode::SUCCESS);

    m_PosixHardlinkReader->m_pollThreadDone = false;
    EXPECT_EQ(m_PosixHardlinkReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    m_PosixHardlinkReader->m_threadDone = false;
    m_PosixHardlinkReader->m_pollThreadDone = true;
    EXPECT_EQ(m_PosixHardlinkReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);
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
* check点：线程正常中断
*/
TEST_F(PosixHardlinkReaderTest, ThreadFunc2)
{
    stub.set(ADDR(PosixHardlinkReader, IsComplete), Stub_PosixDirReader_False);
    stub.set(ADDR(PosixHardlinkReader, IsAbort), Stub_PosixDirReader_True);
    EXPECT_NO_THROW(m_PosixHardlinkReader->ThreadFunc());
    stub.reset(ADDR(PosixHardlinkReader, IsComplete));
    stub.reset(ADDR(PosixHardlinkReader, IsAbort));
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行失败
*/
TEST_F(PosixHardlinkReaderTest, IsComplete)
{
    bool ret = m_PosixHardlinkReader->IsComplete();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行成功
*/
TEST_F(PosixHardlinkReaderTest, IsComplete2)
{
    BackupQueueConfig config;
    config.maxMemorySize = 1;
    config.maxMemorySize = 2;

    m_PosixHardlinkReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_PosixHardlinkReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_PosixHardlinkReader->m_controlInfo->m_controlFileReaderProduce = 0;
    m_PosixHardlinkReader->m_controlInfo->m_readConsume = 0;
    bool ret = m_PosixHardlinkReader->IsComplete();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程成功终止
*/
TEST_F(PosixHardlinkReaderTest, IsAbort)
{
    m_PosixHardlinkReader->m_abort = true;
    bool ret = m_PosixHardlinkReader->IsAbort();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程终止失败
*/
TEST_F(PosixHardlinkReaderTest, IsAbort2)
{
    m_PosixHardlinkReader->m_abort = false;
    m_PosixHardlinkReader->m_controlInfo->m_failed = false;
    m_PosixHardlinkReader->m_controlInfo->m_controlReaderFailed = false;
    bool ret = m_PosixHardlinkReader->IsAbort();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixHardlinkReaderTest, GetStatus)
{
    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = false;
    BackupPhaseStatus ret = m_PosixHardlinkReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixHardlinkReaderTest, GetStatus2)
{
    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = true;
    m_PosixHardlinkReader->m_abort = true;
    BackupPhaseStatus ret = m_PosixHardlinkReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixHardlinkReaderTest, GetStatus3)
{
    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = true;
    m_PosixHardlinkReader->m_abort = false;
    m_PosixHardlinkReader->m_controlInfo->m_failed = true;
    BackupPhaseStatus ret = m_PosixHardlinkReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixHardlinkReaderTest, GetStatus4)
{
    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = true;
    m_PosixHardlinkReader->m_abort = false;
    m_PosixHardlinkReader->m_controlInfo->m_failed = false;
    m_PosixHardlinkReader->m_controlInfo->m_controlReaderFailed = false;
    BackupPhaseStatus ret = m_PosixHardlinkReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}


static void Stub_PushToAggregator(void* obj)
{
    return;
}

static void Stub_PushToReader(void* obj)
{
    return;
}

/*
 * 用例名称：HandleSuccessEvent
 * 前置条件：无
 * check点：校验HandleSuccessEvent
 */
TEST_F(PosixHardlinkReaderTest, HandleSuccessEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    stub.set(ADDR(PosixHardlinkReader, PushToAggregator), Stub_PushToAggregator);
    stub.set(ADDR(PosixHardlinkReader, PushToReader), Stub_PushToReader);
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixHardlinkReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::CLOSE_SRC;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixHardlinkReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::OPEN_SRC;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleSuccessEvent(taskPtr));
    
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixHardlinkReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::READ_DATA;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleSuccessEvent(taskPtr));

    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_size = m_PosixHardlinkReader->m_params.blockSize +1;
    taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    taskPtr->m_event= HostEvent::INVALID;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleSuccessEvent(taskPtr));
    stub.reset(ADDR(PosixHardlinkReader, PushToAggregator));
    stub.reset(ADDR(PosixHardlinkReader, PushToReader));
}

/*
 * 用例名称：HandleFailedEvent
 * 前置条件：无
 * check点：校验HandleFailedEvent
 */
TEST_F(PosixHardlinkReaderTest, HandleFailedEvent)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    shared_ptr<PosixServiceTask> taskPtr = make_shared<PosixServiceTask>();
    taskPtr->m_fileHandle = fileHandle;
    EXPECT_NO_THROW(m_PosixHardlinkReader->HandleFailedEvent(taskPtr));
}

/*
 * 用例名称：PushToAggregator
 * 前置条件：无
 * check点：校验PushToAggregator
 */
TEST_F(PosixHardlinkReaderTest, PushToAggregator)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_NO_THROW(m_PosixHardlinkReader->PushToAggregator(fileHandle));
}

/*
 * 用例名称：PushToReader
 * 前置条件：无
 * check点：校验PushToReader
 */
TEST_F(PosixHardlinkReaderTest, PushToReader)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_NO_THROW(m_PosixHardlinkReader->PushToReader(fileHandle));
}

/*
 * 用例名称：DecomposeAndPush
 * 前置条件：无
 * check点：校验DecomposeAndPush
 */
TEST_F(PosixHardlinkReaderTest, DecomposeAndPush)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    uint64_t startOffset = 1;
    uint64_t totalSize = 10;
    uint64_t startSeq = 20;
    EXPECT_NO_THROW(m_PosixHardlinkReader->DecomposeAndPush(fileHandle, startOffset, totalSize, startSeq));
}

/*
 * 用例名称：DecomposeAndPush2
 * 前置条件：无
 * check点：校验DecomposeAndPush
 */
TEST_F(PosixHardlinkReaderTest, DecomposeAndPush2)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_NO_THROW(m_PosixHardlinkReader->DecomposeAndPush(fileHandle));
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
TEST_F(PosixHardlinkReaderTest, OpenFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixHardlinkReader->OpenFile(fileHandle), false);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadEmptyData_Suc
 * 前置条件：无
 * check点：校验ReadEmptyData成功
 */
TEST_F(PosixHardlinkReaderTest, ReadEmptyData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    m_PosixHardlinkReader->m_blockBufferMap = make_shared<BlockBufferMap>();
    EXPECT_EQ(m_PosixHardlinkReader->ReadEmptyData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadEmptyData_Fail
 * 前置条件：无
 * check点：校验ReadEmptyData失败
 */
TEST_F(PosixHardlinkReaderTest, ReadEmptyData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixHardlinkReader->ReadEmptyData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}


/*
 * 用例名称：ReadSymlinkData_Suc
 * 前置条件：无
 * check点：校验ReadSymlinkData成功
 */
TEST_F(PosixHardlinkReaderTest, ReadSymlinkData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    m_PosixHardlinkReader->m_blockBufferMap = make_shared<BlockBufferMap>();
    EXPECT_EQ(m_PosixHardlinkReader->ReadSymlinkData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadSymlinkData_Fail
 * 前置条件：无
 * check点：校验ReadSymlinkData失败
 */
TEST_F(PosixHardlinkReaderTest, ReadSymlinkData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixHardlinkReader->ReadSymlinkData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadNormalData_Suc
 * 前置条件：无
 * check点：校验ReadNormalData成功
 */
TEST_F(PosixHardlinkReaderTest, ReadNormalData_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    m_PosixHardlinkReader->m_blockBufferMap = make_shared<BlockBufferMap>();
    EXPECT_EQ(m_PosixHardlinkReader->ReadNormalData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadNormalData_Fail
 * 前置条件：无
 * check点：校验ReadNormalData失败
 */
TEST_F(PosixHardlinkReaderTest, ReadNormalData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixHardlinkReader->ReadNormalData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadData_Fail
 * 前置条件：无
 * check点：校验ReadData失败
 */
TEST_F(PosixHardlinkReaderTest, ReadData_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 40960;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixHardlinkReader->ReadData(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadData_Empty
 * 前置条件：无
 * check点：校验ReadData创建empty
 */
TEST_F(PosixHardlinkReaderTest, ReadData_Empty)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 0;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    m_PosixHardlinkReader->m_blockBufferMap = make_shared<BlockBufferMap>();
    EXPECT_EQ(m_PosixHardlinkReader->ReadData(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Suc
 * 前置条件：无
 * check点：校验CloseFile成功
 */
TEST_F(PosixHardlinkReaderTest, CloseFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_size = 1;
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Suc);
    EXPECT_EQ(m_PosixHardlinkReader->CloseFile(fileHandle), 0);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：CloseFile_Fail
 * 前置条件：无
 * check点：校验CloseFile失败
 */
TEST_F(PosixHardlinkReaderTest, CloseFile_Fail)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    stub.set(ADDR(Module::JobScheduler, Put), Put_Stub_Fail);
    EXPECT_EQ(m_PosixHardlinkReader->CloseFile(fileHandle), -1);
    stub.reset(ADDR(Module::JobScheduler, Put));
}

/*
 * 用例名称：ReadMeta_Suc
 * 前置条件：无
 * check点：校验ReadMeta成功
 */
TEST_F(PosixHardlinkReaderTest, ReadMeta_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    EXPECT_EQ(m_PosixHardlinkReader->ReadMeta(fileHandle), 0);
}


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

/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：校验ThreadFunc
 */
TEST_F(PosixHardlinkReaderTest, ThreadFunc)
{
    stub.set(ADDR(PosixHardlinkReader, IsComplete), IsComplete_Stub_Suc);
    EXPECT_NO_THROW(m_PosixHardlinkReader->ThreadFunc());
    stub.reset(ADDR(PosixHardlinkReader, IsComplete));

    stub.set(ADDR(PosixHardlinkReader, IsComplete), IsComplete_Stub_Fail);
    stub.set(ADDR(PosixHardlinkReader, IsAbort), IsAbort_Stub_Suc);
    EXPECT_NO_THROW(m_PosixHardlinkReader->ThreadFunc());
    stub.reset(ADDR(PosixHardlinkReader, IsAbort));
    stub.reset(ADDR(PosixHardlinkReader, IsComplete));
}

static bool Get_Stub_Suc(){
    return true;
}

/*
 * 用例名称：PollReadTask
 * 前置条件：无
 * check点：校验PollReadTask
 */
TEST_F(PosixHardlinkReaderTest, PollReadTask)
{
    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_NO_THROW(m_PosixHardlinkReader->PollReadTask());

    m_PosixHardlinkReader->m_controlInfo->m_readPhaseComplete = false;
    stub.set(ADDR(Module::JobScheduler, Get), Get_Stub_Suc);
    EXPECT_NO_THROW(m_PosixHardlinkReader->PollReadTask());
    stub.reset(ADDR(Module::JobScheduler, Get));
}
