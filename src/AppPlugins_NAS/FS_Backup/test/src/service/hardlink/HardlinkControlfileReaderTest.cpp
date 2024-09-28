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
#include "hardlink/HardlinkControlFileReader.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int SKIP = 1;
    const int FINISH = 2;
}

class HardlinkControlFileReaderTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    BackupParams m_backupParams {};
    unique_ptr<HardlinkControlFileReader> m_hardlinkCtrlFileReader = nullptr;

    BackupQueueConfig config {DEFAULT_BACKUP_QUEUE_SIZE, DEFAULT_BACKUP_QUEUE_MEMORY_SIZE};
    shared_ptr<BackupControlInfo> m_controlInfo           = make_shared<BackupControlInfo>();
    shared_ptr<BlockBufferMap> m_blockBufferMap           = make_shared<BlockBufferMap>();
    shared_ptr<BackupQueue<FileHandle>> m_readQueue       = make_shared<BackupQueue<FileHandle>>(config);
    shared_ptr<HardLinkMap> m_hardlinkMap                 = make_shared<HardLinkMap>();
};

void HardlinkControlFileReaderTest::SetUp()
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

    m_hardlinkCtrlFileReader = make_unique<HardlinkControlFileReader>(m_backupParams, m_readQueue,
        m_controlInfo, m_blockBufferMap, m_hardlinkMap);
}

void HardlinkControlFileReaderTest::TearDown()
{
    GlobalMockObject::verify(); // 校验mock规范并清除mock规范
}

void HardlinkControlFileReaderTest::SetUpTestCase()
{}

void HardlinkControlFileReaderTest::TearDownTestCase()
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

TEST_F(HardlinkControlFileReaderTest, Abort)
{
    EXPECT_EQ(m_hardlinkCtrlFileReader->Abort(), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkControlFileReaderTest, Enqueue)
{
    std::string ctrlFile = "abc.txt";
    EXPECT_EQ(m_hardlinkCtrlFileReader->Enqueue(ctrlFile), BackupRetCode::SUCCESS);
}

TEST_F(HardlinkControlFileReaderTest, GetStatus)
{
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = false;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetStatus(), BackupPhaseStatus::INPROGRESS);

    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_hardlinkCtrlFileReader->m_abort = true;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetStatus(), BackupPhaseStatus::ABORTED);

    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_hardlinkCtrlFileReader->m_abort = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_failed = true;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetStatus(), BackupPhaseStatus::FAILED);

    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderPhaseComplete = true;
    m_hardlinkCtrlFileReader->m_abort = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_failed = false;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetStatus(), BackupPhaseStatus::COMPLETED);
}

TEST_F(HardlinkControlFileReaderTest, IsAbort)
{
    m_hardlinkCtrlFileReader->m_abort = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_failed = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderFailed = true;
    EXPECT_EQ(m_hardlinkCtrlFileReader->IsAbort(), true);

    m_hardlinkCtrlFileReader->m_abort = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_failed = false;
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderFailed = false;
    EXPECT_EQ(m_hardlinkCtrlFileReader->IsAbort(), false);
}

TEST_F(HardlinkControlFileReaderTest, IsComplete)
{
    m_hardlinkCtrlFileReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_hardlinkCtrlFileReader->m_controlInfo->m_noOfFilesToBackup = 1;
    EXPECT_EQ(m_hardlinkCtrlFileReader->IsComplete(), true);

    m_hardlinkCtrlFileReader->m_controlInfo->m_controlFileReaderProduce = 1;
    m_hardlinkCtrlFileReader->m_controlInfo->m_noOfFilesToBackup = 4;
    EXPECT_EQ(m_hardlinkCtrlFileReader->IsComplete(), false);
}

TEST_F(HardlinkControlFileReaderTest, ThreadFunc)
{
    std::string ctrlFile = "abc.txt";
    MOCKER_CPP(&FSBackupUtils::CheckMetaFileVersion)
            .stubs()
            .will(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V10))
            .then(returnValue(META_VERSION_V20))
            .then(returnValue(META_VERSION_V20));
    MOCKER_CPP(&HardlinkControlFileReader::OpenControlFile)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    MOCKER_CPP(&HardlinkControlFileReader::IsAbort)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(false))
            .then(returnValue(true))
            .then(returnValue(true));
    MOCKER_CPP(&HardlinkControlFileReader::IsComplete)
            .stubs()
            .will(returnValue(true))
            .then(returnValue(false))
            .then(returnValue(false));
    m_hardlinkCtrlFileReader->Enqueue(ctrlFile);
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->ThreadFunc());

    MOCKER_CPP(&HardlinkControlFileReader::ReadControlFileEntryAndProcessV10)
            .stubs()
            .will(returnValue(Module::FAILED));
    m_hardlinkCtrlFileReader->Enqueue(ctrlFile);
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->ThreadFunc());

    MOCKER_CPP(&HardlinkControlFileReader::ReadControlFileEntryAndProcess)
            .stubs()
            .will(returnValue(SKIP))
            .then(returnValue(Module::SUCCESS));
    m_hardlinkCtrlFileReader->Enqueue(ctrlFile);
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->ThreadFunc());

    m_hardlinkCtrlFileReader->Enqueue(ctrlFile);
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->ThreadFunc());

    m_hardlinkCtrlFileReader->Enqueue(ctrlFile);
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->ThreadFunc());
    EXPECT_TRUE(m_hardlinkCtrlFileReader->m_controlInfo->m_controlReaderFailed);
}

TEST_F(HardlinkControlFileReaderTest, ReadControlFileEntryAndProcess)
{
    Module::HardlinkCtrlEntry linkEntry {};
    Module::HardlinkCtrlInodeEntry inodeEntry {};
    ParentInfo parentInfo {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    MOCKER_CPP(&HardlinkControlFileReader::ReadControlFileEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(FINISH));
    inodeEntry.inode = 1;
    int ret =
        m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcess(linkEntry, inodeEntry, parentInfo, fileHandle);
    EXPECT_EQ(ret, SKIP);

    inodeEntry.inode = 0;
    linkEntry.dirName = "d1";
    linkEntry.fileName = "1.txt";
    MOCKER_CPP(&HardlinkControlFileReader::ProcessFileEntry)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcess(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);

    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcess(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, Module::FAILED);

    linkEntry.dirName = "";
    linkEntry.fileName = "";
    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcess(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, SKIP);

    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcess(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(HardlinkControlFileReaderTest, ReadControlFileEntryAndProcessV10)
{
    ScannerHardLinkCtrl::LinkEntry linkEntry {};
    ScannerHardLinkCtrl::InodeEntry inodeEntry {};
    ParentInfo parentInfo {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    std::string controlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_scannerHardLinkCtrl =
        std::make_unique<ScannerHardLinkCtrl::CtrlFile>(controlFile);

    MOCKER_CPP(&ScannerHardLinkCtrl::CtrlFile::ReadEntry)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_READ_EOF));
    inodeEntry.inode = 1;
    int ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, SKIP);

    inodeEntry.inode = 0;
    linkEntry.dirName = "d1";
    linkEntry.fileName = "1.txt";
    MOCKER_CPP(&HardlinkControlFileReader::ProcessFileEntryV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, Module::SUCCESS);

    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, Module::FAILED);

    linkEntry.dirName = "";
    linkEntry.fileName = "";
    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, SKIP);

    ret = m_hardlinkCtrlFileReader->ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry,
        parentInfo, fileHandle);
    EXPECT_EQ(ret, FINISH);
}

TEST_F(HardlinkControlFileReaderTest, OpenControlFile)
{
    std::string ctrlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_metaFileVersion = META_VERSION_V10;
    MOCKER_CPP(&HardlinkControlFileReader::OpenControlFileV10)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);

    m_hardlinkCtrlFileReader->m_metaFileVersion = META_VERSION_V20;
    MOCKER_CPP(&HardlinkControlFileReader::OpenControlFileV20)
            .stubs()
            .will(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFile(ctrlFile), Module::SUCCESS);
}

TEST_F(HardlinkControlFileReaderTest, OpenControlFileV10)
{
    std::string controlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_scannerHardLinkCtrl =
        std::make_unique<ScannerHardLinkCtrl::CtrlFile>(controlFile);
    MOCKER_CPP(&ScannerHardLinkCtrl::CtrlFile::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    MOCKER_CPP(&HardlinkControlFileReader::FillStatsFromControlHeaderV10)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV10(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);

    m_hardlinkCtrlFileReader->m_scannerHardLinkCtrl = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV10(controlFile), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, OpenControlFileV20)
{
    std::string controlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_hardlinkCtrlParser = make_unique<Module::HardlinkCtrlParser>(controlFile);
    MOCKER_CPP(&Module::HardlinkCtrlParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    MOCKER_CPP(&HardlinkControlFileReader::FillStatsFromControlHeader)
            .stubs()
            .will(returnValue(Module::SUCCESS))
            .then(returnValue(Module::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV20(controlFile), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);

    m_hardlinkCtrlFileReader->m_hardlinkCtrlParser = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenControlFileV20(controlFile), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, ReadControlFileEntry)
{
    Module::HardlinkCtrlEntry linkEntry {};
    Module::HardlinkCtrlInodeEntry inodeEntry {};
    std::string controlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_hardlinkCtrlParser = make_unique<Module::HardlinkCtrlParser>(controlFile);
    MOCKER_CPP(&Module::HardlinkCtrlParser::ReadEntry)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::READ_EOF));
    EXPECT_EQ(m_hardlinkCtrlFileReader->ReadControlFileEntry(linkEntry, inodeEntry), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->ReadControlFileEntry(linkEntry, inodeEntry), FINISH);
}

TEST_F(HardlinkControlFileReaderTest, OpenMetaControlFile)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    MOCKER_CPP(&Module::MetaParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFile(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFile(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFile(metaFile), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, OpenMetaControlFileV10)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    MOCKER_CPP(&NasScanner::ScannerBackupMeta::Open)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenMetaControlFileV10(metaFile), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, OpenXMetaControlFile)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_xMetaParser = make_unique<Module::XMetaParser>(metaFile);
    MOCKER_CPP(&Module::XMetaParser::Open)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenXMetaControlFile(metaFile), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenXMetaControlFile(metaFile), Module::FAILED);

    metaFile = "";
    EXPECT_EQ(m_hardlinkCtrlFileReader->OpenXMetaControlFile(metaFile), Module::FAILED);
}

/*TEST_F(HardlinkControlFileReaderTest, ReadFileMeta)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    Module::FileMeta fileMeta {};
    uint64_t offset = 0;
    MOCKER_CPP(&Module::MetaParser::ReadFileMeta,
        Module::CTRL_FILE_RETCODE(Module::MetaParser::*)(Module::FileMeta, uint64_t))
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->ReadFileMeta(fileMeta, offset), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->ReadFileMeta(fileMeta, offset), Module::FAILED);
}*/

TEST_F(HardlinkControlFileReaderTest, GetMetaFile)
{
    std::string metaFileName = "metaFile.txt";
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->GetMetaFile(metaFileName));
}

TEST_F(HardlinkControlFileReaderTest, GetXMetaFile)
{
    uint64_t xMetaFileIndex = 4;
    EXPECT_NO_THROW(m_hardlinkCtrlFileReader->GetXMetaFile(xMetaFileIndex));
}

TEST_F(HardlinkControlFileReaderTest, GetOpenedMetaFileName)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedMetaFileName(), metaFile);

    m_hardlinkCtrlFileReader->m_metaParser = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedMetaFileName(), "");
}

TEST_F(HardlinkControlFileReaderTest, GetOpenedMetaFileNameV10)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_scannerBackupMeta = make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedMetaFileNameV10(), metaFile);

    m_hardlinkCtrlFileReader->m_scannerBackupMeta = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedMetaFileNameV10(), "");
}

TEST_F(HardlinkControlFileReaderTest, GetOpenedXMetaFileName)
{
    std::string metaFile = "metaFile.txt";
    m_hardlinkCtrlFileReader->m_xMetaParser = make_unique<Module::XMetaParser>(metaFile);
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedXMetaFileName(), metaFile);

    m_hardlinkCtrlFileReader->m_xMetaParser = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->GetOpenedXMetaFileName(), "");
}

TEST_F(HardlinkControlFileReaderTest, ProcessFileEntry)
{
    ParentInfo parentInfo {};
    Module::HardlinkCtrlEntry linkEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    linkEntry.metaFileName = "";
    std::string metaFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_metaParser = make_unique<Module::MetaParser>(metaFile);

    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntry(parentInfo, linkEntry, fileHandle), Module::FAILED);

    linkEntry.metaFileName = "metaFile.txt";
    MOCKER_CPP(&HardlinkControlFileReader::GetOpenedMetaFileName)
            .stubs()
            .will(invoke(GetOpenedMetaFileName_Stub1))
            .then(invoke(GetOpenedMetaFileName_Stub2))
            .then(invoke(GetOpenedMetaFileName_Stub2))
            .then(invoke(GetOpenedMetaFileName_Stub2));
    MOCKER_CPP(&HardlinkControlFileReader::OpenMetaControlFile)
            .stubs()
            .will(returnValue(Module::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntry(parentInfo, linkEntry, fileHandle), Module::FAILED);

    MOCKER_CPP(&HardlinkControlFileReader::ReadFileMeta)
            .stubs()
            .will(returnValue(Module::FAILED))
            .then(returnValue(Module::SUCCESS))
            .then(returnValue(Module::SUCCESS));
    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntry(parentInfo, linkEntry, fileHandle), Module::FAILED);

    MOCKER_CPP(&HardlinkControlFileReader::FillFileMetaData)
            .stubs()
            .will(ignoreReturnValue())
            .then(ignoreReturnValue());
    MOCKER_CPP(&HardLinkMap::Exist)
            .stubs()
            .will(returnValue(false))
            .then(returnValue(true));
    MOCKER_CPP(&HardLinkMap::Insert)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntry(parentInfo, linkEntry, fileHandle), Module::SUCCESS);

    MOCKER_CPP(&HardLinkMap::InsertLinks)
            .stubs()
            .will(ignoreReturnValue());
    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntry(parentInfo, linkEntry, fileHandle), Module::SUCCESS);
}

TEST_F(HardlinkControlFileReaderTest, ProcessFileEntryV10)
{
    ParentInfo parentInfo {};
    ScannerHardLinkCtrl::LinkEntry linkEntry {};
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    linkEntry.metaFileName = "";
    std::string metaFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);

    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntryV10(parentInfo, linkEntry, fileHandle), Module::FAILED);

    linkEntry.metaFileName = "metaFile.txt";
    MOCKER_CPP(&HardlinkControlFileReader::GetOpenedMetaFileNameV10)
            .stubs()
            .will(invoke(GetOpenedMetaFileName_Stub1));
    MOCKER_CPP(&HardlinkControlFileReader::OpenMetaControlFileV10)
            .stubs()
            .will(returnValue(Module::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->ProcessFileEntryV10(parentInfo, linkEntry, fileHandle), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, FillFileMetaData)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "./1.txt";
    Module::FileMeta fileMeta {};
    fileMeta.m_size = 100;
    fileHandle.m_file->m_aclText = "abc";

    m_hardlinkCtrlFileReader->FillFileMetaData(fileHandle, fileMeta);
    EXPECT_EQ(fileHandle.m_file->m_size, fileMeta.m_size);
}

TEST_F(HardlinkControlFileReaderTest, FillFileMetaDataV10)
{
    FileHandle fileHandle {};
    fileHandle.m_file = std::make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_fileName = "./1.txt";
    NasScanner::FileMeta fileMeta {};
    fileHandle.m_file->m_aclText = "abc";
    fileMeta.m_fh.len = 1;

    EXPECT_EQ(m_hardlinkCtrlFileReader->FillFileMetaDataV10(fileHandle, fileMeta), Module::SUCCESS);

    fileMeta.m_fh.len = 0;
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillFileMetaDataV10(fileHandle, fileMeta), Module::SUCCESS);
}

TEST_F(HardlinkControlFileReaderTest, FillStatsFromControlHeader)
{
    std::string controlFile = "abc.txt";
    m_hardlinkCtrlFileReader->m_hardlinkCtrlParser = make_unique<Module::HardlinkCtrlParser>(controlFile);
    MOCKER_CPP(&Module::HardlinkCtrlParser::GetHeader)
            .stubs()
            .will(returnValue(Module::CTRL_FILE_RETCODE::SUCCESS))
            .then(returnValue(Module::CTRL_FILE_RETCODE::FAILED));
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeader(), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);

    m_hardlinkCtrlFileReader->m_hardlinkCtrlParser = nullptr;
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeader(), Module::FAILED);
}

TEST_F(HardlinkControlFileReaderTest, FillStatsFromControlHeaderV10)
{
    std::string controlFile = "abc.txt";
    MOCKER_CPP(&ScannerHardLinkCtrl::CtrlFile::GetHeader)
            .stubs()
            .will(returnValue(NAS_CTRL_FILE_RET_SUCCESS))
            .then(returnValue(NAS_CTRL_FILE_RET_FAILED));

    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeaderV10(), Module::SUCCESS);
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);

    m_hardlinkCtrlFileReader->m_scannerHardLinkCtrl =
        std::make_unique<ScannerHardLinkCtrl::CtrlFile>(controlFile);
    EXPECT_EQ(m_hardlinkCtrlFileReader->FillStatsFromControlHeaderV10(), Module::FAILED);
}
