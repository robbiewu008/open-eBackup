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
#include "ObjectCopyReader.h"
#include "log/Log.h"
#include "BackupConstants.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int RETRY_TIME_MILLISENCOND = 1000;
    constexpr int MAX_RETRY_CNT = 20;
    constexpr int RETRY_INTERVAL_TIME = 5;
    const uint32_t INVALID_MEMORY = static_cast<uint32_t>(-1);
}

ObjectCopyReader::ObjectCopyReader(const ReaderParams &copyReaderParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : ReaderBase(copyReaderParams)
{
    INFOLOG("Construct ObjectCopyReader!");
    m_failureRecorder = failureRecorder;
    m_srcAdvParams = dynamic_pointer_cast<ObjectBackupAdvanceParams>(m_backupParams.srcAdvParams);
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_copyReader";
    m_pktStats = make_shared<PacketStats>();

    m_params.reqID = m_backupParams.commonParams.reqID;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_params.saveMeta = m_srcAdvParams->saveMeta;
    m_params.authArgs = m_srcAdvParams->authArgs;
    m_params.excludeMeta = m_srcAdvParams->excludeMeta;
    for (auto &item : m_srcAdvParams->buckets) {
        m_params.bucketNames.emplace_back(item);
    }
}

ObjectCopyReader::~ObjectCopyReader()
{
    INFOLOG("Destruct ObjectCopyReader!");
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

BackupRetCode ObjectCopyReader::Start()
{
    INFOLOG("Start and create thread pool %s, size %d", m_threadPoolKey.c_str(), m_srcAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_srcAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }

    try {
        m_thread = thread(&ObjectCopyReader::ThreadFunc, this);
        m_pollThread = std::thread(&ObjectCopyReader::PollReadTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectCopyReader::Abort()
{
    INFOLOG("ObjectCopyReader Abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectCopyReader::Destroy()
{
    INFOLOG("ObjectCopyReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or ObjectCopyReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus ObjectCopyReader::GetStatus()
{
    m_pktStats->Print();
    if (!m_controlInfo->m_readPhaseComplete) {
        return BackupPhaseStatus::INPROGRESS;
    }
    if (m_abort) {
        return BackupPhaseStatus::ABORTED;
    }
    if (m_failed || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        return m_failReason;
    }
    return BackupPhaseStatus::COMPLETED;
}

int ObjectCopyReader::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED);
    PushToAggregator(fileHandle); // push to aggregate to write to open dst
    PushToReader(fileHandle); // decompose to blocks and push to aggregate
    return Module::SUCCESS;
}

int ObjectCopyReader::ReadData(FileHandle& fileHandle)
{
    DBGLOG("Enter ReadData: %s block info: %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);

    FileDescState dstState = fileHandle.m_file->GetDstState();
    if (dstState == FileDescState::WRITE_FAILED) {
        // File is writed failed, so needn't to read and push to writer queue
        DBGLOG("%s is writed failed, needn't to read data", fileHandle.m_file->m_fileName.c_str());
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readReqCnt ||
            fileHandle.m_file->m_size == 0) {
            fileHandle.m_file->SetSrcState(FileDescState::READED);
            PushToReader(fileHandle); // push to reader queue to close handle
        }
        return SUCCESS;
    }

    if (dstState == FileDescState::WRITE_SKIP || dstState == FileDescState::END) {
        DBGLOG("Write skip file! %s", fileHandle.m_file->m_fileName.c_str());
        if (fileHandle.m_file->GetSrcState() != FileDescState::SRC_CLOSED) {
            WARNLOG("Write skip file , close. %s", fileHandle.m_file->m_fileName.c_str());
            fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
            CloseFile(fileHandle);
        }
        return SUCCESS;
    }

    // 大小为0，说明是空文件，不需要申请内存
    if (fileHandle.m_block.m_size != 0) {
        fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
        memset_s(fileHandle.m_block.m_buffer, fileHandle.m_block.m_size, 0, fileHandle.m_block.m_size);
    }
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }

    return SUCCESS;
}

// 保存的文件不包含元数据
int ObjectCopyReader::ReadMeta(FileHandle& fileHandle)
{
    DBGLOG("Read meta for file %s", fileHandle.m_file->m_fileName.c_str());
    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::READ_META, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int ObjectCopyReader::CloseFile(FileHandle& fileHandle)
{
    if (fileHandle.m_file->m_blockStats.m_totalErrCnt == 0) {
        ++m_controlInfo->m_noOfFilesRead;
    } else {
        ++m_controlInfo->m_noOfFilesReadFailed;
    }
    DBGLOG("Close file %s, readed Files: %d, readed fail files: %d, ErrBlockCnt: %d",
        fileHandle.m_file->m_fileName.c_str(),
        m_controlInfo->m_noOfFilesRead.load(),
        m_controlInfo->m_noOfFilesReadFailed.load(),
        fileHandle.m_file->m_blockStats.m_totalErrCnt.load());
    return Module::SUCCESS;
}

bool ObjectCopyReader::IsAbort() const
{
    if (m_abort || m_failed || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlInfoFailed %d controlReaderFailed %d",
            m_abort, m_failed, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

void ObjectCopyReader::PrintControlInfo(std::string head)
{
    INFOLOG("%s: controlReaderComplete %d readQueueSize %llu timerSize %llu "
        "(noOfFilesRead %llu noOfDirRead %llu noOfFilesReadFailed %llu "
        "skipFileCnt %llu skipDirCnt %llu unaggregatedFiles %llu emptyFiles %llu m_unaggregatedFaildFiles %llu) "
        "(totalFiles %llu totalDir %llu unarchiveFiles %llu)", head.c_str(),
        m_controlInfo->m_controlReaderPhaseComplete.load(), m_readQueue->GetSize(), m_timer.GetCount(),
        m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
        m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_skipFileCnt.load(),
        m_controlInfo->m_skipDirCnt.load(), m_controlInfo->m_unaggregatedFiles.load(),
        m_controlInfo->m_emptyFiles.load(), m_controlInfo->m_unaggregatedFaildFiles.load(),
        m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
        m_controlInfo->m_unarchiveFiles.load());
}

bool ObjectCopyReader::IsComplete()
{
    if (m_controlInfo->m_controlReaderPhaseComplete && m_readQueue->Empty() && (m_timer.GetCount() == 0) &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt +
        m_controlInfo->m_unaggregatedFiles + m_controlInfo->m_emptyFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_controlInfo->m_unarchiveFiles))) {
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }

    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        PrintControlInfo("CopyReader check is complete");
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        FSBackupUtils::MemoryTrim();
    }
    return false;
}

bool ObjectCopyReader::IsNeedRead(FileHandle& fileHandle)
{
    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        FSBackupUtils::IsHandleMetaModified(fileHandle.m_file->m_scannermode,
        m_backupParams.commonParams.backupDataFormat)) {
        // if status is meta_modified, no need to read data
        DBGLOG("No need read file %s", fileHandle.m_file->m_fileName.c_str());
        return false;
    }

    return true;
}

void ObjectCopyReader::PushToAggregator(FileHandle& fileHandle)
{
    DBGLOG("push to aggregateQueue %s blockInfo %llu %llu %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);

    fileHandle.m_retryCnt = 0;
    m_aggregateQueue->WaitAndPush(fileHandle);
    ++m_controlInfo->m_readProduce;
    return;
}

void ObjectCopyReader::ProcessReadEntries(FileHandle& fileHandle)
{
    std::shared_ptr<FileDesc> fileDesc = fileHandle.m_file;
    FileDescState state = fileDesc->GetSrcState();
    DBGLOG("process read entry (filename: %s, key: %s), state: %d, retryCnt: %d, scannermode: %s",
        fileDesc->m_fileName.c_str(), fileDesc->m_obsKey.c_str(), static_cast<int>(state), fileHandle.m_retryCnt,
        fileHandle.m_file->m_scannermode.c_str());

    if (!IsNeedRead(fileHandle)) {
        // 读数据返回的结果中包含了元数据，但如果没有进行读操作，需要单独下发读元数据
        ReadMeta(fileHandle);
        return;
    }

    // 是小文件直接读数据, 提前结束
    if (fileHandle.m_file->m_size <= m_params.blockSize && state != FileDescState::AGGREGATED) {
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
        ReadData(fileHandle);
    } else if (state == FileDescState::READED) {
        CloseFile(fileHandle);
    } else if (state == FileDescState::READ_FAILED) {
        // 如果有块下载失败，状态会设置为READ_FAILED, 不会进入ReadData进行下载数据。
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt <= fileHandle.m_file->m_blockStats.m_readReqCnt) {
            CloseFile(fileHandle);
        }
    }

    return;
}

int64_t ObjectCopyReader::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fileHandle : fileHandles) {
        DBGLOG("Process timer %s", fileHandle.m_file->m_fileName.c_str());
        ProcessReadEntries(fileHandle);
    }
    return delay;
}

void ObjectCopyReader::HandleComplete()
{
    INFOLOG("Complete ObjectCopyReader");
    m_controlInfo->m_readPhaseComplete = true;
}

void ObjectCopyReader::BlockReadQueuePop() const
{
    if ((m_blockBufferMap->m_blockBufferSize > m_srcAdvParams->maxMemory) ||
        (m_blockBufferMap->m_blockBufferCount > m_backupParams.commonParams.maxBufferCnt)) {
        m_readQueue->BlockPop();
        m_blockBufferMap->Print();
    } else {
        m_readQueue->CancelBlockPop();
    }
}

bool ObjectCopyReader::CheckObsServer()
{
    if (!m_needCheckServer) {
        return true;
    }
    m_needCheckServer = false;
    DBGLOG("Check object server connectivity.");
    auto obsCtx = CloudServiceManager::CreateInst(m_params.authArgs);
    auto checkReq = std::make_unique<CheckConnectRequest>();
    checkReq->retryConfig.retryInterval = RETRY_INTERVAL_TIME;
    checkReq->retryConfig.retryNum = MAX_RETRY_CNT;
    OBSResult result = obsCtx->CheckConnect(checkReq);
    if (!result.IsSucc()) {
        int64_t errorCode = result.GetLinuxErrorCode();
        ERRLOG("Check object server connect failed. errcode: %ld", errorCode);
        m_controlInfo->m_failed = true;
        return false;
    }
    return true;
}

void ObjectCopyReader::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("ObjectCopyReader main thread start!");

    while (true) {
        if (IsComplete()) {
            INFOLOG("Read phase is complete!");
            break;
        }
        if (IsAbort()) {
            INFOLOG("ObjectCopyReader main thread abort!");
            break;
        }
        if (!CheckObsServer()) {
            break;
        }
        BlockReadQueuePop();
        int64_t delay = ProcessTimers();
        if (delay > QUEUE_TIMEOUT_MILLISECOND) {
            delay = QUEUE_TIMEOUT_MILLISECOND;
        }
        FileHandle fileHandle;
        if (m_readQueue->WaitAndPop(fileHandle, delay)) {
            ProcessReadEntries(fileHandle);
        }
    }
    INFOLOG("ObjectCopyReader main thread end!");
    HandleComplete();
    return;
}

void ObjectCopyReader::DecomposeAndPush(
    FileHandle& fileHandle, uint64_t startOffset, uint64_t totalSize, uint64_t& startSeqCnt)
{
    uint32_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t fullBlockNum = totalSize / blockSize;
    uint32_t remainSize = totalSize % blockSize;
    fileHandle.m_file->m_blockStats.m_totalCnt += ((remainSize == 0) ? fullBlockNum : (fullBlockNum + 1));
    DBGLOG("decompose %s size %llu blocks %d", fileHandle.m_file->m_fileName.c_str(),
        totalSize, fileHandle.m_file->m_blockStats.m_totalCnt.load());

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

void ObjectCopyReader::PushToReader(FileHandle& fileHandle)
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
        uint64_t startSeqCnt = 0;
        DecomposeAndPush(fileHandle, 0, fileHandle.m_file->m_size, startSeqCnt);
    }
    return;
}

void ObjectCopyReader::HandleSuccessEvent(std::shared_ptr<ObjectServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    ObjectEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("Object copy reader success %s event %d state %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), static_cast<int>(state));
    // 只读元数据或者读小文件后，直接push给aggregator
    if ((event == ObjectEvent::READ_META) || (fileHandle.m_file->m_size <= m_params.blockSize)) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_CLOSED);
        DBGLOG("Readed files : %s", fileHandle.m_file->m_fileName.c_str());
        CloseFile(fileHandle);
        PushToAggregator(fileHandle);
        return;
    }

    if (event == ObjectEvent::READ_DATA) {
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt <= fileHandle.m_file->m_blockStats.m_readReqCnt ||
            fileHandle.m_file->m_size == 0) {
            fileHandle.m_file->SetSrcState(FileDescState::READED);
            PushToReader(fileHandle); // push to aggregate
        }
        PushToAggregator(fileHandle); // file handle with data block, push to aggregate to write to write data
        return;
    }
    return;
}

bool ObjectCopyReader::HandleFailedEventInner(
    FileHandle &fileHandle, std::shared_ptr<ObjectServiceTask> &taskPtr)
{
    ObjectEvent event = taskPtr->m_event;
    if (fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {  /* If state is READ_FAILED, needn't retry */
        if (fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT && !taskPtr->IsCriticalError()) {
            m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
            return false;
        }
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
        if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            // skipping to the failed cnt inc for zip files in restore which are not used in writetask
            if (!fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE) &&
                fileHandle.m_file->GetDstState() != FileDescState::WRITE_FAILED) {
                // 若write的状态为WRITE_FAILED时，说明该文件已经被writer记为失败
                m_controlInfo->m_noOfFilesFailed++;
            }
        } else {
            m_controlInfo->m_noOfDirFailed++;
        }
        fileHandle.m_file->UnlockCommonMutex();
        fileHandle.m_errNum = taskPtr->m_errDetails.second;
        m_failedList.emplace_back(fileHandle);
    }
    // 最后一个块会执行PushToReader添加到读队列中，用于关闭操作
    if (event == ObjectEvent::READ_DATA) {
        ++fileHandle.m_file->m_blockStats.m_readReqCnt;
        ++fileHandle.m_file->m_blockStats.m_totalErrCnt;
        if (fileHandle.m_file->m_blockStats.m_totalCnt <= fileHandle.m_file->m_blockStats.m_readReqCnt) {
            CloseFile(fileHandle);
            WARNLOG("The last block reader failed %s m_readReqCnt %d, m_totalCnt %d",
                fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_file->m_blockStats.m_readReqCnt.load(),
                fileHandle.m_file->m_blockStats.m_totalCnt.load());
        } else {
            PushToReader(fileHandle);
        }
    }
    return true;
}
void ObjectCopyReader::HandleFailedEvent(std::shared_ptr<ObjectServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    ObjectEvent event = taskPtr->m_event;
    ++fileHandle.m_retryCnt;
    ERRLOG("Object copy reader failed %s, %u, event %d retry cnt %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_size, static_cast<int>(event), fileHandle.m_retryCnt);

    if (taskPtr->m_errDetails.second == EAGAIN) {
        m_needCheckServer = true;
    }

    if (!HandleFailedEventInner(fileHandle, taskPtr)) {
        return;
    }
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError()) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    if (!fileHandle.m_errMessage.empty()) {
        m_failedList.emplace_back(fileHandle);
    }
    // NATIVE format doesn't need push to aggregator.
    if ((m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) ||
        FSBackupUtils::OnlyGenerateSqlite(m_backupParams.commonParams.genSqlite)) {
        PushToAggregator(fileHandle); // file handle so aggregate can handle failurs of reading file
    }
    ERRLOG("copy read failed for file %s, %llu, metaInfo: %u, %llu, totalFailed: %llu, %llu",
        fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->m_metaFileIndex, fileHandle.m_file->m_metaFileOffset,
        m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_noOfDirFailed.load(),
        m_controlInfo->m_noOfFilesFailed.load());
    return;
}

void ObjectCopyReader::PollReadTask()
{
    std::shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start ObjectCopyReader PollReadTask thread");

    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            break;
        }
        if (m_controlInfo->m_readPhaseComplete) {
            INFOLOG("Finish HostCopyReader PollReadTask thread");
            return;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            std::shared_ptr<ObjectServiceTask> task = dynamic_pointer_cast<ObjectServiceTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }
            if (task->m_result == SUCCESS) {
                HandleSuccessEvent(task);
            } else {
                HandleFailedEvent(task);
            }
            DBGLOG("read tasks consume cnt for now");
        }
    }
    INFOLOG("Finish HostCopyReader PollReadTask thread");
    return;
}
