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
#ifndef DELETE_AGGREGATOR_H
#define DELETE_AGGREGATOR_H

#include <memory>
#include <thread>
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "CommonServiceParams.h"
#include "FileAggregator.h"

class DeleteAggregator {
public:
    explicit DeleteAggregator(const AggregatorParams& aggregatorParams);
    DeleteAggregator(
        BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo);
    ~DeleteAggregator();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupPhaseStatus GetStatus();
    bool IsAbort();
    bool IsComplete();

private:
    void ThreadFunc();

private:
    BackupParams m_backupParams;
    std::unique_ptr<FileAggregator> m_fileAggregator = nullptr;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;

    std::shared_ptr<BackupControlInfo> m_controlInfo;

private:
    bool m_abort { false };
    time_t m_isCompleteTimer { 0 };
};

#endif // DELETE_AGGREGATOR_H