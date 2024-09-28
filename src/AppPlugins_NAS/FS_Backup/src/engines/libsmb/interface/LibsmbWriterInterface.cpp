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
#include "LibsmbWriterInterface.h"
#include <sys/types.h>
#include <fcntl.h>
#include "log/Log.h"
#include "BackupConstants.h"
#include "ParserStructs.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr int RETRY_WAIT_IN_SEC = 1;
    constexpr uint8_t MAX_SMB_RETRY_COUNT = 3;
    constexpr uint16_t DIRPATH_LENGTH = 4096;
    constexpr uint32_t DIRECTORY_SIZE = 4096;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr auto NAS_SCANNERBACKUPCTRL_ENTRY_MODE_META_MODIFIED = "mm";
    const int ONE_THOUSAND_UNIT_CONVERSION = 1000;
    constexpr auto DIR_MTIME_MAX_RETRY_TIMES = 5;
    constexpr int READONLY_ATTR_POS = 0;
    constexpr int HIDDEN_ATTR_POS = 1;
    constexpr uint32_t FILEMODE_BITLENGTH = 32;
}

int SendWriterRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData, LibsmbEvent event)
{
    DBGLOG("Writer Send %s Request: %s, seq: %d", GetLibsmbEventName(event).c_str(),
        fileHandle.m_file->m_fileName.c_str(), cbData->fileHandle.m_block.m_seq);

    cbData->event = event;
    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(event), PKT_COUNTER::SENT);
    int ret = 0;
    switch (event) {
        case LibsmbEvent::OPEN_DST:
            ret = SendOpenDstRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::WRITE:
            ret = SendWriteRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::CLOSE_DST:
            ret = SendCloseDstRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::SET_SD:
            ret = SendSetSdRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::STAT_DST:
            ret = SendStatRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::SET_BASIC_INFO:
        case LibsmbEvent::SET_BASIC_INFO_DIR:
            ret = SendSetBasicInfoRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::LINK:
            ret = SendHardlinkRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::UNLINK:
            ret = SendUnlinkRequest(fileHandle, cbData);
            break;
        case LibsmbEvent::RESET_ATTR:
            ret = SendResetAttrRequest(fileHandle, cbData);
            break;

        default:
            break;
    }
    if (ret != SUCCESS) {
        cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(cbData->event), PKT_COUNTER::RECVD);
        delete cbData;
        cbData = nullptr;
        return FAILED;
    }
    return SUCCESS;
}

void WriterCallBack(struct smb2_context *smb2, int status, void *data, void *privateData)
{
    SmbWriterCommonData *cbData = static_cast<SmbWriterCommonData *>(privateData);
    if (cbData == nullptr || cbData->fileHandle.m_file == nullptr || cbData->pktStats == nullptr) {
        if (cbData != nullptr) {
            delete cbData;
            cbData = nullptr;
            ERRLOG("WriterCallBack failed: fileHandle.m_file or cbData->pktStats is nullptr, status:%d", status);
        } else {
            ERRLOG("WriterCallBack failed: cbData is nullptr, status:%d", status);
        }
        return;
    }

    if (status < SUCCESS) {
        mode_t mode = cbData->fileHandle.m_file->m_mode;
        if (mode == FILE_IS_ADS_FILE || mode == FILE_HAVE_ADS) {
            DBGLOG("Block ADS Open flag set as false:%s", cbData->fileHandle.m_file->m_fileName.c_str());
            *(cbData->isBlockAdsOpen) = false;
        }
        if (!IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            // 统计失败计数，ServerCheck用
            CheckStatusAndIncStat(status, LibsmbEventToPKT_TYPE(cbData->event), cbData->pktStats);
            FSBackupUtils::RecordFailureDetail(cbData->failureRecorder, cbData->fileHandle.m_file->m_fileName, -status);
        }
    }

    cbData->pktStats->Increment(LibsmbEventToPKT_TYPE(cbData->event), PKT_COUNTER::RECVD);

    DBGLOG("Writer Event: %s, file: %s, status: %d, seq: %d",
        GetLibsmbEventName(cbData->event).c_str(), cbData->fileHandle.m_file->m_fileName.c_str(),
        status, cbData->fileHandle.m_block.m_seq);

    WriterCallBackHandleEvent(smb2, status, data, cbData);
    return;
}

void WriterCallBackHandleEvent(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    switch (cbData->event) {
        case LibsmbEvent::OPEN_DST:
            SmbOpenDstCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::WRITE:
            SmbWriteCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::CLOSE_DST:
            SmbCloseDstCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::SET_SD:
            SmbSetSdCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::STAT_DST:
            SmbStatCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::SET_BASIC_INFO:
            SmbSetBasicInfoCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::SET_BASIC_INFO_DIR:
            SmbSetBasicInfoCbForDir(smb2, status, data, cbData);
            break;
        case LibsmbEvent::LINK:
            SmbHardLinkCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::UNLINK:
            SmbUnlinkCb(smb2, status, data, cbData);
            break;
        case LibsmbEvent::RESET_ATTR:
            SmbResetAttrCb(smb2, status, data, cbData);
            break;
        default:
            break;
    }
}

int SendOpenDstRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    uint32_t flag = O_WRONLY | O_CREAT;
    mode_t mode = cbData->fileHandle.m_file->m_mode;
    if (mode == FILE_IS_ADS_FILE || mode == FILE_HAVE_ADS) {
        // open带有ADS的源文件且flag置上o_trunc时，这个文件的ADS也会被清掉
        DBGLOG("Send OpenDst Request ADS relative, don't set o_trunc: %s, openFlag: %d", smbPath.c_str(), flag);
    } else if ((cbData->params.backupType == BackupType::RESTORE ||
        cbData->params.backupType == BackupType::FILE_LEVEL_RESTORE) &&
        !cbData->fileHandle.m_file->IsFlagSet(TRUNCATE)) {
        flag |= O_EXCL;
    } else {
        flag |= O_TRUNC;
    }
    DBGLOG("Send OpenDst Request: %s, openFlag: %d", smbPath.c_str(), flag);
    int ret = cbData->writeSmbContext->SmbOpenAsync(smbPath.c_str(), flag, WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Open Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void HandleSmbOpenDstCbError(struct smb2_context *smb2, int status, SmbWriterCommonData *cbData)
{
    // need to remove file from hardlink map
    if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        cbData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (status == -ENOENT || status == -ENOTDIR) { // 父目录不存在
        cbData->dirQueue->Push(cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (status == -EEXIST) {
        DBGLOG("SmbOpenDstCb file exists: %s, status: %d, errno: %s",
            cbData->fileHandle.m_file->m_fileName.c_str(), status, smb2_get_error(smb2));
        ProcessRestorePolicy(cbData);
        return;
    }
    if (status == -EACCES) {
        WARNLOG("SmbOpenDstCb file access deny. file : %s, errno: %s",
            cbData->fileHandle.m_file->m_fileName.c_str(), smb2_get_error(smb2));
        if (CheckAttrNotReseted(cbData)) {
            // for readonly access deny , is normal process , don't count access_err
            cbData->pktStats->DecrementError(PKT_ERROR::NO_ACCESS_ERR);
            ResetFileAttr(cbData);
            return;
        }
    }
    if (status == -EISDIR) {
        DBGLOG("SmbOpenDstCb dir need to be replaced to file, dir : %s", cbData->fileHandle.m_file->m_fileName.c_str());
        cbData->fileHandle.m_file->SetDstState(FileDescState::REPLACE_DIR);
        cbData->dirQueue->Push(cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    ++cbData->controlInfo->m_noOfFilesFailed;
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    ERRLOG("SmbOpenDstCb failed: %s, status: %d, errno: %s, totalFailed: %llu",
        cbData->fileHandle.m_file->m_fileName.c_str(), status, smb2_get_error(smb2),
        cbData->controlInfo->m_noOfFilesFailed.load());
    delete cbData;
    cbData = nullptr;
    return;
}

bool CheckAttrNotReseted(SmbWriterCommonData *cbData)
{
    DBGLOG("CheckAttrNotReseted: %s, %d", cbData->fileHandle.m_file->m_fileName.c_str(),
        cbData->fileHandle.m_file->IsFlagSet(ATTR_RESETED));
    if (!cbData->fileHandle.m_file->IsFlagSet(ATTR_RESETED)) {
        DBGLOG("attr not reseted for file: %s", cbData->fileHandle.m_file->m_fileName.c_str());
        return true;
    }
    return false;
}

void SmbOpenDstCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;

    if (status < SUCCESS) {
        HandleSmbOpenDstCbError(smb2, status, cbData);
        return;
    }

    cbData->fileHandle.m_file->dstIOHandle.smbFh = static_cast<struct smb2fh*>(data);
    // 大小为0直接close
    if (cbData->fileHandle.m_file->m_size == 0) {
        cbData->fileHandle.m_file->SetDstState(FileDescState::WRITED);
        cbData->writeQueue->Push(cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }

    cbData->fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
    delete cbData;
    cbData = nullptr;
}

int ProcessRestorePolicy(SmbWriterCommonData *cbData)
{
    DBGLOG("restore policy file %s", cbData->fileHandle.m_file->m_fileName.c_str());
    if (cbData->params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        ++cbData->controlInfo->m_noOfFilesCopied;
        cbData->controlInfo->m_noOfBytesCopied += cbData->fileHandle.m_file->m_size;
        cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName);
        DBGLOG("ignore exists file %s success!", cbData->fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        cbData = nullptr;
        return SUCCESS;
    }
    if (cbData->params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE_OLDER) {
        DBGLOG("restore policy is OVERWRITE_OLDER, need to get file mtime, file %s",
            cbData->fileHandle.m_file->m_fileName.c_str());
        SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::STAT_DST);
        return SUCCESS;
    }
    if (cbData->params.restoreReplacePolicy == RestoreReplacePolicy::OVERWRITE) {
        DBGLOG("restore policy is OVERWRITE, overwrite file mtime, file %s",
            cbData->fileHandle.m_file->m_fileName.c_str());
        cbData->fileHandle.m_file->SetFlag(TRUNCATE);
        SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::OPEN_DST);
        return SUCCESS;
    }
    cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    ++cbData->controlInfo->m_noOfFilesFailed;
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    ERRLOG("invalid restore policy %d, totalFailed: %llu", static_cast<int>(cbData->params.restoreReplacePolicy),
        cbData->controlInfo->m_noOfFilesFailed.load());
    delete cbData;
    cbData = nullptr;
    return FAILED;
}

void ResetFileAttr(SmbWriterCommonData *cbData)
{
    if (cbData->params.restoreReplacePolicy == RestoreReplacePolicy::IGNORE_EXIST) {
        cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        ++cbData->controlInfo->m_noOfFilesCopied;
        cbData->controlInfo->m_noOfBytesCopied += cbData->fileHandle.m_file->m_size;
        cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName);
        WARNLOG("ignore access deny file %s success!", cbData->fileHandle.m_file->m_fileName.c_str());
        delete cbData;
        cbData = nullptr;
        return;
    }
    // 把hidden 和 readonly 干掉
    DBGLOG("remove hidden and readonly for file : %s", cbData->fileHandle.m_file->m_fileName.c_str());
    SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::RESET_ATTR);
}

int SendWriteRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    if (fileHandle.m_block.m_buffer == nullptr) {
        ERRLOG("block buffer is nullptr!");
        return FAILED;
    }
    int ret = cbData->writeSmbContext->SmbWriteAsync(fileHandle.m_file->dstIOHandle.smbFh,
        (const uint8_t *)(fileHandle.m_block.m_buffer), fileHandle.m_block.m_size,
        fileHandle.m_block.m_offset, WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send SmbWriteCb Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbWriteCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;

    if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        HandleSmbWriteStatusRetry(status, cbData);
        return;
    }
    if (status < SUCCESS || IsFileReadOrWriteFailed(cbData->fileHandle)) {
        HandleSmbWriteStatusFailed(smb2, data, status, cbData);
        return;
    }
    HandleSmbWriteStatusSuccess(cbData);
    return;
}

void HandleSmbWriteStatusSuccess(SmbWriterCommonData *cbData)
{
    FileDescState state = cbData->fileHandle.m_file->GetDstState();
    // state为FILEHANDLE_INVALID时，仍有可能有写成功的报文返回，直接置会被覆盖
    if (state != FileDescState::FILEHANDLE_INVALID) {
        cbData->fileHandle.m_file->SetDstState(FileDescState::PARTIAL_WRITED);
    }
    ++(cbData->fileHandle.m_file->m_blockStats.m_writeRespCnt);
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    if (cbData->fileHandle.m_file->m_blockStats.m_writeRespCnt == cbData->fileHandle.m_file->m_blockStats.m_totalCnt) {
        DBGLOG("All blocks writed for %s", cbData->fileHandle.m_file->m_fileName.c_str());
        cbData->fileHandle.m_file->SetDstState(FileDescState::WRITED);
        auto newCbData = new(nothrow) SmbWriterCommonData(*cbData);
        if (newCbData == nullptr) {
            ERRLOG("Failed to allocate Memory for newCbData");
            delete cbData;
            cbData = nullptr;
            return;
        }
        newCbData->fileHandle = cbData->fileHandle;
        SendWriterRequest(newCbData->fileHandle, newCbData, LibsmbEvent::CLOSE_DST);
    }

    delete cbData;
    cbData = nullptr;
    return;
}

void HandleSmbWriteStatusFailed(struct smb2_context *smb2, void *data, int status, SmbWriterCommonData *cbData)
{
    data = data;
    ++(cbData->fileHandle.m_file->m_blockStats.m_writeRespCnt);
    ERRLOG("Write failed file:%s, status:%s, errno:%d", cbData->fileHandle.m_file->m_fileName.c_str(),
        smb2_get_error(smb2), status);
    if (!IsFileReadOrWriteFailed(cbData->fileHandle)) {
        ++cbData->controlInfo->m_noOfFilesFailed;
        ERRLOG("write file failed: %s, totalFailed: %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
            cbData->controlInfo->m_noOfFilesFailed.load());
    }
    cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName, cbData->fileHandle);
    cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    if (cbData->fileHandle.m_file->m_blockStats.m_writeRespCnt == cbData->fileHandle.m_file->m_blockStats.m_totalCnt ||
        cbData->fileHandle.m_file->m_size == 0) {
        auto newCbData = new(nothrow) SmbWriterCommonData(*cbData);
        if (newCbData == nullptr) {
            ERRLOG("Failed to allocate Memory for cbData");
            delete cbData;
            cbData = nullptr;
            return;
        }
        newCbData->fileHandle = cbData->fileHandle;
        SendWriterRequest(newCbData->fileHandle, newCbData, LibsmbEvent::CLOSE_DST);
    }
    delete cbData;
    cbData = nullptr;
}

void HandleSmbWriteStatusRetry(int status, SmbWriterCommonData *cbData)
{
    SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
    cbData->pktStats->Increment(PKT_TYPE::WRITE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);

    if ((status == -ENETRESET || status == -EBADF) && cbData->fileHandle.m_file->dstIOHandle.smbFh != nullptr) {
        ERRLOG("Write failed file:%s, errno:-ENETRESET %d, set dsthandle to nullptr, need to reopen",
            cbData->fileHandle.m_file->m_fileName.c_str(), status);
        FileHandle fileHandle;
        fileHandle.m_file = cbData->fileHandle.m_file;
        fileHandle.m_file->dstIOHandle.smbFh = nullptr;
        fileHandle.m_file->SetDstState(FileDescState::FILEHANDLE_INVALID);
        fileHandle.m_block.m_seq = 0; // seq为0， 重新open
        cbData->timer->Insert(fileHandle, ONE_THOUSAND_UNIT_CONVERSION);
    }

    delete cbData;
    cbData = nullptr;
}

int SendCloseDstRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    DBGLOG("SendCloseDstRequest: %s", fileHandle.m_file->m_fileName.c_str());
    int ret = cbData->writeSmbContext->SmbCloseAsync(
        fileHandle.m_file->dstIOHandle.smbFh, WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("SendCloseDstRequest Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbCloseDstCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;
    mode_t mode = cbData->fileHandle.m_file->m_mode;
    if (mode == FILE_IS_ADS_FILE ||
        (cbData->params.backupDataFormat == BackupDataFormat::AGGREGATE && mode == FILE_HAVE_ADS)) {
        DBGLOG("Block ADS Open flag set as false:%s", cbData->fileHandle.m_file->m_fileName.c_str());
        *(cbData->isBlockAdsOpen) = false;
    }

    if (IsFileReadOrWriteFailed(cbData->fileHandle)) {
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (status < SUCCESS) {
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::CLOSE, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        ++cbData->controlInfo->m_noOfFilesFailed;
        ERRLOG("SmbCloseDstCb failed file:%s, status:%s, errno:%d, totalFailed: %llu",
            cbData->fileHandle.m_file->m_fileName.c_str(), smb2_get_error(smb2), status,
            cbData->controlInfo->m_noOfFilesFailed.load());
        delete cbData;
        cbData = nullptr;
        return;
    }

    cbData->fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    if (cbData->fileHandle.m_file->m_mode != FILE_IS_ADS_FILE) { // ADS文件没有属性
        cbData->writeQueue->Push(cbData->fileHandle);
    }

    delete cbData;
    cbData = nullptr;
}

int SendSetSdRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    wstring wsInput {};
    try {
        wstring_convert<codecvt_utf8<wchar_t>> converter;
        wsInput = converter.from_bytes(fileHandle.m_file->m_aclText).c_str();
    } catch (std::exception &e) {
        ERRLOG("error caught: %s", e.what());
        return FAILED;
    }
    DBGLOG("acl text file %s: acl %s", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_aclText.c_str());
    int ret = cbData->writeSmbContext->SmbSetSdAsync(smbPath.c_str(),
        const_cast<wchar_t *>(wsInput.c_str()), WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Open Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbSetSdCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;

    if (status < SUCCESS) {
        ERRLOG("SetSd failed file:%s, status:%s, errno:%d", cbData->fileHandle.m_file->m_fileName.c_str(),
            smb2_get_error(smb2), status);
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        if (cbData->fileHandle.m_file->IsFlagSet(IS_DIR)) {
            ++cbData->controlInfo->m_noOfDirFailed;
        } else {
            ++cbData->controlInfo->m_noOfFilesFailed;
        }
        ERRLOG("write file failed: %s, totalFailed: %llu, %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
            cbData->controlInfo->m_noOfDirFailed.load(), cbData->controlInfo->m_noOfFilesFailed.load());
        delete cbData;
        cbData = nullptr;
        return;
    }

    // 如果是目录，时间在dir phase设置
    if (cbData->fileHandle.m_file->IsFlagSet(IS_DIR)) {
        if (cbData->fileHandle.m_file->m_scannermode == CTRL_ENTRY_MODE_META_MODIFIED) {
            ++cbData->controlInfo->m_noOfDirCopied;
        }
        delete cbData;
        cbData = nullptr;
        return;
    }

    SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::SET_BASIC_INFO);
}

int SendSetBasicInfoRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    DBGLOG("SetBasicInfo file %s: attr %d btime %d atime %d mtime %d ctime %d", fileHandle.m_file->m_fileName.c_str(),
        cbData->fileHandle.m_file->m_fileAttr, cbData->fileHandle.m_file->m_btime, cbData->fileHandle.m_file->m_atime,
        cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime);
    struct SMB2_BASIC_INFO basic_info = {cbData->fileHandle.m_file->m_fileAttr, cbData->fileHandle.m_file->m_btime,
        cbData->fileHandle.m_file->m_atime, cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime,
        0, 0, 0, 0};
    int retVal = cbData->writeSmbContext->SmbSetBasicInfoAsync(
        smbPath.c_str(), &basic_info, WriterCallBack, cbData);
    if (retVal != SUCCESS) {
        ERRLOG("Send SetBasicInfo Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbSetBasicInfoCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;

    if (status < SUCCESS) {
        ERRLOG("SetBasicInfo failed file:%s, status:%s, errno:%d, retryCnt:%d",
            cbData->fileHandle.m_file->m_fileName.c_str(), smb2_get_error(smb2), status, cbData->fileHandle.m_retryCnt);
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        ++cbData->controlInfo->m_noOfFilesFailed;
        ERRLOG("write file failed: %s, totalFailed: %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
            cbData->controlInfo->m_noOfFilesFailed.load());
        delete cbData;
        cbData = nullptr;
        return;
    }
    mode_t mode = cbData->fileHandle.m_file->m_mode;
    if (mode == FILE_HAVE_ADS) {
        DBGLOG("Block ADS Open flag set as false:%s", cbData->fileHandle.m_file->m_fileName.c_str());
        *(cbData->isBlockAdsOpen) = false;
    }
    ++cbData->controlInfo->m_noOfFilesCopied;
    if (!cbData->fileHandle.IsOnlyMetaModified()) {
        cbData->controlInfo->m_noOfBytesCopied += cbData->fileHandle.m_file->m_size;
    }
    delete cbData;
    cbData = nullptr;
}

void SmbSetBasicInfoCbForDir(struct smb2_context* /* smb2 */, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;
    if (status == SUCCESS) {
        DBGLOG("mtime set success: %s, atime:%d, mtime: %d, ctime: %d, btime: %d, file_attr:%d",
            cbData->fileHandle.m_file->m_dirName.c_str(), cbData->fileHandle.m_file->m_atime,
            cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime,
            cbData->fileHandle.m_file->m_btime, cbData->fileHandle.m_file->m_fileAttr);
        ++cbData->controlInfo->m_noOfDirCopied;
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (status == -ENOENT) {
        WARNLOG("mtime dir not exist: %s", cbData->fileHandle.m_file->m_dirName.c_str());
        ++cbData->controlInfo->m_noOfDirFailed;
    } else if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        cbData->fileHandle.m_retryCnt++;
        ERRLOG("mtime retry: %s, retryCnt = %d",
            cbData->fileHandle.m_file->m_dirName.c_str(), cbData->fileHandle.m_retryCnt);
        cbData->writeQueue->Push(cbData->fileHandle);
    } else {
        ERRLOG("mtime set failed: %s, atime:%d, mtime: %d, ctime: %d, btime: %d, file_attr:%d",
            cbData->fileHandle.m_file->m_dirName.c_str(), cbData->fileHandle.m_file->m_atime,
            cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime,
            cbData->fileHandle.m_file->m_btime, cbData->fileHandle.m_file->m_fileAttr);
        ++cbData->controlInfo->m_noOfDirFailed;
    }
    delete cbData;
    cbData = nullptr;
    return;
}

int SendStatRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    auto stat = new(nothrow) smb2_stat_64();
    if (stat == nullptr) {
        ERRLOG("Failed to allocate Memory for stat");
        return FAILED;
    }
    int ret = cbData->writeSmbContext->SmbStatAsync(smbPath.c_str(), stat, WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send Stat Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbStatCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    auto *st = static_cast<struct smb2_stat_64 *>(data);
    if (status < SUCCESS) {
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        ERRLOG("lstat failed: %s, status: %d, error: %s", cbData->fileHandle.m_file->m_fileName.c_str(),
            status, smb2_get_error(smb2));
        delete cbData;
        cbData = nullptr;
        return;
    }
    if (st->smb2_mtime < cbData->fileHandle.m_file->m_mtime) {
        DBGLOG("restore policy is OVERWRITE_OLDER, overwrite, file %s dstTime(%d) < srcTime(%d).",
            cbData->fileHandle.m_file->m_fileName.c_str(), st->smb2_mtime, cbData->fileHandle.m_file->m_mtime);
        cbData->fileHandle.m_file->SetFlag(TRUNCATE);
        SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::OPEN_DST);
    } else {
        DBGLOG("restore policy is OVERWRITE_OLDER, skip, file %s dstTime(%d) > srcTime(%d).",
            cbData->fileHandle.m_file->m_fileName.c_str(), st->smb2_mtime, cbData->fileHandle.m_file->m_mtime);
        ++cbData->controlInfo->m_noOfFilesCopied;
        cbData->controlInfo->m_noOfBytesCopied += cbData->fileHandle.m_file->m_size;
        cbData->blockBufferMap->Delete(cbData->fileHandle.m_file->m_fileName);
        cbData->fileHandle.m_file->SetDstState(FileDescState::WRITE_SKIP);
        delete cbData;
        cbData = nullptr;
    }
}

int SendHardlinkRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    string smbTargetPath = RemoveFirstSeparator(cbData->linkTargetPath);
    ConcatRootPath(smbTargetPath, cbData->params.dstRootPath);
    DBGLOG("Send Hardlink source: %s, target: %s", smbPath.c_str(), smbTargetPath.c_str());
    if (cbData->writeSmbContext->SmbHardLinkAsync(smbTargetPath.c_str(),
        smbPath.c_str(), WriterCallBack, cbData) != SUCCESS) {
        ERRLOG("Failed to send hardlink req: %s err: %s", cbData->fileHandle.m_file->m_fileName,
               cbData->writeSmbContext->SmbGetError());
        return FAILED;
    }
    return SUCCESS;
}

void SmbHardLinkCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    data = data;
    if (status < SUCCESS) {
        DBGLOG("HardLinkCb req failed for: %s, status: %d, error: %s",
            cbData->fileHandle.m_file->m_fileName.c_str(), status, smb2_get_error(smb2));
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::OPEN, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        if (status == -ENOTDIR || status == -ENOENT || status == -ESTALE) {
            DBGLOG("parent path not exist:  %s", cbData->fileHandle.m_file->m_fileName);
            cbData->dirQueue->Push(cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        if (status == -EEXIST) {
            if (cbData->params.backupType != BackupType::BACKUP_FULL) {
                INFOLOG("hardlink exist, remove old: %s", cbData->fileHandle.m_file->m_fileName);
                SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::UNLINK);
            }
            cbData->timer->Insert(cbData->fileHandle, ONE_THOUSAND_UNIT_CONVERSION);
            cbData->pktStats->Increment(PKT_TYPE::HARDLINK, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            return;
        }
        ++cbData->controlInfo->m_noOfFilesFailed;
        ERRLOG("write file failed: %s, totalFailed: %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
            cbData->controlInfo->m_noOfFilesFailed.load());
        delete cbData;
        cbData = nullptr;
        return;
    }

    cbData->hardlinkMap->IncreaseRef(cbData->fileHandle.m_file->m_inode);
    ++cbData->controlInfo->m_noOfFilesCopied;
    delete cbData;
    cbData = nullptr;
}

int SendUnlinkRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);
    DBGLOG("Send Unlink: %s", smbPath.c_str());
    if (cbData->writeSmbContext->SmbUnlinkAsync(smbPath.c_str(), WriterCallBack, cbData) != SUCCESS) {
        ERRLOG("Failed to send unlink req: %s err: %s", cbData->fileHandle.m_file->m_fileName,
               cbData->writeSmbContext->SmbGetError());
        return FAILED;
    }
    return SUCCESS;
}

void SmbUnlinkCb(struct smb2_context *smb2, int status, void *data, SmbWriterCommonData *cbData)
{
    smb2 = smb2;
    data = data;
    if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
        cbData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
        SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
        delete cbData;
        cbData = nullptr;
        return;
    }
    delete cbData;
    cbData = nullptr;
}

int SendResetAttrRequest(FileHandle &fileHandle, SmbWriterCommonData *cbData)
{
    string smbPath = RemoveFirstSeparator(fileHandle.m_file->m_fileName);
    ConcatRootPath(smbPath, cbData->params.dstRootPath);

    uint32_t fileMode = cbData->fileHandle.m_file->m_fileAttr;
    fileMode &= ~(1UL << READONLY_ATTR_POS);
    fileMode &= ~(1UL << HIDDEN_ATTR_POS);

    DBGLOG("ResetAttr file %s: attr %d btime %d atime %d mtime %d ctime %d", fileHandle.m_file->m_fileName.c_str(),
        fileMode, cbData->fileHandle.m_file->m_btime, cbData->fileHandle.m_file->m_atime,
        cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime);
    struct SMB2_BASIC_INFO basic_info = {fileMode, cbData->fileHandle.m_file->m_btime,
        cbData->fileHandle.m_file->m_atime, cbData->fileHandle.m_file->m_mtime, cbData->fileHandle.m_file->m_ctime,
        0, 0, 0, 0};
    int ret = cbData->writeSmbContext->SmbSetBasicInfoAsync(
        smbPath.c_str(), &basic_info, WriterCallBack, cbData);
    if (ret != SUCCESS) {
        ERRLOG("Send SetBasicInfo Request Failed: %s", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    return SUCCESS;
}

void SmbResetAttrCb(struct smb2_context *smb2, int status, void * /* data */, SmbWriterCommonData *cbData)
{
    if (status < SUCCESS) {
        ERRLOG("SetBasicInfo failed file:%s, status:%s, errno:%d, retryCnt:%d",
            cbData->fileHandle.m_file->m_fileName.c_str(), smb2_get_error(smb2), status, cbData->fileHandle.m_retryCnt);
        if (IfNeedRetry(cbData->fileHandle.m_retryCnt, DEFAULT_ERROR_SINGLE_FILE_CNT, status)) {
            cbData->pktStats->Increment(PKT_TYPE::SETMETA, PKT_COUNTER::FAILED, 1, PKT_ERROR::RETRIABLE_ERR);
            SmbEnqueueToTimer(cbData->timer, cbData->fileHandle);
            delete cbData;
            cbData = nullptr;
            return;
        }
        cbData->fileHandle.m_file->SetFlag(ATTR_RESETED);
        ++cbData->controlInfo->m_noOfFilesFailed;
        ERRLOG("write file failed: %s, totalFailed: %llu", cbData->fileHandle.m_file->m_fileName.c_str(),
            cbData->controlInfo->m_noOfFilesFailed.load());
        delete cbData;
        cbData = nullptr;
        return;
    }
    // to open dst
    ++cbData->fileHandle.m_retryCnt;
    cbData->fileHandle.m_file->SetFlag(ATTR_RESETED);
    DBGLOG("file set flag attr_reseted: %s", cbData->fileHandle.m_file->m_fileName.c_str());
    SendWriterRequest(cbData->fileHandle, cbData, LibsmbEvent::OPEN_DST);
}
