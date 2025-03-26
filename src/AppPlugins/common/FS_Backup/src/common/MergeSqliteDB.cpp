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
#include "MergeSqliteDB.h"
#include "FSBackupUtils.h"
#include "Utils.h"
#include "Snowflake.h"
#include "ThreadPoolFactory.h"

using namespace std;
using namespace Module;

namespace {
    const string SQLITE_DIR = "sqlite";
    const string SQLITE_ALIAS_DIR = "sqlitealias";
    const string INDEX_DB_FILE_NAME = "copymetadata.sqlite";
    const int MERGE_THREAD_NUM = 16;
    const int MAX_PROCESS_TASK_NUM = 32;
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const int64_t MAX_MERGE_RECORD_NUM = 100000; // 10w
    const int RETRY_TIME_MILLISENCOND = 1000;
}

void MergeServiceTask::Exec()
{
    m_result = MergeCurDirDbFiles();
}

int MergeServiceTask::GetTotalRecordNum(std::vector<std::string>& fileList, int64_t& totalRecordNum)
{
    IndexDetails indexInfo {};
    for (const auto &srcDbFile : fileList) {
        int64_t curRecordNum = 0;
        int ret = indexInfo.QueryRecordNum(m_sqliteDb, srcDbFile.substr(m_sqlitePath.length()), curRecordNum);
        if (ret != Module::SUCCESS) {
            return ret;
        }
        totalRecordNum += curRecordNum;
        if (totalRecordNum > MAX_MERGE_RECORD_NUM) {
            return Module::SUCCESS;
        }
    }

    return Module::SUCCESS;
}

int MergeServiceTask::MergeIntoDstSqliteDB(std::vector<std::string>& fileList)
{
    std::string dstDbFile = m_dirPath + Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    std::vector<std::string> failedList {};
    IndexDetails indexInfo {};
    for (const auto &srcDbFile : fileList) {
        // 增量备份copymetadata.sqlite已经存在，不需要合并自己
        if (srcDbFile.substr(m_sqlitePath.length()) == dstDbFile) {
            continue;
        }
        int ret = indexInfo.MergeDbFile(m_sqliteDb, dstDbFile, srcDbFile.substr(m_sqlitePath.length()));
        if (ret != Module::SUCCESS) {
            failedList.emplace_back(srcDbFile);
            continue;
        }
        FSBackupUtils::RemoveFile(srcDbFile);
    }

    for (const auto &srcDbFile : failedList) {
        DBGLOG("Retry %s", srcDbFile.c_str());
        int ret = indexInfo.MergeDbFile(m_sqliteDb, dstDbFile, srcDbFile.substr(m_sqlitePath.length()));
        if (ret != Module::SUCCESS) {
            return ret;
        }
        FSBackupUtils::RemoveFile(srcDbFile);
    }

    return Module::SUCCESS;
}

int MergeServiceTask::MergeCurDirDbFiles()
{
    std::string fullPath = m_sqlitePath + m_dirPath;
    std::vector<std::string> fileList {};
    std::vector<std::string> dirList {};
    if (!FSBackupUtils::Exists(fullPath)) {
        // 非永久增量备份，增量没有文件时，不会创建sqlite目录
        INFOLOG("Not exist %s", fullPath.c_str());
        return Module::SUCCESS;
    }
    int ret = FSBackupUtils::GetFileWithSubDirListInDir(fullPath, fileList, dirList);
    if (ret != Module::SUCCESS) {
        return ret;
    }
    for (const auto &subDir : dirList) {
        // SQLiteDB 用的是相对路径
        m_dirQueue->Push(subDir.substr(m_sqlitePath.length()));
    }

    DBGLOG("Begin to merge all sqlites under dir %s, file number: %lu", fullPath.c_str(), fileList.size());

    std::string dstDbFile = m_dirPath + Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    if (fileList.size() == 1) {
        FSBackupUtils::Rename(fileList[0], m_sqlitePath + dstDbFile);
        DBGLOG("Finish to merge all sqlites under dir %s", fullPath.c_str());
        return Module::SUCCESS;
    }

    int64_t totalRecordNum = 0;
    ret = GetTotalRecordNum(fileList, totalRecordNum);
    if (ret != Module::SUCCESS) {
        ERRLOG("Get record num failed.");
        return ret;
    }

    if (totalRecordNum > MAX_MERGE_RECORD_NUM) {
        WARNLOG("Total records in table T_COPY_METADATA over 10w");
        return Module::SUCCESS;
    }

    ret = MergeIntoDstSqliteDB(fileList);
    if (ret != Module::SUCCESS) {
        ERRLOG("Merge all sqlites err under dir %s", fullPath.c_str());
        return ret;
    }

    DBGLOG("Finish to merge all sqlites under dir %s", fullPath.c_str());
    return Module::SUCCESS;
}

MergeSqliteDB::MergeSqliteDB(const std::string &metaPath)
{
    m_sqlitePath = metaPath + Module::PATH_SEPARATOR + SQLITE_DIR;
    m_aliasPath = metaPath + Module::PATH_SEPARATOR + SQLITE_ALIAS_DIR;

    FSBackupUtils::RecurseCreateDirectory(m_aliasPath);
    m_idGenerator = make_shared<Snowflake>();
    m_idGenerator->SetMachine(Module::GetMachineId());
    m_sqliteDb = make_shared<SQLiteDB>(m_sqlitePath, m_aliasPath, m_idGenerator);

    BackupQueueConfig config;
    config.maxSize = DEFAULT_BACKUP_QUEUE_SIZE;
    config.maxMemorySize = DEFAULT_BACKUP_QUEUE_MEMORY_SIZE;
    m_dirQueue = std::make_shared<BackupQueue<std::string>>(config);
}

MergeSqliteDB::~MergeSqliteDB()
{
    if (m_mainThread.joinable()) {
        m_mainThread.join();
    }
    if (m_pollThread.joinable()) {
        m_pollThread.join();
    }
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destroy thread pool %s", m_threadPoolKey.c_str());
    FSBackupUtils::MemoryTrim();
}

bool MergeSqliteDB::IsComplete()
{
    return m_dirQueue->Empty() && (m_taskProduce == m_taskConsume);
}

int MergeSqliteDB::ThreadFunc()
{
    INFOLOG("Start merge DB main thread");
    m_dirQueue->Push(""); // sqlite 作为根目录
    while (true) {
        if (IsComplete()) {
            m_isComplete = true;
            break;
        }

        // 限制并发处理数量
        while (m_taskProduce >= m_taskConsume + MAX_PROCESS_TASK_NUM) {
            Module::SleepFor(std::chrono::milliseconds(QUEUE_TIMEOUT_MILLISECOND));
        }

        std::string dirPath;
        if (!m_dirQueue->WaitAndPop(dirPath, QUEUE_TIMEOUT_MILLISECOND)) {
            continue;
        }

        auto task = make_shared<MergeServiceTask>(m_sqliteDb, m_sqlitePath, m_dirQueue, dirPath);
        if (!m_jsPtr->Put(task, true, TIME_LIMIT_OF_PUT_TASK)) {
            ERRLOG("put task failed");
            m_result = Module::FAILED;
        }
        ++m_taskProduce;
    }
    INFOLOG("End merge DB main thread");
    return Module::SUCCESS;
}

void MergeSqliteDB::PollWriteTask()
{
    INFOLOG("Start PollWriteTask thread");
    std::shared_ptr<ExecutableItem> threadPoolRes;
    while (true) {
        if (m_jsPtr->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            std::shared_ptr<MergeServiceTask> taskPtr = dynamic_pointer_cast<MergeServiceTask>(threadPoolRes);
            if (taskPtr == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }
            ++m_taskConsume;
            if (taskPtr->m_result == SUCCESS) {
                DBGLOG("Handle success for %s", taskPtr->m_dirPath.c_str());
            } else {
                ERRLOG("Handle failed for %s", taskPtr->m_dirPath.c_str());
                m_result = taskPtr->m_result;
            }
        }
        if (m_isComplete) {
            break;
        }
    }

    INFOLOG("End PollWriteTask thread");
    return;
}

int MergeSqliteDB::MergeAlldbFilesUnderSameDir(bool &isComplete)
{
    m_threadPoolKey = "BackupMergeSqliteDB_" + std::to_string(m_idGenerator->GenerateId());
    INFOLOG("create thread pool %s size %d", m_threadPoolKey.c_str(), MERGE_THREAD_NUM);
    std::shared_ptr<void> defer(nullptr, [&](...) { isComplete = true; });

    m_jsPtr = make_shared<JobScheduler>(*ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, MERGE_THREAD_NUM));
    if (m_jsPtr == nullptr) {
        ERRLOG("Create thread pool failed");
        return Module::FAILED;
    }

    try {
        m_mainThread = std::thread(&MergeSqliteDB::ThreadFunc, this);
        m_pollThread = std::thread(&MergeSqliteDB::PollWriteTask, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return Module::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return Module::FAILED;
    }

    time_t lastTime = FSBackupUtils::GetCurrentTime();
    while (!m_isComplete) {
        Module::SleepFor(std::chrono::seconds(1));
        time_t curTime = FSBackupUtils::GetCurrentTime();
        if (curTime- lastTime >= COMPLETION_CHECK_INTERVAL) {
            INFOLOG("Wait to finish ... (Produce: %lu, Consume: %lu)", m_taskProduce.load(), m_taskConsume.load());
            lastTime = curTime;
        }
    }

    INFOLOG("End merge sqlite DB. (Produce: %lu, Consume: %lu)", m_taskProduce.load(), m_taskConsume.load());
    return m_result;
}
