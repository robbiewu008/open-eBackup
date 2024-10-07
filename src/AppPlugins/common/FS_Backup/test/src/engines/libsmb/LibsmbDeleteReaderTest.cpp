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
#include "llt_stub/stub.h"
#include "llt_stub/addr_pri.h"
#include "LibsmbDeleteReader.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
namespace  {
    constexpr uint8_t FILEDESC_IS_DIR = 1;
    constexpr auto RECONNECT_CONTEXT_RETRY_TIMES = 5;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr auto COMPOUND_READ_MAX_SIZE = 4 * 1024 * 1024;
    constexpr auto OPENED_FILEHANDLE_REACH_THRESHOLD = 10000;
    constexpr auto DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
}

class LibsmbDeleteReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    std::unique_ptr<LibsmbDeleteReader> m_libsmbDeleteReader = nullptr;
};

void LibsmbDeleteReaderTest::SetUp()
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    BackupParams params;
    params.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    params.dstEngine = BackupIOEngine::POSIX;
    params.srcAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();
    params.dstAdvParams = std::make_shared<LibsmbBackupAdvanceParams>();

    ReaderParams deleteReaderParams {};
    deleteReaderParams.backupParams = params;
    deleteReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libsmbDeleteReader = std::make_unique<LibsmbDeleteReader>(deleteReaderParams);
}

void LibsmbDeleteReaderTest::TearDown()
{}

void LibsmbDeleteReaderTest::SetUpTestCase()
{}

void LibsmbDeleteReaderTest::TearDownTestCase()
{}

static bool IsComplete_Stub_True(void *obj)
{
    return true;
}

static bool IsComplete_Stub_False(void *obj)
{
    return false;
}

static bool IsAbort_Stub_True(void *obj)
{
    return true;
}

static bool IsAbort_Stub_False(void *obj)
{
    return false;
}

/*
* 用例名称：CheckStart
* 前置条件：
* check点：验证Start返回值
*/
// TEST_F(LibsmbDeleteReaderTest, CheckStart)
// {
//     EXPECT_NO_THROW(m_libsmbDeleteReader->Start());
// }

/*
* 用例名称：CheckAbort
* 前置条件：
* check点：验证Abort返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckAbort)
{
    EXPECT_EQ(m_libsmbDeleteReader->Abort(), BackupRetCode::SUCCESS);
}

/*
* 用例名称：CheckGetStatus
* 前置条件：
* check点：验证GetStatus返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckGetStatus)
{
    m_libsmbDeleteReader->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_libsmbDeleteReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libsmbDeleteReader->m_controlInfo->m_readPhaseComplete = true;
    m_libsmbDeleteReader->m_abort = true;
    EXPECT_EQ(m_libsmbDeleteReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libsmbDeleteReader->m_abort = false;
    m_libsmbDeleteReader->m_controlInfo->m_failed = true;
    m_libsmbDeleteReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbDeleteReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_libsmbDeleteReader->m_controlInfo->m_failed = false;
    m_libsmbDeleteReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbDeleteReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

/*
* 用例名称：CheckIsAbort
* 前置条件：
* check点：验证IsAbort返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckIsAbort)
{
    m_libsmbDeleteReader->m_abort = true;
    m_libsmbDeleteReader->m_controlInfo->m_failed = true;
    m_libsmbDeleteReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_libsmbDeleteReader->IsAbort(), true);

    m_libsmbDeleteReader->m_abort = false;
    m_libsmbDeleteReader->m_controlInfo->m_failed = false;
    m_libsmbDeleteReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libsmbDeleteReader->IsAbort(), false);
}

/*
* 用例名称：CheckIsComplete
* 前置条件：
* check点：验证IsComplete返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckIsComplete)
{
    m_libsmbDeleteReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_libsmbDeleteReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_libsmbDeleteReader->m_controlInfo->m_readConsume = 1;
    EXPECT_EQ(m_libsmbDeleteReader->IsComplete(), true);
}

/*
* 用例名称：CheckThreadFunc
* 前置条件：
* check点：验证ThreadFunc返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckThreadFunc)
{
    Stub stub;
    stub.set(ADDR(LibsmbDeleteReader, IsComplete), IsComplete_Stub_True);
    EXPECT_NO_THROW(m_libsmbDeleteReader->ThreadFunc());
    stub.reset(ADDR(LibsmbDeleteReader, IsComplete));

    stub.set(ADDR(LibsmbDeleteReader, IsComplete), IsComplete_Stub_False);
    stub.set(ADDR(LibsmbDeleteReader, IsAbort), IsAbort_Stub_True);
    EXPECT_NO_THROW(m_libsmbDeleteReader->ThreadFunc());
    stub.reset(ADDR(LibsmbDeleteReader, IsComplete));
    stub.reset(ADDR(LibsmbDeleteReader, IsAbort));
}

/*
* 用例名称：CheckOpenFile
* 前置条件：
* check点：验证OpenFile返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckOpenFile)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteReader->OpenFile(fileHandle), SUCCESS);
}

/*
* 用例名称：CheckReadData
* 前置条件：
* check点：验证ReadData返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckReadData)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteReader->ReadData(fileHandle), SUCCESS);
}

/*
* 用例名称：CheckReadMeta
* 前置条件：
* check点：验证ReadMeta返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckReadMeta)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteReader->ReadMeta(fileHandle), SUCCESS);
}

/*
* 用例名称：CheckCloseFile
* 前置条件：
* check点：验证CloseFile返回值
*/
TEST_F(LibsmbDeleteReaderTest, CheckCloseFile)
{
    FileHandle fileHandle;
    EXPECT_EQ(m_libsmbDeleteReader->CloseFile(fileHandle), SUCCESS);
}

