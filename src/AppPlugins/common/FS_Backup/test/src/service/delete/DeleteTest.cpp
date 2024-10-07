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
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "LibnfsDeleteReader.h"
#include "LibnfsDeleteWriter.h"
#include "delete/Delete.h"
#include "delete/DeleteControlFileReader.h"
#include "delete/DeleteAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class DeleteTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams deleteReaderParams {};
    WriterParams deleteWriterParams {};
    unique_ptr<Delete> m_delete = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DeleteTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->ip = "10.247.77.219";
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->sharePath = "/Ajo_New";
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->maxPendingAsyncReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->minPendingAsyncReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->maxPendingWriteReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->minPendingWriteReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->maxPendingReadReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->minPendingReadReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->serverCheckMaxCount = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->serverCheckSleepTime = 30;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams)->serverCheckRetry = 3;

    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->ip = "10.247.77.219";
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->sharePath = "/Ajo_R";
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->maxPendingAsyncReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->minPendingAsyncReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->maxPendingWriteReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->minPendingWriteReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->maxPendingReadReqCnt = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->minPendingReadReqCnt = 80;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->serverCheckMaxCount = 100;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->serverCheckSleepTime = 30;
    dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams)->serverCheckRetry = 3;

    CommonParams commonParams {};
    commonParams.maxBufferCnt = 10;
    commonParams.maxBufferSize = 10 * 1024; // 10kb
    commonParams.maxErrorFiles = 100;
    commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::NONE;
    commonParams.jobId = "jobId";
    commonParams.subJobId = "subJobId";
    commonParams.reqID = 0x10000;
    m_backupParams.commonParams = commonParams;

    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.phase = BackupPhase::DELETE_STAGE;

    deleteReaderParams.backupParams = m_backupParams;
    deleteReaderParams.readQueuePtr = m_readQueue;
    deleteReaderParams.aggregateQueuePtr = m_aggregateQueue;
    deleteReaderParams.controlInfo = m_controlInfo;
    deleteReaderParams.blockBufferMap = m_blockBufferMap;

    deleteWriterParams.backupParams = m_backupParams;
    deleteWriterParams.writeQueuePtr = m_writeQueue;
    deleteWriterParams.controlInfo = m_controlInfo;
    deleteWriterParams.blockBufferMap = m_blockBufferMap;

    m_delete = make_unique<Delete>(m_backupParams);
}

void DeleteTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DeleteTest::SetUpTestCase()
{}

void DeleteTest::TearDownTestCase()
{}

static time_t GetCurrentTimeInSeconds_Stub()
{
    chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t currTime = chrono::duration_cast<chrono::seconds>(duration).count();
    return currTime;
}

TEST_F(DeleteTest, Start)
{
    m_delete->m_controlFileReader = make_unique<DeleteControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    m_delete->m_aggregator = make_unique<DeleteAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo);
    m_delete->m_reader = make_unique<LibnfsDeleteReader>(deleteReaderParams);
    m_delete->m_writer = make_unique<LibnfsDeleteWriter>(deleteWriterParams);

    DeleteControlFileReader deleteControlFileReader(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    DeleteAggregator deleteAggregator(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    LibnfsDeleteReader libnfsDeleteReader(deleteReaderParams);
    LibnfsDeleteWriter libnfsDeleteWriter(deleteWriterParams);

    MOCKER_CPP_VIRTUAL(deleteControlFileReader, &DeleteControlFileReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_delete->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsDeleteReader, &LibnfsDeleteReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Delete> m_delete1 = make_unique<Delete>(m_backupParams);
    EXPECT_EQ(m_delete1->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(deleteAggregator, &DeleteAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Delete> m_delete2 = make_unique<Delete>(m_backupParams);
    EXPECT_EQ(m_delete2->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsDeleteWriter, &LibnfsDeleteWriter::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Delete> m_delete3 = make_unique<Delete>(m_backupParams);
    EXPECT_EQ(m_delete3->Start(), BackupRetCode::FAILED);
    unique_ptr<Delete> m_delete4 = make_unique<Delete>(m_backupParams);
    EXPECT_EQ(m_delete4->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteTest, Abort)
{
    DeleteControlFileReader deleteControlFileReader(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    DeleteAggregator deleteAggregator(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    LibnfsDeleteReader libnfsDeleteReader(deleteReaderParams);
    LibnfsDeleteWriter libnfsDeleteWriter(deleteWriterParams);

    m_delete->m_abort = true;
    EXPECT_EQ(m_delete->Abort(), BackupRetCode::SUCCESS);

    m_delete->m_abort = false;
    MOCKER_CPP_VIRTUAL(deleteControlFileReader, &DeleteControlFileReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsDeleteReader, &LibnfsDeleteReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(deleteAggregator, &DeleteAggregator::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsDeleteWriter, &LibnfsDeleteWriter::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_delete->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteTest, Destroy)
{
    EXPECT_EQ(m_delete->Destroy(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteTest, Enqueue)
{
    string ctrlFile = "abc.txt";
    m_delete->m_controlFileReader =
        make_unique<DeleteControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    EXPECT_EQ(m_delete->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(DeleteTest, GetStatus)
{
    DeleteControlFileReader deleteControlFileReader(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    DeleteAggregator deleteAggregator(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    LibnfsDeleteReader libnfsDeleteReader(deleteReaderParams);
    LibnfsDeleteWriter libnfsDeleteWriter(deleteWriterParams);

    MOCKER_CPP(&Delete::IsMemberNull)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_delete->GetStatus(), BackupPhaseStatus::FAILED);

    m_delete->m_abort = true;
    MOCKER_CPP_VIRTUAL(deleteControlFileReader, &DeleteControlFileReader::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP_VIRTUAL(libnfsDeleteReader, &LibnfsDeleteReader::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP_VIRTUAL(deleteAggregator, &DeleteAggregator::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP_VIRTUAL(libnfsDeleteWriter, &LibnfsDeleteWriter::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP(&Delete::IsAborted)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_delete->GetStatus(), BackupPhaseStatus::ABORTED);
    EXPECT_EQ(m_delete->GetStatus(), BackupPhaseStatus::ABORT_INPROGRESS);

    m_delete->m_abort = false;
}

TEST_F(DeleteTest, GetStats)
{
    m_delete->m_controlInfo = make_shared<BackupControlInfo>();
    MOCKER_CPP(&Module::ParserUtils::GetCurrentTimeInSeconds)
            .stubs()
            .will(invoke(GetCurrentTimeInSeconds_Stub));
    m_delete->GetStats();
}

TEST_F(DeleteTest, GetFailedDetails)
{
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::POSIX, BackupIOEngine::POSIX);
    fileHandle.m_file->m_fileName = "test";
    std::vector<FileHandle> map1;
    map1.push_back(fileHandle);
    std::vector<FileHandle> map2;
    map2.push_back(fileHandle);
    map2.push_back(fileHandle);
    std::vector<FileHandle> map3;
    map3.push_back(fileHandle);
    map3.push_back(fileHandle);
    map3.push_back(fileHandle);
    std::vector<FileHandle> map4;
    map4.push_back(fileHandle);
    map4.push_back(fileHandle);
    map4.push_back(fileHandle);
    map4.push_back(fileHandle);
    MOCKER_CPP(&ReaderBase::GetFailedList)
            .stubs()
            .will(returnValue(map1))
            .then(returnValue(map2));
    MOCKER_CPP(&WriterBase::GetFailedList)
            .stubs()
            .will(returnValue(map1))
            .then(returnValue(map3));
    EXPECT_EQ(m_delete->GetFailedDetails().size(), 1);
    EXPECT_EQ(m_delete->GetFailedDetails().size(), 1);
}

TEST_F(DeleteTest, CreateBackupStatistic)
{
    m_delete->CreateBackupStatistic();
}

TEST_F(DeleteTest, CreateBackupQueue)
{
    m_delete->CreateBackupQueue();
}
/*
TEST_F(DeleteTest, CreateBackupEngine1)
{
    BackupParams backupParams {};
    HostBackupAdvanceParams posixBackupAdvanceParams {};
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>(posixBackupAdvanceParams);
    backupParams.srcAdvParams->dataPath = "xyz";
    backupParams.dstAdvParams->dataPath = "abc";
    backupParams.srcAdvParams->serverCheckMaxCount = 100;
    backupParams.dstAdvParams->serverCheckMaxCount = 100;
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;

    MOCKER_CPP(&Delete::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&Delete::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_delete->CreateBackupEngine(backupParams);

    m_delete->CreateBackupEngine(m_backupParams);

    ArchiveRestoreAdvanceParams archiveRestoreAdvanceParams {};
    backupParams.srcAdvParams = make_shared<ArchiveRestoreAdvanceParams>(archiveRestoreAdvanceParams);
    backupParams.srcAdvParams->dataPath = "xyz";
    BackupParams backupParams2 {};
    backupParams2.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    m_delete->CreateBackupEngine(backupParams2);

    BackupParams backupParams3 {};
    LibsmbBackupAdvanceParams libsmbBackupAdvanceParams {};
    backupParams3.srcAdvParams = make_shared<LibsmbBackupAdvanceParams>(libsmbBackupAdvanceParams);
    backupParams3.srcAdvParams->serverCheckMaxCount = 100;
    backupParams3.srcEngine = BackupIOEngine::LIBSMB;
    backupParams3.dstEngine = BackupIOEngine::LIBSMB;
    m_delete->CreateBackupEngine(backupParams3);
}

TEST_F(DeleteTest, CreateBackupEngine)
{
    const string source = "xyz";
    const string destination = "abc";
    string metaPath = "abcd";
    bool writeMeta = true;

    MOCKER_CPP(&Delete::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Delete::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue());
    m_delete->CreateBackupEngine(source, destination, metaPath, writeMeta);
*/
TEST_F(DeleteTest, IsMemberNull)
{
    m_delete->m_controlFileReader = make_unique<DeleteControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    m_delete->m_aggregator = make_unique<DeleteAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo);
    m_delete->m_reader = make_unique<LibnfsDeleteReader>(deleteReaderParams);
    m_delete->m_writer = make_unique<LibnfsDeleteWriter>(deleteWriterParams);

    EXPECT_EQ(m_delete->IsMemberNull(), false);

    m_delete->m_controlFileReader = nullptr;
    EXPECT_EQ(m_delete->IsMemberNull(), true);
}
TEST_F(DeleteTest, IsCompleted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::COMPLETED;

    bool ret = m_delete->IsCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_delete->IsCompleted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(DeleteTest, IsAborted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::ABORTED;

    bool ret = m_delete->IsAborted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_delete->IsAborted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(DeleteTest, GetFailureStatus)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::FAILED;

    MOCKER_CPP(&FSBackupUtils::IsCiticalError)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    BackupPhaseStatus ret = FSBackupUtils::GetFailureStatus(readerStatus, writerStatus);
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;

    ret = FSBackupUtils::GetFailureStatus(readerStatus1, writerStatus1);
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE);

    BackupPhaseStatus controlFileReaderStatus2 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus2 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus2 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus2 = BackupPhaseStatus::COMPLETED;

    ret = FSBackupUtils::GetFailureStatus(readerStatus2, writerStatus2);
    EXPECT_EQ(ret, BackupPhaseStatus::FAILED);
}

TEST_F(DeleteTest, IsStatusFailed)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_delete->IsStatusFailed(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::COMPLETED;
    EXPECT_EQ(m_delete->IsStatusFailed(copyStatus1), false);
}

TEST_F(DeleteTest, IsCiticalError)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::FAILED;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus1), false);
}

TEST_F(DeleteTest, FillReaderParams)
{
    ReaderParams deleteReaderParams = m_delete->WrapReaderParams();
}

TEST_F(DeleteTest, FillWriterParams)
{
    WriterParams deleteWriterParams = m_delete->WrapWriterParams();
}
