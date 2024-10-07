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
#include "mockcpp/mockcpp.hpp"
#include "log/Log.h"
#include "BackupQueue.h"
#include "Backup.h"
#include "BackupMgr.h"
#include "PacketStats.h"
#include "LibnfsHardlinkReader.h"
#include "libnfs_ctx/NfsContextWrapper.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace  {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class LibnfsHardlinkReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    void FillReaderParams(ReaderParams &hardlinkReaderParams);
    BackupParams m_backupParams {};

    ReaderParams hardlinkReaderParams {};
    std::unique_ptr<LibnfsHardlinkReader> m_libnfsHardlinkReader = nullptr;
};

void LibnfsHardlinkReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    hardlinkReaderParams.backupParams = m_backupParams;
    hardlinkReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    hardlinkReaderParams.hardlinkMap = make_shared<HardLinkMap>();
    m_libnfsHardlinkReader = std::make_unique<LibnfsHardlinkReader>(hardlinkReaderParams);
}

void LibnfsHardlinkReaderTest::FillReaderParams(ReaderParams &hardlinkReaderParams)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    hardlinkReaderParams.backupParams = m_backupParams;
    hardlinkReaderParams.readQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.aggregateQueuePtr = std::make_shared<BackupQueue<FileHandle>>(config);
    hardlinkReaderParams.controlInfo = std::make_shared<BackupControlInfo>();
    hardlinkReaderParams.blockBufferMap = std::make_shared<BlockBufferMap>();
    hardlinkReaderParams.hardlinkMap = make_shared<HardLinkMap>();
}

void LibnfsHardlinkReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void LibnfsHardlinkReaderTest::SetUpTestCase()
{}

void LibnfsHardlinkReaderTest::TearDownTestCase()
{}

TEST_F(LibnfsHardlinkReaderTest, Start)
{
    FillReaderParams(hardlinkReaderParams);

    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    m_libnfsHardlinkReader->m_abort = true;
    MOCKER_CPP(&LibnfsHardlinkReader::FillReadContainers)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    EXPECT_EQ(m_libnfsHardlinkReader->Start(), BackupRetCode::SUCCESS);
    MOCKER_CPP(&LibnfsHardlinkReader::DeleteReadContainers)
            .stubs()
            .will(ignoreReturnValue());
    std::unique_ptr<LibnfsHardlinkReader> m_libnfsHardlinkReader1 =
        std::make_unique<LibnfsHardlinkReader>(hardlinkReaderParams);
    EXPECT_EQ(m_libnfsHardlinkReader1->Start(), BackupRetCode::FAILED);
}

TEST_F(LibnfsHardlinkReaderTest, Abort)
{
    BackupRetCode ret = m_libnfsHardlinkReader->Abort();
    EXPECT_EQ(ret, BackupRetCode::SUCCESS);
}

TEST_F(LibnfsHardlinkReaderTest, GetStatus)
{
    m_libnfsHardlinkReader->m_controlInfo->m_readPhaseComplete = false;
    EXPECT_EQ(m_libnfsHardlinkReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_libnfsHardlinkReader->m_abort = true;
    m_libnfsHardlinkReader->m_controlInfo->m_readPhaseComplete = true;
    EXPECT_EQ(m_libnfsHardlinkReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_libnfsHardlinkReader->m_abort = false;
    m_libnfsHardlinkReader->m_controlInfo->m_failed = false;
    m_libnfsHardlinkReader->m_controlInfo->m_controlReaderFailed = true;
    m_libnfsHardlinkReader->m_failReason = BackupPhaseStatus::FAILED;
    EXPECT_EQ(m_libnfsHardlinkReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_libnfsHardlinkReader->m_abort = false;
    m_libnfsHardlinkReader->m_controlInfo->m_failed = false;
    m_libnfsHardlinkReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_libnfsHardlinkReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(LibnfsHardlinkReaderTest, ProcRetryTimers)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->ProcRetryTimers());
}

TEST_F(LibnfsHardlinkReaderTest, ExpireRetryTimers)
{
    EXPECT_NO_THROW(Libnfscommonmethods::ExpireRetryTimers(m_libnfsHardlinkReader->m_timer));
}

TEST_F(LibnfsHardlinkReaderTest, GetRetryTimerCnt)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->GetRetryTimerCnt());
}

TEST_F(LibnfsHardlinkReaderTest, IsRetryReqEmpty)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsRetryReqEmpty());
}

TEST_F(LibnfsHardlinkReaderTest, OpenFile)
{
    FillReaderParams(hardlinkReaderParams);
    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_file->m_size = 0;

    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->OpenFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->m_size = 4;
    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    memset_s(fileHandle.m_file->srcIOHandle.nfsFh, sizeof(struct nfsfh), 0, sizeof(struct nfsfh));
    fileHandle.m_file->srcIOHandle.nfsFh->fh.len = 0;
    MOCKER_CPP(&SendOpenRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->OpenFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh->fh.len = 64;
    fileHandle.m_file->srcIOHandle.nfsFh->fh.val = (char *) malloc(64);
    memset_s(fileHandle.m_file->srcIOHandle.nfsFh->fh.val, 64, 0, 64);

    fileHandle.m_file->m_size = 4;
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->OpenFile(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkReaderTest, ReadMeta)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    int ret = m_libnfsHardlinkReader->ReadMeta(fileHandle);
    EXPECT_EQ(ret, MP_SUCCESS);
}

TEST_F(LibnfsHardlinkReaderTest, ReadData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "/1.txt";
    fileHandle.m_retryCnt = 1;
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkReader->m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&SendReadRequest)
            .stubs()
            .will(returnValue(0))
            .then(returnValue(1));
    EXPECT_EQ(m_libnfsHardlinkReader->ReadData(fileHandle), MP_SUCCESS);

    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->m_size = 0;
    MOCKER_CPP(&HandleZeroSizeFileRead)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsHardlinkReader->ReadData(fileHandle), MP_SUCCESS);

    m_libnfsHardlinkReader->m_backupParams.commonParams.blockSize = 1;
    fileHandle.m_file->m_size = 4;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsHardlinkReader->ReadData(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&LibnfsHardlinkReader::IsBlockSend)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsHardlinkReader->ReadData(fileHandle), MP_FAILED);

    fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;

    FillReaderParams(hardlinkReaderParams);
    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ReadData(fileHandle), MP_SUCCESS);

    fileHandle.m_file->SetSrcState(FileDescState::INIT);
    fileHandle.m_file->SetDstState(FileDescState::INIT);
    fileHandle.m_file->m_size = 4;
    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    EXPECT_NO_THROW(m_libnfsHardlinkReader->ReadData(fileHandle));
}

TEST_F(LibnfsHardlinkReaderTest, CloseFile)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    MOCKER_CPP(&FileDesc::IsFlagSet)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsHardlinkReader->CloseFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*) malloc(sizeof(struct nfsfh));
    MOCKER_CPP(&SendSrcCloseRequest)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->CloseFile(fileHandle), MP_SUCCESS);

    fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
    EXPECT_EQ(m_libnfsHardlinkReader->CloseFile(fileHandle), MP_FAILED);
}

TEST_F(LibnfsHardlinkReaderTest, IsComplete)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_pktStats = make_shared<PacketStats>();

    m_libnfsHardlinkReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    bool ret = m_libnfsHardlinkReader->IsComplete();
    EXPECT_EQ(ret, true);
}

TEST_F(LibnfsHardlinkReaderTest, HandleComplete)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->srcIOHandle.nfsFh = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_pktStats = make_shared<PacketStats>();

    EXPECT_NO_THROW(m_libnfsHardlinkReader->m_readQueue->Push(fileHandle));
    EXPECT_NO_THROW(m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPush(fileHandle));
    EXPECT_NO_THROW(m_libnfsHardlinkReader->HandleComplete());
}

TEST_F(LibnfsHardlinkReaderTest, CanRecv)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    FileHandle fileHandle1 {};
    fileHandle1.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsHardlinkReader->m_readQueue->Push(fileHandle1);
    m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPush(fileHandle);

    MOCKER_CPP(&LibnfsHardlinkReader::IsBlockRecv)
            .stubs()
            .will(returnValue(false));
    EXPECT_EQ(m_libnfsHardlinkReader->CanRecv(this), true);
}

TEST_F(LibnfsHardlinkReaderTest, IsBlockRecv)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    FileHandle fileHandle1 {};
    fileHandle1.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsHardlinkReader->m_readQueue->Push(fileHandle1);
    EXPECT_NO_THROW(m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPush(fileHandle));

    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsBlockRecv());
}

TEST_F(LibnfsHardlinkReaderTest, BlockRecv)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->BlockRecv());
}

TEST_F(LibnfsHardlinkReaderTest, IsResumeRecv)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    FileHandle fileHandle1 {};
    fileHandle1.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};

    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    EXPECT_NO_THROW(m_libnfsHardlinkReader->m_readQueue->Push(fileHandle1));
    EXPECT_NO_THROW(m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPush(fileHandle));

    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsResumeRecv());
}

TEST_F(LibnfsHardlinkReaderTest, ResumeRecv)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->ResumeRecv());
}

TEST_F(LibnfsHardlinkReaderTest, IsBlockSend)
{
    m_libnfsHardlinkReader->m_pktStats = make_shared<PacketStats>();
    m_libnfsHardlinkReader->m_advParams->maxPendingAsyncReqCnt = 0;
    m_libnfsHardlinkReader->m_advParams->serverCheckMaxCount = 0;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferCount = 10;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferCnt = 5;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferSize = 100;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferSize =50;
    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsBlockSend());
}

TEST_F(LibnfsHardlinkReaderTest, BlockSend)
{
    EXPECT_NO_THROW(m_libnfsHardlinkReader->BlockSend());
}

TEST_F(LibnfsHardlinkReaderTest, IsResumeSend)
{
    m_libnfsHardlinkReader->m_pktStats = make_shared<PacketStats>();
    m_libnfsHardlinkReader->m_advParams->minPendingAsyncReqCnt = 10;
    m_libnfsHardlinkReader->m_advParams->serverCheckMaxCount = 0;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferCount = 5;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferCnt = 10;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferSize = 10;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferSize =50;
    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsResumeSend());
}

TEST_F(LibnfsHardlinkReaderTest, ResumeSend)
{
    m_libnfsHardlinkReader->ResumeSend();
}

TEST_F(LibnfsHardlinkReaderTest, IsResumeSendCb)
{
    MOCKER_CPP(&LibnfsHardlinkReader::IsResumeSend)
            .stubs()
            .will(returnValue(true));

    EXPECT_NO_THROW(m_libnfsHardlinkReader->IsResumeSendCb(this));
}

TEST_F(LibnfsHardlinkReaderTest, ResumeSendCb)
{
    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    MOCKER_CPP(&LibnfsHardlinkReader::ResumeSend)
            .stubs()
            .will(ignoreReturnValue());

    EXPECT_NO_THROW(m_libnfsHardlinkReader->ResumeSendCb(this));
}

TEST_F(LibnfsHardlinkReaderTest, ProcessPartialReadQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue->Push(fileHandle);

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true));

    EXPECT_NO_THROW(m_libnfsHardlinkReader->ProcessPartialReadQueue());
}

TEST_F(LibnfsHardlinkReaderTest, ProcessReadQueue)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue->Push(fileHandle);
    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_readQueue->Push(fileHandle);

    m_libnfsHardlinkReader->ProcessReadQueue();

    m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPop(fileHandle);
    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_readQueue->Push(fileHandle);

    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true));
    EXPECT_NO_THROW(m_libnfsHardlinkReader->ProcessReadQueue());
}

TEST_F(LibnfsHardlinkReaderTest, ProcessReadEntries)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 16832;
    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = false;

    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsHardlinkReader->ProcessReadEntries(fileHandle);

    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = true;
    m_libnfsHardlinkReader->ProcessReadEntries(fileHandle);

    fileHandle.m_file->m_mode = 0;
    MOCKER_CPP(&LibnfsHardlinkReader::ProcessFileTypes)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsHardlinkReader::IsResumeRecv)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkReader::ResumeRecv)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessReadEntries(fileHandle), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkReaderTest, ProcessFileTypes)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_mode = 16832;
    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = false;

    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::LINK))
            .then(returnValue(FileDescState::LINK))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle), MP_SUCCESS);

    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = true;
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle), MP_SUCCESS);

    FileHandle fileHandle1 {};
    fileHandle1.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle1.m_file->m_mode = 41471;
    MOCKER_CPP(&ReadLink)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle1), MP_SUCCESS);

    FileHandle fileHandle2 {};
    fileHandle2.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle2.m_file->m_mode = 4480;
    MOCKER_CPP(&PushToAggregator)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = false;
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle2), MP_SUCCESS);

    m_libnfsHardlinkReader->m_backupParams.commonParams.writeDisable = true;
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle2), MP_SUCCESS);

    FileHandle fileHandle3 {};
    fileHandle3.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle3.m_file->m_mode = 0;
    MOCKER_CPP(&LibnfsHardlinkReader::ProcessFileToRead)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileTypes(fileHandle3), MP_SUCCESS);
}

TEST_F(LibnfsHardlinkReaderTest, ProcessFileToRead)
{
    FillReaderParams(hardlinkReaderParams);
    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_blockStats.m_readRespCnt = 1;
    fileHandle.m_file->m_blockStats.m_readReqCnt = 1;

    MOCKER_CPP(&FileDesc::GetDstState)
            .stubs()
            .will(returnValue(FileDescState::WRITE_SKIP))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::INIT));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    MOCKER_CPP(&FileDesc::GetSrcState)
            .stubs()
            .will(returnValue(FileDescState::INIT))
            .then(returnValue(FileDescState::SRC_OPENED))
            .then(returnValue(FileDescState::READED))
            .then(returnValue(FileDescState::SRC_CLOSED));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::OpenFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::ReadData)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileToRead(fileHandle), MP_SUCCESS);

    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::CloseFile)
            .stubs()
            .will(returnValue(0));
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileToRead(fileHandle), MP_SUCCESS);
    EXPECT_EQ(m_libnfsHardlinkReader->ProcessFileToRead(fileHandle), MP_SUCCESS);
}
TEST_F(LibnfsHardlinkReaderTest, HandleQueueBlock)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    FileHandle fileHandle1 {};
    fileHandle1.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    m_libnfsHardlinkReader->m_readQueue = std::make_shared<BackupQueue<FileHandle>>(config);
    m_libnfsHardlinkReader->m_partialReadQueue = std::make_shared<BackupQueue<FileHandle>>(config);

    m_libnfsHardlinkReader->m_readQueue->Push(fileHandle1);
    m_libnfsHardlinkReader->m_partialReadQueue->WaitAndPush(fileHandle);

    m_libnfsHardlinkReader->m_pktStats = make_shared<PacketStats>();
    m_libnfsHardlinkReader->m_advParams->minPendingAsyncReqCnt = 10;
    m_libnfsHardlinkReader->m_advParams->serverCheckMaxCount = 0;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferCount = 5;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferCnt = 10;
    m_libnfsHardlinkReader->m_blockBufferMap->m_blockBufferSize = 10;
    m_libnfsHardlinkReader->m_backupParams.commonParams.maxBufferSize =50;

    EXPECT_NO_THROW(m_libnfsHardlinkReader->HandleQueueBlock());
}

TEST_F(LibnfsHardlinkReaderTest, FillReadContainers)
{
    MOCKER_CPP(&Libnfscommonmethods::FillNfsContextContainer)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(false));
    EXPECT_EQ(m_libnfsHardlinkReader->FillReadContainers(), MP_SUCCESS);
    EXPECT_EQ(m_libnfsHardlinkReader->FillReadContainers(), MP_FAILED);
}

TEST_F(LibnfsHardlinkReaderTest, ThreadFunc)
{
    MOCKER_CPP(&Libnfscommonmethods::IsAbort)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&LibnfsHardlinkReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    m_libnfsHardlinkReader->ThreadFunc();

    MOCKER_CPP(&Libnfscommonmethods::NfsServerCheck)
            .stubs()
            .will(returnValue(0));
    MOCKER_CPP(&LibnfsHardlinkReader::IsComplete)
            .stubs()
            .will(returnValue(true));
    MOCKER_CPP(&LibnfsHardlinkReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_NO_THROW(m_libnfsHardlinkReader->ThreadFunc());
}
