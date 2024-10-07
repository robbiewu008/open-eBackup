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
#ifndef COPY_AGGREGATOR_H
#define COPY_AGGREGATOR_H

#include <memory>
#include <thread>

#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "FSBackupUtils.h"
#include "SqliteOps.h"
#include "FileAggregator.h"
#include "CommonServiceParams.h"

class CopyAggregator {
public:
    explicit CopyAggregator(const AggregatorParams& aggregatorParams);
    CopyAggregator(
        BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> readQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<BlockBufferMap> blockBufferMap);
    ~CopyAggregator();

    BackupRetCode Start();
    BackupRetCode Abort();
    BackupPhaseStatus GetStatus();

private:
    bool IsAbort();
    bool IsComplete();
    bool IsCompleteSqlTask();
    bool HandleComplete();
    void PushToWriter(FileHandle &fileHandle);
    void ThreadFunc();
    void ThreadFuncSqlTask();
    void WaitForAggregatorStart();
    bool IsAggregate() const;
    inline bool IsMemoryCheck() const;

private:
    BackupParams m_backupParams;
    std::unique_ptr<FileAggregator> m_fileAggregator = nullptr;

    std::thread m_thread;
    std::thread m_sqlPollthread;
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;

    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;

private:
    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };
    time_t m_isCompleteSqlThreadTimer { 0 };
};

#endif // COPY_AGGREGATOR_H