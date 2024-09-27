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
#include "LibnfsDirMetaReader.h"
#include "LibnfsDirMetaWriter.h"
#include "dir/Dir.h"
#include "dir/DirControlFileReader.h"
#include "dir/DirAggregator.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

class DirTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    ReaderParams dirReaderParams {};
    WriterParams dirWriterParams {};
    unique_ptr<Dir> m_dir = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DirTest::SetUp()
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
    m_backupParams.phase = BackupPhase::DIR_STAGE;
    m_backupParams.commonParams = commonParams;

    dirReaderParams.backupParams = m_backupParams;
    dirReaderParams.readQueuePtr = m_readQueue;
    dirReaderParams.aggregateQueuePtr = m_aggregateQueue;
    dirReaderParams.controlInfo = m_controlInfo;
    dirReaderParams.blockBufferMap = m_blockBufferMap;

    dirWriterParams.backupParams = m_backupParams;
    dirWriterParams.writeQueuePtr = m_writeQueue;
    dirWriterParams.controlInfo = m_controlInfo;
    dirWriterParams.blockBufferMap = m_blockBufferMap;

    m_dir = make_unique<Dir>(m_backupParams);
}

void DirTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DirTest::SetUpTestCase()
{}

void DirTest::TearDownTestCase()
{}

static time_t GetCurrentTimeInSeconds_Stub()
{
    chrono::time_point<chrono::system_clock> now = chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    time_t currTime = chrono::duration_cast<chrono::seconds>(duration).count();
    return currTime;
}

TEST_F(DirTest, Start)
{
    m_dir->m_controlFileReader = make_unique<DirControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    m_dir->m_aggregator = make_unique<DirAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo);
    m_dir->m_reader = make_unique<LibnfsDirMetaReader>(dirReaderParams);
    m_dir->m_writer = make_unique<LibnfsDirMetaWriter>(dirWriterParams);

    DirControlFileReader dirControlFileReader(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    DirAggregator dirAggregator(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    LibnfsDirMetaReader libnfsDirMetaReader(dirReaderParams);
    LibnfsDirMetaWriter libnfsDirMetaWriter(dirWriterParams);

    MOCKER_CPP_VIRTUAL(dirControlFileReader, &DirControlFileReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_dir->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsDirMetaReader, &LibnfsDirMetaReader::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Dir> m_dir1 = make_unique<Dir>(m_backupParams);
    EXPECT_EQ(m_dir1->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(dirAggregator, &DirAggregator::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Dir> m_dir2 = make_unique<Dir>(m_backupParams);
    EXPECT_EQ(m_dir2->Start(), BackupRetCode::FAILED);
    MOCKER_CPP_VIRTUAL(libnfsDirMetaWriter, &LibnfsDirMetaWriter::Start)
            .stubs()
            .will(returnValue(BackupRetCode::FAILED))
            .then(returnValue(BackupRetCode::SUCCESS));
    unique_ptr<Dir> m_dir3 = make_unique<Dir>(m_backupParams);
    EXPECT_EQ(m_dir3->Start(), BackupRetCode::FAILED);
    unique_ptr<Dir> m_dir4 = make_unique<Dir>(m_backupParams);
    EXPECT_EQ(m_dir4->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DirTest, Abort)
{
    DirControlFileReader dirControlFileReader(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    DirAggregator dirAggregator(m_backupParams, m_aggregateQueue, m_writeQueue, m_controlInfo);
    LibnfsDirMetaReader libnfsDirMetaReader(dirReaderParams);
    LibnfsDirMetaWriter libnfsDirMetaWriter(dirWriterParams);

    m_dir->m_abort = true;
    EXPECT_EQ(m_dir->Abort(), BackupRetCode::SUCCESS);

    m_dir->m_abort = false;
    MOCKER_CPP_VIRTUAL(dirControlFileReader, &DirControlFileReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsDirMetaReader, &LibnfsDirMetaReader::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(dirAggregator, &DirAggregator::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    MOCKER_CPP_VIRTUAL(libnfsDirMetaWriter, &LibnfsDirMetaWriter::Abort)
            .stubs()
            .will(returnValue(BackupRetCode::SUCCESS));
    EXPECT_EQ(m_dir->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DirTest, Destroy)
{
    EXPECT_EQ(m_dir->Destroy(), BackupRetCode::SUCCESS);
}

TEST_F(DirTest, Enqueue)
{
    string ctrlFile = "abc.txt";
    m_dir->m_controlFileReader =
        make_unique<DirControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    EXPECT_EQ(m_dir->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(DirTest, GetStats)
{
    m_dir->m_controlInfo = make_shared<BackupControlInfo>();
    MOCKER_CPP(&Module::ParserUtils::GetCurrentTimeInSeconds)
            .stubs()
            .will(invoke(GetCurrentTimeInSeconds_Stub));
    m_dir->GetStats();
}

TEST_F(DirTest, GetFailedDetails)
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
    EXPECT_EQ(m_dir->GetFailedDetails().size(), 1);
    EXPECT_EQ(m_dir->GetFailedDetails().size(), 1);
}

TEST_F(DirTest, CreateBackupStatistic)
{
    m_dir->CreateBackupStatistic();
}

TEST_F(DirTest, CreateBackupQueue)
{
    m_dir->CreateBackupQueue();
}

/*TEST_F(DirTest, CreateBackupEngine1)
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

    MOCKER_CPP(&Dir::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&Dir::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_dir->CreateBackupEngine(backupParams);

    m_dir->CreateBackupEngine(m_backupParams);

    ArchiveRestoreAdvanceParams archiveRestoreAdvanceParams {};
    backupParams.srcAdvParams = make_shared<ArchiveRestoreAdvanceParams>(archiveRestoreAdvanceParams);
    backupParams.srcAdvParams->dataPath = "xyz";
    BackupParams backupParams2 {};
    backupParams2.srcEngine = BackupIOEngine::ARCHIVE_CLIENT;
    m_dir->CreateBackupEngine(backupParams2);

    BackupParams backupParams3 {};
    LibsmbBackupAdvanceParams libsmbBackupAdvanceParams {};
    backupParams3.srcAdvParams = make_shared<LibsmbBackupAdvanceParams>(libsmbBackupAdvanceParams);
    backupParams3.srcAdvParams->serverCheckMaxCount = 100;
    backupParams3.srcEngine = BackupIOEngine::LIBSMB;
    backupParams3.dstEngine = BackupIOEngine::LIBSMB;
    m_dir->CreateBackupEngine(backupParams3);
}

TEST_F(DirTest, CreateBackupEngine)
{
    const string source = "xyz";
    const string destination = "abc";
    string metaPath = "abcd";
    bool writeMeta = true;

    MOCKER_CPP(&Dir::FillReaderParams)
            .stubs()
            .will(ignoreReturnValue());
    MOCKER_CPP(&Dir::FillWriterParams)
            .stubs()
            .will(ignoreReturnValue());
    m_dir->CreateBackupEngine(source, destination, metaPath, writeMeta);
}*/

TEST_F(DirTest, IsMemberNull)
{
    m_dir->m_controlFileReader = make_unique<DirControlFileReader>(m_backupParams, m_readQueue, m_controlInfo, m_blockBufferMap);
    m_dir->m_aggregator = make_unique<DirAggregator>(m_backupParams, m_aggregateQueue, m_writeQueue,
        m_controlInfo);
    m_dir->m_reader = make_unique<LibnfsDirMetaReader>(dirReaderParams);
    m_dir->m_writer = make_unique<LibnfsDirMetaWriter>(dirWriterParams);

    EXPECT_EQ(m_dir->IsMemberNull(), false);

    m_dir->m_controlFileReader = nullptr;
    EXPECT_EQ(m_dir->IsMemberNull(), true);
}
TEST_F(DirTest, IsCompleted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::COMPLETED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::COMPLETED;

    bool ret = m_dir->IsCompleted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_dir->IsCompleted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(DirTest, IsAborted)
{
    BackupPhaseStatus controlFileReaderStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus readerStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus aggregatorStatus = BackupPhaseStatus::ABORTED;
    BackupPhaseStatus writerStatus = BackupPhaseStatus::ABORTED;

    bool ret = m_dir->IsAborted(controlFileReaderStatus, readerStatus, aggregatorStatus, writerStatus);
    EXPECT_EQ(ret, true);

    BackupPhaseStatus controlFileReaderStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus readerStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus aggregatorStatus1 = BackupPhaseStatus::FAILED;
    BackupPhaseStatus writerStatus1 = BackupPhaseStatus::FAILED;
    ret = m_dir->IsAborted(controlFileReaderStatus1, readerStatus1, aggregatorStatus1, writerStatus1);
    EXPECT_EQ(ret, false);
}

TEST_F(DirTest, GetFailureStatus)
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

TEST_F(DirTest, IsStatusFailed)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(m_dir->IsStatusFailed(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::COMPLETED;
    EXPECT_EQ(m_dir->IsStatusFailed(copyStatus1), false);
}

TEST_F(DirTest, IsCiticalError)
{
    const BackupPhaseStatus copyStatus = BackupPhaseStatus::FAILED_PROT_SERVER_NOTREACHABLE;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus), true);

    const BackupPhaseStatus copyStatus1 = BackupPhaseStatus::FAILED;
    EXPECT_EQ(FSBackupUtils::IsCiticalError(copyStatus1), false);
}

TEST_F(DirTest, FillReaderParams)
{
    ReaderParams dirReaderParams = m_dir->WrapReaderParams();
}

TEST_F(DirTest, FillWriterParams)
{
    WriterParams dirWriterParams = m_dir->WrapWriterParams();
}