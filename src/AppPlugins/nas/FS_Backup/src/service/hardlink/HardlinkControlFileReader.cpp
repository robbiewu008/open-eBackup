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
#include "HardlinkControlFileReader.h"
#include "log/Log.h"
#include "ParserUtils.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
#ifdef WIN32
    const string PATH_SEP = "\\";
#else
    const string PATH_SEP = "/";
#endif
const std::string CTRL_FILE_PATH_SEPARATOR = "/"; /* Control file and XMeta always using slash as path separator */
const int SKIP = 1;
const int FINISH = 2;
const int NUMBER_ONE = 1;
const int QUEUE_TIMEOUT_MILLISECOND = 200;
const string XMETA_FILENAME_PFX = "xmeta_file_";
const int XMETA_FILENAME_LEN = 1024;
}

HardlinkControlFileReader::HardlinkControlFileReader(const ReaderParams& readerParams)
    : m_backupParams(readerParams.backupParams),
    m_readQueue(readerParams.readQueuePtr),
    m_controlInfo(readerParams.controlInfo),
    m_blockBufferMap(readerParams.blockBufferMap),
    m_hardlinkMap(readerParams.hardlinkMap)
{}
 
HardlinkControlFileReader::HardlinkControlFileReader(BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap,
    shared_ptr<HardLinkMap> hardlinkMap)
    : m_backupParams(backupParams),
      m_readQueue(readQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap),
      m_hardlinkMap(hardlinkMap)
{
}

HardlinkControlFileReader::~HardlinkControlFileReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/* Public APIs */
BackupRetCode HardlinkControlFileReader::Start()
{
    INFOLOG("HardlinkControlFileReader start!");
    try {
        m_thread = std::thread(&HardlinkControlFileReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    m_controlInfo->m_startTime = Module::ParserUtils::GetCurrentTimeInSeconds();
    return BackupRetCode::SUCCESS;
}

BackupRetCode HardlinkControlFileReader::Abort()
{
    INFOLOG("HardlinkControlFileReader abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HardlinkControlFileReader::Enqueue(string contrlFile)
{
    INFOLOG("HardlinkControlFileReader enqueue: %s", contrlFile.c_str());
    m_controlFileQueue.push(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus HardlinkControlFileReader::GetStatus()
{
    return FSBackupUtils::GetControlFileReaderStatus(m_controlInfo, m_abort);
}

bool HardlinkControlFileReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

bool HardlinkControlFileReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("HardlinkContorlFileReader check is complete: controlFileReaderProduce %d total %d",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_noOfFilesToBackup.load());
    }
    if (m_controlInfo->m_controlFileReaderProduce == m_controlInfo->m_noOfFilesToBackup) {
        INFOLOG("HardlinkContorlFileReader complete: controlFileReaderProduce %d total %d",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_noOfFilesToBackup.load());
        return true;
    }
    return false;
}

void HardlinkControlFileReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    string controlFile;
    controlFile = m_controlFileQueue.front();
    m_controlFileQueue.pop();
    m_metaFileVersion = FSBackupUtils::CheckMetaFileVersion(m_backupParams.scanAdvParams.metaFilePath);
    INFOLOG("HardlinkControlFileReader start, get control file from control file queue: %s", controlFile.c_str());
    if (OpenControlFile(controlFile) != Module::SUCCESS) {
        m_controlInfo->m_controlReaderFailed = true;
        m_controlInfo->m_controlReaderPhaseComplete = true;
        ERRLOG("HardlinkControlFileReader main thread end after failure!");
        return;
    }
    ParentInfo parentInfo;
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        FileHandle fileHandle;
        fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
        int ret;
        if (m_metaFileVersion == META_VERSION_V10) {
            ScannerHardLinkCtrl::LinkEntry linkEntry {};
            ScannerHardLinkCtrl::InodeEntry inodeEntry {};
            ret = ReadControlFileEntryAndProcessV10(linkEntry, inodeEntry, parentInfo, fileHandle);
        } else {
            HardlinkCtrlEntry linkEntry;
            HardlinkCtrlInodeEntry inodeEntry;
            ret = ReadControlFileEntryAndProcess(linkEntry, inodeEntry, parentInfo, fileHandle);
        }

        if (ret == Module::FAILED) {
            break;
        }
        if (ret == SKIP) {
            continue;
        }
        while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                break;
            }
        }
        ++m_controlInfo->m_controlFileReaderProduce;
    }
    m_controlInfo->m_controlReaderPhaseComplete = true;
    INFOLOG("HardlinkControlFileReader main thread end!");
    return;
}

int HardlinkControlFileReader::ReadControlFileEntryAndProcess(HardlinkCtrlEntry& linkEntry,
    HardlinkCtrlInodeEntry& inodeEntry, ParentInfo& parentInfo, FileHandle& fileHandle)
{
    int ret = ReadControlFileEntry(linkEntry, inodeEntry);
    if (ret == FINISH) {
        return FINISH;
    }
    if (inodeEntry.inode != 0) {
        parentInfo.nlink = inodeEntry.linkCount;
        return SKIP;
    } else if (!linkEntry.dirName.empty() && !linkEntry.fileName.empty()) {
        ret = ProcessFileEntry(parentInfo, linkEntry, fileHandle);
        if (ret == Module::FAILED) {
            return Module::FAILED;
        }
        return Module::SUCCESS;
    }
    return SKIP;
}

int HardlinkControlFileReader::ReadControlFileEntryAndProcessV10(ScannerHardLinkCtrl::LinkEntry& linkEntry,
    ScannerHardLinkCtrl::InodeEntry& inodeEntry, ParentInfo& parentInfo, FileHandle& fileHandle)
{
    NAS_CTRL_FILE_RETCODE ret = m_scannerHardLinkCtrl->ReadEntry(linkEntry, inodeEntry);
    if (ret == NAS_CTRL_FILE_RET_READ_EOF) {
        return FINISH;
    }
    if (inodeEntry.inode != 0) {
        parentInfo.nlink = inodeEntry.linkCount;
        return SKIP;
    } else if (!linkEntry.dirName.empty() && !linkEntry.fileName.empty()) {
        int ret1 = ProcessFileEntryV10(parentInfo, linkEntry, fileHandle);
        DBGLOG("ProcessFileEntry: %s, %s, %d", linkEntry.dirName.c_str(), linkEntry.fileName.c_str(),
            ret1);
        if (ret1 == Module::FAILED) {
            return Module::FAILED;
        }
        return Module::SUCCESS;
    }
    return SKIP;
}

int HardlinkControlFileReader::OpenControlFile(const string& controlFile)
{
    if (m_metaFileVersion == META_VERSION_V10) {
        return OpenControlFileV10(controlFile);
    }
    return OpenControlFileV20(controlFile);
}

int HardlinkControlFileReader::OpenControlFileV10(const string& controlFile)
{
    m_scannerHardLinkCtrl = std::make_unique<ScannerHardLinkCtrl::CtrlFile>(controlFile);
    if (!m_scannerHardLinkCtrl) {
        ERRLOG("create backup hardlink control instance failed!");
        return Module::FAILED;
    }
    int ret = m_scannerHardLinkCtrl->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("Open control file failed!");
        return Module::FAILED;
    }

    if (FillStatsFromControlHeaderV10() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from hardlink control file : %s", controlFile.c_str());
        return Module::FAILED;
    }
    INFOLOG("FillStatsFromControlHeaderV10 success");
    return Module::SUCCESS;
}

int HardlinkControlFileReader::OpenControlFileV20(const string& controlFile)
{
    m_hardlinkCtrlParser = std::make_unique<HardlinkCtrlParser>(controlFile);
    if (m_hardlinkCtrlParser == nullptr) {
        return Module::FAILED;
    }

    CTRL_FILE_RETCODE ret = m_hardlinkCtrlParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("failed to open hardlink control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    if (FillStatsFromControlHeader() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from hardlink control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int HardlinkControlFileReader::ReadControlFileEntry(
    HardlinkCtrlEntry& linkEntry, HardlinkCtrlInodeEntry& inodeEntry)
{
    CTRL_FILE_RETCODE ret = m_hardlinkCtrlParser->ReadEntry(linkEntry, inodeEntry);
    if (ret == CTRL_FILE_RETCODE::READ_EOF) {
        return FINISH;
    }
    return Module::SUCCESS;
}

int HardlinkControlFileReader::OpenMetaControlFile(string metaFile)
{
    if (m_backupParams.backupType == BackupType::RESTORE) {
        m_metaParser = std::make_unique<MetaParser>(metaFile);
    } else {
        m_metaParser = std::make_unique<MetaParser>(metaFile, true);
    }
    if (m_metaParser == nullptr) {
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE retCode = m_metaParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (retCode != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HardlinkControlFileReader::OpenMetaControlFileV10(const string& metaFile)
{
    m_scannerBackupMeta = std::make_unique<NasScanner::ScannerBackupMeta>(metaFile);
    if (m_scannerBackupMeta == nullptr) {
        ERRLOG("create backup meta control instance failed!");
        return Module::FAILED;
    }
    int ret = m_scannerBackupMeta->Open(NAS_CTRL_FILE_OPEN_MODE_READ);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        ERRLOG("Open meta file failed!");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HardlinkControlFileReader::OpenXMetaControlFile(string metaFile)
{
    m_xMetaParser = std::make_unique<XMetaParser>(metaFile);
    if (m_xMetaParser == nullptr) {
        ERRLOG("m_xMetaParser nulptr");
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE ret = m_xMetaParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("m_xMetaParser Open failed. Ret: %d: ", ret);
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HardlinkControlFileReader::ReadFileMeta(FileMeta& fileMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = m_metaParser->ReadFileMeta(fileMeta, offset);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int HardlinkControlFileReader::ReadFileXMeta(FileHandle& fileHandle, const FileMeta& fileMeta)
{
    vector<XMetaField> entryList {};
    if (m_xMetaParser->ReadXMeta(entryList, fileMeta.m_xMetaFileOffset) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Failed to read xmeta file");
        return Module::FAILED;
    }
#ifdef _NAS
    for (auto entry : entryList) {
        if (entry.m_xMetaType == XMETA_TYPE::XMETA_TYPE_NFSFH && m_backupParams.scanAdvParams.useXmetaFileHandle) {
            int ret = FSBackupUtils::SetSrcFileHandleForNfs(fileHandle, entry.m_value.length(),
                entry.m_value.c_str());
            if (ret != Module::SUCCESS) {
                return Module::FAILED;
            }
        }
    }
#endif
    if (m_backupParams.commonParams.writeAcl) {
        /* For Linux/NFS */
        fileHandle.m_file->m_aclText = ParserUtils::ParseAccessAcl(entryList);
        /* For Windows/SMB */
        fileHandle.m_file->m_securityDescriptor = ParserUtils::ParseSecurityDescriptor(entryList);
    }
    if (m_backupParams.commonParams.writeExtendAttribute) {
        fileHandle.m_file->m_xattr   = ParserUtils::ParseXattr(entryList);
    }
    if (m_backupParams.commonParams.writeSparseFile) {
        fileHandle.m_file->m_sparse  = ParserUtils::ParseSparseInfo(entryList);
    }

#ifdef WIN32
    /* reuse the m_attr to store some XMeta pair for Windows Backup. These XMeta will only used for file */
    std::string win32SymlinkTarget = ParserUtils::ParseSymbolicLinkTargetPath(entryList);
    if (!win32SymlinkTarget.empty()) {
        fileHandle.m_file->m_xattr.emplace_back(EXTEND_ATTR_KEY_WIN32_SYMBOLIC_TARGET, win32SymlinkTarget);
    }
    std::string win32JunctionTarget = ParserUtils::ParseJunctionPointTargetPath(entryList);
    if (!win32JunctionTarget.empty()) {
        fileHandle.m_file->m_xattr.emplace_back(EXTEND_ATTR_KEY_WIN32_JUNCTION_TARGET, win32JunctionTarget);
    }
#endif
    return Module::SUCCESS;
}

string HardlinkControlFileReader::GetMetaFile(string metaFileName)
{
    return m_backupParams.scanAdvParams.metaFilePath + PATH_SEP + metaFileName;
}

string HardlinkControlFileReader::GetXMetaFile(uint64_t xMetaFileIndex)
{
    char xMetaFile[XMETA_FILENAME_LEN] = {0};
    int iRet =  snprintf_s(xMetaFile, XMETA_FILENAME_LEN, XMETA_FILENAME_LEN - 1, "%s%s%s%lu",
        m_backupParams.scanAdvParams.metaFilePath.c_str(), PATH_SEP.c_str(),
        XMETA_FILENAME_PFX.c_str(), xMetaFileIndex);
    if (iRet <= Module::SUCCESS) {
        return "";
    }
    return xMetaFile;
}

string HardlinkControlFileReader::GetOpenedMetaFileName() const
{
    if (m_metaParser == nullptr) {
        return "";
    }
    return m_metaParser->GetFileName();
}

string HardlinkControlFileReader::GetOpenedMetaFileNameV10() const
{
    if (m_scannerBackupMeta == nullptr) {
        return "";
    }
    return m_scannerBackupMeta->GetFileName();
}

string HardlinkControlFileReader::GetOpenedXMetaFileName() const
{
    if (m_xMetaParser == nullptr) {
        return "";
    }
    return m_xMetaParser->GetFileName();
}

int HardlinkControlFileReader::ProcessFileEntry(
    ParentInfo& parentInfo, const HardlinkCtrlEntry& linkEntry, FileHandle& fileHandle)
{
    if (linkEntry.metaFileName.empty()) {
        ERRLOG("meta file is empty");
        return Module::FAILED;
    }
    if (GetOpenedMetaFileName() != GetMetaFile(linkEntry.metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(linkEntry.metaFileName);
        if (OpenMetaControlFile(parentInfo.metaFileName) != Module::SUCCESS) {
            ERRLOG("open meta file failed!");
            return Module::FAILED;
        }
    }

    FileMeta fileMeta;
    if (ReadFileMeta(fileMeta, linkEntry.metaFileOffset) != Module::SUCCESS) {
        ERRLOG("read file meta failed!");
        return Module::FAILED;
    }

    fileHandle.m_file->m_fileName = linkEntry.dirName + CTRL_FILE_PATH_SEPARATOR  + linkEntry.fileName;
    fileHandle.m_file->m_dirName = linkEntry.dirName;
    fileHandle.m_file->m_fileCount = linkEntry.m_hardLinkFilesCnt;
    DBGLOG("Enter ProcessFileEntry: %s", fileHandle.m_file->m_fileName.c_str());
    FillFileMetaData(fileHandle, fileMeta);

    if (fileMeta.m_xMetaFileOffset != 0) {
        string xMetafilename = GetXMetaFile(fileMeta.m_xMetaFileIndex);
        if (GetOpenedXMetaFileName() != xMetafilename) {
            if (OpenXMetaControlFile(xMetafilename) != Module::SUCCESS) {
                ERRLOG("Failed to open xmeta file");
                return Module::FAILED;
            }
        }

        /* Get fh from XMetaFile */
        if (ReadFileXMeta(fileHandle, fileMeta) != Module::SUCCESS) {
            return Module::FAILED;
        }
    }

    ProcessHardlink(parentInfo, fileHandle);

    return Module::SUCCESS;
}

void HardlinkControlFileReader::ProcessHardlink(ParentInfo& parentInfo, FileHandle& fileHandle)
{
    DBGLOG("Enter ProcessHardlink!");
    if (!m_hardlinkMap->Exist(fileHandle.m_file->m_inode)) {
        DBGLOG("set init state: %s", fileHandle.m_file->m_fileName.c_str());
        HardLinkDesc hardlinkDesc;
        hardlinkDesc.targetPath = fileHandle.m_file->m_fileName;
        hardlinkDesc.linkCount = parentInfo.nlink;
        hardlinkDesc.refCount = 1;
        m_hardlinkMap->Insert(fileHandle.m_file->m_inode, hardlinkDesc);
        return;
    }
    DBGLOG("set src link state : %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_file->SetSrcState(FileDescState::LINK);
    m_hardlinkMap->InsertLinks(fileHandle.m_file->m_inode, fileHandle);
    return;
}

int HardlinkControlFileReader::ProcessFileEntryV10(ParentInfo& parentInfo, ScannerHardLinkCtrl::LinkEntry& linkEntry,
    FileHandle& fileHandle)
{
    if (linkEntry.metaFileName.empty()) {
        ERRLOG("meta file is empty");
        return Module::FAILED;
    }
    if (GetOpenedMetaFileNameV10() != GetMetaFile(linkEntry.metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(linkEntry.metaFileName);
        if (OpenMetaControlFileV10(parentInfo.metaFileName) != Module::SUCCESS) {
            ERRLOG("open meta file failed!");
            return Module::FAILED;
        }
    }

    NasScanner::FileMeta fileMeta;
    int ret = m_scannerBackupMeta->ReadFileMeta(fileMeta, linkEntry.metaFileReadLen, linkEntry.metaFileOffset);
    if (ret == NAS_CTRL_FILE_RET_FAILED) {
        ERRLOG("ReadFileMeta failed for: %s", linkEntry.fileName.c_str());
        return Module::FAILED;
    }

    fileHandle.m_file->m_fileName = linkEntry.dirName + CTRL_FILE_PATH_SEPARATOR  + linkEntry.fileName;
    DBGLOG("Enter ProcessFileEntry: %s", fileHandle.m_file->m_fileName.c_str());
    FillFileMetaDataV10(fileHandle, fileMeta);

    ProcessHardlink(parentInfo, fileHandle);

    return Module::SUCCESS;
}

void HardlinkControlFileReader::FillFileMetaData(FileHandle& fileHandle, const FileMeta& fileMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    fileHandle.m_file->m_dirName = parentDirName;
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + 1,
        fullPath.length() - parentDirName.length() - 1);
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_size    = fileMeta.m_size;
    fileHandle.m_file->m_rdev    = fileMeta.m_rdev;
    fileHandle.m_file->m_atime   = fileMeta.m_atime;
    fileHandle.m_file->m_mtime   = fileMeta.m_mtime;
    fileHandle.m_file->m_ctime   = fileMeta.m_ctime;
    fileHandle.m_file->m_inode   = fileMeta.m_inode;
    fileHandle.m_file->m_mode    = fileMeta.m_mode;
    fileHandle.m_file->m_uid     = fileMeta.m_uid;
    fileHandle.m_file->m_gid     = fileMeta.m_gid;
    fileHandle.m_file->m_nlink   = fileMeta.m_nlink;

    if (!fileHandle.m_file->m_aclText.empty() || !fileHandle.m_file->m_securityDescriptor.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }

    return;
}

int HardlinkControlFileReader::FillFileMetaDataV10(FileHandle& fileHandle, const NasScanner::FileMeta& fileMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    fileHandle.m_file->m_dirName = parentDirName;
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + 1,
        fullPath.length() - parentDirName.length() - 1);
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_size    = fileMeta.m_size;
    fileHandle.m_file->m_rdev    = fileMeta.m_rdev;
    fileHandle.m_file->m_inode   = fileMeta.m_inode;
    fileHandle.m_file->m_atime   = fileMeta.m_atime;
    fileHandle.m_file->m_mtime   = fileMeta.m_mtime;
    fileHandle.m_file->m_ctime   = fileMeta.m_ctime;
    fileHandle.m_file->m_mode    = fileMeta.m_mode;
    fileHandle.m_file->m_uid     = fileMeta.m_uid;
    fileHandle.m_file->m_gid     = fileMeta.m_gid;
    fileHandle.m_file->m_nlink   = fileMeta.m_nlink;

    if (!fileHandle.m_file->m_aclText.empty()) {
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }
#ifdef _NAS
    if (fileMeta.m_fh.len > 0) {
        int ret = FSBackupUtils::SetSrcFileHandleForNfs(fileHandle, fileMeta.m_fh.len, fileMeta.m_fh.value);
        if (ret != Module::SUCCESS) {
            return Module::FAILED;
        }
    }
#endif
    return Module::SUCCESS;
}

int HardlinkControlFileReader::FillStatsFromControlHeader() const
{
    HardlinkCtrlParser::Header header;
    if (m_hardlinkCtrlParser == nullptr) {
        return Module::FAILED;
    }

    if (m_hardlinkCtrlParser->GetHeader(header) != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }

    m_controlInfo->m_noOfFilesToBackup += header.stats.noOfFiles;
    m_controlInfo->m_noOfBytesToBackup += header.stats.dataSize;

    return Module::SUCCESS;
}

int HardlinkControlFileReader::FillStatsFromControlHeaderV10() const
{
    ScannerHardLinkCtrl::Header header;
    if (!m_scannerHardLinkCtrl) {
        if (m_scannerHardLinkCtrl->GetHeader(header) != NAS_CTRL_FILE_RET_SUCCESS) {
            ERRLOG("Get ControlFile header failed!");
            return Module::FAILED;
        }
    } else {
        ERRLOG("m_scannerHardlinkCtrl is nullptr");
        return Module::FAILED;
    }

    m_controlInfo->m_noOfFilesToBackup += header.stats.noOfFiles;
    m_controlInfo->m_noOfBytesToBackup += header.stats.dataSize;
    return Module::SUCCESS;
}
