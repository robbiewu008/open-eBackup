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
#include "Win32CopyReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "Win32BackupEngineUtils.h"
#include "HostCopyReader.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Win32BackupEngineUtils;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
}

Win32CopyReader::Win32CopyReader(
    const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostCopyReader(copyReaderParams, failureRecorder)
{
    INFOLOG("Construct Win32CopyReader!");
}

Win32CopyReader::~Win32CopyReader()
{
    INFOLOG("Destruct Win32CopyReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    CloseOpenedHandle();
}

int Win32CopyReader::ReadEmptyData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadEmptyData: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    fileHandle.IsAdsFile() ? ++m_controlInfo->m_noOfSubStreamRead : ++m_controlInfo->m_noOfFilesRead;
    PushToAggregator(fileHandle);
    DBGLOG("%s is empty file, needn't read it! readed for now: %d %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfSubStreamRead.load());
    return SUCCESS;
}

/* read ADS file */
int Win32CopyReader::ReadMeta(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadMeta: %s", fileHandle.m_file->m_fileName.c_str());
    if (m_backupParams.commonParams.adsProcessType != AdsProcessType::ADS_PROCESS_TYPE_FROM_BACKUP) {
        if (fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED) {
            m_aggregateQueue->Push(fileHandle);
            ++m_controlInfo->m_readProduce;
            return SUCCESS;
        }
        fileHandle.m_file->SetSrcState(FileDescState::META_READED);
        if (fileHandle.IsDir()) {
            return SUCCESS;
        }
        m_readQueue->Push(fileHandle);
        return SUCCESS;
    }
    Win32TaskExtendContext extendContext {};
    extendContext.readQueuePtr = m_readQueue;
    extendContext.aggregateQueuePtr = m_aggregateQueue;
    extendContext.controlInfo = m_controlInfo;
    auto task = make_shared<Win32ServiceTask>(
        HostEvent::READ_META, m_blockBufferMap, fileHandle, m_params, extendContext);
    if ((m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false)) {
        ERRLOG("put read meta task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

bool Win32CopyReader::HandleAdsFile(FileHandle& fileHandle)
{
    if (!fileHandle.IsAdsFile()) {
        return true;
    }
    // is ADS file
    if (m_controlInfo->m_streamHostFilePendingMap->IsHostFailed(fileHandle.m_file->m_fileName)) {
        ERRLOG("ADS file %s set to failed due to host failed", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfSubStreamCopied;
        ++m_controlInfo->m_noOfSubStreamRead;
        return false;
    }
    if (!m_controlInfo->m_streamHostFilePendingMap->IsHostWriteComplete(fileHandle.m_file->m_fileName)) {
        DBGLOG("ADS file %s entered m_timer due to host not completed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, RETRY_TIME_MILLISENCOND);
        return false;
    }
    return true;
}

void Win32CopyReader::ProcessReadEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("process read entry %s size %llu state %d blockInfo %llu %llu %u %u scannermode %s",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size,
        (int)state, fileHandle.m_block.m_seq, fileHandle.m_block.m_offset,
        fileHandle.m_block.m_size, fileHandle.m_file->m_mode, fileHandle.m_file->m_scannermode.c_str());

    if (!HandleAdsFile(fileHandle)) {
        return;
    }

    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        PushToAggregator(fileHandle);
        ReadMeta(fileHandle); /* directory may have ADS, need to check in ReadMeta */
        return;
    }

    if (ProcessReadEntriesScannerMode(fileHandle)) {
        return;
    }

    /* fileHandle not directory, may contain ADS, check ADS meta first */
    if (state == FileDescState::INIT || state == FileDescState::AGGREGATED) {
        ReadMeta(fileHandle);
        return;
    }

    /* fileHandle ADS must be meta readed, select small file optimization branch or common open file branch  */
    /* 1. symlink/small file optimization branch */
    if (fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_params.writeSparseFile, fileHandle) && state != FileDescState::AGGREGATED) {
        DBGLOG("read small file %s size %llu", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);
        fileHandle.m_block.m_size = fileHandle.m_file->m_size;
        fileHandle.m_block.m_offset = 0;
        fileHandle.m_block.m_seq = 1;
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
        ReadData(fileHandle);
        return;
    }
    
    /* 2. common file open branch */
    if (state == FileDescState::META_READED) {
        OpenFile(fileHandle);
        return;
    }

    if (state == FileDescState::SRC_OPENED && !WriteFailedAndSkipRead(fileHandle)) {
        // Check whether write failed, need'nt to read data when write failed
        if (!WriteFailedAndSkipRead(fileHandle)) {
            ReadData(fileHandle);
        }
        return;
    }
    if (state == FileDescState::READED) {
        CloseFile(fileHandle);
        return;
    }
    return;
}

void Win32CopyReader::HandleSuccessEvent(shared_ptr<Win32ServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("Win32 copy reader success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), static_cast<int>(state));
    // 小文件readdata后已经close了， 直接push给aggregator
    // prevent these case from increasing m_noOfFilesRead : 1. is dir 2. empty file (not symlink) 3. READ_META event
    if (fileHandle.m_file->IsFlagSet(IS_DIR) ||
        // ReadEmptyData won't create task but symlink data (0 size) will create READ_DATA
        (fileHandle.m_file->m_size == 0 && !FSBackupUtils::IsSymLinkFile(fileHandle)) ||
        event == HostEvent::READ_META) {
        return;
    }
    
    if (fileHandle.m_file->m_size <= m_params.blockSize &&
        !ProcessFileHandleAsSparseFile(m_backupParams.commonParams.writeSparseFile, fileHandle)) {
        /* sparse small file may have hole less than block size */
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        fileHandle.IsAdsFile() ? ++m_controlInfo->m_noOfSubStreamRead : ++m_controlInfo->m_noOfFilesRead;
        PushToAggregator(fileHandle);
        DBGLOG("Readed small files : %s, readed for now: %d %d", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfSubStreamRead.load());
        return;
    }

    if (event == HostEvent::CLOSE_SRC) {
        fileHandle.IsAdsFile() ? ++m_controlInfo->m_noOfSubStreamRead : ++m_controlInfo->m_noOfFilesRead;
        DBGLOG("file: %s, Readed Files for now : %d %d", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfSubStreamRead.load());
        return;
    }

    if (event == HostEvent::OPEN_SRC) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
        PushToAggregator(fileHandle); // push to aggregate to write to open dst
        PushToReader(fileHandle); // decompose to blocks and push to aggregate
        return;
    }

    if (event == HostEvent::READ_DATA) {
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readReqCnt ||
            fileHandle.m_file->m_size == 0) {
            fileHandle.m_file->SetSrcState(FileDescState::READED);
            PushToReader(fileHandle); // push to aggregate
        }
        PushToAggregator(fileHandle); // file handle with data block, push to aggregate to write to write data
        return;
    }
    return;
}

void Win32CopyReader::PostFileFailCopiedOperation(const FileHandle& fileHandle)
{
    DBGLOG("Reader PostFileFailCopiedOperation, fileName: %s, mode: %u IsAdsFile : %d, HasAds: %d",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
        fileHandle.IsAdsFile(), fileHandle.HasAdsFile());
    // dir with ADS won't enter this branch
    if (fileHandle.HasAdsFile()) {
        m_controlInfo->m_streamHostFilePendingMap->MarkHostFailed(fileHandle.m_file->m_fileName);
    }
    if (fileHandle.IsAdsFile()) {
        ++m_controlInfo->m_noOfSubStreamCopied;
        ++m_controlInfo->m_noOfSubStreamRead;
        m_controlInfo->m_streamHostFilePendingMap->DecStreamPending(fileHandle.m_file->m_fileName);
    }
}

void Win32CopyReader::HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    ++fileHandle.m_retryCnt;
    DBGLOG("%s copy reader failed %s, %u, event %d retry cnt %d",
        OS_PLATFORM_NAME.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size,
        static_cast<int>(event), fileHandle.m_retryCnt);
    FileDescState state = fileHandle.m_file->GetSrcState();

    if (FSBackupUtils::IsStuck(m_controlInfo)) {
        ERRLOG("set backup to failed due to stucked!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        return;
    }

    if (m_params.discardReadError && (taskPtr->m_errDetails.second == ERROR_FILE_NOT_FOUND ||
        taskPtr->m_errDetails.second == ERROR_PATH_NOT_FOUND)) {
        // 如果设置了忽略读失败，对于文件不存在的情况，认为该文件备份成功，设置为跳过文件
        WARNLOG("File %s not exist, discardReadError is true, ignore it", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesWriteSkip;
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        return;
    }
    if (state != FileDescState::READ_FAILED &&  /* If state is READ_FAILED, needn't retry */
        fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT && !taskPtr->IsCriticalError()) {
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return;
    }
    if (state != FileDescState::READ_FAILED) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
        PostFileFailCopiedOperation(fileHandle);  // 处理ADS流文件或者ADS的宿主文件
        if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            // skipping to the failed cnt inc for zip files in restore which are not used in writetask
            if (!fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE) &&
                fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
                // 若write的状态为WRITE_FAILED时，说明该文件已经被writer记为失败
                ++m_controlInfo->m_noOfFilesFailed;
                fileHandle.m_errNum = taskPtr->m_errDetails.second;
                m_failedList.emplace_back(fileHandle);
            }
        } else {
            ++m_controlInfo->m_noOfDirFailed;
        }
        fileHandle.m_file->UnlockCommonMutex();
        ++m_controlInfo->m_noOfFilesReadFailed;
    }
    PostOfHandleFailedEvent(fileHandle, taskPtr);
}

void Win32CopyReader::PostOfHandleFailedEvent(FileHandle& fileHandle, std::shared_ptr<OsPlatformServiceTask> taskPtr)
{
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    // NATIVE format doesn't need push to aggregator.
    if (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) {
        PushToAggregator(fileHandle); // file handle so aggregate can handle failurs of reading file
    }
    ERRLOG("copy read failed for file %s, %llu, metaInfo: %u, %llu, totalFailed: %llu, %llu",
        fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesReadFailed.load(),
        fileHandle.m_file->m_metaFileIndex, fileHandle.m_file->m_metaFileOffset,
        m_controlInfo->m_noOfDirFailed.load(), m_controlInfo->m_noOfFilesFailed.load());
}

bool Win32CopyReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyReader check is complete: controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskproduce %llu consume %llu) "
            "(noOfSubStreamRead %llu noOfSubStreamFound %llu) "
            "(noOfFilesRead %llu noOfDirRead %llu noOfFilesReadFailed %llu skipFileCnt %llu "
            "skipDirCnt %llu unaggregatedFiles %llu emptyFiles %llu unaggregatedFailedFiles %llu) "
            "(totalFiles %llu totalDir %llu unarchiveFiles %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfSubStreamRead.load(), m_controlInfo->m_noOfSubStreamFound.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(), m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_noOfDirToBackup.load(), m_controlInfo->m_unarchiveFiles.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_readTaskProduce == m_controlInfo->m_readTaskConsume) &&
        (m_controlInfo->m_noOfSubStreamRead == m_controlInfo->m_noOfSubStreamFound) &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt + m_controlInfo->m_noOfFilesWriteSkip +
        m_controlInfo->m_unaggregatedFiles + m_controlInfo->m_emptyFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_controlInfo->m_unarchiveFiles))) {
        INFOLOG("CopyReader complete: controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskproduce %llu consume %llu) "
            "(noOfSubStreamRead %llu noOfSubStreamFound %llu) "
            "(noOfFilesRead %llu noOfDirRead %llu noOfFilesReadFailed %llu skipFileCnt %llu "
            "skipDirCnt %llu unaggregatedFiles %llu emptyFiles %llu unaggregatedFailedFiles %llu) "
            "(totalFiles %llu totalDir %llu unarchiveFiles %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfSubStreamRead.load(), m_controlInfo->m_noOfSubStreamFound.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(), m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_noOfDirToBackup.load(), m_controlInfo->m_unarchiveFiles.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

void Win32CopyReader::CloseOpenedHandle()
{
    for (auto it = m_srcOpenedHandleSet.begin(); it != m_srcOpenedHandleSet.end(); it++) {
        DBGLOG("Close handle(%s)", (*it)->m_fileName.c_str());
        if ((*it)->srcIOHandle.win32Fd != nullptr && (*it)->srcIOHandle.win32Fd != INVALID_HANDLE_VALUE) {
            CloseHandle((*it)->srcIOHandle.win32Fd);
            (*it)->srcIOHandle.win32Fd = nullptr;
        }
    }
}