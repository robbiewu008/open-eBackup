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
#include "LinkDeleteRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsLinkDeleteCbData* CreateLinkDeleteCbData(FileHandle &fileHandle, NfsCommonData &commonData)
{
    auto cbData = new(nothrow) NfsLinkDeleteCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;

    return cbData;
}

/* =====================================================
 * Async Request. This is done in Restore flow alone
 * ===================================================== */

int SendLinkDelete(FileHandle &fileHandle, NfsLinkDeleteCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send linkdelete req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int ret = MP_FAILED;
    if (cbData->nfsfh == nullptr) {
        ret = nfs->NfsUnlinkAsync(fileHandle.m_file->m_fileName.c_str(), SendLinkDeleteCb, cbData);
    } else {
        ret = nfs->NfsUnlinkAsyncWithParentFh(fileHandle.m_file->m_onlyFileName.c_str(), cbData->nfsfh,
            SendLinkDeleteCb, cbData);
    }
    if (ret != MP_SUCCESS) {
        ERRLOG("Send Link Delete req failed for: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(),
            nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendLinkDeleteCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    UNREFERENCE_PARAM(data);

    auto cbData = (NfsLinkDeleteCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    if (status < MP_SUCCESS) {
        HandleLinkDeleteFailure(commonData, status, fileHandle, nfs);
        return;
    }

    /* At this point it is assumed that the target file/dir/linkfile is deleted */
    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->SetDstState(FileDescState::LSTAT);
    commonData->writeQueue->Push(fileHandle);
}

void HandleLinkDeleteFailure(NfsCommonData *commonData, int status, FileHandle &fileHandle,
    struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT) {
        WARNLOG("File: %s already deleted ", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetDstState(FileDescState::LSTAT);
        commonData->writeQueue->Push(fileHandle);
    } else if (status == -BACKUP_ERR_EISDIR || status == -BACKUP_ERR_EEXIST || status == -BACKUP_ERR_NOTEMPTY) {
        commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED);
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL_RESTORE);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("delete link req failed for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            LinkDeleteFailureHandling(commonData, status, fileHandle);
        } else {
            DBGLOG("delete Link req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->controlInfo->m_noOfDstRetryCount++;
            commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::RETRIED);
        }
    } else if (status != MP_SUCCESS) {
        ERRLOG("Delete link failed: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        LinkDeleteFailureHandling(commonData, status, fileHandle);
    }

    return;
}

void LinkDeleteFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::LINKDELETE);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER);
}

/* ================================================
 * Sync Request
 * ================================================ */

int SendLinkDeleteSync(FileHandle &fileHandle, NfsLinkDeleteCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    NfsCommonData *commonData = cbData->writeCommonData;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }
    delete cbData;
    cbData = nullptr;

    shared_ptr<NfsContextWrapper> nfs = commonData->syncNfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send linkdelete req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        return MP_FAILED;
    }
    commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::SENT);
    int status = nfs->NfsUnlinkLock(fileHandle.m_file->m_fileName.c_str());
    commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::RECVD);
    int ret = HandleLinkDeleteSyncStatus(status, fileHandle, nfs, commonData);
    if (ret != MP_SUCCESS) {
        return MP_FAILED;
    }

    DBGLOG("Delete file success: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_retryCnt = 0;
    if (fileHandle.m_file->GetDstState() == FileDescState::LINK_DEL_FAILED) {
        return MP_SUCCESS;
    }
    if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
        string targetPath {};
        commonData->hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath);
        if (strcmp(fileHandle.m_file->m_fileName.c_str(), targetPath.c_str()) != 0) {
            fileHandle.m_file->SetDstState(FileDescState::LINK);
            if (commonData->writeQueue != nullptr) {
                commonData->writeQueue->Push(fileHandle);
            }
            return MP_SUCCESS;
        }
    }

    if (S_ISLNK(fileHandle.m_file->m_mode)) {
        fileHandle.m_file->SetDstState(FileDescState::LSTAT);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    }

    return MP_SUCCESS;
}

int HandleLinkDeleteSyncStatus(int status, FileHandle &fileHandle, shared_ptr<NfsContextWrapper> nfs,
    NfsCommonData *commonData)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }

    if (status == -BACKUP_ERR_ENOENT) {
        WARNLOG("File: %s already deleted", fileHandle.m_file->m_fileName.c_str());
    } else if (status == -BACKUP_ERR_EISDIR || status == -BACKUP_ERR_EEXIST || status == -BACKUP_ERR_NOTEMPTY) {
        if (LibNfsDeleteDirectorySync(fileHandle.m_file->m_fileName,
            commonData->syncNfsContextContainer) != MP_SUCCESS) {
            LinkDeleteFailureHandling(commonData, status, fileHandle);
            return MP_FAILED;
        }
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        fileHandle.m_retryCnt++;
        commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("delete link req failed for for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            LinkDeleteFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("delete Link req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::LINKDELETE, PKT_COUNTER::RETRIED);
        }
        return MP_FAILED;
    } else if (status != MP_SUCCESS) {
        ERRLOG("remove link file failed: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs->GetNfsContext()));
        LinkDeleteFailureHandling(commonData, status, fileHandle);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}
