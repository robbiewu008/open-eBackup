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
#ifndef MERGE_SQLITE_DB_H
#define MERGE_SQLITE_DB_H

#include "backup_layout/SqliteOps.h"
#include "BackupQueue.h"
#include "ThreadPool.h"

class MergeServiceTask : public Module::ExecutableItem {
public:
    MergeServiceTask(std::shared_ptr<SQLiteDB> sqliteDb, std::string& sqlitePath,
        std::shared_ptr<BackupQueue<std::string>> dirQueue, std::string& dirPath)
        : m_dirPath(dirPath), m_sqliteDb(sqliteDb), m_sqlitePath(sqlitePath), m_dirQueue(dirQueue)
    {}
    virtual ~MergeServiceTask() {}
    void Exec();
    std::string m_dirPath;

private:
    int GetTotalRecordNum(std::vector<std::string>& fileList, int64_t& totalRecordNum);
    int MergeIntoDstSqliteDB(std::vector<std::string>& fileList);
    int MergeCurDirDbFiles();

    std::shared_ptr<SQLiteDB> m_sqliteDb { nullptr };
    std::string m_sqlitePath;
    std::shared_ptr<BackupQueue<std::string>> m_dirQueue { nullptr };
};

class MergeSqliteDB {
public:
    explicit MergeSqliteDB(const std::string &metaPath);
    ~MergeSqliteDB();
    int MergeAlldbFilesUnderSameDir(bool &isComplete);

private:
    int ThreadFunc();
    void PollWriteTask();
    bool IsComplete();

private:
    std::string m_sqlitePath;
    std::string m_aliasPath;
    std::shared_ptr<SQLiteDB> m_sqliteDb { nullptr };
    std::shared_ptr<Module::Snowflake> m_idGenerator { nullptr };

    std::thread             m_mainThread;
    std::thread             m_pollThread;
    std::atomic<uint64_t>   m_taskProduce { 0 };
    std::atomic<uint64_t>   m_taskConsume { 0 };
    std::string m_threadPoolKey;
    std::shared_ptr<Module::JobScheduler> m_jsPtr { nullptr };
    std::shared_ptr<BackupQueue<std::string>> m_dirQueue { nullptr };

    bool m_isComplete { false };
    int m_result { 0 };
};

#endif // MERGE_SQLITE_DB_H
