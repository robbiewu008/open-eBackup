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
#include "CreateRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsCreateCbData* CreateCreateCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    uint32_t openFlag, RestoreReplacePolicy restoreReplacePolicy)
{
    auto cbData = new(nothrow) NfsCreateCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->nfsfh = nfsfh;
    cbData->openFlag |= openFlag;
    cbData->restoreReplacePolicy = restoreReplacePolicy;

    if (fileHandle.m_file->IsFlagSet(TRUNCATE)) {
        cbData->openFlag |= O_TRUNC;
    }

    return cbData;
}

int SendCreate(FileHandle &fileHandle, NfsCreateCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send create req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int cRet = MP_FAILED;
    if (cbData->nfsfh == nullptr) {
        cRet = nfs->NfsCreateAsync(fileHandle.m_file->m_fileName.c_str(), cbData->openFlag, fileHandle.m_file->m_mode,
            SendCreateCb, cbData);
    } else {
        cRet = nfs->NfsCreateAsyncWithDirHandle(cbData->nfsfh, fileHandle.m_file->m_onlyFileName.c_str(),
            fileHandle.m_file->m_dirName.c_str(), cbData->openFlag, fileHandle.m_file->m_mode, SendCreateCb, cbData);
    }
    if (cRet != MP_SUCCESS) {
        ERRLOG("Send create req failed for: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::SENT);
    return MP_SUCCESS;
}

void SendCreateCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsCreateCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    struct nfsfh *nfsfh = cbData->nfsfh;

    auto fileHandle = cbData->fileHandle;
    auto restoreReplacePolicy = cbData->restoreReplacePolicy;
    delete cbData;
    cbData = nullptr;

    DBGLOG("Enter SendCreateCb for: %s", fileHandle.m_file->m_fileName.c_str());

    commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        if (status == MP_SUCCESS) {
            struct nfsfh *nfsFh = (struct nfsfh *)data;
            FreeNfsFh(nfsFh);
            data = nullptr;
        }
        return;
    }

    if (status == -BACKUP_ERR_EEXIST) {
        DBGLOG("File exist: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));

        // Send to lstat with nfsfh
        SendLstatFromCreate(fileHandle, commonData, nfsfh, restoreReplacePolicy);
        return;
    }

    if (status < MP_SUCCESS) {
        HandleCreateFailure(commonData, fileHandle, status, nfs);
        return;
    }

    fileHandle.m_retryCnt = 0;

    fileHandle.m_file->dstIOHandle.nfsFh = (struct nfsfh *)data;
    fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
}

void HandleCreateFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (IS_LIBNFS_NEED_RETRY(status)) {
        DBGLOG("Create file failed for %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("Max create req retry reached for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            CreateFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("create retry: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::CREATE, PKT_COUNTER::RETRIED);
        }
    } else if (status == -BACKUP_ERR_ENOENT || status == -BACKUP_ERR_NOTDIR) {
        ERRLOG("Parent dir not present. Create file failed for %s, Status: %d, ERR: %s",
            fileHandle.m_file->m_fileName.c_str(), status, nfs_get_error(nfs));
        CreateFailureHandling(commonData, status, fileHandle);
    } else if (status == -BACKUP_ERR_EISDIR) {
        // We got a file creation request with same name as the directory present in BackupFS.
        // delete directory as delete entry will come later. Without deleting we cannot create file.
        WARNLOG("Already dir present with same name as %s, Status: %d, ERR: %s",
            fileHandle.m_file->m_fileName.c_str(), status, nfs_get_error(nfs));
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else {
        ERRLOG("Create file failed for %s, Status: %d, ERR: %s",
            fileHandle.m_file->m_fileName.c_str(), status, nfs_get_error(nfs));
        CreateFailureHandling(commonData, status, fileHandle);
    }
}

void CreateFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::CREATE);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, true, true);
}

int SendLstatFromCreate(FileHandle &fileHandle, NfsCommonData *commonData, struct nfsfh *nfsfh,
    RestoreReplacePolicy restoreReplacePolicy)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }

    NfsLstatCbData *cbData = CreateLstatCbData(fileHandle, *commonData, nfsfh, restoreReplacePolicy);
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        CreateFailureHandling(commonData, -BACKUP_ERR_EEXIST, fileHandle);
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::LSTAT) != MP_SUCCESS) {
        CreateFailureHandling(commonData, -BACKUP_ERR_EEXIST, fileHandle);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}