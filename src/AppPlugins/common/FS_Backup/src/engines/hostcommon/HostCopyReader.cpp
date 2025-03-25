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
#include "HostCopyReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "OsPlatformDefines.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int OOM_SLEEP_SECOND = 1;
    const uint32_t INVALID_MEMORY = static_cast<uint32_t>(-1);
    const int RETRY_TIME_MILLISENCOND = 1000;
}

HostCopyReader::HostCopyReader(
    const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder)
    : ReaderBase(copyReaderParams)
{
    INFOLOG("Construct HostCopyReader!");
    m_srcAdvParams = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.srcAdvParams);
    m_params.srcRootPath = m_srcAdvParams->dataPath;
    m_params.srcTrimPrefix = m_backupParams.commonParams.trimReaderPrefix;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.writeSparseFile = m_backupParams.commonParams.writeSparseFile;
    m_params.discardReadError = m_backupParams.commonParams.discardReadError;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_params.maxBlockNum = m_backupParams.commonParams.maxBlockNum;
    m_params.adsProcessType = m_backupParams.commonParams.adsProcessType;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_copyReader";
    m_failureRecorder = failureRecorder;
}

HostCopyReader::~HostCopyReader()
{
    //  WARN! ALL thread that calls subclass functions MUST BE joined in the destructor of SUBCLASS!
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct HostCopyReader, destroy thread pool %s", m_threadPoolKey.c_str());
    FSBackupUtils::MemoryTrim();
}

BackupRetCode HostCopyReader::Start()
{
    INFOLOG("Start %sCopyReader, create thread pool %s size %d",
        OS_PLATFORM_NAME.c_str(), m_threadPoolKey.c_str(), m_srcAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_srcAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&HostCopyReader::ThreadFunc, this);
        m_pollThread = std::thread(&HostCopyReader::PollReadTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostCopyReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode HostCopyReader::Destroy()
{
    if (!m_threadDone) {
        ERRLOG("ThreadFunc didn't finish! Check if latency is too big or HostCopyReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

bool HostCopyReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

bool HostCopyReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyReader check is complete: controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskproduce %llu consume %llu) (noOfFilesRead %llu noOfDirRead %llu noOfFilesReadFailed %llu "
            "skipFileCnt %llu skipDirCnt %llu unaggregatedFiles %llu emptyFiles %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu totalDir %llu unarchiveFiles %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load());
    }
    if (m_controlInfo->m_controlReaderPhaseComplete &&
        m_readQueue->Empty() &&
        (m_timer.GetCount() == 0) &&
        (m_controlInfo->m_readTaskProduce == m_controlInfo->m_readTaskConsume) &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt + m_controlInfo -> m_noOfFilesWriteSkip +
        m_controlInfo->m_unaggregatedFiles + m_controlInfo->m_emptyFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_controlInfo->m_unarchiveFiles))) {
        INFOLOG("CopyReader complete: controlReaderComplete %d readQueueSize %llu timerSize %llu "
            "(readTaskproduce %llu consume %llu) (noOfFilesRead %llu noOfDirRead %llu noOfFilesReadFailed %llu "
            "skipFileCnt %llu skipDirCnt %llu unaggregatedFiles %llu emptyFiles %llu m_unaggregatedFaildFiles %llu) "
            "(totalFiles %llu totalDir %llu unarchiveFiles %llu)",
            m_controlInfo->m_controlReaderPhaseComplete.load(),
            m_readQueue->GetSize(), m_timer.GetCount(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_skipFileCnt.load(), m_controlInfo->m_skipDirCnt.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

int HostCopyReader::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::OPEN_SRC, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    m_srcOpenedHandleSet.insert(fileHandle.m_file);
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int HostCopyReader::ReadSymlinkData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s symlink file", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_file->m_size + 1];
    fileHandle.m_block.m_size = fileHandle.m_file->m_size;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    DBGLOG("total blocks: %d, file size: %llu",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);

    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int HostCopyReader::ReadHugeObjectData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s file", fileHandle.m_file->m_fileName.c_str());
    if (fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
        ERRLOG("This huge object file (key: %s, name: %s) should not be aggrated.",
            fileHandle.m_file->m_obsKey.c_str(), fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }

    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    DBGLOG("total blocks: %d, file size: %llu",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);

    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto taskptr = make_shared<OsPlatformServiceTask>(
        HostEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(taskptr, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }

    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int HostCopyReader::ReadNormalData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s block info: %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
    FileDescState state = fileHandle.m_file->GetDstState();
    if (state == FileDescState::WRITE_SKIP || state == FileDescState::END) {
        FileDescState srcState = fileHandle.m_file->GetSrcState();
        DBGLOG("Write skip file! %s", fileHandle.m_file->m_fileName.c_str());
        if (srcState != FileDescState::SRC_CLOSED) {
            WARNLOG("Write skip file , close. %s", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
            CloseFile(fileHandle);
        }
        return SUCCESS;
    }

    fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;

    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

bool HostCopyReader::WriteFailedAndSkipRead(FileHandle& fileHandle)
{
    if (fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
        return false;
    }
    // File is writed failed, so needn't to read and push to writer queue
    DBGLOG("%s is writed failed, needn't to read data", fileHandle.m_file->m_fileName.c_str());
    ++fileHandle.m_file->m_blockStats.m_readReqCnt;
    if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readReqCnt ||
        fileHandle.m_file->m_size == 0) {
        fileHandle.m_file->SetSrcState(FileDescState::READED);
        PushToReader(fileHandle); // push to reader queue to close handle
    }
    return true;
}

int HostCopyReader::ReadData(FileHandle& fileHandle)
{
    // symlink
    if (FSBackupUtils::IsSymLinkFile(fileHandle)) {
        return ReadSymlinkData(fileHandle);
    }

    if ((fileHandle.m_file->m_size == 0) ||
        (m_backupParams.commonParams.writeSparseFile && fileHandle.m_block.m_size == 0)) {
        return ReadEmptyData(fileHandle);
    }

    return ReadNormalData(fileHandle);
}

int HostCopyReader::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Enter CloseFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<OsPlatformServiceTask>(
        HostEvent::CLOSE_SRC, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        ERRLOG("put close src task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    m_srcOpenedHandleSet.erase(fileHandle.m_file);
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

void HostCopyReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sCopyReader ThreadFunc thread", OS_PLATFORM_NAME.c_str());
    while (true) {
        if (IsComplete()) {
            INFOLOG("Complete HostCopyReader ThreadFunc thread");
            m_threadDone = true;
            return;
        }
        if (IsAbort()) {
            INFOLOG("Abort HostCopyReader ThreadFunc thread");
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
    INFOLOG("Finish HostCopyReader ThreadFunc thread");
    m_threadDone = true;
    return;
}

void HostCopyReader::PollReadTask()
{
    std::shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start %sCopyReader PollReadTask thread", OS_PLATFORM_NAME.c_str());

    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            m_pollThreadDone = true;
            break;
        }
        if (m_controlInfo->m_readPhaseComplete) {
            INFOLOG("Finish HostCopyReader PollReadTask thread");
            m_pollThreadDone = true;
            return;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            std::shared_ptr<OsPlatformServiceTask> task = dynamic_pointer_cast<OsPlatformServiceTask>(threadPoolRes);
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
    INFOLOG("Finish HostCopyReader PollReadTask thread");
    m_pollThreadDone = true;
    return;
}

int64_t HostCopyReader::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fileHandle : fileHandles) {
        DBGLOG("Process timer %s", fileHandle.m_file->m_fileName.c_str());
        ProcessReadEntries(fileHandle);
    }
    return delay;
}

void HostCopyReader::BlockReadQueuePop() const
{
    uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
    if ((m_srcAdvParams->maxMemory != INVALID_MEMORY) && (totalBufferSize > m_srcAdvParams->maxMemory)) {
        m_readQueue->BlockPop();
        m_blockBufferMap->Print();
    } else {
        m_readQueue->CancelBlockPop();
    }
}

void HostCopyReader::PushToAggregator(FileHandle& fileHandle)
{
    DBGLOG("push to aggregateQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
    fileHandle.m_retryCnt = 0;
    m_aggregateQueue->WaitAndPush(fileHandle);
    ++m_controlInfo->m_readProduce;
    return;
}

void HostCopyReader::DecomposeAndPush(
    FileHandle& fileHandle, uint64_t startOffset, uint64_t totalSize, uint64_t& startSeqCnt) const
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
    }

    for (uint64_t i = 0; i < fullBlockNum; i++) {
        fileHandle.m_block.m_size = blockSize;
        fileHandle.m_block.m_offset = startOffset + blockSize * i;
        fileHandle.m_block.m_seq = startSeqCnt + i + 1;
        DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
    }

    if (remainSize != 0) {
        fileHandle.m_block.m_size = remainSize;
        fileHandle.m_block.m_offset = startOffset + blockSize * fullBlockNum;
        fileHandle.m_block.m_seq = startSeqCnt + fullBlockNum + 1;
        DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
            fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
        m_readQueue->Push(fileHandle);
    }

    startSeqCnt = fileHandle.m_block.m_seq;
    DBGLOG("finish decompose %s offset %llu size %llu", fileHandle.m_file->m_fileName.c_str(), startOffset, totalSize);
    return;
}

void HostCopyReader::DecomposeAndPush(FileHandle& fileHandle) const
{
    uint64_t startSeqCnt = 0;
    if (m_backupParams.commonParams.writeSparseFile && !fileHandle.m_file->m_sparse.empty()) {
        DBGLOG("decompose sparse file %s sparse number %llu",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_sparse.size());
        for (auto &info : fileHandle.m_file->m_sparse) {
            DBGLOG("sparse info offset %llu length %llu", info.first, info.second);
            DecomposeAndPush(fileHandle, info.first, info.second, startSeqCnt);
        }
    } else {
        DecomposeAndPush(fileHandle, 0, fileHandle.m_file->m_size, startSeqCnt);
    }

    return;
}

void HostCopyReader::PushToReader(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("push to reader %s state %u blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(), state,
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
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
        if (FSBackupUtils::IsSymLinkFile(fileHandle)) {
            DBGLOG("push to readQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
            m_readQueue->Push(fileHandle);
            return;
        }
        DecomposeAndPush(fileHandle);
    }
}

bool HostCopyReader::ProcessReadEntriesScannerMode(FileHandle& fileHandle)
{
    // 恢复对于仅修改元数据的文件和目录不进行优化
    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        FSBackupUtils::IsHandleMetaModified(fileHandle.m_file->m_scannermode,
        m_backupParams.commonParams.backupDataFormat)) {
        // if status is meta_modified, no need to read data
        ++m_controlInfo->m_noOfFilesRead;
        PushToAggregator(fileHandle);
        return true;
    }
    return false;
}

void HostCopyReader::CloseOpenedHandle()
{
    INFOLOG("Default Implementation!");
}