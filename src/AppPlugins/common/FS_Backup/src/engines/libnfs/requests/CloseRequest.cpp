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
#include "CloseRequest.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsCloseCbData* CreateCloseCbData(FileHandle &fileHandle, NfsCommonData &commonData, BACKUP_DIRECTION direction)
{
    auto cbData = new(nothrow) NfsCloseCbData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->fileHandle = fileHandle;
    cbData->commonData = &commonData;
    cbData->direction = direction;

    if (direction == BACKUP_DIRECTION::SRC) {
        cbData->nfsFh = fileHandle.m_file->srcIOHandle.nfsFh;
    } else if (direction == BACKUP_DIRECTION::DST) {
        cbData->nfsFh = fileHandle.m_file->dstIOHandle.nfsFh;
    }

    return cbData;
}

int SendClose(FileHandle &fileHandle, NfsCloseCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData nullptr");
        return MP_FAILED;
    }
    if (cbData->nfsFh == nullptr) {
        ERRLOG("cbData->nfsFh nullptr");
        return MP_FAILED;
    }

    NfsCommonData *commonData = cbData->commonData;
    shared_ptr<NfsContextWrapper> nfs = cbData->commonData->nfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send close req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        FreeNfsFh(cbData->nfsFh);
        ProcessCloseCompletion(fileHandle, cbData->commonData);
        delete cbData;
        return MP_FAILED;
    }
    if (nfs->NfsCloseAsync(cbData->nfsFh, SendCloseCb, cbData) != MP_SUCCESS) {
        ERRLOG("Send DstClose req failed for: %s, Err: %s", fileHandle.m_file->m_fileName.c_str(), nfs->NfsGetError());
        FreeNfsFh(cbData->nfsFh);
        ProcessCloseCompletion(fileHandle, cbData->commonData);
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }
    commonData->nfsContextContainer->IncSendCnt(REQ_CNT_PER_NFS_CONTEXT);
    commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::SENT);

    return MP_SUCCESS;
}

void SendCloseCb(int status, struct nfs_context *nfs, void *data, void *privateData)
{
    auto cbData = (NfsCloseCbData *)privateData;
    if (cbData == nullptr) {
        ERRLOG("cbData is nullptr");
        return;
    }

    data = data;
    auto fileHandle = cbData->fileHandle;

    NfsCommonData *commonData = cbData->commonData;
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        FreeNfsFh(cbData->nfsFh);
        fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
        fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
        delete cbData;
        cbData = nullptr;
        return;
    }
    delete cbData;
    cbData = nullptr;

    commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD);
    if (commonData->IsResumeSendCb(commonData->commonObj)) {
        commonData->ResumeSendCb(commonData->commonObj);
    }

    if (status < MP_SUCCESS) {
        ERRLOG("DstClose failed for: %s, Status: %d, ERR: %s", fileHandle.m_file->m_fileName.c_str(), status,
            nfs_get_error(nfs));
        commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::FAILED);
    }

    ProcessCloseCompletion(fileHandle, commonData);
}


void ProcessCloseCompletion(FileHandle &fileHandle, NfsCommonData *commonData)
{
    if (commonData == nullptr) {
        ERRLOG("commonData is nullptr");
        return;
    }

    fileHandle.m_file->dstIOHandle.nfsFh = nullptr;
    if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
        fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    }

    if (IS_FILE_COPY_FAILED(fileHandle)) {
        fileHandle.m_file->SetDstState(FileDescState::LINK_DEL_FAILED);
        if (commonData->writeQueue != nullptr) {
            commonData->writeQueue->Push(fileHandle);
        }
        return;
    }

    if (fileHandle.m_file->m_nlink > 1 && commonData->hardlinkMap != nullptr) {
        if (commonData->hardlinkMap->IsTargetCopied(fileHandle.m_file->m_inode) == false) {
            commonData->hardlinkMap->SetTargetCopied(fileHandle.m_file->m_inode);
        }
    }
}

/* ------------------------------------------------------------------
 *                   Sync Request for src close
 * ------------------------------------------------------------------ */

int SendCloseSync(FileHandle &fileHandle, NfsCloseCbData *cbData)
{
    if (cbData == nullptr) {
        ERRLOG("cbData nullptr");
        return MP_FAILED;
    }
    if (cbData->nfsFh == nullptr) {
        ERRLOG("cbData->nfsFh nullptr");
        delete cbData;
        cbData = nullptr;
        return MP_FAILED;
    }

    shared_ptr<NfsContextWrapper> nfs = cbData->commonData->syncNfsContextContainer->GetCurrContext();
    if (nfs == nullptr) {
        ERRLOG("nfs wrapper is null. Send close req failed for: %s", fileHandle.m_file->m_fileName.c_str());
        FreeNfsFh(cbData->nfsFh);
        delete cbData;
        return MP_FAILED;
    }
    cbData->commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::SENT);
    int status = nfs->NfsClose(cbData->nfsFh);
    cbData->commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD);
    if (cbData->commonData->IsResumeSendCb(cbData->commonData->commonObj)) {
        cbData->commonData->ResumeSendCb(cbData->commonData->commonObj);
    }

    if (status < MP_SUCCESS) {
        ERRLOG("SrcClose failed for: %s, Status: %d", fileHandle.m_file->m_fileName.c_str(), status);
        FreeNfsFh(cbData->nfsFh);
        cbData->commonData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::FAILED);
    }

    fileHandle.m_file->srcIOHandle.nfsFh = nullptr;
    if (fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        cbData->commonData->controlInfo->m_noOfFilesRead++;
    }
    delete cbData;
    cbData = nullptr;
    return MP_SUCCESS;
}
