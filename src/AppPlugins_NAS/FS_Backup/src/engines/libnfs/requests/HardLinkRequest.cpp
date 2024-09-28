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
#include "HardLinkRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsHardLinkCbData* CreateHardLinkCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    RestoreReplacePolicy restoreReplacePolicy, string targetPath)
{
    auto cbData = new(nothrow) NfsHardLinkCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->nfsfh = nfsfh;
    cbData->restoreReplacePolicy = restoreReplacePolicy;
    cbData->targetPath = targetPath;

    return cbData;
}

int SendHardLink(FileHandle &fileHandle, NfsHardLinkCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    /* todo: need write libnfs API which uses parentfh */
    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send hardlink req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = nfs->NfsHardLinkAsync(cbData->targetPath.c_str(), fileHandle.m_file->m_fileName.c_str(),
        SendHardLinkCb, cbData);
    if (res != MP_SUCCESS) {
        ERRLOG("Failed to send hardlink req for: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(),
            nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::HARDLINK, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendHardLinkCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    data = data;
    auto cbData = (NfsHardLinkCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    auto restoreReplacePolicy = cbData->restoreReplacePolicy;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::HARDLINK, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    if (status < MP_SUCCESS) {
        HandleHardLinkFailure(commonData, fileHandle, restoreReplacePolicy, status, nfs);
        return;
    }

    /* Success case */
    fileHandle.m_retryCnt = 0;
    commonData->hardlinkMap->IncreaseRef(fileHandle.m_file->m_inode);
    commonData->controlInfo->m_noOfFilesCopied++;
    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
}

void HandleHardLinkFailure(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy, int status, struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT || status == -ESTALE) {
        ERRLOG("target path not exist for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        HardlinkFailureHandling(commonData, status, fileHandle);
    } else if (status == -BACKUP_ERR_NOTDIR) {
        ERRLOG("parent path not exist: %s", fileHandle.m_file->m_fileName.c_str());
        HardlinkFailureHandling(commonData, status, fileHandle);
    } else if (status == -BACKUP_ERR_EEXIST) {
        DBGLOG("link exist: %s", fileHandle.m_file->m_fileName.c_str());
        HandleExistStatus(commonData, fileHandle, restoreReplacePolicy);
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::HARDLINK, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("Max hlink req retry reached for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            HardlinkFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("hlink retry: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::HARDLINK, PKT_COUNTER::RETRIED);
        }
    } else if (status == -BACKUP_ERR_EISDIR) {
        // We got a file creation request with same name as the directory present in BackupFS.
        // delete directory as delete entry will come later. Without deleting we cannot create file.
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("hardlink req failed for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        HardlinkFailureHandling(commonData, status, fileHandle);
    }
}

void HandleExistStatus(NfsCommonData *commonData, FileHandle &fileHandle,
    RestoreReplacePolicy restoreReplacePolicy)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    fileHandle.m_file->SetDstState(FileDescState::LINK_DEL);
    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }
}

void HardlinkFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::HARDLINK);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, false, false);
}
