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
#include "OpenRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    constexpr auto MODULE_NAME = "OpenReq";
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsOpenCbData* CreateOpenCbData(FileHandle &fileHandle, NfsCommonData &commonData)
{
    auto cbData = new(nothrow) NfsOpenCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->commonData = &commonData;

    return cbData;
}

int SendOpen(FileHandle &fileHandle, NfsOpenCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData nullptr");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->commonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send open req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = nfs->NfsOpenAsync(fileHandle.m_file->m_fileName.c_str(), O_RDONLY, SendOpenCb, cbData);
    if (res != MP_SUCCESS) {
        ERRLOG("Send Open req failed for: %s ERR: %S", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->commonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->commonData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendOpenCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsOpenCbData*)privateData;
    VALIDATE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    if (status < MP_SUCCESS) {
        HandleOpenFailure(commonData, status, nfs, fileHandle);
        return;
    }

    fileHandle.m_retryCnt = 0;
    fileHandle.m_file->srcIOHandle.nfsFh = (struct nfsfh*)data;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);

    /* Send to read queue */
    commonData->readQueue->Push(fileHandle);
    return;
}

void HandleOpenFailure(NfsCommonData *commonData, int status, struct nfs_context *nfs, FileHandle &fileHandle)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }
    if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("Open req failed for: %s Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            OpenFailureHandling(commonData, status, fileHandle);
        } else {
            WARNLOG("open req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::RETRIED);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("Open req faied for: %s, Status: %d, %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        OpenFailureHandling(commonData, status, fileHandle);
    }
}

void OpenFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    CheckForCriticalError(commonData, status, PKT_TYPE::OPEN);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_READER);

    // Push to aggregate queue & then to write queue. This is to ensure we aggregate counters are incremented correctly
    ++commonData->controlInfo->m_readProduce;
    if (commonData->aggregateQueue != nullptr) {
        commonData->aggregateQueue->Push(fileHandle);
    }
}
