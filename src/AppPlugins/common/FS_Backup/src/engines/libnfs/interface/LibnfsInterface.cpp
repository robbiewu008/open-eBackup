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
#include "LibnfsInterface.h"

using namespace std;
using namespace Libnfscommonmethods;

namespace {
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 10;
    constexpr int RETRY_WAIT_IN_SEC = 1;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

int SendNfsRequest(FileHandle &fileHandle, void *cbData, LibnfsEvent event)
{
    if (event < LibnfsEvent::LINK_UTIME) {
        return SendNfsRequest1(fileHandle, cbData, event);
    } else {
        return SendNfsRequest2(fileHandle, cbData, event);
    }
    return MP_SUCCESS;
}

int SendNfsRequest1(FileHandle &fileHandle, void *cbData, LibnfsEvent event)
{
    switch (event) {
        case LibnfsEvent::OPEN: {
            return SendOpen(fileHandle, (NfsOpenCbData *)cbData);
        }
        case LibnfsEvent::READ: {
            return SendRead(fileHandle, (NfsReadCbData *)cbData);
        }
        case LibnfsEvent::READLINK: {
            return SendReadlink(fileHandle, (NfsReadlinkCbData *)cbData);
        }
        case LibnfsEvent::SRC_CLOSE: {
            return SendCloseSync(fileHandle, (NfsCloseCbData *)cbData);
        }
        case LibnfsEvent::LSTAT: {
            return SendLstat(fileHandle, (NfsLstatCbData *)cbData);
        }
        case LibnfsEvent::CREATE: {
            return SendCreate(fileHandle, (NfsCreateCbData *)cbData);
        }
        case LibnfsEvent::WRITE: {
            return SendWrite(fileHandle, (NfsWriteCbData *)cbData);
        }
        case LibnfsEvent::WRITE_META: {
            return SendSetMeta(fileHandle, (NfsSetMetaCbData *)cbData);
        }
        case LibnfsEvent::MKNOD: {
            return SendMknod(fileHandle, (NfsMknodCbData *)cbData);
        }
        case LibnfsEvent::SYMLINK: {
            return SendSymLink(fileHandle, (NfsSymLinkCbData *)cbData);
        }
        default: {
            break;
        }
    }
    return MP_SUCCESS;
}

int SendNfsRequest2(FileHandle &fileHandle, void *cbData, LibnfsEvent event)
{
    switch (event) {
        case LibnfsEvent::LINK_UTIME: {
            return SendLinkUtime(fileHandle, (NfsLinkUtimeCbData *)cbData);
        }
        case LibnfsEvent::HARDLINK: {
            return SendHardLink(fileHandle, (NfsHardLinkCbData *)cbData);
        }
        case LibnfsEvent::MKDIR: {
            return SendMkdir(fileHandle, (NfsMkdirCbData *)cbData);
        }
        case LibnfsEvent::DIR_DELETE: {
            return SendDirDelete(fileHandle, (NfsDirDeleteCbData *)cbData);
        }
        case LibnfsEvent::LINK_DELETE: {
            return SendLinkDeleteSync(fileHandle, (NfsLinkDeleteCbData *)cbData);
        }
        case LibnfsEvent::LINK_DELETE_FOR_RESTORE: {
            return SendLinkDelete(fileHandle, (NfsLinkDeleteCbData *)cbData);
        }
        case LibnfsEvent::DST_CLOSE: {
            return SendClose(fileHandle, (NfsCloseCbData *)cbData);
        }
        default: {
            break;
        }
    }
    return MP_SUCCESS;
}

int SendOpenRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsOpenCbData *cbData = CreateOpenCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::OPEN) != MP_SUCCESS) {
        HandleSendReaderNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendReadRequest(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    NfsReadCbData *cbData = CreateReadCbData(fileHandle, commonData, blockBufferMap);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::READ) != MP_SUCCESS) {
        HandleSendReaderNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendReadLinkRequest(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    NfsReadlinkCbData *cbData = CreateReadlinkCbData(fileHandle, commonData, blockBufferMap);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::READLINK) != MP_SUCCESS) {
        HandleSendReaderNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendSrcCloseRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsCloseCbData *cbData = CreateCloseCbData(fileHandle, commonData, BACKUP_DIRECTION::SRC);
    if (cbData == nullptr) {
        Libnfscommonmethods::FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
        return MP_FAILED;
    }

    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::SRC_CLOSE) != MP_SUCCESS) {
        HandleSendReaderNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendCreateRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, uint32_t openFlag, NfsCommonData &commonData,
    BackupParams backupParams)
{
    NfsCreateCbData *cbData = CreateCreateCbData(fileHandle, commonData, nfsfh, openFlag,
        backupParams.commonParams.restoreReplacePolicy);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::CREATE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendWriteRequest(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    NfsWriteCbData *cbData = CreateWriteCbData(fileHandle, commonData, blockBufferMap);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::WRITE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendSetMetaRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsSetMetaCbData *cbData = CreateSetMetaCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::WRITE_META) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendLstatRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData, BackupParams backupParams)
{
    NfsLstatCbData *cbData = CreateLstatCbData(fileHandle, commonData, nfsfh,
        backupParams.commonParams.restoreReplacePolicy);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::LSTAT) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendSymlinkRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData,
    std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    NfsSymLinkCbData *cbData = CreateSymLinkCbData(fileHandle, commonData, nfsfh, blockBufferMap);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::SYMLINK) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendSymlinkUtimeRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsLinkUtimeCbData *cbData = CreateLinkUtimeCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::LINK_UTIME) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendMknodRequest(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData)
{
    NfsMknodCbData *cbData = CreateMknodCbData(fileHandle, commonData, nfsfh);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::MKNOD) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendHardlinkRequest(FileHandle &fileHandle, string targetPath, struct nfsfh *nfsfh, NfsCommonData &commonData,
    BackupParams backupParams)
{
    NfsHardLinkCbData *cbData = CreateHardLinkCbData(fileHandle, commonData, nfsfh,
        backupParams.commonParams.restoreReplacePolicy, targetPath);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::HARDLINK) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendDstCloseRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsCloseCbData *cbData = CreateCloseCbData(fileHandle, commonData, BACKUP_DIRECTION::DST);
    if (cbData == nullptr) {
        Libnfscommonmethods::FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
        return MP_FAILED;
    }

    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::DST_CLOSE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendLinkDeleteRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsLinkDeleteCbData *cbData = CreateLinkDeleteCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::LINK_DELETE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendLinkDeleteRequestForRestore(FileHandle &fileHandle, struct nfsfh *nfsfh, NfsCommonData &commonData)
{
    NfsLinkDeleteCbData *cbData = CreateLinkDeleteCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    cbData->nfsfh = nfsfh;
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::LINK_DELETE_FOR_RESTORE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int SendDirDeleteRequest(FileHandle &fileHandle, NfsCommonData &commonData)
{
    NfsDirDeleteCbData *cbData = CreateDirDeleteCbData(fileHandle, commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::DIR_DELETE) != MP_SUCCESS) {
        HandleSendWriterNfsRequestFailure(fileHandle, commonData);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int ReadLink(FileHandle &fileHandle, NfsCommonData &commonData, shared_ptr<BlockBufferMap> blockBufferMap)
{
    return SendReadLinkRequest(fileHandle, commonData, blockBufferMap);
}

int WriteSymLinkMeta(FileHandle &fileHandle, NfsCommonData &commonData)
{
    return SendSymlinkUtimeRequest(fileHandle, commonData);
}

int LstatFile(FileHandle &fileHandle, NfsCommonData &commonData, BackupParams backupParams,
    std::shared_ptr<FileHandleCache> fileHandleCache)
{
    DBGLOG("LstatFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    return SendLstatRequest(fileHandle, nfsfh, commonData, backupParams);
}

int CreateSymlink(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<FileHandleCache> fileHandleCache,
    std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    return SendSymlinkRequest(fileHandle, nfsfh, commonData, blockBufferMap);
}

int CreateSpecialFile(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<FileHandleCache> fileHandleCache)
{
    auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    return SendMknodRequest(fileHandle, nfsfh, commonData);
}

int CreateHardlink(FileHandle &fileHandle, NfsCommonData &commonData, BackupParams backupParams,
    std::shared_ptr<FileHandleCache> fileHandleCache)
{
    auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    string targetPath {};
    if (commonData.hardlinkMap->GetTargetPath(fileHandle.m_file->m_inode, targetPath) != MP_SUCCESS) {
        FSBackupUtils::RecordFailureDetail(commonData.failureRecorder, fileHandle.m_file->m_fileName, ENOLINK);
        commonData.controlInfo->m_noOfFilesFailed++;
        ERRLOG("Target create failed for: %s. target: %s, totalFailed: %llu",
            fileHandle.m_file->m_fileName.c_str(), targetPath.c_str(),
            commonData.controlInfo->m_noOfFilesFailed.load());
        if (!commonData.skipFailure) {
            commonData.controlInfo->m_failed = true;
        }
        return MP_FAILED;
    }

    /* Until Target file is completely copied, don't create a hardlink to it */
    if (commonData.hardlinkMap->IsTargetCopied(fileHandle.m_file->m_inode) == false) {
        commonData.writeWaitQueue->Push(fileHandle);
        return MP_SUCCESS;
    }

    return SendHardlinkRequest(fileHandle, targetPath, nfsfh, commonData, backupParams);
}

int LinkDelete(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<BlockBufferMap> blockBufferMap)
{
    if (!IS_TO_BE_OPENED(fileHandle)) {
        if (fileHandle.m_file->GetDstState() == FileDescState::LINK_DEL_FAILED) {
            /* This is fileHandle for write of failed file. This is already closed and send for deletion.
             * So free the buffer and ignore further processing */
            WARNLOG("remove link for file: %s, %llu", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_file->m_inode);
            if (commonData.hardlinkMap != nullptr) {
                commonData.hardlinkMap->RemoveLink(fileHandle);
                commonData.hardlinkMap->RemoveElement(fileHandle.m_file->m_inode);
            }
            blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        } else {
            commonData.writeWaitQueue->Push(fileHandle);
        }
        return MP_SUCCESS;
    }
    if (IS_EMPTY_SYMLINK_BLOCK(fileHandle)) {
        /* In aggregate restore, an empty block is produced for symlink also, inline with other files.
         * This doesn't have the target path stored in data buffer. Has to be ignored */
        return MP_SUCCESS;
    }

    return SendLinkDeleteRequest(fileHandle, commonData);
}

int LinkDeleteForRestore(FileHandle &fileHandle, NfsCommonData &commonData,
    std::shared_ptr<FileHandleCache> fileHandleCache)
{
    if (!IS_TO_BE_OPENED(fileHandle)) {
        commonData.writeWaitQueue->Push(fileHandle);
        return MP_SUCCESS;
    }
    if (IS_EMPTY_SYMLINK_BLOCK(fileHandle)) {
        /* In aggregate restore, an empty block is produced for symlink also, inline with other files.
         * This doesn't have the target path stored in data buffer. Has to be ignored */
        return MP_SUCCESS;
    }
    auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    return SendLinkDeleteRequestForRestore(fileHandle, nfsfh, commonData);
}

int DirectoryDelete(FileHandle &fileHandle, NfsCommonData &commonData)
{
    if (!IS_TO_BE_OPENED(fileHandle)) {
        commonData.writeWaitQueue->Push(fileHandle);
        return MP_SUCCESS;
    }
    if (IS_EMPTY_SYMLINK_BLOCK(fileHandle)) {
        /* In aggregate restore, an empty block is produced for symlink also, inline with other files.
         * This doesn't have the target path stored in data buffer. Has to be ignored */
        return MP_SUCCESS;
    }
    DBGLOG("DirectoryDelete: %s", fileHandle.m_file->m_fileName.c_str());

    return SendDirDeleteRequest(fileHandle, commonData);
}

int MakeDirectory(FileHandle &fileHandle, NfsCommonData &commonData, std::shared_ptr<FileHandleCache> fileHandleCache)
{
    uint16_t retryCnt = 0;
    int ret = MP_FAILED;
    do {
        if (IsAbort(commonData)) {
            INFOLOG("Breaking after Abort");
            return MP_SUCCESS;
        }

        auto nfsfh = fileHandleCache->Get(fileHandle.m_file->m_dirName);
        if (ProcessDirParentFh(fileHandle.m_file->m_fileName, fileHandle.m_file->m_dirName, nfsfh,
            retryCnt) != MP_SUCCESS) {
            break;
        }

        NfsMkdirCbData *cbData = CreateMkdirCbData(fileHandle, commonData, retryCnt, fileHandleCache, nfsfh);
        if (cbData == nullptr) {
            return MP_FAILED;
        }
        ret = SendNfsRequest(fileHandle, cbData, LibnfsEvent::MKDIR);
        if (ret != MP_SUCCESS) {
            DBGLOG("Directory creation failed for dir: %s, retrying: %d", fileHandle.m_file->m_fileName.c_str(),
                retryCnt);
            sleep(RETRY_WAIT_IN_SEC);
        }
        delete cbData;
        cbData = nullptr;
        retryCnt++;
    } while ((ret != MP_SUCCESS) && (retryCnt <= DEFAULT_MAX_REQUEST_RETRY));

    if (ret != MP_SUCCESS) {
        ERRLOG("mkdir failed: %s, retry: %d", fileHandle.m_file->m_fileName.c_str(), retryCnt);
        FillFileHandleCacheWithInvalidDirectoryFh(fileHandle.m_file->m_fileName, fileHandleCache);
        commonData.controlInfo->m_noOfDirFailed++;
        if (!commonData.skipFailure) {
            commonData.controlInfo->m_failed = true;
        }
        commonData.pktStats->Increment(PKT_TYPE::MKDIR, PKT_COUNTER::FAILED, OFFSET_10, PKT_ERROR::RETRIABLE_ERR);
    }

    return MP_SUCCESS;
}

void PushToAggregator(FileHandle& fileHandle, NfsCommonData &commonData)
{
    if (!commonData.writeDisable) {
        while (!commonData.aggregateQueue->WaitAndPush(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
            if (IsAbort(commonData)) {
                WARNLOG("LibnfsCopyReader abort!");
                return;
            }
        }
        ++commonData.controlInfo->m_readProduce;
    }
    return;
}

void HandleZeroSizeFileRead(FileHandle &fileHandle, NfsCommonData &commonData,
    shared_ptr<BlockBufferMap> blockBufferMap)
{
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;

    blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);

    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    commonData.controlInfo->m_noOfFilesRead++;

    // Push to aggregate queue & then to write queue
    if (!commonData.writeDisable) {
        PushToAggregator(fileHandle, commonData);
    } else {
        blockBufferMap->Delete(fileHandle.m_file->m_fileName);
        commonData.controlInfo->m_noOfFilesCopied++;
    }
}

void HandleSendWriterNfsRequestFailure(FileHandle &fileHandle, NfsCommonData &commonData)
{
    if (IS_INCREMENT_FAIL_COUNT(fileHandle)) {
        FSBackupUtils::RecordFailureDetail(commonData.failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        commonData.controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
        ERRLOG("write file failed. %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
            commonData.controlInfo->m_noOfFilesFailed.load());
    }

    if (!commonData.skipFailure) {
        commonData.controlInfo->m_failed = true;
    }

    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
}

void HandleSendReaderNfsRequestFailure(FileHandle &fileHandle, NfsCommonData &commonData)
{
    if (IS_INCREMENT_FAIL_COUNT(fileHandle)) {
        FSBackupUtils::RecordFailureDetail(commonData.failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        commonData.controlInfo->m_noOfFilesFailed++;
        ERRLOG("read file failed: %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
            commonData.controlInfo->m_noOfFilesFailed.load());
    }

    if (!commonData.skipFailure) {
        commonData.controlInfo->m_failed = true;
    }

    if (IS_INCREMENT_READ_FAIL_COUNT(fileHandle)) {
        FSBackupUtils::RecordFailureDetail(commonData.failureRecorder, fileHandle.m_file->m_fileName, EINVAL);
        commonData.controlInfo->m_noOfFilesReadFailed++;
    }

    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);

    // Push to aggregate queue & then to write queue. This is to ensure we free the blocks in blockBufferMap
    PushToAggregator(fileHandle, commonData);
}