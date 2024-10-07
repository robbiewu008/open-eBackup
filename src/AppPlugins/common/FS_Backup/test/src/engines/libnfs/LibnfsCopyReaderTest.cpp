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
#include <stdexcept>
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "LibnfsCopyReader.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsCopyReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void FillReaderParams(ReaderParams &copyReaderParams);
    BackupParams m_backupParams {};
    ReaderParams copyReaderParams {};
    std::unique_ptr<LibnfsCopyReader> m_libnfsCopyReader = nullptr;
};

void LibnfsCopyReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    copyReaderParams.backupParams = m_backupParams;
    copyReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();

    m_libnfsCopyReader = std::make_unique<LibnfsCopyReader>(copyReaderParams);
}

void LibnfsCopyReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsCopyReaderTest::SetUpTestCase()
{}

void LibnfsCopyReaderTest::TearDownTestCase()
{}

void LibnfsCopyReaderTest::FillReaderParams(ReaderParams &copyReaderParams)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    copyReaderParams.backupParams = m_backupParams;
    copyReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    copyReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    copyReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
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

// TEST_F(LibnfsCopyReaderTest, Start)
// {
//     FillReaderParams(copyReaderParams);
//     LibnfsCopyReader libnfsCopyReader(copyReaderParams);

//     MOCKER_CPP(&LibnfsCopyReader::FillReadContainers)
//             .stubs()
//             .will(returnValue(0))
//             .then(returnValue(1));
//     MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::ThreadFunc)
//             .stubs()
//             .will(ignoreReturnValue());
//     EXPECT_EQ(m_libnfsCopyReader->Start(), BackupRetCode::SUCCESS);

//     MOCKER_CPP(&LibnfsCopyReader::DeleteReadContainers)
//             .stubs()
//             .will(ignoreReturnValue());
//     std::unique_ptr<LibnfsCopyReader> m_libnfsCopyReader1 =
//         std::make_unique<LibnfsCopyReader>(copyReaderParams);
//     EXPECT_EQ(m_libnfsCopyReader1->Start(), BackupRetCode::FAILED);
// }

TEST_F(LibnfsCopyReaderTest, Abort)
{
    MOCKER_CPP(&LibnfsCopyReader::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyReader::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsCopyReader->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, GetStatus)
{
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());

    m_libnfsCopyReader->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_libnfsCopyReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libnfsCopyReader->m_abort = true;
    m_libnfsCopyReader->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_libnfsCopyReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libnfsCopyReader->m_abort = false;
    m_libnfsCopyReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsCopyReader->m_controlInfo->m_failed = false;
    m_libnfsCopyReader->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsCopyReader->m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
    EXPECT_EQ(m_libnfsCopyReader->GetStatus(), BackupPhaseStatus::FAILED_NOACCESS);

    m_libnfsCopyReader->m_abort = false;
    m_libnfsCopyReader->m_controlInfo->m_failed = false;
    m_libnfsCopyReader->m_controlInfo->m_readPhaseComplete = true;
    m_libnfsCopyReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libnfsCopyReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsCopyReaderTest, ProcRetryTimers)
{
    m_libnfsCopyReader->ProcRetryTimers();
}

TEST_F(LibnfsCopyReaderTest, ExpireRetryTimers)
{
    Libnfscommonmethods::ExpireRetryTimers(m_libnfsCopyReader->m_timer);
}

TEST_F(LibnfsCopyReaderTest, GetRetryTimerCnt)
{
    m_libnfsCopyReader->GetRetryTimerCnt();
}

TEST_F(LibnfsCopyReaderTest, IsRetryReqEmpty)
{
    m_libnfsCopyReader->IsRetryReqEmpty();
}

TEST_F(LibnfsCopyReaderTest, OpenFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_file->m_size = 0;

    FillReaderParams(copyReaderParams);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->OpenFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_size = 5;
    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    memset_s(fileHandle.m_file->srcIOHandle.nfsFh, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
    fileHandle.m_file->srcIOHandle.nfsFh->fh.len = 0;
    MOCKER_CPP(&SendOpenRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->OpenFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh->fh.len = 64;
    fileHandle.m_file->srcIOHandle.nfsFh->fh.val = (char *) malloc(64);
    memset_s(fileHandle.m_file->srcIOHandle.nfsFh->fh.val, 64, 0, 64);

    fileHandle.m_file->m_size = 5;
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->OpenFile(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, ReadMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    EXPECT_EQ(m_libnfsCopyReader->ReadMeta(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, ReadData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_retryCnt = 1;
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);

    FillReaderParams(copyReaderParams);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);

    MOCKER_CPP(&SendReadRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(m_libnfsCopyReader->ReadData(fileHandle), MP_SUCCESS);

    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->m_size = 0;
    MOCKER_CPP(&HandleZeroSizeFileRead)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsCopyReader->ReadData(fileHandle), MP_SUCCESS);

    m_libnfsCopyReader->m_backupParams.commonParams.blockSize = 1;
    fileHandle.m_file->m_size = 4;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsCopyReader->ReadData(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&LibnfsCopyReader::IsBlockSend)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsCopyReader->ReadData(fileHandle), MP_FAILED);

    fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ReadData(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    fileHandle.m_file->m_size = 4;
    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    m_libnfsCopyReader->ReadData(fileHandle);
}

TEST_F(LibnfsCopyReaderTest, CloseFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsCopyReader->CloseFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendSrcCloseRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->CloseFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
    EXPECT_EQ(m_libnfsCopyReader->CloseFile(fileHandle), MP_FAILED);
}

TEST_F(LibnfsCopyReaderTest, PrintIsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyReader->m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyReader->PrintIsComplete(true);
    m_libnfsCopyReader->PrintIsComplete(false);
}

TEST_F(LibnfsCopyReaderTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyReader->m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyReader->m_controlInfo = std::make_shared<BackupControlInfo>();

    MOCKER_CPP(&LibnfsCopyReader::IsRetryReqEmpty)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsCopyReader->IsComplete(), false);
}

TEST_F(LibnfsCopyReaderTest, HandleComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsCopyReader->m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsCopyReader::DeleteReadContainers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Libnfscommonmethods::ExpireRetryTimers)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyReader::PrintIsComplete)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&PacketStats::Print)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&BlockBufferMap::Print)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->HandleComplete();
}

TEST_F(LibnfsCopyReaderTest, CanRecv)
{
    MOCKER_CPP(&LibnfsCopyReader::IsBlockRecv)
            .stubs()
            .will(returnValue(false));
    EXPECT_EQ(m_libnfsCopyReader->CanRecv(&m_libnfsCopyReader), true);
}

TEST_F(LibnfsCopyReaderTest, IsBlockRecv)
{
    EXPECT_EQ(m_libnfsCopyReader->IsBlockRecv(), false);
}

TEST_F(LibnfsCopyReaderTest, BlockRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyReader->BlockRecv();
}

TEST_F(LibnfsCopyReaderTest, IsResumeRecv)
{
    EXPECT_EQ(m_libnfsCopyReader->IsResumeRecv(), true);
}

TEST_F(LibnfsCopyReaderTest, ResumeRecv)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyReader->ResumeRecv();
}

TEST_F(LibnfsCopyReaderTest, IsBlockSend)
{
    m_libnfsCopyReader->m_advParams->maxPendingAsyncReqCnt = 0;
    m_libnfsCopyReader->m_advParams->serverCheckMaxCount = 0;
    m_libnfsCopyReader->m_blockBufferMap->m_blockBufferCount = 10;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferCnt = 10;
    m_libnfsCopyReader->m_blockBufferMap->m_blockBufferSize = 10;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferSize = 10;

    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(invoke(GetValue_Stub2))
            .then(invoke(GetValue_Stub2))
            .then(invoke(GetValue_Stub1))
            .then(invoke(GetValue_Stub1));
    MOCKER_CPP(&LibnfsCopyReader::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub1))
            .then(invoke(GetValue_Stub1));
    EXPECT_EQ(m_libnfsCopyReader->IsBlockSend(), true);

    m_libnfsCopyReader->m_advParams->maxPendingAsyncReqCnt = 10;
    m_libnfsCopyReader->m_advParams->serverCheckMaxCount = 10;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferCnt = 100;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferSize = 100;
    EXPECT_EQ(m_libnfsCopyReader->IsBlockSend(), false);
}

TEST_F(LibnfsCopyReaderTest, BlockSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyReader->BlockSend();
}

TEST_F(LibnfsCopyReaderTest, IsResumeSend)
{
    m_libnfsCopyReader->m_advParams->minPendingAsyncReqCnt = 10;
    m_libnfsCopyReader->m_advParams->serverCheckMaxCount = 10;
    m_libnfsCopyReader->m_blockBufferMap->m_blockBufferCount = 10;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferCnt = 100;
    m_libnfsCopyReader->m_blockBufferMap->m_blockBufferSize = 10;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferSize = 100;

    MOCKER_CPP(&PacketStats::GetValue)
            .stubs()
            .will(invoke(GetValue_Stub1))
            .then(invoke(GetValue_Stub1))
            .then(invoke(GetValue_Stub2))
            .then(invoke(GetValue_Stub2));
    MOCKER_CPP(&LibnfsCopyReader::GetRetryTimerCnt)
            .stubs()
            .will(invoke(GetValue_Stub1))
            .then(invoke(GetValue_Stub1));
    EXPECT_EQ(m_libnfsCopyReader->IsResumeSend(), true);

    m_libnfsCopyReader->m_advParams->minPendingAsyncReqCnt = 0;
    m_libnfsCopyReader->m_advParams->serverCheckMaxCount = 0;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferCnt = 1;
    m_libnfsCopyReader->m_backupParams.commonParams.maxBufferSize = 1;
    EXPECT_EQ(m_libnfsCopyReader->IsResumeSend(), false);
}

TEST_F(LibnfsCopyReaderTest, ResumeSend)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsCopyReader->ResumeSend();
}

TEST_F(LibnfsCopyReaderTest, IsResumeSendCb)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsCopyReader::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    EXPECT_EQ(m_libnfsCopyReader->IsResumeSendCb(&m_libnfsCopyReader), true);
}

TEST_F(LibnfsCopyReaderTest, ResumeSendCb)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_readQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsCopyReader::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->ResumeSendCb(&m_libnfsCopyReader);
}

TEST_F(LibnfsCopyReaderTest, HandleQueueBlock)
{
    MOCKER_CPP(&LibnfsCopyReader::IsBlockRecv)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyReader::IsBlockSend)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyReader::BlockRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyReader::BlockSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->HandleQueueBlock();

    MOCKER_CPP(&LibnfsCopyReader::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::IsResumeSend)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyReader::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->HandleQueueBlock();
}

TEST_F(LibnfsCopyReaderTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsCopyReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->ThreadFunc();

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsCopyReader::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsCopyReader->ThreadFunc();
}

TEST_F(LibnfsCopyReaderTest, ProcessReadEntries)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 16832;
    m_libnfsCopyReader->m_backupParams.commonParams.writeDisable = false;
    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&LibnfsCopyReader::IsResumeRecv)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&LibnfsCopyReader::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP(&LibnfsCopyReader::ProcessFileTypes)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessReadEntries(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, ProcessFileTypes)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_file->m_mode = 41471; // symbolic link

    MOCKER_CPP(&ReadLink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileTypes(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_mode = 4480; // special device -4544
    m_libnfsCopyReader->m_backupParams.commonParams.writeDisable = false;
    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileTypes(fileHandle), MP_SUCCESS);

    m_libnfsCopyReader->m_backupParams.commonParams.writeDisable = true;
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileTypes(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_mode = 16832;
    MOCKER_CPP(&LibnfsCopyReader::ProcessFileToRead)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileTypes(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, ProcessFileToRead)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsCopyReader->m_aggregateQueue = make_shared<BackupQueue<FileHandle>>(config);
    FillReaderParams(copyReaderParams);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);

    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::CloseFile)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::OpenFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::PARTIAL_READED);
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::READED);
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::AGGREGATED);
    EXPECT_EQ(m_libnfsCopyReader->ProcessFileToRead(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, FillReadContainers)
{
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    EXPECT_EQ(m_libnfsCopyReader->FillReadContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsCopyReader->FillReadContainers(), MP_FAILED);
    EXPECT_EQ(m_libnfsCopyReader->FillReadContainers(), MP_SUCCESS);
}

TEST_F(LibnfsCopyReaderTest, DeleteReadContainers)
{
    m_libnfsCopyReader->DeleteReadContainers();
}
