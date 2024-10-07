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
#include "LibsmbCopyWriter.h"
#include <sys/stat.h>
#include "Libsmb.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    constexpr auto RECONNECT_CONTEXT_RETRY_TIMES = 5;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    const int PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND = 5000;
    const int PENDING_PACKET_REACH_THRESHOLD_SLEEP_SECOND = 1;
    constexpr auto DEFAULT_POLL_EXPIRED_TIME = 100;
    constexpr int ADS_FILE_BLOCK_MILLISECOND = 1 * 1000;
    constexpr int ONE_THOUSAND_UNIT_CONVERSION = 1000;
}

LibsmbCopyWriter::LibsmbCopyWriter(const WriterParams &copyWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(copyWriterParams)
{
    INFOLOG("Construct LibsmbCopyWriter!");
    m_failureRecorder = failureRecorder;
    m_dstAdvParams = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams);
    FillContextParams(m_params.dstSmbContextArgs, m_dstAdvParams);
    INFOLOG("serverCheckMaxCount: %d!", m_dstAdvParams->serverCheckMaxCount);
    m_pktStats = make_shared<PacketStats>();

    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.dstRootPath = dynamic_pointer_cast<LibsmbBackupAdvanceParams>(m_backupParams.dstAdvParams)->rootPath;
}

LibsmbCopyWriter::~LibsmbCopyWriter()
{
    INFOLOG("Destruct LibsmbCopyWriter!");
    if (m_syncThread.joinable()) {
        m_syncThread.join();
    }
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

int LibsmbCopyWriter::SmbConnectContexts()
{
    m_asyncContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_asyncContext == nullptr) {
        ERRLOG("Smb connect contexts failed!");
        return FAILED;
    }
    m_syncContext = SmbConnectContext(m_params.dstSmbContextArgs);
    if (m_syncContext == nullptr) {
        ERRLOG("Smb connect contexts failed!");
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbCopyWriter::SmbDisconnectContexts()
{
    INFOLOG("Write SmbDisconnectContexts");
    SmbDisconnectContext(m_asyncContext);
}

void LibsmbCopyWriter::SmbDisconnectSyncContexts()
{
    INFOLOG("Write SmbDisconnectSyncContexts");
    SmbDisconnectContext(m_syncContext);
}

BackupRetCode LibsmbCopyWriter::Start()
{
    INFOLOG("LibsmbCopyWriter call Start!");

    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_dirQueue = make_shared<BackupQueue<FileHandle>>(config);

    if (SmbConnectContexts() != SUCCESS) {
        return BackupRetCode::FAILED;
    }
    try {
        m_syncThread = thread(&LibsmbCopyWriter::SyncThreadFunc, this);
        m_thread = thread(&LibsmbCopyWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        SmbDisconnectContexts();
        SmbDisconnectSyncContexts();
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        SmbDisconnectContexts();
        SmbDisconnectSyncContexts();
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbCopyWriter::Abort()
{
    INFOLOG("LibsmbCopyWriter Enter Abort");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibsmbCopyWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}


BackupPhaseStatus LibsmbCopyWriter::GetStatus()
{
    m_pktStats->Print();
    if (!m_controlInfo->m_writePhaseComplete) {
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

bool LibsmbCopyWriter::IsAbort() const
{
    if (m_abort || m_failed || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlInfoFailed %d controlReaderFailed %d",
            m_abort, m_failed, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool LibsmbCopyWriter::IsComplete()
{
    /* in write phase, faild item should contain not only write failed but also read failed */
    /* thus, use BackupControlInfo::m_noOfFilesFailed and BackupControlInfo::m_noOfDirFailed */
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("smbCopyWriter aggrComplete %d "
            "(pending packet counts %d, writeQueueSize %d, dirQueueSize %d, writeCacheSize %d) "
            "(noOfFilesCopied %d noOfDirCopied %d aggrFiles %d "
            "skipFiles %d skipDir %d backupFailedFiles %d backupFailedDir %d m_unaggregatedFaildFiles %llu) "
            "(totalFiles %d, totalDirs %d archiveFiles %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_writeQueue->GetSize(), m_dirQueue->GetSize(), m_writeCache.size(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_aggregatedFiles.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_archiveFiles.load());
    }

    if ((m_controlInfo->m_aggregatePhaseComplete) &&
        (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) == 0) &&
        (m_timer.GetCount() == 0) &&
        m_writeQueue->Empty() &&
        m_dirQueue->Empty() &&
        (m_writeCache.size() == 0)) {
        INFOLOG("smbCopyWriter aggrComplete %d "
            "(pending packet counts %d, writeQueueSize %d, dirQueueSize %d, writeCacheSize %d) "
            "(noOfFilesCopied %d noOfDirCopied %d aggrFiles %d "
            "skipFiles %d skipDir %d backupFailedFiles %d backupFailedDir %d m_unaggregatedFaildFiles %llu) "
            "(totalFiles %d, totalDirs %d archiveFiles %d)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING),
            m_writeQueue->GetSize(), m_dirQueue->GetSize(), m_writeCache.size(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_aggregatedFiles.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_archiveFiles.load());
        return true;
    }

    return false;
}

bool LibsmbCopyWriter::IsMkdirComplete() const
{
    if (m_controlInfo->m_writePhaseComplete && m_dirQueue->Empty()) {
        DBGLOG("CopyWriter mkdir thread complete!");
        return true;
    }
    return false;
}

void LibsmbCopyWriter::HandleComplete()
{
    INFOLOG("Complete LibsmbCopyWriter");
    m_controlInfo->m_writePhaseComplete = true;
}

int LibsmbCopyWriter::OpenFile(FileHandle &fileHandle)
{
    if (fileHandle.m_file->m_mode == FILE_IS_ADS_FILE || fileHandle.m_file->m_mode == FILE_HAVE_ADS) {
        DBGLOG("Block ADS Open flag set as true:%s", fileHandle.m_file->m_fileName.c_str());
        m_isBlockAdsOpen = true;
    }
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::OPEN_DST) != SUCCESS) {
        return FAILED;
    }
    DBGLOG("Enter OpenFile:%s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int LibsmbCopyWriter::WriteMeta(FileHandle &fileHandle)
{
    // 对于文件，接收到setsd的响应后，会继续发setbasicinfo请求
    // 对于目录，setbasicinfo请求会在dirmtime阶段再发
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::SET_SD) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbCopyWriter::WriteData(FileHandle &fileHandle)
{
    ++fileHandle.m_file->m_blockStats.m_writeReqCnt;
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        WriteDataRequestFailed(fileHandle);
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::WRITE) != SUCCESS) {
        WriteDataRequestFailed(fileHandle);
        return FAILED;
    }
    return SUCCESS;
}

void LibsmbCopyWriter::WriteDataRequestFailed(FileHandle &fileHandle)
{
    ++fileHandle.m_file->m_blockStats.m_writeRespCnt;
    if (!IsFileReadOrWriteFailed(fileHandle)) {
        ++m_controlInfo->m_noOfFilesFailed;
        ERRLOG("Write failed for file: %s, totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesFailed.load());
    }
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
    if (fileHandle.m_file->m_blockStats.m_writeRespCnt == fileHandle.m_file->m_blockStats.m_totalCnt) {
        CloseFile(fileHandle);
    }
}

int LibsmbCopyWriter::CloseFile(FileHandle &fileHandle)
{
    if (fileHandle.m_file->dstIOHandle.smbFh == nullptr) {
        DBGLOG("OpenFile, dstIoHandle is nullptr:%s", fileHandle.m_file->m_fileName.c_str());
        return SUCCESS;
    }
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::CLOSE_DST) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbCopyWriter::DeleteFile(FileHandle &fileHandle)
{
    auto cbData = GetSmbWriterCommonData(fileHandle);
    if (cbData == nullptr) {
        return FAILED;
    }
    if (SendWriterRequest(fileHandle, cbData, LibsmbEvent::UNLINK) != SUCCESS) {
        return FAILED;
    }
    return SUCCESS;
}

int LibsmbCopyWriter::ServerCheck()
{
    /* If max no-space pending count is reached, abort the job */
    uint32_t noSpaceErrCount = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_SPACE_ERR);
    if (noSpaceErrCount >= DEFAULT_MAX_NOSPACE) {
        ERRLOG("Threshold reached for DEFAULT_MAX_NOSPACE");
        m_failReason = BackupPhaseStatus::FAILED_NOSPACE;
        m_failed = true;
        return FAILED;
    }
    uint32_t noAccessErrCount = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::NO_ACCESS_ERR);
    if (noAccessErrCount >= DEFAULT_MAX_NOACCESS) {
        ERRLOG("Threshold reached for DEFAULT_MAX_NOACCESS");
        m_failReason = BackupPhaseStatus::FAILED_NOACCESS;
        m_failed = true;
        return FAILED;
    }

    /* Nas Server Check for Destination side */
    uint32_t retryableErrCount = m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::RETRIABLE_ERR);
    if (retryableErrCount >= m_dstAdvParams->serverCheckMaxCount) {
        ERRLOG("Threshold reached calling dst servercheck");
        m_suspend = true;
        int ret = HandleConnectionException(m_asyncContext, m_params.dstSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
        if (ret != SUCCESS) {
            ERRLOG("Stop and Abort read phase due to server inaccessible");
            m_failed = true;
            m_suspend = false;
            FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
            return FAILED;
        } else {
            INFOLOG("Server reachable");
            m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
        }
        m_suspend = false;
    }

    return SUCCESS;
}

int64_t LibsmbCopyWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int expiredCount =
        m_dstAdvParams->maxPendingAsyncReqCnt - m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING);
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles, expiredCount);
    if (delay > POLL_MAX_TIMEOUT) {
        delay = POLL_MAX_TIMEOUT;
    }
    for (FileHandle& fh : fileHandles) {
        if (IsWriterRequestReachThreshold()) {
            m_timer.Insert(fh, PENDING_PACKET_REACH_THRESHOLD_TIMER_MILLISECOND);
            continue;
        }
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        ProcessFileDescState(fh);
    }
    return delay;
}

/* check if we can take ThreadFunc into base class */
void LibsmbCopyWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbCopyWriter main thread start!");
    int ret = 0;
    while (true) {
        if (ServerCheck() != SUCCESS) {
            break;
        }
        if (IsComplete()) {
            INFOLOG("Write phase is complete");
            break;
        }
        if (IsAbort()) {
            WARNLOG("Main thread abort");
            break;
        }

        if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_dstAdvParams->maxPendingAsyncReqCnt) {
            DBGLOG("SmbCopyWriter PENDING packet(%d) reach threshold.",
                m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
            ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
            if (ret < 0 && ProcessConnectionException() != SUCCESS) {
                break;
            }
            continue;
        }

        ProcessWriteEntries();

        ProcessTimers();

        ret = m_asyncContext->Poll(DEFAULT_POLL_EXPIRED_TIME);
        // ret < 0说明连接有问题，需要重连, 如果ProcessConnectionException也返回失败，说明重连失败
        if (ret < 0 && ProcessConnectionException() != SUCCESS) {
            break;
        }

        ClearWriteCache();
    }
    SmbDisconnectContexts();
    HandleComplete();
    INFOLOG("LibsmbCopyWriter main thread end!");
    return;
}

int LibsmbCopyWriter::ProcessConnectionException()
{
    ERRLOG("dst connection exception");
    int retVal = HandleConnectionException(m_asyncContext, m_params.dstSmbContextArgs, RECONNECT_CONTEXT_RETRY_TIMES);
    if (retVal != SUCCESS) {
        ERRLOG("Stop and Abort read phase due to server inaccessible");
        m_failed = true;
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        return FAILED;
    }
    INFOLOG("Server reachable");
    m_pktStats->ResetErrorCounter(PKT_TYPE::TOTAL);
    return SUCCESS;
}

void LibsmbCopyWriter::ProcessWriteEntries()
{
    while (!m_writeQueue->Empty()) {
        if (IsAbort() || IsWriterRequestReachThreshold()) {
            break;
        }
        FileHandle fileHandle;
        if (!m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
            break;
        }
        ProcessFileDescState(fileHandle);
    }
}

bool LibsmbCopyWriter::IsWriterRequestReachThreshold() const
{
    if (m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING) > m_dstAdvParams->maxPendingAsyncReqCnt) {
        DBGLOG("SmbCopyWriter PENDING packet(%d) reach threshold.",
            m_pktStats->GetValue(PKT_TYPE::TOTAL, PKT_COUNTER::PENDING));
        return true;
    }
    return false;
}

void LibsmbCopyWriter::ProcessFileDescState(FileHandle fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();
    DBGLOG("ProcessFileDescState name: %s, %d, %d, %d, originalFileCount: %llu", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->IsFlagSet(IS_DIR), fileHandle.m_file->m_mode, state, fileHandle.m_file->m_originalFileCount);

    if (state == FileDescState::LINK_DEL) {
        DeleteFile(fileHandle);
    }

    if (state == FileDescState::DST_CLOSED) {
        if (m_params.writeMeta) {
            WriteMeta(fileHandle);
        } else if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
            m_controlInfo->m_noOfBytesCopied += fileHandle.m_file->m_size;
        }
    }

    // FileDescState is INIT
    ProcessFileDescStateINIT(fileHandle);

    if (IsFileReadOrWriteFailed(fileHandle)) {
        // 读或写失败: 对于备份，删除这个文件；对于恢复，则不删除用户的文件
        ++fileHandle.m_file->m_blockStats.m_writeReqCnt;
        ++fileHandle.m_file->m_blockStats.m_writeRespCnt;
        if (fileHandle.m_file->m_blockStats.m_writeRespCnt == fileHandle.m_file->m_blockStats.m_totalCnt) {
            if (IsBackupTask(m_params.backupType)) {
                DeleteFile(fileHandle);
            } else {
                CloseFile(fileHandle);
            }
        }
    }
    if (state == FileDescState::DST_OPENED || state == FileDescState::PARTIAL_WRITED) {
        WriteData(fileHandle);
    } else if (state == FileDescState::WRITED) {
        CloseFile(fileHandle);
    } else if (state == FileDescState::FILEHANDLE_INVALID) {
        if (fileHandle.m_block.m_seq == 0) {
            OpenFile(fileHandle);
        } else {
            m_timer.Insert(fileHandle, ONE_THOUSAND_UNIT_CONVERSION);
        }
    }
    return;
}

void LibsmbCopyWriter::ProcessFileDescStateINIT(FileHandle &fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();
    if (state == FileDescState::INIT) {
        if ((fileHandle.m_file->m_mode == FILE_IS_ADS_FILE || fileHandle.m_file->m_mode == FILE_HAVE_ADS) &&
            m_isBlockAdsOpen) {
            m_timer.Insert(fileHandle, ADS_FILE_BLOCK_MILLISECOND);
            return;
        }
        if (fileHandle.m_file->GetSrcState() == FileDescState::READ_FAILED) {
            // 读失败，且writer还没open这个文件，那么就跳过这个文件
            fileHandle.m_file->SetDstState(FileDescState::END);
            DBGLOG("ProcessFileDescState read failed, set write state to END, name: %s",
                fileHandle.m_file->m_fileName.c_str());
            return;
        }
        if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
            DBGLOG("ProcessFileDescState put dir: %s to dirQueue", fileHandle.m_file->m_fileName.c_str());
            m_dirQueue->Push(fileHandle);
        } else if (IsOpenBlock(fileHandle)) {
            OpenFile(fileHandle);
        } else {
            m_writeCache[fileHandle.m_file->m_fileName].push_back(fileHandle);
        }
    }
    return;
}

bool LibsmbCopyWriter::IsOpenBlock(const FileHandle& fileHandle)
{
    return ((fileHandle.m_block.m_size == 0) && (fileHandle.m_block.m_seq == 0));
}

void LibsmbCopyWriter::SyncThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibsmbCopyWriter mkdir thread start!");
    while (true) {
        if (IsMkdirComplete()) {
            INFOLOG("SyncThreadFunc thread is complete");
            break;
        }
        if (IsAbort()) {
            WARNLOG("Sync thread abort");
            break;
        }
        FileHandle fileHandle;
        bool ret = m_dirQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS);
        if (ret) {
            auto cbData = GetSmbWriterCommonData(fileHandle);
            if (cbData == nullptr) {
                continue;
            }
            if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
                cbData->path = fileHandle.m_file->m_dirName;
            } else {
                cbData->path = fileHandle.m_file->m_fileName;
            }
            DBGLOG("SyncThreadFunc get file desc from write queue :%s, %s",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str());
            FileDescState state = fileHandle.m_file->GetDstState();
            if (state == FileDescState::DIR_DEL) {
                SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::DELETE);
            } else if (state == FileDescState::REPLACE_DIR) {
                SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::REPLACE_DIR);
            } else {
                SendWriterSyncRequest(fileHandle, cbData, LibsmbEvent::MKDIR);
            }
        }
    }
    SmbDisconnectSyncContexts();
    INFOLOG("LibsmbCopyWriter SyncThreadFunc thread end!");
    return;
}

void LibsmbCopyWriter::FillSmbWriterCommonData(SmbWriterCommonData *writerCommonData)
{
    writerCommonData->writeSmbContext = m_asyncContext;
    writerCommonData->mkdirSmbContext = m_syncContext;
    writerCommonData->writeQueue = m_writeQueue;
    writerCommonData->dirQueue = m_dirQueue;
    writerCommonData->blockBufferMap = m_blockBufferMap;
    writerCommonData->params = m_params;

    writerCommonData->timer = &m_timer;
    writerCommonData->controlInfo = m_controlInfo;
    writerCommonData->pktStats = m_pktStats;
    writerCommonData->isBlockAdsOpen = &m_isBlockAdsOpen;
    writerCommonData->failureRecorder = m_failureRecorder;
}

SmbWriterCommonData* LibsmbCopyWriter::GetSmbWriterCommonData(FileHandle &fileHandle)
{
    auto cbData = new(nothrow) SmbWriterCommonData();
    if (cbData == nullptr) {
        ERRLOG("Failed to allocate Memory for cbData");
        return nullptr;
    }
    FillSmbWriterCommonData(cbData);
    cbData->fileHandle = fileHandle;
    return cbData;
}

void LibsmbCopyWriter::ClearWriteCache()
{
    if (m_writeCache.empty()) {
        return;
    }

    for (auto itr = m_writeCache.begin(); itr != m_writeCache.end();) {
        FileDescState state = itr->second[0].m_file->GetDstState();
        if (!itr->second.empty() && (state == FileDescState::DST_OPENED)) {
            ClearWriteCacheStateOpened(itr->second);
            itr = m_writeCache.erase(itr);
        } else if (IsFileReadOrWriteFailed(itr->second[0]) ||
            state == FileDescState::WRITE_SKIP || state == FileDescState::WRITED) {
            // ignore或者replace_older，存在不需要写的文件，需要把这些文件清掉
            DBGLOG("erase from cache %s", itr->second[0].m_file->m_fileName.c_str());
            itr = m_writeCache.erase(itr);
        } else {
            itr++;
        }
    }
}

void LibsmbCopyWriter::ClearWriteCacheStateOpened(std::vector<FileHandle> &fileHandles)
{
    for (auto& fileHandle : fileHandles) {
        DBGLOG("push from cache %s", fileHandle.m_file->m_fileName.c_str());
        if (fileHandle.m_file->m_size <= MAX_SMALL_FILE_SIZE) {
            WriteData(fileHandle);
        } else {
            m_writeQueue->Push(fileHandle);
        }
    }
}
