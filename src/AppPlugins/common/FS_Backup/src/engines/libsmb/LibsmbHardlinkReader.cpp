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
#include "LibsmbHardlinkReader.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int OOM_SLEEP_SECOND = 1;
    constexpr int RECONNECT_CONTEXT_RETRY_TIMES = 5;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr int OPENED_FILEHANDLE_REACH_THRESHOLD = 10000;
    constexpr int DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
}

LibsmbHardlinkReader::LibsmbHardlinkReader(const ReaderParams &hardlinkReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : ReaderBase(hardlinkReaderParams)
{
    INFOLOG("Construct LibsmbHardlinkReader!");
    m_failureRecorder = failureRecorder;
    m_srcAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams);
    FillContextParams(m_params.srcSmbContextArgs, m_srcAdvParams);
    INFOLOG("serverCheckMaxCount: %d!", m_srcAdvParams->serverCheckMaxCount);
    m_pktStats = make_shared<PacketStats>();

    BackupQueueConfig config {};
    config.maxSize = MAX_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_partialReadQueue = make_shared<BackupQueue<FileHandle>>(config);
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_params.srcRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.srcAdvParams)->rootPath;
}

LibsmbHardlinkReader::~LibsmbHardlinkReader()
{
    INFOLOG("Destruct LibsmbHardlinkReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbHardlinkReader::SmbConnectContexts()
{
    m_asyncContext = SmbConnectContext(m_params.srcSmbContextArgs);
    if (m_asyncContext == nullptr) {
        ERRLOG("Libsmb Hardlink Reader Create Context failed, context is nullptr");
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbHardlinkReader::SmbDisconnectContexts()
{
    INFOLOG("LibsmbHardlinkReader SmbDisconnectContexts");
    SmbDisconnectContext(m_asyncContext);
}

BackupRetCode LibsmbHardlinkReader::Start()
{
    INFOLOG("Start LibsmbHardlinkReader!");
        
    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&LibsmbHardlinkReader::ThreadFunc, this);
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

BackupRetCode LibsmbHardlinkReader::Abort()
{
    INFOLOG("LibsmbHardlinkReader Abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbHardlinkReader::Destroy()
{
    INFOLOG("LibsmbHardlinkReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or LibsmbHardlinkReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibsmbHardlinkReader::GetStatus()
{
    return FSBackupUtils::GetReaderStatus(m_controlInfo, m_abort);
}

bool LibsmbHardlinkReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool LibsmbHardlinkReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("controlReaderComplete %d "
            "(pending packet counts %d readQueueSize %d partialReadQueueSize %d timerSize %d) "
            "(noOfFilesRead %d noOfDirRead %d noOfFilesReadFailed %d "
            "skipFileCnt %d skipDirCnt %d unaggregatedFiles %llu emptyFiles %llu) (totalFiles %d totalDir %d)"
            "readQueueEmpty: %d",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_readQueue->GetSize(), m_partialReadQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(), m_readQueue->Empty());
    }
    if ((m_controlInfo->m_controlReaderPhaseComplete) &&
        m_readQueue->Empty() && m_partialReadQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0)) {
        INFOLOG("controlReaderComplete %d (pending packet counts %d) "
            "(m_noOfFilesRead %d m_noOfDirRead %d m_noOfFilesReadFailed %d "
            "skipFileCnt %d skipDirCnt %d unaggregatedFiles %llu emptyFiles %llu) (totalFiles %d totalDir %d)"
            "readQueueEmpty: %d",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(), m_readQueue->Empty());
        return true;
    }
    return false;
}

void LibsmbHardlinkReader::HandleComplete()
{
    INFOLOG("Complete LibsmbHardlinkReader");
    m_controlInfo->m_readPhaseComplete = true;
}

int LibsmbHardlinkReader::ServerCheck()
{
    /* If max no-space pending count is reached, abort the job */
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_ACCESS_ERR) >= DEFAULT_MAX_NOACCESS) {
        ERRLOG("Threshold reached for DEFAULT_MAX_NOACCESS");
        m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
        m_controlInfo->m_failed = true;
        return FAILED;
    }
    /* Nas Server Check for Source side */
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR) >= m_srcAdvParams->serverCheckMaxCount) {
        ERRLOG("Threshold reached calling src servercheck");
        int ret = HandleConnectionException(m_asyncContext, m_params.srcSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
        if (ret != SUCCESS) {
            ERRLOG("Stop and Abort read phase due to server inaccessible");
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
            m_controlInfo->m_failed = true;
            return FAILED;
        } else {
            INFOLOG("Server reachable");
            m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
        }
    }
    return SUCCESS;
}

int64_t LibsmbHardlinkReader::ProcessTimers()
{
    if (IsReaderRequestReachThreshold()) {
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        DBGLOG("SmbCopyReader memroy(%llu) or PENDING packet(%d) reach threshold.",
            totalBufferSize, m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        return 0;
    }
    vector<FileHandle> fileHandles;
    int expiredCount =
        m_srcAdvParams->maxPendingAsyncReqCnt - m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING);
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles, expiredCount);
    DBGLOG("Process timer get time %d, count: %d", delay, m_timer.GetCount());
    if (delay > POLL_MAX_TIMEOUT) {
        delay = POLL_MAX_TIMEOUT;
    }
    for (FileHandle& fh : fileHandles) {
        if (IsReaderRequestReachThreshold()) {
            m_timer.Insert(fh, PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND);
            continue;
        }
        DBGLOG("Process timer file: %s", fh.m_file->m_fileName.c_str());
        ProcessFileDescState(fh);
    }
    return delay;
}


void LibsmbHardlinkReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start LibsmbHardlinkReader main thread!");
    int ret = 0;
    while (true) {
        if (ServerCheck() != SUCCESS) {
            break;
        }
        // must judge IsComplete after generate task
        if (IsComplete()) {
            INFOLOG("Read phase is complete!");
            break;
        }
        if (IsAbort()) {
            INFOLOG("Abort LibsmbHardlinkReader main thread!");
            break;
        }
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > MAX_PENDING_REQUEST_COUNT ||
            totalBufferSize > m_backupParams.commonParams.maxBufferSize) {
            DBGLOG("SmbCopyReader PENDING packet(%d) reach threshold.",
                m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
            ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
            if (ret < 0 && ProcessConnectionException() != SUCCESS) {
                break;
            }
            continue;
        }
        ProcessPartialReadEntries();

        ProcessReadEntries();

        ProcessTimers();

        ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
        if (ret < 0 && ProcessConnectionException() != SUCCESS) {
            break;
        }
    }
    SmbDisconnectContexts();
    HandleComplete();
    INFOLOG("LibsmbCopyReader main thread end!");
    m_threadDone = true;
    return;
}

int LibsmbHardlinkReader::ProcessConnectionException()
{
    WARNLOG("src connection exception");
    int ret = HandleConnectionException(m_asyncContext, m_params.srcSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
    if (ret != SUCCESS) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason);
        m_controlInfo->m_failed = true;
        return FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
    return SUCCESS;
}

void LibsmbHardlinkReader::ProcessReadEntries()
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

void LibsmbHardlinkReader::ProcessPartialReadEntries()
{
    while (!m_partialReadQueue->Empty()) {
        if (IsAbort()) {
            break;
        }
        uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
        if (totalBufferSize > m_backupParams.commonParams.maxBufferSize ||
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > MAX_PENDING_REQUEST_COUNT) {
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

bool LibsmbHardlinkReader::IsReaderRequestReachThreshold() const
{
    uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
    if (totalBufferSize > m_backupParams.commonParams.maxBufferSize ||
        m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > MAX_PENDING_REQUEST_COUNT ||
        (m_pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::SENT) -
        m_pktStats->GetValue(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD) >= MAX_OPENED_FILEHANDLE_COUNT)) {
        DBGLOG("SmbCopyReader memroy(%llu) or PENDING packet(%d) or openedCount(%d) reach threshold.",
            totalBufferSize, m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_pktStats->GetValue(PKT_TYPE::OPEN, PKT_COUNTER::SENT) -
            m_pktStats->GetValue(PKT_TYPE::CLOSE, PKT_COUNTER::RECVD));
        return true;
    }
    return false;
}

void LibsmbHardlinkReader::ProcessFileDescState(FileHandle fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    if (IsFileReadOrWriteFailed(fileHandle) || fileHandle.m_file->GetDstState() == FileDescState::WRITE_SKIP) {
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_block.m_seq) { // 最后一个block
            CloseFile(fileHandle);
        }
        return;
    }

    if (state == FileDescState::INIT || state == FileDescState::META_READED) {
        OpenFile(fileHandle);
    }
    if (state == FileDescState::SRC_OPENED || state == FileDescState::PARTIAL_READED) {
        ReadData(fileHandle);
    } else if (state == FileDescState::READED) {
        CloseFile(fileHandle);
    }
    if (state == FileDescState::LINK) {
        fileHandle.m_file->SetDstState(FileDescState::LINK);
        m_aggregateQueue->Push(fileHandle);
        ++m_controlInfo->m_noOfFilesRead;
        ++m_controlInfo->m_readProduce;
        DBGLOG("HardlinkReader pass link %s to aggregate", fileHandle.m_file->m_fileName.c_str());
    }
    return;
}

void LibsmbHardlinkReader::Init(FileHandle fileHandle)
{
    ++m_controlInfo->m_readConsume;
}

int LibsmbHardlinkReader::OpenFile(FileHandle& fileHandle)
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

int LibsmbHardlinkReader::ReadData(FileHandle& fileHandle)
{
    return ReadNormalData(fileHandle);
}

int LibsmbHardlinkReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle.m_file->SetSrcState(FileDescState::META_READED);
    m_readQueue->Push(fileHandle);
    return SUCCESS;
}

int LibsmbHardlinkReader::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("CloseFile :%s.", fileHandle.m_file->m_fileName.c_str());
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }

    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::CLOSE_SRC) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbHardlinkReader::ReadNormalData(FileHandle& fileHandle)
{
    // 读非空文件
    DBGLOG("Enter ReadData: %s block info: %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);

    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
    auto cbData = GetSmbReaderCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    if (SendReaderRequest(fileHandle, cbData, LibsmbEvent::READ) != SUCCESS) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
        return FAILED;
    }
    return SUCCESS;
}


void LibsmbHardlinkReader::FillSmbReaderCommonData(SmbReaderCommonData *readerCommonData)
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
    readerCommonData->failureRecorder = m_failureRecorder;
}

SmbReaderCommonData* LibsmbHardlinkReader::GetSmbReaderCommonData(FileHandle &fileHandle)
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
