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
#include "ReadRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    constexpr auto MODULE_NAME = "ReadRequest";
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsReadCbData* CreateReadCbData(FileHandle &fileHandle, NfsCommonData &commonData,
    shared_ptr<BlockBufferMap> blockBufferMap)
{
    auto cbData = new(nothrow) NfsReadCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->commonData = &commonData;
    cbData->blockBufferMap = blockBufferMap;

    return cbData;
}

int SendRead(FileHandle &fileHandle, NfsReadCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData nullptr");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->commonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send read req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    if (nfs->NfsReadFileAsync(fileHandle.m_file->srcIOHandle.nfsFh, fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_offset, fileHandle.m_block.m_size, SendReadCb, cbData) != 0) {
        ERRLOG("Failed to send read req: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->commonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->commonData->pktStats->IncrementRead(PKT_COUNTER::SENT, fileHandle.m_block.m_size);
    return MP_SUCCESS;
}

void SendReadCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsReadCbData *)privateData;
    VALIDATE_COMMON_DATA_PTR_RECEIVED(cbData);

    auto fileHandle = cbData->fileHandle;

    if (!IS_LIBNFS_NEED_RETRY(status)) {
        fileHandle.m_file->m_blockStats.m_readRespCnt++;
    }

    commonData->pktStats->IncrementRead(PKT_COUNTER::RECVD, fileHandle.m_block.m_size);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        if (status == MP_SUCCESS) {
            struct nfs_mcb_read_data* readData = (struct nfs_mcb_read_data*)data;
            if (readData->nfsfh != nullptr) {
                FreeNfsFh(readData->nfsfh);
            }
        }
        delete cbData;
        cbData = nullptr;
        return;
    }

    if ((IS_FILE_COPY_FAILED(fileHandle) || fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP) &&
        (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_readReqCnt)) {
        // Send close
        SendCloseFile(fileHandle, commonData);

        // Push to aggregate queue & then to write queue. This is to ensure we free the blocks in blockBufferMap
        ++commonData->controlInfo->m_readProduce;
        if (commonData->aggregateQueue != nullptr) {
            commonData->aggregateQueue->Push(fileHandle);
        }
        delete cbData;
        cbData = nullptr;
        return;
    }

    if (status < MP_SUCCESS) {
        HandleReadFailure(cbData, fileHandle, status, nfs);
        delete cbData;
        cbData = nullptr;
        return;
    }

    fileHandle.m_retryCnt = 0;

    HandleReadSuccess(cbData, fileHandle, data, status);
    delete cbData;
    cbData = nullptr;
    return;
}

void HandleReadSuccess(NfsReadCbData *cbData, FileHandle &fileHandle, void *data, int status)
{
    if (!ValidateCommonDataPtr(cbData)) {
        return;
    }

    NfsCommonData *commonData = cbData->commonData;

    struct nfs_mcb_read_data* readData = (struct nfs_mcb_read_data*)data;
    uint64_t blockSize = fileHandle.m_block.m_size;

    int ret = memcpy_s(fileHandle.m_block.m_buffer, blockSize, readData->data, blockSize);
    if (ret != 0) {
        ERRLOG("memcpy of memoryBlock failed. Err: %d, Filename: %s, offset: %lu, BlockSize: %lu", ret,
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_offset, blockSize);
        ReadFailureHandling(cbData, status, fileHandle);
        return;
    }

    if (readData->nfsfh != nullptr) {
        fileHandle.m_file->SetFlag(BADHANDLE_ERR_HIT);
        if (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_readReqCnt) {
            FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
            fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
            fileHandle.m_file->srcIOHandle.nfsFh = readData->nfsfh;
            WARNLOG("Invalid fh replaced for: %s", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->ClearFlag(BADHANDLE_ERR_HIT);
        } else {
            FreeNfsFh(readData->nfsfh);
            readData->nfsfh = nullptr;
        }
    }

    if (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_totalCnt) {
        fileHandle.m_file->SetSrcState(FileDescState::READED);
        // Send close
        SendCloseFile(fileHandle, commonData);
        if (commonData->writeDisable) {
            // Disabled write
            commonData->controlInfo->m_noOfFilesCopied++;
            commonData->controlInfo->m_noOfBytesCopied += fileHandle.m_file->m_size;
        }
    }

    if (!commonData->writeDisable) {
        /* Push to aggregate queue & then to write queue */
        ++commonData->controlInfo->m_readProduce;
        if (commonData->aggregateQueue != nullptr) {
            commonData->aggregateQueue->Push(fileHandle);
        }
    } else {
        // Disabled write
        cbData->blockBufferMap->Delete(fileHandle.m_file->m_fileName);
    }
}

void HandleReadFailure(NfsReadCbData *cbData, FileHandle &fileHandle, int status, struct nfs_context *nfs)
{
    if (!ValidateCommonDataPtr(cbData)) {
        return;
    }

    NfsCommonData *commonData = cbData->commonData;

    if (IS_LIBNFS_RETRIABLE_ERROR(status)) {
        commonData->pktStats->Increment(PKT_TYPE::READ, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_READ_RETRY) {
            ERRLOG("Read req failed for: %s, Offset: %lu, after max retry. Retry Count : %d Status: %d, ERR: %s",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_offset, fileHandle.m_retryCnt, status,
                nfs_get_error(nfs));
            ReadFailureHandling(cbData, status, fileHandle);
        } else {
            WARNLOG("Read retry for: %s,  Offset: %lu, Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_offset, fileHandle.m_retryCnt);
            commonData->pktStats->Increment(PKT_TYPE::READ, PKT_COUNTER::RETRIED);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
        }
    } else {
        ERRLOG("Read req failed for: %s,  Offset: %lu, Retry Count = %d, Status: %d, ERR: %s",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_offset, fileHandle.m_retryCnt, status,
            nfs_get_error(nfs));
        ReadFailureHandling(cbData, status, fileHandle);
    }

    return;
}

void ReadFailureHandling(NfsReadCbData *cbData, int status, FileHandle &fileHandle)
{
    if (!ValidateCommonDataPtr(cbData)) {
        return;
    }

    NfsCommonData *commonData = cbData->commonData;

    CheckForCriticalError(commonData, status, PKT_TYPE::READ);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_READER);

    // Send close
    if (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_readReqCnt) {
        SendCloseFile(fileHandle, commonData);
    }

    // Push to aggregate queue & then to write queue. This is to ensure we free the blocks in blockBufferMap
    ++commonData->controlInfo->m_readProduce;
    if (commonData->aggregateQueue != nullptr) {
        commonData->aggregateQueue->Push(fileHandle);
    }
}

int SendCloseFile(FileHandle &fileHandle, NfsCommonData *commonData)
{
    if (!fileHandle.m_file->IsFlagSet(SRC_CLOSED)) {
        fileHandle.m_file->SetFlag(SRC_CLOSED);
        if (fileHandle.m_file->srcIOHandle.nfsFh == nullptr) {
            ERRLOG("srcIOHandle.nfsFh is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }
        NfsCloseCbData *cbData = CreateCloseCbData(fileHandle, *commonData, BACKUP_DIRECTION::SRC);
        if (cbData == nullptr) {
            ERRLOG("cbData is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
            return FAILED;
        }

        if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::SRC_CLOSE) != MP_SUCCESS) {
            ERRLOG("Send close failed for: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}

bool ValidateCommonDataPtr(NfsReadCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return false;
    }

    NfsCommonData *commonData = cbData->commonData;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return false;
    }

    return true;
}
