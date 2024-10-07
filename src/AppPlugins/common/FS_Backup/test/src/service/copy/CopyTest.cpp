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
#include <iostream>
#include "common/FSBackupUtils.h"
#include "gtest/gtest.h"
#include "mockcpp/mockcpp.hpp"
#include "LibnfsCopyReader.h"
#include "LibnfsCopyWriter.h"
#include "copy/Copy.h"
#include "copy/CopyControlFileReader.h"
#include "copy/CopyAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class CopyTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams copyReaderParams {};
    WriterParams copyWriterParams {};
    unique_ptr<Copy> m_copy = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void CopyTest::SetUp()
{
    m_backupParams.backupType = BackupType::BACKUP_FULL;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);

    CommonParams commonParams {};
    commonParams.metaPath = "/xx-dir/";
    commonParams.jobId = "qqqqqqqqqq";
    commonParams.subJobId = "wwwwwwwwwwwwwww";
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    m_backupParams.commonParams = commonParams;

    copyReaderParams.backupParams = m_backupParams;
    copyReaderParams.readQueuePtr = m_readQueue;
    copyReaderParams.aggregateQueuePtr = m_aggregateQueue;
    copyReaderParams.controlInfo = m_controlInfo;
    copyReaderParams.blockBufferMap = m_blockBufferMap;

    copyWriterParams.backupParams = m_backupParams;
    copyWriterParams.readQueuePtr = m_readQueue;
    copyWriterParams.writeQueuePtr = m_writeQueue;
    copyWriterParams.controlInfo = m_controlInfo;
    copyWriterParams.blockBufferMap = m_blockBufferMap;

    m_copy = make_unique<Copy>(m_backupParams);
}

void CopyTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CopyTest::SetUpTestCase()
{}

void CopyTest::TearDownTestCase()
{}

static time_t GetCurrentTimeInSeconds_Stub()
{
    chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t currTime = chrono::duration_cast<chrono::seconds>(duration).count();
    return currTime;
}

static std::unordered_map<std::string, int> GetFailedList_Stub(void *ob)
{
    std::unordered_map<std::string, int> failedLis = {{"/home/file1", 2}, {"/home/file2", 3}};
    return failedLis;
}

TEST_F(CopyTest, Start)
{
    m_copy->m_controlFileReader = make_unique<CopyControlFileReader>(m_backupParams, m_readQueue,
        m_aggregateQueue, m_controlInfo, m_blockBufferMap);
    m_copy->m_aggregator = make_unique<CopyAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue, m_readQueue,
                                               m_controlInfo, m_blockBufferMap);
    m_copy->m_reader = make_unique<LibnfsCopyReader>(copyReaderParams);
    m_copy->m_writer = make_unique<LibnfsCopyWriter>(copyWriterParams);

    CopyControlFileReader copyControlFileReader(m_backupParams, m_readQueue, m_aggregateQueue, m_controlInfo,
        m_blockBufferMap);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP_VIRTUAL(copyControlFileReader, &CopyControlFileReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_copy->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Copy> m_copy1 = make_unique<Copy>(m_backupParams);
    EXPECT_EQ(m_copy1->Start(), BackupRetCode::FAILED);
    MOCKER_CPP(&CopyAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Copy> m_copy2 = make_unique<Copy>(m_backupParams);
    EXPECT_EQ(m_copy2->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Copy> m_copy3 = make_unique<Copy>(m_backupParams);
    EXPECT_EQ(m_copy3->Start(), BackupRetCode::FAILED);

    unique_ptr<Copy> m_copy4 = make_unique<Copy>(m_backupParams);
    EXPECT_EQ(m_copy4->Start(), BackupRetCode::SUCCESS);
}

TEST_F(CopyTest, Abort)
{
    CopyControlFileReader copyControlFileReader(m_backupParams, m_readQueue, m_aggregateQueue, m_controlInfo,
        m_blockBufferMap);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    m_copy->m_abort = true;
    EXPECT_EQ(m_copy->Abort(), BackupRetCode::SUCCESS);

    m_copy->m_abort = false;
    MOCKER_CPP_VIRTUAL(copyControlFileReader, &CopyControlFileReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP(&CopyAggregator::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_copy->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(CopyTest, Destroy)
{
    EXPECT_EQ(m_copy->Destroy(), BackupRetCode::SUCCESS);
}

TEST_F(CopyTest, Enqueue)
{
    string ctrlFile = "abc.txt";
    m_copy->m_controlFileReader =
        make_unique<CopyControlFileReader>(m_backupParams, m_readQueue, m_aggregateQueue, m_controlInfo,
            m_blockBufferMap);
    EXPECT_EQ(m_copy->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(CopyTest, GetStatus)
{
    CopyControlFileReader copyControlFileReader(m_backupParams, m_readQueue, m_aggregateQueue, m_controlInfo,
        m_blockBufferMap);
    LibnfsCopyReader libnfsCopyReader(copyReaderParams);
    LibnfsCopyWriter libnfsCopyWriter(copyWriterParams);

    MOCKER_CPP(&Copy::IsMemberNull)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::FAILED);

    m_copy->m_abort = true;
    MOCKER_CPP_VIRTUAL(copyControlFileReader, &CopyControlFileReader::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP_VIRTUAL(libnfsCopyReader, &LibnfsCopyReader::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP(&CopyAggregator::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP_VIRTUAL(libnfsCopyWriter, &LibnfsCopyWriter::GetStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::COMPLETED));
    MOCKER_CPP(&Copy::IsAborted)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::ABORTED);
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::ABORT_INPROGRESS);

    m_copy->m_abort = false;
    MOCKER_CPP(&Copy::IsFailedOrCompleted)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    MOCKER_CPP(&FSBackupUtils::GetFailureStatus)
            .stubs()
            .will(returnValue(BackupPhaseStatus::FAILED));
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::COMPLETED);

    MOCKER_CPP(&Copy::IsCompleted)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false));
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::COMPLETED);
    EXPECT_EQ(m_copy->GetStatus(), BackupPhaseStatus::FAILED);
}

TEST_F(CopyTest, GetStats)
{
    m_copy->m_controlInfo = make_shared<BackupControlInfo>();
    MOCKER_CPP(&Module::ParserUtils::GetCurrentTimeInSeconds)
            .stubs()
            .will(invoke(GetCurrentTimeInSeconds_Stub));
    m_copy->GetStats();
}

TEST_F(CopyTest, CreateBackupStatistic)
{
    m_copy->CreateBackupStatistic();
}

TEST_F(CopyTest, GetFailedDetails)
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
    EXPECT_EQ(m_copy->GetFailedDetails().size(), 1);
    EXPECT_EQ(m_copy->GetFailedDetails().size(), 1);
}

/*TEST_F(CopyTest, CreateBackupEngine1)
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

    MOCKER_CPP(&Copy::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&Copy::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_copy->CreateBackupEngine(backupParams);

    m_copy->CreateBackupEngine(m_backupParams);

    ArchiveRestoreAdvanceParams archiveRestoreAdvanceParams {};
    backupParams.srcAdvParams = make_shared<ArchiveRestoreAdvanceParams>(archiveRestoreAdvanceParams);
    backupParams.srcAdvParams->dataPath = "xyz";
    BackupParams backupParams2 {};
    backupParams2.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    m_copy->CreateBackupEngine(backupParams2);

    BackupParams backupParams3 {};
    LibsmbBackupAdvanceParams libsmbBackupAdvanceParams {};
    backupParams3.srcAdvParams = make_shared<LibsmbBackupAdvanceParams>(libsmbBackupAdvanceParams);
    backupParams3.srcAdvParams->serverCheckMaxCount = 100;
    backupParams3.srcEngine = BackupIOEngine::LIBSMB;
    backupParams3.dstEngine = BackupIOEngine::LIBSMB;
    m_copy->CreateBackupEngine(backupParams3);
}

TEST_F(CopyTest, CreateBackupEngine)
{
    const string source = "xyz";
    const string destination = "abc";
    string metaPath = "abcd";
    bool writeMeta = true;

    MOCKER_CPP(&Copy::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Copy::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue());
    m_copy->CreateBackupEngine(source, destination, metaPath, writeMeta);
}*/

TEST_F(CopyTest, IsMemberNull)
{
    m_copy->m_controlFileReader = make_unique<CopyControlFileReader>(m_backupParams, m_readQueue, m_aggregateQueue,
        m_controlInfo, m_blockBufferMap);
    m_copy->m_aggregator = make_unique<CopyAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue, m_readQueue,
                                               m_controlInfo, m_blockBufferMap);
    m_copy->m_reader = make_unique<LibnfsCopyReader>(copyReaderParams);
    m_copy->m_writer = make_unique<LibnfsCopyWriter>(copyWriterParams);

    EXPECT_EQ(m_copy->IsMemberNull(), false);

    m_copy->m_controlFileReader = nullptr;
    EXPECT_EQ(m_copy->IsMemberNull(), true);
}
TEST_F(CopyTest, IsCompleted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::COMPLETED;

    bool ret = m_copy->IsCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_copy->IsCompleted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(CopyTest, IsFailed)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;

    MOCKER_CPP(&Copy::IsStatusFailed)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(true))
            .then(returnValue(false));
    bool ret = m_copy->IsFailedOrCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::COMPLETED;
    ret = m_copy->IsFailedOrCompleted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, true);
}

TEST_F(CopyTest, IsAborted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::ABORTED;

    bool ret = m_copy->IsAborted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_copy->IsAborted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(CopyTest, GetFailureStatus)
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

TEST_F(CopyTest, IsStatusFailed)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_copy->IsStatusFailed(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::COMPLETED;
    EXPECT_EQ(m_copy->IsStatusFailed(copyStatus1), false);
}

TEST_F(CopyTest, IsCiticalError)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::FAILED;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus1), false);
}

TEST_F(CopyTest, FillReaderParams)
{
    ReaderParams copyReaderParams = m_copy->WrapReaderParams();
}

TEST_F(CopyTest, FillWriterParams)
{
    WriterParams copyWriterParams =m_copy->WrapWriterParams();
}
