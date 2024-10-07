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
#include "delete/DeleteControlFileReader.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int SKIP = 1;
    const int FINISH = 2;
}

class DeleteControlFileReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<DeleteControlFileReader> m_deleteCtrlFileReader = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void DeleteControlFileReaderTest::SetUp()
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

    m_deleteCtrlFileReader = make_unique<DeleteControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap);
}

void DeleteControlFileReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void DeleteControlFileReaderTest::SetUpTestCase()
{}

void DeleteControlFileReaderTest::TearDownTestCase()
{}

TEST_F(DeleteControlFileReaderTest, Start)
{
    m_deleteCtrlFileReader->m_abort = true;
    MOCKER_CPP(&DeleteControlFileReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_deleteCtrlFileReader->Start(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteControlFileReaderTest, Abort)
{
    EXPECT_EQ(m_deleteCtrlFileReader->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(DeleteControlFileReaderTest, Enqueue)
{
    std::string ctrlFile = "abc.txt";
    EXPECT_EQ(m_deleteCtrlFileReader->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(DeleteControlFileReaderTest, GetStatus)
{
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_deleteCtrlFileReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_deleteCtrlFileReader->m_abort = true;
    EXPECT_EQ(m_deleteCtrlFileReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_deleteCtrlFileReader->m_abort = false;
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_deleteCtrlFileReader->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_deleteCtrlFileReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_deleteCtrlFileReader->m_abort = false;
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_deleteCtrlFileReader->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_deleteCtrlFileReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(DeleteControlFileReaderTest, IsAbort)
{
    m_deleteCtrlFileReader->m_abort = false;
    m_deleteCtrlFileReader->m_controlInfo->m_failed = false;
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_deleteCtrlFileReader->IsAbort(), true);

    m_deleteCtrlFileReader->m_abort = false;
    m_deleteCtrlFileReader->m_controlInfo->m_failed = false;
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_deleteCtrlFileReader->IsAbort(), false);
}

TEST_F(DeleteControlFileReaderTest, IsComplete)
{
    m_deleteCtrlFileReader->m_fileCount = 1;
    m_deleteCtrlFileReader->m_dirCount = 1;
    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    EXPECT_EQ(m_deleteCtrlFileReader->IsComplete(), true);

    m_deleteCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_deleteCtrlFileReader->IsComplete(), false);
}

TEST_F(DeleteControlFileReaderTest, HandleComplete)
{
    m_deleteCtrlFileReader->HandleComplete();
}

TEST_F(DeleteControlFileReaderTest, ThreadFunc)
{
    std::string ctrlFile = "abc.txt";
    MOCKER_CPP(&FSBackupUtils::CheckMetaFileVersion)
            .stubs()
            .will(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V20));
    MOCKER_CPP(&DeleteControlFileReader::OpenControlFile)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    MOCKER_CPP(&DeleteControlFileReader::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&DeleteControlFileReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&DeleteControlFileReader::HandleComplete)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue())
            .then(ignoreReturnValue());
    m_deleteCtrlFileReader->Enqueue(ctrlFile);
    m_deleteCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&DeleteControlFileReader::ReadControlFileEntryAndProcessV10)
            .stubs()
            .will(returnValue(FINISH));
    m_deleteCtrlFileReader->Enqueue(ctrlFile);
    m_deleteCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&DeleteControlFileReader::ReadControlFileEntryAndProcess)
            .stubs()
            .will(returnValue(SKIP));
    m_deleteCtrlFileReader->Enqueue(ctrlFile);
    m_deleteCtrlFileReader->ThreadFunc();

    m_deleteCtrlFileReader->Enqueue(ctrlFile);
    m_deleteCtrlFileReader->ThreadFunc();
}

TEST_F(DeleteControlFileReaderTest, OpenControlFile)
{
    std::string ctrlFile = "abc.txt";
    m_deleteCtrlFileReader->m_metaFileVersion = META_VERSION_V10;
    MOCKER_CPP(&DeleteControlFileReader::OpenControlFileV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);

    m_deleteCtrlFileReader->m_metaFileVersion = META_VERSION_V20;
    MOCKER_CPP(&DeleteControlFileReader::OpenControlFileV20)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);
}

TEST_F(DeleteControlFileReaderTest, ReadControlFileEntry)
{
    Module::DeleteCtrlEntry deleteEntry {};
    std::string fileName = "1.txt";
    MOCKER_CPP(&Module::DeleteCtrlParser::ReadEntry)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::READ_EOF))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS));
    EXPECT_EQ(m_deleteCtrlFileReader->ReadControlFileEntry(deleteEntry, fileName), FINISH);
    EXPECT_EQ(m_deleteCtrlFileReader->ReadControlFileEntry(deleteEntry, fileName), Module::SUCCESS);
}

TEST_F(DeleteControlFileReaderTest, FillStatsFromControlHeader)
{
    MOCKER_CPP(&Module::DeleteCtrlParser::GetHeader)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_deleteCtrlFileReader->FillStatsFromControlHeader(), Module::SUCCESS);
    EXPECT_EQ(m_deleteCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);
}

TEST_F(DeleteControlFileReaderTest, OpenControlFileV10)
{
    std::string controlFile = "abc.txt";
    m_deleteCtrlFileReader->m_scannerDeleteCtrl = std::make_unique<BackupDeleteCtrl>(controlFile);
    MOCKER_CPP(&BackupDeleteCtrl::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    MOCKER_CPP(&DeleteControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFileV10(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
}

TEST_F(DeleteControlFileReaderTest, OpenControlFileV20)
{
    std::string controlFile = "abc.txt";
    m_deleteCtrlFileReader->m_deleteCtrlParser = std::make_unique<Module::DeleteCtrlParser>(controlFile);
    MOCKER_CPP(&Module::DeleteCtrlParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    MOCKER_CPP(&DeleteControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    m_deleteCtrlFileReader->OpenControlFileV20(controlFile);
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
    EXPECT_EQ(m_deleteCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
}

TEST_F(DeleteControlFileReaderTest, FillStatsFromControlHeaderV10)
{
    std::string controlFile = "abc.txt";
    m_deleteCtrlFileReader->m_scannerDeleteCtrl = std::make_unique<BackupDeleteCtrl>(controlFile);
    MOCKER_CPP(&BackupDeleteCtrl::GetHeader)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    m_deleteCtrlFileReader->FillStatsFromControlHeaderV10();
    m_deleteCtrlFileReader->FillStatsFromControlHeaderV10();
    m_deleteCtrlFileReader->FillStatsFromControlHeaderV10();
}

TEST_F(DeleteControlFileReaderTest, ReadControlFileEntryAndProcess)
{
    Module::DeleteCtrlEntry deleteEntry {};
    deleteEntry.m_isDel = 1;
    deleteEntry.m_absPath = ".";
    std::string fileName {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    ParentInfo parentInfo {};
    parentInfo.dirName = ".";

    MOCKER_CPP(&DeleteControlFileReader::ReadControlFileEntry)
            .stubs()
            .will(returnValue(SKIP))
            .then(returnValue(SKIP))
            .then(returnValue(SKIP))
            .then(returnValue(FINISH));
    int ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcess(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, SKIP);

    deleteEntry.m_isDel = 0;
    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcess(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, SKIP);

    deleteEntry.m_absPath = "";
    fileName = "1.txt";
    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcess(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, SKIP);

    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcess(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(DeleteControlFileReaderTest, ReadControlFileEntryAndProcessV10)
{
    BackupDeleteCtrlEntry deleteEntry {};
    deleteEntry.m_isDel = 1;
    deleteEntry.m_absPath = ".";
    std::string fileName {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    ParentInfo parentInfo {};
    parentInfo.dirName = ".";

    std::string controlFile = "abc.txt";
    m_deleteCtrlFileReader->m_scannerDeleteCtrl = std::make_unique<BackupDeleteCtrl>(controlFile);
    MOCKER_CPP(&BackupDeleteCtrl::ReadEntry)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_READ_EOF));
    int ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcessV10(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, NAS_CTRL_FILE_RET_SUCCESS);

    deleteEntry.m_isDel = 0;
    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcessV10(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, SKIP);

    deleteEntry.m_absPath = "";
    fileName = "1.txt";
    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcessV10(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, NAS_CTRL_FILE_RET_SUCCESS);

    ret = m_deleteCtrlFileReader->ReadControlFileEntryAndProcessV10(deleteEntry, fileName,
        fileHandle, parentInfo);
    EXPECT_EQ(ret, FINISH);
}
