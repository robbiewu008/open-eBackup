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
#include "ArchiveCopyReader.h"
#include "ThreadPoolFactory.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#ifdef WIN32
#include "Win32BackupEngineUtils.h"
#endif

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int READ_QUEUE_TIMEOUT = 200;
    const int ARCHIVE_CLINET_POOL_SIZE = 1;
    const int RETRY_TIME_MILLISENCOND = 1000;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const uint8_t FILEDESC_IS_DIR = 1;
}

ArchiveCopyReader::ArchiveCopyReader(const ReaderParams &copyReaderParams) : ReaderBase(copyReaderParams)
{
    INFOLOG("Construct ArchiveCopyReader!");
    m_srcAdvParams = dynamic_pointer_cast<ArchiveRestoreAdvanceParams>(m_backupParams.srcAdvParams);
    m_params.srcRootPath = m_srcAdvParams->dataPath;
    m_params.backupDataFormat = m_backupParams.commonParams.backupDataFormat;
    m_params.restoreReplacePolicy = m_backupParams.commonParams.restoreReplacePolicy;
    m_params.backupType = m_backupParams.backupType;
    m_threadPoolKey = m_backupParams.commonParams.subJobId + "_archiveCopyReader";
}

ArchiveCopyReader::~ArchiveCopyReader()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct ArchiveCopyReader, destroy thread pool %s", m_threadPoolKey.c_str());
}

void ArchiveCopyReader::SetArchiveClient(std::shared_ptr<ArchiveClientBase> client)
{
    m_archiveClient = client;
}

BackupRetCode ArchiveCopyReader::Start()
{
    INFOLOG("Start ArchiveCopyReader, create thread pool %s %d", m_threadPoolKey.c_str(), ARCHIVE_CLINET_POOL_SIZE);
    m_jsPtr = make_shared<JobScheduler>(
        *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, ARCHIVE_CLINET_POOL_SIZE));
    if (m_jsPtr == nullptr) {
        ERRLOG("jsptr is nullptr");
        return BackupRetCode::FAILED;
    }
    try {
        m_thread = std::thread(&ArchiveCopyReader::ThreadFunc, this);
        m_pollThread = std::thread(&ArchiveCopyReader::PollReadTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    } catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveCopyReader::Abort()
{
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveCopyReader::Destroy()
{
    INFOLOG("ArchiveCopyReader Destroy!");
    if (!m_threadDone) {
        ERRLOG("Thread func didn't finish! Check if latency is too big or ArchiveCopyReader hasn't started!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    if (!m_pollThreadDone) {
        ERRLOG("PollThread didn't finish! Check if latency is too big!");
        return BackupRetCode::DESTROY_FAILED_BACKUP_IN_PROGRESS;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode ArchiveCopyReader::Enqueue(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return BackupRetCode::SUCCESS;
}

bool ArchiveCopyReader::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("ArchiveCopyReader controlReaderPhaseComplete %d, readQueueSize %llu "
            "(readedFiles %llu, readedDir %llu, readFailedFiles %llu, "
            "skipFileCnt %llu, skipDirCnt %llu, emptyFiles %llu, unaggregatedFiles %llu, unaggregatedFaildFiles %llu) "
            "(totalFiles %llu, totalDir %llu, unarchiveFiles %llu)  +++totalbuffersize:%llu"
            "readTaskProduce: %llu, readTaskConsume: %llu",
            m_controlInfo->m_controlReaderPhaseComplete.load(),  m_readQueue->GetSize(),
            m_controlInfo->m_noOfFilesRead.load(), m_controlInfo->m_noOfDirRead.load(),
            m_controlInfo->m_noOfFilesReadFailed.load(), m_controlInfo->m_skipFileCnt.load(),
            m_controlInfo->m_skipDirCnt.load(), m_controlInfo->m_emptyFiles.load(),
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesToBackup.load(), m_controlInfo->m_noOfDirToBackup.load(),
            m_controlInfo->m_unarchiveFiles.load(), m_blockBufferMap->GetTotalBufferSize(),
            m_controlInfo->m_readTaskProduce.load(), m_controlInfo->m_readTaskConsume.load());
    }
    if ((m_controlInfo->m_controlReaderPhaseComplete) &&
        ((m_controlInfo->m_noOfFilesRead + m_controlInfo->m_noOfDirRead + m_controlInfo->m_noOfFilesReadFailed +
        m_controlInfo->m_skipFileCnt + m_controlInfo->m_skipDirCnt + m_controlInfo->m_emptyFiles +
        m_controlInfo->m_unaggregatedFiles + m_controlInfo->m_unaggregatedFaildFiles) ==
        (m_controlInfo->m_noOfFilesToBackup + m_controlInfo->m_noOfDirToBackup + m_controlInfo->m_unarchiveFiles)) &&
        (m_controlInfo->m_readTaskProduce == m_controlInfo->m_readTaskConsume)) {
        INFOLOG("Complete ArchiveCopyReader");
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

int ArchiveCopyReader::OpenFile(FileHandle& fileHandle)
{
    DBGLOG("Enter OpenFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto openTask = make_shared<ArchiveServiceTask>(
        ArchiveEvent::OPEN_SRC, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
    if (!(m_jsPtr->Put(openTask, true, TIME_LIMIT_OF_PUT_TASK))) {
        ERRLOG("put open file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int ArchiveCopyReader::ReadMeta(FileHandle& fileHandle)
{
    fileHandle = fileHandle;
    return SUCCESS;
}

int ArchiveCopyReader::CloseFile(FileHandle& fileHandle)
{
    DBGLOG("Enter CloseFile: %s", fileHandle.m_file->m_fileName.c_str());
    auto task = make_shared<ArchiveServiceTask>(
        ArchiveEvent::CLOSE_SRC, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
    if (!m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK)) {
        ERRLOG("put close file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int ArchiveCopyReader::ReadData(FileHandle& fileHandle)
{
    if (FSBackupUtils::IsSymLinkFile(fileHandle)) {
        return ReadSymlinkData(fileHandle);
    }

    if (fileHandle.m_file->m_size == 0) {
        return ReadEmptyData(fileHandle);
    }
 
    return ReadNormalData(fileHandle);
}

int ArchiveCopyReader::ReadSymlinkData(FileHandle& fileHandle)
{
#ifdef WIN32
    DBGLOG("win32 not need to read symlink data: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_file->SetSrcState(FileDescState::READED);
    m_controlInfo->m_noOfFilesRead++;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1; // in case of being dropped in IsOpenBlock
    PushFileHandleToAggregator(fileHandle);
    return SUCCESS;
#else
    DBGLOG("Enter ArchiveCopyReader ReadData: %s symlink file", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_buffer = new uint8_t[SYMLINK_BLOCK_SIZE];
    fileHandle.m_block.m_size = SYMLINK_BLOCK_SIZE;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    DBGLOG("ReadSymlinkData file size: %llu, total blocks: %d",
        fileHandle.m_file->m_size, fileHandle.m_file->m_blockStats.m_totalCnt.load());

    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
    if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
        ERRLOG("ReadSymlinkData task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("ReadSymlinkData total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
#endif
}

int ArchiveCopyReader::ReadEmptyData(FileHandle& fileHandle)
{
    DBGLOG("Enter ArchiveCopyReader ReadEmptyData: %s", fileHandle.m_file->m_fileName.c_str());
    fileHandle.m_block.m_size = 0;
    fileHandle.m_block.m_offset = 0;
    fileHandle.m_block.m_seq = 1;
    fileHandle.m_file->m_blockStats.m_totalCnt = 1;
    DBGLOG("ReadEmpty size[%llu], offset[%llu], total blocks[%d]",
        fileHandle.m_block.m_size, fileHandle.m_block.m_offset, fileHandle.m_block.m_seq,
        fileHandle.m_file->m_blockStats.m_totalCnt.load());

    m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
    auto task = make_shared<ArchiveServiceTask>(
        ArchiveEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
    if (!m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK)) {
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
        ERRLOG("ReadEmptyData put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
        m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
        return FAILED;
    }
    ++m_controlInfo->m_readTaskProduce;
    DBGLOG("ReadEmptyData total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int ArchiveCopyReader::ReadNormalData(FileHandle& fileHandle)
{
    DBGLOG("Enter ArchiveCopyReader ReadNormalData: %s", fileHandle.m_file->m_fileName.c_str());
    uint32_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t fileSize = fileHandle.m_file->m_size;
    uint64_t fullBlockNum = fileSize / blockSize;
    uint32_t remainSize = fileSize % blockSize;

    fileHandle.m_file->m_blockStats.m_totalCnt = (remainSize == 0) ? fullBlockNum : fullBlockNum + 1;
    DBGLOG("ReadNormalData total blocks: %u, file size: %lu",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);
    DBGLOG("ReadNormalData blockSize[%u], fileSize[%lu], fullBlockNum[%lu], remainSize[%u]",
        blockSize, fileSize, fullBlockNum, remainSize);
    for (uint64_t i = 0; i < fullBlockNum; i++) {
        fileHandle.m_block.m_size = blockSize;
        fileHandle.m_block.m_offset = blockSize * i;
        fileHandle.m_block.m_seq = i + 1;
        fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
        DBGLOG("ReadNormalData size[%u] offset[%llu], seq[%llu]",
            fileHandle.m_block.m_size, fileHandle.m_block.m_offset, fileHandle.m_block.m_seq);
        m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
        auto task = make_shared<ArchiveServiceTask>(
            ArchiveEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
        if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
            ERRLOG("ReadNormalData put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
            m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
            return FAILED;
        }
        ++m_controlInfo->m_readTaskProduce;
    }

    if (remainSize != 0) {
        fileHandle.m_block.m_size = remainSize;
        fileHandle.m_block.m_offset = blockSize * fullBlockNum;
        fileHandle.m_block.m_seq = fullBlockNum + 1;
        fileHandle.m_block.m_buffer = new uint8_t[fileHandle.m_block.m_size];
        DBGLOG("ReadNormalData size[%lu] offset[%lu], seq[%lu]",
            fileHandle.m_block.m_size, fileHandle.m_block.m_offset, fileHandle.m_block.m_seq);
        m_blockBufferMap->Add(fileHandle.m_file->m_fileName, fileHandle);
        auto task = make_shared<ArchiveServiceTask>(
            ArchiveEvent::READ_DATA, m_blockBufferMap, fileHandle, m_params, m_archiveClient);
        if (m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK) == false) {
            m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
            ERRLOG("ReadNormalData put read file task %s failed", fileHandle.m_file->m_fileName.c_str());
            m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
            return FAILED;
        }
        ++m_controlInfo->m_readTaskProduce;
    }
    DBGLOG("ReadNormalData total readTask produce for now: %d", m_controlInfo->m_readTaskProduce.load());
    return SUCCESS;
}

int64_t ArchiveCopyReader::ProcessTimers()
{
    DBGLOG("Enter ProcessTimers");
    vector<FileHandle> fileHandles;
    int64_t delay = m_timer.GetExpiredEventAndTime(fileHandles);
    for (FileHandle& fh : fileHandles) {
        DBGLOG("Process timer %s", fh.m_file->m_fileName.c_str());
        ProcessReadEntries(fh);
    }
    return delay;
}

void ArchiveCopyReader::ProcessReadEntries(FileHandle& fileHandle)
{
    FileDescState state = fileHandle.m_file->GetSrcState();
    DBGLOG("ArchiveCopyReader ProcessReadEntries: %s, state: %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(state));
    if (fileHandle.m_file->IsFlagSet(IS_DIR)) {
        ++m_controlInfo->m_noOfDirRead;
        PushFileHandleToAggregator(fileHandle);
        return;
    }

    if (state == FileDescState::INIT) {
        OpenFile(fileHandle);
        return;
    }
    if (state == FileDescState::SRC_OPENED || state == FileDescState::PARTIAL_READED) {
        ReadData(fileHandle);
        return;
    }
    if ((state == FileDescState::READED) || (state == FileDescState::META_READED)) {
        CloseFile(fileHandle);
        return;
    }
    // this will be executed only for aggregated restore or need to some optimized approach
    if (state == FileDescState::AGGREGATED) {
        DBGLOG("ArchiveCopyReader aggregated file Name: %s , file size: %u",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);
        ++m_controlInfo->m_readProduce;
        m_aggregateQueue->WaitAndPush(fileHandle);
        return;
    }

    m_controlInfo->m_noOfFilesReadFailed++;
    WARNLOG("No state to handle: %s, state: %d", fileHandle.m_file->m_fileName.c_str(), static_cast<int>(state));
    return;
}

/* check if we can take ThreadFunc into base class */
void ArchiveCopyReader::ThreadFunc()
{
    INFOLOG("ArchiveCopyReader main thread start!");
    while (true) {
        if (IsAbort()) {
            WARNLOG("ArchiveCopyReader main thread abort!");
            m_threadDone = true;
            return;
        }
        BlockReadQueuePop();
        int64_t delay = ProcessTimers();
        FileHandle fileHandle;
        DBGLOG("ThreadFunc WaitAndPop queue size: %d", m_readQueue->GetSize());
        bool ret = m_readQueue->WaitAndPop(
            fileHandle, delay < QUEUE_TIMEOUT_MILLISECOND ? delay : QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            ProcessReadEntries(fileHandle);
        }
        // must judge IsComplete after generate task
        if (IsComplete()) {
            DBGLOG("Read phase is complete!");
            m_threadDone = true;
            return;
        }
    }
    INFOLOG("ArchiveCopyReader main thread end!");
    m_threadDone = true;
    return;
}

void ArchiveCopyReader::BlockReadQueuePop() const
{
    uint64_t totalBufferSize = m_blockBufferMap->GetTotalBufferSize();
    uint32_t maxMemory = MEMORY_THRESHOLD_HIGH;
    if (totalBufferSize > maxMemory) {
        m_readQueue->BlockPop();
    } else {
        m_readQueue->CancelBlockPop();
    }
}

bool ArchiveCopyReader::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlReaderFailed %d",
            m_abort, m_controlInfo->m_failed.load(), m_controlInfo->m_controlReaderFailed.load());
        m_controlInfo->m_readPhaseComplete = true;
        return true;
    }
    return false;
}

void ArchiveCopyReader::PollReadTask()
{
    INFOLOG("ArchiveCopyReader PollReadTask");
    shared_ptr<ExecutableItem> threadPoolRes;

    while (true) {
        if (m_controlInfo == nullptr) {
            ERRLOG("m_controlInfo nullptr");
            break;
        }
        if (m_controlInfo->m_readPhaseComplete) {
            INFOLOG("Finish ArchiveCopyReader PollReadTask thread");
            m_pollThreadDone = true;
            return;
        }
        if (m_jsPtr == nullptr) {
            ERRLOG("m_jsPtr nullptr");
            break;
        }
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            std::shared_ptr<ArchiveServiceTask> task = dynamic_pointer_cast<ArchiveServiceTask>(threadPoolRes);
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

    INFOLOG("ArchiveCopyReader PollReadTask end!");
    m_pollThreadDone = true;
    return;
}

void ArchiveCopyReader::HandleSuccessEvent(shared_ptr<ArchiveServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    DBGLOG("Enter HandleSuccessEvent: %s, copy status: %d, event: %d",
        fileHandle.m_file->m_fileName.c_str(),
        static_cast<int>(fileHandle.m_file->GetSrcState()), static_cast<int>(taskPtr->m_event));

    if (taskPtr->m_event == ArchiveEvent::CLOSE_SRC) {
        m_controlInfo->m_noOfFilesRead++;
        DBGLOG("Readed Files for now : %d", m_controlInfo->m_noOfFilesRead.load());
    }

    if (taskPtr->m_event == ArchiveEvent::OPEN_SRC) {
        fileHandle.m_file->SetSrcState(FileDescState::SRC_OPENED); // INIT -> SRC_OPENED
        DBGLOG("Put SRC_OPENED file to read queue %s", fileHandle.m_file->m_fileName.c_str());
        m_readQueue->Push(fileHandle);
        // push to aggregate to write to open dst
        PushFileHandleToAggregator(fileHandle);
    }

    // continue PARTIAL_READED after first READ_DATA
    if (taskPtr->m_event == ArchiveEvent::READ_DATA) {
        DBGLOG("Handle READ_DATA %s", fileHandle.m_file->m_fileName.c_str());
        fileHandle.m_file->SetSrcState(FileDescState::PARTIAL_READED); // READ_DATA -> PARTIAL_READED
        fileHandle.m_file->m_blockStats.m_readReqCnt++;
        // get whole file
        if (fileHandle.m_file->m_blockStats.m_totalCnt == fileHandle.m_file->m_blockStats.m_readReqCnt ||
            fileHandle.m_file->m_size == 0) {
            fileHandle.m_file->SetSrcState(FileDescState::READED); // PARTIAL_READED|READ_DATA -> READED
            m_readQueue->Push(fileHandle);
            DBGLOG("File is readed: %s, total: %d, readcnt: %d, size: %d", fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_blockStats.m_readReqCnt.load(),
                fileHandle.m_file->m_size);
        }
        DBGLOG("Put PARTIAL_READED file to read queue %s", fileHandle.m_file->m_fileName.c_str());
        PushFileHandleToAggregator(fileHandle);
    }
}

void ArchiveCopyReader::HandleFailedEvent(shared_ptr<ArchiveServiceTask> taskPtr)
{
    FileHandle fileHandle = taskPtr->m_fileHandle;
    fileHandle.m_retryCnt++;
    ERRLOG("Enter HandleFailedEvent %s %d %d",
        fileHandle.m_file->m_fileName.c_str(), static_cast<int>(taskPtr->m_event), fileHandle.m_retryCnt);
    if (taskPtr->m_result != 0) {
        ERRLOG("Problem with Product Env. Timeout!!");
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        m_controlInfo->m_failed = true;
        fileHandle.m_errNum = taskPtr->m_result;
        m_failedList.emplace_back(fileHandle);
        return;
    }
    if (fileHandle.m_retryCnt >= DEFAULT_ERROR_SINGLE_FILE_CNT) {
        fileHandle.m_file->SetSrcState(FileDescState::READ_FAILED);
        ++m_controlInfo->m_noOfFilesReadFailed;
        if (!fileHandle.m_file->IsFlagSet(IS_DIR)) {
            m_controlInfo->m_noOfFilesFailed++;
        } else {
            m_controlInfo->m_noOfDirFailed++;
        }
        m_blockBufferMap->Delete(fileHandle.m_file->m_fileName, fileHandle);
        ERRLOG("copy read failed for file %s %llu, totalFailed: %llu",
            fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_noOfFilesReadFailed.load(),
            m_controlInfo->m_noOfFilesFailed.load());
        return;
    }
    m_timer.Insert(fileHandle, fileHandle.m_retryCnt * RETRY_TIME_MILLISENCOND);
}

void ArchiveCopyReader::PushFileHandleToAggregator(FileHandle& fileHandle)
{
    DBGLOG("ArchiveCopyReader push to aggregate, %s", fileHandle.m_file->m_fileName.c_str());
    ++m_controlInfo->m_readProduce;
    m_aggregateQueue->WaitAndPush(fileHandle);
    return;
}