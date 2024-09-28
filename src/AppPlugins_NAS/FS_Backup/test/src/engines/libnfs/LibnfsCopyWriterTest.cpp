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
#include "LibnfsCopyWriter.h"
#include "log/Log.h"
#include "BackupQueue.h"
#include "Backup.h"
#include "BackupMgr.h"
#include "PacketStats.h"
#include "libnfs_ctx/NfsContextWrapper.h"
#include "LibnfsInterface.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsCopyWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    WriterParams copyWriterParams {};
    void FillWriterParams(WriterParams &copyWriterParams);
    std::unique_ptr<LibnfsCopyWriter> m_libnfsCopyWriter = nullptr;
};

void LibnfsCopyWriterTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    FillWriterParams(copyWriterParams);
    m_libnfsCopyWriter = std::make_unique<LibnfsCopyWriter>(copyWriterParams);
}

void LibnfsCopyWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsCopyWriterTest::SetUpTestCase()
{}

void LibnfsCopyWriterTest::TearDownTestCase()
{}

void LibnfsCopyWriterTest::FillWriterParams(WriterParams &copyWriterParams)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    copyWriterParams.backupParams = m_backupParams;
    copyWriterParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();
}

static nfsfh* Get_Stub()
{
    struct nfsfh *nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    return nfsFh;
}

static uint64_t GetValue_Stub1()
{
    uint64_t val = 0;
    return val;
}

static uint64_t GetValue_Stub2()
{
    uint64_t val = 1;
    return val;
}

TEST_F(LibnfsCopyWriterTest, Start)
{
    m_libnfsCopyWriter->m_abort = true;
    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP(&LibnfsCopyWriter::FillWriteContainers)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::StartMkdirThread)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->Start(), BackupRetCode::SUCCESS);

    MOCKER_CPP(&LibnfsCopyWriter::DeleteWriteContainers)
            .stubs()
            .will(ignoreReturnValue());
    std::unique_ptr<LibnfsCopyWriter> m_libnfsCopyWriter1 =
        std::make_unique<LibnfsCopyWriter>(copyWriterParams);
    EXPECT_EQ(m_libnfsCopyWriter1->Start(), BackupRetCode::FAILED);
}

TEST_F(LibnfsCopyWriterTest, StartMkdirThread)
{
    m_libnfsCopyWriter->m_abort = true;
    MOCKER_CPP(&LibnfsCopyWriter::MkdirThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_libnfsCopyWriter->StartMkdirThread(), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, Abort)
{
    MOCKER_CPP(&LibnfsCopyWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsCopyWriter->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, GetStatus)
{
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());

    m_libnfsCopyWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_libnfsCopyWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libnfsCopyWriter->m_abort = true;
    m_libnfsCopyWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_EQ(m_libnfsCopyWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libnfsCopyWriter->m_abort = false;
    m_libnfsCopyWriter->m_controlInfo->m_failed = false;
    m_libnfsCopyWriter->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsCopyWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsCopyWriter->m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
    EXPECT_EQ(m_libnfsCopyWriter->GetStatus(), BackupPhaseStatus::FAILED_NOACCESS);

    m_libnfsCopyWriter->m_abort = false;
    m_libnfsCopyWriter->m_controlInfo->m_failed = false;
    m_libnfsCopyWriter->m_controlInfo->m_controlReaderFailed = false;
    m_libnfsCopyWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_EQ(m_libnfsCopyWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsCopyWriterTest, ProcRetryTimers)
{
    m_libnfsCopyWriter->ProcRetryTimers();
}

TEST_F(LibnfsCopyWriterTest, ExpireRetryTimers)
{
    Libnfscommonmethods::ExpireRetryTimers(m_libnfsCopyWriter->m_timer);
}

TEST_F(LibnfsCopyWriterTest, GetRetryTimerCnt)
{
    m_libnfsCopyWriter->GetRetryTimerCnt();
}

TEST_F(LibnfsCopyWriterTest, IsRetryReqEmpty)
{
    m_libnfsCopyWriter->IsRetryReqEmpty();
}

TEST_F(LibnfsCopyWriterTest, OpenFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_dirName = "d2";

    MOCKER_CPP(&FileHandleCache::Get)
            .stubs()
            .will(invoke(Get_Stub));
    MOCKER_CPP(&Libnfscommonmethods::ProcessParentFh)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP(&SendCreateRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->OpenFile(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsCopyWriter->OpenFile(fileHandle), MP_FAILED);
}

TEST_F(LibnfsCopyWriterTest, WriteData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_size = 0;

    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->WriteData(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_size = 5;
    fileHandle.m_retryCnt = 0;
    MOCKER_CPP(&SendWriteRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->WriteData(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, WriteMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    m_libnfsCopyWriter->m_backupParams.commonParams.writeMeta = true;
    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP(&SendSetMetaRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->WriteMeta(fileHandle), MP_SUCCESS);

    m_libnfsCopyWriter->m_backupParams.commonParams.writeMeta = false;
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->WriteMeta(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, CloseFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "1.txt";
    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;

    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsCopyWriter->CloseFile(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsCopyWriter->CloseFile(fileHandle), MP_FAILED);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendDstCloseRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->CloseFile(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, PrintIsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->PrintIsComplete(true);
    m_libnfsCopyWriter->PrintIsComplete(false);
}

TEST_F(LibnfsCopyWriterTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_libnfsCopyWriter->m_mkdirComplete = true;

    MOCKER_CPP(&LibnfsCopyWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::IsRetryReqEmpty)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsCopyWriter->IsComplete(), true);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(invoke(GetValue_Stub2));
    EXPECT_EQ(m_libnfsCopyWriter->IsComplete(), false);
}

TEST_F(LibnfsCopyWriterTest, CanRecv)
{
    MOCKER_CPP(&LibnfsCopyWriter::IsBlockRecv)
            .stubs()
            .will(returnValue(true));
    EXPECT_EQ(m_libnfsCopyWriter->CanRecv(&m_libnfsCopyWriter), false);
}

TEST_F(LibnfsCopyWriterTest, HandleComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsCopyWriter::DeleteWriteContainers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Libnfscommonmethods::ExpireRetryTimers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileHandleCache::Clear)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->HandleComplete();
}

TEST_F(LibnfsCopyWriterTest, IsBlockRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_mkdirSyncQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->IsBlockRecv();

    auto constexpr MAX_BACKUP_QUEUE_SIZE = 0;
    m_libnfsCopyWriter->IsBlockRecv();
}

TEST_F(LibnfsCopyWriterTest, BlockRecv)
{
    m_libnfsCopyWriter->BlockRecv();
}

TEST_F(LibnfsCopyWriterTest, IsResumeRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_mkdirSyncQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->IsResumeRecv();
}

TEST_F(LibnfsCopyWriterTest, ResumeRecv)
{
    m_libnfsCopyWriter->ResumeRecv();
}

TEST_F(LibnfsCopyWriterTest, IsBlockSend)
{
    m_libnfsCopyWriter->m_advParams->maxPendingAsyncReqCnt = 10;
    m_libnfsCopyWriter->m_advParams->serverCheckMaxCount = 10;
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(10))
            .then(returnValue(8));
    MOCKER_CPP(&LibnfsCopyWriter::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub2))
            .then(invoke(GetValue_Stub1));
    EXPECT_EQ(m_libnfsCopyWriter->IsBlockSend(), true);
    EXPECT_EQ(m_libnfsCopyWriter->IsBlockSend(), false);
}

TEST_F(LibnfsCopyWriterTest, BlockSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->BlockSend();
}

TEST_F(LibnfsCopyWriterTest, IsResumeSend)
{
    m_libnfsCopyWriter->m_advParams->minPendingAsyncReqCnt = 10;
    m_libnfsCopyWriter->m_advParams->serverCheckMaxCount = 10;
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(9));
    MOCKER_CPP(&LibnfsCopyWriter::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub1));
    m_libnfsCopyWriter->IsResumeSend();
}

TEST_F(LibnfsCopyWriterTest, ResumeSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->ResumeSend();
}

TEST_F(LibnfsCopyWriterTest, IsResumeSendCb)
{
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    EXPECT_EQ(m_libnfsCopyWriter->IsResumeSendCb(&m_libnfsCopyWriter), true);
}

TEST_F(LibnfsCopyWriterTest, ResumeSendCb)
{
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->ResumeSendCb(&m_libnfsCopyWriter);
}

TEST_F(LibnfsCopyWriterTest, HandleQueueBlock)
{
    MOCKER_CPP(&LibnfsCopyWriter::IsBlockRecv)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyWriter::IsBlockSend)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyWriter::BlockRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::BlockSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->HandleQueueBlock();

    MOCKER_CPP(&LibnfsCopyWriter::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->HandleQueueBlock();
}

TEST_F(LibnfsCopyWriterTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyWriter::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->ThreadFunc();

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsCopyWriter::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->ThreadFunc();
}

TEST_F(LibnfsCopyWriterTest, ProcessWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    m_libnfsCopyWriter->ProcessWriteQueue();

    MOCKER_CPP(&LibnfsCopyWriter::IsBlockSend)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&LibnfsCopyWriter::ProcessWriteEntries)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&LibnfsCopyWriter::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyWriter->ProcessWriteQueue();
}

TEST_F(LibnfsCopyWriterTest, ProcessWriteEntries)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 16832;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_mkdirSyncQueue = make_shared<BackupQueue<FileHandle>>(config);
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_META_MODIFIED;
    MOCKER_CPP(&LibnfsCopyWriter::ProcessMetaModifiedFiles)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_BOTH_MODIFIED;
    m_libnfsCopyWriter->m_backupParams.backupType = BackupType::RESTORE;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs()
            .will(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::LINK_DEL_FAILED))
            .then(returnValue(FileDescState::LINK_DEL_FOR_RESTORE))
            .then(returnValue(FileDescState::DIR_DEL_RESTORE))
            .then(returnValue(FileDescState::DST_CLOSED));
    MOCKER_CPP(&LstatFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    m_libnfsCopyWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    MOCKER_CPP(&LinkDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    m_libnfsCopyWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_BOTH_MODIFIED;
    MOCKER_CPP(&LinkDeleteForRestore)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&DirectoryDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&LibnfsCopyWriter::ProcessFilesCreateWriteClose)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, ProcessMetaModifiedFiles)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_block.m_size = 5;
    FileDescState state {};
    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    EXPECT_EQ(m_libnfsCopyWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    m_libnfsCopyWriter->m_backupParams.backupType = BackupType::RESTORE;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    state = FileDescState::INIT;
    MOCKER_CPP(&LstatFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    m_libnfsCopyWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    fileHandle.m_file->m_mode = 41471;
    MOCKER_CPP(&WriteSymLinkMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, ProcessFilesCreateWriteClose)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    FileDescState state = FileDescState::READ_FAILED;
    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::READ_FAILED))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::WRITE_FAILED))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::CloseFile)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    fileHandle.m_file->m_blockStats.m_writeRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 1;
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 41471;
    state = FileDescState::INIT;
    MOCKER_CPP(&LibnfsCopyWriter::ProcessSymlink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    state = FileDescState::WRITE_SKIP;
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::LSTAT;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&LibnfsCopyWriter::ProcessFileOpen)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_block.m_size = 5;
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::PARTIAL_WRITED;
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::WriteData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::WRITED;
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::META_WRITED;
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, ProcessSymlink)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    FileDescState state = FileDescState::LSTAT;

    MOCKER_CPP(&CreateSymlink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessSymlink(fileHandle, state), MP_SUCCESS);

    MOCKER_CPP(&WriteSymLinkMeta)
            .stubs()
            .will(returnValue(0));
    state = FileDescState::LINK_DEL_FAILED;
    EXPECT_EQ(m_libnfsCopyWriter->ProcessSymlink(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, ProcessFileOpen)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 4480;
    FillWriterParams(copyWriterParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP(&CreateSpecialFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFileOpen(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::OpenFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyWriter->ProcessFileOpen(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, BatchPush)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyWriter->m_writeWaitQueue->Push(fileHandle);
    m_libnfsCopyWriter->BatchPush();
}

TEST_F(LibnfsCopyWriterTest, MkdirThreadFunc)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyWriter->m_mkdirSyncQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    m_libnfsCopyWriter->MkdirThreadFunc();

    m_libnfsCopyWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_libnfsCopyWriter->MkdirThreadFunc();

}
TEST_F(LibnfsCopyWriterTest, FillWriteContainers)
{
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsCopyWriter->FillWriteContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsCopyWriter->FillWriteContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsCopyWriter->FillWriteContainers(), MP_SUCCESS);
}

TEST_F(LibnfsCopyWriterTest, DeleteWriteContainers)
{
    m_libnfsCopyWriter->DeleteWriteContainers();
}
