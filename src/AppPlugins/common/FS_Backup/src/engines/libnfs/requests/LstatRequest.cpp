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
#include "LstatRequest.h"

using namespace Module;
using namespace std;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsLstatCbData* CreateLstatCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    RestoreReplacePolicy restoreReplacePolicy)
{
    auto cbData = new(nothrow) NfsLstatCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->restoreReplacePolicy = restoreReplacePolicy;
    cbData->nfsfh = nfsfh;

    return cbData;
}

int SendLstat(FileHandle &fileHandle, NfsLstatCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send lstat req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = MP_FAILED;
    if (cbData->nfsfh == nullptr) {
        res = nfs->NfsLstatAsync(fileHandle.m_file->m_fileName.c_str(), SendLstatCb, cbData);
    } else {
        res = nfs->NfsLstatAsyncWithDirHandle(fileHandle.m_file->m_onlyFileName.c_str(), cbData->nfsfh,
            SendLstatCb, cbData);
    }

    if (res != MP_SUCCESS) {
        ERRLOG("Send lstat req failed for: %s, %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendLstatCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsLstatCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    auto restoreReplacePolicy = cbData->restoreReplacePolicy;
    auto nfsfh = cbData->nfsfh;

    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    fileHandle.m_file->SetDstState(FileDescState::LSTAT);

    if (status < MP_SUCCESS) {
        HandleLstatFailure(commonData, fileHandle, status, nfs);
        return;
    }

    // Lstat returns that file/dir exists.
    fileHandle.m_retryCnt = 0;
    auto st = (struct nfs_stat_64 *)data;
    if (CheckConditionsForBackupOrRestoreJob(commonData, fileHandle, restoreReplacePolicy, st)) {
        // INCR backup scenario for non hardlink files
        if (S_ISDIR(st->nfs_mode)) {
            // We got a file creation request with same name as the directory present in BackupFS.
            // delete directory as delete entry will come later. Without deleting we cannot create file.
            WARNLOG("Already dir present with same name as %s", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
        } else if (S_ISLNK(st->nfs_mode) || IsSpecialDeviceFile(st->nfs_mode)) {
            fileHandle.m_file->SetDstState(FileDescState::LINK_DEL_FOR_RESTORE);
        } else if (S_ISREG(st->nfs_mode)) {
            // need to test and see if existing mknod/symlink will be deleted & created as normal file
            SendCreateWithTruncateFlag(fileHandle, commonData, nfsfh, restoreReplacePolicy);
            return;
        }
        PushToWriteQueue(commonData, fileHandle);
    }
}

void HandleLstatFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status,
    struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT || status == -BACKUP_ERR_NOTDIR) {
        // ERR_NOTDIR will come in this situation.
        // delete a file and create a dir with same name, and create a file inside the dir.
        // so when lstat been sent, the parent path may be still a file.
        DBGLOG("lstat file not exist: %s", fileHandle.m_file->m_fileName.c_str());
        if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
            string targetPath {};
            commonData->hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
            if (strcmp(fileHandle.m_file->m_fileName.c_str(), targetPath.c_str()) != 0) {
                fileHandle.m_file->SetDstState(FileDescState::LINK);
            }
        }
        fileHandle.m_retryCnt = 0;
        PushToWriteQueue(commonData, fileHandle);
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("lstat req failed for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            LstatFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("lstat req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::RETRIED);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("lstat failed for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        LstatFailureHandling(commonData, status, fileHandle);
    }
}

bool CheckConditionsForBackupOrRestoreJob(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy, struct nfs_stat_64 *st)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return false;
    }

    string targetPath {};
    if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
        commonData->hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
        if (strcmp(fileHandle.m_file->m_fileName.c_str(), targetPath.c_str()) == 0) {
            /* Always overwrite hardlink files */
            HandleOverWrite(commonData, fileHandle, st, restoreReplacePolicy);
            return false;
        } else {
            fileHandle.m_file->SetDstState(FileDescState::LINK);
            PushToWriteQueue(commonData, fileHandle);
            return false;
        }
    }
    if (!CheckConditionsForRestore(commonData, fileHandle, restoreReplacePolicy, st, targetPath)) {
        return false;
    }

    return true;
}

bool CheckConditionsForRestore(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy, struct nfs_stat_64 *st, string targetPath)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return false;
    }

    if (restoreReplacePolicy == RestoreReplacePolicy::NONE) {
        return true;
    } else if ((restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) ||
        (restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER &&
        fileHandle.m_file->m_mtime > st->nfs_mtime)) {
        HandleOverWrite(commonData, fileHandle, st, restoreReplacePolicy);
        return false;
    } else {
        if ((restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) ||
            (restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER &&
            !(fileHandle.m_file->m_mtime > st->nfs_mtime))) {
            if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr &&
                strcmp(fileHandle.m_file->m_fileName.c_str(), targetPath.c_str()) != 0) {
                commonData->hardlinkMap->IncreaseRef(fileHandle.m_file->m_inode);
            }
            fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
            commonData->controlInfo->m_noOfFilesWriteSkip++;
        }
        return false;
    }
    return true;
}

void HandleOverWrite(NfsCommonData *commonData, FileHandle &fileHandle, struct nfs_stat_64 *st,
    RestoreReplacePolicy restoreReplacePolicy)
{
    if (commonData == nullptr || st == nullptr) {
        ERRLOG("commonData or st is nullptr");
        return;
    }

    if (fileHandle.m_file->GetDstState() == FileDescState::LINK || S_ISLNK(fileHandle.m_file->m_mode)
        || IS_SPECIAL_DEVICE_FILE(fileHandle)) {
        PushToWriteQueue(commonData, fileHandle);
        return;
    }

    /* The file to be created is a regular file.
     * If the existing object is anything other than regular file, it has to be deleted */
    if (S_ISDIR(st->nfs_mode)) {
        // We got a file creation request with same name as the directory present in TargetFS.
        // Without deleting we cannot create file.
        WARNLOG("Already dir present with same name as %s", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
    } else if (S_ISLNK(st->nfs_mode) || IsSpecialDeviceFile(st->nfs_mode)) {
        // We got a file creation request with same name as the symlink or special file present in TargetFS.
        // Without deleting we cannot create file.
        WARNLOG("Already symlink or special file present with same name as %s", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetDstState(FileDescState::LINK_DEL_FOR_RESTORE);
    } else if (S_ISREG(st->nfs_mode) && fileHandle.m_file->m_size == 0 && st->nfs_size != 0) {
        fileHandle.m_file->SetFlag(TRUNCATE);
    }

    if (fileHandle.m_file->m_size != 0 && restoreReplacePolicy != RestoreReplacePolicy::NONE) {
        fileHandle.m_file->SetDstState(FileDescState::LINK_DEL_FOR_RESTORE);
    }

    PushToWriteQueue(commonData, fileHandle);
}

void LstatFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::LSTAT);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER);
}

void PushToWriteQueue(NfsCommonData *commonData, FileHandle &fileHandle)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }
}

int SendCreateWithTruncateFlag(FileHandle &fileHandle, NfsCommonData *commonData, struct nfsfh *nfsfh,
    RestoreReplacePolicy restoreReplacePolicy)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }

    NfsCreateCbData *cbData = CreateCreateCbData(fileHandle, *commonData, nfsfh, O_TRUNC, restoreReplacePolicy);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::CREATE) != MP_SUCCESS) {
        CreateFailureHandling(commonData, -BACKUP_ERR_EEXIST, fileHandle);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

bool IsSpecialDeviceFile(mode_t mode)
{
    if (S_ISCHR(mode) || S_ISBLK(mode) || S_ISFIFO(mode)) {
        return true;
    }

    return false;
}