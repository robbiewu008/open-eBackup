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
#include "LibnfsDirMetaWriter.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsDirMetaWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    WriterParams dirWriterParams {};
    std::unique_ptr<LibnfsDirMetaWriter> m_libnfsDirMetaWriter = nullptr;
};

void LibnfsDirMetaWriterTest::SetUp()
{
    ThreadPoolFactory::InitThreadPool(32, 32);
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    WriterParams dirWriterParams {};
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    dirWriterParams.backupParams = m_backupParams;
    dirWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libnfsDirMetaWriter = std::make_unique<LibnfsDirMetaWriter>(dirWriterParams);
}

void LibnfsDirMetaWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsDirMetaWriterTest::SetUpTestCase()
{

}

void LibnfsDirMetaWriterTest::TearDownTestCase()
{

}

static ThreadPool* GetThreadPoolInstance_Stub(ThreadPoolType type)
{
    int nThreads = 32;
    ThreadPool *tempThreadPool = new(nothrow) ThreadPool(nThreads);
    return tempThreadPool;
}
/*
 * 用例名称:检查Libnfs备份流程
 * 前置条件：生成扫描结果文件controlFile 和 metaFile
 * check点：Libnfs备份流程顺利运行
 */

TEST_F(LibnfsDirMetaWriterTest, Start)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    dirWriterParams.backupParams = m_backupParams;
    dirWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    dirWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    dirWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    LibnfsDirMetaWriter libnfsDirMetaWriter(dirWriterParams);
    m_libnfsDirMetaWriter->m_abort = true;
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP_VIRTUAL(libnfsDirMetaWriter, &LibnfsDirMetaWriter::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsDirMetaWriter::MonitorWriteTask)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsDirMetaWriter->Start(), BackupRetCode::SUCCESS);

    std::unique_ptr<LibnfsDirMetaWriter> m_libnfsDirMetaWriter1 =
        std::make_unique<LibnfsDirMetaWriter>(dirWriterParams);
    EXPECT_EQ(m_libnfsDirMetaWriter1->Start(), BackupRetCode::FAILED);
}

TEST_F(LibnfsDirMetaWriterTest, Abort)
{
    BackupRetCode ret = m_libnfsDirMetaWriter->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsDirMetaWriterTest, GetStatus)
{
    m_libnfsDirMetaWriter->m_controlInfo->m_writePhaseComplete = false;
    BackupPhaseStatus ret = m_libnfsDirMetaWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::INPROGRESS);

    m_libnfsDirMetaWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDirMetaWriter->m_abort = true;
    ret = m_libnfsDirMetaWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::ABORTED);

    m_libnfsDirMetaWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDirMetaWriter->m_abort = false;
    m_libnfsDirMetaWriter->m_controlInfo->m_failed = false;
    m_libnfsDirMetaWriter->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsDirMetaWriter->m_failReason = BackupPhaseStatus::FAILED;
    ret = m_libnfsDirMetaWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);

    m_libnfsDirMetaWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsDirMetaWriter->m_abort = false;
    m_libnfsDirMetaWriter->m_controlInfo->m_failed = false;
    m_libnfsDirMetaWriter->m_controlInfo->m_controlReaderFailed = false;
    ret = m_libnfsDirMetaWriter->GetStatus();
    EXPECT_EQ(ret, BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsDirMetaWriterTest, OpenFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaWriter->OpenFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaWriterTest, WriteMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    MOCKER_CPP(&Module::JobScheduler::Put)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&NfsContextContainer::IncSendCnt)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsDirMetaWriter->WriteMeta(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsDirMetaWriter->WriteMeta(fileHandle), MP_FAILED);
}

TEST_F(LibnfsDirMetaWriterTest, WriteData)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaWriter->WriteData(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaWriterTest, CloseFile)
{
    FileHandle fileHandle {};
    int ret = m_libnfsDirMetaWriter->CloseFile(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsDirMetaWriterTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsDirMetaWriter->m_writeQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsDirMetaWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsDirMetaWriter::IsRetryReqEmpty)
            .stubs()
            .will(returnValue(true));
    m_libnfsDirMetaWriter->m_controlInfo->m_aggregateProduce = 5;
    m_libnfsDirMetaWriter->m_writerProduce = 5;
    m_libnfsDirMetaWriter->m_runningJob = 0;
    m_libnfsDirMetaWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_libnfsDirMetaWriter->m_controlInfo->m_writerConsume = 5;
    EXPECT_EQ(m_libnfsDirMetaWriter->IsComplete(), true);

    m_libnfsDirMetaWriter->m_controlInfo->m_aggregatePhaseComplete = false;
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->IsComplete());
}

TEST_F(LibnfsDirMetaWriterTest, HandleComplete)
{
    MOCKER_CPP(&LibnfsDirMetaWriter::ExpireRetryTimers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsDirMetaWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->HandleComplete());
}

TEST_F(LibnfsDirMetaWriterTest, IsBlockSend)
{
    m_libnfsDirMetaWriter->m_runningJob = 40;
    bool ret = m_libnfsDirMetaWriter->IsBlockSend();
    EXPECT_EQ(ret, true);

    m_libnfsDirMetaWriter->m_runningJob = 10;
    ret = m_libnfsDirMetaWriter->IsBlockSend();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDirMetaWriterTest, BlockSend)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->BlockSend());
}

TEST_F(LibnfsDirMetaWriterTest, IsResumeSend)
{
    m_libnfsDirMetaWriter->m_runningJob = 10;
    bool ret = m_libnfsDirMetaWriter->IsResumeSend();
    EXPECT_EQ(ret, true);

    m_libnfsDirMetaWriter->m_runningJob = 40;
    ret = m_libnfsDirMetaWriter->IsResumeSend();
    EXPECT_EQ(ret, false);
}

TEST_F(LibnfsDirMetaWriterTest, ResumeSend)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->ResumeSend());
}

TEST_F(LibnfsDirMetaWriterTest, HandleQueueBlock)
{
    m_libnfsDirMetaWriter->m_runningJob = 10;
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->HandleQueueBlock());
}

TEST_F(LibnfsDirMetaWriterTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    m_libnfsDirMetaWriter->ThreadFunc();

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsDirMetaWriter::IsComplete)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->ThreadFunc());
}

TEST_F(LibnfsDirMetaWriterTest, PushToWriter)
{
    FileHandle fileHandle {};
    uint16_t count;
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->PushToWriter(fileHandle, count));
}

TEST_F(LibnfsDirMetaWriterTest, MonitorWriteTask)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->MonitorWriteTask());

    MOCKER_CPP(&LibnfsDirMetaWriter::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->MonitorWriteTask());

    m_libnfsDirMetaWriter->m_jsPtr = nullptr;
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->MonitorWriteTask());
}

TEST_F(LibnfsDirMetaWriterTest, ProcRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->ProcRetryTimers());
}

TEST_F(LibnfsDirMetaWriterTest, ExpireRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->ExpireRetryTimers());
}

TEST_F(LibnfsDirMetaWriterTest, GetRetryTimerCnt)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->GetRetryTimerCnt());
}

TEST_F(LibnfsDirMetaWriterTest, IsRetryReqEmpty)
{
    EXPECT_NO_THROW(m_libnfsDirMetaWriter->IsRetryReqEmpty());
}
