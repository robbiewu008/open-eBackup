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
#include "LibsmbWriterSyncInterface.h"
#include <fcntl.h>
#include "log/Log.h"
#include "BackupConstants.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr int RETRY_WAIT_IN_SEC = 1;
    constexpr uint8_t MAX_SMB_RETRY_COUNT = 3;
    constexpr uint8_t OFFSET_10 = 10;
    constexpr uint16_t DIRPATH_LENGTH = 4096;
    constexpr uint32_t DIRECTORY_SIZE = 4096;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr auto NAS_SCANNERBACKUPCTRL_ENTRY_MODE_META_MODIFIED = "mm";
    constexpr auto RECONNECT_CONTEXT_RETRY_TIMES = 5;
}

int SendWriterSyncRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    DBGLOG("Send %s Request: %s", GetLibsmbEventName(event).c_str(), fileHandle.m_file->m_fileName.c_str());
    cbData->event = event;

    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(event), PKT_COUNTER::SENT);
    int ret = 0;
    switch (event) {
        case LibsmbEvent::MKDIR:
            ret = SmbMkdirRecursive(fileHandle, cbData);
            break;
        case LibsmbEvent::REPLACE_DIR:
            ret = SmbDeleteAll(fileHandle, cbData);
            if (ret == SUCCESS) {
                ++cbData->controlInfo->m_noOfDirDeleted;
                fileHandle.m_file->SetDstState(FileDescState::INIT);
                cbData->writeQueue->Push(fileHandle);
                break;
            }
            // push the deleteInfo back to the queue
            if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, ret)) {
                SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
                break;
            }
            FSBackupUtils::RecordFailureDetail(cbData->failureRecorder, cbData->fileHandle.m_file->m_fileName, -ret);
            ++cbData->controlInfo->m_noOfDirFailed;
            break;
        case LibsmbEvent::DELETE: {
            ret = SmbDeleteAll(fileHandle, cbData);
            if (ret == SUCCESS) {
                ++cbData->controlInfo->m_noOfDirDeleted;
                break;
            }
            // push the deleteInfo back to the queue
            if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, ret)) {
                SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
                break;
            }
            FSBackupUtils::RecordFailureDetail(cbData->failureRecorder, cbData->fileHandle.m_file->m_fileName, -ret);
            ++cbData->controlInfo->m_noOfDirFailed;
            break;
        }
        default:
            break;
    }
    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(event), PKT_COUNTER::RECVD);
    delete cbData;
    cbData = nullptr;
    return SUCCESS;
}

int SmbMkdirRecursive(FileHandle fileHandle, SmbWriterCommonData *mkdirParams)
{
    uint16_t retryCnt = 0;
    int ret = FAILED;
    do {
        ret = SmbMkdirSync(retryCnt, mkdirParams);
        if (ret != SUCCESS) {
            mkdirParams->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, OFFSET_10, PKT_ERROR::RETRIABLE_ERR);
            ERRLOG("Directory creation failed for dir: %s, retryCnt: %d, ret: %d",
                mkdirParams->path.c_str(), retryCnt, ret);
            sleep(RETRY_WAIT_IN_SEC);
        }
        retryCnt++;
    } while ((ret != SUCCESS) && (retryCnt <= MAX_SMB_RETRY_COUNT));

    if (ret != SUCCESS) {
        if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
            ++mkdirParams->controlInfo->m_noOfDirFailed;
        } else {
            ++mkdirParams->controlInfo->m_noOfFilesFailed;
        }
        ERRLOG("mkdir failed: %s, retryCnt: %d, totalFailed: %llu, %llu", mkdirParams->path.c_str(), retryCnt,
            mkdirParams->controlInfo->m_noOfDirFailed.load(), mkdirParams->controlInfo->m_noOfFilesFailed.load());
        /*
         * if fileHandle refers to a file, and it's parent dir failed to create,
         * it's dstState will be FileDescState::INIT, preventing it being removed from writeCache
         */
        fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
        return FAILED;
    }

    bool isDir = fileHandle.m_file->IsFlagSet(IS_DIR);
    DBGLOG("SmbMkdirRecursive file: %s, parentpath: %s, isDir: %d",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str(), isDir);
    // 文件
    if (!isDir) {
        mkdirParams->writeQueue->Push(fileHandle);
        return SUCCESS;
    }
    // 目录
    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    mkdirParams->writeQueue->Push(fileHandle);

    return SUCCESS;
}

int SmbRemoveFileAndDirRecursive(FileHandle &fileHandle, SmbMkdirParams rmdirParams)
{
    int ret = FAILED;
    if (fileHandle.m_file->m_fileName.empty()) {
        return ret;
    }
    struct smb2_stat_64 st;
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, rmdirParams.writerCommonData->params.dstRootPath);
    ret = rmdirParams.mkdirSmbContext->SmbStat64(smbPath.c_str(), &st);
    if (ret != SUCCESS) {
        return ret;
    }
    if (st.smb2_type == SMB2_TYPE_DIRECTORY) {
        // call api to remove all file and dir inside
    } else {
        // call api to remove file
    }
    return ret;
}

int SmbMkdirSync(uint16_t retryCnt, SmbWriterCommonData *mkdirParams)
{
    string smbPath = RemoveFirstSeparator(mkdirParams->path);
    ConcatRootPath(smbPath, mkdirParams->params.dstRootPath);

    int ret = mkdirParams->mkdirSmbContext->SmbMkdir(smbPath.c_str());
    if (ret == -ENOENT || ret == -ENOTDIR) {
        string targetFilePath = GetPathName(smbPath);
        if (MakeDirRecursively(targetFilePath, mkdirParams) != SUCCESS) {
            ERRLOG("parent directory creation failed: %s, retryCnt: %d", targetFilePath.c_str(), retryCnt);
            return FAILED;
        }
        ret = mkdirParams->mkdirSmbContext->SmbMkdir(smbPath.c_str());
    }
    ret = HandleMkdirSyncReqStatus(ret, mkdirParams->path, retryCnt, mkdirParams);
    return ret;
}

int HandleMkdirSyncReqStatus(int status, string curPath,
    uint16_t retryCnt, SmbWriterCommonData *mkdirParams)
{
    DBGLOG("HandleMkdirSyncReqStatus: %s, status: %d, retryCnt: %d", curPath.c_str(), status, retryCnt);
    if (status == SUCCESS) {
        // 备份目录计数不计算根目录
        if (curPath != "/") {
            ++mkdirParams->controlInfo->m_noOfDirCopied;
        }
        return SUCCESS;
    }

    if (status == -EEXIST) {
        if (HandleDirExist(curPath, retryCnt, mkdirParams) != SUCCESS) {
            return FAILED;
        }
        return SUCCESS;
    }

    if (IfNeedRetry(mkdirParams->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        mkdirParams->pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        HandleConnectionException(mkdirParams->mkdirSmbContext, mkdirParams->params.dstSmbContextArgs,
            RECONNECT_CONTEXT_RETRY_TIMES);
        DBGLOG("mkdir failed, retrying: %s, status: %d, error: %s, retryCnt: %d",
            mkdirParams->path.c_str(), status, mkdirParams->mkdirSmbContext->SmbGetError().c_str(), retryCnt);
        return FAILED;
    }

    CheckStatusAndIncStat(status, LibsmbEventToPKT_TYPE(mkdirParams->event), mkdirParams->pktStats);
    ERRLOG("mkdir failed: %s, status: %d, error: %s, retryCnt: %d",
        mkdirParams->path.c_str(), status, mkdirParams->mkdirSmbContext->SmbGetError().c_str(), retryCnt);
    return FAILED;
}

int MakeDirRecursively(const string &targetFilePath, SmbWriterCommonData *mkdirParams)
{
    size_t dirPathLen = targetFilePath.size();
    if (dirPathLen == 0 || dirPathLen > DIRPATH_LENGTH) {
        ERRLOG("dir: %s, length: %d", targetFilePath.c_str(), targetFilePath.size());
        return FAILED;
    }
    string dirPath = targetFilePath;
    string parentDir = ".";
    /* remove trailing slash */
    if (dirPath.at(dirPath.size() - 1) == '/') {
        dirPath.replace(dirPath.size() - 1, 1, "");
    }

    string::size_type pos = 1;
    while (pos < dirPathLen) {
        pos = dirPath.find("/", pos, 1);
        if (pos == string::npos) {
            break;
        }
        string subPath = dirPath.substr(0, pos);
        pos++;
        DBGLOG("mkdir recv file: %s, pos:%d", subPath.c_str(), pos);
        if (StatAndMkDir(subPath, parentDir, mkdirParams) != SUCCESS) {
            return FAILED;
        }
    }

    DBGLOG("mkdir recv file: %s, pos:%d", targetFilePath.c_str(), pos);
    if (StatAndMkDir(targetFilePath, parentDir, mkdirParams) != SUCCESS) {
        return FAILED;
    }

    return SUCCESS;
}

int StatAndMkDir(string dirPath, string &parentDirPath, SmbWriterCommonData *mkdirParams)
{
    DBGLOG("StatAndMkDir mkdir: %s", dirPath.c_str());
    uint16_t retryCnt = 0;
    int ret = FAILED;
    string smbPath = RemoveFirstSeparator(dirPath);
    /**
     * don't concat prefix for aggragation here,
     * StatAndMkDir ensure dirPath is the final smb path
     **/
    do {
        ret = mkdirParams->mkdirSmbContext->SmbMkdir(smbPath.c_str());
        ret = HandleMkdirSyncReqStatus(ret, dirPath, retryCnt, mkdirParams);
        if (ret != SUCCESS) {
            sleep(RETRY_WAIT_IN_SEC);
        }
        retryCnt++;
    } while ((ret != SUCCESS) && (retryCnt <= MAX_SMB_RETRY_COUNT));

    if (ret != SUCCESS) {
        ERRLOG("Create failed: %s", dirPath.c_str());
        return FAILED;
    }

    parentDirPath = dirPath;
    return SUCCESS;
}

int HandleDirExist(string &curPath, uint16_t retryCnt, SmbWriterCommonData *mkdirParams)
{
    DBGLOG("HandleDirExist mkdir: %s", mkdirParams->path.c_str());
    struct smb2_stat_64 st;
    string smbPath = RemoveFirstSeparator(curPath);
    ConcatRootPath(smbPath, mkdirParams->params.dstRootPath);
    int ret = mkdirParams->mkdirSmbContext->SmbStat64(smbPath.c_str(), &st);
    if (ret != SUCCESS) {
        return SUCCESS;
    }
    if (st.smb2_type == SMB2_TYPE_DIRECTORY) {
        // We got a directory creation request with same name as the file present in BackupFS.
        // delete file as delete entry will come later. Without deleting we cannot create dir.
        return SUCCESS;
    }

    ret = mkdirParams->mkdirSmbContext->SmbUnlink(smbPath.c_str());
    if (ret < SUCCESS) {
        ERRLOG("unlink failed: %s, ret: %d, err: %s, retryCnt:%d", curPath.c_str(),
            ret, mkdirParams->mkdirSmbContext->SmbGetError(), retryCnt);
        return FAILED;
    }

    ret = mkdirParams->mkdirSmbContext->SmbMkdir(smbPath.c_str());
    if (ret < SUCCESS) {
        ERRLOG("mkdir failed: %s, status: %d, err: %s, retryCnt:%d", curPath.c_str(),
            ret, mkdirParams->mkdirSmbContext->SmbGetError(), retryCnt);
        return FAILED;
    }

    return SUCCESS;
}

int SmbDeleteAll(FileHandle& fileHandle, SmbWriterCommonData *smbDeleteParams)
{
    std::string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, smbDeleteParams->params.dstRootPath);
    return DeleteAllFilesInside(smbPath, smbDeleteParams);
}

int DeleteAllFilesInside(string& path, SmbWriterCommonData *smbDeleteParams)
{
    int subFileDeleteFailed = 0;
    struct smb2dirent *smb2dirent = nullptr;
    
    struct smb2dir *smb2dir = smbDeleteParams->mkdirSmbContext->SmbOpendir(path.c_str());
    if (smb2dir == nullptr) {
        DBGLOG("opendir failed for: %s %s", path.c_str(), smbDeleteParams->mkdirSmbContext->SmbGetError().c_str());
        return FAILED;
    }

    while ((smb2dirent = smbDeleteParams->mkdirSmbContext->SmbReadDir(smb2dir)) != nullptr) {
        if (!strcmp(smb2dirent->name, ".") || !strcmp(smb2dirent->name, "..")) {
            continue;
        }
        string fullPath(path);
        fullPath.append("/");
        fullPath.append(smb2dirent->name);
        if (smb2dirent->st.smb2_type == SMB2_TYPE_DIRECTORY) {
            int ret = DeleteAllFilesInside(fullPath, smbDeleteParams);
            if (ret != SUCCESS) {
                ERRLOG("Failed to delete sub dir: %s, %d", path.c_str(), ret);
                ++subFileDeleteFailed;
                continue;
            }
            DBGLOG("Delete subdir success: %s.", fullPath.c_str());
        } else {
            int ret = smbDeleteParams->mkdirSmbContext->SmbUnlink(fullPath.c_str());
            if (ret == SUCCESS) {
                DBGLOG("Delete subfile success: %s.", fullPath.c_str());
            } else if (ret == -ENOENT) {
                DBGLOG("Sub file: %s already deleted", fullPath.c_str());
            } else {
                ERRLOG("Failed to delete sub file: %s, %d", path.c_str(), ret);
                subFileDeleteFailed++;
            }
        }
    }

    // if no files/dir left in this directory, then remove it
    if (subFileDeleteFailed == 0) {
        int ret = smbDeleteParams->mkdirSmbContext->SmbRmdir(path.c_str());
        if (ret != SUCCESS) {
            DBGLOG("Failed to delete dir: %s, %s", path.c_str(),
                smbDeleteParams->mkdirSmbContext->SmbGetError().c_str());
            return ret;
        }
    } else {
        return -EAGAIN;
    }
    return SUCCESS;
}

int SmbUnlink(SmbWriterCommonData *linkDeleteParams)
{
    string smbPath = RemoveFirstSeparator(linkDeleteParams->fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, linkDeleteParams->params.dstRootPath);
    int ret = linkDeleteParams->mkdirSmbContext->SmbUnlink(smbPath.c_str());
    if (ret == SUCCESS) {
        DBGLOG("Delete target link file success: %s", linkDeleteParams->fileHandle.m_file->m_fileName.c_str());
    } else if (ret == -ENOENT) {
        DBGLOG("Target Link File: %s already deleted", linkDeleteParams->fileHandle.m_file->m_fileName.c_str());
    } else if (ret == -EISDIR) {
        return DeleteAllFilesInside(smbPath, linkDeleteParams);
    } else {
        DBGLOG("remove link file failed:  %s error: %s", linkDeleteParams->fileHandle.m_file->m_fileName.c_str(),
               linkDeleteParams->mkdirSmbContext->SmbGetError().c_str());
        return FAILED;
    }
    return SUCCESS;
}
