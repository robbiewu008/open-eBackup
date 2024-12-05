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
#include "HostHardlinkReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
using namespace std;
using namespace Module;
using namespace FS_Backup;
namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int OOM_SLEEP_SECOND = 1;
    const int RETRY_TIME_MILLISENCOND = 1000;
    const uint32_t INVALID_MEMORY = static_cast<uint32_t>(-1);
}
 
HostHardlinkReader::HostHardlinkReader(
    const ReaderParams &hardlinkReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : ReaderBase(hardlinkReaderParams)
{
    INFOLOG("Construct HostHardlinkReader");
    m_srcAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.srcAdvParams);
    m_dstAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.srcRootPath = m_srcAdvParams->dataPath;
    m_params.dstRootPath = m_dstAdvParams->dataPath;
    m_params.srcTrimPrefix = m_backupParams.commonParams.trimReaderPrefix;
    m_params.writeSparseFile = m_backupParams.commonParams.writeSparseFile;
    m_params.discardReadError = m_backupParams.commonParams.discardReadError;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_hardlinkReader";
    m_failureRecorder = failureRecorder;
}
 
HostHardlinkReader::~HostHardlinkReader()
{
    //  WARN! ALL thread that calls subclass functions MUST BE joined in the destructor of SUBCLASS!
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct HostHardlinkReader, destroy thread pool %s", m_threadPoolKey.c_str());
}
 
BackupRetCode HostHardlinkReader::Start()
{
    INFOLOG("Start HostHardlinkReader, create thread pool %s size %d",
        m_threadPoolKey.c_str(), m_srcAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_srcAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostHardlinkReader::ThreadFunc, this);
        m_pollThread = std::thread(&HostHardlinkReader::PollReadTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}
 
BackupRetCode HostHardlinkReader::Abort()
{
    INFOLOG("Abort %sHardlinkReader", OS_PLATFORM_NAME.c_str());
    m_abort = true;
    return BackupRetCode::SUCCESS;
}
 
BackupRetCode HostHardlinkReader::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostHardlinkReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool HostHardlinkReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}
 
bool HostHardlinkReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("HardlinkReader check is complete: "
            "controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskProduce %llu readTaskConsume %llu) "
            "(readedFiles %llu readFailedFiles %llu skipFiles %llu) (total %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_readTaskProduce == m_controlInfo->m_readTaskConsume)) {
        INFOLOG("HardlinkReader complete: "
            "controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskProduce %llu readTaskConsume %llu) "
            "(readedFiles %llu readFailedFiles %llu skipFiles %llu) (total %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_noOfFilesToBackup.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}
 
int HostHardlinkReader::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    std::shared_ptr<OsPlatformServiceTask> openTask = make_shared<OsPlatformServiceTask>(
        HostEvent::OPEN_SRC, m_blockBufferMap, fileHandle, m_params);
    if ((m_jsPtr->Put(openTask) == false)) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int HostHardlinkReader::ReadData(FileHandle& fileHandle)
{
    if (FSBackupUtils::IsSymLinkFile(fileHandle)) {
        return ReadSymlinkData(fileHandle);
    }
    // Symlink need to be implement
    if ((fileHandle.m_file->m_size == 0) ||
        (m_backupParams.commonParams.writeSparseFile && fileHandle.m_block.m_size == 0)) {
        return ReadEmptyData(fileHandle);
    }
 
    return ReadNormalData(fileHandle);
}
 
int HostHardlinkReader::ReadMeta(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadMeta: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle = fileHandle;
    return SUCCESS;
}
 
int HostHardlinkReader::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Enter CloseFile: %s", fileHandle.m_file->m_fileName.c_str());
    std::shared_ptr<OsPlatformServiceTask> task = make_shared<OsPlatformServiceTask>(
        HostEvent::CLOSE_SRC, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put close file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}
 
int64_t HostHardlinkReader::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        ProcessReadEntries(fh);
    }
    return delay;
}
 
void HostHardlinkReader::BlockReadQueuePop() const
{
    uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
    if ((m_srcAdvParams->maxMemory != INVALID_MEMORY) && (totalBufferSize > m_srcAdvParams->maxMemory)) {
        m_readQueue->BlockPop();
    } else {
        m_readQueue->CancelBlockPop();
    }
}
 
void HostHardlinkReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sHardlinkReader ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (IsComplete()) {
            m_threadDone = true;
            return;
        }
        if (IsAbort()) {
            m_threadDone = true;
            return;
        }
        BlockReadQueuePop();
        int64_t delay = ProcessTimers();
        FileHandle fileHandle;
        bool ret = m_readQueue->WaitAndPop(
            fileHandle, delay < QUEUE_TIMEOUT_MILLISECOND ? delay : QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ProcessReadEntries(fileHandle);
        }
    }
    INFOLOG("Finish HostHardlinkReader ThreadFunc thread!");
    m_threadDone = true;
    return;
}
 
void HostHardlinkReader::PollReadTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sHardlinkReader PollReadTask thread", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            m_pollThreadDone = true;
            break;
        }
        if (m_controlInfo->m_readPhaseComplete) {
            INFOLOG("Finish HostHardlinkReader PollReadTask thread");
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
            ++m_controlInfo->m_readTaskConsume;
            DBGLOG("read tasks consume cnt for now %llu", m_controlInfo->m_readTaskConsume.load());
        }
    }
    INFOLOG("Finish HostHardlinkReader PollReadTask thread");
    m_pollThreadDone = true;
    return;
}

void HostHardlinkReader::HandleFailedEvent(shared_ptr<OsPlatformServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    HostEvent event = taskPtr->m_event;
    fileHandle.m_retryCnt++;
    DBGLOG("Host hardlink reader failed %s event %d retry cnt %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), fileHandle.m_retryCnt);
    FileDescState state = fileHandle.m_file->GetSrcState();
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
        if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
            // 源文件不在hardlinkMap的links里， 这里是设置源文件的失败
            ++m_controlInfo->m_noOfFilesFailed;
            // 这里是设置links里面链接文件的失败。
            HandleFailedLink(taskPtr->m_fileHandle);
        }
        fileHandle.m_file->UnlockCommonMutex();
        fileHandle.m_errNum = taskPtr->m_errDetails.second;
        m_failedList.emplace_back(fileHandle);
    }

    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    ERRLOG("hardlink read failed for file %s %llu, totalFailed: %llu",
        fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesReadFailed.load(),
        m_controlInfo->m_noOfFilesFailed.load());
    return;
}
 
void HostHardlinkReader::PushToAggregator(FileHandle& fileHandle)
{
    DBGLOG("push to aggregateQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
    fileHandle.m_retryCnt = 0;
    m_aggregateQueue->WaitAndPush(fileHandle);
    ++m_controlInfo->m_readProduce;
    return;
}
 
void HostHardlinkReader::PushToReader(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    if (state == FileDescState::READED) {
        DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
        return;
    }
    if (state == FileDescState::SRC_OPENED) {
        if (fileHandle.m_file->m_size == 0) {
            DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
            m_readQueue->Push(fileHandle);
            return;
        }
        // Symlink implement
        if (FSBackupUtils::IsSymLinkFile(fileHandle)) {
            DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
            m_readQueue->Push(fileHandle);
            return;
        }
        DecomposeAndPush(fileHandle);
    }
}
 
void HostHardlinkReader::DecomposeAndPush(
    FileHandle& fileHandle, uint64_t startOffset, uint64_t totalSize, uint64_t& startSeq) const
{
    uint32_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t fullBlockNum = totalSize / blockSize;
    uint32_t remainSize = totalSize % blockSize;
    fileHandle.m_file->m_blockStats.m_totalCnt += ((remainSize == 0) ? fullBlockNum : (fullBlockNum + 1));
    DBGLOG("decompose %s size %llu blocks %d", fileHandle.m_file->m_fileName.c_str(),
        totalSize, fileHandle.m_file->m_blockStats.m_totalCnt.load());
 
    if (m_backupParams.commonParams.writeSparseFile && fileHandle.m_file->m_blockStats.m_totalCnt == 0) {
        DBGLOG("push pure sparse file to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
        return;
    }
    for (uint64_t i = 0; i < fullBlockNum; i++) {
        fileHandle.m_block.m_size = blockSize;
        fileHandle.m_block.m_offset = startOffset + blockSize * i;
        fileHandle.m_block.m_seq = startSeq + i + 1;
        DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
    }
    if (remainSize != 0) {
        fileHandle.m_block.m_size = remainSize;
        fileHandle.m_block.m_offset = startOffset + blockSize * fullBlockNum;
        fileHandle.m_block.m_seq = startSeq + fullBlockNum + 1;
        DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
    }
 
    startSeq = fileHandle.m_block.m_seq;
    DBGLOG("finish decompose %s offset %llu size %llu", fileHandle.m_file->m_fileName.c_str(), startOffset, totalSize);
    return;
}
 
void HostHardlinkReader::DecomposeAndPush(FileHandle& fileHandle) const
{
    uint64_t startSeq = 0;
    if (m_backupParams.commonParams.writeSparseFile && !fileHandle.m_file->m_sparse.empty()) {
        DBGLOG("decompose sparse file %s sparse number %llu",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_sparse.size());
        for (const std::pair<uint64_t, uint64_t>& range : fileHandle.m_file->m_sparse) {
            DBGLOG("sparse info offset %llu length %llu", range.first, range.second);
            DecomposeAndPush(fileHandle, range.first, range.second, startSeq);
        }
    } else {
        DecomposeAndPush(fileHandle, 0, fileHandle.m_file->m_size, startSeq);
    }
    return;
}

void HostHardlinkReader::HandleFailedLink(FileHandle& fileHandle)
{
    // 读失败， 将hardlinkmap里同inode的fileHandle都置为失败， 并添加到failedList
    ERRLOG("Enter Handle Failed Link: %s", fileHandle.m_file->m_fileName.c_str());
    vector<FileHandle> links =  m_hardlinkMap->GetLinksAndClear(fileHandle.m_file->m_inode);
    for (FileHandle& fh : links) {
        DBGLOG("set hardlink to fail, %s", fh.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
    }
    m_hardlinkMap->InsertFailedInode(fileHandle.m_file->m_inode);
}