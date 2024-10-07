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
#include "MknodRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsMknodCbData* CreateMknodCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh)
{
    auto cbData = new(nothrow) NfsMknodCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->nfsfh = nfsfh;

    return cbData;
}

int SendMknod(FileHandle &fileHandle, NfsMknodCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    /* todo: need write libnfs API which uses parentfh */
    int cRet = MP_FAILED;
    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send mknod req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    cRet = nfs->NfsMknodAsync(fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
        fileHandle.m_file->m_rdev, SendMknodCb, cbData);
    if (cRet != MP_SUCCESS) {
        ERRLOG("Send create req failed for: %s, Err: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendMknodCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    data = data;
    auto cbData = (NfsMknodCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        if (status == MP_SUCCESS) {
            struct nfsfh *nfsFh = (struct nfsfh*)data;
            if (nfsFh != nullptr) {
                FreeNfsFh(nfsFh);
            }
        }
        return;
    }

    if (status < MP_SUCCESS) {
        HandleMknodFailure(commonData, fileHandle, status, nfs);
        return;
    }

    HandleMknodSuccess(commonData, fileHandle, data, nfs);
}

void HandleMknodSuccess(NfsCommonData *commonData, FileHandle &fileHandle, void *data, struct nfs_context *nfs)
{
    nfs = nfs;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    fileHandle.m_retryCnt = 0;

    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*)data;
    fileHandle.m_file->SetDstState(FileDescState::WRITED);
    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }
    commonData->controlInfo->m_noOfFilesCopied++;

    if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
        INFOLOG("SetTargetCopied %s", fileHandle.m_file->m_fileName.c_str());
        if (commonData->hardlinkMap->IsTargetCopied(fileHandle.m_file->m_inode) == false) {
            commonData->hardlinkMap->SetTargetCopied(fileHandle.m_file->m_inode);
        }
    }
}

void HandleMknodFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT || status == -BACKUP_ERR_NOTDIR) {
        ERRLOG("parent path not exist: %s", fileHandle.m_file->m_fileName.c_str());
        MknodFailureHandling(commonData, status, fileHandle);
    } else if (status == -BACKUP_ERR_EISDIR || status == -BACKUP_ERR_EEXIST) {
        // We got a file creation request with same name as the directory present in BackupFS.
        // delete directory as delete entry will come later. Without deleting we cannot create file.
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("Mknod req failed for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            MknodFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("mknod req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_onlyFileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::RETRIED);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("Mknod failed for %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        MknodFailureHandling(commonData, status, fileHandle);
    }
}

void MknodFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::CREATE);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER);
}
