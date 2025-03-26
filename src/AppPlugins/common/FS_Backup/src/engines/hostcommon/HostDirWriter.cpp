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
#include "HostDirWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "OsPlatformDefines.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int RETRY_TIME_MILLISENCOND = 1000;
}

HostDirWriter::HostDirWriter(
    const WriterParams &dirWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : WriterBase(dirWriterParams)
{
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.writeAcl = m_backupParams.commonParams.writeAcl;
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.dstTrimPrefix = m_backupParams.commonParams.trimWriterPrefix;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_dirWriter";
    m_failureRecorder = failureRecorder;
}

HostDirWriter::~HostDirWriter()
{
    //  WARN! ALL thread that calls subclass functions MUST BE joined in the destructor of SUBCLASS!
    if (m_thread.joinable()) {
        m_thread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct %sDirWriter, destroy thread pool %s", OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str());
}

BackupRetCode HostDirWriter::Start()
{
    INFOLOG("Start %sDirWriter, create thread pool %s size %d",
        OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostDirWriter::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDirWriter::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostDirWriter::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostDirWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

int HostDirWriter::WriteMeta(FileHandle& fileHandle)
{
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::WRITE_META, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put write meta file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}

int HostDirWriter::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int HostDirWriter::WriteData(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int HostDirWriter::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

bool HostDirWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}

bool HostDirWriter::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DirWriter check is complete: "
            "aggrComplete %d writeQueueSize %llu (writeTaskProduce %llu writeTaskConsume %llu)"
            "(writedDir %llu writeFailedDir %llu) (controlFileReaderProduce %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_noOfDirToBackup.load());
    }

    if (m_controlInfo->m_aggregatePhaseComplete &&
        m_writeQueue->Empty() &&
        (m_controlInfo->m_writeTaskProduce == m_controlInfo->m_writeTaskConsume) &&
        ((m_controlInfo->m_noOfDirCopied + m_controlInfo->m_noOfDirFailed) ==
        m_controlInfo->m_noOfDirToBackup)) {
        INFOLOG("DirWriter complete: "
            "aggrComplete %d writeQueueSize %llu (writeTaskProduce %llu writeTaskConsume %llu)"
            "(writedDir %llu writeFailedDir %llu) (controlFileReaderProduce %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(), m_writeQueue->GetSize(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfDirCopied.load(),
            m_controlInfo->m_noOfDirFailed.load(),
            m_controlInfo->m_noOfDirToBackup.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    return false;
}

int64_t HostDirWriter::ProcessTimers()
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
            WriteMeta(fh);
        }
    }
    return delay;
}

/* check if we can take ThreadFunc into base class */
void HostDirWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sDirWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
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
                WriteMeta(fileHandle);
            }
        }
        PollWriteTask();
    }
    INFOLOG("Finish %sDirWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    m_threadDone = true;
    return;
}

void HostDirWriter::PollWriteTask()
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

void HostDirWriter::HandleSuccessEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle& fileHandle = taskPtr->m_fileHandle;
    FileDescState state = fileHandle.m_file->GetDstState();

    ++m_controlInfo->m_noOfDirCopied;
    DBGLOG("Win32 dir success %s writeDir %d",
        fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfDirCopied.load());

    if (state == FileDescState::INIT) {
        fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
    }
    return;
}