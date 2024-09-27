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
#include "mockcpp/mockcpp.hpp"
#include "LibnfsDeleteReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsDeleteReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams deleteReaderParams {};
    std::unique_ptr<LibnfsDeleteReader> m_libnfsDeleteReader = nullptr;
};

void LibnfsDeleteReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    deleteReaderParams.backupParams = m_backupParams;
    deleteReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libnfsDeleteReader = std::make_unique<LibnfsDeleteReader>(deleteReaderParams);
}

void LibnfsDeleteReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsDeleteReaderTest::SetUpTestCase()
{

}

void LibnfsDeleteReaderTest::TearDownTestCase()
{

}

/*
 * 用例名称:检查Libnfs备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Libnfs备份流程顺利运行
 */

TEST_F(LibnfsDeleteReaderTest, Start)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    deleteReaderParams.backupParams = m_backupParams;
    deleteReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibnfsDeleteReader libnfsDeleteReader(deleteReaderParams);
    m_libnfsDeleteReader->m_abort = true;
    MOCKER_CPP_VIRTUAL(libnfsDeleteReader, &LibnfsDeleteReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    BackupRetCode ret = m_libnfsDeleteReader->Start();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDeleteReaderTest, Abort)
{
    BackupRetCode ret = m_libnfsDeleteReader->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDeleteReaderTest, GetStatus)
{
    m_libnfsDeleteReader->m_controlInfo->m_readPhaseComplete = false;
    BackupPhaseStatus ret = m_libnfsDeleteReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);

    m_libnfsDeleteReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDeleteReader->m_abort = true;
    ret = m_libnfsDeleteReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);

    m_libnfsDeleteReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDeleteReader->m_abort = false;
    m_libnfsDeleteReader->m_controlInfo->m_failed = false;
    m_libnfsDeleteReader->m_controlInfo->m_controlReaderFailed = true;
    ret = m_libnfsDeleteReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);

    m_libnfsDeleteReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDeleteReader->m_abort = false;
    m_libnfsDeleteReader->m_controlInfo->m_failed = false;
    m_libnfsDeleteReader->m_controlInfo->m_controlReaderFailed = false;
    ret = m_libnfsDeleteReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsDeleteReaderTest, IsComplete)
{
    m_libnfsDeleteReader->m_controlInfo->m_controlFileReaderProduce = 5;
    m_libnfsDeleteReader->m_controlInfo->m_readConsume = 5;
    m_libnfsDeleteReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    bool ret = m_libnfsDeleteReader->IsComplete();
    EXPECT_EQ(ret, true);

    m_libnfsDeleteReader->m_controlInfo->m_controlFileReaderProduce = 5;
    m_libnfsDeleteReader->m_controlInfo->m_readConsume = 4;
    m_libnfsDeleteReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    ret = m_libnfsDeleteReader->IsComplete();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDeleteReaderTest, HandleComplete)
{
    EXPECT_NO_THROW(m_libnfsDeleteReader->HandleComplete());
}

TEST_F(LibnfsDeleteReaderTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsDeleteReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    //m_libnfsDeleteReader->m_readQueue->WaitAndPush(fileHandle);
    //m_libnfsDeleteReader->m_backupParams.commonParams.writeDisable = false;

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&LibnfsDeleteReader::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsDeleteReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsDeleteReader->ThreadFunc());

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsDeleteReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsDeleteReader->ThreadFunc());
}

TEST_F(LibnfsDeleteReaderTest, OpenFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteReader->OpenFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteReaderTest, ReadData)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteReader->ReadData(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteReaderTest, ReadMeta)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteReader->ReadMeta(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteReaderTest, CloseFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteReader->CloseFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}