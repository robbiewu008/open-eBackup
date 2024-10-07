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
#include "LinkUtimeRequest.h"

using namespace Module;
using namespace std;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsLinkUtimeCbData* CreateLinkUtimeCbData(FileHandle &fileHandle, NfsCommonData &commonData)
{
    auto cbData = new(nothrow) NfsLinkUtimeCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;

    return cbData;
}

int SendLinkUtime(FileHandle &fileHandle, NfsLinkUtimeCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    struct timeval times[ATIME_MTIME_ARRAY_LEN];
    struct timeval *timesPtr = nullptr;
    times[ATIME_STRUCT] = {time_t(fileHandle.m_file->m_atime), NUMBER_ZERO};
    times[MTIME_STRUCT] = {time_t(fileHandle.m_file->m_mtime), NUMBER_ZERO};
    timesPtr = times;

    /* todo: need write libnfs API which uses parentfh */
    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send readlink req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    if (nfs->NfsLutimeAsync(fileHandle.m_file->m_fileName.c_str(), timesPtr, SendLinkUtimeCb, cbData) != MP_SUCCESS) {
        ERRLOG("Failed to send readlink req for: %s, %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::SENT);
    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);

    return MP_SUCCESS;
}

void SendLinkUtimeCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    data = data;

    auto cbData = (NfsLinkUtimeCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

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

    if (status < MP_SUCCESS) {
        HandleLinkUtimeFailure(commonData, status, fileHandle, nfs);
        return;
    }

    fileHandle.m_retryCnt = 0;

    if (fileHandle.m_file->m_scannermode != CTRL_ENTRY_MODE_META_MODIFIED) {
        commonData->controlInfo->m_noOfFilesCopied++;

        if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
            INFOLOG("SetTargetCopied %s", fileHandle.m_file->m_fileName.c_str());
            if (commonData->hardlinkMap->IsTargetCopied(fileHandle.m_file->m_inode) == false) {
                commonData->hardlinkMap->SetTargetCopied(fileHandle.m_file->m_inode);
            }
        }
    }
}

void HandleLinkUtimeFailure(NfsCommonData *commonData, int status, FileHandle &fileHandle,
    struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT || status == -BACKUP_ERR_NOTDIR) {
        DBGLOG("parent path not exist for: %s", fileHandle.m_file->m_fileName);
    } else if (status == -BACKUP_ERR_EEXIST) {
        DBGLOG("target link exist for: %s", fileHandle.m_file->m_fileName);
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("setting mtime for req failed for: %s, Retry Count: %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            LinkUtimeFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("link utime req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_onlyFileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::RETRIED);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("setting mtime for symlink req failed for: %s, Status: %d, %s", fileHandle.m_file->m_fileName.c_str(),
            status, nfs_get_error(nfs));
        LinkUtimeFailureHandling(commonData, status, fileHandle);
    }
}

void LinkUtimeFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::SETMETA);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, false, false);
}
