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
#include "ObjectCopyWriter.h"
#include "ObjectServiceTask.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int RETRY_TIME_MILLISENCOND = 1000;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr int MAX_RETRY_CNT = 20;
    constexpr int RETRY_INTERVAL_TIME = 5;
}

ObjectCopyWriter::ObjectCopyWriter(
    const WriterParams &copyWriterParams,
    std::shared_ptr<Module::BackupFailureRecorder> failureRecorder) : WriterBase(copyWriterParams)
{
    INFOLOG("Construct ObjectCopyWriter!");
    m_dstAdvParams = dynamic_pointer_cast<ObjectBackupAdvanceParams>(m_backupParams.dstAdvParams);
    m_params.writeMeta = m_backupParams.commonParams.writeMeta;
    m_params.writeAcl = m_backupParams.commonParams.writeAcl;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_params.blockSize = m_backupParams.commonParams.blockSize;
    m_params.maxBlockNum = m_backupParams.commonParams.maxBlockNum;
    m_params.m_uploadInfoMap = std::make_shared<UploadInfoMap>();
    m_params.dataPath = m_dstAdvParams->dataPath;
    m_params.cachePath = m_dstAdvParams->cachePath;
    m_params.authArgs = m_dstAdvParams->authArgs;
    m_params.dstBucket = m_dstAdvParams->dstBucket;
    for (auto &item : m_dstAdvParams->buckets) {
        m_params.bucketNames.emplace_back(item.bucketName);
    }
    m_params.writeCbData.controlInfo = m_controlInfo;
    m_params.writeCbData.maxBlockNum = m_backupParams.commonParams.maxBlockNum;
    m_params.isfineGrainedRestore = m_dstAdvParams->isfineGrainedRestore;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_copyWriter";
    m_failureRecorder = failureRecorder;
}

ObjectCopyWriter::~ObjectCopyWriter()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct ObjectCopyWriter, destroy thread pool %s", m_threadPoolKey.c_str());
    FSBackupUtils::MemoryTrim();
}

BackupRetCode ObjectCopyWriter::Start()
{
    INFOLOG("Start and create thread pool %s size %d", m_threadPoolKey.c_str(), m_dstAdvParams->threadNum);
    m_jsPtr = make_shared<JobScheduler>(
            *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, m_dstAdvParams->threadNum));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&ObjectCopyWriter::ThreadFunc, this);
        m_pollThread = std::thread(&ObjectCopyWriter::PollWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectCopyWriter::Abort()
{
    INFOLOG("ObjectCopyWriter Enter Abort");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ObjectCopyWriter::Destroy()
{
    INFOLOG("ObjectCopyWriter Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread Func didn't finish! Check if latency is too big or ObjectCopyWriter hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

void ObjectCopyWriter::PrintControlInfo(std::string head)
{
    INFOLOG("%s: aggrComplete %d writeQueueSize %llu writeCacheSize %llu timerSize %llu "
        "(writeTaskProduce %llu writeTaskConsume %llu) "
        "(writedFiles %llu writedDir %llu aggrFiles %llu "
        "skipFiles %llu skipDir %llu backupFailedFiles %llu backupFailedDir %llu unaggregatedFaildFiles %llu) "
        "(totalFiles %llu totalDirs %llu archiveFiles %llu)", head.c_str(),
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
}

bool ObjectCopyWriter::IsComplete()
{
    /* in write phase, faild item should contain not only write failed but also read failed */
    /* thus, use BackupControlInfo::m_noOfFilesFailed and BackupControlInfo::m_noOfDirFailed */
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        PrintControlInfo("CopyWriter check is complete");
        FSBackupUtils::MemoryTrim();
    }

    if (m_controlInfo->m_aggregatePhaseComplete && m_writeQueue->Empty() && (m_writeCache.size() == 0) &&
        (m_timer.GetCount() == 0) && (m_writeTaskProduce == m_writeTaskConsume) &&
        ((m_controlInfo->m_noOfFilesCopied + m_controlInfo->m_noOfFilesFailed) == m_controlInfo->m_noOfFilesToBackup)) {
        PrintControlInfo("CopyWriter complete");
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }

    return false;
}

bool ObjectCopyWriter::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
                m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_writePhaseComplete = true;
        return true;
    }
    return false;
}

int ObjectCopyWriter::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto taskPtr = make_shared<ObjectServiceTask>(ObjectEvent::OPEN_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(taskPtr) == false) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int ObjectCopyWriter::WriteData(FileHandle& fileHandle)
{
    DBGLOG("Enter WriteData: %s", fileHandle.m_file->m_fileName.c_str());
    m_params.writeCbData.fileName = fileHandle.m_file->m_fileName;
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::WRITE_DATA, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write data task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int ObjectCopyWriter::WriteMeta(FileHandle& fileHandle)
{
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::WRITE_META, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put write meta file task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    return SUCCESS;
}

int ObjectCopyWriter::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Enter CloseFile: %s, DstState: %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(fileHandle.m_file->GetDstState()));
    auto task = make_shared<ObjectServiceTask>(ObjectEvent::CLOSE_DST, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put close task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    DBGLOG("total writeTask produce for now: %d", m_writeTaskProduce.load());
    return SUCCESS;
}

int64_t ObjectCopyWriter::ProcessTimers()
{
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        ProcessWriteEntries(fh);
    }
    return delay;
}

void ObjectCopyWriter::ClearWriteCache(std::string fileName)
{
    if (m_writeCache.empty()) {
        return;
    }

    std::lock_guard<std::mutex> lk(m_cacheMutex);
    for (auto itr = m_writeCache.begin(); itr != m_writeCache.end();) {
        if (!itr->second.empty() &&
            ((!fileName.empty() && (itr->first.find(fileName) == 0)) ||
             (itr->second[0].m_file->GetDstState() != FileDescState::INIT))) {
            for (auto& i : itr->second) {
                m_writeQueue->Push(i);
            }
            itr = m_writeCache.erase(itr);
        } else {
            itr++;
        }
    }
}

void ObjectCopyWriter::InsertWriteCache(FileHandle& fileHandle)
{
    DBGLOG("push to cache %s, blockInfo %llu %llu %u", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size);
    std::lock_guard<std::mutex> lk(m_cacheMutex);
    m_writeCache[fileHandle.m_file->m_fileName].push_back(fileHandle);
}

bool ObjectCopyWriter::IsOpenBlock(const FileHandle& fileHandle)
{
    return ((fileHandle.m_block.m_size == 0) && (fileHandle.m_block.m_seq == 0));
}

bool ObjectCopyWriter::CheckObsServer()
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

/* check if we can take ThreadFunc into base class */
void ObjectCopyWriter::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start ObjectCopyWriter ThreadFunc thread!");
    while (true) {
        if (IsComplete()) {
            INFOLOG("Complete ObjectCopyWriter ThreadFunc thread!");
            break;
        }
        if (IsAbort()) {
            INFOLOG("Abort ObjectCopyWriter ThreadFunc thread!");
            break;
        }
        if (!CheckObsServer()) {
            break;
        }
        int64_t delay = ProcessTimers();
        if (delay > QUEUE_TIMEOUT_MILLISECOND) {
            delay = QUEUE_TIMEOUT_MILLISECOND;
        }
        FileHandle fileHandle;
        bool ret = m_writeQueue->WaitAndPop(fileHandle, delay);
        if (ret) {
            ProcessWriteEntries(fileHandle);
        }
        ClearWriteCache("");
    }
    INFOLOG("Finish ObjectCopyWriter ThreadFunc thread");
    m_threadDone = true;
    return;
}

void ObjectCopyWriter::PollWriteTask()
{
    std::shared_ptr<ExecutableItem> threadPoolRes;
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("Start ObjectCopyWriter PollWriteTask thread");

    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            break;
        }
        if (m_controlInfo->m_writePhaseComplete) {
            INFOLOG("Finish ObjectCopyWriter PollWriteTask thread");
            m_pollThreadDone = true;
            return;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            shared_ptr<ObjectServiceTask> taskPtr = dynamic_pointer_cast<ObjectServiceTask>(threadPoolRes);
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

    INFOLOG("Finish ObjectCopyWriter PollWriteTask thread");
    m_pollThreadDone = true;
    return;
}

void ObjectCopyWriter::CloseWriteFailedHandle(FileHandle& fileHandle)
{
    // Close failed file handle when all handle's block habe been delete from m_blockBufferMap
    if (m_blockBufferMap->Get(fileHandle.m_file->m_fileName) == nullptr) {
        CloseFile(fileHandle);
    }
}

void ObjectCopyWriter::HandleSuccessEvent(std::shared_ptr<ObjectServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    ObjectEvent event = taskPtr->m_event;
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("Object copy writer success %s event %d state %d",
           fileHandle.m_file->m_fileName.c_str(), static_cast<int>(event), (int)state);

    if (event == ObjectEvent::CREATE_DIR) {
        m_createdBucket.emplace(GetDstBucketName(fileHandle.m_file->m_fileName));
        ClearWriteCache(fileHandle.m_file->m_fileName);
    } else if ((event == ObjectEvent::OPEN_DST) && (state != FileDescState::WRITE_SKIP)) {
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        if (fileHandle.m_file->m_size <= m_params.blockSize) {
            m_writeQueue->Push(fileHandle);
        }
    } else if (event == ObjectEvent::WRITE_META || state == FileDescState::WRITE_SKIP) {
        fileHandle.m_file->SetDstState(FileDescState::END);
        m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        if (state == FileDescState::WRITE_SKIP) {
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        }
        DBGLOG("Finish backup file %s total backup file %d",
               fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesCopied.load());
    } else if (event == ObjectEvent::CLOSE_DST) {
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
    } else if (event == ObjectEvent::WRITE_DATA) {
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

void ObjectCopyWriter::HandleFailedEvent(std::shared_ptr<ObjectServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    ObjectEvent event = taskPtr->m_event;
    ++fileHandle.m_retryCnt;

    ERRLOG("Object copy writer failed %s event %d retry cnt %d seq %d", fileHandle.m_file->m_fileName.c_str(),
           static_cast<int>(event), fileHandle.m_retryCnt, fileHandle.m_block.m_seq);

    if (taskPtr->m_errDetails.second == EAGAIN) {
        m_needCheckServer = true;
    }

    FileDescState state = fileHandle.m_file->GetDstState();
    if (state != FileDescState::WRITE_FAILED) {
        /* If state is WRITE_FAILED, needn't retry */
        if (fileHandle.m_retryCnt < DEFAULT_ERROR_SINGLE_FILE_CNT && !taskPtr->IsCriticalError()) {
            m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
            return;
        }
        FSBackupUtils::RecordFailureDetail(m_failureRecorder, taskPtr->m_errDetails);
        // 通过设置公共锁，防止read和write同时失败设置FAILED时导致两边都不计数的问题
        fileHandle.m_file->LockCommonMutex();
        fileHandle.m_file->SetDstState(FileDescState::WRITE_FAILED);
        // failed dirs are collected in the dir phase.
        if (!fileHandle.m_file->IsFlagSet(IS_DIR) &&
            fileHandle.m_file->GetSrcState() != FileDescState::READ_FAILED) {
            // 若read的状态为READ_FAILED时，说明该文件已经被reader记为失败
            m_controlInfo->m_noOfFilesFailed += fileHandle.m_file->m_originalFileCount;
            if (taskPtr->m_errDetails.second == EISDIR) {
                m_controlInfo->m_noOfDirFailed++;
            }
        }
        fileHandle.m_file->UnlockCommonMutex();
        fileHandle.m_errNum = taskPtr->m_errDetails.second;
        m_failedList.emplace_back(fileHandle);
    }
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    if ((event != ObjectEvent::CLOSE_DST) && (event != ObjectEvent::WRITE_META) && !IsSmallFile(fileHandle)) {
        // 1. close操作过了之后不再重复close; 2. 小文件不需要再close;
        CloseWriteFailedHandle(fileHandle);
    }
    if (!m_backupParams.commonParams.skipFailure || taskPtr->IsCriticalError() || (event == ObjectEvent::CREATE_DIR)) {
        ERRLOG("set backup to failed!");
        m_controlInfo->m_failed = true;
        m_controlInfo->m_backupFailReason = taskPtr->m_backupFailReason;
    }
    ERRLOG("copy write failed for file %s, totalFailed: %llu %llu",
           fileHandle.m_file->m_fileName.c_str(),
           m_controlInfo->m_noOfFilesFailed.load(), m_controlInfo->m_noOfDirFailed.load());
    return;
}

std::string ObjectCopyWriter::GetDstBucketName(std::string& path)
{
    if (!m_params.dstBucket.bucketName.empty()) {
        return m_params.dstBucket.bucketName;
    }

    for (auto &item : m_params.bucketNames) {
        // 不包含首字符"/"路径分割符
        if (path.substr(1, item.length()) == item) {
            return item;
        }
    }
    return "";
}

int ObjectCopyWriter::CreateBucket(FileHandle& fileHandle)
{
    std::string dstBucketName = GetDstBucketName(fileHandle.m_file->m_fileName);
    DBGLOG("Create bucket %s", dstBucketName.c_str());
    if (m_createdBucket.find(dstBucketName) != m_createdBucket.end()) {
        DBGLOG("Bucket %s is exist.", dstBucketName.c_str());
        return SUCCESS;
    }

    auto task = make_shared<ObjectServiceTask>(ObjectEvent::CREATE_DIR, m_blockBufferMap, fileHandle, m_params);
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put create bucket task %s failed", fileHandle.m_file->m_fileName.c_str());
        return FAILED;
    }
    ++m_writeTaskProduce;
    return SUCCESS;
}

bool ObjectCopyWriter::IsSmallFile(FileHandle& fileHandle)
{
    return (fileHandle.m_file->m_size <= m_params.blockSize);
}

bool ObjectCopyWriter::IsIgnore(FileHandle& fileHandle)
{
    return IsSmallFile(fileHandle) && IsOpenBlock(fileHandle);
}

void ObjectCopyWriter::ProcessInitState(FileHandle& fileHandle)
{
    if (m_createdBucket.find(GetDstBucketName(fileHandle.m_file->m_fileName)) == m_createdBucket.end()) {
        if (!IsIgnore(fileHandle)) {
            InsertWriteCache(fileHandle);
        }
        return;
    }

    // 是小文件直接写数据, 提前退出，不需要open/close等操作，减少流程，提升性能
    if (fileHandle.m_file->m_size <= m_params.blockSize) {
        // 对于小文件， 意义为非open的block直接丢弃
        if (IsOpenBlock(fileHandle)) {
            return;
        }
        fileHandle.m_file->SetDstState(FileDescState::DST_OPENED);
        WriteData(fileHandle);
        return;
    }

    // 超大文件调用OBS的另外接口单独处理，不需要open/close等操作
    if (fileHandle.m_file->IsFlagSet(HUGE_OBJECT_FILE)) {
        WriteData(fileHandle);
        return;
    }

    if (IsOpenBlock(fileHandle)) {
        OpenFile(fileHandle);
    } else {
        InsertWriteCache(fileHandle);
    }
    return;
}

// Hint:: need to add 'RecordFailureDetail' and unify later
void ObjectCopyWriter::ProcessWriteEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetDstState();

    DBGLOG("process write entry (filename: %s, key: %s) state %d blockInfo %llu %llu %u scannermode %s",
           fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_obsKey.c_str(), (int)state,
           fileHandle.m_block.m_seq, fileHandle.m_block.m_offset, fileHandle.m_block.m_size,
           fileHandle.m_file->m_scannermode.c_str());

    // 对象储存没有目录，元数据中的目录是为了方便文件划分而加的，恢复时，除了桶创建的目录，不需要写
    if (fileHandle.IsDir()) {
        CreateBucket(fileHandle);
        return;
    }

    // 对于meta_modified的文件，在writer的路径只有INIT->WRITEMETA->END
    if ((m_params.backupType == BackupType::BACKUP_FULL || m_params.backupType == BackupType::BACKUP_INC) &&
        FSBackupUtils::IsHandleMetaModified(fileHandle.m_file->m_scannermode,
        m_backupParams.commonParams.backupDataFormat)) {
        WriteMeta(fileHandle);
        return;
    }

    if (state == FileDescState::INIT) {
        ProcessInitState(fileHandle);
    } else if ((state == FileDescState::DST_OPENED) || (state == FileDescState::PARTIAL_WRITED)) {
        WriteData(fileHandle);
    } else if (state == FileDescState::WRITED) {
        CloseFile(fileHandle);
    } else if (state == FileDescState::DST_CLOSED) {
        WriteMeta(fileHandle);
    } else if (state == FileDescState::WRITE_FAILED || state == FileDescState::WRITE_SKIP ||
               state == FileDescState::END) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    }

    return;
}

void ObjectCopyWriter::ProcessWriteData(FileHandle& fileHandle)
{
    fileHandle.m_file->SetDstState(FileDescState::PARTIAL_WRITED);
    ++fileHandle.m_file->m_blockStats.m_writeReqCnt;
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
    m_controlInfo->m_noOfBytesCopied += fileHandle.m_block.m_size;
    DBGLOG("Writed for %s, writeReqCnt: %d, totalCnt: %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->m_blockStats.m_writeReqCnt.load(), fileHandle.m_file->m_blockStats.m_totalCnt.load());
    bool isAllFinish = (fileHandle.m_file->m_blockStats.m_writeReqCnt == fileHandle.m_file->m_blockStats.m_totalCnt);
    if (IsSmallFile(fileHandle) || isAllFinish) { // 小文件或所有的块都写完成了
        DBGLOG("All blocks writed for %s", fileHandle.m_file->m_fileName.c_str());
        if (fileHandle.m_file->IsFlagSet(HUGE_OBJECT_FILE)) {
            // 超大文件不需要close，写完成后直接写元数据
            if (m_params.writeMeta) {
                fileHandle.m_file->SetDstState(FileDescState::DST_CLOSED);
                m_writeQueue->Push(fileHandle);
            } else {
                fileHandle.m_file->SetDstState(FileDescState::END);
                m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
            }
        } else if (fileHandle.m_file->m_size <= m_params.blockSize) {
            // 小文件不需要close，，也不用单独写元数据，在写data时，元数据一起处理了
            fileHandle.m_file->SetDstState(FileDescState::END);
            m_controlInfo->m_noOfFilesCopied += fileHandle.m_file->m_originalFileCount;
        } else {
            fileHandle.m_file->SetDstState(FileDescState::WRITED);
            m_writeQueue->Push(fileHandle);
        }
    }
    return;
}
