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
#ifndef FILE_AGGREGATOR_H
#define FILE_AGGREGATOR_H

#include <string>
#include <vector>
#include <memory>
#include <unordered_map>
#include "system/System.hpp"
#include "ThreadPoolFactory.h"
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupQueue.h"
#include "backup_layout/SqliteOps.h"
#include "BlockBufferMap.h"
#include "Snowflake.h"
#include "FileAggregateTask.h"

constexpr uint8_t     IS_READ_COMPLETE             = 0x01;          /* Read Completed */
constexpr uint8_t     NEED_PUSH_BLOB_FH            = 0x02;          /* Need push blog fh */
constexpr uint8_t     IS_BLOB_READ_FAILED          = 0x04;          /* Reading blob file failed */

class AggregateNormalInfo {
public:
    FileHandle aggregatedfileHandle;                      // blob file handle
    std::shared_ptr<std::vector<FileHandle>> normalFileHandles; // normal files file hanldes related to aggregated file
    uint32_t   numOfNormalFiles;                          // num of normal files from db
    uint32_t   aggFileSize;
    uint32_t   numOfFilesRestored;                        // num of files restored
    uint32_t   emptyCnt;

    void SetFlag(uint8_t flagBit)
    {
        m_flag |= flagBit;
    }

    void ClearFlag(uint8_t flagBit)
    {
        m_flag &= ~(flagBit);
    }

    bool IsFlagSet(uint8_t flagBit) const
    {
        return (m_flag & flagBit);
    }

private:
    uint8_t m_flag { 0 };
};

class AggregateFileMap {
public:
    void Insert(std::string key, std::shared_ptr<AggregateNormalInfo> info)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateFileInfoMap.emplace(make_pair(key, info));
        return;
    }

    void Erase(std::string key)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateFileInfoMap.find(key);
        if (it != m_aggregateFileInfoMap.end()) {
            m_aggregateFileInfoMap.erase(key);
        }
        return;
    }

    std::shared_ptr<AggregateNormalInfo> GetInfo(std::string key)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateFileInfoMap.find(key);
        if (it != m_aggregateFileInfoMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateFileInfoMap.clear();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_aggregateFileInfoMap.empty();
    }

    size_t Size()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_aggregateFileInfoMap.size();
    }

    std::shared_ptr<AggregateNormalInfo> GetNPop()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::shared_ptr<AggregateNormalInfo> output ;
        std::unordered_map<std::string, std::shared_ptr<AggregateNormalInfo>>::iterator it =
            m_aggregateFileInfoMap.begin();
        if (it != m_aggregateFileInfoMap.end()) {
            // Accessing the key
            std::string key = it->first;
            // Accessing the value
            output = it->second;
            m_aggregateFileInfoMap.erase(key);
            return output;
        }
        return nullptr;
    }

public:
    std::mutex m_mtx {};
    std::unordered_map<std::string, std::shared_ptr<AggregateNormalInfo>> m_aggregateFileInfoMap;
};

class AggRestoreInfo {
public:
    std::string blobFileName;
    uint32_t normalFileOffset;
    uint32_t normalFileSize;
};

class NormalFileMap {
public:
    void Insert(std::string key, std::shared_ptr<AggRestoreInfo> info)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_normalFileMap.emplace(make_pair(key, info));
        return;
    }

    void Erase(std::string key)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_normalFileMap.find(key);
        if (it != m_normalFileMap.end()) {
            m_normalFileMap.erase(key);
        }
        return;
    }

    std::shared_ptr<AggRestoreInfo> GetInfo(std::string key)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_normalFileMap.find(key);
        if (it != m_normalFileMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    bool Exists(std::string key)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_normalFileMap.find(key);
        if (it != m_normalFileMap.end()) {
            return true;
        }
        return false;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_normalFileMap.clear();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_normalFileMap.empty();
    }

    size_t Size()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_normalFileMap.size();
    }

    std::shared_ptr<AggRestoreInfo> GetNPop()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        std::shared_ptr<AggRestoreInfo> output ;
        std::unordered_map<std::string, std::shared_ptr<AggRestoreInfo>>::iterator it =
            m_normalFileMap.begin();
        if (it != m_normalFileMap.end()) {
            // Accessing the key
            std::string key = it->first;
            // Accessing the value
            output = it->second;
            m_normalFileMap.erase(key);
            return output;
        }
        return nullptr;
    }

public:
    std::mutex m_mtx {};
    std::unordered_map<std::string, std::shared_ptr<AggRestoreInfo>> m_normalFileMap;
};

class AggregateDirInfo {
public:
    AggregateDirInfo()
    {}
    ~AggregateDirInfo()
    {
        DBGLOG("~AggregateDirInfo(). dirName: %s", m_dirName.c_str());
    }

    std::string m_dirName {};          // folder name
    uint32_t m_totalFilesCount { 0 };     // total files in this dir
    uint32_t m_readCompletedFilesCount { 0 };  // read completed size
    uint32_t m_readFailedFilesCount { 0 };     // read failed files during backup

    uint32_t m_archiveFileSize { 0 };
    uint32_t m_archiveBufferCount { 0 };    // 当前目录待聚合的buffer数量
    std::shared_ptr<std::vector<FileHandle>> m_aggregatedFiles { nullptr };
    std::shared_ptr<std::vector<FileHandle>> m_nonAggregatedFiles { nullptr };

    // restore related
    uint32_t m_genFilesCount = 0;     // generated files (blob files) in this dir
    // actual file name : AggRestoreInfo. used not to read same blob file for multiple normal files
    NormalFileMap m_InfoMap;
    // aggregatefilename: AggregateNormalInfo;
    AggregateFileMap m_aggFileMap;
};

class AggregateDirMap {
public:
    void Insert(std::string dirPath, std::shared_ptr<AggregateDirInfo> dirInfo)
    {
        DBGLOG("Insert: %s", dirPath.c_str());
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateDirInfoMap.emplace(make_pair(dirPath, dirInfo));
        return;
    }

    void Erase(std::string dirPath)
    {
        DBGLOG("Erase: %s", dirPath.c_str());
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateDirInfoMap.find(dirPath);
        if (it != m_aggregateDirInfoMap.end()) {
            m_aggregateDirInfoMap.erase(dirPath);
        }
        return;
    }

    std::shared_ptr<AggregateDirInfo> GetDirInfo(std::string dirPath)
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        auto it = m_aggregateDirInfoMap.find(dirPath);
        if (it != m_aggregateDirInfoMap.end()) {
            return it->second;
        }
        return nullptr;
    }

    std::vector<std::shared_ptr<AggregateDirInfo>> GetBiggerSizeDir(uint32_t averageSize)
    {
        INFOLOG("Enter GetBiggerSizeDir: %u", averageSize);
        std::lock_guard<std::mutex> lock(m_mtx);
        std::vector<std::shared_ptr<AggregateDirInfo>> res {};
        for (auto it = m_aggregateDirInfoMap.begin(); it != m_aggregateDirInfoMap.end(); it++) {
            DBGLOG("check dir %s, %u", it->first.c_str(), it->second->m_archiveFileSize);
            if (it->second->m_archiveFileSize >= averageSize) {
                DBGLOG("Get bigger size dir : %s", it->first.c_str());
                res.push_back(it->second);
            }
        }
        INFOLOG("GetBifferSizeDir: %u", res.size());
        return res;
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        m_aggregateDirInfoMap.clear();
    }

    bool Empty()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_aggregateDirInfoMap.empty();
    }

    uint64_t GetSize()
    {
        std::lock_guard<std::mutex> lock(m_mtx);
        return m_aggregateDirInfoMap.size();
    }

public:
    std::mutex m_mtx {};
    std::unordered_map<std::string, std::shared_ptr<AggregateDirInfo>> m_aggregateDirInfoMap;
    uint32_t m_aggregatorFileSizeTotal { 0 };        // total memory usage for aggregator
    uint32_t m_aggregatorBufferCountTotal { 0 };     // 聚合中总的buffer数
};

class FileAggregator {
public:
    FileAggregator(const BackupParams& backupParams,
                    std::shared_ptr<BackupQueue<FileHandle>> writeQueue,
                    std::shared_ptr<BackupQueue<FileHandle>> readQueue,
                    std::shared_ptr<BlockBufferMap> blockBufferMap,
                    std::shared_ptr<BackupControlInfo> controlInfo);
    ~FileAggregator();

private:
    BackupParams m_backupParams;
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue;
    std::shared_ptr<BackupQueue<FileHandle>> m_readQueue;
    std::shared_ptr<BlockBufferMap> m_blockBufferMap;
    std::shared_ptr<BackupControlInfo> m_controlInfo;
    std::shared_ptr<SQLiteDB> m_sqliteDb;
    std::string m_sqliteDBRootPath;
    std::string m_sqliteDBAliasPath;
    std::shared_ptr<Module::Snowflake> m_idGenerator;

    uint32_t m_maxFileSizeToAggregate { 0 };
    uint32_t m_maxAggregateFileSize { 0 };

    std::string m_threadPoolKey;
    std::string m_sqlThreadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr;
    std::vector<std::shared_ptr<Module::JobScheduler>> m_jsSqlPtr;
    bool m_started { false };
    bool m_abort { false };
    bool m_isAggregateFileSetProcessed { false };
    std::vector<std::shared_ptr<BlobFileList>> m_blobFileList;

public:
    std::atomic<uint64_t> m_aggTaskProduce { 0 };
    std::atomic<uint64_t> m_aggTaskConsume { 0 };
    std::atomic<uint64_t> m_sqliteTaskProduce  { 0 };
    std::atomic<uint64_t> m_sqliteTaskConsume  { 0 };
    AggregateDirMap m_aggregateDirMap;

    BackupRetCode Start();
    BackupRetCode Abort();
    void HandleComplete();
    void Aggregate(FileHandle &fh);
    void PollAggregateTask();
    void PollSqlAggregateTask();
    void CheckMemory();
    void ExecuteSqliteTasks();
    bool IsBlobListEmpty();
    void PrintSqliteTaskDistribution();
    uint32_t CanAcceptMoreWork();
    bool IsAggregateStarted() const;
    int DeleteSqliteRecord(FileHandle& fileHandle);

private:
    bool IsAbort() const;
    void PushToWriteQueue(FileHandle &fileHandle) const;
    void CheckAndCreateAggregateTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo);
    void ProcessDirectory(FileHandle &fileHandle);
    void ProcessAggBackup(FileHandle &fileHandle);
    void ProcessFile(FileHandle &fileHandle);
    void ProcessEmptyFileReadCompletion(FileHandle &fileHandle);
    void ProcessReadFailedFiles(FileHandle &fileHandle);
    void ProcessAggrFileReadCompletion(FileHandle &fileHandle);
    void ProcessNonAggrFileRead(FileHandle &fileHandle);
    void CreateAggregateTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo);
    void CreateSqliteIndexTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo) const;
    void CreateSqliteIndexTaskForEmptyDir(const std::string &dirName);
    void CreateSqliteIndexTaskForDir(FileHandle &fileHandle) const;
    void CreateSqliteIndexForAllParentDirs(std::string parentDir) const;
    void HandleAggrFileSet() const;
    void HandleSuccessEvent(std::shared_ptr<FileAggregateTask> taskPtr) const;
    void HandleFailureEvent(std::shared_ptr<FileAggregateTask> taskPtr);
    void SqlHandleSuccessEvent(std::shared_ptr<SqliteTask> taskPtr) const;
    void SqlHandleFailureEvent(std::shared_ptr<SqliteTask> taskPtr) const;
    void HandleCreateSqliteIndexFailed(std::shared_ptr<FileAggregateTask> taskPtr);
    bool IsAggrHang() const;
    void CheckReaderBlock();

    /* Restore related functions */
    void ProcessAggRestore(FileHandle &fileHandle);
    void ProcessAggRestoreInteraction(FileHandle &fileHandle);
    void ProcessAggRestoreNonAggrFile(FileHandle &fileHandle);
    void ProcessAggRestoreNormalFile(FileHandle &fileHandle);
    void ProcessAggRestoreBlobFile(FileHandle &blobFileHandle);
    void ProcessAggRestoreEmptyFile(FileHandle &fileHandle);
    void ProcessAggRestoreReadFailedBlobFile(FileHandle &fileHandle);
    void AddNormalnGenFileToAggregateFileMap(FileHandle &fileHandle, FileHandle &blobfileHandle,
            std::shared_ptr<AggregateDirInfo> aggregateDirInfo, uint32_t numOfNormalFiles);
    void PushToReadQueue(FileHandle &fileHandle);
    void FillTheInfoMap(std::vector<AggSqlRestoreInfo>& vecNormalFiles,
        std::shared_ptr<AggregateDirInfo> aggregateDirInfo, const std::string &aggFileName, uint32_t &numOfNormalFiles);
    void CreateUnAggregateTask(std::shared_ptr<AggregateNormalInfo> aggFileInfo,
            std::shared_ptr<AggregateDirInfo> aggregateDirInfo, bool isAllFilesRcvd);
    void CleanUpTheDirMap(std::shared_ptr<AggregateDirInfo> aggregateDirInfo);
    void FillAndPushToReadQueue(FileHandle &fileHandle, const std::string aggfileName, uint64_t aggFileSize,
        std::shared_ptr<AggregateDirInfo> aggDirInfo, uint32_t numOfNormalFiles);
    void NormalFileInFileMap(std::string blobFileName, FileHandle &fileHandle,
        std::shared_ptr<AggregateDirInfo> aggregateDirInfo);
    void CleanTheBlobFile(std::shared_ptr<AggregateNormalInfo> info) const;
    void FillAndPushToReadQueueOnly(FileHandle &fileHandle, const std::string aggfileName,
        std::shared_ptr<AggregateNormalInfo> aggFileInfo, std::shared_ptr<AggregateDirInfo> aggDirInfo);
    bool GetBlobFileHandleWithSingleBuffer(FileHandle &oldBlobFileHandle, FileHandle &newBlobFileHandle);
    void CreateTaskForSqliteIndex(std::shared_ptr<std::vector<FileHandle>> fileHandleList,
        const std::string &dirName, std::string &archiveFileName, uint64_t fileSize) const;
    std::string GetDbFile(std::string dirPath);
    int GetNormalFilesFromMultiSqliteByName(std::string& dirPath, std::string& fileName,
        AggSqlRestoreQueryInfo& info, std::vector<AggSqlRestoreInfo>& vecNormalFiles);
    int GetRecordFromMultiSqliteByName(FileHandle &fileHandle, std::string& dbFile,
        std::vector<IndexDetails>& vecIndexInfo);
    int GetFileMetaByName(FileHandle &fileHandle);
};

#endif
