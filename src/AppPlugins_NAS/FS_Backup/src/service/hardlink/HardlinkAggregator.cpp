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
#include "HardlinkAggregator.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "Utils.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
#ifdef WIN32
    const string PATH_SEPARATOR_CHAR = "\\";
#else
    const string PATH_SEPARATOR_CHAR = "/";
#endif
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    const string SQLITE_DIR = "sqlite";
    const string SQLITE_ALIAS_DIR = "sqlitealias";
    const int HARDLINK_AGGR_FILES_THRESHOLD = 100;
    const int HOST_NAME_LEN = 512;
    const uint16_t MAX_BLOB_FILE_PER_SQLITE_TASK = 1;
    const uint16_t MAX_SQL_TASK_RUNNING = 10;
    const int JOB_SCHED_PUT_TIMEOUT_MILLISEC = 100;
}

HardlinkAggregator::HardlinkAggregator(const AggregatorParams& aggregatorParams)
    : m_backupParams(aggregatorParams.backupParams),
    m_aggregateQueue(aggregatorParams.aggregateQueuePtr),
    m_writeQueue(aggregatorParams.writeQueuePtr),
    m_controlInfo(aggregatorParams.controlInfo),
    m_hardlinkMap(aggregatorParams.hardlinkMap)
{
    m_threadPoolKey = m_backupParams.commonParams.jobId + "_FileAggregatorSqliteOps";
    m_idGenerator = make_shared<Snowflake>();
    m_idGenerator->SetMachine(Module::GetMachineId());
    m_sqliteDBRootPath = m_backupParams.commonParams.metaPath + PATH_SEPARATOR_CHAR + SQLITE_DIR;
    m_sqliteDBAliasPath = m_backupParams.commonParams.metaPath + PATH_SEPARATOR_CHAR + SQLITE_ALIAS_DIR
         + PATH_SEPARATOR_CHAR + m_backupParams.commonParams.subJobId;
    m_sqliteDb = make_shared<SQLiteDB>(m_sqliteDBRootPath, m_sqliteDBAliasPath, m_idGenerator);
    FSBackupUtils::RecurseCreateDirectory(m_sqliteDBAliasPath);
}

HardlinkAggregator::HardlinkAggregator(
    BackupParams& backupParams,
    shared_ptr<BackupQueue<FileHandle>> aggregateQueuePtr,
    shared_ptr<BackupQueue<FileHandle>> writeQueuePtr,
    shared_ptr<BackupControlInfo> controlInfo,
    shared_ptr<HardLinkMap> hardlinkMap)
    : m_backupParams(backupParams),
      m_aggregateQueue(aggregateQueuePtr),
      m_writeQueue(writeQueuePtr),
      m_controlInfo(controlInfo),
      m_hardlinkMap(hardlinkMap)
{
    m_threadPoolKey = m_backupParams.commonParams.jobId + "_FileAggregatorSqliteOps";
    m_idGenerator = make_shared<Snowflake>();
    m_idGenerator->SetMachine(Module::GetMachineId());
    m_sqliteDBRootPath = backupParams.commonParams.metaPath + PATH_SEPARATOR_CHAR + SQLITE_DIR;
    m_sqliteDBAliasPath = backupParams.commonParams.metaPath + PATH_SEPARATOR_CHAR + SQLITE_ALIAS_DIR
         + PATH_SEPARATOR_CHAR + m_backupParams.commonParams.subJobId;
    m_sqliteDb = make_shared<SQLiteDB>(m_sqliteDBRootPath, m_sqliteDBAliasPath, m_idGenerator);
    FSBackupUtils::RecurseCreateDirectory(m_sqliteDBAliasPath);
}

HardlinkAggregator::~HardlinkAggregator()
{
    if (m_thread.joinable()) {
        m_thread.join();
    }
    m_jsPtr.clear();
    m_blobFileList.clear();
    FSBackupUtils::RemoveDir(m_sqliteDBAliasPath);
}

/* Public APIs */
BackupRetCode HardlinkAggregator::Start()
{
    INFOLOG("Start HardlinkAggregator, create thread pool %s size %d",
        m_threadPoolKey.c_str(), m_backupParams.commonParams.aggregateThreadNum);
    for (uint16_t i = 0; i < MAX_JOB_SCHDULER; i++) {
        std::shared_ptr<Module::JobScheduler> t_jsPtr = make_shared<JobScheduler>(
            *ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey + std::to_string(i), 1));
    if (t_jsPtr == nullptr) {
            ERRLOG("Create thread pool failed");
            return BackupRetCode::FAILED;
        }
        m_jsPtr.push_back(t_jsPtr);

        std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
        m_blobFileList.push_back(tblobFileList);
    }

    try {
        m_thread = std::thread(&HardlinkAggregator::ThreadFunc, this);
    } catch (exception &e) {
        ERRLOG("Create thread func failed: %s", e.what());
        return BackupRetCode::FAILED;
    }  catch (...) {
        ERRLOG("Create thread func failed: unknow reason");
        return BackupRetCode::FAILED;
    }

    return BackupRetCode::SUCCESS;
}

BackupRetCode HardlinkAggregator::Abort()
{
    INFOLOG("HardlinkAggregator abort!");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

BackupPhaseStatus HardlinkAggregator::GetStatus()
{
    return FSBackupUtils::GetAggregateStatus(m_controlInfo, m_abort);
}

bool HardlinkAggregator::IsAbort()
{
    if (m_abort || m_controlInfo->m_failed || m_controlInfo->m_controlReaderFailed) {
        INFOLOG("abort %d failed %d controlFileReaderFailed %d", m_abort, m_controlInfo->m_failed.load(),
            m_controlInfo->m_controlReaderFailed.load());
        return true;
    }
    return false;
}

bool HardlinkAggregator::IsComplete()
{
    if ((FSBackupUtils::GetCurrentTime() - m_isCompleteTimer) > COMPLETION_CHECK_INTERVAL) {
        m_isCompleteTimer = FSBackupUtils::GetCurrentTime();
        INFOLOG("HardlinkAggregator check is complete: aggrConsume %llu aggrProduce %llu copyReadProduce %llu "
            "readPhaseComplete %d aggTaskProduce %llu aggTaskConsume %llu aggregateQueueSize %llu IsBlobListEmpty %d",
            m_controlInfo->m_aggregateConsume.load(), m_controlInfo->m_aggregateProduce.load(),
            m_controlInfo->m_readProduce.load(), m_controlInfo->m_readPhaseComplete.load(),
            m_aggTaskProduce.load(),  m_aggTaskConsume.load(), m_aggregateQueue->GetSize(), IsBlobListEmpty());
    }
    if ((m_controlInfo->m_readProduce.load() == m_controlInfo->m_aggregateConsume.load()) &&
        m_controlInfo->m_readPhaseComplete &&
        (m_aggTaskProduce.load() == m_aggTaskConsume.load()) &&
        m_aggregateQueue->Empty()
        && IsBlobListEmpty()) {
        INFOLOG("HardlinkAggregator complete: aggrConsume %llu aggrProduce %llu copyReadProduce %llu "
            "readPhaseComplete %d aggTaskProduce %llu aggTaskConsume %llu aggregateQueueSize %llu IsBlobListEmpty %d",
            m_controlInfo->m_aggregateConsume.load(), m_controlInfo->m_aggregateProduce.load(),
            m_controlInfo->m_readProduce.load(), m_controlInfo->m_readPhaseComplete.load(),
            m_aggTaskProduce.load(),  m_aggTaskConsume.load(), m_aggregateQueue->GetSize(), IsBlobListEmpty());
        return true;
    }
    if ((m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) &&
                ((m_backupParams.backupType == BackupType::BACKUP_FULL) ||
                (m_backupParams.backupType == BackupType::BACKUP_INC))) {
        PrintSqliteTaskDistribution();
    }

    return false;
}

/* Private methods */
void HardlinkAggregator::HandleSuccessEvent(shared_ptr<SqliteTask> taskPtr)
{
    DBGLOG("Handle event is success.");
}

void HardlinkAggregator::HandleFailureEvent(shared_ptr<SqliteTask> taskPtr)
{
    ERRLOG("Handle event is failure.");
}

void HardlinkAggregator::PollHardlinkAggregateTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;
    for (uint16_t i = 0; i < m_jsPtr.size(); i++) {
        while (!m_jsPtr[i]->Empty()) {
            m_jsPtr[i]->Get(threadPoolRes);
            shared_ptr<SqliteTask> task = dynamic_pointer_cast<SqliteTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }

            if (task->m_result == SUCCESS) {
                HandleSuccessEvent(task);
            } else {
                HandleFailureEvent(task);
            }

            (*task->m_sqliteTaskConsume)++;
            DBGLOG("Hardlink aggTaskConsume :%llu", m_aggTaskConsume.load());
        }
    }

    return;
}

void HardlinkAggregator::PushToWriteQueue(FileHandle &fileHandle)
{
    while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        if (IsAbort()) {
            return;
        }
    }

    ++m_controlInfo->m_aggregateProduce;
    DBGLOG("Hardlink aggregate put file to write queue, %s, %d", fileHandle.m_file->m_fileName.c_str(),
           m_controlInfo->m_aggregateProduce.load());
}

void HardlinkAggregator::CreateSqliteIndexTask(shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo)
{
    auto blobFile = std::make_shared<BlobFileDetails>();
    blobFile->aggregatedFileSize = 0;
    blobFile->archiveFileName = "";
    blobFile->m_dirPath =  aggregateDirInfo->m_dirName;
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;
    for (uint64_t i = 0; i < aggregateDirInfo->m_hardlinkFiles->size(); i++) {
        SmallFileDesc t_fileDesc;
        t_fileDesc.m_onlyFileName = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_onlyFileName;
        t_fileDesc.m_aggregateFileOffset = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_aggregateFileOffset;
        t_fileDesc.m_size = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_size;
        t_fileDesc.m_ctime = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_ctime;
        t_fileDesc.m_mtime = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_mtime;
        t_fileDesc.m_flag = (*aggregateDirInfo->m_hardlinkFiles)[i].m_file->m_flag;
        blobFile->m_smallFileDescList.push_back(t_fileDesc);
    }

    /* Ensure to give an idx which is between 0 to 7 */
    uint16_t hashIndex = FSBackupUtils::GetHashIndexForSqliteTask(blobFile->m_dirPath);
    std::unique_lock<std::mutex> lk(m_blobFileList[hashIndex]->m_mtx);
    m_blobFileList[hashIndex]->m_blobFileDetailsList.emplace(blobFile);
    m_blobFileList[hashIndex]->m_numOfBlobsFilesInserted++;
    lk.unlock();

    /* Create a new vector to handle next set of hardlinks */
    aggregateDirInfo->m_hardlinkFiles.reset();
    aggregateDirInfo->m_hardlinkFiles = make_shared<vector<FileHandle>>();
}

void HardlinkAggregator::CheckAndCreateAggregateTask(const std::string &dirName)
{
    shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = m_aggregateHardlinkMap.GetDirInfo(dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("CheckAndCreateAggregateTask dir %s failed, aggregateDirInfo is null", dirName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }
    DBGLOG("readCompletedFilesCount = %u, m_totalHardlinkFilesCount = %u, dirName = %s",
        aggregateDirInfo->m_readCompletedFilesCount,
        aggregateDirInfo->m_totalHardlinkFilesCount, dirName.c_str());

    if ((aggregateDirInfo->m_readCompletedFilesCount == aggregateDirInfo->m_totalHardlinkFilesCount) ||
        (aggregateDirInfo->m_hardlinkFiles->size() == HARDLINK_AGGR_FILES_THRESHOLD)) {
        CreateSqliteIndexTask(aggregateDirInfo);
    }
    // Clear the map entries
    if (aggregateDirInfo->m_readCompletedFilesCount == aggregateDirInfo->m_totalHardlinkFilesCount) {
        m_aggregateHardlinkMap.Erase(dirName);
    }
}

void HardlinkAggregator::HandleHardlinkAggregate(FileHandle &fileHandle)
{
    string dirName = fileHandle.m_file->m_dirName;
    if (!m_aggregateHardlinkMap.Exist(fileHandle.m_file->m_dirName)) {
        shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = make_shared<AggregateHardlinkDirInfo>();
        aggregateDirInfo->m_hardlinkFiles = make_shared<vector<FileHandle>>();
        aggregateDirInfo->m_dirName = dirName;
        aggregateDirInfo->m_totalHardlinkFilesCount = fileHandle.m_file->m_fileCount;
        aggregateDirInfo->m_readCompletedFilesCount = 1;
        aggregateDirInfo->m_hardlinkFiles->push_back(fileHandle);
        DBGLOG("Insert dir info (%s) into aggregateHardlinkInfoMap m_totalHardlinkFilesCount : %u",
            dirName.c_str(), aggregateDirInfo->m_totalHardlinkFilesCount);
        m_aggregateHardlinkMap.Insert(dirName, aggregateDirInfo);
    } else {
        shared_ptr<AggregateHardlinkDirInfo> aggregateDirInfo = m_aggregateHardlinkMap.GetDirInfo(dirName);
        if (aggregateDirInfo == nullptr) {
            ++m_controlInfo->m_noOfFilesFailed;
            ERRLOG("HardlinkAggregator %s failed, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
            return;
        }
        aggregateDirInfo->m_readCompletedFilesCount++;
        aggregateDirInfo->m_hardlinkFiles->push_back(fileHandle);
    }

    CheckAndCreateAggregateTask(dirName);
}

void HardlinkAggregator::ThreadFunc()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    INFOLOG("HardlinkAggregator main thread start!");
    bool isAggregate = (m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) ? true : false;
    bool isBackup = (m_backupParams.backupType == BackupType::BACKUP_FULL ||
        m_backupParams.backupType == BackupType::BACKUP_INC) ? true : false;

    while (true) {
        if (IsAbort() || IsComplete()) {
            break;
        }
        if (!isAggregate || CanAcceptMoreWork()) {
            FileHandle fileHandle;
            bool ret = m_aggregateQueue->WaitAndPop(fileHandle, QUEUE_TIMEOUT_MILLISECOND);
            if (ret) {
                DBGLOG("aggregate get file from read queue, %s", fileHandle.m_file->m_fileName.c_str());
                ++m_controlInfo->m_aggregateConsume;

                if (isAggregate && isBackup) {
                    HandleHardlinkAggregate(fileHandle);
                }

                PushToWriteQueue(fileHandle);
            }
        }

        if (isAggregate && isBackup) {
            ExecuteSqliteTasks();
            PollHardlinkAggregateTask();
        }
    }
    m_controlInfo->m_aggregatePhaseComplete = true;
    INFOLOG("HardlinkAggregator main thread end!");
    return;
}

void HardlinkAggregator::ExecuteSqliteTasks()
{
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::unique_lock<std::mutex> lk(m_blobFileList[i]->m_mtx);
        std::queue<std::shared_ptr<BlobFileDetails>>& blobFileDetailsList = m_blobFileList[i]->m_blobFileDetailsList;
        if (blobFileDetailsList.size() < MAX_BLOB_FILE_PER_SQLITE_TASK) {
            lk.unlock();
            continue;
        }

        auto task = make_shared<SqliteTask>(SqliteEvent::SQL_TASK_PTR_CREATE_INDEX,
                    m_idGenerator, &m_aggTaskConsume, m_backupParams);
        while (!blobFileDetailsList.empty()) {
            auto it = blobFileDetailsList.front();
            blobFileDetailsList.pop();
            task->m_blobFileDetailsList.push_back(it);
        }
        lk.unlock();

        if (m_jsPtr[i]->Put(task, true, JOB_SCHED_PUT_TIMEOUT_MILLISEC) == false) {
            WARNLOG("Put SQLITE task for the blob files timedout, retry later");
            lk.lock();
            for (size_t i = 0; i < task->m_blobFileDetailsList.size(); i++) {
                blobFileDetailsList.emplace(task->m_blobFileDetailsList[i]);
            }
            lk.unlock();
            continue;
        }
        m_aggTaskProduce++;
    }
}

bool HardlinkAggregator::IsBlobListEmpty() const
{
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::lock_guard<std::mutex> lk(m_blobFileList[i]->m_mtx);
        if (m_blobFileList[i]->m_blobFileDetailsList.size() != 0) {
            return false;
        }
    }
    return true;
}

void HardlinkAggregator::PrintSqliteTaskDistribution() const
{
    INFOLOG("Hash Distribution of Sqlite job");
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        DBGLOG(" Slot[%u]--%u", i, m_blobFileList[i]->m_numOfBlobsFilesInserted);
    }
}

bool HardlinkAggregator::CanAcceptMoreWork() const
{
    uint32_t sqliteTaskCount = 0;
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::lock_guard<std::mutex> lk(m_blobFileList[i]->m_mtx);
        sqliteTaskCount += m_blobFileList[i]->m_blobFileDetailsList.size();
    }

    if (sqliteTaskCount > MAX_SQL_TASK_RUNNING) {
        return false;
    }

    return true;
}
