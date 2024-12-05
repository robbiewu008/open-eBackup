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
#include "LibsmbCopyReader.h"
#include "Libsmb.h"
#include "log/Log.h"
#include "BackupConstants.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr int RECONNECT_CONTEXT_RETRY_TIMES = 5;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr int OPENED_FILEHANDLE_REACH_THRESHOLD = 10000;
    constexpr int DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
    constexpr int ONE_THOUSAND_UNIT_CONVERSION = 1000;
}

LibsmbCopyReader::LibsmbCopyReader(const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : ReaderBase(copyReaderParams)
{
    INFOLOG("Construct LibsmbCopyReader!");
    m_failureRecorder = failureRecorder;
    m_srcAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams);
    FillContextParams(m_params.srcSmbContextArgs, m_srcAdvParams);
    INFOLOG("serverCheckMaxCount: %d!", m_srcAdvParams->serverCheckMaxCount);
    BackupQueueConfig config {};
    config.maxSize = MAX_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_pktStats = make_shared<PacketStats>();

    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_params.srcRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams)->rootPath;
}

LibsmbCopyReader::~LibsmbCopyReader()
{
    INFOLOG("Destruct LibsmbCopyReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbCopyReader::SmbConnectContexts()
{
    m_asyncContext = SmbConnectContext(m_params.srcSmbContextArgs);
    if (m_asyncContext == nullptr) {
        ERRLOG("Libsmb Copy Reader Create Context failed, context is nullptr");
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbCopyReader::SmbDisconnectContexts()
{
    INFOLOG("Reader SmbDisconnectContexts");
    SmbDisconnectContext(m_asyncContext);
}

BackupRetCode LibsmbCopyReader::Start()
{
    INFOLOG("LibsmbCopyReader start!");

    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = thread(&LibsmbCopyReader::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        SmbDisconnectContexts();
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        SmbDisconnectContexts();
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbCopyReader::Abort()
{
    INFOLOG("LibsmbCopyReader Abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbCopyReader::Destroy()
{
    INFOLOG("LibsmbCopyReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or LibsmbCopyReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbCopyReader::GetStatus()
{
    m_pktStats->Print();
    if (!m_controlInfo->m_readPhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (m_abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (m_failed || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        return m_failReason;
    }
    return BackupPhaseStatus::COMPLETED;
}

bool LibsmbCopyReader::IsAbort() const
{
    if (m_abort || m_failed || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlInfoFailed %d controlReaderFailed %d",
            m_abort, m_failed, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool LibsmbCopyReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("SmbCopyReader:Controlfilereader: %d "
            "(pending packet counts %d readQueueSize %d partialReadQueueSize %d timerSize %d) "
            "(noOfFilesRead %d noOfDirRead %d noOfFilesReadFaile %d "
            "skipFileCnt %d skipDir %d unaggregatedFiles %llu emptyFiles %llu unaggregatedFaildFiles %llu) "
            "(totalFiles %d totalDir %d unarchiveFiles %llu adsFilesCnt %llu) BufferSize: %llu",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_readQueue->GetSize(), m_partialReadQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load(), m_adsFileCnt.load(), m_blockBufferMap->GetTotalBufferSize());
    }
    if ((m_controlInfo->m_controlReaderPhaseComplete) && m_readQueue->Empty() && (m_timer.GetCount() == 0) &&
        (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0) &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt +
        m_controlInfo->m_unaggregatedFiles + m_controlInfo->m_emptyFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_adsFileCnt +
        m_controlInfo->m_unarchiveFiles))) {
        INFOLOG("SmbCopyReader:Controlfilereader: %d "
            "(pending packet counts %d readQueueSize %d partialReadQueueSize %d timerSize %d) "
            "(noOfFilesRead %d noOfDirRead %d noOfFilesReadFaile %d "
            "skipFileCnt %d skipDir %d unaggregatedFiles %llu emptyFiles %llu unaggregatedFaildFiles %llu) "
            "(totalFiles %d totalDir %d unarchiveFiles %llu, adsFiles %llu) BufferSize: %llu",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_readQueue->GetSize(), m_partialReadQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load(), m_adsFileCnt.load(), m_blockBufferMap->GetTotalBufferSize());
        return true;
    }
    return false;
}

void LibsmbCopyReader::HandleComplete()
{
    INFOLOG("Complete LibsmbCopyReader");
    m_controlInfo->m_readPhaseComplete = true;
}

int LibsmbCopyReader::ServerCheck()
{
    /* If max no-space pending count is reached, abort the job */
    uint32_t noAccessErrCount = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_ACCESS_ERR);
    if (noAccessErrCount >= DEFAULT_MAX_NOACCESS) {
        ERRLOG("Threshold reached for DEFAULT_MAX_NOACCESS");
        m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
        m_failed = true;
        return FAILED;
    }
    /* Nas Server Check for Source side */
    uint32_t retryableErrCount = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR);
    if (retryableErrCount >= m_srcAdvParams->serverCheckMaxCount) {
        ERRLOG("Threshold reached calling src servercheck");
        int retVal = HandleConnectionException(m_asyncContext, m_params.srcSmbContextArgs,
            RECONNECT_CONTEXT_RETRY_TIMES);
        if (retVal != SUCCESS) {
            ERRLOG("Stop and Abort read phase due to server inaccessible");
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
            m_failed = true;
            return FAILED;
        } else {
            INFOLOG("Nas Server reachable");
            m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
        }
    }

    return SUCCESS;
}

int64_t LibsmbCopyReader::ProcessTimers()
{
    if (IsReaderRequestReachThreshold()) {
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        DBGLOG("SmbCopyReader memroy(%llu) or PENDING packet(%d) reach threshold.",
            totalBufferSize, m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        return 0;
    }
    vector<FileHandle> fileHandles;
    int expiredCnt =
        m_srcAdvParams->maxPendingAsyncReqCnt - m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING);
    int64_t delayVal = m_timer.GetExpiredEventAndTime(fileHandles, expiredCnt);
    DBGLOG("Process timer get time %d, count: %d", delayVal, m_timer.GetCount());
    if (delayVal > POLL_MAX_TIMEOUT) {
        delayVal = POLL_MAX_TIMEOUT;
    }
    for (FileHandle& fh : fileHandles) {
        if (IsReaderRequestReachThreshold()) {
            m_timer.Insert(fh, PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND);
            continue;
        }
        DBGLOG("Process timer file: %s", fh.m_file->m_fileName.c_str());
        ProcessFileDescState(fh);
    }
    return delayVal;
}

/* check if we can take ThreadFunc into base class */
void LibsmbCopyReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbCopyReader main thread start!");
    int ret = 0;
    while (true) {
        // 检查context连接是否还正常，若已断开，会尝试重连
        if (ServerCheck() != SUCCESS) {
            break;
        }
        // must judge IsComplete after generate task
        if (IsComplete()) {
            INFOLOG("Read phase is complete!");
            break;
        }
        if (IsAbort()) {
            INFOLOG("LibsmbCopyReader main thread abort!");
            break;
        }
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_srcAdvParams->maxPendingAsyncReqCnt ||
            totalBufferSize > m_backupParams.commonParams.maxBufferSize) {
            DBGLOG("SmbCopyReader PENDING packet(%d) or totalBufferSize(%llu) reach threshold.",
                m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING), totalBufferSize);
            ret = m_asyncContext->Poll(m_srcAdvParams->pollExpiredTime);
            if (ret < 0 && ProcessConnectionException() != SUCCESS) {
                break;
            }
            continue;
        }

        ProcessPartialReadEntries();

        ProcessReadEntries();

        ProcessTimers();

        ret = m_asyncContext->Poll(m_srcAdvParams->pollExpiredTime);
        // ret < 0说明连接有问题，需要重连, 如果ProcessConnectionException也返回失败，说明重连失败
        if (ret < 0 && ProcessConnectionException() != SUCCESS) {
            break;
        }
    }
    INFOLOG("LibsmbCopyReader main thread end!");
    SmbDisconnectContexts();
    HandleComplete();
    m_threadDone = true;
    return;
}

int LibsmbCopyReader::ProcessConnectionException()
{
    ERRLOG("src connection exception");
    int retVal = HandleConnectionException(m_asyncContext, m_params.srcSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
    if (retVal != SUCCESS) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
        m_failed = true;
        return FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
    return SUCCESS;
}

void LibsmbCopyReader::ProcessReadEntries()
{
    while (!m_readQueue->Empty()) {
        if (IsAbort()) {
            break;
        }
        if (IsReaderRequestReachThreshold()) {
            break;
        }
        FileHandle fileHandle;
        if (!m_readQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ProcessFileDescState(fileHandle);
    }
}

void LibsmbCopyReader::ProcessPartialReadEntries()
{
    while (!m_partialReadQueue->Empty()) {
        if (IsAbort()) {
            break;
        }
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        if (totalBufferSize > m_backupParams.commonParams.maxBufferSize ||
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_srcAdvParams->maxPendingAsyncReqCnt) {
            DBGLOG("SmbCopyReader memroy(%llu) or PENDING packet(%d) reach threshold.", totalBufferSize,
                m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
            break;
        }
        FileHandle fileHandle;
        if (!m_partialReadQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ProcessFileDescState(fileHandle);
    }
}

bool LibsmbCopyReader::IsReaderRequestReachThreshold() const
{
    uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
    int64_t openedCount = GetActualOpenedFileHandleCount(m_pktStats);
    if (totalBufferSize > m_backupParams.commonParams.maxBufferSize ||
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) >
        m_srcAdvParams->maxPendingAsyncReqCnt ||
        openedCount >= m_srcAdvParams->maxOpenedFilehandleCount) {
        DBGLOG("SmbCopyReader memroy(%llu) or PENDING packet(%d) or openedCount(%d) reach threshold."
            "open_sent(%d), open_failed(%d), open_retry(%d), close_recvd(%d)",
            totalBufferSize, m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING), openedCount,
            m_pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::SENT),
            m_pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::FAILED),
            m_pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::RETRIABLE_ERR),
            m_pktStats->GetValue(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD));
        return true;
    }
    return false;
}

void LibsmbCopyReader::HandleFileOnlyMetaModified(FileHandle &fileHandle)
{
    DBGLOG("%s is only meta modified", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    m_aggregateQueue->Push(fileHandle);
    ++m_controlInfo->m_readProduce;
    ++m_controlInfo->m_noOfFilesRead;
}

void LibsmbCopyReader::ProcessFileDescState(FileHandle fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("ProcessFileDescState: %s, state: %d, retryCnt: %d",
        fileHandle.m_file->m_fileName.c_str(), (int)state, fileHandle.m_retryCnt);

    if (fileHandle.IsOnlyMetaModified()) {
        HandleFileOnlyMetaModified(fileHandle);
        return;
    }

    if (IsFileReadOrWriteFailed(fileHandle) || fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP) {
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        ++fileHandle.m_file->m_blockStats.m_readRespCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readRespCnt) {
            CloseFile(fileHandle);
        }
        return;
    }

    ProcessFileDescStateInteraction(fileHandle);
    return;
}

void LibsmbCopyReader::ProcessFileDescStateInteraction(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    if (state == FileDescState::INIT || state == FileDescState::AGGREGATED) {
        if (fileHandle.IsDir()) {
            m_aggregateQueue->Push(fileHandle); // send to aggregate immediately
            ++m_controlInfo->m_readProduce;
        }
        ReadMeta(fileHandle); // check ADS
    } else if (state == FileDescState::META_READED) {
        if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            OpenFile(fileHandle);
        } else {
            m_aggregateQueue->Push(fileHandle);
            ++m_controlInfo->m_readProduce;
        }
    } else if (state == FileDescState::SRC_OPENED || state == FileDescState::PARTIAL_READED) {
        ReadData(fileHandle);
    } else if (state == FileDescState::READED) {
        // 文件读完或文件读失败都需要close
        CloseFile(fileHandle);
    } else if (state == FileDescState::FILEHANDLE_INVALID) {
        if (fileHandle.m_block.m_seq == 0) {
            OpenFile(fileHandle);
        } else {
            m_timer.Insert(fileHandle, ONE_THOUSAND_UNIT_CONVERSION);
        }
    }
}

void LibsmbCopyReader::Init(FileHandle fileHandle)
{
    ++m_controlInfo->m_readConsume;
}

int LibsmbCopyReader::OpenFile(FileHandle &fileHandle)
{
    if (fileHandle.m_file->m_size == 0) {
        m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        m_aggregateQueue->Push(fileHandle);
        ++m_controlInfo->m_readProduce;
        ++m_controlInfo->m_noOfFilesRead;
        return SUCCESS;
    }
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::OPEN_SRC) != SUCCESS) {
        return FAILED;
    }

    return SUCCESS;
}

int LibsmbCopyReader::ReadData(FileHandle &fileHandle)
{
    return ReadNormalData(fileHandle);
}

int LibsmbCopyReader::ReadMeta(FileHandle &fileHandle)
{
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::ADS) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbCopyReader::CloseFile(FileHandle &fileHandle)
{
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::CLOSE_SRC) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbCopyReader::ReadNormalData(FileHandle& fileHandle)
{
    ++fileHandle.m_file->m_blockStats.m_readReqCnt;
    // 读非空文件
    DBGLOG("Enter ReadData: %s block info: %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);

    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        ReadNormalDataRequestFailed(fileHandle);
        return FAILED;
    }

    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::READ) != SUCCESS) {
        ReadNormalDataRequestFailed(fileHandle);
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbCopyReader::ReadNormalDataRequestFailed(FileHandle& fileHandle)
{
    ++fileHandle.m_file->m_blockStats.m_readRespCnt;
    if (!IsFileReadOrWriteFailed(fileHandle)) {
        if (!fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
            ++m_controlInfo->m_noOfFilesFailed;
            ERRLOG("Read failed for file : %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
                m_controlInfo->m_noOfFilesFailed.load());
        }
    }
    if (fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
        ++m_controlInfo->m_noOfFilesReadFailed;
    }
    fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
    m_aggregateQueue->Push(fileHandle);
    ++m_controlInfo->m_readProduce;
    if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readRespCnt) {
        CloseFile(fileHandle);
    }
}

void LibsmbCopyReader::FillSmbReaderCommonData(SmbReaderCommonData *readerCommonData)
{
    readerCommonData->readSmbContext = m_asyncContext;
    readerCommonData->readQueue = m_readQueue;
    readerCommonData->partialReadQueue = m_partialReadQueue;
    readerCommonData->aggregateQueue = m_aggregateQueue;
    readerCommonData->blockBufferMap = m_blockBufferMap;
    readerCommonData->params = m_params;

    readerCommonData->timer = &m_timer;
    readerCommonData->controlInfo = m_controlInfo;
    readerCommonData->pktStats = m_pktStats;

    readerCommonData->adsFileCnt = &m_adsFileCnt;
    readerCommonData->failureRecorder = m_failureRecorder;
}

SmbReaderCommonData* LibsmbCopyReader::GetSmbReaderCommonData(FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbReaderCommonData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    FillSmbReaderCommonData(cbData);
    cbData->fileHandle = fileHandle;
    return cbData;
}
