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
#include "CopyAggregator.h"
#include "log/Log.h"
#include "common/Thread.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
}

CopyAggregator::CopyAggregator(const AggregatorParams& aggregatorParams)
    : m_backupParams(aggregatorParams.backupParams),
    m_aggregateQueue(aggregatorParams.aggregateQueuePtr),
    m_writeQueue(aggregatorParams.writeQueuePtr),
    m_readQueue(aggregatorParams.readQueuePtr),
    m_controlInfo(aggregatorParams.controlInfo),
    m_blockBufferMap(aggregatorParams.blockBufferMap)
{
    m_fileAggregator = make_unique<FileAggregator>(
        aggregatorParams.backupParams,
        aggregatorParams.writeQueuePtr,
        aggregatorParams.readQueuePtr,
        aggregatorParams.blockBufferMap,
        aggregatorParams.controlInfo);
}

CopyAggregator::CopyAggregator(
    BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<BlockBufferMap> blockBufferMap)
    : m_backupParams(backupParams),
      m_aggregateQueue(aggregateQueuePtr),
      m_writeQueue(writeQueuePtr),
      m_readQueue(readQueuePtr),
      m_controlInfo(controlInfo),
      m_blockBufferMap(blockBufferMap)
{
    m_fileAggregator = make_unique<FileAggregator>(backupParams, writeQueuePtr,
        readQueuePtr, blockBufferMap, controlInfo);
}

CopyAggregator::~CopyAggregator()
{
    INFOLOG("Aggregate call destory");
    if (m_thread.joinable()) {
        m_thread.join();
    }
    FSBackupUtils::MemoryTrim();
}

bool CopyAggregator::IsAggregate() const
{
    if (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) {
        return true;
    }

    if (FSBackupUtils::OnlyGenerateSqlite(m_backupParams.commonParams.genSqlite)) {
        return true;
    }

    return false;
}

/* Public APIs */
BackupRetCode CopyAggregator::Start()
{
    INFOLOG("CopyAggregator start!");
    try {
        m_thread = std::thread(&CopyAggregator::ThreadFunc, this);
        if (IsAggregate()) {
            m_sqlPollthread = std::thread(&CopyAggregator::ThreadFuncSqlTask, this);
        }
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }

    if (m_fileAggregator->Start() != BackupRetCode::SUCCESS) {
        ERRLOG("Start fileAggregator failed");
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode CopyAggregator::Abort()
{
    INFOLOG("Abort CopyAggregator");
    m_abort = true;
    m_fileAggregator->Abort();
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus CopyAggregator::GetStatus()
{
    return FSBackupUtils::GetAggregateStatus(m_controlInfo, m_abort);
}

bool CopyAggregator::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlFileReaderFailed %d", m_abort, m_controlInfo->m_failed.load(),
            m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool CopyAggregator::IsComplete()
{
    if (m_controlInfo->m_controlReaderFailed) {
        ERRLOG("control reader failed, exit dir reader!");
        return true;
    }
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyAggregator check complete : aggregateConsume - %d, copyReadProduce - %d, readPhaseComplete - %d, "
                "aggTaskProduce - %d, aggTaskConsume - %d, aggregateQueueEmpty - %d, aggregateQueueSize - %u, "
                "BlockBufferCount: %llu, BlockBufferSize: %llu, AggregatorFileSize: %u AggregatorFileCount: %u",
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_fileAggregator->m_aggTaskProduce.load(),
            m_fileAggregator->m_aggTaskConsume.load(),
            m_aggregateQueue->Empty(),
            m_aggregateQueue->GetSizeWithOutLock(),
            m_blockBufferMap->m_blockBufferCount.load(),
            m_blockBufferMap->m_blockBufferSize.load(),
            m_fileAggregator->m_aggregateDirMap.m_aggregatorFileSizeTotal,
            m_fileAggregator->m_aggregateDirMap.m_aggregatorBufferCountTotal);
        FSBackupUtils::MemoryTrim();
    }
    if ((m_controlInfo->m_aggregateConsume == m_controlInfo->m_readProduce) &&
        (m_fileAggregator->m_aggTaskProduce == m_fileAggregator->m_aggTaskConsume) &&
        m_controlInfo->m_readPhaseComplete &&
        m_aggregateQueue->Empty()) {
        INFOLOG("CopyAggregator complete: aggregateConsume %llu copyReadProduce %llu readPhaseComplete %d"
                " aggTaskProduce %llu aggTaskConsume %llu aggregateQueueSize %llu",
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_fileAggregator->m_aggTaskProduce.load(),
            m_fileAggregator->m_aggTaskConsume.load(),
            m_aggregateQueue->GetSize());
        INFOLOG("BlockBufferCount: %llu, BlockBufferSize: %llu",
            m_blockBufferMap->m_blockBufferCount.load(), m_blockBufferMap->m_blockBufferSize.load());
        return true;
    }

    return false;
}

bool CopyAggregator::IsCompleteSqlTask()
{
    if (m_controlInfo->m_controlReaderFailed) {
        ERRLOG("control reader failed, exit sqlite poll thread!");
        return true;
    }
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteSqlThreadTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteSqlThreadTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("CopyAggregator sqlite poll check complete : aggregateConsume - %d, copyReadProduce - %d,"
            "readPhaseComplete - %d, sqlTaskProduce - %llu, sqlTaskConsume - %llu, aggregateQueueEmpty - %d,"
            "aggregateQueueSize - %u BlockBufferCount: %llu, BlockBufferSize: %llu, IsBlobListEmpty %d",
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_fileAggregator->m_sqliteTaskProduce.load(),
            m_fileAggregator->m_sqliteTaskConsume.load(),
            m_aggregateQueue->Empty(),
            m_aggregateQueue->GetSizeWithOutLock(),
            m_blockBufferMap->m_blockBufferCount.load(),
            m_blockBufferMap->m_blockBufferSize.load(),
            m_fileAggregator->IsBlobListEmpty());
    }
    if ((m_controlInfo->m_aggregateConsume == m_controlInfo->m_readProduce) &&
        (m_fileAggregator->m_sqliteTaskProduce == m_fileAggregator->m_sqliteTaskConsume) &&
        (m_fileAggregator->m_aggTaskProduce == m_fileAggregator->m_aggTaskConsume) &&
        m_controlInfo->m_readPhaseComplete &&
        m_aggregateQueue->Empty() &&
        m_fileAggregator->IsBlobListEmpty()) {
        INFOLOG("CopyAggregator sqlite poll complete: aggregateConsume %llu copyReadProduce %llu readPhaseComplete %d"
                " sqlTaskProduce %llu sqlTaskConsume %llu aggregateQueueSize %llu IsBlobListEmpty %d",
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_fileAggregator->m_sqliteTaskProduce.load(),
            m_fileAggregator->m_sqliteTaskConsume.load(),
            m_aggregateQueue->GetSize(),
            m_fileAggregator->IsBlobListEmpty());

        if ((m_backupParams.backupType == BackupType::BACKUP_FULL) ||
            (m_backupParams.backupType == BackupType::BACKUP_INC)) {
            m_fileAggregator->PrintSqliteTaskDistribution();
        }
        return true;
    }
    return false;
}

/* Private methods */
void CopyAggregator::PushToWriter(FileHandle &fileHandle)
{
    while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            return;
        }
    }

    ++m_controlInfo->m_aggregateProduce;
    DBGLOG("aggregate put file to write queue, %s, %d", fileHandle.m_file->m_fileName.c_str(),
           m_controlInfo->m_aggregateProduce.load());
}

void CopyAggregator::WaitForAggregatorStart()
{
    while (!m_fileAggregator->IsAggregateStarted()) {
        if (IsAbort()) {
            WARNLOG("CopyAggregator thread abort!");
            return;
        }
        DBGLOG("FileAggregator not started. waiting");
        Module::SleepFor(std::chrono::seconds(1)); /* Sleep for 1 second */
    }
}

void CopyAggregator::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("CopyAggregator main thread start!");

    WaitForAggregatorStart();

    bool popThisCircle = false;

    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }

        if (!IsAggregate() || m_fileAggregator->CanAcceptMoreWork()) {
            FileHandle fileHandle;
            bool ret = m_aggregateQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
            if (ret) {
                popThisCircle = true;

                if (IsAggregate()) {
                    m_fileAggregator->Aggregate(fileHandle);
                } else {
                    PushToWriter(fileHandle);
                }
                ++m_controlInfo->m_aggregateConsume;
            }
        }

        if (!popThisCircle) {
            Module::SleepFor(chrono::milliseconds(1));
        }
        popThisCircle = false;

        if (IsAggregate()) {
            m_fileAggregator->PollAggregateTask();
        }
        /*  if aggrQueue is empty and maxBlockBufferSz is reached */
        if (IsMemoryCheck()) {
            DBGLOG("invocking CheckMemory");
            m_fileAggregator->CheckMemory();
        }
    }

    INFOLOG("FileAggregation tasks completed, waiting for Sqlite Tasks to complete");
    int64_t time1 = FSBackupUtils::GetMilliSecond();
    if (IsAggregate()) {
        if (m_sqlPollthread.joinable()) {
            m_sqlPollthread.join();
        }
    }
    int64_t time2 = FSBackupUtils::GetMilliSecond();
    INFOLOG("Sqlite tasks completed, time taken after FileAggregation completion = %llu", (time2 - time1));

    m_fileAggregator->HandleComplete();
    m_controlInfo->m_aggregatePhaseComplete = true;
    INFOLOG("CopyAggregator main thread end!");
    return;
}

inline bool CopyAggregator::IsMemoryCheck() const
{
    if (!IsAggregate()) {
        return false;
    }
    if ((m_backupParams.backupType != BackupType::RESTORE) &&
        (m_backupParams.backupType != BackupType::FILE_LEVEL_RESTORE)) {
        return false;
    }
    if (!(m_aggregateQueue->Empty())) {
        return false;
    }
    if (!(m_blockBufferMap->m_blockBufferCount >= m_backupParams.commonParams.maxBufferCnt ||
            m_blockBufferMap->m_blockBufferSize >= m_backupParams.commonParams.maxBufferSize)) {
        return false;
    }
    return true;
}

void CopyAggregator::ThreadFuncSqlTask()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("CopyAggregator sql poll task thread start!");

    WaitForAggregatorStart();

    while (true) {
        if (IsAbort() || IsCompleteSqlTask()) {
            break;
        }
        m_fileAggregator->ExecuteSqliteTasks();
        m_fileAggregator->PollSqlAggregateTask();
    }

    INFOLOG("CopyAggregator sql poll task thread end!");
    return;
}
