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
#include "ReadLinkRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsReadlinkCbData* CreateReadlinkCbData(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    auto cbData = new(nothrow) NfsReadlinkCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->commonData = &commonData;
    cbData->blockBufferMap = blockBufferMap;

    return cbData;
}

int SendReadlink(FileHandle &fileHandle, NfsReadlinkCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData nullptr");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->commonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send readlink req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = nfs->NfsReadLinkAsync(fileHandle.m_file->m_fileName.c_str(), SendReadlinkCb, cbData);
    if (res != MP_SUCCESS) {
        ERRLOG("Failed to send readlink req: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->commonData->pktStats->Increment(PKT_TYPE::READLINK, PKT_COUNTER::SENT);
    cbData->commonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);

    return MP_SUCCESS;
}

void SendReadlinkCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsReadlinkCbData *)privateData;
    VALIDATE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;
    commonData->pktStats->Increment(PKT_TYPE::READLINK, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (IS_LIBNFS_RETRIABLE_ERROR(status)) {
        commonData->pktStats->Increment(PKT_TYPE::READLINK, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            HandleReadlinkFailure(commonData, status, fileHandle);
        } else {
            DBGLOG("readlink req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::READLINK, PKT_COUNTER::RETRIED);
        }
        delete cbData;
        cbData = nullptr;
        return;
    } else if (status < MP_SUCCESS) {
        ERRLOG("Readlink req failed for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        HandleReadlinkFailure(commonData, status, fileHandle);
        delete cbData;
        return;
    }

    if (HandleReadlinkSuccess(cbData, fileHandle, data) != MP_SUCCESS) {
        HandleReadlinkFailure(commonData, status, fileHandle);
    }
    delete cbData;
    return;
}

int HandleReadlinkSuccess(NfsReadlinkCbData *cbData, FileHandle &fileHandle, void *data)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }

    NfsCommonData *commonData = cbData->commonData;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return MP_FAILED;
    }

    fileHandle.m_retryCnt = 0;

    /* Need to create a block for symlink as we need to aggregate symlinks also if aggregation is enabled */
    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_file->m_size + 1];
    if (fileHandle.m_block.m_buffer == nullptr) {
        ERRLOG("memalloc failed for Filename: %s, BlockSize: %lu",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size);
        HandleReadlinkFailure(commonData, 0, fileHandle);
        return MP_FAILED;
    }
    fileHandle.m_block.m_size = fileHandle.m_file->m_size;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    cbData->blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);

    int ret = memcpy_s(fileHandle.m_block.m_buffer, fileHandle.m_file->m_size, (char*)data,
        fileHandle.m_file->m_size);
    if (ret != 0) {
        ERRLOG("memcpy of memoryBlock failed. Err: %d, Filename: %s, BlockSize: %lu", ret,
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size);
        HandleReadlinkFailure(commonData, 0, fileHandle);
        return MP_FAILED;
    }

    fileHandle.m_block.m_buffer[fileHandle.m_file->m_size] = '\0';

    fileHandle.m_file->SetSrcState(FileDescState::READED);

    commonData->controlInfo->m_noOfFilesRead++;

    /* Push to write queue */
    if (!commonData->writeDisable) {
        ++commonData->controlInfo->m_readProduce;
        if (commonData->aggregateQueue != nullptr) {
            commonData->aggregateQueue->Push(fileHandle);
        }
    } else {
        commonData->controlInfo->m_noOfFilesCopied++;
    }

    return MP_SUCCESS;
}

void HandleReadlinkFailure(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    ERRLOG("readlink failed for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);

    CheckForCriticalError(commonData, status, PKT_TYPE::READLINK);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_READER);
}
