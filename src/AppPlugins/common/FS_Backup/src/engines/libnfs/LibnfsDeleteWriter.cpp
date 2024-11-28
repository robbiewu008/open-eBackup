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
#include "LibnfsDeleteWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Libnfscommonmethods;

namespace {
    constexpr uint64_t MAX_BACKUP_QUEUE_SIZE = 10000;
    constexpr uint64_t MIN_BACKUP_QUEUE_SIZE = 8000;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr uint64_t MAX_THREADPOOL_TASK = 32;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

LibnfsDeleteWriter::LibnfsDeleteWriter(const WriterParams &deleteWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(deleteWriterParams)
{
    m_failureRecorder = failureRecorder;
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.jobStartTime = m_advParams->jobStartTime;
    m_params.deleteJobStartTime = m_advParams->deleteJobStartTime;
    m_params.dstRootPath = m_advParams->dataPath;
    m_params.backupParams = m_backupParams;
    m_params.writeObj = this;
    m_params.failureRecorder = m_failureRecorder;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_deleteWriter";

    m_commonData.failureRecorder = m_failureRecorder;
    m_commonData.controlInfo = m_controlInfo;
    m_commonData.abort = &m_abort;

    m_pktStats = make_shared<PacketStats>();

    FillNfsServerCheckParams();
}

LibnfsDeleteWriter::~LibnfsDeleteWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_monitorWriteThread.joinable()) {
        m_monitorWriteThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct LibnfsDeleteWriter, destroy thread pool %s", m_threadPoolKey.c_str());
}

BackupRetCode LibnfsDeleteWriter::Start()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("Start LibnfsDeleteWriter, create thread pool %s", m_threadPoolKey.c_str());

    string dstRootPath = NFS_URL + m_advParams->ip + SEP + m_advParams->sharePath + SEP;
    if (!FillNfsContextContainer(dstRootPath, SERVER_CHECK_NFS_CONTEXT_CNT, m_nfsContextContainer, m_backupParams,
        m_advParams->serverCheckSleepTime)) {
        m_nfsContextContainer.DestroyNfsContext();
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        ERRLOG("Create Write NFS containers failed");
        return BackupRetCode::FAILED;
    }

    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey));
    try {
        m_thread = thread(&LibnfsDeleteWriter::ThreadFunc, this);
        m_monitorWriteThread = thread(&LibnfsDeleteWriter::MonitorWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknown reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDeleteWriter::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsDeleteWriter abort!");
    m_abort = true;
    HandleAbort();
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDeleteWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsDeleteWriter::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsDeleteWriter GetStatus!");

    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, m_failReason);
}

int LibnfsDeleteWriter::OpenFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDeleteWriter::WriteMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDeleteWriter::WriteData(FileHandle& fileHandle)
{
    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::DELETE, m_controlInfo, fileHandle, m_params, m_pktStats);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write data (delete) task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    m_writerProduce++;
    m_runningJob++;
    return MP_SUCCESS;
}

int LibnfsDeleteWriter::CloseFile(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

void LibnfsDeleteWriter::PrintIsComplete(bool forcePrint)
{
    string prefixStr {};
    if (forcePrint) {
        prefixStr = "completed";
    } else {
        prefixStr = "in Progress";
    }
    if (forcePrint == true || ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL)) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DeleteWriter check is %s."
            "aggregateComplete  %d, controlInfo->m_aggregateProduce  %d, writerProduce %d, "
            "writeQueueEmpty: %d, runningJob: %d, RetryEmpty: %d", prefixStr.c_str(),
            m_controlInfo->m_aggregatePhaseComplete.load(), m_controlInfo->m_aggregateProduce.load(),
            m_writerProduce.load(), m_writeQueue->Empty(), m_runningJob.load(), IsRetryReqEmpty());
    }
}

bool LibnfsDeleteWriter::IsComplete()
{
    PrintIsComplete(false);

    if (m_controlInfo->m_aggregatePhaseComplete && m_writeQueue->Empty() && m_runningJob == 0 && IsRetryReqEmpty()
        && (m_controlInfo->m_aggregateProduce == m_writerProduce)) {
        INFOLOG("Delete writer complete!");
        return true;
    }
    return m_controlInfo->m_writePhaseComplete.load();
}

void LibnfsDeleteWriter::HandleComplete()
{
    INFOLOG("Complete LibnfsDeleteWriter");
    PrintIsComplete(true);
    m_nfsContextContainer.DestroyNfsContext();
    m_controlInfo->m_writePhaseComplete = true;
}

bool LibnfsDeleteWriter::IsBlockRecv() const
{
    if (m_writeQueue->GetSizeWithOutLock() >= MAX_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDeleteWriter::BlockRecv() const
{
    m_writeQueue->BlockPush();
}

bool LibnfsDeleteWriter::IsResumeRecv() const
{
    if (m_writeQueue->GetSizeWithOutLock() < MIN_BACKUP_QUEUE_SIZE) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDeleteWriter::ResumeRecv() const
{
    m_writeQueue->CancelBlockPush();
}

bool LibnfsDeleteWriter::IsBlockSend() const
{
    if (m_runningJob > MAX_THREADPOOL_TASK) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDeleteWriter::BlockSend() const
{
    m_writeQueue->BlockPop();
}

bool LibnfsDeleteWriter::IsResumeSend() const
{
    if (m_runningJob < MAX_THREADPOOL_TASK) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDeleteWriter::ResumeSend() const
{
    m_writeQueue->CancelBlockPop();
}

/* check if we can take ThreadFunc into base class */
void LibnfsDeleteWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDeleteWriter main thread start!");
    while (true) {
        if (IsAbort(m_commonData) || NfsServerCheck(m_nasServerCheckParams) == MP_FAILED) {
            WARNLOG("LibnfsDeleteWriter main thread abort");
            break;
        }

        if (IsComplete()) {
            break;
        }

        ProcRetryTimers();

        if (IsBlockRecv()) {
            BlockRecv();
        } else if (IsResumeRecv()) {
            ResumeRecv();
        }

        if (IsBlockSend()) {
            BlockSend();
        } else if (IsResumeSend()) {
            ResumeSend();
        }

        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ++m_controlInfo->m_writerConsume;
            WriteData(fileHandle);
        }
    }
    HandleComplete();
    INFOLOG("LibnfsDeleteWriter main thread end!");
    return;
}

void LibnfsDeleteWriter::MonitorWriteTask()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDeleteWriter monitor thread start!");
    while (true) {
        if (IsAbort(m_commonData)) {
            WARNLOG("LibnfsDeleteWriter monitor thread abort!");
            return;
        }
        if (IsComplete()) {
            INFOLOG("complete , exit monitor thread");
            return;
        }
        if (!m_jsPtr) {
            INFOLOG("jsptr is nullptr, return");
            return;
        }
        shared_ptr<ExecutableItem> threadPoolRes;
        // block until get
        DBGLOG("Waiting for write task complete");
        if (m_jsPtr->Get(threadPoolRes, true, BACKUP_QUEUE_WAIT_TO_MS)) {
            DBGLOG("Get a write task complete!");
            shared_ptr<LibnfsServiceTask> taskPtr = dynamic_pointer_cast<LibnfsServiceTask>(threadPoolRes);
            if (taskPtr == nullptr) {
                ERRLOG("taskPtr is nullptr");
                continue;
            }

            m_runningJob--;
        }

        if (IsResumeSend()) {
            ResumeSend();
        }
    }
    PrintIsComplete(true);
    INFOLOG("LibnfsDeleteWriter monitor thread end!");
    return;
}

void LibnfsDeleteWriter::HandleAbort()
{
    INFOLOG("Handling LibnfsDeleteWriter Abort");

    while (m_runningJob > 0) {
        shared_ptr<ExecutableItem> threadPoolRes;
        // block until get
        DBGLOG("Waiting for write task complete");
        if (m_jsPtr->Get(threadPoolRes)) {
            DBGLOG("Get a write task complete!");
            shared_ptr<LibnfsServiceTask> task = dynamic_pointer_cast<LibnfsServiceTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                continue;
            }

            m_runningJob--;
        }
    }

    // Clear the retry MAP
    ExpireRetryTimers();

    INFOLOG("Handling LibnfsDeleteWriter Abort completed");
}

void LibnfsDeleteWriter::FillNfsServerCheckParams()
{
    m_nasServerCheckParams.pktStats = m_pktStats;
    m_nasServerCheckParams.controlInfo = m_controlInfo;
    m_nasServerCheckParams.failReason = m_failReason;
    m_nasServerCheckParams.nfsContextContainer = &m_nfsContextContainer;
    m_nasServerCheckParams.advParams = m_advParams;
    m_nasServerCheckParams.backupType = m_backupParams.backupType;
    m_nasServerCheckParams.direction = "Writer";
    m_nasServerCheckParams.phase = m_backupParams.phase;
    m_nasServerCheckParams.ratelimitTimer = m_ratelimitTimer;
}

void LibnfsDeleteWriter::ProcRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        m_writeQueue->Push(fileHandle);
        m_controlInfo->m_aggregateProduce++;
    }
}

void LibnfsDeleteWriter::ExpireRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.ClearAllEvents(fileHandles);
}

uint64_t LibnfsDeleteWriter::GetRetryTimerCnt()
{
    return m_timer.GetCount();
}

bool LibnfsDeleteWriter::IsRetryReqEmpty()
{
    return (m_timer.GetCount() == 0);
}
