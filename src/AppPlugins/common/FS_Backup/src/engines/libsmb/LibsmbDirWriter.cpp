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
#include "LibsmbDirWriter.h"
#include "log/Log.h"
#include "LibsmbWriterInterface.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr auto DIR_MTIME_MAX_RETRY_TIMES = 5;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr int DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr int RECONNECT_CONTEXT_RETRY_TIMES = 5;
}

LibsmbDirWriter::LibsmbDirWriter(const WriterParams &dirWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(dirWriterParams)
{
    m_failureRecorder = failureRecorder;
    m_dstAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams);
    FillContextParams(m_params.dstSmbContextArgs, m_dstAdvParams);
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_pktStats = make_shared<PacketStats>();
    m_params.dstRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams)->rootPath;
}

LibsmbDirWriter::~LibsmbDirWriter()
{
    INFOLOG("Destruct LibsmbDirWriter!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbDirWriter::SmbConnectContexts()
{
    m_asyncContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_asyncContext == nullptr) {
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbDirWriter::SmbDisconnectContexts()
{
    SmbDisconnectContext(m_asyncContext);
}

BackupRetCode LibsmbDirWriter::Start()
{
    INFOLOG("LibsmbDirWrite call Start!");

    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&LibsmbDirWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        SmbDisconnectContexts();
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        SmbDisconnectContexts();
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDirWriter::Abort()
{
    INFOLOG("LibsmbDirWriter Abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbDirWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbDirWriter::GetStatus()
{
    m_pktStats->Print();
    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, BackupPhaseStatus::FAILED);
}

int LibsmbDirWriter::OpenFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

SmbWriterCommonData* LibsmbDirWriter::GetSmbWriterCommonData(FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    cbData->writeSmbContext = m_asyncContext;
    cbData->writeQueue = m_writeQueue;
    cbData->params = m_params;
    cbData->controlInfo = m_controlInfo;
    cbData->pktStats = m_pktStats;
    cbData->fileHandle = fileHandle;
    cbData->failureRecorder = m_failureRecorder;
    return cbData;
}

int LibsmbDirWriter::WriteMeta(FileHandle &fileHandle)
{
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_BASIC_INFO_DIR) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbDirWriter::WriteData(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int LibsmbDirWriter::CloseFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

bool LibsmbDirWriter::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        HandleComplete();
        return true;
    }
    return false;
}

bool LibsmbDirWriter::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("aggrComplete %d (noOfDirCopied %d noOfDirFailed %d)"
            "(controlFileReaderProduce %d) pending packet counts %d",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_controlInfo->m_noOfDirCopied.load(), m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_controlFileReaderProduce.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
    }
    if ((m_controlInfo->m_aggregatePhaseComplete) && m_writeQueue->Empty() &&
        (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0)) {
        INFOLOG("aggrComplete %d (noOfDirCopied %d noOfDirFailed %d)"
            "(controlFileReaderProduce %d) pending packet counts %d",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_controlInfo->m_noOfDirCopied.load(), m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_controlFileReaderProduce.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        HandleComplete();
        return true;
    }
    return false;
}

void LibsmbDirWriter::HandleComplete()
{
    INFOLOG("Complete LibsmbCopyWriter");
    m_controlInfo->m_writePhaseComplete = true;
}

bool LibsmbDirWriter::IsWriterRequestReachThreshold() const
{
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_dstAdvParams->maxPendingAsyncReqCnt) {
        DBGLOG("LibsmbDirWriter PENDING packet(%d) reach threshold.",
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        return true;
    }
    return false;
}

void LibsmbDirWriter::ProcessWriteEntries()
{
    while (!m_writeQueue->Empty()) {
        if (IsAbort() || IsWriterRequestReachThreshold()) {
            break;
        }
        FileHandle fileHandle;
        if (!m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ++m_controlInfo->m_writerConsume;
        FileDescState state = fileHandle.m_file->GetDstState();
        if (state == FileDescState::INIT) {
            WriteMeta(fileHandle);
        }
    }
}

int LibsmbDirWriter::ProcessConnectionException()
{
    ERRLOG("dst connection exception");
    int retCode = HandleConnectionException(m_asyncContext, m_params.dstSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
    if (retCode != SUCCESS) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        m_failed = true;
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        return FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
    return SUCCESS;
}

/* check if we can take ThreadFunc into base class */
void LibsmbDirWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbDirWriter main thread start!");
    int ret = 0;
    while (true) {
        if (IsComplete()) {
            break;
        }
        if (IsAbort()) {
            WARNLOG("LibsmbDirWriter main thread abort!");
            break;
        }
        if (IsWriterRequestReachThreshold()) {
            ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
            if (ret < 0 && ProcessConnectionException() != SUCCESS) {
                break;
            }
            continue;
        }
        ProcessWriteEntries();
        ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
        // ret < 0说明连接有问题，需要重连, 如果ProcessConnectionException也返回失败，说明重连失败
        if (ret < 0 && ProcessConnectionException() != SUCCESS) {
            break;
        }
    }
    SmbDisconnectContexts();
    INFOLOG("LibsmbDirWriter main thread end!");
    return;
}
