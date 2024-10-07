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
#include "LibnfsHardlinkWriter.h"

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

namespace {
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
    constexpr uint64_t MIN_BACKUP_QUEUE_SIZE = 8000;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 10;
    constexpr uint64_t MAX_THREADPOOL_TASK = 32;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

LibnfsHardlinkWriter::LibnfsHardlinkWriter(const WriterParams &hardlinkWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : WriterBase(hardlinkWriterParams),
    m_nfsContextContainer(m_backupParams.commonParams.reqID),
    m_syncNfsContextContainer(m_backupParams.commonParams.reqID)
{
    m_failureRecorder = failureRecorder;
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams);

    m_pktStats = make_shared<PacketStats>();
    m_fileHandleCache = make_shared<FileHandleCache>();

    m_writeQueue->RegisterPredicate(CanRecv, this);
    BackupQueueConfig config {};
    config.maxSize = MAX_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_writeWaitQueue = make_shared<BackupQueue<FileHandle>>(config);

    config.maxSize = MAX_BACKUP_QUEUE_SIZE;
    m_mkdirSyncQueue = make_shared<BackupQueue<FileHandle>>(config);

    m_commonData.syncNfsContextContainer = &m_syncNfsContextContainer;
    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.readQueue = m_readQueue;
    m_commonData.writeQueue = m_writeQueue;
    m_commonData.writeWaitQueue = m_writeWaitQueue;
    m_commonData.pktStats = m_pktStats;
    m_commonData.controlInfo = m_controlInfo;
    m_commonData.hardlinkMap = m_hardlinkMap;
    m_commonData.abort = &m_abort;
    m_commonData.timer = &m_timer;
    m_commonData.IsResumeSendCb = IsResumeSendCb;
    m_commonData.ResumeSendCb = ResumeSendCb;
    m_commonData.commonObj = this;
    m_commonData.skipFailure = m_backupParams.commonParams.skipFailure;
    m_commonData.writeDisable = m_backupParams.commonParams.writeDisable;
    m_commonData.writeMeta = m_backupParams.commonParams.writeMeta;
    m_commonData.failureRecorder = m_failureRecorder;

    FillNfsServerCheckParams();
}

LibnfsHardlinkWriter::~LibnfsHardlinkWriter()
{
    if (m_mkdirThread.joinable()) {
        m_mkdirThread.join();
    }

    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode LibnfsHardlinkWriter::Start()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsHardlinkWriter start!");
    if (FillWriteContainers() != MP_SUCCESS) {
        DeleteWriteContainers();
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        ERRLOG("Create Write NFS containers failed");
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&LibnfsHardlinkWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Writer thread creation failed");
        DeleteWriteContainers();
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        DeleteWriteContainers();
        return BackupRetCode::FAILED;
    }

    StartMkdirThread();

    return BackupRetCode::SUCCESS;
}

int LibnfsHardlinkWriter::StartMkdirThread()
{
    INFOLOG("StartMkdirThread");
    try {
        m_mkdirThread = thread(&LibnfsHardlinkWriter::MkdirThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Exception when creating MkdirThreadFunc is: %s", e.what());
        return MP_FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

BackupRetCode LibnfsHardlinkWriter::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsHardlinkWriter abort!");
    m_abort = true;
    ResumeRecv();
    ResumeSend();
    m_mkdirSyncQueue->Clear();
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsHardlinkWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsHardlinkWriter::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsHardlinkWriter GetStatus!");
    m_pktStats->Print();
    m_blockBufferMap->Print();

    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, m_failReason);
}

void LibnfsHardlinkWriter::ProcRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        ProcessWriteEntries(fileHandle);
    }
    RatelimitIncreaseMaxPendingRequestCount(LIBNFS_WRITER, m_ratelimitTimer, m_advParams);
}

uint64_t LibnfsHardlinkWriter::GetRetryTimerCnt()
{
    return m_timer.GetCount();
}

bool LibnfsHardlinkWriter::IsRetryReqEmpty()
{
    return (m_timer.GetCount() == 0);
}

int LibnfsHardlinkWriter::OpenFile(FileHandle &fileHandle)
{
    auto nfsfh = m_fileHandleCache->Get(fileHandle.m_file->m_dirName);
    if (ProcessParentFh(fileHandle, m_commonData, nfsfh) != MP_SUCCESS) {
        return MP_FAILED;
    }

    uint32_t openFlag = 0;
    if (m_backupParams.backupType == BackupType::BACKUP_INC) {
        openFlag = O_EXCL;
    }

    return SendCreateRequest(fileHandle, nfsfh, openFlag, m_commonData, m_backupParams);
}

int LibnfsHardlinkWriter::WriteData(FileHandle &fileHandle)
{
    if (fileHandle.m_file->m_size == 0) {
        // Nothing to write. Delete the block, send SETATTR and return
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        fileHandle.m_file->SetDstState(FileDescState::WRITED);
        m_controlInfo->m_noOfFilesCopied++;
        return WriteMeta(fileHandle);
    }

    if (fileHandle.m_retryCnt == 0) {
        fileHandle.m_file->m_blockStats.m_writeReqCnt++;
    }

    return SendWriteRequest(fileHandle, m_commonData, m_blockBufferMap);
}

int LibnfsHardlinkWriter::WriteMeta(FileHandle &fileHandle)
{
    if (m_backupParams.commonParams.writeMeta) {
        return SendSetMetaRequest(fileHandle, m_commonData);
    }

    /* We are skipping Set Meta for the aggregate file. Need to set the state to
     * META_WRITED for the close to happen */
    fileHandle.m_file->SetDstState(FileDescState::META_WRITED);
    return CloseFile(fileHandle);
}

int LibnfsHardlinkWriter::CloseFile(FileHandle &fileHandle)
{
    if (!fileHandle.m_file->IsFlagSet(DST_CLOSED)) {
        fileHandle.m_file->SetFlag(DST_CLOSED);
        if (fileHandle.m_file->dstIOHandle.nfsFh == nullptr) {
            WARNLOG("dstIOHandle.nfsFh is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }

        return SendDstCloseRequest(fileHandle, m_commonData);
    }
    return MP_SUCCESS;
}

bool LibnfsHardlinkWriter::IsReplacePolicySkip() const
{
    if (m_backupParams.backupType == BackupType::RESTORE &&
        m_backupParams.commonParams.restoreReplacePolicy != RestoreReplacePolicy::OVERWRITE) {
        return true;
    }
    return false;
}

void LibnfsHardlinkWriter::PrintIsComplete(bool forcePrint)
{
    string prefixStr {};
    if (forcePrint) {
        prefixStr = "completed";
    } else {
        prefixStr = "in Progress";
    }
    if (forcePrint == true || ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL)) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("LibnfsHardlinkWriter check is %s. aggrComplete %d "
            "(writedFiles %d writeSkippedFiles %d skipFiles %d backupFailedFiles %d)"
            "(totalFiles %d, archiveFiles %d), "
            "writeQueueEmpty: %d, writeWaitQueueEmpty: %d, Pending Count: %lu,"
            "RetryEmpty: %d, mkdirComplete: %d",
            prefixStr.c_str(),
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_controlInfo->m_noOfFilesCopied.load(),
            m_controlInfo->m_noOfFilesWriteSkip.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_archiveFiles.load(),
            m_writeQueue->Empty(),
            m_writeWaitQueue->Empty(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            IsRetryReqEmpty(),
            m_mkdirComplete);
    }
}

bool LibnfsHardlinkWriter::IsComplete()
{
    PrintIsComplete(false);

    bool writeComplete =
        m_controlInfo->m_aggregatePhaseComplete &&
        m_mkdirComplete &&
        m_writeQueue->Empty() &&
        m_writeWaitQueue->Empty() &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0 &&
        IsRetryReqEmpty();
    if (!writeComplete) {
        return false;
    }

    INFOLOG("BlockBufferCount: %llu, BlockBufferSize: %llu",
        m_blockBufferMap->m_blockBufferCount.load(), m_blockBufferMap->m_blockBufferSize.load());

    return true;
}

bool LibnfsHardlinkWriter::CanRecv(void *cbData)
{
    auto *writerObj = static_cast<LibnfsHardlinkWriter *>(cbData);
    return !writerObj->IsBlockRecv();
}

void LibnfsHardlinkWriter::HandleComplete()
{
    INFOLOG("LibnfsHardlinkWriter Enter HandleComplete");
    DeleteWriteContainers();
    FileHandle fileHandle {};
    while (m_writeQueue->TryPop(fileHandle)) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        if (fileHandle.m_file->dstIOHandle.nfsFh != nullptr) {
            FreeNfsFh(fileHandle.m_file->dstIOHandle.nfsFh);
        }
    }

    // Clear the retry MAP
    ExpireRetryTimers(m_timer);
    m_fileHandleCache->Clear();
    PrintIsComplete(true);
    m_pktStats->Print();
    m_blockBufferMap->Print();
    m_controlInfo->m_writePhaseComplete = true;
}

bool LibnfsHardlinkWriter::IsBlockRecv() const
{
    if ((m_writeQueue->GetSizeWithOutLock() + m_writeWaitQueue->GetSizeWithOutLock() +
        m_mkdirSyncQueue->GetSizeWithOutLock())
        >= MAX_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsHardlinkWriter::BlockRecv() const
{
    m_writeQueue->BlockPush();
}

bool LibnfsHardlinkWriter::IsResumeRecv() const
{
    if ((m_writeQueue->GetSizeWithOutLock() + m_writeWaitQueue->GetSizeWithOutLock() +
        m_mkdirSyncQueue->GetSizeWithOutLock())
        < MIN_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsHardlinkWriter::ResumeRecv() const
{
    m_writeQueue->CancelBlockPush();
}

bool LibnfsHardlinkWriter::IsBlockSend()
{
    uint64_t pendingCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) + GetRetryTimerCnt();
    if (pendingCnt >= m_advParams->maxPendingAsyncReqCnt ||
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) >= m_advParams->serverCheckMaxCount) {
        return true;
    } else {
        return false;
    }
}

void LibnfsHardlinkWriter::BlockSend() const
{
    m_writeQueue->BlockPop();
}

bool LibnfsHardlinkWriter::IsResumeSend()
{
    uint64_t pendingCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) + GetRetryTimerCnt();
    if (pendingCnt < m_advParams->minPendingAsyncReqCnt &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) < m_advParams->serverCheckMaxCount) {
        return true;
    } else {
        return false;
    }
}

void LibnfsHardlinkWriter::ResumeSend() const
{
    m_writeQueue->CancelBlockPop();
}

bool LibnfsHardlinkWriter::IsResumeSendCb(void *cbObj)
{
    auto cpyWriter = (LibnfsHardlinkWriter *)cbObj;
    return cpyWriter->IsResumeSend();
}

void LibnfsHardlinkWriter::ResumeSendCb(void *cbObj)
{
    auto cpyWriter = (LibnfsHardlinkWriter *)cbObj;
    cpyWriter->ResumeSend();
}

void LibnfsHardlinkWriter::HandleQueueBlock()
{
    if (IsBlockRecv()) {
        BlockRecv();
    } else if (IsResumeRecv()) {
        ResumeRecv();
    }

    if (IsBlockSend()) {
        BlockSend();
    } else if (IsResumeSend()) {
        ResumeSend();
    }
}

/* check if we can take ThreadFunc into base class */
void LibnfsHardlinkWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsHardlinkWriter main thread start!");
    while (true) {
        if (IsAbort(m_commonData) || NfsServerCheck(m_nasServerCheckParams) == MP_FAILED) {
            INFOLOG("Breaking after Abort");
            break;
        }

        if (IsComplete()) {
            INFOLOG("Breaking after Completed");
            break;
        }

        ProcRetryTimers();      /* Process the retry of requests */
        HandleQueueBlock();     /* Handling of Blocking/Resuming of the BackupQueue */
        ProcessWriteQueue();    /* Pop the entries from writeQueue & process them */

        shared_ptr<NfsContextWrapper> nfs = m_nfsContextContainer.GetCurrContext();
        if (nfs == nullptr) {
            ERRLOG("nfs wrapper is null. NfsServerCheck failed");
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
            break;
        }
        nfs->Poll(POLL_EXPIRE_TIMEOUT);

        BatchPush();            /* Push all the entries in m_writeWaitQueue to m_writeQueue */
    }
    HandleComplete();
    INFOLOG("LibnfsHardlinkWriter main thread end!");
}

void LibnfsHardlinkWriter::ProcessWriteQueue()
{
    while (!m_writeQueue->Empty()) {
        if (IsAbort(m_commonData) || IsBlockSend()) {
            break;
        }
        FileHandle fileHandle {};
        if (!m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        if (ProcessWriteEntries(fileHandle) != MP_SUCCESS) {
            if (IsResumeRecv()) {
                ResumeRecv();
            }
            break;
        }
        if (IsResumeRecv()) {
            ResumeRecv();
        }
    }
}

int LibnfsHardlinkWriter::ProcessWriteEntries(FileHandle &fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();
    if (S_ISDIR(fileHandle.m_file->m_mode)) {
        m_mkdirSyncQueue->Push(fileHandle);
    } else if (fileHandle.m_file->m_scannermode == CTRL_ENTRY_MODE_META_MODIFIED) {
        return ProcessMetaModifiedFiles(fileHandle, state);
    } else if (m_backupParams.backupType == BackupType::RESTORE && state == FileDescState::INIT
        && IS_TO_BE_OPENED(fileHandle)) {
        // In lstat should take care to change the dstState
        return LstatFile(fileHandle, m_commonData, m_backupParams, m_fileHandleCache);
    } else if (state == FileDescState::LINK_DEL || state == FileDescState::LINK_DEL_FAILED) {
        // Should set dstState to LSTAT after link delete
        return LinkDelete(fileHandle, m_commonData, m_blockBufferMap);
    } else if (state == FileDescState::LINK_DEL_FOR_RESTORE) {
        // Should set dstState to LSTAT after link delete
        return LinkDeleteForRestore(fileHandle, m_commonData);
    } else if (state == FileDescState::DIR_DEL || state == FileDescState::DIR_DEL_RESTORE) {
        // Should set dstState to INIT/LSTAT after dir delete
        return DirectoryDelete(fileHandle, m_commonData);
    } else {
        return ProcessFilesCreateWriteClose(fileHandle, state);
    }

    return MP_SUCCESS;
}

int LibnfsHardlinkWriter::ProcessMetaModifiedFiles(FileHandle &fileHandle, FileDescState state)
{
    if (!IS_TO_BE_OPENED(fileHandle)) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        return MP_SUCCESS;
    }
    if (m_backupParams.backupType == BackupType::RESTORE && state == FileDescState::INIT) {
        // In lstat should take care to change the dstState
        return LstatFile(fileHandle, m_commonData, m_backupParams, m_fileHandleCache);
    } else {
        if (fileHandle.m_file->GetSrcState() == FileDescState::LINK) {
            fileHandle.m_file->SetDstState(FileDescState::LINK);
            return CreateHardlink(fileHandle, m_commonData, m_backupParams, m_fileHandleCache);
        }
        if (S_ISLNK(fileHandle.m_file->m_mode)) {
            // Symlink
            return WriteSymLinkMeta(fileHandle, m_commonData);
        } else {
            // Normal files with METABACKUP_TYPE_FILE_AND_FOLDER
            return WriteMeta(fileHandle);
        }
    }

    return MP_SUCCESS;
}

int LibnfsHardlinkWriter::ProcessFilesCreateWriteClose(FileHandle &fileHandle, FileDescState state)
{
    if (IS_FILE_COPY_FAILED(fileHandle) || state == FileDescState::WRITE_SKIP) {
        if (fileHandle.m_file->m_blockStats.m_writeRespCnt == fileHandle.m_file->m_blockStats.m_writeReqCnt) {
            // In close callback, should check this to send to link delete queue
            // For symlink & mknod files the dstFileHandle will be null. So will be ignored from sending close
            CloseFile(fileHandle);
        }

        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);

        return MP_SUCCESS;
    }

    if (fileHandle.m_file->GetSrcState() == FileDescState::LINK) {
        fileHandle.m_file->SetDstState(FileDescState::LINK);
        return CreateHardlink(fileHandle, m_commonData, m_backupParams, m_fileHandleCache);
    }

    if (S_ISLNK(fileHandle.m_file->m_mode)) {
        // Symlink
        return ProcessSymlink(fileHandle, state);
    }

    if (state == FileDescState::INIT || state == FileDescState::LSTAT) {
        // We should send create request only if blockSize & seq, both are 0. Others will be for writing.
        if (IS_TO_BE_OPENED(fileHandle)) {
            return ProcessFileOpen(fileHandle);
        } else {
            // Pushing to m_writeWaitQueue as the dst file is yet to be opened
            m_writeWaitQueue->Push(fileHandle);
        }
    } else if (state == FileDescState::DST_OPENED || state == FileDescState::PARTIAL_WRITED) {
        return WriteData(fileHandle);
    } else if (state == FileDescState::WRITED) {
        return WriteMeta(fileHandle);
    } else if (state == FileDescState::META_WRITED) {
        return CloseFile(fileHandle);
    }

    return MP_SUCCESS;
}

int LibnfsHardlinkWriter::ProcessSymlink(FileHandle &fileHandle, FileDescState state)
{
    if (state == FileDescState::INIT || state == FileDescState::LSTAT) {
        return CreateSymlink(fileHandle, m_commonData, m_fileHandleCache, m_blockBufferMap);
    } else {
        return WriteSymLinkMeta(fileHandle, m_commonData);
    }

    return MP_SUCCESS;
}

int LibnfsHardlinkWriter::ProcessFileOpen(FileHandle &fileHandle)
{
    if (IS_SPECIAL_DEVICE_FILE(fileHandle)) {
        return CreateSpecialFile(fileHandle, m_commonData, m_fileHandleCache);
    } else {
        return OpenFile(fileHandle);
    }

    return MP_SUCCESS;
}

void LibnfsHardlinkWriter::BatchPush() const
{
    FileHandle fileHandle {};
    while (m_writeWaitQueue->TryPop(fileHandle)) {
        m_writeQueue->Push(fileHandle);
    }
}

void LibnfsHardlinkWriter::MkdirThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("MkdirThreadFunc started");

    while (true) {
        if (IsAbort(m_commonData)) {
            INFOLOG("Breaking after Abort");
            break;
        }

        if (m_nasServerCheckParams.suspend) {
            std::this_thread::sleep_for(chrono::milliseconds(DEFAULT_FAILURE_SLEEP));
            continue;
        }

        if (m_controlInfo->m_aggregatePhaseComplete && m_mkdirSyncQueue->Empty() && m_writeQueue->Empty()) {
            INFOLOG("Breaking mkdir thread after completed");
            break;
        }

        FileHandle fileHandle {};
        if (m_mkdirSyncQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            MakeDirectory(fileHandle, m_commonData, m_fileHandleCache);
        }

        if (IsResumeRecv()) {
            ResumeRecv();
        }
    }
    m_mkdirComplete = true;
    INFOLOG("MkdirThreadFunc end!");
}

void LibnfsHardlinkWriter::FillNfsServerCheckParams()
{
    m_nasServerCheckParams.pktStats = m_pktStats;
    m_nasServerCheckParams.controlInfo = m_controlInfo;
    m_nasServerCheckParams.failReason = m_failReason;
    m_nasServerCheckParams.nfsContextContainer = &m_syncNfsContextContainer;
    m_nasServerCheckParams.advParams = m_advParams;
    m_nasServerCheckParams.backupType = m_backupParams.backupType;
    m_nasServerCheckParams.direction = "Writer";
    m_nasServerCheckParams.phase = m_backupParams.phase;
    m_nasServerCheckParams.ratelimitTimer = m_ratelimitTimer;
}

int LibnfsHardlinkWriter::FillWriteContainers()
{
    INFOLOG(" Server IP: %s, sharePath: %s", m_advParams->ip.c_str(), m_advParams->sharePath.c_str());
    string dstRootPath = NFS_URL + m_advParams->ip + SEP + m_advParams->sharePath + SEP;

    if (!FillNfsContextContainer(dstRootPath, WRITE_NFS_CONTEXT_CNT, m_nfsContextContainer, m_backupParams,
        m_advParams->serverCheckSleepTime)) {
        return MP_FAILED;
    }

    if (!FillNfsContextContainer(dstRootPath, SERVER_CHECK_NFS_CONTEXT_CNT, m_syncNfsContextContainer,
        m_backupParams, m_advParams->serverCheckSleepTime)) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

void LibnfsHardlinkWriter::DeleteWriteContainers()
{
    m_nfsContextContainer.DestroyNfsContext();
    m_syncNfsContextContainer.DestroyNfsContext();
}
