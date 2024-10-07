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
#include "LibsmbReaderInterface.h"
#include <fcntl.h>
#include "log/Log.h"
#include "Backup.h"
#include "BackupConstants.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace  {
    constexpr int ADS_FILE_SUFFIX_LENGTH = 6;
    const int ONE_THOUSAND_UNIT_CONVERSION = 1000;
}

int SendReaderRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData, LibsmbEvent event)
{
    DBGLOG("Reader Send %s Request: %s, seq: %d",
        GetLibsmbEventName(event).c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_seq);
    if (cbData == nullptr || cbData->pktStats == nullptr) {
        ERRLOG("cbData is nullptr!");
        return FAILED;
    }
    cbData->event = event;
    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(event), PKT_COUNTER::SENT);
    int retVal = 0;
    switch (event) {
        case LibsmbEvent::OPEN_SRC:
            retVal = SendOpenRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::READ:
            retVal = SendReadRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::CLOSE_SRC:
            retVal = SendCloseRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::ADS:
            retVal = SendAdsRequest(fileHandle, cbData);
            break;
        default:
            break;
    }
    if (retVal != SUCCESS) {
        cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(cbData->event), PKT_COUNTER::RECVD);
        delete cbData;
        cbData = nullptr;
        return FAILED;
    }
    return SUCCESS;
}

void ReaderCallBack(struct smb2_context *smb2, int status, void *data, void *privateData)
{
    auto cbData = static_cast<SmbReaderCommonData *>(privateData);
    if (cbData == nullptr || cbData->fileHandle.m_file == nullptr || cbData->pktStats == nullptr) {
        if (cbData != nullptr) {
            delete cbData;
            cbData = nullptr;
            ERRLOG("ReaderCallBack failed: fileHandle.m_file or cbData->pktStats is nullptr, status:%d", status);
        } else {
            ERRLOG("ReaderCallBack failed: cbData is nullptr, status:%d", status);
        }
        return;
    }
    if (status < 0 && !IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        // 统计失败计数，ServerCheck用
        CheckStatusAndIncStat(status, LibsmbEventToPKT_TYPE(cbData->event), cbData->pktStats);
        FSBackupUtils::RecordFailureDetail(cbData->failureRecorder, cbData->fileHandle.m_file->m_fileName, -status);
    }
    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(cbData->event), PKT_COUNTER::RECVD);

    DBGLOG("Reader Event: %s, file: %s, status: %d, seq: %d",
        GetLibsmbEventName(cbData->event).c_str(), cbData->fileHandle.m_file->m_fileName.c_str(),
        status, cbData->fileHandle.m_block.m_seq);

    switch (cbData->event) {
        case LibsmbEvent::OPEN_SRC:
            SmbOpenCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::READ:
            SmbReadCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::CLOSE_SRC:
            SmbCloseCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::ADS:
            SmbAdsCb(smb2, status, data, cbData);
            break;
        default:
            break;
    }
    return;
}

int SendOpenRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.srcRootPath);

    int ret = cbData->readSmbContext->SmbOpenAsync(
        smbPath.c_str(), O_RDONLY, ReaderCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Open Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbOpenCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData)
{
    if (status < SUCCESS) {
        HandleSmbOpenStatusFailed(smb2, status, cbData);
        return;
    }

    // 给writer发一个用于open的filehandle
    std::shared_ptr<FileDesc> file = cbData->fileHandle.m_file;
    FileDescState oldState = file->GetSrcState();
    file->srcIOHandle.smbFh = static_cast<struct smb2fh*>(data);
    file->SetSrcState(FileDescState::SRC_OPENED);
    
    if (oldState == FileDescState::FILEHANDLE_INVALID) {
        // 重新打开文件的情况，不需要创建新的Block
        delete cbData;
        cbData = nullptr;
        return;
    }
    
    cbData->aggregateQueue->Push(cbData->fileHandle);
    ++cbData->controlInfo->m_readProduce;

    SmbOpenCbSendBlock(cbData);

    delete cbData;
    cbData = nullptr;
}

void SmbOpenCbSendBlock(SmbReaderCommonData *cbData)
{
    std::shared_ptr<FileDesc> file = cbData->fileHandle.m_file;
    uint32_t blockSize = cbData->params.blockSize;
    uint64_t fileSize = file->m_size;
    uint64_t fullBlockNum = fileSize / blockSize;
    uint32_t remainSize = fileSize % blockSize;

    file->m_blockStats.m_totalCnt = (remainSize == 0) ? fullBlockNum : fullBlockNum + 1;
    DBGLOG("SmbOpenCb file:%s, total blocks:%d, file size:%llu, blockSize:%d, fullBlockNum:%d, remainSize:%d",
        file->m_fileName.c_str(), file->m_blockStats.m_totalCnt.load(), file->m_size, blockSize,
        fullBlockNum, remainSize);

    for (uint64_t i = 0; i < fullBlockNum + 1; i++) {
        if (i == fullBlockNum && remainSize == 0) {
            break;
        }
        cbData->fileHandle.m_block.m_size = (i == fullBlockNum) ? remainSize : blockSize; // 最后一个块大小是remainSize
        cbData->fileHandle.m_block.m_offset = blockSize * i;
        cbData->fileHandle.m_block.m_seq = i + 1;
        if (fileSize <= MAX_SMALL_FILE_SIZE) {
            // 小文件直接发
            auto newCbData = new(nothrow) SmbReaderCommonData(*cbData);
            if (newCbData == nullptr) {
                ERRLOG("Failed to allocate Memory for cbData");
                return;
            }
            newCbData->fileHandle = cbData->fileHandle;
            newCbData->fileHandle.m_block.m_buffer = new uint8_t[newCbData->fileHandle.m_block.m_size];
            newCbData->blockBufferMap->Add(newCbData->fileHandle.m_file->m_fileName, newCbData->fileHandle);
            if (SendReaderRequest(newCbData->fileHandle, newCbData, LibsmbEvent::READ) != SUCCESS) {
                newCbData->blockBufferMap->Delete(newCbData->fileHandle.m_file->m_fileName, newCbData->fileHandle);
                return;
            }
        } else {
            cbData->partialReadQueue->Push(cbData->fileHandle);
        }
    }
}

void HandleSmbOpenStatusFailed(struct smb2_context *smb2, int status, SmbReaderCommonData *cbData)
{
    if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        cbData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    FileDescState state = cbData->fileHandle.m_file->GetSrcState();
    if (state == FileDescState::FILEHANDLE_INVALID) {
        cbData->fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    }
    // skipping to the failed cnt inc for zip files in restore which are not used in writetask
    if (!cbData->fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
        cbData->controlInfo->m_noOfFilesReadFailed++;
    }
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    ERRLOG("Open failed file:%s, status:%s, errno:%d", cbData->fileHandle.m_file->m_fileName.c_str(),
        smb2_get_error(smb2), status);
    delete cbData;
    cbData = nullptr;
}

int SendReadRequest(FileHandle fileHandle, SmbReaderCommonData *cbData)
{
    if (fileHandle.m_block.m_buffer == nullptr) {
        ERRLOG("block buffer is nullptr!");
        return FAILED;
    }
    DBGLOG("Send Read Request : %s, size: %d, offset: %llu",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size, fileHandle.m_block.m_offset);
    std::shared_ptr<Module::SmbContextWrapper> context = cbData->readSmbContext;
    int ret = context->SmbReadAsync(fileHandle.m_file->srcIOHandle.smbFh,
                                    (uint8_t *)(fileHandle.m_block.m_buffer),
                                    fileHandle.m_block.m_size,
                                    fileHandle.m_block.m_offset,
                                    ReaderCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Read Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbReadCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData)
{
    if (status < 0 && IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        HandleSmbReadStatusRetry(status, cbData);
        return;
    } else if ((uint64_t)status < cbData->fileHandle.m_block.m_size &&
        cbData->fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT) {
        cbData->pktStats->Increment(PKT_TYPE::READ, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
        SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    } else if (status < 0 || IsFileReadOrWriteFailed(cbData->fileHandle)) {
        HandleSmbReadStatusFailed(smb2, data, status, cbData);
        return;
    }

    HandleSmbReadStatusSuccess(cbData);
    return;
}

void HandleSmbReadStatusSuccess(SmbReaderCommonData *cbData)
{
    FileDescState state = cbData->fileHandle.m_file->GetSrcState();
    // state为FILEHANDLE_INVALID时，仍有可能有读成功的报文返回，直接置会被覆盖
    if (state != FileDescState::FILEHANDLE_INVALID) {
        cbData->fileHandle.m_file->SetSrcState(FileDescState::PARTIAL_READED);
    }
    ++cbData->fileHandle.m_file->m_blockStats.m_readRespCnt;
    if (cbData->fileHandle.m_file->m_blockStats.m_totalCnt == cbData->fileHandle.m_file->m_blockStats.m_readRespCnt) {
        cbData->fileHandle.m_file->SetSrcState(FileDescState::READED);
        auto newCbData = new(nothrow) SmbReaderCommonData(*cbData);
        if (newCbData == nullptr) {
            ERRLOG("Failed to allocate Memory for new cbData");
            delete cbData;
            cbData = nullptr;
            return;
        }
        newCbData->fileHandle = cbData->fileHandle;
        SendReaderRequest(newCbData->fileHandle, newCbData, LibsmbEvent::CLOSE_SRC);
    }
    cbData->aggregateQueue->Push(cbData->fileHandle);
    ++cbData->controlInfo->m_readProduce;
    delete cbData;
    cbData = nullptr;
    return;
}

void HandleSmbReadStatusFailed(struct smb2_context *smb2, void *data, int status, SmbReaderCommonData *cbData)
{
    data = data;
    ++(cbData->fileHandle.m_file->m_blockStats.m_readRespCnt);
    if (!IsFileReadOrWriteFailed(cbData->fileHandle)) {
        if (!cbData->fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
            ++cbData->controlInfo->m_noOfFilesFailed;
            ERRLOG("read file failed: %s, totalFailed: %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
                cbData->controlInfo->m_noOfFilesFailed.load());
        }
    }
    if (cbData->fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
        ++cbData->controlInfo->m_noOfFilesReadFailed;
    }
    cbData->fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);

    cbData->aggregateQueue->Push(cbData->fileHandle);
    ++cbData->controlInfo->m_readProduce;

    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    ERRLOG("Read failed file:%s, status:%s, errno:%d, retry:%d, seq: %d", cbData->fileHandle.m_file->m_fileName.c_str(),
        smb2_get_error(smb2), status, cbData->fileHandle.m_retryCnt, cbData->fileHandle.m_retryCnt);
    if (cbData->fileHandle.m_file->m_blockStats.m_totalCnt == cbData->fileHandle.m_file->m_blockStats.m_readRespCnt) {
        auto newCbData = new(nothrow) SmbReaderCommonData(*cbData);
        if (newCbData == nullptr) {
            ERRLOG("Failed to allocate Memory for cbData");
            delete cbData;
            cbData = nullptr;
            return;
        }
        newCbData->fileHandle = cbData->fileHandle;
        SendReaderRequest(newCbData->fileHandle, newCbData, LibsmbEvent::CLOSE_SRC);
    }
    delete cbData;
    cbData = nullptr;
}

void HandleSmbReadStatusRetry(int status, SmbReaderCommonData *cbData)
{
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
    cbData->pktStats->Increment(PKT_TYPE::READ, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);

    if ((status == -ENETRESET || status == -EBADF) && cbData->fileHandle.m_file->srcIOHandle.smbFh != nullptr) {
        ERRLOG("Read failed file:%s, errno:-ENETRESET %d, set srchandle to nullptr, need to reopen",
            cbData->fileHandle.m_file->m_fileName.c_str(), status);
        FileHandle fileHandle;
        fileHandle.m_file = cbData->fileHandle.m_file;
        fileHandle.m_file->srcIOHandle.smbFh = nullptr;
        fileHandle.m_file->SetSrcState(FileDescState::FILEHANDLE_INVALID);
        fileHandle.m_block.m_seq = 0; // seq为0， 重新open
        cbData->timer->Insert(fileHandle, ONE_THOUSAND_UNIT_CONVERSION);
    }

    delete cbData;
    cbData = nullptr;
}

int SendCloseRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData)
{
    int ret = cbData->readSmbContext->SmbCloseAsync(
        fileHandle.m_file->srcIOHandle.smbFh, ReaderCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Close Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbCloseCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData)
{
    data = data;
    if (IsFileReadOrWriteFailed(cbData->fileHandle)) {
        delete cbData;
        cbData = nullptr;
        return;
    }
    ++cbData->controlInfo->m_noOfFilesRead;

    if (status < SUCCESS) {
        ERRLOG("Close failed file:%s, status:%s, errno:%d", cbData->fileHandle.m_file->m_fileName.c_str(),
            smb2_get_error(smb2), status);
        delete cbData;
        cbData = nullptr;
        return;
    }

    cbData->fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);

    delete cbData;
    cbData = nullptr;
}

int SendAdsRequest(FileHandle &fileHandle, SmbReaderCommonData *cbData)
{
    struct smb2_filestream_info *adsInfo;
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.srcRootPath);
    int ret = cbData->readSmbContext->SmbGetStreamInfoAsync(
        smbPath.c_str(), &adsInfo, ReaderCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Ads Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbAdsCb(struct smb2_context *smb2, int status, void *data, SmbReaderCommonData *cbData)
{
    if (status < SUCCESS) {
        HandleSmbAdsStatusFailed(smb2, status, cbData);
        return;
    }

    auto adsInfo = static_cast<struct smb2_filestream_info *>(data);
    struct smb2_filestream_node *iter = adsInfo->fss;
    uint32_t adsFileCount = 0;
    while (iter != nullptr) {
        FileHandle fileHandle;
        fileHandle.m_file = make_shared<FileDesc>(BackupIOEngine::LIBSMB, BackupIOEngine::LIBSMB);
        DBGLOG("AdsCb file: %s has a adsfile: %s, size: %d", cbData->fileHandle.m_file->m_fileName.c_str(),
            (const char *)iter->stream_name, iter->stream_size);
        int streamNameLength = strlen((const char*)iter->stream_name);
        if (streamNameLength <= ADS_FILE_SUFFIX_LENGTH) {
            iter = iter->next;
            continue;
        }
        std::string streamName((const char*)iter->stream_name, streamNameLength - ADS_FILE_SUFFIX_LENGTH);
        fileHandle.m_file->m_fileName = cbData->fileHandle.m_file->m_fileName + streamName;
        fileHandle.m_file->m_dirName = cbData->fileHandle.m_file->m_dirName;
        fileHandle.m_file->m_size = iter->stream_size;
        fileHandle.m_file->ClearFlag(IS_DIR);
        fileHandle.m_file->m_mode = FILE_IS_ADS_FILE; // 表示这个文件是ADS文件
        fileHandle.m_file->SetSrcState(FileDescState::META_READED);
        cbData->readQueue->Push(fileHandle);
        iter = iter->next;
        ++adsFileCount;
    }
    DBGLOG("AdsCb: %s, count: %d, state: %d", cbData->fileHandle.m_file->m_fileName.c_str(), adsFileCount,
        cbData->fileHandle.m_file->GetSrcState());
    *cbData->adsFileCnt += adsFileCount;

    SmbAdsCbDispachFilehandle(cbData, adsFileCount);

    smb2_free_data(smb2, adsInfo);
    delete cbData;
    cbData = nullptr;
}

void HandleSmbAdsStatusFailed(struct smb2_context *smb2, int status, SmbReaderCommonData *cbData)
{
    if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        cbData->pktStats->Increment(PKT_TYPE::LSTAT, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (status == -ENOENT || status == -EINVAL) {
        DBGLOG("SmbAdsCb file: %s, status: %d", cbData->fileHandle.m_file->m_fileName.c_str(), status);
    } else {
        ERRLOG("SmbAdsCb failed: %s, status: %d, error: %s", cbData->fileHandle.m_file->m_fileName.c_str(),
            status, smb2_get_error(smb2));
    }
    SmbAdsCbDispachFilehandle(cbData, 0);
    delete cbData;
    cbData = nullptr;
    return;
}

void SmbAdsCbDispachFilehandle(SmbReaderCommonData *cbData, uint32_t adsFileCount)
{
    if (cbData->fileHandle.IsDir()) {
        return; // dir fileHandle pushed to aggregateQueue at the beginning of ProcessFileDescState
    }
    if (adsFileCount > 0) {
        cbData->fileHandle.m_file->m_mode = FILE_HAVE_ADS; // 表示这个文件有ADS，后续不会聚合
    }
    // not an ads file, push to aggr queue
    if (adsFileCount == 0 && cbData->fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED) {
        cbData->aggregateQueue->Push(cbData->fileHandle);
        ++cbData->controlInfo->m_readProduce;
        return;
    }
    cbData->fileHandle.m_file->SetSrcState(FileDescState::META_READED);
    cbData->readQueue->Push(cbData->fileHandle);
}
