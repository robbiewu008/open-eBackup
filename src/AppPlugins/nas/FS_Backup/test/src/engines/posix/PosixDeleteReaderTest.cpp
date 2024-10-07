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
#include "PosixDeleteReader.h"
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

class PosixDeleteReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<PosixDeleteReader> m_posixDeleteReader = nullptr;
    Stub stub;
};

void PosixDeleteReaderTest::SetUp()
{
    BackupQueueConfig config { DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE };
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<HostBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<HostBackupAdvanceParams>();

    ReaderParams deleteReaderParams {};
    deleteReaderParams.backupParams = params;
    deleteReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_posixDeleteReader = std::make_unique<PosixDeleteReader>(deleteReaderParams);
}

void PosixDeleteReaderTest::TearDown() {}

void PosixDeleteReaderTest::SetUpTestCase() {}

void PosixDeleteReaderTest::TearDownTestCase() {}

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
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete失败
 */
TEST_F(PosixDeleteReaderTest, IsComplete_Fail)
{
    EXPECT_EQ(m_posixDeleteReader->IsComplete(), false);
}

/*
 * 用例名称：IsComplete
 * 前置条件：
 * check点：IsComplete成功
 */
TEST_F(PosixDeleteReaderTest, IsComplete_True)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_controlReaderPhaseComplete = true;
    m_posixDeleteReader->m_readQueue->Clear();
    controlInfo->m_controlFileReaderProduce = 0;
    controlInfo->m_readConsume = 0;
    m_posixDeleteReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteReader->IsComplete(), true);
}


/*
 * 用例名称：Abort
 * 前置条件：无
 * check点：IsAbort成功
 */
TEST_F(PosixDeleteReaderTest, IsAbort)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = false;
    m_posixDeleteReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    m_posixDeleteReader->m_controlInfo = controlInfo;

    EXPECT_EQ(m_posixDeleteReader->IsAbort(), true);
}

/*
 * 用例名称：IsAbort_Fail
 * 前置条件：无
 * check点：IsAbort失败
 */
TEST_F(PosixDeleteReaderTest, IsAbort_Fail)
{
    EXPECT_EQ(m_posixDeleteReader->IsAbort(), false);
}

/*
 * 用例名称：GetStatus_NotNeedPhaseCpl
 * 前置条件：无
 * check点：校验GetStatus无需解析完成
 */
TEST_F(PosixDeleteReaderTest, GetStatus_NotNeedPhaseCpl)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = false;
    m_posixDeleteReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteReader->GetStatus(), BackupPhaseStatus::INPROGRESS);
}

/*
 * 用例名称：GetStatus_Abort
 * 前置条件：无
 * check点：校验GetStatus被终止
 */
TEST_F(PosixDeleteReaderTest, GetStatus_Abort)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixDeleteReader->m_abort = true;
    m_posixDeleteReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteReader->GetStatus(), BackupPhaseStatus::ABORTED);
}

/*
 * 用例名称：GetStatus_Fail
 * 前置条件：无
 * check点：校验GetStatus失败
 */
TEST_F(PosixDeleteReaderTest, GetStatus_Fail)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixDeleteReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = true;
    m_posixDeleteReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteReader->GetStatus(), BackupPhaseStatus::FAILED);
}

/*
 * 用例名称：GetStatus_Suc
 * 前置条件：无
 * check点：校验GetStatus成功
 */
TEST_F(PosixDeleteReaderTest, GetStatus_Suc)
{
    std::shared_ptr<BackupControlInfo> controlInfo = make_shared<BackupControlInfo>();
    controlInfo->m_readPhaseComplete = true;
    m_posixDeleteReader->m_abort = false;
    controlInfo->m_failed = false;
    controlInfo->m_controlReaderFailed = false;
    m_posixDeleteReader->m_controlInfo = controlInfo;
    EXPECT_EQ(m_posixDeleteReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}


/*
 * 用例名称：ThreadFunc
 * 前置条件：无
 * check点：校验ThreadFunc
 */
TEST_F(PosixDeleteReaderTest, ThreadFunc)
{
    stub.set(ADDR(PosixDeleteReader, IsComplete), IsComplete_Stub_Suc);
    EXPECT_NO_THROW(m_posixDeleteReader->ThreadFunc());
    stub.reset(ADDR(PosixDeleteReader, IsComplete));

    stub.set(ADDR(PosixDeleteReader, IsComplete), IsComplete_Stub_Fail);
    stub.set(ADDR(PosixDeleteReader, IsAbort), IsAbort_Stub_Suc);
    EXPECT_NO_THROW(m_posixDeleteReader->ThreadFunc());
    stub.reset(ADDR(PosixDeleteReader, IsAbort));
    stub.reset(ADDR(PosixDeleteReader, IsComplete));
}

/*
 * 用例名称：Destroy
 * 前置条件：ThreadFunc校验成功
 * check点：校验Destroy
 */
TEST_F(PosixDeleteReaderTest, Destroy)
{
    m_posixDeleteReader->m_threadDone = false;
    EXPECT_EQ(m_posixDeleteReader->Destroy(), BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS);

    stub.set(ADDR(PosixDeleteReader, IsComplete), IsComplete_Stub_Suc);
    m_posixDeleteReader->ThreadFunc();
    stub.reset(ADDR(PosixDeleteReader, IsComplete));
    EXPECT_EQ(m_posixDeleteReader->Destroy(), BackupRetCode::SUCCESS);
    m_posixDeleteReader->m_threadDone = false;

    stub.set(ADDR(PosixDeleteReader, IsComplete), IsComplete_Stub_Fail);
    stub.set(ADDR(PosixDeleteReader, IsAbort), IsAbort_Stub_Suc);
    m_posixDeleteReader->ThreadFunc();
    stub.reset(ADDR(PosixDeleteReader, IsAbort));
    stub.reset(ADDR(PosixDeleteReader, IsComplete));
    EXPECT_EQ(m_posixDeleteReader->Destroy(), BackupRetCode::SUCCESS);
    m_posixDeleteReader->m_threadDone = false;
}

/*
 * 用例名称：OpenFile_Suc
 * 前置条件：无
 * check点：校验OpenFile成功
 */
TEST_F(PosixDeleteReaderTest, OpenFile_Suc)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_EQ(m_posixDeleteReader->OpenFile(fileHandle), 0);
}

/*
 * 用例名称：ReadData
 * 前置条件：无
 * check点：校验ReadData
 */
TEST_F(PosixDeleteReaderTest, ReadData)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_EQ(m_posixDeleteReader->ReadData(fileHandle), 0);
}

/*
 * 用例名称：ReadMeta
 * 前置条件：无
 * check点：校验ReadMeta
 */
TEST_F(PosixDeleteReaderTest, ReadMeta)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_EQ(m_posixDeleteReader->ReadMeta(fileHandle), 0);
}

/*
 * 用例名称：CloseFile
 * 前置条件：无
 * check点：校验CloseFile
 */
TEST_F(PosixDeleteReaderTest, CloseFile)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test.txt";
    EXPECT_EQ(m_posixDeleteReader->CloseFile(fileHandle), 0);
}
