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
#include "SetMetaRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsSetMetaCbData* CreateSetMetaCbData(FileHandle &fileHandle, NfsCommonData &commonData)
{
    auto cbData = new(nothrow) NfsSetMetaCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;

    return cbData;
}

int SendSetMeta(FileHandle &fileHandle, NfsSetMetaCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    struct nfs_stat_64 allMetaData {};
    allMetaData.nfs_mode = fileHandle.m_file->m_mode;
    allMetaData.nfs_uid = fileHandle.m_file->m_uid;
    allMetaData.nfs_gid = fileHandle.m_file->m_gid;
    allMetaData.nfs_atime = fileHandle.m_file->m_atime;
    allMetaData.nfs_mtime = fileHandle.m_file->m_mtime;

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send setmeta req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = MP_FAILED;
    if (fileHandle.m_file->dstIOHandle.nfsFh == nullptr) {
        DBGLOG("Applying all metadata for file: %s", fileHandle.m_file->m_fileName.c_str());
        res = nfs->NfsFchmodChownUtimeAsync(fileHandle.m_file->m_fileName.c_str(),
            allMetaData, MetaModifiedCb, cbData);
    } else {
        res = nfs->NfsChmodChownUtimeAsync(fileHandle.m_file->dstIOHandle.nfsFh,
            fileHandle.m_file->m_fileName.c_str(), allMetaData, SendSetMetaCb, cbData);
    }

    if (res != MP_SUCCESS) {
        ERRLOG("Send Chown req failed for: %s  Error: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendSetMetaCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsSetMetaCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);
    data = data;

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        if (fileHandle.m_file->dstIOHandle.nfsFh  != nullptr) {
            FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
            fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
        }
        return;
    }

    if (status < MP_SUCCESS) {
        HandleSetMetaFailure(commonData, fileHandle, status, nfs);
        return;
    }

    fileHandle.m_retryCnt = 0;

    if (!fileHandle.m_file->IsFlagSet(IS_DIR) && (fileHandle.m_file->dstIOHandle.nfsFh != nullptr)) {
        fileHandle.m_file->SetDstState(FileDescState::META_WRITED);
        commonData->controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        SendDstCloseFile(fileHandle, commonData);
    }
}

void MetaModifiedCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsSetMetaCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);
    data =  data;

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    if (status == -BACKUP_ERR_ENOENT) {
        DBGLOG("chown file not exist: %s", fileHandle.m_file->m_fileName.c_str());
        return;
    } else if (status < MP_SUCCESS) {
        ERRLOG("chown failed for %s, Status: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        return;
    }

    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->SetDstState(FileDescState::META_WRITED);

    if (fileHandle.m_file->m_scannermode == CTRL_ENTRY_MODE_META_MODIFIED) {
        if (S_ISDIR(fileHandle.m_file->m_mode)) {
            commonData->controlInfo->m_noOfDirCopied++;
        } else {
            commonData->controlInfo->m_noOfFilesCopied++;
        }
    }
}

void HandleSetMetaFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status, struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT) {
        DBGLOG("file not exist: %s", fileHandle.m_file->m_fileName.c_str());
        SetMetaFailureHandling(commonData, status, fileHandle);
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("chown req failed for: %s Retry Count : %d", fileHandle.m_file->m_onlyFileName.c_str(),
                fileHandle.m_retryCnt);
            SetMetaFailureHandling(commonData, status, fileHandle);
        } else {
            DBGLOG("Chown req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::RETRIED);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("chown failed for %s, Status: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        SetMetaFailureHandling(commonData, status, fileHandle);
    }
}

void SetMetaFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::SETMETA);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, false, false);
    if (fileHandle.m_file->m_scannermode != CTRL_ENTRY_MODE_META_MODIFIED) {
        commonData->controlInfo->m_noOfBytesCopied -= fileHandle.m_file->m_size;
    }
    SendDstCloseFile(fileHandle, commonData);
}

int SendDstCloseFile(FileHandle &fileHandle, NfsCommonData *commonData)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }
    if (!fileHandle.m_file->IsFlagSet(DST_CLOSED)) {
        fileHandle.m_file->SetFlag(DST_CLOSED);
        if (fileHandle.m_file->dstIOHandle.nfsFh == nullptr) {
            ERRLOG("dstIOHandle.nfsFh is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }

        NfsCloseCbData *closeCbData = CreateCloseCbData(fileHandle, *commonData, BACKUP_DIRECTION::DST);
        if (closeCbData == nullptr) {
            FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
            return MP_FAILED;
        }

        if (SendNfsRequest(fileHandle, closeCbData, LibnfsEvent::DST_CLOSE) != MP_SUCCESS) {
            if (IS_INCREMENT_FAIL_COUNT(fileHandle)) {
                FSBackupUtils::RecordFailureDetail(commonData->failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
                commonData->controlInfo->m_noOfFilesFailed++;
                ERRLOG("request file failed: %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
                    commonData->controlInfo->m_noOfFilesFailed.load());
            }

            if (!commonData->skipFailure) {
                commonData->controlInfo->m_failed = true;
            }

            fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}
