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
#include "LibnfsHardlinkReader.h"
#include "LibnfsHardlinkWriter.h"
#include "hardlink/Hardlink.h"
#include "hardlink/HardlinkControlFileReader.h"
#include "hardlink/HardlinkAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class HardlinkTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams hardlinkReaderParams {};
    WriterParams hardlinkWriterParams {};
    unique_ptr<Hardlink> m_hardlink = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<HardLinkMap> m_hardlinkMap                 = make_shared<HardLinkMap>();
};

void HardlinkTest::SetUp()
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

    hardlinkReaderParams.backupParams = m_backupParams;
    hardlinkReaderParams.readQueuePtr = m_readQueue;
    hardlinkReaderParams.aggregateQueuePtr = m_aggregateQueue;
    hardlinkReaderParams.blockBufferMap = m_blockBufferMap;
    hardlinkReaderParams.controlInfo = m_controlInfo;
    hardlinkReaderParams.hardlinkMap = m_hardlinkMap;

    hardlinkWriterParams.backupParams = m_backupParams;
    hardlinkWriterParams.writeQueuePtr = m_writeQueue;
    hardlinkWriterParams.readQueuePtr = m_readQueue;
    hardlinkWriterParams.blockBufferMap = m_blockBufferMap;
    hardlinkWriterParams.controlInfo = m_controlInfo;
    hardlinkWriterParams.hardlinkMap = m_hardlinkMap;

    m_hardlink = make_unique<Hardlink>(m_backupParams);
}

void HardlinkTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void HardlinkTest::SetUpTestCase()
{}

void HardlinkTest::TearDownTestCase()
{}

static time_t GetCurrentTimeInSeconds_Stub()
{
    chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t currTime = chrono::duration_cast<chrono::seconds>(duration).count();
    return currTime;
}

TEST_F(HardlinkTest, Start)
{
    m_hardlink->m_controlFileReader = make_unique<HardlinkControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
    m_hardlink->m_aggregator = make_unique<HardlinkAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
    m_hardlink->m_reader = make_unique<LibnfsHardlinkReader>(hardlinkReaderParams);
    m_hardlink->m_writer = make_unique<LibnfsHardlinkWriter>(hardlinkWriterParams);

    HardlinkControlFileReader hardlinkControlFileReader(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
    HardlinkAggregator hardlinkAggregator(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    MOCKER_CPP_VIRTUAL(hardlinkControlFileReader, &HardlinkControlFileReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_hardlink->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Hardlink> m_hardlink1 = make_unique<Hardlink>(m_backupParams);
    EXPECT_EQ(m_hardlink1->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(hardlinkAggregator, &HardlinkAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Hardlink> m_hardlink2 = make_unique<Hardlink>(m_backupParams);
    EXPECT_EQ(m_hardlink2->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Hardlink> m_hardlink3 = make_unique<Hardlink>(m_backupParams);
    EXPECT_EQ(m_hardlink3->Start(), BackupRetCode::FAILED);
    unique_ptr<Hardlink> m_hardlink4 = make_unique<Hardlink>(m_backupParams);
    EXPECT_EQ(m_hardlink4->Start(), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkTest, Abort)
{
    HardlinkControlFileReader hardlinkControlFileReader(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
    HardlinkAggregator hardlinkAggregator(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
    LibnfsHardlinkReader libnfsHardlinkReader(hardlinkReaderParams);
    LibnfsHardlinkWriter libnfsHardlinkWriter(hardlinkWriterParams);

    m_hardlink->m_abort = true;
    EXPECT_EQ(m_hardlink->Abort(), BackupRetCode::SUCCESS);

    m_hardlink->m_abort = false;
    MOCKER_CPP_VIRTUAL(hardlinkControlFileReader, &HardlinkControlFileReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkReader, &LibnfsHardlinkReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(hardlinkAggregator, &HardlinkAggregator::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsHardlinkWriter, &LibnfsHardlinkWriter::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_hardlink->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkTest, Destroy)
{
    EXPECT_EQ(m_hardlink->Destroy(), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkTest, Enqueue)
{
    string ctrlFile = "abc.txt";
    m_hardlink->m_controlFileReader =
        make_unique<HardlinkControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
    EXPECT_EQ(m_hardlink->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkTest, GetStats)
{
    m_hardlink->m_controlInfo = make_shared<BackupControlInfo>();
    MOCKER_CPP(&Module::ParserUtils::GetCurrentTimeInSeconds)
            .stubs()
            .will(invoke(GetCurrentTimeInSeconds_Stub));
    BackupStats stats = m_hardlink->GetStats();
    EXPECT_EQ(stats.noOfDirToBackup, m_controlInfo->m_noOfDirToBackup);
}

TEST_F(HardlinkTest, GetFailedDetails)
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
    MOCKER_CPP(&WriterBase::GetFailedList)
            .stubs()
            .will(returnValue(map1))
            .then(returnValue(map2));
    MOCKER_CPP(&ReaderBase::GetFailedList)
            .stubs()
            .will(returnValue(map1))
            .then(returnValue(map3));
    EXPECT_EQ(m_hardlink->GetFailedDetails().size(), 1);
    EXPECT_EQ(m_hardlink->GetFailedDetails().size(), 1);
}

TEST_F(HardlinkTest, CreateBackupStatistic)
{
    EXPECT_NO_THROW(m_hardlink->CreateBackupStatistic());
}

TEST_F(HardlinkTest, CreateBackupQueue)
{
    EXPECT_NO_THROW(m_hardlink->CreateBackupQueue());
}

TEST_F(HardlinkTest, IsMemberNull)
{
    m_hardlink->m_controlFileReader = make_unique<HardlinkControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
    m_hardlink->m_aggregator = make_unique<HardlinkAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo, m_hardlinkMap);
    m_hardlink->m_reader = make_unique<LibnfsHardlinkReader>(hardlinkReaderParams);
    m_hardlink->m_writer = make_unique<LibnfsHardlinkWriter>(hardlinkWriterParams);

    EXPECT_EQ(m_hardlink->IsMemberNull(), false);

    m_hardlink->m_controlFileReader = nullptr;
    EXPECT_EQ(m_hardlink->IsMemberNull(), true);
}
TEST_F(HardlinkTest, IsCompleted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::COMPLETED;

    bool ret = m_hardlink->IsCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_hardlink->IsCompleted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(HardlinkTest, IsAborted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::ABORTED;

    bool ret = m_hardlink->IsAborted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_hardlink->IsAborted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(HardlinkTest, GetFailureStatus)
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

TEST_F(HardlinkTest, IsStatusFailed)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_hardlink->IsStatusFailed(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::COMPLETED;
    EXPECT_EQ(m_hardlink->IsStatusFailed(copyStatus1), false);
}

TEST_F(HardlinkTest, IsCiticalError)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::FAILED;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus1), false);
}
