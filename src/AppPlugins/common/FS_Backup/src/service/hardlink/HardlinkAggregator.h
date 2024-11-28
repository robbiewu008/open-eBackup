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
#ifndef HARDLINK_AGGREGATOR_H
#define HARDLINK_AGGREGATOR_H

#include <memory>
#include <thread>
#include <unordered_map>
#include "Backup.h"
#include "BackupStructs.h"
#include "BackupQueue.h"
#include "FileAggregateTask.h"
#include "ThreadPoolFactory.h"
#include "Snowflake.h"
#include "CommonServiceParams.h"

class AggregateHardlinkDirInfo {
public:
    AggregateHardlinkDirInfo()
    {}
    ~AggregateHardlinkDirInfo()
    {}

    std::string m_dirName {};          // folder name
    uint32_t m_totalHardlinkFilesCount { 0 };     // total files in this dir
    uint32_t m_readCompletedFilesCount { 0 };  // read hardlink files completed size

    std::shared_ptr<std::vector<FileHandle>> m_hardlinkFiles{};
};

class AggregateHardlinkDirMap {
public:
    void Insert(const std::string& dirPath, std::shared_ptr<AggregateHardlinkDirInfo> dirInfo)
    {
        DBGLOG("Insert: %s", dirPath.c_str());
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateHardlinkInfoMap.emplace(make_pair(dirPath, dirInfo));
        return;
    }

    void Erase(const std::string& dirPath)
    {
        DBGLOG("Erase: %s", dirPath.c_str());
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateHardlinkInfoMap.find(dirPath);
        if (it != m_aggregateHardlinkInfoMap.end()) {
            m_aggregateHardlinkInfoMap.erase(dirPath);
        }
        return;
    }

    bool Exist(const std::string& dirPath) const
    {
        std::lock_guard<std::mutex> lk(m_mtx);
        auto it = m_aggregateHardlinkInfoMap.find(dirPath);
        if (it == m_aggregateHardlinkInfoMap.end()) {
            return false;
        }
        return true;
    }

    std::shared_ptr<AggregateHardlinkDirInfo> GetDirInfo(const std::string& dirPath) const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateHardlinkInfoMap.find(dirPath);
        if (it != m_aggregateHardlinkInfoMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateHardlinkInfoMap.clear();
    }

    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_aggregateHardlinkInfoMap.empty();
    }

private:
    mutable std::mutex m_mtx {};
    std::unordered_map<std::string, std::shared_ptr<AggregateHardlinkDirInfo>> m_aggregateHardlinkInfoMap;
};

class HardlinkAggregator {
public:
    explicit HardlinkAggregator(const AggregatorParams& aggregatorParams);
    HardlinkAggregator(
        BackupParams& backupParams,
        std::shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<HardLinkMap> hardlinkMap);
    ~HardlinkAggregator();

    virtual BackupRetCode Start();
    virtual BackupRetCode Abort();
    virtual BackupPhaseStatus GetStatus();
    bool IsAbort();
    bool IsComplete();

private:
    void ThreadFunc();
    void PushToWriteQueue(FileHandle &fileHandle);
    void CreateSqliteIndexTask(std::shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo);
    void PollHardlinkAggregateTask();
    void HandleSuccessEvent(std::shared_ptr<SqliteTask> taskPtr);
    void HandleFailureEvent(std::shared_ptr<SqliteTask> taskPtr);
    void HandleHardlinkAggregate(FileHandle &fileHandle);
    void CheckAndCreateAggregateTask(const std::string &dirName);
    bool IsBlobListEmpty() const;
    bool CanAcceptMoreWork() const;
    void PrintSqliteTaskDistribution() const;
    void ExecuteSqliteTasks();

private:
    BackupParams m_backupParams;

    std::thread m_thread;
    std::shared_ptr<BackupQueue<FileHandle>> m_aggregateQueue { nullptr };
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue { nullptr };

    std::shared_ptr<BackupControlInfo> m_controlInfo { nullptr };
    std::shared_ptr<HardLinkMap> m_hardlinkMap { nullptr };

private:
    bool m_abort { false };
    std::string m_threadPoolKey;
    std::vector<std::shared_ptr<Module::JobScheduler>> m_jsPtr;
    std::shared_ptr<SQLiteDB> m_sqliteDb { nullptr };
    std::string m_sqliteDBRootPath;
    std::string m_sqliteDBAliasPath;
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };
    std::atomic<uint64_t> m_aggTaskProduce { 0 };
    std::atomic<uint64_t> m_aggTaskConsume { 0 };

    AggregateHardlinkDirMap m_aggregateHardlinkMap;
    time_t m_isCompleteTimer { 0 };
    std::vector<std::shared_ptr<BlobFileList>> m_blobFileList;
};

#endif // HARDLINK_AGGREGATOR_H