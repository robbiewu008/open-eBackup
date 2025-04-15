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
#include "LibnfsCopyReader.h"

namespace {
    constexpr uint64_t MIN_BACKUP_QUEUE_SIZE = 8000;
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 10;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

using namespace std;
using namespace Module;
using namespace Libnfscommonmethods;

LibnfsCopyReader::LibnfsCopyReader(const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : ReaderBase(copyReaderParams),
    m_nfsContextContainer(m_backupParams.commonParams.reqID)
{
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.srcAdvParams);

    BackupQueueConfig config {};
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    config.maxSize = MAX_BACKUP_QUEUE_SIZE;
    m_pktStats = make_shared<PacketStats>();
    m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_readQueue->RegisterPredicate(CanRecv, this);
    m_failureRecorder = failureRecorder;

    m_commonData.nfsContextContainer = &m_nfsContextContainer;
    m_commonData.syncNfsContextContainer = &m_syncNfsContextContainer;
    m_commonData.pktStats = m_pktStats;
    m_commonData.controlInfo = m_controlInfo;
    m_commonData.readQueue = m_readQueue;
    m_commonData.aggregateQueue = m_aggregateQueue;
    m_commonData.failureRecorder = m_failureRecorder;
    m_commonData.timer = &m_timer;
    m_commonData.abort = &m_abort;
    m_commonData.commonObj = this;
    m_commonData.IsResumeSendCb = IsResumeSendCb;
    m_commonData.ResumeSendCb = ResumeSendCb;
    m_commonData.skipFailure = m_backupParams.commonParams.skipFailure;
    m_commonData.writeDisable = m_backupParams.commonParams.writeDisable;

    FillNfsServerCheckParams();
}

LibnfsCopyReader::~LibnfsCopyReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

BackupRetCode LibnfsCopyReader::Start()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsCopyReader start!");
    if (FillReadContainers() != MP_SUCCESS) {
        DeleteReadContainers();
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
        ERRLOG("Create Read NFS containers failed");
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&LibnfsCopyReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        DeleteReadContainers();
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        DeleteReadContainers();
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsCopyReader::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsCopyReader abort!");
    m_abort = true;
    ResumeRecv();
    ResumeSend();
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsCopyReader::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsCopyReader::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsCopyReader GetStatus!");
    m_pktStats->Print();
    m_blockBufferMap->Print();

    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort, m_failReason);
}

void LibnfsCopyReader::ProcRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        ProcessReadEntries(fileHandle);
    }

    RatelimitIncreaseMaxPendingRequestCount(LIBNFS_READER, m_ratelimitTimer, m_advParams);
}

uint64_t LibnfsCopyReader::GetRetryTimerCnt()
{
    return m_timer.GetCount();
}

bool LibnfsCopyReader::IsRetryReqEmpty()
{
    return (m_timer.GetCount() == 0);
}

int LibnfsCopyReader::OpenFile(FileHandle &fileHandle)
{
    if (fileHandle.m_retryCnt == 0) {
        // Send to write thread for creation of the file
        PushToAggregator(fileHandle, m_commonData);
    }

    if (fileHandle.m_file->m_size == 0) {
        // Zero size file. Nothing to read. So no need to send nfs_open request
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);

        // Start ReadData to create the 0 size block and make blockBufferMap entry
        return ReadData(fileHandle);
    }

    // If fh is not available from scanner, send open request
    if (fileHandle.m_file->srcIOHandle.nfsFh == nullptr || fileHandle.m_file->srcIOHandle.nfsFh->fh.len == 0) {
        return SendOpenRequest(fileHandle, m_commonData);
    }

    // FileHandle available form scanner. Start Reading
    fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
    return ReadData(fileHandle);
}

int LibnfsCopyReader::ReadMeta(FileHandle &fileHandle)
{
    DBGLOG("ReadMeta: %s", fileHandle.m_file->m_fileName.c_str());
    return MP_SUCCESS;
}

int LibnfsCopyReader::ReadData(FileHandle &fileHandle)
{
    if (fileHandle.m_retryCnt != 0 && fileHandle.m_block.m_size != 0) {
        // This is a retry. Directly send read request
        return SendReadRequest(fileHandle, m_commonData, m_blockBufferMap);
    }

    if (fileHandle.m_file->m_size == 0) {
        // Zero size file. Nothing to read. So skip sending nfs_read request
        HandleZeroSizeFileRead(fileHandle, m_commonData, m_blockBufferMap);
        return MP_SUCCESS;
    }

    uint64_t fileSize = fileHandle.m_file->m_size;
    uint64_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t totalCnt = (fileSize / blockSize) + (((fileSize % blockSize) != 0) ? 1 : 0);

    fileHandle.m_file->m_blockStats.m_totalCnt = totalCnt;
    fileHandle.m_file->SetSrcState(FileDescState::PARTIAL_READED);

    while (fileHandle.m_file->m_blockStats.m_readReqCnt < totalCnt) {
        if (IsAbort(m_commonData)) {
            break;
        }
        if (IsBlockSend()) {
            m_partialReadQueue->Push(fileHandle);
            return MP_FAILED;
        }
        if (fileHandle.m_file->IsFlagSet(BADHANDLE_ERR_HIT)) {
            WARNLOG("Badhandle hit for %s", fileHandle.m_file->m_fileName.c_str());
            m_partialReadQueue->Push(fileHandle);
            return MP_FAILED;
        }
        if (IS_FILE_COPY_FAILED(fileHandle) || fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP) {
            if (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_readReqCnt) {
                CloseFile(fileHandle);
            }
            break;
        }

        if (ConstructReadBlock(fileHandle, blockSize) != MP_SUCCESS) {
            HandleSendReaderNfsRequestFailure(fileHandle, m_commonData);
            return MP_FAILED;
        }
        fileHandle.m_file->m_blockStats.m_readReqCnt++;
        m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);

        if (SendReadRequest(fileHandle, m_commonData, m_blockBufferMap) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }

    return MP_SUCCESS;
}

int LibnfsCopyReader::CloseFile(FileHandle &fileHandle)
{
    if (!fileHandle.m_file->IsFlagSet(SRC_CLOSED)) {
        fileHandle.m_file->SetFlag(SRC_CLOSED);
        if (fileHandle.m_file->srcIOHandle.nfsFh == nullptr) {
            WARNLOG("srcIOHandle.nfsFh is nullptr: %s", fileHandle.m_file->m_fileName.c_str());
            return MP_FAILED;
        }
        return SendSrcCloseRequest(fileHandle, m_commonData);
    }
    return MP_SUCCESS;
}

void LibnfsCopyReader::PrintIsComplete(bool forcePrint)
{
    string prefixStr {};
    if (forcePrint) {
        prefixStr = "completed";
    } else {
        prefixStr = "in Progress";
    }
    if (forcePrint == true || ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL)) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("LibnfsCopyReader check is %s. controlReaderComplete %d "
            "(readedFiles %d readedDir %d readFailedFiles %d "
            "skipFileCnt %d skipDirCnt %d unaggregatedFiles %llu emptyFiles %llu unaggregatedFailedFiles %llu) "
            "(totalFiles %d totalDir %d unarchiveFiles %llu),"
            "ReadQueue empty: %d, partialReadQueue empty: %d, Pending Count: %lu, RetryEmpty: %d, "
            "readQueueSize: %u, partialReadQueueSize: %u",
            prefixStr.c_str(), m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load(),
            m_readQueue->Empty(), m_partialReadQueue->Empty(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING), IsRetryReqEmpty(),
            m_readQueue->GetSizeWithOutLock(), m_partialReadQueue->GetSizeWithOutLock());
    }
}

bool LibnfsCopyReader::IsComplete()
{
    PrintIsComplete(false);

    bool readComplete =
        m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty() &&
        m_partialReadQueue->Empty() &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0 &&
        IsRetryReqEmpty() &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt + m_controlInfo->m_unaggregatedFiles +
        m_controlInfo->m_emptyFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_controlInfo->m_unarchiveFiles));
    if (!readComplete) {
        return false;
    }

    INFOLOG("BlockBufferCount: %llu, BlockBufferSize: %llu",
        m_blockBufferMap->m_blockBufferCount.load(), m_blockBufferMap->m_blockBufferSize.load());
    return true;
}

void LibnfsCopyReader::HandleComplete()
{
    INFOLOG("LibnfsCopyReader Enter HandleComplete");
    DeleteReadContainers();

    FileHandle fileHandle {};
    while (m_partialReadQueue->TryPop(fileHandle)) {
        if (fileHandle.m_file->srcIOHandle.nfsFh != nullptr) {
            FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
        }
    }

    while (m_readQueue->TryPop(fileHandle)) {
        if (fileHandle.m_file->srcIOHandle.nfsFh != nullptr) {
            FreeNfsFh(fileHandle.m_file->srcIOHandle.nfsFh);
        }
    }

    // Clear the retry map
    ExpireRetryTimers(m_timer);
    PrintIsComplete(true);
    m_pktStats->Print();
    m_blockBufferMap->Print();
    m_controlInfo->m_readPhaseComplete = true;
}

bool LibnfsCopyReader::CanRecv(void *cbData)
{
    auto *readObj = static_cast<LibnfsCopyReader *>(cbData);
    return !readObj->IsBlockRecv();
}

bool LibnfsCopyReader::IsBlockRecv()
{
    if ((m_partialReadQueue->GetSizeWithOutLock() + m_readQueue->GetSizeWithOutLock()) >= MAX_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsCopyReader::BlockRecv()
{
    m_readQueue->BlockPush();
}

bool LibnfsCopyReader::IsResumeRecv() const
{
    if ((m_partialReadQueue->GetSizeWithOutLock() + m_readQueue->GetSizeWithOutLock()) < MIN_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsCopyReader::ResumeRecv()
{
    m_readQueue->CancelBlockPush();
}

bool LibnfsCopyReader::IsBlockSend()
{
    uint64_t pendingCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) + GetRetryTimerCnt();
    uint64_t retriableErrCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR);
    if (pendingCnt >= m_advParams->maxPendingAsyncReqCnt ||
        m_blockBufferMap->m_blockBufferCount >= m_backupParams.commonParams.maxBufferCnt ||
        m_blockBufferMap->m_blockBufferSize >= m_backupParams.commonParams.maxBufferSize ||
        retriableErrCnt >= m_advParams->serverCheckMaxCount) {
        DBGLOG("pendingCnt: %llu, maxPendingAsyncReqCnt: %u, blockBufferCount: %llu, maxBufferCnt: %u, "
            "blockBufferSize: %llu, maxBufferSize: %u, retriableErrCnt: %u, serverCheckMaxCount: %u",
            pendingCnt, m_advParams->maxPendingAsyncReqCnt,
            m_blockBufferMap->m_blockBufferCount.load(),
            m_backupParams.commonParams.maxBufferCnt,
            m_blockBufferMap->m_blockBufferSize.load(),
            m_backupParams.commonParams.maxBufferSize,
            retriableErrCnt, m_advParams->serverCheckMaxCount);
        return true;
    } else {
        return false;
    }
}

void LibnfsCopyReader::BlockSend() const
{
    m_readQueue->BlockPop();
    m_partialReadQueue->BlockPop();
}

bool LibnfsCopyReader::IsResumeSend()
{
    uint64_t pendingCnt = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) + GetRetryTimerCnt();
    if (pendingCnt < m_advParams->minPendingAsyncReqCnt &&
        m_blockBufferMap->m_blockBufferCount < m_backupParams.commonParams.maxBufferCnt &&
        m_blockBufferMap->m_blockBufferSize < m_backupParams.commonParams.maxBufferSize &&
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) < m_advParams->serverCheckMaxCount) {
        return true;
    } else {
        return false;
    }
}

void LibnfsCopyReader::ResumeSend() const
{
    m_readQueue->CancelBlockPop();
    m_partialReadQueue->CancelBlockPop();
}

bool LibnfsCopyReader::IsResumeSendCb(void *cbObj)
{
    auto cpyReader = (LibnfsCopyReader *)cbObj;
    return cpyReader->IsResumeSend();
}

void LibnfsCopyReader::ResumeSendCb(void *cbObj)
{
    auto cpyReader = (LibnfsCopyReader *)cbObj;
    cpyReader->ResumeSend();
}

void LibnfsCopyReader::HandleQueueBlock()
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
void LibnfsCopyReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsCopyReader main thread started!");
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

        /* If the size of m_readBlockAsyncQueue hits the threshold, no more blocks will be send for that file.
         * The file will be pushed to m_partialReadQueue. The next time prefernece will be given for the
         * entries in m_partialReadQueue */
        ProcessPartialReadQueue();
        ProcessReadQueue();     /* Pop the entries from readQueue & process them */

        shared_ptr<NfsContextWrapper> nfs = m_nfsContextContainer.GetCurrContext();
        if (nfs == nullptr) {
            ERRLOG("nfs wrapper is null");
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
            break;
        }
        nfs->Poll(POLL_EXPIRE_TIMEOUT);
    }
    HandleComplete();
    INFOLOG("LibnfsCopyReader main thread end!");
    m_threadDone = true;
}

void LibnfsCopyReader::ProcessPartialReadQueue()
{
    while (!m_partialReadQueue->Empty()) {
        if (IsAbort(m_commonData) || IsBlockSend()) {
            break;
        }
        FileHandle fileHandle {};
        if (!m_partialReadQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        if (ProcessReadEntries(fileHandle) != MP_SUCCESS) {
            break;
        }
    }
}

void LibnfsCopyReader::ProcessReadQueue()
{
    if (!m_partialReadQueue->Empty()) {
        return;
    }

    while (!m_readQueue->Empty()) {
        if (IsAbort(m_commonData) || IsBlockSend()) {
            break;
        }
        FileHandle fileHandle {};
        if (!m_readQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ProcessReadEntries(fileHandle);
    }
}

int LibnfsCopyReader::ProcessReadEntries(FileHandle &fileHandle)
{
    // Check if there is any chance that dir with same name of file to be restored, may shown as file exists
    // check the case for mm also
    int ret = ProcessFileTypes(fileHandle);
    if (IsResumeRecv()) {
        ResumeRecv();
    }
    return ret;
}

int LibnfsCopyReader::ProcessFileTypes(FileHandle &fileHandle)
{
    if (S_ISLNK(fileHandle.m_file->m_mode) && fileHandle.m_file->GetSrcState() != FileDescState::AGGREGATED) {
        // If control Entry is symlink, send readLink request to get targetPath
        return ReadLink(fileHandle, m_commonData, m_blockBufferMap);
    } else if (IS_SPECIAL_DEVICE_FILE(fileHandle)) {
        // Special device files has nothing to read. So directly send to write
        m_controlInfo->m_noOfFilesRead++;
        if (!m_backupParams.commonParams.writeDisable) {
            PushToAggregator(fileHandle, m_commonData);
        } else {
            m_controlInfo->m_noOfFilesCopied++;
        }
    } else {
        return ProcessFileToRead(fileHandle);
    }

    return MP_SUCCESS;
}

int LibnfsCopyReader::ProcessFileToRead(FileHandle &fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();

    if (IS_FILE_COPY_FAILED(fileHandle) || fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP) {
        if (fileHandle.m_file->m_blockStats.m_readRespCnt == fileHandle.m_file->m_blockStats.m_readReqCnt) {
            CloseFile(fileHandle);
        }
        return MP_SUCCESS;
    }

    if (state == FileDescState::INIT) {
        return OpenFile(fileHandle);
    } else if (state == FileDescState::SRC_OPENED || state == FileDescState::PARTIAL_READED) {
        return ReadData(fileHandle);
    } else if (state == FileDescState::READED) {
        return CloseFile(fileHandle);
    }

    return MP_SUCCESS;
}

void LibnfsCopyReader::FillNfsServerCheckParams()
{
    m_nasServerCheckParams.pktStats = m_pktStats;
    m_nasServerCheckParams.controlInfo = m_controlInfo;
    m_nasServerCheckParams.failReason = m_failReason;
    m_nasServerCheckParams.nfsContextContainer = &m_syncNfsContextContainer;
    m_nasServerCheckParams.advParams = m_advParams;
    m_nasServerCheckParams.backupType = m_backupParams.backupType;
    m_nasServerCheckParams.direction = "Reader";
    m_nasServerCheckParams.phase = m_backupParams.phase;
    m_nasServerCheckParams.ratelimitTimer = m_ratelimitTimer;
}

int LibnfsCopyReader::FillReadContainers()
{
    string srcRootPath = NFS_URL + m_advParams->ip + m_advParams->sharePath + SEP;

    if (!FillNfsContextContainer(srcRootPath, READ_NFS_CONTEXT_CNT, m_nfsContextContainer, m_backupParams,
        m_advParams->serverCheckSleepTime)) {
        return MP_FAILED;
    }

    if (!FillNfsContextContainer(srcRootPath, SERVER_CHECK_NFS_CONTEXT_CNT, m_syncNfsContextContainer,
        m_backupParams, m_advParams->serverCheckSleepTime)) {
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

void LibnfsCopyReader::DeleteReadContainers()
{
    m_nfsContextContainer.DestroyNfsContext();
    m_syncNfsContextContainer.DestroyNfsContext();
}