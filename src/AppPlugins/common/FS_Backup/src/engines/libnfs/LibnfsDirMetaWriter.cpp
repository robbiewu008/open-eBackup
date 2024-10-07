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
#include "LibnfsDirMetaWriter.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "LibnfsDirMetaWriter.h"
#include "LibnfsStructs.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;
using namespace Libnfscommonmethods;

namespace {
    constexpr uint64_t BACKUP_QUEUE_WAIT_TO_MS = 50;
    constexpr uint64_t MAX_THREADPOOL_TASK = 32;
    constexpr uint64_t MAX_FILEHANDLECACHE_SIZE = 110000;
    const int MP_SUCCESS = 0;
    const int MP_FAILED = 1;
}

LibnfsDirMetaWriter::LibnfsDirMetaWriter(const WriterParams &dirWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(dirWriterParams),
    m_nfsContextContainer(m_backupParams.commonParams.reqID)
{
    m_failureRecorder = failureRecorder;
    m_advParams = dynamic_pointer_cast<LibnfsBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_fileHandleCache = make_shared<FileHandleCache>();
    m_pktStats = make_shared<PacketStats>();

    m_params.jobStartTime = m_advParams->jobStartTime;
    m_params.deleteJobStartTime = m_advParams->deleteJobStartTime;
    m_params.backupParams = m_backupParams;
    m_params.fileHandleCache = m_fileHandleCache;
    m_params.abort = &m_abort;
    m_params.writeObj = this;
    m_params.failureRecorder = m_failureRecorder;

    m_commonData.failureRecorder = m_failureRecorder;
    m_commonData.controlInfo = m_controlInfo;
    m_commonData.abort = &m_abort;

    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_dirMetaWriter";

    FillNfsServerCheckParams();
}

LibnfsDirMetaWriter::~LibnfsDirMetaWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_monitorWriteThread.joinable()) {
        m_monitorWriteThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct LibnfsDirMetaWriter, destroy thread pool %s", m_threadPoolKey.c_str());
}

BackupRetCode LibnfsDirMetaWriter::Start()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("Start LibnfsDirMetaWriter, create thread pool %s", m_threadPoolKey.c_str());
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey));

    string dstRootPath = NFS_URL + m_advParams->ip + SEP + m_advParams->sharePath + SEP;
    if (!FillNfsContextContainer(dstRootPath, DIR_MTIME_NFS_CONTEXT_CNT, m_nfsContextContainer, m_backupParams,
        m_advParams->serverCheckSleepTime)) {
        m_nfsContextContainer.DestroyNfsContext();
        FSBackupUtils::SetServerNotReachableErrorCode(m_backupParams.backupType, m_failReason, false);
        ERRLOG("Create Write NFS containers failed");
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&LibnfsDirMetaWriter::ThreadFunc, this);
        m_monitorWriteThread = thread(&LibnfsDirMetaWriter::MonitorWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDirMetaWriter::Abort()
{
    lock_guard<mutex> lk(mtx);
    INFOLOG("LibnfsDirMetaWriter abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode LibnfsDirMetaWriter::Destroy()
{
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus LibnfsDirMetaWriter::GetStatus()
{
    lock_guard<mutex> lk(mtx);
    DBGLOG("Enter LibnfsDirMetaWriter GetStatus!");

    return FSBackupUtils::GetWriterStatus(m_controlInfo, m_abort, m_failReason);
}

int LibnfsDirMetaWriter::OpenFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDirMetaWriter::WriteMeta(FileHandle &fileHandle)
{
    m_params.nfsCtx = m_nfsContextContainer.GetCurrContext();
    if (m_params.nfsCtx == nullptr) {
        ERRLOG("nfs wrapper is null. put write data failed for: %s", fileHandle.m_file->m_fileName.c_str());
        return MP_FAILED;
    }
    auto task = make_shared<LibnfsServiceTask>(LibnfsEvent::WRITE_META, m_controlInfo, fileHandle, m_params,
        m_pktStats);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write data (write meta) task %s failed", fileHandle.m_file->m_fileName.c_str());
        return MP_FAILED;
    }

    m_runningJob++;
    m_nfsContextContainer.IncSendCnt(1);
    return MP_SUCCESS;
}

int LibnfsDirMetaWriter::WriteData(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

int LibnfsDirMetaWriter::CloseFile(FileHandle &fileHandle)
{
    fileHandle = fileHandle;
    return MP_SUCCESS;
}

void LibnfsDirMetaWriter::PrintIsComplete(bool forcePrint)
{
    string prefixStr {};
    if (forcePrint) {
        prefixStr = "completed";
    } else {
        prefixStr = "in Progress";
    }
    if (forcePrint == true || ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL)) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DirWriter check is %s. aggregateComplete  %d, aggregateProduce  %d,"
            "writerConsume  %d, writerProduce %d, writeQueueEmpty: %d, runningJob: %d, "
            "RetryEmpty: %d, FileHandleCache Size: %lu: ", prefixStr.c_str(),
            m_controlInfo->m_aggregatePhaseComplete.load(), m_controlInfo->m_aggregateProduce.load(),
            m_controlInfo->m_writerConsume.load(), m_writerProduce.load(),
            m_writeQueue->Empty(), m_runningJob.load(), IsRetryReqEmpty(), m_fileHandleCache->Size());
    }
}

bool LibnfsDirMetaWriter::IsComplete()
{
    PrintIsComplete(false);

    if (m_controlInfo->m_aggregatePhaseComplete && m_writeQueue->Empty() && m_runningJob == 0 && IsRetryReqEmpty()
        && (m_controlInfo->m_aggregateProduce == m_controlInfo->m_writerConsume)
        && (m_controlInfo->m_writerConsume == m_writerProduce)) {
        INFOLOG("Dir writer complete!");
        return true;
    }
    return m_controlInfo->m_writePhaseComplete.load();
}

void LibnfsDirMetaWriter::HandleComplete()
{
    INFOLOG("LibnfsDeleteWriter Enter HandleComplete");

    // Clear the retry MAP
    ExpireRetryTimers();
    while (m_runningJob != 0) {
        DBGLOG("Waiting for threadpool tasks to end");
        sleep(1);
    }
    m_fileHandleCache->Clear();
    PrintIsComplete(true);
    m_nfsContextContainer.DestroyNfsContext();
    m_controlInfo->m_writePhaseComplete = true;
}

bool LibnfsDirMetaWriter::IsBlockSend() const
{
    if (m_runningJob > MAX_THREADPOOL_TASK) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDirMetaWriter::BlockSend() const
{
    m_writeQueue->BlockPop();
}

bool LibnfsDirMetaWriter::IsResumeSend() const
{
    if (m_runningJob < MAX_THREADPOOL_TASK) {
        return true;
    } else {
        return false;
    }
}

void LibnfsDirMetaWriter::ResumeSend() const
{
    m_writeQueue->CancelBlockPop();
}

void LibnfsDirMetaWriter::HandleQueueBlock()
{
    if (IsBlockSend()) {
        BlockSend();
    } else if (IsResumeSend()) {
        ResumeSend();
    }
}

/* check if we can take ThreadFunc into base class */
void LibnfsDirMetaWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDirMetaWriter main thread start!");
    while (true) {
        if (IsAbort(m_commonData) || NfsServerCheck(m_nasServerCheckParams) == MP_FAILED) {
            WARNLOG("LibnfsDirMetaWriter main thread abort!");
            break;
        }

        if (IsComplete()) {
            INFOLOG("LibnfsDirMetaWriter main thread completed!");
            break;
        }

        ProcRetryTimers();

        HandleQueueBlock();

        if (m_fileHandleCache->Size() > MAX_FILEHANDLECACHE_SIZE) {
            WARNLOG("fileHandleCache hit max threshold. Need to clear.");
            while (m_runningJob != 0) {
                DBGLOG("Waiting for threadpool tasks to end");
                sleep(1);
            }

            m_fileHandleCache->Clear();
            WARNLOG("fileHandleCache cleared.");
        }

        uint16_t count = 0;
        FileHandle fileHandle {};
        do {
            if (m_writeQueue->WaitAndPop(fileHandle, BACKUP_QUEUE_WAIT_TO_MS)) {
                PushToWriter(fileHandle, count);
            } else {
                break;
            }
        } while (count < DIR_MTIME_NFS_CONTEXT_CNT);

        if (!m_params.fileHandleList.empty()) {
            WriteMeta(fileHandle);
            m_writerProduce += m_params.fileHandleList.size();
            m_params.fileHandleList.clear();
        }
    }
    HandleComplete();
    INFOLOG("LibnfsDirMetaWriter main thread end!");
    return;
}

void LibnfsDirMetaWriter::PushToWriter(FileHandle &fileHandle, uint16_t &count)
{
    m_params.fileHandleList.push_back(fileHandle);
    ++m_controlInfo->m_writerConsume;
    ++count;
}

void LibnfsDirMetaWriter::MonitorWriteTask()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("LibnfsDirMetaWriter monitor thread start!");
    while (true) {
        if (IsAbort(m_commonData)) {
            WARNLOG("LibnfsDirMetaWriter monitor thread abort!");
            break;
        }
        if (m_controlInfo->m_writePhaseComplete) {
            INFOLOG("complete , exit monitor thread");
            break;
        }
        if (!m_jsPtr) {
            INFOLOG("jsptr is nullptr, return");
            break;
        }

        shared_ptr<ExecutableItem> threadPoolRes;
        if (m_jsPtr->Get(threadPoolRes, true, BACKUP_QUEUE_WAIT_TO_MS)) {
            DBGLOG("Get a write task complete!");
            shared_ptr<LibnfsServiceTask> task = dynamic_pointer_cast<LibnfsServiceTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                continue;
            }
            m_runningJob--;
        }
        if (IsResumeSend()) {
            ResumeSend();
        }
    }
    PrintIsComplete(true);
    INFOLOG("LibnfsDirMetaWriter monitor thread end!");
    return;
}

void LibnfsDirMetaWriter::FillNfsServerCheckParams()
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

void LibnfsDirMetaWriter::ProcRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle fileHandle : fileHandles) {
        m_writeQueue->Push(fileHandle);
        m_controlInfo->m_aggregateProduce++;
    }
}

void LibnfsDirMetaWriter::ExpireRetryTimers()
{
    vector<FileHandle> fileHandles {};
    m_timer.ClearAllEvents(fileHandles);
}

uint64_t LibnfsDirMetaWriter::GetRetryTimerCnt()
{
    return m_timer.GetCount();
}

bool LibnfsDirMetaWriter::IsRetryReqEmpty()
{
    return (m_timer.GetCount() == 0);
}
