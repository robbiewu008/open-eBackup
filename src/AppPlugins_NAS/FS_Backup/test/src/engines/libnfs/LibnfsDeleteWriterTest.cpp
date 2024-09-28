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
#include "LibnfsDeleteWriter.h"


using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsDeleteWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    WriterParams deleteWriterParams {};
    std::unique_ptr<LibnfsDeleteWriter> m_libnfsDeleteWriter = nullptr;
};

void LibnfsDeleteWriterTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    deleteWriterParams.backupParams = m_backupParams;
    deleteWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libnfsDeleteWriter = std::make_unique<LibnfsDeleteWriter>(deleteWriterParams);
}

void LibnfsDeleteWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsDeleteWriterTest::SetUpTestCase()
{

}

void LibnfsDeleteWriterTest::TearDownTestCase()
{

}

/*
 * 用例名称:检查Libnfs备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Libnfs备份流程顺利运行
 */

TEST_F(LibnfsDeleteWriterTest, Start)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    deleteWriterParams.backupParams = m_backupParams;
    deleteWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    deleteWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    deleteWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibnfsDeleteWriter libnfsDeleteWriter(deleteWriterParams);
    m_libnfsDeleteWriter->m_abort = true;
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP_VIRTUAL(libnfsDeleteWriter, &LibnfsDeleteWriter::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsDeleteWriter::MonitorWriteTask)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsDeleteWriter->Start(), BackupRetCode::SUCCESS);

    std::unique_ptr<LibnfsDeleteWriter> m_libnfsDeleteWriter1 =
        std::make_unique<LibnfsDeleteWriter>(deleteWriterParams);
    EXPECT_EQ(m_libnfsDeleteWriter1->Start(), BackupRetCode::FAILED);
}

TEST_F(LibnfsDeleteWriterTest, Abort)
{
    BackupRetCode ret = m_libnfsDeleteWriter->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDeleteWriterTest, GetStatus)
{
    m_libnfsDeleteWriter->m_controlInfo->m_writePhaseComplete = false;
    BackupPhaseStatus ret = m_libnfsDeleteWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);

    m_libnfsDeleteWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDeleteWriter->m_abort = true;
    ret = m_libnfsDeleteWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);

    m_libnfsDeleteWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDeleteWriter->m_abort = false;
    m_libnfsDeleteWriter->m_controlInfo->m_failed = false;
    m_libnfsDeleteWriter->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsDeleteWriter->m_failReason = BackupPhaseStatus::FAILED;
    ret = m_libnfsDeleteWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);

    m_libnfsDeleteWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDeleteWriter->m_abort = false;
    m_libnfsDeleteWriter->m_controlInfo->m_failed = false;
    m_libnfsDeleteWriter->m_controlInfo->m_controlReaderFailed = false;
    ret = m_libnfsDeleteWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsDeleteWriterTest, OpenFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteWriter->OpenFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteWriterTest, WriteData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsDeleteWriter->WriteData(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsDeleteWriter->WriteData(fileHandle), FAILED);
}

TEST_F(LibnfsDeleteWriterTest, WriteMeta)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteWriter->WriteMeta(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteWriterTest, CloseFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDeleteWriter->CloseFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDeleteWriterTest, PrintIsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsDeleteWriter->m_writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    bool forcePrint = true;
    EXPECT_NO_THROW(m_libnfsDeleteWriter->PrintIsComplete(forcePrint));
}

TEST_F(LibnfsDeleteWriterTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsDeleteWriter->m_writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsDeleteWriter->m_controlInfo = std::make_shared<BackupControlInfo>();

    MOCKER_CPP(&LibnfsDeleteWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&LibnfsDeleteWriter::IsRetryReqEmpty)
            .stubs()
            .will(returnValue(true));
    m_libnfsDeleteWriter->m_controlInfo->m_aggregateProduce = 5;
    m_libnfsDeleteWriter->m_writerProduce = 5;
    m_libnfsDeleteWriter->m_runningJob = 0;
    m_libnfsDeleteWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    EXPECT_EQ(m_libnfsDeleteWriter->IsComplete(), true);

    m_libnfsDeleteWriter->m_controlInfo->m_aggregatePhaseComplete = false;
    m_libnfsDeleteWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_libnfsDeleteWriter->IsComplete(), false);
}

TEST_F(LibnfsDeleteWriterTest, HandleComplete)
{
    MOCKER_CPP(&LibnfsDeleteWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsDeleteWriter->HandleComplete());
}

TEST_F(LibnfsDeleteWriterTest, IsBlockRecv)
{
    bool ret = m_libnfsDeleteWriter->IsBlockRecv();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDeleteWriterTest, BlockRecv)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->BlockRecv());
}

TEST_F(LibnfsDeleteWriterTest, IsResumeRecv)
{
    bool ret = m_libnfsDeleteWriter->IsResumeRecv();
    EXPECT_EQ(ret, true);
}

TEST_F(LibnfsDeleteWriterTest, ResumeRecv)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ResumeRecv());
}

TEST_F(LibnfsDeleteWriterTest, IsBlockSend)
{
    m_libnfsDeleteWriter->m_runningJob = 40;
    bool ret = m_libnfsDeleteWriter->IsBlockSend();
    EXPECT_EQ(ret, true);

    m_libnfsDeleteWriter->m_runningJob = 10;
    ret = m_libnfsDeleteWriter->IsBlockSend();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDeleteWriterTest, BlockSend)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->BlockSend());
}

TEST_F(LibnfsDeleteWriterTest, IsResumeSend)
{
    m_libnfsDeleteWriter->m_runningJob = 10;
    bool ret = m_libnfsDeleteWriter->IsResumeSend();
    EXPECT_EQ(ret, true);

    m_libnfsDeleteWriter->m_runningJob = 40;
    ret = m_libnfsDeleteWriter->IsResumeSend();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDeleteWriterTest, ResumeSend)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ResumeSend());
}

TEST_F(LibnfsDeleteWriterTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ThreadFunc());

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsDeleteWriter::IsComplete)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ThreadFunc());
}

TEST_F(LibnfsDeleteWriterTest, MonitorWriteTask)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsDeleteWriter->MonitorWriteTask());

    MOCKER_CPP(&LibnfsDeleteWriter::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsDeleteWriter->MonitorWriteTask());

    m_libnfsDeleteWriter->m_jsPtr = nullptr;
    EXPECT_NO_THROW(m_libnfsDeleteWriter->MonitorWriteTask());
}

TEST_F(LibnfsDeleteWriterTest, ProcRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ProcRetryTimers());
}

TEST_F(LibnfsDeleteWriterTest, ExpireRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->ExpireRetryTimers());
}

TEST_F(LibnfsDeleteWriterTest, GetRetryTimerCnt)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->GetRetryTimerCnt());
}

TEST_F(LibnfsDeleteWriterTest, IsRetryReqEmpty)
{
    EXPECT_NO_THROW(m_libnfsDeleteWriter->IsRetryReqEmpty());
}
