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
#include "DirAggregator.h"
#include "log/Log.h"
#include "FSBackupUtils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
}

DirAggregator::DirAggregator(const AggregatorParams& aggregatorParams)
    : m_backupParams(aggregatorParams.backupParams),
    m_aggregateQueue(aggregatorParams.aggregateQueuePtr),
    m_writeQueue(aggregatorParams.writeQueuePtr),
    m_controlInfo(aggregatorParams.controlInfo) {}
    
DirAggregator::DirAggregator(
    BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo)
    : m_backupParams(backupParams),
      m_aggregateQueue(aggregateQueuePtr),
      m_writeQueue(writeQueuePtr),
      m_controlInfo(controlInfo)
{
}

DirAggregator::~DirAggregator()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }

    INFOLOG("Destruct DirAggregator");
}

/* Public APIs */
BackupRetCode DirAggregator::Start()
{
    INFOLOG("Start DirAggregator");

    try {
        m_thread = std::thread(&DirAggregator::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode DirAggregator::Abort()
{
    INFOLOG("DirAggregator abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus DirAggregator::GetStatus()
{
    DBGLOG("Enter DirAggregator GetStatus");
    return FSBackupUtils::GetAggregateStatus(m_controlInfo, m_abort);
}

bool DirAggregator::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlFileReaderFailed %d", m_abort, m_controlInfo->m_failed.load(),
            m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool DirAggregator::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("DirAggregator check is complete : (readPhaseComplete %d) (aggregateConsume %llu copyReadProduce %llu"
            " aggregateProduce %llu) aggregateQueueEmpty %d",
            m_controlInfo->m_readPhaseComplete.load(),
            m_controlInfo->m_aggregateConsume.load(), m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_aggregateProduce.load(), m_aggregateQueue->Empty());
    }
    if (m_controlInfo->m_readPhaseComplete &&
        (m_controlInfo->m_readProduce.load() == m_controlInfo->m_aggregateConsume.load()) &&
        m_aggregateQueue->Empty()) {
        INFOLOG("DirAggregator complete : (readPhaseComplete %d) (aggregateConsume %llu copyReadProduce %llu"
            " aggregateProduce %llu aggregateQueueEmpty %d",
            m_controlInfo->m_readPhaseComplete.load(),
            m_controlInfo->m_aggregateConsume.load(), m_controlInfo->m_readProduce.load(),
            m_controlInfo->m_aggregateProduce.load(), m_aggregateQueue->Empty());
        return true;
    }

    return false;
}

void DirAggregator::PushToWriteQueue(FileHandle &fileHandle)
{
    while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            INFOLOG("DirAggregator main thread end!");
            return;
        }
    }
    DBGLOG("DirAggregator put file to write queue, %s", fileHandle.m_file->m_fileName.c_str());
    ++m_controlInfo->m_aggregateProduce;
}

void DirAggregator::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("DirAggregator main thread start!");

    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        FileHandle fileHandle;
        
        bool ret = m_aggregateQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
        if (ret) {
            DBGLOG("DirAggregator get file from aggregate queue, %s", fileHandle.m_file->m_fileName.c_str());
            ++m_controlInfo->m_aggregateConsume;

            PushToWriteQueue(fileHandle);
        }
    }
    m_controlInfo->m_aggregatePhaseComplete = true;
    INFOLOG("DirAggregator main thread end!");
    return;
}