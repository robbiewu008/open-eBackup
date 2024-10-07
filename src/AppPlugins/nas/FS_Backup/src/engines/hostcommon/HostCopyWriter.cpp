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
#include "HostCopyWriter.h"
#include "HostServiceTask.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "OsPlatformDefines.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
}

HostCopyWriter::HostCopyWriter(
    const WriterParams &copyWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(copyWriterParams)
{
    INFOLOG("Construct HostCopyWriter!");
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.writeAcl = m_backupParams.commonParams.writeAcl;
    m_params.dstTrimPrefix = m_backupParams.commonParams.trimWriterPrefix;
    m_params.writeExtendAttribute = m_backupParams.commonParams.writeExtendAttribute;
    m_params.writeSparseFile = m_backupParams.commonParams.writeSparseFile;
    m_params.discardReadError = m_backupParams.commonParams.discardReadError;
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_copyWriter";
    m_failureRecorder = failureRecorder;
}

HostCopyWriter::~HostCopyWriter()
{
    //  WARN! ALL thread that calls subclass functions MUST BE joined in the destructor of SUBCLASS!
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct %sCopyWriter, destroy thread pool %s", OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str());
    FSBackupUtils::MemoryTrim();
}

BackupRetCode HostCopyWriter::Start()
{
    INFOLOG("Start HostCopyWriter, create thread pool %s size %d",
        m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostCopyWriter::ThreadFunc, this);
        m_pollThread = std::thread(&HostCopyWriter::PollWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostCopyWriter::Abort()
{
    INFOLOG("%sCopyWriter Enter Abort", OS_PLATFORM_NAME.c_str());
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostCopyWriter::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostCopyWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool HostCopyWriter::IsComplete()
{
    /* in write phase, faild item should contain not only write failed but also read failed */
    /* thus, use BackupControlInfo::m_noOfFilesFailed and BackupControlInfo::m_noOfDirFailed */
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyWriter check is complete: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(writedFiles %llu aggrFiles %llu "
            "skipFiles %llu backupFailedFiles %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu archiveFiles %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_writeTaskProduce.load(), m_writeTaskConsume.load(),
            m_controlInfo->m_noOfFilesCopied.load(),
            m_controlInfo->m_aggregatedFiles.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(),
            m_controlInfo->m_archiveFiles.load());
    }

    if (m_controlInfo->m_aggregatePhaseComplete &&
        m_writeQueue->Empty() &&
        (m_writeCache.size() == 0) &&
        (m_timer.GetCount() == 0) &&
        (m_writeTaskProduce == m_writeTaskConsume) &&
        ((m_controlInfo->m_noOfFilesCopied  +
        m_controlInfo->m_noOfFilesFailed) == m_controlInfo->m_noOfFilesToBackup)) {
        INFOLOG("CopyWriter complete: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(writedFiles %llu writedDir %llu aggrFiles %llu "
            "skipFiles %llu skipDir %llu backupFailedFiles %llu backupFailedDir %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu totalDirs %llu archiveFiles %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_writeTaskProduce.load(), m_writeTaskConsume.load(),
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
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    return false;
}

bool HostCopyWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}

int HostCopyWriter::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto taskPtr = make_shared<OsPlatformServiceTask>(
        HostEvent::OPEN_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(taskPtr) == false) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    m_dstOpenedHandleSet.insert(fileHandle.m_file);
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int HostCopyWriter::WriteData(FileHandle& fileHandle)
{
    DBGLOG("Enter WriteData: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::WRITE_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write data task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int HostCopyWriter::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Enter CloseFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::CLOSE_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put close task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    m_dstOpenedHandleSet.erase(fileHandle.m_file);
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int HostCopyWriter::CreateDir(FileHandle& fileHandle)
{
    DBGLOG("Enter CreateDir: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::CREATE_DIR, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put create dir task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int64_t HostCopyWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        ProcessWriteEntries(fh);
    }
    return delay;
}

void HostCopyWriter::ClearWriteCache()
{
    if (m_writeCache.empty()) {
        return;
    }

    for (auto itr = m_writeCache.begin(); itr != m_writeCache.end();) {
        if (!itr->second.empty() && (itr->second[0].m_file->GetDstState() != FileDescState::INIT)) {
            for (auto& i : itr->second) {
                DBGLOG("push from cache %s", i.m_file->m_fileName.c_str());
                m_writeQueue->Push(i);
            }
            itr = m_writeCache.erase(itr);
        } else {
            itr++;
        }
    }
}

void HostCopyWriter::InsertWriteCache(FileHandle& fileHandle)
{
    DBGLOG("push to cache %s", fileHandle.m_file->m_fileName.c_str());
    m_writeCache[fileHandle.m_file->m_fileName].push_back(fileHandle);
}

bool HostCopyWriter::IsOpenBlock(const FileHandle& fileHandle)
{
    return ((fileHandle.m_block.m_size == 0) && (fileHandle.m_block.m_seq == 0));
}

/* check if we can take ThreadFunc into base class */
void HostCopyWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sCopyWriter ThreadFunc thread!", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (IsComplete()) {
            INFOLOG("Complete %sCopyWriter ThreadFunc thread!", OS_PLATFORM_NAME.c_str());
            m_threadDone = true;
            return;
        }
        if (IsAbort()) {
            INFOLOG("Abort %sCopyWriter ThreadFunc thread!", OS_PLATFORM_NAME.c_str());
            m_threadDone = true;
            return;
        }
        int64_t delay = ProcessTimers();
        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(
            fileHandle, delay < QUEUE_TIMEOUT_MILLISECOND ? delay : QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ProcessWriteEntries(fileHandle);
        }
        ClearWriteCache();
    }
    INFOLOG("Finish %sCopyWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    m_threadDone = true;
    return;
}

void HostCopyWriter::PollWriteTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sCopyReader PollWriteTask thread", OS_PLATFORM_NAME.c_str());

    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            break;
        }
        if (m_controlInfo->m_writePhaseComplete) {
            INFOLOG("Finish %sCopyWriter PollWriteTask thread", OS_PLATFORM_NAME.c_str());
            m_pollThreadDone = true;
            return;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            shared_ptr<OsPlatformServiceTask> taskPtr = dynamic_pointer_cast<OsPlatformServiceTask>(threadPoolRes);
            if (taskPtr == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }
            ++m_writeTaskConsume;
            DBGLOG("write tasks consume cnt for now %llu", m_writeTaskConsume.load());
            if (taskPtr->m_result == SUCCESS) {
                HandleSuccessEvent(taskPtr);
            } else {
                HandleFailedEvent(taskPtr);
            }
        }
    }

    INFOLOG("Finish %sCopyWriter PollWriteTask thread", OS_PLATFORM_NAME.c_str());
    m_pollThreadDone = true;
    return;
}

void HostCopyWriter::CloseWriteFailedHandle(FileHandle& fileHandle)
{
    // Close failed file handle when all handle's block habe been delete from m_blockBufferMap
    if (m_blockBufferMap->Get(fileHandle.m_file->m_fileName) == nullptr) {
        CloseFile(fileHandle);
    }
}

void HostCopyWriter::HandleSuccessEvent(std::shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("Host copy writer success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), (int)state);
    if ((event == HostEvent::OPEN_DST) && (state != FileDescState::WRITE_SKIP)) {
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        return;
    }

    if (event == HostEvent::WRITE_META || state == FileDescState::WRITE_SKIP) {
        fileHandle.m_file->SetDstState(FileDescState::END);
        m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        if (state == FileDescState::WRITE_SKIP) {
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        }
        DBGLOG("Finish backup file %s total backup file %d",
            fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesCopied.load());
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
            m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
            DBGLOG("Finish backup file %s originalFileCount %d total backup file %d",
                fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_originalFileCount,
                m_controlInfo->m_noOfFilesCopied.load());
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

void HostCopyWriter::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
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
        // failed dirs are collected in the dir phase.
        if (!fileHandle.m_file->IsFlagSet(IS_DIR) &&
            fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
            // 若read的状态为READ_FAILED时，说明该文件已经被reader记为失败
            m_controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
        }
        fileHandle.m_file->UnlockCommonMutex();
        fileHandle.m_errNum = taskPtr->m_errDetails.second;
        m_failedList.emplace_back(fileHandle);
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

void HostCopyWriter::CloseOpenedHandle()
{
    INFOLOG("Default Implementation!");
}
