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
#include "common/FSBackupUtils.h"
#include "dir/DirControlFileReader.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int SKIP = 1;
    const int FINISH = 2;
}

class DirControlFileReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<DirControlFileReader> m_dirCtrlFileReader = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DirControlFileReaderTest::SetUp()
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
    commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    m_backupParams.commonParams = commonParams;

    m_dirCtrlFileReader = make_unique<DirControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap);
}

void DirControlFileReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DirControlFileReaderTest::SetUpTestCase()
{}

void DirControlFileReaderTest::TearDownTestCase()
{}

TEST_F(DirControlFileReaderTest, Start)
{
    m_dirCtrlFileReader->m_abort = true;
    MOCKER_CPP(&DirControlFileReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_dirCtrlFileReader->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DirControlFileReaderTest, Abort)
{
    EXPECT_EQ(m_dirCtrlFileReader->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DirControlFileReaderTest, Enqueue)
{
    std::string ctrlFile = "abc.txt";
    EXPECT_EQ(m_dirCtrlFileReader->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(DirControlFileReaderTest, GetStatus)
{
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_dirCtrlFileReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_dirCtrlFileReader->m_abort = true;
    EXPECT_EQ(m_dirCtrlFileReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_dirCtrlFileReader->m_abort = false;
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_dirCtrlFileReader->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_dirCtrlFileReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_dirCtrlFileReader->m_abort = false;
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_dirCtrlFileReader->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_dirCtrlFileReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(DirControlFileReaderTest, IsAbort)
{
    m_dirCtrlFileReader->m_abort = false;
    m_dirCtrlFileReader->m_controlInfo->m_failed = false;
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_dirCtrlFileReader->IsAbort(), true);

    m_dirCtrlFileReader->m_abort = false;
    m_dirCtrlFileReader->m_controlInfo->m_failed = false;
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_dirCtrlFileReader->IsAbort(), false);
}

TEST_F(DirControlFileReaderTest, IsComplete)
{
    m_dirCtrlFileReader->m_dirCount = 1;
    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    EXPECT_EQ(m_dirCtrlFileReader->IsComplete(), true);

    m_dirCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_dirCtrlFileReader->IsComplete(), false);
}

TEST_F(DirControlFileReaderTest, ThreadFunc)
{
    std::string ctrlFile = "abc.txt";
    MOCKER_CPP(&FSBackupUtils::CheckMetaFileVersion)
            .stubs()
            .will(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V20));
    MOCKER_CPP(&DirControlFileReader::OpenControlFile)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    MOCKER_CPP(&DirControlFileReader::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&DirControlFileReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    m_dirCtrlFileReader->Enqueue(ctrlFile);
    m_dirCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&DirControlFileReader::ReadControlFileEntryAndProcessV10)
            .stubs()
            .will(returnValue(FINISH));
    m_dirCtrlFileReader->Enqueue(ctrlFile);
    m_dirCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&DirControlFileReader::ReadControlFileEntryAndProcess)
            .stubs()
            .will(returnValue(SKIP));
    m_dirCtrlFileReader->Enqueue(ctrlFile);
    m_dirCtrlFileReader->ThreadFunc();

    m_dirCtrlFileReader->Enqueue(ctrlFile);
    m_dirCtrlFileReader->ThreadFunc();
}

TEST_F(DirControlFileReaderTest, OpenControlFile)
{
    std::string ctrlFile = "abc.txt";
    m_dirCtrlFileReader->m_metaFileVersion = META_VERSION_V10;
    MOCKER_CPP(&DirControlFileReader::OpenControlFileV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);

    m_dirCtrlFileReader->m_metaFileVersion = META_VERSION_V20;
    MOCKER_CPP(&DirControlFileReader::OpenControlFileV20)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);
}

TEST_F(DirControlFileReaderTest, ReadControlFileEntry)
{
    Module::MtimeCtrlEntry dirEntry {};
    std::string controlFile = "Abc.txt";
    m_dirCtrlFileReader->m_mtimeCtrlParser = make_unique<Module::MtimeCtrlParser>(controlFile);
    MOCKER_CPP(&Module::MtimeCtrlParser::ReadEntry)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::READ_EOF))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS));
    EXPECT_EQ(m_dirCtrlFileReader->ReadControlFileEntry(dirEntry), FINISH);
    EXPECT_EQ(m_dirCtrlFileReader->ReadControlFileEntry(dirEntry), Module::SUCCESS);
}

TEST_F(DirControlFileReaderTest, ProcessDirEntry)
{
    Module::MtimeCtrlEntry dirEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    EXPECT_EQ(m_dirCtrlFileReader->ProcessDirEntry(dirEntry, fileHandle), Module::SUCCESS);
}

TEST_F(DirControlFileReaderTest, ProcessDirEntryV10)
{
    BackupMtimeCtrlEntry dirEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    EXPECT_EQ(m_dirCtrlFileReader->ProcessDirEntryV10(dirEntry, fileHandle), Module::SUCCESS);
}

TEST_F(DirControlFileReaderTest, FillStatsFromControlHeader)
{
    MOCKER_CPP(&Module::MtimeCtrlParser::GetHeader)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_dirCtrlFileReader->FillStatsFromControlHeader(), Module::SUCCESS);
    EXPECT_EQ(m_dirCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);
}

TEST_F(DirControlFileReaderTest, OpenControlFileV10)
{
    std::string controlFile = "abc.txt";
    m_dirCtrlFileReader->m_scannerMtimeCtrl = std::make_unique<BackupMtimeCtrl>(controlFile);
    MOCKER_CPP(&BackupMtimeCtrl::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    MOCKER_CPP(&DirControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFileV10(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
}

TEST_F(DirControlFileReaderTest, OpenControlFileV20)
{
    std::string controlFile = "abc.txt";
    m_dirCtrlFileReader->m_mtimeCtrlParser = make_unique<Module::MtimeCtrlParser>(controlFile);
    MOCKER_CPP(&Module::MtimeCtrlParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    MOCKER_CPP(&DirControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    m_dirCtrlFileReader->OpenControlFileV20(controlFile);
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
    EXPECT_EQ(m_dirCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
}

TEST_F(DirControlFileReaderTest, FillStatsFromControlHeaderV10)
{
    std::string controlFile = "abc.txt";
    m_dirCtrlFileReader->m_scannerMtimeCtrl = std::make_unique<BackupMtimeCtrl>(controlFile);
    MOCKER_CPP(&BackupMtimeCtrl::GetHeader)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    EXPECT_EQ(m_dirCtrlFileReader->FillStatsFromControlHeaderV10(), Module::SUCCESS);
    EXPECT_EQ(m_dirCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);

    m_dirCtrlFileReader->m_scannerMtimeCtrl = nullptr;
    EXPECT_EQ(m_dirCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);
}

TEST_F(DirControlFileReaderTest, ReadControlFileEntryAndProcess)
{
    Module::MtimeCtrlEntry dirEntry {};
    dirEntry.m_absPath = "d1";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    MOCKER_CPP(&DirControlFileReader::ReadControlFileEntry)
            .stubs()
            .will(returnValue(SKIP))
            .then(returnValue(SKIP))
            .then(returnValue(FINISH));
    MOCKER_CPP(&DirControlFileReader::ProcessDirEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    int ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcess(dirEntry, fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_absPath = ".";
    ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcess(dirEntry, fileHandle);
    EXPECT_EQ(ret, SKIP);

    ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcess(dirEntry, fileHandle);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(DirControlFileReaderTest, ReadControlFileEntryAndProcessV10)
{
    BackupMtimeCtrlEntry dirEntry {};
    dirEntry.m_absPath = "d1";
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    std::string controlFile = "abc.txt";
    m_dirCtrlFileReader->m_scannerMtimeCtrl = std::make_unique<BackupMtimeCtrl>(controlFile);
    MOCKER_CPP(&BackupMtimeCtrl::ReadEntry)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_READ_EOF));
    MOCKER_CPP(&DirControlFileReader::ProcessDirEntryV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    int ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcessV10(dirEntry, fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_absPath = ".";
    ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcessV10(dirEntry, fileHandle);
    EXPECT_EQ(ret, SKIP);

    ret = m_dirCtrlFileReader->ReadControlFileEntryAndProcessV10(dirEntry, fileHandle);
    EXPECT_EQ(ret, FINISH);
}