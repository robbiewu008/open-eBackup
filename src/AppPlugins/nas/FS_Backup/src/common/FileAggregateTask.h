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
#ifndef FILE_AGGREGATE_TASK_H
#define FILE_AGGREGATE_TASK_H

#include <memory>
#include "system/System.hpp"
#ifndef WIN32
#include <unistd.h>
#endif
#include <fcntl.h>
#include "ThreadPool.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "BlockBufferMap.h"
#include "BackupQueue.h"
#include "backup_layout/SqliteOps.h"
#include "Snowflake.h"
#include "JsonHelper.h"

enum class AggregateEvent {
    AGGREGATE_FILES_AND_CREATE_INDEX = 0,
    UNAGGREGATE_FILES = 1,
    INVALID_EVENT
};

enum class SqliteEvent {
    SQL_TASK_PTR_CREATE_INDEX = 10,
    INVALID_EVENT
};

struct ObsMetaData {
    std::string k;
    std::string v;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(k, k)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(v, v)
    END_SERIAL_MEMEBER
};

struct FileAttr {
    std::vector<ObsMetaData> meta;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(meta, meta)
    END_SERIAL_MEMEBER
};

class SmallFileDesc {
public:
    bool IsFlagSet(uint32_t flagBit) const
    {
        return (m_flag & flagBit);
    }

    uint32_t m_size { 0 };
    uint32_t m_aggregateFileOffset { 0 };
    uint64_t m_ctime { 0 };
    uint64_t m_mtime { 0 };
    std::string m_onlyFileName;
    std::string m_obsKey;
    std::string m_metaData;
    uint32_t m_flag { 0 };
};

class BlobFileDetails {
public:
    std::string m_dirPath;
    std::string m_sqliteDBRootPath;
    std::string archiveFileName;
    std::vector<SmallFileDesc> m_smallFileDescList {};
    std::shared_ptr<SQLiteDB> m_sqliteDb { nullptr };
    uint32_t aggregatedFileSize { 0 };
};

class BlobFileList {
public:
    BlobFileList() {}
    BlobFileList(BlobFileList &rhs)
    {
        m_blobFileDetailsList = rhs.m_blobFileDetailsList;
        m_numOfBlobsFilesInserted = rhs.m_numOfBlobsFilesInserted;
    }

    BlobFileList& operator=(BlobFileList other)
    {
        m_blobFileDetailsList = other.m_blobFileDetailsList;
        m_numOfBlobsFilesInserted = other.m_numOfBlobsFilesInserted;
        return *this;
    }

    std::mutex m_mtx {};
    std::queue<std::shared_ptr<BlobFileDetails>> m_blobFileDetailsList {};
    uint32_t m_numOfBlobsFilesInserted { 0 };
};

class UnaggregatedTaskParms {
public:
    AggregateEvent event;
    BackupParams backupParams;
    std::shared_ptr<std::vector<FileHandle>> fileHandleList;
    std::string dirPath;
    std::shared_ptr<BackupQueue<FileHandle>> writeQueue;
    std::shared_ptr<BlockBufferMap> blockBufferMap;
    std::shared_ptr<BackupControlInfo> controlInfo;
    std::shared_ptr<SQLiteDB> sqliteDb;
    std::string sqliteDBRootPath;
    FileHandle blobfileHandle;
    bool isDeleteBlobFile;
};

class FileAggregateTask : public Module::ExecutableItem {
public:
    FileAggregateTask(AggregateEvent event,
        BackupParams& backupParams,
        std::shared_ptr<std::vector<FileHandle>> fileHandleList, std::string dirPath,
        std::shared_ptr<BackupQueue<FileHandle>> writeQueue,
        std::shared_ptr<BlockBufferMap> blockBufferMap,
        std::shared_ptr<BackupControlInfo> controlInfo,
        std::shared_ptr<SQLiteDB> sqliteDb,
        std::string sqliteDBRootPath,
        std::shared_ptr<Module::Snowflake> idGenerator,
        std::atomic<uint64_t> &sqlTaskProduce,
        std::atomic<uint64_t> &sqlTaskConsume,
        std::vector<std::shared_ptr<BlobFileList>> blobFileList)
        : m_event(event), m_backupParams(backupParams),
          m_inputFileHandleList(fileHandleList), m_dirPath(dirPath), m_writeQueue(writeQueue),
          m_blockBufferMap(blockBufferMap), m_controlInfo(controlInfo), m_sqliteDb(sqliteDb),
          m_sqliteDBRootPath(sqliteDBRootPath), m_idGenerator(idGenerator),
          m_sqliteTaskProduce(&sqlTaskProduce), m_sqliteTaskConsume(&sqlTaskConsume),
          m_blobFileList(blobFileList)
    {
        DBGLOG("FileAggregateTask constructor");
    }

    FileAggregateTask(AggregateEvent event,
        std::shared_ptr<std::vector<FileHandle>> fileHandleList, std::string dirPath,
        std::shared_ptr<SQLiteDB> sqliteDb,
        std::string sqliteDBRootPath,
        std::shared_ptr<Module::Snowflake> idGenerator,
        std::atomic<uint64_t> &sqlTaskProduce,
        std::atomic<uint64_t> &sqlTaskConsume,
        std::vector<std::shared_ptr<BlobFileList>> blobFileList)
        : m_event(event), m_inputFileHandleList(fileHandleList),
          m_dirPath(dirPath), m_sqliteDb(sqliteDb), m_sqliteDBRootPath(sqliteDBRootPath),
          m_idGenerator(idGenerator), m_sqliteTaskProduce(&sqlTaskProduce),
          m_sqliteTaskConsume(&sqlTaskConsume), m_blobFileList(blobFileList)
    {
        DBGLOG("FileAggregateTask constructor");
    }

    explicit FileAggregateTask(UnaggregatedTaskParms &taskInfo)
        : m_event(taskInfo.event), m_backupParams(taskInfo.backupParams),
          m_inputFileHandleList(taskInfo.fileHandleList),
          m_dirPath(taskInfo.dirPath),
          m_writeQueue(taskInfo.writeQueue),
          m_blockBufferMap(taskInfo.blockBufferMap), m_controlInfo(taskInfo.controlInfo),
          m_sqliteDb(taskInfo.sqliteDb),
          m_sqliteDBRootPath(taskInfo.sqliteDBRootPath),
          m_blobfileHandle(taskInfo.blobfileHandle),
          m_isDeleteBlobFile(taskInfo.isDeleteBlobFile)
    {
        DBGLOG("FileAggregateTask constructor-3");
    }
    virtual ~FileAggregateTask() {};
    void Exec() override;

    AggregateEvent m_event { AggregateEvent::INVALID_EVENT };
    BackupParams m_backupParams;
    std::shared_ptr<std::vector<FileHandle>> m_inputFileHandleList {};
    std::vector<FileHandle> m_outputFileHandleList {};
    std::string m_dirPath {};
    std::shared_ptr<BackupQueue<FileHandle>> m_writeQueue { nullptr };
    std::shared_ptr<BlockBufferMap> m_blockBufferMap { nullptr };
    std::shared_ptr<BackupControlInfo> m_controlInfo { nullptr };
    std::shared_ptr<SQLiteDB> m_sqliteDb { nullptr };
    std::string m_sqliteDBRootPath { nullptr };
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };
    FileHandle m_blobfileHandle {};
    bool m_isDeleteBlobFile {false};
    std::atomic<uint64_t> *m_sqliteTaskProduce;
    std::atomic<uint64_t> *m_sqliteTaskConsume;
    std::vector<std::shared_ptr<BlobFileList>> m_blobFileList { nullptr };

private:
    /* Backup related APIs */
    void DoAggrFilesAndCreateIndex();
    std::string GenerateArchiveFileName();
    void PushArchiveFileToWriter(FileHandle& fileHandle);
    std::string GetUniqueIdStr() const;
    void CreateTaskForSqliteIndex(FileHandle &fileHandle, std::string &archiveFileName);
    std::string GetDbFile(std::string &dirPath);

    /* Restore related APIs */
    void HandleUnaggregation();
    void CreateFileHandles(uint8_t *buffer, FileHandle& fileHandle);
    void CreateNPushToWriteQueue(uint8_t *buffer, FileHandle& fileHandle, bool isSingleBlk = false);
    void DeleteFileList(std::string &parentDir, std::vector<std::string> &blobFileList) const;
    int GetNormalFilesFromMultiSqliteByName(std::vector<std::string> fileList, std::string& fileName,
        AggSqlRestoreQueryInfo& info, std::vector<AggSqlRestoreInfo>& vecNormalFiles);
    void DeleteOldBlobFiles();
};

class SqliteTask : public Module::ExecutableItem {
public:
    SqliteTask(SqliteEvent event, std::shared_ptr<Module::Snowflake> idGenerator,
        std::atomic<uint64_t> *sqlTaskConsume, BackupParams &backupParams)
        : m_event(event), m_idGenerator(idGenerator), m_sqliteTaskConsume(sqlTaskConsume), m_backupParams(backupParams)
    {
        DBGLOG("FileAggregateTask constructor");
    }

    virtual ~SqliteTask() {};
    void Exec() override;

private:
    /* Backup related APIs */
    int32_t CreateSqliteDb(std::shared_ptr<BlobFileDetails> blobFileDetails, std::string &dbFile);
    void DoCreateSqliteIndex(std::shared_ptr<BlobFileDetails> blobFileDetails, uint16_t &index);
    std::string GetUniqueIdStr();
    int32_t InsertIndexInfo(std::shared_ptr<BlobFileDetails> blobFileDetails, sqlite3_stmt *sqlStmt,
        std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr);
    void DoCreateSqliteIndexBlobList();
    std::string GetDbFile(std::string &dirPath);
    std::string GetObsKey(const std::string &key, const std::string &type);

public:
    SqliteEvent m_event { SqliteEvent::INVALID_EVENT };
    std::vector<std::shared_ptr<BlobFileDetails>> m_blobFileDetailsList;
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };
    std::vector<uint16_t> m_failedIndex;
    std::atomic<uint64_t> *m_sqliteTaskConsume;
    BackupParams m_backupParams;
};

#endif // FILE_AGGREGATE_TASK_H