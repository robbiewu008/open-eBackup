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
#include "ObjectDeleteWriter.h"
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

ObjectDeleteWriter::ObjectDeleteWriter(
    const WriterParams &deleteWriterParams, std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : WriterBase(deleteWriterParams)
{
    INFOLOG("Construct ObjectDeleteWriter!");
    m_dstAdvParams = dynamic_pointer_cast<ObjectBackupAdvanceParams>(m_backupParams.dstAdvParams);

    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.writeAcl = m_backupParams.commonParams.writeAcl;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.authArgs = m_dstAdvParams->authArgs;
    m_params.dstBucket = m_dstAdvParams->dstBucket;
    for (auto &item : m_dstAdvParams->buckets) {
        m_params.bucketNames.emplace_back(item.bucketName);
    }
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_deleteWriter";
    m_failureRecorder = failureRecorder;
}

ObjectDeleteWriter::~ObjectDeleteWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct ObjectDeleteWriter, destroy thread pool %s", m_threadPoolKey.c_str());
}

BackupRetCode ObjectDeleteWriter::Start()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start ObjectDeleteWriter, create thread pool %s, number %d",
        m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&ObjectDeleteWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectDeleteWriter::Abort()
{
    INFOLOG("Abort ObjectDeleteWriter");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectDeleteWriter::Destroy()
{
    INFOLOG("ObjectDeleteWriter Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or ObjectDeleteWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

int ObjectDeleteWriter::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Open file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int ObjectDeleteWriter::WriteMeta(FileHandle& fileHandle)
{
    DBGLOG("Write meta for file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

int ObjectDeleteWriter::WriteData(FileHandle& fileHandle)
{
    DBGLOG("Write data for %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::DELETE_ITEM, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write data (delete) task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}

int ObjectDeleteWriter::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Close file %s", fileHandle.m_file->m_fileName.c_str());
    return SUCCESS;
}

bool ObjectDeleteWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}

bool ObjectDeleteWriter::IsComplete()
{
    if (m_controlInfo->m_aggregatePhaseComplete && m_writeQueue->Empty() && (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_writeTaskProduce == m_controlInfo->m_writeTaskConsume)) {
        INFOLOG("DeleteWriter complete: aggregateComplete %d writeQueueSize %llu timerSize %llu "
            "writeTaskProduce %llu writeTaskConsume %llu",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_timer.GetCount(), m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DeleteWriter check is complete: aggregateComplete %d writeQueueSize %llu timerSize %llu "
            "writeTaskProduce %llu writeTaskConsume %llu",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_timer.GetCount(), m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load());
    }

    return false;
}

int64_t ObjectDeleteWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        FileDescState state = fh.m_file->GetDstState();
        if (state == FileDescState::INIT) {
            WriteData(fh);
        }
    }
    return delay;
}

/* check if we can take ThreadFunc into base class */
void ObjectDeleteWriter::ThreadFunc()
{
    INFOLOG("Start ObjectDeleteWriter ThreadFunc thread");
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
        delay = (delay < QUEUE_TIMEOUT_MILLISECOND) ? delay : QUEUE_TIMEOUT_MILLISECOND;
        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(fileHandle, delay);
        if (ret) {
            ++m_controlInfo->m_writerConsume;
            FileDescState state = fileHandle.m_file->GetDstState();
            if (state == FileDescState::INIT) {
                WriteData(fileHandle);
            }
        }
        PollWriteTask();
    }
    INFOLOG("Finish ObjectDeleteWriter ThreadFunc thread");
    m_threadDone = true;
    return;
}

void ObjectDeleteWriter::PollWriteTask()
{
    shared_ptr<ExecutableItem> treadPoolRes;

    while (m_jsPtr->Get(treadPoolRes, false)) {
        shared_ptr<ObjectServiceTask> taskPtr = dynamic_pointer_cast<ObjectServiceTask>(treadPoolRes);
        if (taskPtr == nullptr) {
            ERRLOG("task is nullptr");
            break;
        }
        ++m_controlInfo->m_writeTaskConsume;
        if (taskPtr->m_result == SUCCESS) {
            HandleSuccessEvent(taskPtr);
        } else {
            HandleFailedEvent(taskPtr);
        }
    }

    return;
}

void ObjectDeleteWriter::HandleSuccessEvent(shared_ptr<ObjectServiceTask> taskPtr)
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

    DBGLOG("delete success %s, m_deleteDir %d, m_deleteFile %d",
        fileHandle.m_file->m_fileName.c_str(), m_deleteDir.load(), m_deleteFile.load());

    if (state == FileDescState::INIT) {
        fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    }

    return;
}

void ObjectDeleteWriter::HandleFailedEvent(shared_ptr<ObjectServiceTask> taskPtr)
{
    FileHandle& fileHandle = taskPtr->m_fileHandle;
    fileHandle.m_retryCnt++;

    DBGLOG("delete failed %s retry cnt %d", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_retryCnt);

    if (fileHandle.m_retryCnt>= DEFAULT_ERROR_SINGLE_FILE_CNT || taskPtr->IsCriticalError()) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
            fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
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
        if (!fileHandle.m_errMessage.empty()) {
            m_failedList.emplace_back(fileHandle);
        }
        ERRLOG("delete failed for file %s %llu %llu, totalFailed: %llu, %llu",
            fileHandle.m_file->m_fileName.c_str(), m_deleteFailedDir.load(), m_deleteFailedFile.load(),
            m_controlInfo->m_noOfDirFailed.load(), m_controlInfo->m_noOfFilesFailed.load());
        return;
    }

    m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
    return;
}
