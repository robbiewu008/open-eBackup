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
#include "log/Log.h"
#include "ThreadPoolFactory.h"
#include "stub.h"
#include "PosixDirReader.h"
#include "BackupQueue.h"
#include "FileAggregator.h"


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

class PosixDirReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
    unique_ptr<PosixDirReader> m_DirReader = nullptr;
};

void PosixDirReaderTest::SetUp()
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

    ReaderParams dirReaderParams;
    dirReaderParams.backupParams = params;
    dirReaderParams.readQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.aggregateQueuePtr = make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.controlInfo = make_shared<BackupControlInfo>();

    m_DirReader = make_unique<PosixDirReader>(dirReaderParams);
}

void PosixDirReaderTest::TearDown()
{}

void PosixDirReaderTest::SetUpTestCase()
{}

void PosixDirReaderTest::TearDownTestCase()
{}

/*
* 用例名称：Abort
* 前置条件：无
* check点：判断是否中断成功
*/
TEST_F(PosixDirReaderTest, Abort)
{
    BackupRetCode ret = m_DirReader->Abort();
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
TEST_F(PosixDirReaderTest, ThreadFunc)
{
    stub.set(ADDR(PosixDirReader, IsComplete), Stub_PosixDirReader_True);
    EXPECT_NO_THROW(m_DirReader->ThreadFunc());
    stub.reset(ADDR(PosixDirReader, IsComplete));
}

/*
* 用例名称：ThreadFunc
* 前置条件：无
* check点：线程正常中断
*/
TEST_F(PosixDirReaderTest, ThreadFunc2)
{
    stub.set(ADDR(PosixDirReader, IsComplete), Stub_PosixDirReader_False);
    stub.set(ADDR(PosixDirReader, IsAbort), Stub_PosixDirReader_True);
    EXPECT_NO_THROW(m_DirReader->ThreadFunc());
    stub.reset(ADDR(PosixDirReader, IsComplete));
    stub.reset(ADDR(PosixDirReader, IsAbort));
}

/*
 * 用例名称：Destroy
 * 前置条件：ThreadFunc校验成功
 * check点：校验Destroy
 */
TEST_F(PosixDirReaderTest, Destroy)
{
    m_DirReader->m_threadDone = false;
    EXPECT_EQ(m_DirReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    stub.set(ADDR(PosixDirReader, IsComplete), Stub_PosixDirReader_True);
    m_DirReader->ThreadFunc();
    stub.reset(ADDR(PosixDirReader, IsComplete));
    EXPECT_EQ(m_DirReader->Destroy(), BackupRetCode::SUCCESS);
    m_DirReader->m_threadDone = false;

    stub.set(ADDR(PosixDirReader, IsComplete), Stub_PosixDirReader_False);
    stub.set(ADDR(PosixDirReader, IsAbort), Stub_PosixDirReader_True);
    m_DirReader->ThreadFunc();
    stub.reset(ADDR(PosixDirReader, IsAbort));
    stub.reset(ADDR(PosixDirReader, IsComplete));
    EXPECT_EQ(m_DirReader->Destroy(), BackupRetCode::SUCCESS);
    m_DirReader->m_threadDone = false;
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
TEST_F(PosixDirReaderTest, OpenFile)
{
    FileHandle fileHandle;
    BlockBuffer blockBuffer;
    blockBuffer.m_size = 1;
    fileHandle.m_block = blockBuffer;
    fileHandle.m_retryCnt= 1;
    int ret = m_DirReader->OpenFile(fileHandle);
    ret = m_DirReader->ReadMeta(fileHandle);
    ret = m_DirReader->ReadData(fileHandle);
    ret = m_DirReader->CloseFile(fileHandle);
    EXPECT_EQ(ret, 0);
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行失败
*/
TEST_F(PosixDirReaderTest, IsComplete)
{
    bool ret = m_DirReader->IsComplete();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：IsComplete
* 前置条件：无
* check点：线程执行成功
*/
TEST_F(PosixDirReaderTest, IsComplete2)
{
    BackupQueueConfig config;
    config.maxMemorySize = 1;
    config.maxMemorySize = 2;

    m_DirReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_DirReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_DirReader->m_controlInfo->m_controlFileReaderProduce = 0;
    m_DirReader->m_controlInfo->m_readConsume = 0;
    bool ret = m_DirReader->IsComplete();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程成功终止
*/
TEST_F(PosixDirReaderTest, IsAbort)
{
    m_DirReader->m_abort = true;
    bool ret = m_DirReader->IsAbort();
    EXPECT_EQ(ret, true);
}

/*
* 用例名称：IsAbort
* 前置条件：无
* check点：线程终止失败
*/
TEST_F(PosixDirReaderTest, IsAbort2)
{
    m_DirReader->m_abort = false;
    m_DirReader->m_controlInfo->m_failed = false;
    m_DirReader->m_controlInfo->m_controlReaderFailed = false;
    bool ret = m_DirReader->IsAbort();
    EXPECT_EQ(ret, false);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirReaderTest, GetStatus)
{
    m_DirReader->m_controlInfo->m_readPhaseComplete = false;
    BackupPhaseStatus ret = m_DirReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirReaderTest, GetStatus2)
{
    m_DirReader->m_controlInfo->m_readPhaseComplete = true;
    m_DirReader->m_abort = true;
    BackupPhaseStatus ret = m_DirReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirReaderTest, GetStatus3)
{
    m_DirReader->m_controlInfo->m_readPhaseComplete = true;
    m_DirReader->m_abort = false;
    m_DirReader->m_controlInfo->m_failed = true;
    BackupPhaseStatus ret = m_DirReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);
}

/*
* 用例名称：GetStatus
* 前置条件：无
* check点：获取状态码
*/
TEST_F(PosixDirReaderTest, GetStatus4)
{
    m_DirReader->m_controlInfo->m_readPhaseComplete = true;
    m_DirReader->m_abort = false;
    m_DirReader->m_controlInfo->m_failed = false;
    m_DirReader->m_controlInfo->m_controlReaderFailed = false;
    BackupPhaseStatus ret = m_DirReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}

/*
* 用例名称：Start
* 前置条件：无
* check点：开始成功运行
*/
// TEST_F(PosixDirReaderTest, Start)
// {
//     BackupRetCode ret = m_DirReader->Start();
//     EXPECT_EQ(ret, BackupRetCode::SUCCESS);
// }
