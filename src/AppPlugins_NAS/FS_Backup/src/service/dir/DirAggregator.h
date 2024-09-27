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
#ifndef DIR_AGGREGATOR_H
#define DIR_AGGREGATOR_H

#include <memory>
#include <thread>
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "ThreadPoolFactory.h"
#include "ThreadPool.h"
#include "backup_layout/SqliteOps.h"
#include "FileAggregateTask.h"
#include "CommonServiceParams.h"

class DirAggregator {
public:
    explicit DirAggregator(const AggregatorParams& aggregatorParams);
    DirAggregator(
        BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo);
    ~DirAggregator();
    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupPhaseStatus GetStatus();
    bool IsAbort();
    bool IsComplete();

private:
    void ThreadFunc();
    void PushToWriteQueue(FileHandle &fileHandle);

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;

    std::shared_ptr<BackupControlInfo> m_controlInfo;

    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };
};

#endif // DIR_AGGREGATOR_H