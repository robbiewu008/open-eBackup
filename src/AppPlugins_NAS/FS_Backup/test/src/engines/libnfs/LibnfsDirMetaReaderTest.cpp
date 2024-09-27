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
#include "LibnfsDirMetaReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsDirMetaReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams dirReaderParams {};
    std::unique_ptr<LibnfsDirMetaReader> m_libnfsDirMetaReader = nullptr;
};

void LibnfsDirMetaReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    dirReaderParams.backupParams = m_backupParams;
    dirReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libnfsDirMetaReader = std::make_unique<LibnfsDirMetaReader>(dirReaderParams);
}

void LibnfsDirMetaReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsDirMetaReaderTest::SetUpTestCase()
{

}

void LibnfsDirMetaReaderTest::TearDownTestCase()
{

}
/*
 * 用例名称:检查Libnfs备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Libnfs备份流程顺利运行
 */

TEST_F(LibnfsDirMetaReaderTest, Start)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    dirReaderParams.backupParams = m_backupParams;
    dirReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibnfsDirMetaReader libnfsDirMetaReader(dirReaderParams);
    m_libnfsDirMetaReader->m_abort = true;
    MOCKER_CPP_VIRTUAL(libnfsDirMetaReader, &LibnfsDirMetaReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    BackupRetCode ret = m_libnfsDirMetaReader->Start();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDirMetaReaderTest, Abort)
{
    BackupRetCode ret = m_libnfsDirMetaReader->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDirMetaReaderTest, GetStatus)
{
    m_libnfsDirMetaReader->m_controlInfo->m_readPhaseComplete = false;
    BackupPhaseStatus ret = m_libnfsDirMetaReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);

    m_libnfsDirMetaReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDirMetaReader->m_abort = true;
    ret = m_libnfsDirMetaReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);

    m_libnfsDirMetaReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDirMetaReader->m_abort = false;
    m_libnfsDirMetaReader->m_controlInfo->m_failed = false;
    m_libnfsDirMetaReader->m_controlInfo->m_controlReaderFailed = true;
    ret = m_libnfsDirMetaReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);

    m_libnfsDirMetaReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsDirMetaReader->m_abort = false;
    m_libnfsDirMetaReader->m_controlInfo->m_failed = false;
    m_libnfsDirMetaReader->m_controlInfo->m_controlReaderFailed = false;
    ret = m_libnfsDirMetaReader->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsDirMetaReaderTest, IsComplete)
{
    m_libnfsDirMetaReader->m_controlInfo->m_controlFileReaderProduce = 5;
    m_libnfsDirMetaReader->m_controlInfo->m_readConsume = 5;
    m_libnfsDirMetaReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    bool ret = m_libnfsDirMetaReader->IsComplete();
    EXPECT_EQ(ret, true);

    m_libnfsDirMetaReader->m_controlInfo->m_controlFileReaderProduce = 5;
    m_libnfsDirMetaReader->m_controlInfo->m_readConsume = 4;
    m_libnfsDirMetaReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    ret = m_libnfsDirMetaReader->IsComplete();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDirMetaReaderTest, HandleComplete)
{
    EXPECT_NO_THROW(m_libnfsDirMetaReader->HandleComplete());
}

TEST_F(LibnfsDirMetaReaderTest, ThreadFunc)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsDirMetaReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    //m_libnfsDirMetaReader->m_readQueue->WaitAndPush(fileHandle);
    //m_libnfsDirMetaReader->m_backupParams.commonParams.writeDisable = false;

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&LibnfsDirMetaReader::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsDirMetaReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsDirMetaReader->ThreadFunc();

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsDirMetaReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsDirMetaReader->ThreadFunc());
}

TEST_F(LibnfsDirMetaReaderTest, OpenFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaReader->OpenFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaReaderTest, ReadData)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaReader->ReadData(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaReaderTest, ReadMeta)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaReader->ReadMeta(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaReaderTest, CloseFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaReader->CloseFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}