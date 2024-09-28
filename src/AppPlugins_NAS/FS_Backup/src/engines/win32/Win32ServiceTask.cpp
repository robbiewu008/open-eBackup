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
#include "Win32ServiceTask.h"
#include <sddl.h>
#include <fileapi.h>
#include <AclAPI.h>
#include <handleapi.h>
#include <filesystem>
#include <errhandlingapi.h>
#include "FileSystemUtil.h"
#include "Win32PathUtils.h"
#include "Win32BackupEngineUtils.h"
#include "log/Log.h"
#include "FSBackupUtils.h"


using namespace std;
using namespace FS_Backup;
using namespace Module;
using namespace Win32BackupEngineUtils;

namespace {
    const uint64_t UNIX_TIME_START = 0x019DB1DED53E8000; /* January 1, 1970 (start of Unix epoch) in "ticks" */
    const uint64_t TICKS_PER_SECOND = 10000000; /* a tick is 100ns */
    // DACL_SECURITY_INFORMATION | SACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION
    const uint32_t ALL_SEC_INFORMATION = 0x0000000F;
}

void Win32ServiceTask::SetCriticalErrorInfo(uint64_t err)
{
    if (err == ERROR_DISK_FULL) {
        m_backupFailReason = BackupPhaseStatus::FAILED_NOSPACE;
    }
    return;
}

bool Win32ServiceTask::IsCriticalError() const
{
    if (m_backupFailReason == BackupPhaseStatus::FAILED_NOSPACE) {
        return true;
    }
    return false;
}

void Win32ServiceTask::HandleOpenSrc()
{
    string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    DBGLOG("Enter HandleOpenSrc, %s", srcFile.c_str());
    wstring wSrcFile = Win32BackupEngineUtils::ExtenedPathW(srcFile);
    m_fileHandle.m_file->srcIOHandle.win32Fd = ::CreateFileW(
        wSrcFile.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL);
    if (m_fileHandle.m_file->srcIOHandle.win32Fd == INVALID_HANDLE_VALUE) {
        m_errDetails = {srcFile, GetLastError()};
        ERRLOG("open failed %s errno %d", srcFile.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    DBGLOG("open %s success!", srcFile.c_str());
    m_result = SUCCESS;
    return;
}

void Win32ServiceTask::HandleOpenDst()
{
    string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("Enter HandleOpenDst, %s", dstFile.c_str());
    if (ProcessOpenDst(dstFile) != SUCCESS) {
        m_result = FAILED;
        return;
    }
    DBGLOG("create %s success!", dstFile.c_str());
    m_result = SUCCESS;
    return;
}

bool Win32ServiceTask::ProcessCreateSymlink(const std::string& dstFile)
{
    if (m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) {
        DBGLOG("X8000 storage not support windows symlink yet! Skip.");
        return true;
    }
    bool isDirectory = FileHandleHasAttribute(m_fileHandle, FILE_ATTRIBUTE_DIRECTORY);
    std::string linkPath = dstFile;
    std::string targetPath;
    if (IsFileHandleSymbolicLink(m_fileHandle)) {
        targetPath = GetSymbolicLinkTargetPath(m_fileHandle);
    } else {
        targetPath = GetJunctionPointTargetPath(m_fileHandle);
    }
    DBGLOG("detected dst filehandle is a symlink, linkPath: %s, targetPath: %s, isDir: %u",
        linkPath.c_str(), targetPath.c_str(), isDirectory);
    std::wstring wLinkPath = Win32BackupEngineUtils::ExtenedPathW(linkPath);
    std::wstring wTargetPath = FileSystemUtil::Utf8ToUtf16(targetPath);
    if (!NeedCreateSymlink(linkPath)) {
        return true;
    }
    if (Win32BackupEngineUtils::IsFileExistsW(wLinkPath) &&
        m_params.restoreReplacePolicy == RestoreReplacePolicy::RENAME) {
        std::string originDstFile = FileSystemUtil::Utf16ToUtf8(wLinkPath);
        wLinkPath = Win32BackupEngineUtils::GetUnusedNewPathW(wLinkPath);
        std::string newDstFile = FileSystemUtil::Utf16ToUtf8(wLinkPath);
        DBGLOG("rename symlink path %s => %s", originDstFile.c_str(), newDstFile.c_str());
    }
    if (!::CreateSymbolicLinkW(
        wLinkPath.c_str(),
        wTargetPath.c_str(),
        isDirectory ? SYMBOLIC_LINK_FLAG_DIRECTORY : 0)) {
        m_errDetails = { dstFile, ::GetLastError() };
        ERRLOG("create symbolic link failed, linkPath: %s, targetPath: %s, isDir: %u",
            linkPath.c_str(), targetPath.c_str(), isDirectory);
        return false;
    }
    return true;
}

bool Win32ServiceTask::NeedCreateSymlink(const std::string& dstFile) const
{
    if (m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) {
        DBGLOG("just restore need create new symlink");
        return false;
    }
    std::optional<FileSystemUtil::StatResult> srcStatResult = FileSystemUtil::Stat(dstFile);
    if (!srcStatResult) {
        DBGLOG("source file: %s don't exist", dstFile.c_str());
        return true;
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST ||
        (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER &&
        srcStatResult->ModifyTime() >= m_fileHandle.m_file->m_mtime)) {
        return false;
    }
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::RENAME) {
        return true;
    }
    Win32BackupEngineUtils::RemovePath(dstFile);
    return true;
}

int Win32ServiceTask::ProcessOpenDst(const std::string& dstFile)
{
    wstring wDstFile = Win32BackupEngineUtils::ExtenedPathW(dstFile);
    DWORD creationDisposition = CREATE_ALWAYS;
    if (m_params.backupType == BackupType::RESTORE || m_params.backupType == BackupType::FILE_LEVEL_RESTORE) {
        creationDisposition = CREATE_NEW;
    }
    /*
     * Backup both symbolic link and junction link as symbolic link
     */
    if (FSBackupUtils::IsSymLinkFile(m_fileHandle)) {
        DBGLOG("skip open dst for symlink %s", dstFile.c_str());
        return SUCCESS;
    }
    m_fileHandle.m_file->dstIOHandle.win32Fd = ::CreateFileW(wDstFile.c_str(), GENERIC_WRITE, FILE_SHARE_WRITE, NULL,
        creationDisposition, FILE_ATTRIBUTE_NORMAL, NULL);
    if (m_fileHandle.m_file->dstIOHandle.win32Fd == INVALID_HANDLE_VALUE) {
        /* When success, need to check whether the file is a sparse file. So can't return */
        DWORD lastError = GetLastError();
        if (lastError == ERROR_FILE_EXISTS) {
            if (ProcessRestorePolicy(dstFile) != SUCCESS) {
                return FAILED;
            }
        } else if (lastError == ERROR_PATH_NOT_FOUND) {
            WARNLOG("parent dir of %s is not exist, create it now", dstFile.c_str());
            DWORD errorCode = 0; // error for dir create
            if (!Win32BackupEngineUtils::CreateDirectoryRecursively(Win32PathUtil::GetParentDir(dstFile),
                                                                    m_params.dstRootPath, errorCode)) {
                m_errDetails = {dstFile, lastError};
                return FAILED;
            }
            if (ProcessOpenDst(dstFile) != SUCCESS) {
                return FAILED;
            }
        } else {
            m_errDetails = {dstFile, GetLastError()};
            ERRLOG("create file failed %s, errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    if (m_fileHandle.m_file->GetDstState() != FileDescState::WRITE_SKIP &&
        (ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle) || IsEmptySparseFile(m_fileHandle))) {
        if (!InitSparseFile(m_fileHandle.m_file->m_fileName,
            m_fileHandle.m_file->dstIOHandle.win32Fd, m_fileHandle.m_file->m_size)) {
            m_errDetails = {dstFile, ::GetLastError()};
            ERRLOG("init sparse file failed %s errno %d", dstFile.c_str(), m_errDetails.second);
            return FAILED;
        }
    }
    return SUCCESS;
}

int Win32ServiceTask::ProcessRestorePolicy(const string& dstFile)
{
    DBGLOG("restore policy file %s", dstFile.c_str());
    if (m_params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        DBGLOG("ignore exists file %s success!", dstFile.c_str());
        return SUCCESS;
    } else if (m_params.restoreReplacePolicy == RestoreReplacePolicy::RENAME) {
        return ProcessOverrideRenamePolicy(dstFile);
    } else if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER) {
        return ProcessOverwriteOlderPolicy(dstFile);
    } else if (m_params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) {
        return ProcessOverwritePolicy(dstFile);
    } else {
        ERRLOG("invalid restore policy %d", (int)m_params.restoreReplacePolicy);
        return FAILED;
    }
    return FAILED;
}

/**
 * if file exists, open it with renamed path
 */
int Win32ServiceTask::ProcessOverrideRenamePolicy(const std::string& dstFile)
{
    std::wstring wDstFile = Win32BackupEngineUtils::ExtenedPathW(dstFile);
    std::wstring wnewDstFile = Win32BackupEngineUtils::GetUnusedNewPathW(wDstFile);
    std::string newDstFile = FileSystemUtil::Utf16ToUtf8(wnewDstFile);
    m_fileHandle.m_file->dstIOHandle.win32Fd = ::CreateFileW(
        wnewDstFile.c_str(),
        GENERIC_WRITE,
        0,
        nullptr,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        nullptr);
    if (m_fileHandle.m_file->dstIOHandle.win32Fd == INVALID_HANDLE_VALUE) {
        m_errDetails = { dstFile, GetLastError() };
        ERRLOG("open renamed path failed %s errno %d, origin path: %s",
            newDstFile.c_str(), m_errDetails.second, dstFile.c_str());
        return FAILED;
    }
    DBGLOG("apply rename overite policy success, rename %s => %s", dstFile.c_str(), newDstFile.c_str());
    return SUCCESS;
}

int Win32ServiceTask::ProcessOverwriteOlderPolicy(const string& dstFile)
{
    std::optional<FileSystemUtil::StatResult> srcStatResult = FileSystemUtil::Stat(dstFile);
    if (!srcStatResult) {
        m_errDetails = {dstFile, GetLastError()};
        ERRLOG("stat failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    if (srcStatResult->ModifyTime() >= m_fileHandle.m_file->m_mtime) {
        DBGLOG("skip %s success!", dstFile.c_str());
        m_fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        return SUCCESS;
    }
    return ProcessOverwritePolicy(dstFile);
}

int Win32ServiceTask::ProcessOverwritePolicy(const string& dstFile)
{
    wstring wDstFile = Win32BackupEngineUtils::ExtenedPathW(dstFile);
    // check if file attribute is hidden
    bool isHidden = FileHandleHasAttribute(m_fileHandle, FILE_ATTRIBUTE_HIDDEN);
    if (isHidden) {
        // set hidden attribute
        m_fileHandle.m_file->dstIOHandle.win32Fd = ::CreateFileW(wDstFile.c_str(), GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL | FILE_ATTRIBUTE_HIDDEN, NULL);
    } else {
        m_fileHandle.m_file->dstIOHandle.win32Fd = ::CreateFileW(wDstFile.c_str(), GENERIC_WRITE, 0, NULL,
            CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (m_fileHandle.m_file->dstIOHandle.win32Fd == INVALID_HANDLE_VALUE) {
        m_errDetails = {dstFile, GetLastError()};
        ERRLOG("open failed %s errno %d", dstFile.c_str(), m_errDetails.second);
        return FAILED;
    }
    DBGLOG("overwrite %s success!", dstFile.c_str());
    return SUCCESS;
}

/* check if handle is valid and set error */
bool Win32ServiceTask::CheckWin32HandleValid(HANDLE hFile, const std::string& filePath)
{
    if (!IsValidWin32Handle(hFile)) {
        m_errDetails = {filePath, ::GetLastError()};
        ERRLOG("invalid win32 fileHandle %s, errno %d", filePath.c_str(), m_errDetails.second);
        m_result = FAILED;
        return false;
    }
    return true;
}

void Win32ServiceTask::HandleReadData()
{
    if (FSBackupUtils::IsSymLinkFile(m_fileHandle)) {
        m_result = SUCCESS; // Win32 sym target stored in XMeta, skip readding
        return;
    }

    string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    DBGLOG("Enter HandleReadData, %s", srcFile.c_str());
    if (m_fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle)) {
        DBGLOG("CreateFile small file %s blockSize %d", srcFile.c_str(), m_params.blockSize);
        wstring wSrcFile = Win32BackupEngineUtils::ExtenedPathW(srcFile);
        m_fileHandle.m_file->srcIOHandle.win32Fd = ::CreateFileW(wSrcFile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
            OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    }
    if (!CheckWin32HandleValid(m_fileHandle.m_file->srcIOHandle.win32Fd, srcFile)) {
        return;
    }

    DWORD numberOfBytesRead = 0;
    OVERLAPPED ov {};
    Win32BackupEngineUtils::ConvertUint64ToDword(m_fileHandle.m_block.m_offset, ov.OffsetHigh, ov.Offset);
    BOOL res = ReadFile(m_fileHandle.m_file->srcIOHandle.win32Fd, (LPVOID)(m_fileHandle.m_block.m_buffer),
        (DWORD)m_fileHandle.m_block.m_size, &numberOfBytesRead, &ov);
    if (!res || numberOfBytesRead != m_fileHandle.m_block.m_size) {
        uint32_t errcode = (GetLastError() == 0 ? E_BACKUP_READ_LESS_THAN_EXPECTED : GetLastError());
        m_errDetails = {srcFile, errcode};
        ERRLOG("ReadFile failed %s, cnt %d, size %llu, errno %d",
            srcFile.c_str(), numberOfBytesRead, m_fileHandle.m_block.m_size, m_errDetails.second);
        SetCriticalErrorInfo(m_errDetails.second);
        if (m_params.discardReadError) {
            m_fileHandle.m_file->SetFlag(READ_FAILED_DISCARD);
            m_result = SUCCESS;
        } else {
            m_result = FAILED;
            return;
        }
    }
    DBGLOG("ReadFile success %s blockInfo %llu %llu %u!", srcFile.c_str(),
        m_fileHandle.m_block.m_seq, m_fileHandle.m_block.m_offset, m_fileHandle.m_block.m_size);
    m_result = SUCCESS;

    if (m_fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle)) {
        CloseHandle(m_fileHandle.m_file->srcIOHandle.win32Fd);
        m_fileHandle.m_file->srcIOHandle.win32Fd = INVALID_HANDLE_VALUE;
        DBGLOG("CloseHandle small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
    }

    return;
}

void Win32ServiceTask::PushSubStreamFileHandleToReadQueue(const std::wstring& wStreamName)
{
    std::string streamName = FileSystemUtil::Utf16ToUtf8(wStreamName);
    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::WIN32_IO, BackupIOEngine::WIN32_IO);
    fileHandle.m_file->m_fileName = m_fileHandle.m_file->m_fileName + ":" + streamName;
    fileHandle.m_file->m_dirName = m_fileHandle.m_file->m_dirName;
    fileHandle.m_file->m_atime = m_fileHandle.m_file->m_atime;
    fileHandle.m_file->m_ctime = m_fileHandle.m_file->m_ctime;
    fileHandle.m_file->m_mtime = m_fileHandle.m_file->m_mtime;
    fileHandle.m_file->m_fileAttr = m_fileHandle.m_file->m_fileAttr;
    fileHandle.m_file->m_mode = FILE_IS_ADS_FILE; /* mark this file is a ADS sub stream */
    DBGLOG("push ADS file to readQueue: %s %s",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str());
    /* fetch size in bytes of ADS files */
    std::string srcFile = PathConcat(m_params.srcRootPath, fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    std::wstring wSrcFile = Win32BackupEngineUtils::ExtenedPathW(srcFile);
    std::optional<FileSystemUtil::StatResult> statResult = FileSystemUtil::StatW(wSrcFile);
    if (!statResult) {
        ERRLOG("stat ADS file %s failed", srcFile.c_str());
        return;
    }
    fileHandle.m_file->m_size = statResult->Size();
    fileHandle.m_file->ClearFlag(IS_DIR); // ads file must be handled as a file rather than a directory
    fileHandle.m_file->SetSrcState(FileDescState::META_READED);
    m_extendContext.readQueuePtr->Push(fileHandle);
    ++m_extendContext.controlInfo->m_noOfSubStreamFound;
    ++m_extendContext.subStreamCount;

    m_extendContext.controlInfo->m_streamHostFilePendingMap->IncStreamPending(fileHandle.m_file->m_fileName);
}

/* Post ADS detect routine start here, process the origin fileHandle */
void Win32ServiceTask::PostReadMetaRoutine() const
{
    /* directory fileHandle has been pushed to aggregate queue at the beginning of Win32CopyReader.ProcessReadEntries */
    if (m_fileHandle.IsDir()) {
        if (m_extendContext.subStreamCount > 0) {
            // dir won't be calculated in host backup, so mark complete here to prevent stream handle stucking in timer
            DBGLOG("dir %s has substream, mark copied ahead", m_fileHandle.m_file->m_fileName.c_str());
            m_extendContext.controlInfo->m_streamHostFilePendingMap->MarkHostWriteComplete(
                m_fileHandle.m_file->m_fileName);
        }
        return;
    }
    /* 1. has sub stream, to mark this file FILE_HAVE_ADS, won't be aggregated */
    if (m_extendContext.subStreamCount > 0) {
        m_fileHandle.m_file->m_mode |= FILE_HAVE_ADS;
    }
    /* 2. have no sub stream and is aggregated, just push to aggregate queue direcly */
    if (m_extendContext.subStreamCount == 0 && m_fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED) {
        m_extendContext.aggregateQueuePtr->Push(m_fileHandle);
        ++m_extendContext.controlInfo->m_readProduce;
        return;
    }

    /* re-push the handle to read queue with srcState set to META_READED */
    m_fileHandle.m_file->SetSrcState(FileDescState::META_READED);
    m_extendContext.readQueuePtr->Push(m_fileHandle);
    return;
}

void Win32ServiceTask::HandleFindStreamFailed(const std::string& srcFile, DWORD lastError)
{
    // ADS read fail won't make file fail, let READ_DATA stage to handle the 'not exist' case
    m_result = SUCCESS;

    // 1. have no sub stream
    if (lastError == ERROR_HANDLE_EOF) {
        DBGLOG("this file don't have any other data stream: %s", srcFile.c_str());
        return;
    }
    
    // 2. host file not found may be not a fatal error (common in aggregation restore case)
    if (lastError == ERROR_FILE_NOT_FOUND &&
        m_params.backupDataFormat == BackupDataFormat::AGGREGATE &&
        (m_params.backupType == BackupType::FILE_LEVEL_RESTORE || m_params.backupType == BackupType::RESTORE)) {
        DBGLOG("skip read meta for %s during aggregation", srcFile.c_str());
        return;
    }

    // 3. base storage do not support symlink (both backup/restore won't support symfile ADS)
    if (lastError == ERROR_FILE_NOT_FOUND && FSBackupUtils::IsSymLinkFile(m_fileHandle)) {
        DBGLOG("skip read meta for symlink file %s, not support", srcFile.c_str());
        return;
    }

    // 3. other reason, fail
    m_errDetails = {srcFile, lastError};
    ERRLOG("find stream for file %s failed, err %d", srcFile.c_str(), lastError);
}

/* read ADS as meta for Windows file, and push streams as new FileHandle to m_readQueue */
void Win32ServiceTask::HandleReadMeta()
{
    string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    DBGLOG("Enter HandleReadMeta, %s", srcFile.c_str());
    wstring wSrcFile = Win32BackupEngineUtils::ExtenedPathW(srcFile);
    WIN32_FIND_STREAM_DATA streamData;
    HANDLE streamHandle = ::FindFirstStreamW(
        wSrcFile.c_str(),
        _STREAM_INFO_LEVELS::FindStreamInfoStandard,
        &streamData,
        0);
    DWORD lastError = 0;
    if (streamHandle == INVALID_HANDLE_VALUE) {
        lastError = ::GetLastError();
        HandleFindStreamFailed(srcFile, lastError);
        return PostReadMetaRoutine();
    }
    // enummerate and proccess the ADS
    do {
        std::optional<std::wstring> streamNameRes = Win32BackupEngineUtils::GetStreamNameW(streamData.cStreamName);
        if (!streamNameRes) { /* skip main data stream or invalid stream name */
            continue;
        }
        PushSubStreamFileHandleToReadQueue(streamNameRes.value());
        /* build new fileHandle for sub stream and push to readQueue */
    } while (::FindNextStreamW(streamHandle, &streamData));
    /* sync stream count of this file to global ADS counter */
    lastError = ::GetLastError();
    if (lastError == ERROR_HANDLE_EOF) {
        /* read succeed and reached EOF */
        DBGLOG("this file don't have more stream files, file %s", m_fileHandle.m_file->m_fileName.c_str());
        ::FindClose(streamHandle);
        m_result = SUCCESS;
        return PostReadMetaRoutine();
    } else {
        /* some error happend when fetching data stream */
        ERRLOG("failed to find next stream for file: %s, error: %d",
            m_fileHandle.m_file->m_fileName.c_str(), lastError);
        ::FindClose(streamHandle);
        m_result = FAILED;
        m_errDetails = {srcFile, lastError};
        return PostReadMetaRoutine();
    }
}

void Win32ServiceTask::HandleWriteData()
{
    string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    /* if fileHandle is a symlink */
    if (FSBackupUtils::IsSymLinkFile(m_fileHandle)) {
        DBGLOG("write data for symlink %s", dstFile.c_str());
        m_result = ProcessCreateSymlink(dstFile) ? SUCCESS : FAILED;
        return;
    }
 
    DBGLOG("Enter HandleWriteData: %s, offset: %lu, blocksize: %lu",
        dstFile.c_str(), m_fileHandle.m_block.m_offset, m_fileHandle.m_block.m_size);
    if (m_fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle)) {
        DBGLOG("CreateFile small file %s blockSize %d", dstFile.c_str(), m_params.blockSize);
        // optimation : if none write deny security descriptor, set descriptor directly.
        m_result = ProcessOpenDst(dstFile);
        if ((m_result == FAILED) || (m_fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP)) {
            DBGLOG("ProcessOpenDst %s failed for small dst fd", dstFile.c_str());
            return;
        }
    }
    if (!CheckWin32HandleValid(m_fileHandle.m_file->dstIOHandle.win32Fd, dstFile)) {
        ERRLOG("not opened file. %s", dstFile.c_str());
        return;
    }
    DWORD writedSize = 0;
    OVERLAPPED ov {};
    Win32BackupEngineUtils::ConvertUint64ToDword(m_fileHandle.m_block.m_offset, ov.OffsetHigh, ov.Offset);
    BOOL res = WriteFile(m_fileHandle.m_file->dstIOHandle.win32Fd, (LPVOID)(m_fileHandle.m_block.m_buffer),
        (DWORD)m_fileHandle.m_block.m_size, &writedSize, &ov);
    if (!res || writedSize != m_fileHandle.m_block.m_size) {
        m_errDetails = {dstFile, GetLastError()};
        ERRLOG("win32 WriteFile failed %s size %u cnt %d errno %d",
            dstFile.c_str(), m_fileHandle.m_block.m_size, writedSize, m_errDetails.second);
        SetCriticalErrorInfo(m_errDetails.second);
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    CloseSmallFileDstFd();
    return;
}

// 该方法合入Module
LARGE_INTEGER ConvertSecondsToWin32Time(uint64_t time)
{
    LARGE_INTEGER li;
    li.QuadPart = time * TICKS_PER_SECOND + UNIX_TIME_START;
    return li;
}

bool Win32ServiceTask::SetFileInformation()
{
    FILE_BASIC_INFO fileBasicInfo {};
    fileBasicInfo.LastAccessTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_atime);
    fileBasicInfo.CreationTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_ctime);
    fileBasicInfo.LastWriteTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_mtime);
    fileBasicInfo.ChangeTime = fileBasicInfo.LastWriteTime;
    fileBasicInfo.FileAttributes = static_cast<DWORD>(m_fileHandle.m_file->m_fileAttr);
    if (!SetFileInformationByHandle(m_fileHandle.m_file->dstIOHandle.win32Fd,
                                    FileBasicInfo,
                                    &fileBasicInfo,
                                    sizeof(FILE_BASIC_INFO))) {
        return false;
    }
    return true;
}

void Win32ServiceTask::CloseSmallFileDstFd()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle)) {
        if (!CloseHandle(m_fileHandle.m_file->dstIOHandle.win32Fd)) {
            m_errDetails = {m_fileHandle.m_file->m_fileName, GetLastError()};
            WARNLOG("Close failed %s, errno: %d", m_fileHandle.m_file->m_fileName.c_str(), GetLastError());
        }
        DBGLOG("close small file %s", m_fileHandle.m_file->m_fileName.c_str());
    }
}

void Win32ServiceTask::HandleWriteMetaForADS()
{
    // default success, ADS file cannot failed (subtask will hang due to count mismatch otherwise)
    m_result = SUCCESS;

    // 1. check if is the last ADS
    uint64_t pendingStreamNum =
        m_extendContext.controlInfo->m_streamHostFilePendingMap->PendingStreamNum(m_fileHandle.m_file->m_fileName);
    if (pendingStreamNum > 1) {
        DBGLOG("%s is not the final ADS of the host (%d pending), skip write meta",
            m_fileHandle.m_file->m_fileName.c_str(), pendingStreamNum);
        return;
    }
    
    // 2. write meta for host file
    std::string hostFilePath = m_fileHandle.m_file->m_fileName;
    auto pos = hostFilePath.rfind(":");
    if (pos == std::string::npos) {
        ERRLOG("failed to write meta for ADS file, invalid ADS stream path: %s",
            m_fileHandle.m_file->m_fileName.c_str());
        return;
    }
    hostFilePath = hostFilePath.substr(0, pos);
    std::string dstPath = PathConcat(m_params.dstRootPath, hostFilePath, m_params.dstTrimPrefix);
    DBGLOG("write meta for ADS file %s, main stream path: %s",
        m_fileHandle.m_file->m_fileName.c_str(), dstPath.c_str());
    std::wstring wDstPath = Win32BackupEngineUtils::ExtenedPathW(dstPath);

    HANDLE hFile = ::CreateFileW(
        wDstPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        ERRLOG("Open %s to set information failed, errno %d", dstPath.c_str(), ::GetLastError());
        return;
    }
    FILE_BASIC_INFO fileBasicInfo {};
    fileBasicInfo.LastAccessTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_atime);
    fileBasicInfo.CreationTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_ctime);
    fileBasicInfo.LastWriteTime = ConvertSecondsToWin32Time(m_fileHandle.m_file->m_mtime);
    fileBasicInfo.ChangeTime = fileBasicInfo.LastWriteTime;
    fileBasicInfo.FileAttributes = static_cast<DWORD>(m_fileHandle.m_file->m_fileAttr);

    // SetFileInformationByHandle to set for main stream
    if (!::SetFileInformationByHandle(hFile, FileBasicInfo, &fileBasicInfo, sizeof(FILE_BASIC_INFO))) {
        ERRLOG("SetFileInformationByHandle failed, host file: %s, errno %d",
            dstPath.c_str(), ::GetLastError());
    }

    ::CloseHandle(hFile);
    return;
}

void Win32ServiceTask::HandleWriteMeta()
{
    if (m_fileHandle.IsAdsFile()) {
        return HandleWriteMetaForADS();
    }
    // 处理特殊文件待完成
    string dstPath = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("Enter HandleWriteMeta write meta %s", dstPath.c_str());
    /* if fileHandle is a symlink, skip write meta when backup, as X8000 storage not support symlink */
    if ((FSBackupUtils::IsSymLinkFile(m_fileHandle))
        && (m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC)) {
        DBGLOG("skip write meta for symlink %s when doing backup", dstPath.c_str());
        m_result = SUCCESS;
        return;
    }
    wstring wDstPath = Win32BackupEngineUtils::ExtenedPathW(dstPath);
    m_fileHandle.m_file->dstIOHandle.win32Fd = ::CreateFileW(
        wDstPath.c_str(),
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OPEN_REPARSE_POINT,
        NULL);
    if (m_fileHandle.m_file->dstIOHandle.win32Fd == INVALID_HANDLE_VALUE) {
        m_errDetails = {dstPath, GetLastError()};
        ERRLOG("Open %s to set information failed, errno %d", dstPath.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    if (!SetFileInformation()) {
        ERRLOG("set file information failed , %s, %d, error: %d", dstPath.c_str(),
            m_fileHandle.m_file->m_fileAttr, GetLastError());
        m_errDetails = {dstPath, GetLastError()};
        ::CloseHandle(m_fileHandle.m_file->dstIOHandle.win32Fd);
        m_result = FAILED;
        return;
    }
    DBGLOG("write meta %s success!", dstPath.c_str());
    ::CloseHandle(m_fileHandle.m_file->dstIOHandle.win32Fd);
    DWORD setDescriptorError = 0;
    if (m_params.writeAcl && !SetSecurityDescriptorW(wDstPath, setDescriptorError)) {
        m_errDetails = { dstPath, setDescriptorError };
        m_result = FAILED;
        return;
    }
    m_result = SUCCESS;
    return;
}

bool Win32ServiceTask::InitSparseFile(const std::string& filepath, HANDLE hFile, uint64_t size)
{
    DBGLOG("init sparse file %s with size %llu", filepath.c_str(), size);
    if (hFile == INVALID_HANDLE_VALUE || hFile == nullptr) {
        ERRLOG("failed to init sparse file %s , handle is invalid!", filepath.c_str());
        return false;
    }
    LARGE_INTEGER sizeEx;
    DWORD dwTemp = 0;
    if (!::DeviceIoControl(hFile, FSCTL_SET_SPARSE, nullptr, 0, nullptr, 0, &dwTemp, nullptr)) {
        ERRLOG("set sparse file flag failed for %s, backup as normal file", filepath.c_str());
        // maybe filesystem not support sparse file, will backup as normal file
    }
    if (size == 0) { // no need to alloc hole or physical size
        return true;
    }
    sizeEx.QuadPart = size;
    if (!::SetFilePointerEx(hFile, sizeEx, nullptr, FILE_BEGIN)) {
        ERRLOG("failed to move file pointer offset to %llu for file %s", size, filepath.c_str());
        return false;
    }
    // if sparse flag set failed, this API will allocate physical space
    if (!::SetEndOfFile(hFile)) {
        ERRLOG("failed to allocate %llu bytes for file %s", size, filepath.c_str());
        return false;
    }
    // reset file
    sizeEx.QuadPart = 0;
    if (!::SetFilePointerEx(hFile, sizeEx, nullptr, FILE_BEGIN)) {
        ERRLOG("failed to reset pointer offset to zero for file %s", filepath.c_str());
        return false;
    }
    return true;
}

static bool EnablePrivilege(const wchar_t* privilegeName)
{
    HANDLE hToken = nullptr;
    TOKEN_PRIVILEGES tokenPrivileges;

    if (!::OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken)) {
        WARNLOG("failed to enabled privilege, error: %u", ::GetLastError());
        return false;
    }

    if (!::LookupPrivilegeValue(nullptr, privilegeName, &tokenPrivileges.Privileges[0].Luid)) {
        WARNLOG("failed to enabled privilege, error: %u", ::GetLastError());
        ::CloseHandle(hToken);
        return false;
    }

    tokenPrivileges.PrivilegeCount = 1;
    tokenPrivileges.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

    if (!::AdjustTokenPrivileges(hToken, FALSE, &tokenPrivileges, 0, nullptr, nullptr)) {
        WARNLOG("failed to enabled privilege, error: %u", ::GetLastError());
        ::CloseHandle(hToken);
        return false;
    }

    if (::GetLastError() == ERROR_NOT_ALL_ASSIGNED) {
        WARNLOG("failed to enabled privilege, error: %u", ::GetLastError());
        ::CloseHandle(hToken);
        return false;
    }

    ::CloseHandle(hToken);
    return true;
}

bool Win32ServiceTask::SetSecurityDescriptorW(const std::wstring& wPath, DWORD& errorCode) const
{
    if (m_fileHandle.m_file->m_securityDescriptor.empty()) {
        WARNLOG("writeAcl is set true, but SecurityDescriptor ACE string is empty!");
        return true;
    }

    EnablePrivilege(SE_RESTORE_NAME);

    wstring wAclText = FileSystemUtil::Utf8ToUtf16(m_fileHandle.m_file->m_securityDescriptor);
    PSECURITY_DESCRIPTOR pSecurityDescriptor = nullptr;
    // RAII, prevent memory leak
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ::LocalFree((pSecurityDescriptor == nullptr) ? nullptr : pSecurityDescriptor);
    });

    if (!::ConvertStringSecurityDescriptorToSecurityDescriptorW(
        wAclText.c_str(), SDDL_REVISION_1, &pSecurityDescriptor, NULL)) {
        errorCode = ::GetLastError();
        ERRLOG("ConvertStringSecurityDescriptorToSecurityDescriptor failed, errno %d", errorCode);
        return false;
    }

    uint32_t sdFlag = ALL_SEC_INFORMATION;

    // extract DACL and write to file
    PACL pDacl = nullptr;
    BOOL bPresent = FALSE;
    BOOL bDefaulted = FALSE;
    if (!::GetSecurityDescriptorDacl(pSecurityDescriptor, &bPresent, &pDacl, &bDefaulted) || !bPresent) {
        errorCode = ::GetLastError();
        ERRLOG("Get dAcl from psd failed, bDacl:%u, bDacl: %u, err: %u", bPresent, bDefaulted, errorCode);
        return false;
    }

    // do not set SACL, keep null
    PACL pSacl = nullptr;
    if (!::GetSecurityDescriptorSacl(pSecurityDescriptor, &bPresent, &pSacl, &bDefaulted) || !bPresent) {
        DBGLOG("Get sAcl from psd failed, bPresent:%u, bDefaulted: %u, err: %u",
            bPresent, bDefaulted, ::GetLastError());
        sdFlag = DACL_SECURITY_INFORMATION | OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION;
    }

    // extract Owner and write to file
    PSID pOwner = nullptr;
    if (!::GetSecurityDescriptorOwner(pSecurityDescriptor, &pOwner, &bDefaulted)) {
        WARNLOG("Get owner from psd failed, bDefaulted: %u, err: %u", bDefaulted, ::GetLastError());
    }

    PSID pGroup = nullptr;
    if (!::GetSecurityDescriptorGroup(pSecurityDescriptor, &pGroup, &bDefaulted)) {
        WARNLOG("Get group from psd failed, bDefaulted: %u, err: %u", bDefaulted, ::GetLastError());
    }

    DWORD res = ::SetNamedSecurityInfoW(const_cast<LPWSTR>(wPath.c_str()),
        SE_FILE_OBJECT, sdFlag, pOwner, pGroup, pDacl, pSacl);
    if (res != ERROR_SUCCESS) {
        ERRLOG("Set dAcl failed, errno %u", res);
        errorCode = res;
        return false;
    }
    return true;
}

void Win32ServiceTask::HandleLink()
{
    string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    string target = m_params.linkTarget;
    DBGLOG("Hardlink %s to %s ", dstFile.c_str(), target.c_str());
    wstring wDstFile = Win32BackupEngineUtils::ExtenedPathW(dstFile);
    wstring wTarget = Win32BackupEngineUtils::ExtenedPathW(target);
    // This api only can create 1023 hard links for per file
    if (::CreateHardLinkW(wDstFile.c_str(), wTarget.c_str(), NULL)) {
        DBGLOG("Hardlink %s success!", dstFile.c_str());
        m_result = SUCCESS;
        return;
    }
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        /* Exsiting same name file, delete it first */
        if (Win32DeleteFile(dstFile) && ::CreateHardLinkW(wDstFile.c_str(), wTarget.c_str(), NULL)) {
            DBGLOG("Hardlink %s success!", dstFile.c_str());
            m_result = SUCCESS;
            return;
        }
    }
    m_errDetails = {dstFile, GetLastError()};
    ERRLOG("Hardlink %s failed, errno %d", dstFile.c_str(), m_errDetails.second);
    m_result = FAILED;
    return;
}

void Win32ServiceTask::HandleCloseSrc()
{
    if (m_fileHandle.m_file->m_size <= m_params.blockSize) {
        DBGLOG("ignore close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    string srcFile = PathConcat(m_params.srcRootPath, m_fileHandle.m_file->m_fileName, m_params.srcTrimPrefix);
    DBGLOG("Enter HandleCloseSrc, %s", srcFile.c_str());
    if (!CloseHandle(m_fileHandle.m_file->srcIOHandle.win32Fd)) {
        m_errDetails = {srcFile, GetLastError()};
        WARNLOG("%s already have been closed, errno %d", srcFile.c_str(), m_errDetails.second);
    }
    m_result = SUCCESS;
    return;
}

void Win32ServiceTask::HandleCloseDst()
{
    string dstFile = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("Enter HandleCloseDst, %s", dstFile.c_str());
    if (m_fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, m_fileHandle)) {
        DBGLOG("ignore close small file %s size %llu blockSize %d",
            m_fileHandle.m_file->m_fileName.c_str(), m_fileHandle.m_file->m_size, m_params.blockSize);
        m_result = SUCCESS;
        return;
    }
    if (!IsValidWin32Handle(m_fileHandle.m_file->dstIOHandle.win32Fd)) {
        WARNLOG("invalid handle!");
    }
    if (!CloseHandle(m_fileHandle.m_file->dstIOHandle.win32Fd)) {
        m_errDetails = {dstFile, GetLastError()};
        WARNLOG("%s already have been closed, errno %d", dstFile.c_str(), m_errDetails.second);
    }
    m_result = SUCCESS;
    return;
}

void Win32ServiceTask::HandleDelete()
{
    DBGLOG("Enter HandleDelete");
    string path = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    wstring wPath = Win32BackupEngineUtils::ExtenedPathW(path);
    if (m_fileHandle.IsDir()) {
        if (!RemoveDirectory(wPath.c_str())) {
            m_errDetails = {path, GetLastError()};
            ERRLOG("Delete dir %s failed, errno %d", path.c_str(), m_errDetails.second);
            return;
        }
    } else {
        if (!Win32DeleteFile(path)) {
            m_errDetails = {path, GetLastError()};
            m_result = FAILED;
            return;
        }
    }
    m_result = SUCCESS;
    return;
}

bool Win32ServiceTask::Win32DeleteFile(const string& filePath)
{
    wstring wFilePath = Win32BackupEngineUtils::ExtenedPathW(filePath);
    if (!::DeleteFileW(wFilePath.c_str())) {
        DWORD lastError = GetLastError();
        if (lastError == ERROR_ACCESS_DENIED) {
            WARNLOG("File %s is a read-only file, remove the read-only attribute now!", filePath.c_str());
            return DeleteReadOnlyFile(filePath);
        } else if (lastError == ERROR_FILE_NOT_FOUND) {
            WARNLOG("File %s had been deleted!", filePath.c_str());
            return true;
        }
        ERRLOG("Delete file %s failed, errno %d", filePath.c_str(), lastError);
        return false;
    }
    return true;
}

bool Win32ServiceTask::IsValidWin32Handle(const HANDLE& handle) const
{
    return handle != nullptr && handle != INVALID_HANDLE_VALUE;
}

bool Win32ServiceTask::DeleteReadOnlyFile(const string& filePath)
{
    wstring wFilePath = Win32BackupEngineUtils::ExtenedPathW(filePath);
    if (!SetFileAttributes(wFilePath.c_str(),
        static_cast<DWORD>(m_fileHandle.m_file->m_fileAttr) & ~FILE_ATTRIBUTE_READONLY)) {
        ERRLOG("Remove the read-only attribute of file %s failed, errno %d", filePath.c_str(), GetLastError());
        return false;
    }
    if (!::DeleteFileW(wFilePath.c_str())) {
        ERRLOG("Delete file %s failed, errno %d", filePath.c_str(), GetLastError());
        return false;
    }
    return true;
}

void Win32ServiceTask::HandleCreateDir()
{
    string dstDir = PathConcat(m_params.dstRootPath, m_fileHandle.m_file->m_fileName, m_params.dstTrimPrefix);
    DBGLOG("HandleCreateDir: %s", dstDir.c_str());
    DWORD errorCode = 0;
    if (!Win32BackupEngineUtils::CreateDirectoryRecursively(dstDir, m_params.dstRootPath, errorCode)) {
        m_errDetails = { dstDir, errorCode };
        ERRLOG("Dir %s create failed, errno %d", dstDir.c_str(), m_errDetails.second);
        m_result = FAILED;
        return;
    }
    wstring wDstDir = Win32BackupEngineUtils::ExtenedPathW(dstDir);
    DWORD setDescriptorError = 0;
    if (m_params.writeAcl && !SetSecurityDescriptorW(wDstDir, setDescriptorError)) {
        ERRLOG("set security descriptor for dir %s failed", dstDir.c_str());
        m_errDetails = { dstDir, setDescriptorError };
        m_result = FAILED;
        return;
    }
    DBGLOG("HandleCreateDir Success!");
    m_result = SUCCESS;
    return;
}
