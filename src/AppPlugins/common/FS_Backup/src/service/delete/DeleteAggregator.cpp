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
#include "DeleteAggregator.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "backup_layout/SqliteOps.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
}

DeleteAggregator::DeleteAggregator(const AggregatorParams& aggregatorParams)
    : m_backupParams(aggregatorParams.backupParams), m_aggregateQueue(aggregatorParams.aggregateQueuePtr),
    m_writeQueue(aggregatorParams.writeQueuePtr), m_controlInfo(aggregatorParams.controlInfo)
{
    m_fileAggregator = make_unique<FileAggregator>(aggregatorParams.backupParams, aggregatorParams.writeQueuePtr,
        aggregatorParams.readQueuePtr, aggregatorParams.blockBufferMap, aggregatorParams.controlInfo);
}

DeleteAggregator::DeleteAggregator(
    BackupParams& backupParams, std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
    std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr, std::shared_ptr<BackupControlInfo> controlInfo)
    : m_backupParams(backupParams),
      m_aggregateQueue(aggregateQueuePtr),
      m_writeQueue(writeQueuePtr),
      m_controlInfo(controlInfo)
{
    INFOLOG("DeleteAggregator constructor");
}

DeleteAggregator::~DeleteAggregator()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
}

/* Public APIs */
BackupRetCode DeleteAggregator::Start()
{
    INFOLOG("DeleteAggregator start!");
    try {
        m_thread = std::thread(&DeleteAggregator::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode DeleteAggregator::Abort()
{
    WARNLOG("DeleteAggregator abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus DeleteAggregator::GetStatus()
{
    return FSBackupUtils::GetAggregateStatus(m_controlInfo, m_abort);
}

bool DeleteAggregator::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlFileReaderFailed %d", m_abort, m_controlInfo->m_failed.load(),
            m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool DeleteAggregator::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DeleteAggregator check is complete: "
            "readProduce %llu aggrConsume %llu aggrProduce %llu readPhaseComplete %d aggrQueueSize %llu",
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_aggregateProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_aggregateQueue->GetSize());
    }
    if ((m_controlInfo->m_readProduce.load() == m_controlInfo->m_aggregateConsume.load()) &&
        m_controlInfo->m_readPhaseComplete &&
        m_aggregateQueue->Empty()) {
        INFOLOG("DeleteAggregator complete: "
            "readProduce %llu aggrConsume %llu aggrProduce %llu readPhaseComplete %d aggrQueueSize %llu",
            m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_aggregateConsume.load(),
            m_controlInfo->m_aggregateProduce.load(),
            m_controlInfo->m_readPhaseComplete.load(),
            m_aggregateQueue->GetSize());
        return true;
    }

    return false;
}

/* Private methods */
void DeleteAggregator::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("DeleteAggregator main thread start!");
    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        FileHandle fileHandle;
        while (!m_aggregateQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort() || IsComplete()) {
                m_controlInfo->m_aggregatePhaseComplete = true;
                INFOLOG("DeleteAggregator main thread end!");
                return;
            }
        }
        DBGLOG("aggregate get file from read queue, %s", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_aggregateConsume;

        if ((m_fileAggregator != nullptr) && (m_fileAggregator->DeleteSqliteRecord(fileHandle) != Module::SUCCESS)) {
            ERRLOG("Delete sqlite record failed. file name: %s", fileHandle.m_file->m_fileName.c_str());
        }

        // do aggregate here
        while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            if (IsAbort()) {
                m_controlInfo->m_aggregatePhaseComplete = true;
                INFOLOG("DeleteAggregator main thread end!");
                return;
            }
        }
        DBGLOG("aggregate put file to write queue, %s", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_aggregateProduce;
    }
    m_controlInfo->m_aggregatePhaseComplete = true;
    INFOLOG("DeleteAggregator main thread end!");
    return;
}