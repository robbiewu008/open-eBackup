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
#include "SymLinkRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsSymLinkCbData* CreateSymLinkCbData(FileHandle &fileHandle, NfsCommonData &commonData, struct nfsfh* nfsfh,
    std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    auto cbData = new(nothrow) NfsSymLinkCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->writeCommonData = &commonData;
    cbData->nfsfh = nfsfh;
    cbData->blockBufferMap = blockBufferMap;

    return cbData;
}

int SendSymLink(FileHandle &fileHandle, NfsSymLinkCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return MP_FAILED;
    }

    if (fileHandle.m_block.m_buffer == nullptr) {
        ERRLOG("Target path is empty. link: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    /* todo: need write libnfs API which uses parentfh */
    shared_ptr<NfsContextWrapper> nfs = cbData->writeCommonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send symlink req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        return MP_FAILED;
    }
    int res = nfs->NfsSymLinkAsync((const char *)fileHandle.m_block.m_buffer,
        fileHandle.m_file->m_fileName.c_str(), SendSymLinkCb, cbData);
    if (res != MP_SUCCESS) {
        ERRLOG("Failed to send readlink req: %s, ERR: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    cbData->writeCommonData->pktStats->Increment(PKT_TYPE::SYMLINK, PKT_COUNTER::SENT);
    cbData->writeCommonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    return MP_SUCCESS;
}

void SendSymLinkCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    data = data;
    auto cbData = (NfsSymLinkCbData *)privateData;
    VALIDATE_WRITE_COMMON_DATA_PTR_RECEIVED(cbData);

    std::shared_ptr<BlockBufferMap> blockBufferMap = cbData->blockBufferMap;
    auto fileHandle = cbData->fileHandle;
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::SYMLINK, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (IS_ABORTED_OR_FAILED(commonData)) {
        return;
    }

    if (status < MP_SUCCESS) {
        HandleSymLinkFailure(commonData, fileHandle, status, nfs);
        return;
    }

    fileHandle.m_retryCnt = 0;

    INFOLOG("Delete blockSize %lu, for file: %s", fileHandle.m_block.m_size, fileHandle.m_file->m_fileName.c_str());
    blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);

    fileHandle.m_file->SetDstState(FileDescState::WRITED);
    if (commonData->writeQueue != nullptr) {
        commonData->writeQueue->Push(fileHandle);
    }
}

void HandleSymLinkFailure(NfsCommonData *commonData, FileHandle &fileHandle, int status,
    struct nfs_context *nfs)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    if (status == -BACKUP_ERR_ENOENT) {
        DBGLOG("target path not exist: %s", fileHandle.m_file->m_fileName.c_str());
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (status == -BACKUP_ERR_NOTDIR) {
        DBGLOG("parent path not exist: %s", fileHandle.m_file->m_fileName.c_str());
        SymlinkFailureHandling(commonData, status, fileHandle);
    } else if (status == -BACKUP_ERR_EEXIST) {
        DBGLOG("target link exist: %s", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetDstState(FileDescState::LINK_DEL);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (IS_LIBNFS_NEED_RETRY(status)) {
        commonData->pktStats->Increment(PKT_TYPE::SYMLINK, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        fileHandle.m_retryCnt++;
        if (fileHandle.m_retryCnt > DEFAULT_MAX_REQUEST_RETRY) {
            ERRLOG("symlink req failed for: %s, Retry Count : %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            SymlinkFailureHandling(commonData, status, fileHandle);
        } else {
            commonData->controlInfo->m_noOfDstRetryCount++;
            DBGLOG("SymLink req enqueue for: %s Retry Count = %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_retryCnt);
            commonData->timer->Insert(fileHandle, fileHandle.m_retryCnt * DEFAULT_REQUEST_RETRY_TIMER);
            commonData->pktStats->Increment(PKT_TYPE::SYMLINK, PKT_COUNTER::RETRIED);
        }
    } else if (status == -BACKUP_ERR_EISDIR) {
        // We got a file creation request with same name as the directory present in BackupFS.
        // delete directory as delete entry will come later. Without deleting we cannot create file.
        commonData->pktStats->Increment(PKT_TYPE::SYMLINK, PKT_COUNTER::FAILED);
        fileHandle.m_file->SetDstState(FileDescState::DIR_DEL);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
    } else if (status < MP_SUCCESS) {
        ERRLOG("symlink req failed for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        SymlinkFailureHandling(commonData, status, fileHandle);
    }
}

void SymlinkFailureHandling(NfsCommonData *commonData, int status, FileHandle &fileHandle)
{
    CheckForCriticalError(commonData, status, PKT_TYPE::SYMLINK);
    RequestFailHandleAndCleanLinkMap(fileHandle, commonData, LIBNFS_WRITER);
}
