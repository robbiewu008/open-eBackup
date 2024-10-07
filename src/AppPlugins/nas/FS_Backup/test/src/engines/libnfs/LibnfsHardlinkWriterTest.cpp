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
#include "LibnfsHardlinkWriter.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsHardlinkWriterTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    WriterParams hardlinkWriterParams {};
    void FillWriterParams(WriterParams &hardlinkWriterParams);
    std::unique_ptr<LibnfsHardlinkWriter> m_libnfsHardlinkWriter = nullptr;
};

void LibnfsHardlinkWriterTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    FillWriterParams(hardlinkWriterParams);
    m_libnfsHardlinkWriter = std::make_unique<LibnfsHardlinkWriter>(hardlinkWriterParams);
}

void LibnfsHardlinkWriterTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsHardlinkWriterTest::SetUpTestCase()
{}

void LibnfsHardlinkWriterTest::TearDownTestCase()
{}

void LibnfsHardlinkWriterTest::FillWriterParams(WriterParams &hardlinkWriterParams)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    hardlinkWriterParams.backupParams = m_backupParams;
    hardlinkWriterParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkWriterParams.writeQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkWriterParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkWriterParams.blockBufferMap = std::make_shared<BlockBufferMap>();
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

TEST_F(LibnfsHardlinkWriterTest, Start)
{
    m_libnfsHardlinkWriter->m_abort = true;
    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP(&LibnfsHardlinkWriter::FillWriteContainers)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::StartMkdirThread)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->Start(), BackupRetCode::SUCCESS);

    MOCKER_CPP(&LibnfsHardlinkWriter::DeleteWriteContainers)
            .stubs()
            .will(ignoreReturnValue());
    std::unique_ptr<LibnfsHardlinkWriter> m_libnfsHardlinkWriter1 =
        std::make_unique<LibnfsHardlinkWriter>(hardlinkWriterParams);
    EXPECT_EQ(m_libnfsHardlinkWriter1->Start(), BackupRetCode::FAILED);
}

// TEST_F(LibnfsHardlinkWriterTest, StartMkdirThread)
// {
//     m_libnfsHardlinkWriter->m_abort = true;
//     MOCKER_CPP(&LibnfsHardlinkWriter::MkdirThreadFunc)
//             .stubs()
//             .will(ignoreReturnValue());
//     EXPECT_EQ(m_libnfsHardlinkWriter->StartMkdirThread(), MP_SUCCESS);
// }

TEST_F(LibnfsHardlinkWriterTest, Abort)
{
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsHardlinkWriter->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, GetStatus)
{
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());

    m_libnfsHardlinkWriter->m_controlInfo->m_writePhaseComplete = false;
    EXPECT_EQ(m_libnfsHardlinkWriter->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libnfsHardlinkWriter->m_abort = true;
    m_libnfsHardlinkWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_EQ(m_libnfsHardlinkWriter->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libnfsHardlinkWriter->m_abort = false;
    m_libnfsHardlinkWriter->m_controlInfo->m_failed = false;
    m_libnfsHardlinkWriter->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsHardlinkWriter->m_controlInfo->m_writePhaseComplete = true;
    m_libnfsHardlinkWriter->m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
    EXPECT_EQ(m_libnfsHardlinkWriter->GetStatus(), BackupPhaseStatus::FAILED_NOACCESS);

    m_libnfsHardlinkWriter->m_abort = false;
    m_libnfsHardlinkWriter->m_controlInfo->m_failed = false;
    m_libnfsHardlinkWriter->m_controlInfo->m_controlReaderFailed = false;
    m_libnfsHardlinkWriter->m_controlInfo->m_writePhaseComplete = true;
    EXPECT_EQ(m_libnfsHardlinkWriter->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsHardlinkWriterTest, ProcRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ProcRetryTimers());
}

TEST_F(LibnfsHardlinkWriterTest, ExpireRetryTimers)
{
    EXPECT_NO_THROW(Libnfscommonmethods::ExpireRetryTimers(m_libnfsHardlinkWriter->m_timer));
}

TEST_F(LibnfsHardlinkWriterTest, GetRetryTimerCnt)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->GetRetryTimerCnt());
}

TEST_F(LibnfsHardlinkWriterTest, IsRetryReqEmpty)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->IsRetryReqEmpty());
}

TEST_F(LibnfsHardlinkWriterTest, OpenFile)
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
    EXPECT_EQ(m_libnfsHardlinkWriter->OpenFile(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsHardlinkWriter->OpenFile(fileHandle), MP_FAILED);
}

TEST_F(LibnfsHardlinkWriterTest, WriteData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_size = 0;

    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->WriteData(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_size = 5;
    fileHandle.m_retryCnt = 0;
    MOCKER_CPP(&SendWriteRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->WriteData(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, WriteMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    m_libnfsHardlinkWriter->m_backupParams.commonParams.writeMeta = true;
    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP(&SendSetMetaRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->WriteMeta(fileHandle), MP_SUCCESS);

    m_libnfsHardlinkWriter->m_backupParams.commonParams.writeMeta = false;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->WriteMeta(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, CloseFile)
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
    EXPECT_EQ(m_libnfsHardlinkWriter->CloseFile(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsHardlinkWriter->CloseFile(fileHandle), MP_FAILED);

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendDstCloseRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->CloseFile(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, IsReplacePolicySkip)
{
    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::RESTORE;
    m_libnfsHardlinkWriter->m_backupParams.commonParams.restoreReplacePolicy = RestoreReplacePolicy::NONE;

    EXPECT_EQ(m_libnfsHardlinkWriter->IsReplacePolicySkip(), true);

    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    EXPECT_EQ(m_libnfsHardlinkWriter->IsReplacePolicySkip(), false);
}

TEST_F(LibnfsHardlinkWriterTest, PrintIsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    EXPECT_NO_THROW(m_libnfsHardlinkWriter->PrintIsComplete(true));
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->PrintIsComplete(false));
}

TEST_F(LibnfsHardlinkWriterTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsHardlinkWriter->m_controlInfo->m_aggregatePhaseComplete = true;
    m_libnfsHardlinkWriter->m_mkdirComplete = true;

    MOCKER_CPP(&LibnfsHardlinkWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::IsRetryReqEmpty)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsHardlinkWriter->IsComplete(), true);
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(invoke(GetValue_Stub2));
    EXPECT_EQ(m_libnfsHardlinkWriter->IsComplete(), false);
}

TEST_F(LibnfsHardlinkWriterTest, HandleComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsHardlinkWriter::DeleteWriteContainers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Libnfscommonmethods::ExpireRetryTimers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&FileHandleCache::Clear)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->HandleComplete());
}

TEST_F(LibnfsHardlinkWriterTest, IsBlockRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    EXPECT_NO_THROW(m_libnfsHardlinkWriter->IsBlockRecv());

    auto constexpr MAX_BACKUP_QUEUE_SIZE = 0;
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->IsBlockRecv());
}

TEST_F(LibnfsHardlinkWriterTest, BlockRecv)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->BlockRecv());
}

TEST_F(LibnfsHardlinkWriterTest, IsResumeRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->IsResumeRecv());
}

TEST_F(LibnfsHardlinkWriterTest, ResumeRecv)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ResumeRecv());
}

TEST_F(LibnfsHardlinkWriterTest, IsBlockSend)
{
    m_libnfsHardlinkWriter->m_advParams->maxPendingAsyncReqCnt = 10;
    m_libnfsHardlinkWriter->m_advParams->serverCheckMaxCount = 10;
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(10))
            .then(returnValue(8));
    MOCKER_CPP(&LibnfsHardlinkWriter::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub2))
            .then(invoke(GetValue_Stub1));
    EXPECT_EQ(m_libnfsHardlinkWriter->IsBlockSend(), true);
    EXPECT_EQ(m_libnfsHardlinkWriter->IsBlockSend(), false);
}

TEST_F(LibnfsHardlinkWriterTest, BlockSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    EXPECT_NO_THROW(m_libnfsHardlinkWriter->BlockSend());
}

TEST_F(LibnfsHardlinkWriterTest, IsResumeSend)
{
    m_libnfsHardlinkWriter->m_advParams->minPendingAsyncReqCnt = 10;
    m_libnfsHardlinkWriter->m_advParams->serverCheckMaxCount = 10;
    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(returnValue(9));
    MOCKER_CPP(&LibnfsHardlinkWriter::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub1));
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->IsResumeSend());
}

TEST_F(LibnfsHardlinkWriterTest, ResumeSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);

    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ResumeSend());
}

TEST_F(LibnfsHardlinkWriterTest, IsResumeSendCb)
{
    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    EXPECT_EQ(m_libnfsHardlinkWriter->IsResumeSendCb(&m_libnfsHardlinkWriter), true);
}

TEST_F(LibnfsHardlinkWriterTest, ResumeSendCb)
{
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ResumeSendCb(&m_libnfsHardlinkWriter));
}

TEST_F(LibnfsHardlinkWriterTest, HandleQueueBlock)
{
    MOCKER_CPP(&LibnfsHardlinkWriter::IsBlockRecv)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsHardlinkWriter::IsBlockSend)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsHardlinkWriter::BlockRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::BlockSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->HandleQueueBlock());

    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->HandleQueueBlock());
}

TEST_F(LibnfsHardlinkWriterTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsHardlinkWriter::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ThreadFunc());

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsHardlinkWriter::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ThreadFunc());
}

TEST_F(LibnfsHardlinkWriterTest, ProcessWriteQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ProcessWriteQueue());

    MOCKER_CPP(&LibnfsHardlinkWriter::IsBlockSend)
            .stubs()
            .will(returnValue(false));
    MOCKER_CPP(&LibnfsHardlinkWriter::ProcessWriteEntries)
            .stubs()
            .will(returnValue(1));
    MOCKER_CPP(&LibnfsHardlinkWriter::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkWriter::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->ProcessWriteQueue());
}

TEST_F(LibnfsHardlinkWriterTest, ProcessWriteEntries)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    fileHandle.m_file->m_mode = 0;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_META_MODIFIED;
    MOCKER_CPP(&LibnfsHardlinkWriter::ProcessMetaModifiedFiles)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_BOTH_MODIFIED;
    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::RESTORE;
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
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    MOCKER_CPP(&LinkDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    fileHandle.m_file->m_scannermode = CTRL_ENTRY_MODE_BOTH_MODIFIED;
    MOCKER_CPP(&LinkDeleteForRestore)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&DirectoryDelete)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&LibnfsHardlinkWriter::ProcessFilesCreateWriteClose)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessWriteEntries(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, ProcessMetaModifiedFiles)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_block.m_size = 5;
    FileDescState state {};
    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::RESTORE;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    state = FileDescState::INIT;
    MOCKER_CPP(&LstatFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    m_libnfsHardlinkWriter->m_backupParams.backupType = BackupType::BACKUP_FULL;
    fileHandle.m_file->m_mode = 41471;
    MOCKER_CPP(&WriteSymLinkMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessMetaModifiedFiles(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, ProcessFilesCreateWriteClose)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    FileDescState state = FileDescState::READ_FAILED;
    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP(&FileDesc::SetSrcState)
            .stubs()
            .will(returnValue(FileDescState::READ_FAILED))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP(&FileDesc::SetDstState)
            .stubs()
            .will(returnValue(FileDescState::WRITE_FAILED))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::CloseFile)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    fileHandle.m_file->m_blockStats.m_writeRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_writeReqCnt = 1;
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 41471;
    state = FileDescState::INIT;
    MOCKER_CPP(&LibnfsHardlinkWriter::ProcessSymlink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    state = FileDescState::WRITE_SKIP;
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::LSTAT;
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_seq = 0;
    MOCKER_CPP(&LibnfsHardlinkWriter::ProcessFileOpen)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    fileHandle.m_block.m_size = 5;
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::PARTIAL_WRITED;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::WriteData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::WRITED;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::WriteMeta)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);

    state = FileDescState::META_WRITED;
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFilesCreateWriteClose(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, ProcessSymlink)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    FileDescState state = FileDescState::LSTAT;

    MOCKER_CPP(&CreateSymlink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessSymlink(fileHandle, state), MP_SUCCESS);

    MOCKER_CPP(&WriteSymLinkMeta)
            .stubs()
            .will(returnValue(0));
    state = FileDescState::LINK_DEL_FAILED;
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessSymlink(fileHandle, state), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, ProcessFileOpen)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 4480;
    FillWriterParams(hardlinkWriterParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP(&CreateSpecialFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFileOpen(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::OpenFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkWriter->ProcessFileOpen(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, BatchPush)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkWriter->m_writeQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkWriter->m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsHardlinkWriter->m_writeWaitQueue->Push(fileHandle);
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->BatchPush());
}

TEST_F(LibnfsHardlinkWriterTest, FillWriteContainers)
{
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsHardlinkWriter->FillWriteContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsHardlinkWriter->FillWriteContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsHardlinkWriter->FillWriteContainers(), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkWriterTest, DeleteWriteContainers)
{
    EXPECT_NO_THROW(m_libnfsHardlinkWriter->DeleteWriteContainers());
}
