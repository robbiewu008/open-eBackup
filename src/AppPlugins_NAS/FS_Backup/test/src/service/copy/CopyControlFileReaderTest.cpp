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
#include "copy/CopyControlFileReader.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int DIR_SKIP = 1;
    const int FILE_SKIP = 2;
    const int FINISH = 3;
}

class CopyControlFileReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<CopyControlFileReader> m_copyCtrlFileReader = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue  = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<BackupQueue<FileHandle>> m_writeQueue      = make_shared<BackupQueue<FileHandle>>(config);
};

void CopyControlFileReaderTest::SetUp()
{
    m_backupParams.backupType = BackupType::RESTORE;
    m_backupParams.srcEngine = BackupIOEngine::LIBNFS;
    m_backupParams.dstEngine = BackupIOEngine::LIBNFS;

    LibnfsBackupAdvanceParams libnfsBackupAdvanceParams {};
    m_backupParams.srcAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.dstAdvParams = make_shared<LibnfsBackupAdvanceParams>(libnfsBackupAdvanceParams);
    m_backupParams.scanAdvParams.metaFilePath = "/home";

    CommonParams commonParams {};
    commonParams.metaPath = "/xx-dir/";
    commonParams.jobId = "qqqqqqqqqq";
    commonParams.subJobId = "wwwwwwwwwwwwwww";
    commonParams.restoreReplacePolicy = RestoreReplacePolicy::OVERWRITE;
    commonParams.backupDataFormat = BackupDataFormat::AGGREGATE;
    commonParams.writeAcl = true;
    commonParams.writeExtendAttribute = true;
    commonParams.writeSparseFile = true;
    commonParams.maxFileSizeToAggregate = 10;
    m_backupParams.commonParams = commonParams;

    m_copyCtrlFileReader = make_unique<CopyControlFileReader>(m_backupParams, m_readQueue, m_aggregateQueue,
        m_controlInfo, m_blockBufferMap);
}

void CopyControlFileReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void CopyControlFileReaderTest::SetUpTestCase()
{}

void CopyControlFileReaderTest::TearDownTestCase()
{}

static std::string ParseAcl_Stub()
{
    std::string val = "";
    return val;
}

static std::vector<pair<string, string>> ParseXattr_Stub()
{
    std::vector<pair<string, string>> xattrlist;
    return xattrlist;
}

static std::vector<pair<uint64_t, uint64_t>> ParseSparseInfo_Stub()
{
    std::vector<pair<uint64_t, uint64_t>> sparselist;
    return sparselist;
}

static std::string GetOpenedMetaFileName_Stub1()
{
    std::string val = "";
    return val;
}

static std::string GetOpenedMetaFileName_Stub2()
{
    std::string val = "/home/metaFile.txt";
    return val;
}

TEST_F(CopyControlFileReaderTest, Start)
{
    m_copyCtrlFileReader->m_abort = true;
    MOCKER_CPP(&CopyControlFileReader::ThreadFunc)
            .stubs()
            .will(ignoreReturnValue());
    //EXPECT_EQ(m_copyCtrlFileReader->Start(), BackupRetCode::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, Abort)
{
    EXPECT_EQ(m_copyCtrlFileReader->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, Enqueue)
{
    std::string ctrlFile = "abc.txt";
    EXPECT_EQ(m_copyCtrlFileReader->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, GetStatus)
{
    m_copyCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_copyCtrlFileReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_copyCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyCtrlFileReader->m_abort = true;
    EXPECT_EQ(m_copyCtrlFileReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_copyCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyCtrlFileReader->m_abort = false;
    m_copyCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_copyCtrlFileReader->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_copyCtrlFileReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_copyCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_copyCtrlFileReader->m_abort = false;
    m_copyCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_copyCtrlFileReader->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_copyCtrlFileReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(CopyControlFileReaderTest, IsAbort)
{
    m_copyCtrlFileReader->m_abort = false;
    m_copyCtrlFileReader->m_controlInfo->m_failed = false;
    m_copyCtrlFileReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_copyCtrlFileReader->IsAbort(), true);

    m_copyCtrlFileReader->m_abort = false;
    m_copyCtrlFileReader->m_controlInfo->m_failed = false;
    m_copyCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_copyCtrlFileReader->IsAbort(), false);
}

TEST_F(CopyControlFileReaderTest, IsComplete)
{
    m_copyCtrlFileReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_copyCtrlFileReader->m_controlInfo->m_skipFileCnt = 1;
    m_copyCtrlFileReader->m_controlInfo->m_noOfFilesToBackup = 1;
    m_copyCtrlFileReader->m_controlInfo->m_noOfDirToBackup = 1;
    EXPECT_EQ(m_copyCtrlFileReader->IsComplete(), true);

    m_copyCtrlFileReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_copyCtrlFileReader->m_controlInfo->m_skipFileCnt = 4;
    EXPECT_EQ(m_copyCtrlFileReader->IsComplete(), false);
}

TEST_F(CopyControlFileReaderTest, ThreadFunc)
{
    std::string ctrlFile = "abc.txt";
    MOCKER_CPP(&FSBackupUtils::CheckMetaFileVersion)
            .stubs()
            .will(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V20));
    MOCKER_CPP(&CopyControlFileReader::OpenControlFile)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    MOCKER_CPP(&CopyControlFileReader::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false));
    MOCKER_CPP(&CopyControlFileReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    m_copyCtrlFileReader->Enqueue(ctrlFile);
    m_copyCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&CopyControlFileReader::ReadControlFileEntryAndProcessV10)
            .stubs()
            .will(returnValue(FINISH));
    m_copyCtrlFileReader->Enqueue(ctrlFile);
    m_copyCtrlFileReader->ThreadFunc();

    MOCKER_CPP(&CopyControlFileReader::ReadControlFileEntryAndProcess)
            .stubs()
            .will(returnValue(Module::FAILED));
    m_copyCtrlFileReader->Enqueue(ctrlFile);
    m_copyCtrlFileReader->ThreadFunc();

    m_copyCtrlFileReader->Enqueue(ctrlFile);
    m_copyCtrlFileReader->ThreadFunc();
}

TEST_F(CopyControlFileReaderTest, ProcessControlFileEntry)
{
    Module::CopyCtrlDirEntry dirEntry {};
    Module::CopyCtrlFileEntry fileEntry {};
    ParentInfo parentInfo {};

    dirEntry.m_dirName = "d1";
    MOCKER_CPP(&CopyControlFileReader::ProcessDirEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    MOCKER_CPP(&CopyControlFileReader::PushFileHandleToReader)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS));
    int ret = m_copyCtrlFileReader->ProcessControlFileEntry(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_dirName = "";
    fileEntry.m_fileName = "1.txt";
    MOCKER_CPP(&CopyControlFileReader::ProcessFileEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    ret = m_copyCtrlFileReader->ProcessControlFileEntry(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_dirName = "";
    fileEntry.m_fileName = "";
    ret = m_copyCtrlFileReader->ProcessControlFileEntry(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, ProcessControlFileEntryV10)
{
    ScannerBackupCtrlDirEntry dirEntry {};
    ScannerBackupCtrlFileEntry fileEntry {};
    ParentInfo parentInfo {};

    dirEntry.m_dirName = "d1";
    MOCKER_CPP(&CopyControlFileReader::ProcessDirEntryV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    MOCKER_CPP(&CopyControlFileReader::PushFileHandleToReader)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS));
    int ret = m_copyCtrlFileReader->ProcessControlFileEntryV10(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_dirName = "";
    fileEntry.m_fileName = "1.txt";
    MOCKER_CPP(&CopyControlFileReader::ProcessFileEntryV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    ret = m_copyCtrlFileReader->ProcessControlFileEntryV10(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    dirEntry.m_dirName = "";
    fileEntry.m_fileName = "";
    ret = m_copyCtrlFileReader->ProcessControlFileEntryV10(dirEntry, fileEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, PushFileHandleToReader)
{
    int ret = DIR_SKIP;
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    EXPECT_EQ(m_copyCtrlFileReader->PushFileHandleToReader(ret, fileHandle), Module::SUCCESS);

    ret = FILE_SKIP;
    EXPECT_EQ(m_copyCtrlFileReader->PushFileHandleToReader(ret, fileHandle), Module::SUCCESS);

    ret = Module::FAILED;
    EXPECT_EQ(m_copyCtrlFileReader->PushFileHandleToReader(ret, fileHandle), Module::FAILED);

    ret = Module::SUCCESS;
    EXPECT_EQ(m_copyCtrlFileReader->PushFileHandleToReader(ret, fileHandle), Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, OpenControlFile)
{
    std::string ctrlFile = "abc.txt";
    m_copyCtrlFileReader->m_metaFileVersion = META_VERSION_V10;
    MOCKER_CPP(&CopyControlFileReader::OpenControlFileV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);

    m_copyCtrlFileReader->m_metaFileVersion = META_VERSION_V20;
    MOCKER_CPP(&CopyControlFileReader::OpenControlFileV20)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, OpenControlFileV10)
{
    std::string controlFile = "abc.txt";
    m_copyCtrlFileReader->m_scannerBackupCtrl = std::make_unique<ScannerBackupCtrl>(controlFile);
    MOCKER_CPP(&ScannerBackupCtrl::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    MOCKER_CPP(&CopyControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV10(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);

    m_copyCtrlFileReader->m_scannerBackupCtrl = nullptr;
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, OpenControlFileV20)
{
    std::string controlFile = "abc.txt";
    m_copyCtrlFileReader->m_copyCtrlParser = make_unique<Module::CopyCtrlParser>(controlFile);
    MOCKER_CPP(&Module::CopyCtrlParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    MOCKER_CPP(&CopyControlFileReader::FillStatsFromControlHeader)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV20(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);

    m_copyCtrlFileReader->m_copyCtrlParser = nullptr;
    EXPECT_EQ(m_copyCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, ReadControlFileEntryAndProcess)
{
    Module::CopyCtrlFileEntry fileEntry {};
    Module::CopyCtrlDirEntry dirEntry {};
    ParentInfo parentInfo {};

    std::string controlFile = "abc.txt";
    m_copyCtrlFileReader->m_copyCtrlParser = make_unique<Module::CopyCtrlParser>(controlFile);
    MOCKER_CPP(&Module::CopyCtrlParser::ReadEntry)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::READ_EOF));
    MOCKER_CPP(&CopyControlFileReader::ProcessControlFileEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    int ret = m_copyCtrlFileReader->ReadControlFileEntryAndProcess(fileEntry, dirEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    ret = m_copyCtrlFileReader->ReadControlFileEntryAndProcess(fileEntry, dirEntry, parentInfo);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(CopyControlFileReaderTest, ReadControlFileEntryAndProcessV10)
{
    ScannerBackupCtrlFileEntry fileEntry {};
    ScannerBackupCtrlDirEntry dirEntry {};
    ParentInfo parentInfo {};

    std::string controlFile = "abc.txt";
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_scannerBackupCtrl = std::make_unique<ScannerBackupCtrl>(controlFile);
    m_copyCtrlFileReader->m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    MOCKER_CPP(&ScannerBackupCtrl::ReadEntry)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_READ_EOF));
    MOCKER_CPP(&CopyControlFileReader::ProcessControlFileEntryV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    int ret = m_copyCtrlFileReader->ReadControlFileEntryAndProcessV10(fileEntry, dirEntry, parentInfo);
    EXPECT_EQ(ret, Module::SUCCESS);

    MOCKER_CPP(&ScannerBackupCtrl::Close)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS));
    MOCKER_CPP(&NasScanner::ScannerBackupMeta::Close)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS));
    ret = m_copyCtrlFileReader->ReadControlFileEntryAndProcessV10(fileEntry, dirEntry, parentInfo);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(CopyControlFileReaderTest, OpenMetaControlFile)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    MOCKER_CPP(&Module::MetaParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFile(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFile(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFile(metaFile), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, OpenMetaControlFileV10)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    MOCKER_CPP(&NasScanner::ScannerBackupMeta::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_copyCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, OpenXMetaControlFile)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_xMetaParser = make_unique<Module::XMetaParser>(metaFile);
    MOCKER_CPP(&Module::XMetaParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->OpenXMetaControlFile(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->OpenXMetaControlFile(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_copyCtrlFileReader->OpenXMetaControlFile(metaFile), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, ReadDirectoryMeta)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    Module::DirMeta dirMeta {};
    uint64_t offset = 0;
    MOCKER_CPP(&Module::MetaParser::ReadDirectoryMeta)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ReadDirectoryMeta(dirMeta, offset), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->ReadDirectoryMeta(dirMeta, offset), Module::FAILED);
}

/*TEST_F(CopyControlFileReaderTest, ReadFileMeta)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    Module::FileMeta fileMeta {};
    uint64_t offset = 0;
    MOCKER(&Module::MetaParser::ReadFileMeta, Module::CTRL_FILE_RETCODE(Module::FileMeta, uint64_t))
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ReadFileMeta(fileMeta, offset), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->ReadFileMeta(fileMeta, offset), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, ReadDirectoryXMeta)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_xMetaParser = make_unique<Module::XMetaParser>(metaFile);
    Module::DirMeta dirMeta {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    MOCKER_CPP(&Module::XMetaParser::ReadXMeta)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    MOCKER_CPP(&Module::ParserUtils::ParseAccessAcl)
            .stubs()
            .will(invoke(ParseAcl_Stub));
    MOCKER_CPP(&Module::ParserUtils::ParseDefaultAcl)
            .stubs()
            .will(invoke(ParseAcl_Stub));
    MOCKER_CPP(&Module::ParserUtils::ParseXattr)
            .stubs()
            .will(invoke(ParseXattr_Stub));
    MOCKER_CPP(&Module::ParserUtils::ParseSparseInfo)
            .stubs()
            .will(invoke(ParseSparseInfo_Stub));
    EXPECT_EQ(m_copyCtrlFileReader->ReadDirectoryXMeta(fileHandle, dirMeta), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->ReadDirectoryXMeta(fileHandle, dirMeta), Module::FAILED);
}*/

TEST_F(CopyControlFileReaderTest, GetMetaFile)
{
    std::string metaFileName = "metaFile.txt";
    m_copyCtrlFileReader->GetMetaFile(metaFileName);
}

TEST_F(CopyControlFileReaderTest, GetXMetaFile)
{
    uint64_t xMetaFileIndex = 4;
    m_copyCtrlFileReader->GetXMetaFile(xMetaFileIndex);
}

TEST_F(CopyControlFileReaderTest, GetOpenedMetaFileName)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    m_copyCtrlFileReader->GetOpenedMetaFileName();

    m_copyCtrlFileReader->m_metaParser = nullptr;
    m_copyCtrlFileReader->GetOpenedMetaFileName();
}

TEST_F(CopyControlFileReaderTest, GetOpenedMetaFileNameV10)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_scannerBackupMeta = make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    m_copyCtrlFileReader->GetOpenedMetaFileNameV10();

    m_copyCtrlFileReader->m_scannerBackupMeta = nullptr;
    m_copyCtrlFileReader->GetOpenedMetaFileNameV10();
}

TEST_F(CopyControlFileReaderTest, GetOpenedXMetaFileName)
{
    std::string metaFile = "metaFile.txt";
    m_copyCtrlFileReader->m_xMetaParser = make_unique<Module::XMetaParser>(metaFile);
    m_copyCtrlFileReader->GetOpenedXMetaFileName();

    m_copyCtrlFileReader->m_xMetaParser = nullptr;
    m_copyCtrlFileReader->GetOpenedXMetaFileName();
}

TEST_F(CopyControlFileReaderTest, ProcessDirEntry)
{
    ParentInfo parentInfo {};
    Module::CopyCtrlDirEntry dirEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = ".";

    dirEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_DELETED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntry(parentInfo, dirEntry, fileHandle), DIR_SKIP);

    dirEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_MODIFIED;
    MOCKER_CPP(&CopyControlFileReader::OpenMetaControlFile)
            .stubs()
            .will(returnValue(Module::FAILED))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntry(parentInfo, dirEntry, fileHandle), Module::FAILED);

    parentInfo.metaFileName = "abc.txt";
    dirEntry.m_metaFileName = "metaFile.txt";
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntry(parentInfo, dirEntry, fileHandle), Module::FAILED);

    parentInfo.metaFileName = "/home/metaFile.txt";
    dirEntry.m_metaFileName = "metaFile.txt";
    MOCKER_CPP(&CopyControlFileReader::ReadDirectoryMeta)
            .stubs()
            .will(returnValue(Module::FAILED))
            .then(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntry(parentInfo, dirEntry, fileHandle), Module::FAILED);

    MOCKER_CPP(&CopyControlFileReader::FillDirMetaData)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntry(parentInfo, dirEntry, fileHandle), Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, ProcessDirEntryV10)
{
    ParentInfo parentInfo {};
    ScannerBackupCtrlDirEntry dirEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = ".";

    dirEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_DELETED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), DIR_SKIP);

    dirEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_MODIFIED;
    MOCKER_CPP(&CopyControlFileReader::OpenMetaControlFileV10)
            .stubs()
            .will(returnValue(Module::FAILED))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), Module::FAILED);

    dirEntry.m_dirName = ".";
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), DIR_SKIP);

    dirEntry.m_dirName = "d1";
    parentInfo.metaFileName = "abc.txt";
    dirEntry.m_metaFileName = "metaFile.txt";
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), Module::FAILED);

    parentInfo.metaFileName = "/home/metaFile.txt";
    dirEntry.m_metaFileName = "metaFile.txt";
    MOCKER_CPP(&NasScanner::ScannerBackupMeta::ReadDirectoryMeta)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_FAILED))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), Module::FAILED);

    MOCKER_CPP(&CopyControlFileReader::FillDirMetaDataV10)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_copyCtrlFileReader->ProcessDirEntryV10(parentInfo, dirEntry, fileHandle), Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, ProcessFileEntry)
{
    ParentInfo parentInfo {};
    Module::CopyCtrlFileEntry fileEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    std::string metaFile = "abc.txt";
    m_copyCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);

    fileEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_DELETED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntry(parentInfo, fileEntry, fileHandle), FILE_SKIP);

    fileEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_MODIFIED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntry(parentInfo, fileEntry, fileHandle), Module::FAILED);

    fileEntry.m_metaFileName = "abc.txt";
    MOCKER_CPP(&CopyControlFileReader::GetOpenedMetaFileName)
            .stubs()
            .will(invoke(GetOpenedMetaFileName_Stub1))
            .then(invoke(GetOpenedMetaFileName_Stub2))
            .then(invoke(GetOpenedMetaFileName_Stub2));
    MOCKER_CPP(&CopyControlFileReader::OpenMetaControlFile)
            .stubs()
            .will(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntry(parentInfo, fileEntry, fileHandle), Module::FAILED);

    fileEntry.m_metaFileName = "metaFile.txt";
    MOCKER_CPP(&CopyControlFileReader::ReadFileMeta)
            .stubs()
            .will(returnValue(Module::FAILED))
            .then(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntry(parentInfo, fileEntry, fileHandle), Module::FAILED);

    MOCKER_CPP(&CopyControlFileReader::FillFileMetaData)
            .stubs()
            .will(ignoreReturnValue());
    fileHandle.m_file->m_mode = 16832;
    fileHandle.m_file->m_size = 0;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntry(parentInfo, fileEntry, fileHandle), Module::SUCCESS); 
}

TEST_F(CopyControlFileReaderTest, ProcessFileEntryV10)
{
    ParentInfo parentInfo {};
    ScannerBackupCtrlFileEntry fileEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    std::string metaFile = "abc.txt";
    m_copyCtrlFileReader->m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);

    fileEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_DELETED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntryV10(parentInfo, fileEntry, fileHandle), FILE_SKIP);

    fileEntry.m_mode = Module::CTRL_ENTRY_MODE_DATA_MODIFIED;
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntryV10(parentInfo, fileEntry, fileHandle), Module::FAILED);

    fileEntry.m_metaFileName = "abc.txt";
    MOCKER_CPP(&CopyControlFileReader::GetOpenedMetaFileNameV10)
            .stubs()
            .will(invoke(GetOpenedMetaFileName_Stub1))
            .then(invoke(GetOpenedMetaFileName_Stub2))
            .then(invoke(GetOpenedMetaFileName_Stub2));
    MOCKER_CPP(&CopyControlFileReader::OpenMetaControlFileV10)
            .stubs()
            .will(returnValue(Module::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntryV10(parentInfo, fileEntry, fileHandle), Module::FAILED);

    //fileEntry.m_metaFileName = "metaFile.txt";
    //MOCKER_CPP(&NasScanner::ScannerBackupMeta::ReadFileMeta,
    //    NAS_CTRL_FILE_RETCODE(NasScanner::ScannerBackupMeta::*)(NasScanner::FileMeta, uint16_t, uint16_t))
    //        .stubs()
    //        .will(returnValue(NAS_CTRL_FILE_RET_FAILED))
    //        .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS));
    //EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntryV10(parentInfo, fileEntry, fileHandle), Module::FAILED);
//
    //MOCKER_CPP(&CopyControlFileReader::FillFileMetaDataV10)
    //        .stubs()
    //        .will(ignoreReturnValue());
    //fileHandle.m_file->m_mode = 16832;
    //fileHandle.m_file->m_size = 0;
    //EXPECT_EQ(m_copyCtrlFileReader->ProcessFileEntryV10(parentInfo, fileEntry, fileHandle), Module::SUCCESS);
}

TEST_F(CopyControlFileReaderTest, FillDirMetaData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = ".";
    Module::DirMeta dirMeta {};
    fileHandle.m_file->m_aclText = "abc";

    m_copyCtrlFileReader->FillDirMetaData(fileHandle, dirMeta);
}

TEST_F(CopyControlFileReaderTest, FillDirMetaDataV10)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "./d1";
    NasScanner::DirectoryMeta dirMeta {};
    dirMeta.m_aclText = "abc";

    m_copyCtrlFileReader->FillDirMetaDataV10(fileHandle, dirMeta);
}

TEST_F(CopyControlFileReaderTest, FillFileMetaData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "./1.txt";
    Module::FileMeta fileMeta {};
    fileHandle.m_file->m_aclText = "abc";
    CopyCtrlFileEntry fileEntry;

    m_copyCtrlFileReader->FillFileMetaData(fileHandle, fileMeta, fileEntry);
}

TEST_F(CopyControlFileReaderTest, FillFileMetaDataV10)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "./1.txt";
    NasScanner::FileMeta fileMeta {};
    fileMeta.m_aclText = "abc";
    fileMeta.m_fh.len = 1;

    m_copyCtrlFileReader->FillFileMetaDataV10(fileHandle, fileMeta);
}

TEST_F(CopyControlFileReaderTest, FillStatsFromControlHeader)
{
    std::string controlFile = "abc.txt";
    m_copyCtrlFileReader->m_copyCtrlParser = make_unique<Module::CopyCtrlParser>(controlFile);
    MOCKER_CPP(&Module::CopyCtrlParser::GetHeader)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeader(), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);

    m_copyCtrlFileReader->m_copyCtrlParser = nullptr;
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);
}

TEST_F(CopyControlFileReaderTest, FillStatsFromControlHeaderV10)
{
    std::string controlFile = "abc.txt";
    m_copyCtrlFileReader->m_scannerBackupCtrl = std::make_unique<ScannerBackupCtrl>(controlFile);
    MOCKER_CPP(&ScannerBackupCtrl::GetHeader)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeaderV10(), Module::SUCCESS);
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);

    m_copyCtrlFileReader->m_scannerBackupCtrl = nullptr;
    EXPECT_EQ(m_copyCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);
}
