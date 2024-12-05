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
#include "Win32CopyWriter.h"
#include "Win32ServiceTask.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "Win32BackupEngineUtils.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Win32BackupEngineUtils;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int RETRY_TIME_MILLISENCOND = 1000;
}

Win32CopyWriter::Win32CopyWriter(
    const WriterParams &copyWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostCopyWriter(copyWriterParams, failureRecorder)
{
    INFOLOG("Construct Win32CopyWriter!");
}

Win32CopyWriter::~Win32CopyWriter()
{
    INFOLOG("Destructing Win32CopyWriter!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    CloseOpenedHandle();
}

int Win32CopyWriter::WriteMeta(FileHandle& fileHandle)
{
    Win32TaskExtendContext extendContext {};
    extendContext.controlInfo = m_controlInfo;
    auto task = make_shared<Win32ServiceTask>(
        HostEvent::WRITE_META, m_blockBufferMap, fileHandle, m_params, extendContext);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write meta file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}

void Win32CopyWriter::ProcessWriteEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("process write entry %s state %d blockInfo %llu %llu %u, scannermode %s",
        fileHandle.m_file->m_fileName.c_str(), (int)state,
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size,
        fileHandle.m_file->m_scannermode.c_str());

    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        CreateDir(fileHandle);
        return;
    }

    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        FSBackupUtils::IsHandleMetaModified(fileHandle.m_file->m_scannermode,
        m_backupParams.commonParams.backupDataFormat)) {
        WriteMeta(fileHandle);
        return;
    }

    // 是小文件直接写数据, 提前退出, 第二个判断是只处理一次
    if (fileHandle.m_file->m_size <= m_params.blockSize
        && state == FileDescState::INIT &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle)) {
        // 对于小文件， 意义为open 的block直接丢弃
        if (IsOpenBlock(fileHandle)) {
            return;
        }
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        WriteData(fileHandle);
        return;
    }

    if (state == FileDescState::INIT) {
        if (IsOpenBlock(fileHandle)) {
            OpenFile(fileHandle);
        } else {
            InsertWriteCache(fileHandle);
        }
    }

    if ((state == FileDescState::DST_OPENED) || (state == FileDescState::PARTIAL_WRITED)) {
        WriteData(fileHandle);
    }
    if (state == FileDescState::WRITED) {
        CloseFile(fileHandle);
    }
    if (state == FileDescState::DST_CLOSED) {
        WriteMeta(fileHandle);
    }
    bool deleteFlag =
        (state == FileDescState::WRITE_FAILED) || (state == FileDescState::WRITE_SKIP) || (state == FileDescState::END);
    if (deleteFlag) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    }

    return;
}

void Win32CopyWriter::ProcessWriteData(FileHandle& fileHandle)
{
    bool processAsSparse = ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle);
    fileHandle.m_file->SetDstState(FileDescState::PARTIAL_WRITED);
    ++fileHandle.m_file->m_blockStats.m_writeReqCnt;
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if (fileHandle.IsAdsFile()) {
        m_controlInfo->m_noOfSubStreamBytesCopied += fileHandle.m_block.m_size;
    } else {
        m_controlInfo->m_noOfBytesCopied += fileHandle.m_block.m_size;
    }
    if ((fileHandle.m_file->m_size <= m_params.blockSize && !processAsSparse) ||
        (fileHandle.m_file->m_blockStats.m_writeReqCnt == fileHandle.m_file->m_blockStats.m_totalCnt) ||
        (fileHandle.m_file->m_size == 0)) {
        DBGLOG("All blocks writed for %s", fileHandle.m_file->m_fileName.c_str());
        if (!m_params.writeMeta && (fileHandle.m_file->m_size <= m_params.blockSize && !processAsSparse)) {
            fileHandle.m_file->SetDstState(FileDescState::END);
            DBGLOG("Finish backup file %s originalFileCount %lu total backup file %lu %lu",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_originalFileCount,
                m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfSubStreamCopied.load());
            PostFileSucceedCopiedOperation(fileHandle);
            DBGLOG("Finish backup file %s originalFileCount %lu total backup file %lu %lu",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_originalFileCount,
                m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfSubStreamCopied.load());
        } else {
            fileHandle.m_file->SetDstState(FileDescState::WRITED);
            m_writeQueue->Push(fileHandle);
        }
    }
    return;
}

bool Win32CopyWriter::IsComplete()
{
    /* in write phase, faild item should contain not only write failed but also read failed */
    /* thus, use BackupControlInfo::m_noOfFilesFailed and BackupControlInfo::m_noOfDirFailed */
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyWriter check is complete: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(noOfSubStreamCopied %llu noOfSubStreamFound %llu) "
            "(writedFiles %llu writedDir %llu aggrFiles %llu "
            "skipFiles %llu skipDir %llu backupFailedFiles %llu backupFailedDir %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu totalDirs %llu archiveFiles %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfSubStreamCopied.load(), m_controlInfo->m_noOfSubStreamFound.load(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_aggregatedFiles.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(), m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfDirFailed.load(), m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_archiveFiles.load());
    }

    if (m_controlInfo->m_aggregatePhaseComplete &&
        m_writeQueue->Empty() &&
        (m_writeCache.size() == 0) &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_writeTaskProduce == m_controlInfo->m_writeTaskConsume) &&
        (m_controlInfo->m_noOfSubStreamCopied == m_controlInfo->m_noOfSubStreamFound) &&
        (m_controlInfo->m_noOfFilesCopied + m_controlInfo->m_noOfFilesFailed + m_controlInfo->m_skipFileCnt +
        m_controlInfo->m_noOfFilesWriteSkip == m_controlInfo->m_noOfFilesToBackup)) {
        INFOLOG("CopyWriter complete: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(noOfSubStreamCopied %llu noOfSubStreamFound %llu) "
            "(writedFiles %llu writedDir %llu aggrFiles %llu "
            "skipFiles %llu skipDir %llu backupFailedFiles %llu backupFailedDir %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu totalDirs %llu archiveFiles %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfSubStreamCopied.load(), m_controlInfo->m_noOfSubStreamFound.load(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_aggregatedFiles.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(), m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfDirFailed.load(), m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_archiveFiles.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    return false;
}

void Win32CopyWriter::PostFileSucceedCopiedOperation(const FileHandle& fileHandle)
{
    DBGLOG("PostFileSucceedCopiedOperation, fileName: %s, mode: %u IsAdsFile : %u, HasAds: %u",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
        fileHandle.IsAdsFile(), fileHandle.HasAdsFile());
    if (fileHandle.IsAdsFile()) {
        m_controlInfo->m_noOfSubStreamCopied += fileHandle.m_file->m_originalFileCount;
        m_controlInfo->m_streamHostFilePendingMap->DecStreamPending(fileHandle.m_file->m_fileName);
    } else {
        // dir with ADS won't enter this branch
        if (fileHandle.HasAdsFile()) {
            m_controlInfo->m_streamHostFilePendingMap->MarkHostWriteComplete(fileHandle.m_file->m_fileName);
        }
        m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
    }
    return;
}

void Win32CopyWriter::PostFileFailCopiedOperation(const FileHandle& fileHandle)
{
    DBGLOG("PostFileFailCopiedOperation, fileName: %s, mode: %u IsAdsFile : %u, HasAds: %u",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
        fileHandle.IsAdsFile(), fileHandle.HasAdsFile());
    // dir with ADS won't enter this branch
    if (fileHandle.HasAdsFile()) {
        m_controlInfo->m_streamHostFilePendingMap->MarkHostWriteFailed(fileHandle.m_file->m_fileName);
    }
    if (fileHandle.IsAdsFile()) {
        m_controlInfo->m_noOfSubStreamCopied += fileHandle.m_file->m_originalFileCount;
        m_controlInfo->m_streamHostFilePendingMap->DecStreamPending(fileHandle.m_file->m_fileName);
    }
    return;
}

void Win32CopyWriter::HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("Win32CopyWriter success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), (int)state);

    if ((event == HostEvent::OPEN_DST) && (state != FileDescState::WRITE_SKIP)) {
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        return;
    }

    if (event == HostEvent::WRITE_META || state == FileDescState::WRITE_SKIP) {
        fileHandle.m_file->SetDstState(FileDescState::END);
        PostFileSucceedCopiedOperation(fileHandle);
        if (state == FileDescState::WRITE_SKIP) {
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        }
        DBGLOG("Finish backup file %s total backup file %lu %lu", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfSubStreamCopied.load());
        return;
    }

    if (event == HostEvent::CLOSE_DST) {
        if (state == FileDescState::WRITE_FAILED) {
            return;
        }
        if (m_params.writeMeta) {
            fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
            m_writeQueue->Push(fileHandle);
        } else {
            fileHandle.m_file->SetDstState(FileDescState::END);
            PostFileSucceedCopiedOperation(fileHandle);
            DBGLOG("Finish backup file %s originalFileCount %lu total backup file %lu %lu",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_originalFileCount,
                m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfSubStreamCopied.load());
        }
        return;
    }

    if (event == HostEvent::WRITE_DATA) {
        if (state == FileDescState::WRITE_FAILED) {
            /* Other task had writed failed, so ignoring this task */
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
            CloseWriteFailedHandle(fileHandle);
            return;
        }
        ProcessWriteData(fileHandle);
    }

    return;
}

void Win32CopyWriter::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    ++fileHandle.m_retryCnt;

    ERRLOG("Host copy writer failed %s event %d retry cnt %d seq %d",
        fileHandle.m_file->m_fileName.c_str(),
        static_cast<int>(event), fileHandle.m_retryCnt, fileHandle.m_block.m_seq);
    FileDescState state = fileHandle.m_file->GetDstState();
    if (state != FileDescState::WRITE_FAILED &&  /* If state is WRITE_FAILED, needn't retry */
        fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT && !taskPtr->IsCriticalError()) {
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return;
    }
    if (state != FileDescState::WRITE_FAILED) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
        PostFileFailCopiedOperation(fileHandle);
        // failed dirs are collected in the dir phase.
        if (!fileHandle.m_file->IsFlagSet(IS_DIR) &&
            fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED && !fileHandle.IsAdsFile()) {
            // 若read的状态为READ_FAILED时，说明该文件已经被reader记为失败
            m_controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
            fileHandle.m_errNum = taskPtr->m_errDetails.second;
            m_failedList.emplace_back(fileHandle);
        }
        fileHandle.m_file->UnlockCommonMutex();
    }
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    CloseWriteFailedHandle(fileHandle);
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    ERRLOG("copy write failed for file %s, totalFailed: %llu %llu",
        fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesFailed.load(), m_controlInfo->m_noOfDirFailed.load());
    return;
}

void Win32CopyWriter::CloseOpenedHandle()
{
    for (auto it = m_dstOpenedHandleSet.begin(); it != m_dstOpenedHandleSet.end(); it++) {
        DBGLOG("Close handle(%s)", (*it)->m_fileName.c_str());
        if ((*it)->dstIOHandle.win32Fd != nullptr && (*it)->dstIOHandle.win32Fd != INVALID_HANDLE_VALUE) {
            CloseHandle((*it)->dstIOHandle.win32Fd);
            (*it)->dstIOHandle.win32Fd = nullptr;
        }
    }
}