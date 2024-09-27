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
#include "WriteRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;
namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsWriteCbData* CreateWriteCbData(FileHandle &fileHandle, NfsCommonData &commonData,
    shared_ptr<BlockBufferMap> blockBufferMap)
{
    auto cbData = new(nothrow) NfsWriteCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->blockBufferMap = blockBufferMap;

    return cbData;
}

int SendWrite(FileHandle &fileHandle, NfsWriteCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send write req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }

    /* We send commit only for every "MAX_BLOCKS_BEFORE_COMMIT (40)" blocks writes */
    uint64_t noOfBlocksWritten = fileHandle.m_file->m_blockStats.m_writeReqCnt;
    if ((noOfBlocksWritten % MAX_BLOCKS_BEFORE_COMMIT == 0) && noOfBlocksWritten != 0 &&
        fileHandle.m_file->GetDstState() < FileDescState::WRITED) {
        DBGLOG("Commit send for file: %s after %lu blocks send to write", fileHandle.m_file->m_fileName.c_str(),
            noOfBlocksWritten);

        NfsWriteCbData *cbDataFsync = CreateWriteCbData(fileHandle, *(cbData->writeCommonData), cbData->blockBufferMap);
        if (cbDataFsync == nullptr) {
            return MP_FAILED;
        }
        if (nfs->NfsFsyncAsync(fileHandle.m_file->dstIOHandle.nfsFh, SendFileSyncCb, cbDataFsync) != MP_SUCCESS) {
            ERRLOG("Send fsync req failed for: %s", fileHandle.m_file->m_fileName.c_str());
            delete cbDataFsync;
            cbDataFsync = nullptr;
            return MP_FAILED;
        }

        cbDataFsync->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
        cbDataFsync->writeCommonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::SENT);
    }

    if (nfs->NfsWriteAsync(fileHandle.m_file->dstIOHandle.nfsFh, fileHandle.m_block.m_offset, fileHandle.m_block.m_size,
        fileHandle.m_block.m_buffer, SendWriteCb, cbData) != MP_SUCCESS) {
        ERRLOG("Send write req failed for: %s, offset: %lu", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_offset);
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::SENT);
    return MP_SUCCESS;
}

void SendWriteCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsWriteCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);
    data = data;

    auto fileHandle = cbData->fileHandle;

    if (!IS_LIBNFS_NEED_RETRY(status) || status == -BACKUP_ERR_EISDIR) {
        fileHandle.m_file->m_blockStats.m_writeRespCnt++;
    }

    commonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        delete cbData;
        cbData = nullptr;
        return;
    }

    if (IS_FILE_COPY_FAILED(fileHandle) &&
        (fileHandle.m_file->m_blockStats.m_writeRespCnt == fileHandle.m_file->m_blockStats.m_writeReqCnt)) {
        // Send to close queue. In main thread, we manage to free blockBuffer & erase
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
        delete cbData;
        return;
    }

    if (status < MP_SUCCESS) {
        HandleWriteFailure(cbData, fileHandle, status, nfs);
        delete cbData;
        return;
    }

    // Success case
    fileHandle.m_retryCnt = 0;
    commonData->controlInfo->m_noOfBytesCopied += fileHandle.m_block.m_size;
    cbData->blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);

    if (fileHandle.m_file->m_blockStats.m_writeRespCnt == fileHandle.m_file->m_blockStats.m_totalCnt) {
        fileHandle.m_file->SetDstState(FileDescState::WRITED);
        // Send to close queue
        SendSetMeta(fileHandle, commonData);
    }
    delete cbData;
    cbData = nullptr;
}

void HandleWriteFailure(NfsWriteCbData *cbData, FileHandle &fileHandle, int status, struct nfs_context *nfs)
{
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return;
    }

    NfsCommonData *commonData = cbData->writeCommonData;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (IS_LIBNFS_NEED_RETRY(status) || status == -BACKUP_ERR_EISDIR) {
        commonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_READ_RETRY) {
            ERRLOG("WriteReq failed for: %s, Offset: %lu, retryCnt: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_offset, fileHandle.m_retryCnt, nfs_get_error(nfs));
        } else {
            DBGLOG("Write retry for: %s, Offset: %lu, retryCount: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_offset, fileHandle.m_retryCnt, nfs_get_error(nfs));
            commonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::RETRIED);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            return;
        }
    }

    WriteFailureHandling(commonData, status, fileHandle);

    ERRLOG("WriteReq failed for: %s, Offset: %lu, Status: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_offset, status, nfs_get_error(nfs));
    cbData->blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);

    // Send to close queue. In main thread, we manage to free blockBuffer & erase
    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }
    return;
}

void WriteFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::WRITE);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER, true, true);
}

void SendFileSyncCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsWriteCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);
    data =  data;

    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (status < MP_SUCCESS) {
        ERRLOG("Fsync req failed for: %s, Status: %d ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
    }
}

int SendSetMeta(FileHandle &fileHandle, NfsCommonData *commonData)
{
    if (!commonData->writeMeta) {
        commonData->controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        fileHandle.m_file->SetDstState(FileDescState::META_WRITED);
        SendCloseForAggregateFile(fileHandle, commonData);
        return MP_SUCCESS;
    }

    NfsSetMetaCbData *cbData = CreateSetMetaCbData(fileHandle, *commonData);
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::WRITE_META) != MP_SUCCESS) {
        WriteFailureHandling(commonData, 0, fileHandle);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendCloseForAggregateFile(FileHandle &fileHandle, NfsCommonData *commonData)
{
    if (!fileHandle.m_file->IsFlagSet(DST_CLOSED)) {
        fileHandle.m_file->SetFlag(DST_CLOSED);
        if (fileHandle.m_file->dstIOHandle.nfsFh == nullptr) {
            ERRLOG("dstIOHandle.nfsFh is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }

        NfsCloseCbData *cbData = CreateCloseCbData(fileHandle, *commonData, BACKUP_DIRECTION::DST);
        if (cbData == nullptr) {
            FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
            return MP_FAILED;
        }

        if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::DST_CLOSE) != MP_SUCCESS) {
            WriteFailureHandling(commonData, 0, fileHandle);
            return MP_FAILED;
        }
    }
    return MP_SUCCESS;
}