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
#include "PosixCopyReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "PosixUtils.h"
#include "HostCopyReader.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
}

PosixCopyReader::PosixCopyReader(
    const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : HostCopyReader(copyReaderParams, failureRecorder)
{
    INFOLOG("Construct PosixCopyReader!");
}

PosixCopyReader::~PosixCopyReader()
{
    INFOLOG("Destruct PosixCopyReader!");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    CloseOpenedHandle();
}

int PosixCopyReader::ReadEmptyData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s empty file", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
    DBGLOG("total blocks: %d, file size: %llu",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);
    ++m_controlInfo->m_noOfFilesRead;
    PushToAggregator(fileHandle);
    DBGLOG("%s is empty file, needn't read it! readed for now: %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesRead.load());
    return SUCCESS;
}

int PosixCopyReader::ReadMeta(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadMeta: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle = fileHandle;
    return SUCCESS;
}

bool PosixCopyReader::isHugeObjectFile(FileHandle& fileHandle)
{
    if ((fileHandle.m_file->m_type == FileType::OBJECT) && (m_params.maxBlockNum != 0) &&
        (fileHandle.m_file->m_size > (uint64_t)m_params.blockSize * m_params.maxBlockNum)) {
        DBGLOG("Set flag HUGE_OBJECT_FILE, file name: %s, file size: %llu, blockSize: %u, maxBlockNum: %u",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size, m_params.blockSize, m_params.maxBlockNum);
        fileHandle.m_file->SetFlag(HUGE_OBJECT_FILE);
        return true;
    }

    return false;
}

void PosixCopyReader::ProcessReadEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("process read entry %s size %llu state %d blockInfo %llu %llu %u scannermode %s",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size,
        (int)state, fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size,
        fileHandle.m_file->m_scannermode.c_str());

    if (ProcessReadEntriesScannerMode(fileHandle)) {
        return;
    }

    if (isHugeObjectFile(fileHandle)) {
        ReadHugeObjectData(fileHandle);
        return;
    }

    // 是小文件直接读数据, 提前结束
    if (fileHandle.m_file->m_size <= m_params.blockSize &&
        state != FileDescState::AGGREGATED) {
        DBGLOG("read small file %s size %llu", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);
        fileHandle.m_block.m_size = fileHandle.m_file->m_size;
        fileHandle.m_block.m_offset = 0;
        fileHandle.m_block.m_seq = 1;
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
        ReadData(fileHandle);
        return;
    }

    if (state == FileDescState::INIT) {
        OpenFile(fileHandle);
    } else if (state == FileDescState::SRC_OPENED) {
        // Check whether write failed, need'nt to read data when write failed
        if (!WriteFailedAndSkipRead(fileHandle)) {
            ReadData(fileHandle);
        }
    } else if ((state == FileDescState::READED) || (state == FileDescState::META_READED)) {
        CloseFile(fileHandle);
    }

    return;
}

void PosixCopyReader::HandleSuccessEvent(shared_ptr<PosixServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("Posix copy reader success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), static_cast<int>(state));
    // 小文件readdata后已经close了, 直接push给aggregator
    if ((fileHandle.m_file->m_size <= m_params.blockSize) || isHugeObjectFile(fileHandle)) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        ++m_controlInfo->m_noOfFilesRead;
        PushToAggregator(fileHandle);
        DBGLOG("Readed files : %s, readed for now: %d", fileHandle.m_file->m_fileName.c_str(),
            m_controlInfo->m_noOfFilesRead.load());
        return;
    }

    if (event == HostEvent::CLOSE_SRC) {
        ++m_controlInfo->m_noOfFilesRead;
        DBGLOG("Readed Files for now : %d", m_controlInfo->m_noOfFilesRead.load());
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

void PosixCopyReader::HandleFailedEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    ++fileHandle.m_retryCnt;
    WARNLOG("event %d retry cnt %d, %s copy reader failed %s, %u", static_cast<int>(event), fileHandle.m_retryCnt,
        OS_PLATFORM_NAME.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_block.m_size);
    FileDescState state = fileHandle.m_file->GetSrcState();

    if (FSBackupUtils::IsStuck(m_controlInfo)) {
        ERRLOG("set backup to failed due to stucked!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        return;
    }

    if (m_params.discardReadError && taskPtr->m_errDetails.second == ENOENT) {
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
    HandleFailedEventInner(state, fileHandle, taskPtr);
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
    ERRLOG("copy read failed for file %s, %llu, totalFailed: %llu, %llu", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_noOfDirFailed.load(),
        m_controlInfo->m_noOfFilesFailed.load());
    return;
}

void PosixCopyReader::HandleFailedEventInner(FileDescState state, FileHandle fileHandle,
    shared_ptr<OsPlatformServiceTask> taskPtr)
{
    if (state != FileDescState::READ_FAILED) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
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
}

void PosixCopyReader::CloseOpenedHandle()
{
    for (auto it = m_srcOpenedHandleSet.begin(); it != m_srcOpenedHandleSet.end(); it++) {
        DBGLOG("Close handle(%s)", (*it)->m_fileName.c_str());
        if ((*it)->srcIOHandle.posixFd != -1) {
            close((*it)->srcIOHandle.posixFd);
            (*it)->srcIOHandle.posixFd = -1;
        }
    }
}