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
#include "NfsWormWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Libnfscommonmethods;

namespace {
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
    constexpr uint64_t MIN_BACKUP_QUEUE_SIZE = 8000;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr uint64_t MAX_THREADPOOL_TASK = 32;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr int RETRY_WAIT_IN_SEC = 1;
    const int MAX_RETRY_TIMES = 3;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

NfsWormWriter::NfsWormWriter(const AntiWriterParams &antiWriterParams) : AntiRansomwareWriter(antiWriterParams),
    m_nfsContextContainer(dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.dstAdvParams)->reqID),
    m_syncNfsContextContainer(dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.dstAdvParams)->reqID)
{
    m_advParams = dynamic_pointer_cast<NfsAntiRansomwareAdvanceParams>(m_backupParams.dstAdvParams);
    m_pktStats = make_shared<PacketStats>();
    m_params.abort = &m_abort;
    m_fileHandleCache = make_shared<FileHandleCache>();
    m_params.fileHandleCache = m_fileHandleCache;

    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_syncNfsContextContainer;
    m_commonData.readQueue = m_readQueue;
    m_commonData.writeQueue = m_writeQueue;
    m_commonData.pktStats = m_pktStats;
    m_commonData.controlInfo = m_controlInfo;
    m_commonData.timer = &m_timer;
    m_commonData.abort = &m_abort;
    m_commonData.commonObj = this;
    m_commonData.IsResumeSendCb = IsResumeSendCb;
    m_commonData.ResumeSendCb = ResumeSendCb;
    m_commonData.skipFailure = m_backupParams.commonParams.skipFailure;
    m_commonData.writeDisable = m_backupParams.commonParams.writeDisable;
    m_commonData.writeMeta = m_backupParams.commonParams.writeMeta;
}

NfsWormWriter::~NfsWormWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct NfsWormWriter, destroy thread pool %s", m_threadPoolKey.c_str());
}

bool NfsWormWriter::IsResumeSendCb(void *cbObj)
{
    auto cpyWriter = (NfsWormWriter *)cbObj;
    return cpyWriter->IsResumeSend();
}

void NfsWormWriter::ResumeSendCb(void *cbObj)
{
    auto cpyWriter = (NfsWormWriter *)cbObj;
    cpyWriter->ResumeSend();
}

bool NfsWormWriter::IsResumeSend()
{
    uint64_t pendingCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) + GetRetryTimerCnt();
    if (pendingCnt < m_advParams->minPendingAsyncReqCnt &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) < m_advParams->serverCheckMaxCount) {
        return true;
    } else {
        return false;
    }
}

void NfsWormWriter::ResumeSend()
{
    m_writeQueue->CancelBlockPop();
}

BackupRetCode NfsWormWriter::Start()
{
    if (FillWriteContainers() != MP_SUCCESS) {
        m_nfsContextContainer.DestroyNfsContext();
        m_syncNfsContextContainer.DestroyNfsContext();
        m_failReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
        ERRLOG("Create Write NFS containers failed");
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&NfsWormWriter::ThreadFunc, this);
    } catch (exception &e) {
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

int NfsWormWriter::FillWriteContainers()
{
    INFOLOG(" Server IP: %s, sharePath: %s", m_advParams->ip.c_str(), m_advParams->sharePath.c_str());
    string dstRootPath = NFS_URL + m_advParams->ip + SEP + m_advParams->sharePath + SEP;

    if (!FillNfsContextContainer(dstRootPath, WRITE_NFS_CONTEXT_CNT, m_nfsContextContainer, m_backupParams,
        m_advParams->serverCheckSleepTime)) {
        ERRLOG("Create m_nfsContextContainer failed");
        return MP_FAILED;
    }

    if (!FillNfsContextContainer(dstRootPath, SERVER_CHECK_NFS_CONTEXT_CNT, m_syncNfsContextContainer,
        m_backupParams, m_advParams->serverCheckSleepTime)) {
        ERRLOG("Create m_syncNfsContextContainer failed");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

BackupRetCode NfsWormWriter::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

bool NfsWormWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool NfsWormWriter::IsComplete()
{
    bool writeComplete =
        m_controlInfo->m_readPhaseComplete &&
        m_writeQueue->Empty() &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0 &&
        IsRetryReqEmpty();
    if (!writeComplete) {
        return false;
    }

    return true;
}

void NfsWormWriter::HandleComplete()
{
    INFOLOG("Complete NfsWormWriter");
    m_nfsContextContainer.DestroyNfsContext();
    m_syncNfsContextContainer.DestroyNfsContext();
    m_controlInfo->m_writePhaseComplete = true;
}

void NfsWormWriter::ThreadFunc()
{
    INFOLOG("NfsWormWriter main thread start!");
    while (true) {
        if (IsAbort() || NfsServerCheck() == MP_FAILED) {
            WARNLOG("NfsWormWriter main thread abort");
            break;
        }

        if (IsComplete()) {
            break;
        }

        ProcRetryTimers();

        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("get file desc from write queue :%s.", fileHandle.m_file->m_fileName.c_str());
            ++m_controlInfo->m_writerConsume;
            if (SendSetMetaRequest(fileHandle) != MP_SUCCESS) {
                ERRLOG("set backup to failed!");
                m_controlInfo->m_failed = true;
                continue;
            }
            ++m_controlInfo->m_noOfFilesCopied;
        }
        
        shared_ptr<NfsContextWrapper> nfs = m_nfsContextContainer.GetCurrContext();
        nfs->Poll(POLL_EXPIRE_TIMEOUT);
    }
    HandleComplete();
    INFOLOG("NfsWormWriter main thread end!");
    return;
}

int NfsWormWriter::SendSetMetaRequest(FileHandle &fileHandle)
{
    NfsSetMetaCbData *cbData = CreateSetMetaCbData(fileHandle, m_commonData);
    if (cbData == nullptr) {
        return MP_FAILED;
    }
    
    if (SendNfsRequest(fileHandle, cbData, LibnfsEvent::WRITE_META) != MP_SUCCESS) {
        HandleSendNfsRequestFailure(fileHandle);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

void NfsWormWriter::HandleSendNfsRequestFailure(FileHandle &fileHandle)
{
    if (!IS_FILE_COPY_FAILED(fileHandle)) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
    }

    if (!m_backupParams.commonParams.skipFailure) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
    }

    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
}

int NfsWormWriter::NfsServerCheck()
{
    /* Nas Server Check for Destination side */
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) < m_advParams->serverCheckMaxCount) {
        return MP_SUCCESS;
    }
    ERRLOG("Threshold reached calling dst servercheck");
    shared_ptr<NfsContextWrapper> nfs = m_syncNfsContextContainer.GetCurrContext();
    if (NasServerCheck(nfs, m_advParams->serverCheckSleepTime, m_advParams->serverCheckRetry) == MP_FAILED) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        m_failReason = BackupPhaseStatus::FAILED_SEC_SERVER_NOTREACHABLE;
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        return MP_FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);

    return MP_SUCCESS;
}

int NfsWormWriter::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int NfsWormWriter::WriteData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int NfsWormWriter::WriteMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int NfsWormWriter::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

void NfsWormWriter::ProcRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        m_writeQueue->Push(fileHandle);
    }
}

uint64_t NfsWormWriter::GetRetryTimerCnt()
{
    return m_timer.GetCount();
}

bool NfsWormWriter::IsRetryReqEmpty()
{
    return (m_timer.GetCount() == 0);
}
