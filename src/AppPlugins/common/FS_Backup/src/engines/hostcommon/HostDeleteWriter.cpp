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
#include "HostDeleteWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int RETRY_TIME_MILLISENCOND = 1000;
}

HostDeleteWriter::HostDeleteWriter(
    const WriterParams &deleteWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : WriterBase(deleteWriterParams)
{
    INFOLOG("Construct %sDeleteWriter!", OS_PLATFORM_NAME.c_str());
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.dstTrimPrefix = m_backupParams.commonParams.trimWriterPrefix;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_deleteWriter";
    m_failureRecorder = failureRecorder;
}

HostDeleteWriter::~HostDeleteWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct %sDeleteWriter, destroy thread pool %s", OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str());
}

BackupRetCode HostDeleteWriter::Start()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sDeleteWriter, create thread pool %s size %d",
        OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostDeleteWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDeleteWriter::Abort()
{
    INFOLOG("Abort %sDeleteWriter", OS_PLATFORM_NAME.c_str());
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDeleteWriter::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostDeleteWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

int HostDeleteWriter::OpenFile(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

int HostDeleteWriter::WriteMeta(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

int HostDeleteWriter::WriteData(FileHandle& fileHandle)
{
    DBGLOG("Enter %sDeleteWriter WriteData: %s", OS_PLATFORM_NAME.c_str(), fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::DELETE_ITEM, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put write data (delete) task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}

int HostDeleteWriter::CloseFile(FileHandle& /* fileHandle */)
{
    return SUCCESS;
}

bool HostDeleteWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}

bool HostDeleteWriter::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DeleteWriter check is complete: "
            "aggregateComplete %d writeQueueSize %llu timerSize %llu "
            "writeTaskProduce %llu writeTaskConsume %llu",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_timer.GetCount(), m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load());
    }
    if (m_controlInfo->m_aggregatePhaseComplete &&
        m_writeQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_writeTaskProduce == m_controlInfo->m_writeTaskConsume)) {
        INFOLOG("DeleteWriter complete: "
            "aggregateComplete %d writeQueueSize %llu timerSize %llu "
            "writeTaskProduce %llu writeTaskConsume %llu",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_timer.GetCount(), m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    return false;
}

int64_t HostDeleteWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        if (IsAbort()) {
            return 0;
        }
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        FileDescState state = fh.m_file->GetDstState();
        if (state == FileDescState::INIT) {
            WriteData(fh);
        }
    }
    return delay;
}

/* check if we can take ThreadFunc into base class */
void HostDeleteWriter::ThreadFunc()
{
    INFOLOG("Start %sDeleteWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (IsComplete()) {
            m_threadDone = true;
            return;
        }
        if (IsAbort()) {
            m_threadDone = true;
            return;
        }
        int64_t delay = ProcessTimers();
        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(
            fileHandle, delay < QUEUE_TIMEOUT_MILLISECOND ? delay : QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ++m_controlInfo->m_writerConsume;
            FileDescState state = fileHandle.m_file->GetDstState();
            if (state == FileDescState::INIT) {
                WriteData(fileHandle);
            }
        }
        PollWriteTask();
    }
    INFOLOG("Finish %sDeleteWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    m_threadDone = true;
    return;
}

void HostDeleteWriter::PollWriteTask()
{
    shared_ptr<ExecutableItem> treadPoolRes;

    while (m_jsPtr->Get(treadPoolRes, false)) {
        shared_ptr<OsPlatformServiceTask> task = dynamic_pointer_cast<OsPlatformServiceTask>(treadPoolRes);
        if (task == nullptr) {
            ERRLOG("task is nullptr");
            break;
        }
        ++m_controlInfo->m_writeTaskConsume;
        if (task->m_result == SUCCESS) {
            HandleSuccessEvent(task);
        } else {
            HandleFailedEvent(task);
        }
    }

    return;
}

void HostDeleteWriter::HandleSuccessEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle& fileHandle = taskPtr->m_fileHandle;
    FileDescState state = fileHandle.m_file->GetDstState();

    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        ++m_deleteDir;
        ++m_controlInfo->m_noOfDirDeleted;
    } else {
        ++m_deleteFile;
        ++m_controlInfo->m_noOfFilesDeleted;
    }

    DBGLOG("%s delete success %s m_deleteDir %d m_deleteFile %d",
        OS_PLATFORM_NAME.c_str(), fileHandle.m_file->m_fileName.c_str(), m_deleteDir.load(), m_deleteFile.load());

    if (state == FileDescState::INIT) {
        fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    }

    return;
}

void HostDeleteWriter::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle& fileHandle = taskPtr->m_fileHandle;
    fileHandle.m_retryCnt++;

    DBGLOG("%s delete failed %s retry cnt %d",
        OS_PLATFORM_NAME.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);

    if (FSBackupUtils::IsStuck(m_controlInfo)) {
        ERRLOG("set backup to failed due to stucked!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        return;
    }

    if (fileHandle.m_retryCnt >= DEFAULT_ERROR_SINGLE_FILE_CNT ||
        taskPtr->IsCriticalError()) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);

        if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
            fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
            // 文件夹错误不进入错误队列
            if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
                fileHandle.m_errNum = taskPtr->m_errDetails.second;
                m_failedList.emplace_back(fileHandle);
            }
        }
        if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
            ++m_deleteFailedDir;
            ++m_controlInfo->m_noOfDirFailed;
        } else {
            ++m_deleteFailedFile;
            ++m_controlInfo->m_noOfFilesFailed;
        }
        if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
            ERRLOG("set backup to failed!");
            m_controlInfo->m_failed = true;
            m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        }
        ERRLOG("delete failed for file %s %llu %llu, totalFailed: %llu, %llu",
            fileHandle.m_file->m_fileName.c_str(), m_deleteFailedDir.load(), m_deleteFailedFile.load(),
            m_controlInfo->m_noOfDirFailed.load(), m_controlInfo->m_noOfFilesFailed.load());
        return;
    }

    m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);

    return;
}