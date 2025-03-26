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
#include "AntiControlFileReader.h"
#include <sys/stat.h>
#include "log/Log.h"
#include "unistd.h"
#include "ParserUtils.h"

using namespace std;
using namespace Module;

namespace {
#ifdef WIN32
    const string PATH_SEP = "\\";
#else
    const string PATH_SEP = "/";
#endif
    const int SKIP = 1;
    const int FINISH = 2;
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
    const uint32_t MAX_UINT = 4294967295;    // 对应有符号数中-1
}

AntiControlFileReader::AntiControlFileReader(BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap)
    : m_backupParams(backupParams),
      m_readQueue(readQueuePtr),
      m_writeQueue(writeQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap)
{
    m_advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.srcAdvParams);
}

AntiControlFileReader::~AntiControlFileReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/* Public APIs */
BackupRetCode AntiControlFileReader::Start()
{
    try {
        m_thread = thread(&AntiControlFileReader::ThreadFunc, this);
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

BackupRetCode AntiControlFileReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode AntiControlFileReader::Enqueue(string contrlFile)
{
    DBGLOG("AntiControlFileReader enqueue: %s", contrlFile.c_str());
    m_controlFileQueue.push(contrlFile);
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus AntiControlFileReader::GetStatus()
{
    INFOLOG("Enter GetStatus");
    if (!m_controlInfo->m_controlReaderPhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (m_abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (m_controlInfo->m_controlReaderFailed || m_controlInfo->m_failed) {
        return BackupPhaseStatus::FAILED;
    }
    return BackupPhaseStatus::COMPLETED;
}

bool AntiControlFileReader::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

bool AntiControlFileReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("ControlFileReader check is complete: controlFileReaderProduce %d skip %llu total %llu",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_skipDirCnt.load(),
            (m_controlInfo->m_noOfFilesToBackup.load() + m_controlInfo->m_noOfDirToBackup.load()));
    }
    if ((m_controlInfo->m_controlFileReaderProduce + m_controlInfo->m_skipDirCnt) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup)) {
        INFOLOG("ControlFileReader complete: controlFileReaderProduce %d skip %llu total %llu",
            m_controlInfo->m_controlFileReaderProduce.load(), m_controlInfo->m_skipDirCnt.load(),
            (m_controlInfo->m_noOfFilesToBackup.load() + m_controlInfo->m_noOfDirToBackup.load()));
        return true;
    }
    return false;
}

void AntiControlFileReader::HandleComplete()
{
    m_controlInfo->m_controlReaderPhaseComplete = true;
    INFOLOG("AntiControlFileReader main thread end!");
}

/* Private methods */
void AntiControlFileReader::ThreadFunc()
{
    string controlFile = m_controlFileQueue.front();
    m_controlFileQueue.pop();
    if (OpenControlFile(controlFile) != Module::SUCCESS) {
        ERRLOG("Open control file failed!");
        m_controlInfo->m_controlReaderFailed = true;
        m_controlInfo->m_controlReaderPhaseComplete = true;
        ERRLOG("AntiControlFileReader main thread end after failure!");
        return;
    }
    ParentInfo parentInfo {};
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        
        CopyCtrlFileEntry fileEntry;
        CopyCtrlDirEntry dirEntry;
        int ret = ReadControlFileEntryAndProcess(fileEntry, dirEntry, parentInfo);
        DBGLOG("ReadControlFileEntryAndProcess ret is %d", ret);
        if (ret == FINISH) {
            INFOLOG("All directory readed in control file: %s", controlFile.c_str());
            m_controlInfo->m_controlReaderPhaseComplete = true;
            break;
        }
        if (ret == Module::FAILED) {
            ERRLOG("Read Control File and Process failed!");
            m_controlInfo->m_controlReaderFailed = true;
            break;
        }
    }
    HandleComplete();
    IsComplete(); // use for print statistic
    return;
}

int AntiControlFileReader::OpenControlFile(const std::string& controlFile)
{
    m_copyCtrlParser = make_unique<CopyCtrlParser>(controlFile);
    if (m_copyCtrlParser == nullptr) {
        return Module::FAILED;
    }
    DBGLOG("Open control file: %s", controlFile.c_str());
    CTRL_FILE_RETCODE ret = m_copyCtrlParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("failed to open control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    if (FillStatsFromControlHeader() != Module::SUCCESS) {
        ERRLOG("failed to fill stats from control file: %s", controlFile.c_str());
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int AntiControlFileReader::ReadControlFileEntry(CopyCtrlFileEntry& fileEntry, CopyCtrlDirEntry& dirEntry)
{
    CTRL_FILE_RETCODE ret = m_copyCtrlParser->ReadEntry(fileEntry, dirEntry);
    if (ret == CTRL_FILE_RETCODE::READ_EOF) {
        return FINISH;
    }
    return Module::SUCCESS;
}

int AntiControlFileReader::ReadControlFileEntryAndProcess(CopyCtrlFileEntry& fileEntry,
    CopyCtrlDirEntry& dirEntry, ParentInfo &parentInfo)
{
    int ret = ReadControlFileEntry(fileEntry, dirEntry);
    if (ret == FINISH) {
        return FINISH;
    }
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);

    // 跳过目录，只对文件进行修改。
    if (!dirEntry.m_dirName.empty()) {
        parentInfo.dirName = dirEntry.m_dirName;
        ++m_controlInfo->m_skipDirCnt;
        DBGLOG("ProcessDirectoryEntry: %s, %d, fileCount: %llu",
            dirEntry.m_dirName.c_str(), ret, fileHandle.m_file->m_fileCount);
        return SKIP;
    } else if (!fileEntry.m_fileName.empty()) {
        ret = ProcessFileEntry(parentInfo, fileEntry, fileHandle);
        DBGLOG("ProcessFileEntry: %s, %d", fileEntry.m_fileName.c_str(), ret);
        return ret;
    } else if (dirEntry.m_dirName.empty() && fileEntry.m_fileName.empty()) {
        ERRLOG("either dirname and filename are empty");
        return Module::SUCCESS;
    }
    
    return Module::SUCCESS;
}


string AntiControlFileReader::GetOpenedMetaFileName()
{
    if (m_metaParser == nullptr) {
        return "";
    }
    return m_metaParser->GetFileName();
}

string AntiControlFileReader::GetMetaFile(std::string metaFileName)
{
    return m_backupParams.scanAdvParams.metaFilePath + PATH_SEP + metaFileName;
}

int AntiControlFileReader::ReadFileMeta(FileMeta& fileMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = m_metaParser->ReadFileMeta(fileMeta, offset);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int AntiControlFileReader::ProcessFileEntry(ParentInfo& parentInfo, const CopyCtrlFileEntry& fileEntry,
    FileHandle& fileHandle)
{
    DBGLOG("Enter ProcessFileEntry");

    if (fileEntry.m_mode == Module::CTRL_ENTRY_MODE_DATA_DELETED) {
        return SKIP;
    }

    // 读取metaFile中的元数据
    if (GetOpenedMetaFileName() != GetMetaFile(fileEntry.m_metaFileName)) {
        parentInfo.metaFileName = GetMetaFile(fileEntry.m_metaFileName);
        if (OpenMetaControlFile(parentInfo.metaFileName) != Module::SUCCESS) {
            ERRLOG("OpenMetaControlFile failed");
            return Module::FAILED;
        }
    }
    FileMeta fileMeta;
    if (ReadFileMeta(fileMeta, fileEntry.metaFileOffset) != Module::SUCCESS) {
        ERRLOG("ReadFileMeta failed");
        return Module::FAILED;
    }

    fileHandle.m_file->m_fileName = parentInfo.dirName + PATH_SEP + fileEntry.m_fileName;

    FillFileMetaData(fileHandle, fileMeta);     // 将读出来的元数据传给fileHandle

    return PushFileHandleToReader(fileHandle);  // controlFileReader to Reader
}

int AntiControlFileReader::OpenMetaControlFile(const std::string& metaFile)
{
    if (m_backupParams.backupType == BackupType::RESTORE) {
        m_metaParser = make_unique<MetaParser>(metaFile);
    } else {
        m_metaParser = make_unique<MetaParser>(metaFile, true);
    }
    if (m_metaParser == nullptr) {
        return Module::FAILED;
    }
    CTRL_FILE_RETCODE ret = m_metaParser->Open(CTRL_FILE_OPEN_MODE::READ);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

void AntiControlFileReader::FillFileMetaData(FileHandle& fileHandle, const FileMeta& fileMeta)
{
    string fullPath = fileHandle.m_file->m_fileName;
    string parentDirName = FSBackupUtils::GetParentDir(fullPath);
    if (parentDirName != PATH_SEP) { // 当根目录是分隔符时，就不需要去掉目录最后的分隔符了
        FSBackupUtils::RemoveTrailingSlashes(parentDirName);
    }
    fileHandle.m_file->m_dirName = parentDirName;
    fileHandle.m_file->m_onlyFileName = fullPath.substr(parentDirName.length() + 1,
                                                        fullPath.length() - parentDirName.length() - 1);
    FSBackupUtils::RemoveLeadingSlashes(fileHandle.m_file->m_onlyFileName);
    
    // 更新atime，修改mode为只读
    fileHandle.m_file->m_atime = m_advParams->atime;
    INFOLOG("isOnlyModifyAtime: %d", m_advParams->isOnlyModifyAtime);
    if (m_advParams->isOnlyModifyAtime) {
        fileHandle.m_file->m_mode = MAX_UINT;
    } else {
        fileHandle.m_file->m_mode = fileMeta.m_mode & ~S_IWUSR & ~S_IWGRP & ~S_IWOTH;
    }
    // 其他元数据保持不变
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_uid      = MAX_UINT;                     // nfs中对uid判断，如果为-1，则不修改
    fileHandle.m_file->m_gid      = MAX_UINT;                     // nfs中对gid判断，如果为-1，则不修改
    fileHandle.m_file->m_size     = fileMeta.m_size;
    fileHandle.m_file->m_rdev     = fileMeta.m_rdev;
    fileHandle.m_file->m_inode    = fileMeta.m_inode;
    fileHandle.m_file->m_btime    = fileMeta.m_btime;
    fileHandle.m_file->m_mtime    = fileMeta.m_mtime;
    fileHandle.m_file->m_ctime    = fileMeta.m_ctime;
    fileHandle.m_file->m_nlink    = fileMeta.m_nlink;
    fileHandle.m_file->m_fileAttr = fileMeta.m_attr;

    if (!fileHandle.m_file->m_aclText.empty()) { /* no need to check SecurityDescriptor for Windows/SMB */
        fileHandle.m_file->SetFlag(ACL_EXIST);
    }

    return;
}

int AntiControlFileReader::PushFileHandleToReader(FileHandle& fileHandle)
{
    while (!m_readQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            return Module::SUCCESS;
        }
    }
    ++m_controlInfo->m_controlFileReaderProduce;
    return Module::SUCCESS;
}

int AntiControlFileReader::FillStatsFromControlHeader()
{
    CopyCtrlParser::Header header;

    if (m_copyCtrlParser == nullptr) {
        return Module::FAILED;
    }

    if (m_copyCtrlParser->GetHeader(header) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("Get control file header failed!");
        return Module::FAILED;
    }

    m_controlInfo->m_noOfDirToBackup += header.stats.noOfDirs;
    m_controlInfo->m_noOfFilesToBackup += header.stats.noOfFiles;
    m_controlInfo->m_noOfBytesToBackup += header.stats.dataSize;

    return Module::SUCCESS;
}