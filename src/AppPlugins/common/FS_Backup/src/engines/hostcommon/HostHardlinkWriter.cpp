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
#include "HostHardlinkWriter.h"
#include "OsPlatformDefines.h"
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

HostHardlinkWriter::HostHardlinkWriter(
    const WriterParams &hardlinkWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : WriterBase(hardlinkWriterParams)
{
    INFOLOG("Construct HostHardlinkWriter");
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.writeAcl = m_backupParams.commonParams.writeAcl;
    m_params.writeExtendAttribute = m_backupParams.commonParams.writeExtendAttribute;
    m_params.writeSparseFile = m_backupParams.commonParams.writeSparseFile;
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.dstTrimPrefix = m_backupParams.commonParams.trimWriterPrefix;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_hardlinkWriter";
    m_failureRecorder = failureRecorder;
}
 
HostHardlinkWriter::~HostHardlinkWriter()
{
    //  WARN! ALL thread that calls subclass functions MUST BE joined in the destructor of SUBCLASS!
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct HostHardlinkWriter, destroy thread pool %s", m_threadPoolKey.c_str());
}
 
BackupRetCode HostHardlinkWriter::Start()
{
    INFOLOG("Start HostHardlinkWriter, create thread pool %s size %d",
        m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostHardlinkWriter::ThreadFunc, this);
        m_pollThread = std::thread(&HostHardlinkWriter::PollWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}
 
BackupRetCode HostHardlinkWriter::Abort()
{
    INFOLOG("HostHardlinkWriter Enter Abort");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostHardlinkWriter::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostHardlinkWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool HostHardlinkWriter::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("HardlinkWriter check is complete: "
            "aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(noOfFilesCopied %llu noOfFilesFailed %llu) (noOfFilesToBackup %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
    }
    if (m_controlInfo->m_aggregatePhaseComplete &&
        m_writeQueue->Empty() &&
        (m_writeCache.size() == 0) &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_writeTaskProduce == m_controlInfo->m_writeTaskConsume) &&
        (m_controlInfo->m_noOfFilesCopied + m_controlInfo->m_noOfFilesFailed == m_controlInfo->m_noOfFilesToBackup)) {
        INFOLOG("HardlinkWriter complete: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
            "(writeTaskProduce %llu writeTaskConsume %llu) "
            "(noOfFilesCopied %llu noOfFilesFailed %llu) (noOfFilesToBackup %llu)",
            m_controlInfo->m_aggregatePhaseComplete.load(),
            m_writeQueue->GetSize(), m_writeCache.size(), m_timer.GetCount(),
            m_controlInfo->m_writeTaskProduce.load(), m_controlInfo->m_writeTaskConsume.load(),
            m_controlInfo->m_noOfFilesCopied.load(), m_controlInfo->m_noOfFilesFailed.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}
 
bool HostHardlinkWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}
 
int HostHardlinkWriter::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::OPEN_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}
 
int HostHardlinkWriter::WriteMeta(FileHandle& fileHandle)
{
    DBGLOG("Enter WriteMeta: %s", fileHandle.m_file->m_fileName.c_str());
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::WRITE_META, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put write meta file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    return SUCCESS;
}
 
int HostHardlinkWriter::WriteData(FileHandle& fileHandle)
{
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::WRITE_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put write data file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    DBGLOG("WriteData: %s, total writeTask produce for now: %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_writeTaskProduce.load());
    return SUCCESS;
}
 
int HostHardlinkWriter::CloseFile(FileHandle& fileHandle)
{
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::CLOSE_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put close file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    DBGLOG("CloseFile:%s ,total writeTask produce for now: %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_writeTaskProduce.load());
    return SUCCESS;
}

int HostHardlinkWriter::LinkFile(FileHandle& fileHandle)
{
    DBGLOG("execute link file : %s", fileHandle.m_file->m_fileName.c_str());
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::LINK, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put write link file task failed %s", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_writeTaskProduce;
    DBGLOG("LinkFile:%s ,total writeTask produce for now: %d", fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_writeTaskProduce.load());
    return SUCCESS;
}
 
void HostHardlinkWriter::ClearWriteCache()
{
    if (m_writeCache.empty()) {
        return;
    }
 
    for (auto it = m_writeCache.begin(); it != m_writeCache.end();) {
        if (!it->second.empty() && (it->second[0].m_file->GetDstState() != FileDescState::INIT)) {
            for (auto& i : it->second) {
                DBGLOG("push from cache %s", i.m_file->m_fileName.c_str());
                m_writeQueue->Push(i);
            }
            it = m_writeCache.erase(it);
        } else {
            it++;
        }
    }
}
 
void HostHardlinkWriter::InsertWriteCache(FileHandle& fileHandle)
{
    DBGLOG("push to cache %s", fileHandle.m_file->m_fileName.c_str());
    m_writeCache[fileHandle.m_file->m_fileName].push_back(fileHandle);
}

int64_t HostHardlinkWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fileHandle : fileHandles) {
        if (IsAbort()) {
            return 0;
        }
        DBGLOG("Process timer %s", fileHandle.m_file->m_fileName.c_str());
        ProcessWriteEntries(fileHandle);
    }
    return delay;
}
 
void HostHardlinkWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sHardlinkWriter ThreadFunc thread", OS_PLATFORM_NAME.c_str());
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
        bool ret = m_writeQueue->WaitAndPop(fileHandle,
            delay < QUEUE_TIMEOUT_MILLISECOND ? delay : QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ProcessWriteEntries(fileHandle);
        }
        ClearWriteCache();
        ProcessHardlinkMap();
    }
    INFOLOG("Finish HostHardlinkWriter ThreadFunc thread");
    m_threadDone = true;
    return;
}
 
void HostHardlinkWriter::PollWriteTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sHardlinkWriter PollWriteTask thread", OS_PLATFORM_NAME.c_str());
 
    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            break;
        }
        if (m_controlInfo->m_writePhaseComplete) {
            INFOLOG("Finish HostHardlinkWriter PollWriteTask thread");
            m_pollThreadDone = true;
            return;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            shared_ptr<OsPlatformServiceTask> task = dynamic_pointer_cast<OsPlatformServiceTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }
            if (task->m_result == SUCCESS) {
                HandleSuccessEvent(task);
            } else {
                HandleFailedEvent(task);
            }
            ++m_controlInfo->m_writeTaskConsume;
            DBGLOG("write tasks consume cnt for now %llu", m_controlInfo->m_writeTaskConsume.load());
        }
    }
    INFOLOG("Finish HostHardlinkWriter PollWriteTask thread");
    m_pollThreadDone = true;
    return;
}

 
void HostHardlinkWriter::ProcessHardlinkMap()
{
    vector<vector<FileHandle>> fileHandles = m_hardlinkMap->GetAllFileHandlesAndClear();
    for (size_t i = 0; i < fileHandles.size(); ++i) {
        string targetPath;
        if (fileHandles[i].empty() || !fileHandles[i][0].m_file) {
            continue;
        }
        int ret = m_hardlinkMap->GetTargetPath(fileHandles[i][0].m_file->m_inode, targetPath);
        if (ret != 0) {
            WARNLOG("not found hardlink target path for inode : %d", fileHandles[i][0].m_file->m_inode);
            continue;
        }
        targetPath = FSBackupUtils::PathConcat(m_params.dstRootPath, targetPath);
        DBGLOG("found target path: %s, file: %s, inode: %u, size: %u",
            targetPath.c_str(),
            fileHandles[i][0].m_file->m_fileName.c_str(),
            fileHandles[i][0].m_file->m_inode,
            fileHandles[i].size());
        m_params.linkTarget = targetPath;
        if (m_hardlinkMap->CheckInodeFailed(fileHandles[i][0].m_file->m_inode)) {
            for (size_t j = 0; j < fileHandles[i].size(); ++j) {
                DBGLOG("set failed hardlink : %s", fileHandles[i][j].m_file->m_fileName.c_str());
                ++m_controlInfo->m_noOfFilesFailed;
            }
            continue;
        }
        for (size_t j = 0; j < fileHandles[i].size(); ++j) {
            if (!fileHandles[i][j].m_file) {
                WARNLOG("fileHandle ptr is null");
                continue;
            }
            DBGLOG("Process hardlink file : %d, %s", j, fileHandles[i][j].m_file->m_fileName.c_str());
            if (!LinkFile(fileHandles[i][j])) {
                continue;
            }
        }
    }
}
 
void HostHardlinkWriter::HandleSuccessEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("Host hardlink writer success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), static_cast<int>(state));

    if ((event == HostEvent::OPEN_DST) && (state != FileDescState::WRITE_SKIP)) {
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
    }

    if ((event == HostEvent::WRITE_META) || (event == HostEvent::LINK) || (state == FileDescState::WRITE_SKIP)) {
        fileHandle.m_file->SetDstState(FileDescState::END);
        if (state == FileDescState::WRITE_SKIP) {
            m_hardlinkMap->SetTargetCopied(fileHandle.m_file->m_inode);
            DBGLOG("set target copied: %d", fileHandle.m_file->m_inode);
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        }
        ++m_controlInfo->m_noOfFilesCopied;
        DBGLOG("Finish backup file %s total backup file %d",
            fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesCopied.load());
    }

    if (event == HostEvent::CLOSE_DST) {
        if (m_params.writeMeta) {
            fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
            m_writeQueue->Push(fileHandle);
        } else {
            fileHandle.m_file->SetDstState(FileDescState::END);
            ++m_controlInfo->m_noOfFilesCopied;
        }
        return;
    }

    if (event == HostEvent::WRITE_DATA) {
        ProcessWriteData(fileHandle);
    }
    return;
}
 
bool HostHardlinkWriter::IsOpenBlock(const FileHandle& fileHandle)
{
    return ((fileHandle.m_block.m_size == 0) && (fileHandle.m_block.m_seq == 0));
}

void HostHardlinkWriter::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    fileHandle.m_retryCnt++;
    FileDescState state = fileHandle.m_file->GetDstState();

    ERRLOG("host hardlink writer failed %s event %d state %d retry cnt %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), state, fileHandle.m_retryCnt);
    
    if (FSBackupUtils::IsStuck(m_controlInfo)) {
        ERRLOG("set backup to failed due to stucked!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
        return;
    }

    if (state != FileDescState::WRITE_FAILED &&  /* If state is WRITE_FAILED, needn't retry */
        state != FileDescState::LINK && /* don't allow retry for link task, (cannot retry in ProcessWriteEntries) */
        fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT && !taskPtr->IsCriticalError()) {
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return;
    }
    if (state != FileDescState::WRITE_FAILED) {
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
        if (fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
            // 如果是LINK , 加一个failed , 然后从hardlinMap里删掉对应的一个
            if (event == HostEvent::LINK) {
                HandleFailedLinkLink(fileHandle);
            } else {
                // 如果是source , 将map里同inode的fileHandle都置为失败， 并添加到failedList
                HandleFailedLinkSource(fileHandle);
            }
            // 文件夹错误不进入错误队列
            if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
                fileHandle.m_errNum = taskPtr->m_errDetails.second;
                m_failedList.emplace_back(fileHandle);
            }
        }
        fileHandle.m_file->UnlockCommonMutex();
    }
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    ERRLOG("hardlink write failed for file %s totalFailed %llu",
        fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesFailed.load());
    return;
}

void HostHardlinkWriter::HandleFailedLinkSource(FileHandle& fileHandle)
{
    // 如果是source , 将map里同inode的fileHandle都置为失败， 并添加到failedList
    ERRLOG("Enter HandleFailedLinkSource: %s", fileHandle.m_file->m_fileName.c_str());
    // 源文件不在hardlinkMap的links里， 这里是设置源文件的失败
    ++m_controlInfo->m_noOfFilesFailed;
    vector<FileHandle> links =  m_hardlinkMap->GetLinksAndClear(fileHandle.m_file->m_inode);
    // 这里是设置links里面链接文件的失败。
    for (FileHandle& fh : links) {
        DBGLOG("set hardlink to fail, %s", fh.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
    }
    m_hardlinkMap->InsertFailedInode(fileHandle.m_file->m_inode);
}

void HostHardlinkWriter::HandleFailedLinkLink(FileHandle& fileHandle)
{
    ERRLOG("Enter HandleFailedLinkLink: %s", fileHandle.m_file->m_fileName.c_str());
    // 如果是LINK , 加一个failed , 然后从hardlinMap里删掉对应的一个
    m_controlInfo->m_noOfFilesFailed++;
    m_hardlinkMap->RemoveLink(fileHandle);
}