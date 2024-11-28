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
#include <codecvt>
#include <sys/stat.h>
#include <chrono>
#include <thread>
#include <cctype>

#include "FileAggregator.h"
#ifndef WIN32
#include <unistd.h>
#include <sys/statvfs.h>
#else
#include "Win32PathUtils.h"
#endif
#include "log/Log.h"
#include "common/Thread.h"
#include "Backup.h"
#include "BackupStructs.h"
#include "FSBackupUtils.h"
#include "Utils.h"
#include "FileAggregateTask.h"

using namespace std;
using namespace Module;
using namespace FS_Backup;

namespace {
#ifdef WIN32
    const string PATH_SEPERATOR = "\\";
#else
    const string PATH_SEPERATOR = "/";
#endif
    const std::string MODULE_NAME = "FileAggregator";
    const int JOB_SCHED_PUT_TIMEOUT_MILLISEC = 100;
    const int QUEUE_TIMEOUT_MILLISECOND = 10;
    const string SQLITE_DIR = "sqlite";
    const string SQLITE_ALIAS_DIR = "sqlitealias";
    const string SQLITE_TMP_DIR = "/tmp";
    const uint32_t KB_IN_BYTES = 1024;
    const string INDEX_DB_FILE_NAME = "copymetadata.sqlite";
    constexpr uint32_t FIVECOUNT = 5;
    const int NUMBER_ONE = 1;
    constexpr uint64_t NUMBER10 = 10;
    constexpr uint32_t MAXFILESINSINGLEBLOBFILE = 512;
    constexpr uint32_t FREE_BUFFER_AFTER_NO_FILES_MAX_TIMES = 100;
    constexpr uint32_t MAX_FILES_IN_SINGLE_SQLITE_TASK = 512;
    const uint16_t MAX_BLOB_FILE_PER_SQLITE_TASK = 1;
    const uint16_t MAX_SQL_TASK_RUNNING = 10;
    constexpr auto MODULE = "FileAggregator";
}

FileAggregator::FileAggregator(
    const BackupParams& backupParams,
    std::shared_ptr<BackupQueue<FileHandle>> writeQueue,
    std::shared_ptr<BackupQueue<FileHandle>> readQueue,
    std::shared_ptr<BlockBufferMap> blockBufferMap,
    std::shared_ptr<BackupControlInfo> controlInfo)
    : m_backupParams(backupParams),
      m_writeQueue(writeQueue),
      m_readQueue(readQueue),
      m_blockBufferMap(blockBufferMap),
      m_controlInfo(controlInfo)
{
    /* Every subtasks uses an independant threadpool for file aggregation, hence prepend sub-taskid to make it unique */
    m_threadPoolKey = m_backupParams.commonParams.subJobId +"_FileAggregator";
    /* All subtasks share same thread pool for sqlite db operations, hence prepend main-taskid for sharing */
    m_sqlThreadPoolKey = m_backupParams.commonParams.jobId + "_FileAggregatorSqliteOps";
    m_idGenerator = make_shared<Snowflake>();
    m_idGenerator->SetMachine(Module::GetMachineId());

    BackupType backupType = m_backupParams.backupType;
    if (m_backupParams.commonParams.useSubJobSqlite && (m_backupParams.phase != BackupPhase::DELETE_STAGE) &&
        ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC))) {
        std::string sqliteTmpPath = m_backupParams.commonParams.sqliteLocalPath;
        std::string& subJobId = m_backupParams.commonParams.subJobId;
        if (sqliteTmpPath != SQLITE_TMP_DIR && CheckSqliteDir(sqliteTmpPath)) {
            m_sqliteDBRootPath = sqliteTmpPath + PATH_SEPERATOR + subJobId + PATH_SEPERATOR + SQLITE_DIR;
            m_sqliteDBAliasPath = sqliteTmpPath + PATH_SEPERATOR + SQLITE_ALIAS_DIR + PATH_SEPERATOR + subJobId;
        } else if (CheckSqliteDir(SQLITE_TMP_DIR)) {
            DBGLOG("Used SQLITE_TMP_DIR");
            sqliteTmpPath = SQLITE_TMP_DIR;
            m_sqliteDBRootPath = sqliteTmpPath + PATH_SEPERATOR + subJobId + PATH_SEPERATOR + SQLITE_DIR;
            m_sqliteDBAliasPath = sqliteTmpPath + PATH_SEPERATOR + SQLITE_ALIAS_DIR + PATH_SEPERATOR + subJobId;
        } else {
            m_sqliteDBRootPath = backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_DIR;
            m_sqliteDBAliasPath = backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_ALIAS_DIR
                + PATH_SEPERATOR + m_backupParams.commonParams.subJobId;
        }
    } else {
        m_sqliteDBRootPath = backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_DIR;
        m_sqliteDBAliasPath = backupParams.commonParams.metaPath + PATH_SEPERATOR + SQLITE_ALIAS_DIR
            + PATH_SEPERATOR + m_backupParams.commonParams.subJobId;
    }
    m_sqliteDb = make_shared<SQLiteDB>(m_sqliteDBRootPath, m_sqliteDBAliasPath, m_idGenerator);
    m_maxFileSizeToAggregate = m_backupParams.commonParams.maxFileSizeToAggregate * KB_IN_BYTES;
    m_maxAggregateFileSize = m_backupParams.commonParams.maxAggregateFileSize * KB_IN_BYTES;
    FSBackupUtils::RecurseCreateDirectory(m_sqliteDBAliasPath);

    if (m_backupParams.commonParams.isReExecutedTask) {
        INFOLOG("This is a re-executed task, jobId: %s, subJobId: %s",
            m_backupParams.commonParams.jobId.c_str(), m_backupParams.commonParams.subJobId.c_str());
    }
}

bool FileAggregator::CheckSqliteDir(const std::string& path)
{
    uint64_t size;
    uint64_t availSize;
    if (!GetDirCapacity(path.c_str(), size, availSize)) {
        ERRLOG("Failed GetDirCapacity");
        return false;
    }
    DBGLOG("File system info: %llu %llu ", size, availSize);

    float tmpSizeInG = static_cast<float>(availSize) / (1024.0f * 1024.0f * 1024.0f);
    INFOLOG("Tmp dir size: %f GB", tmpSizeInG);
    if (tmpSizeInG < 10.0f) {
        return false;
    }
    m_isUsedLocalDir = true;

    return true;
}

bool FileAggregator::GetDirCapacity(const char *pathName, uint64_t &capacity, uint64_t &free)
{
#ifndef __WINDOWS__
    struct statvfs fsInfo;
    int rv = statvfs(pathName, &fsInfo);
    if (rv == 0) {
        capacity = fsInfo.f_frsize * static_cast<uint64_t>(fsInfo.f_blocks);
        free = fsInfo.f_frsize * static_cast<uint64_t>(fsInfo.f_bavail);
        INFOLOG("capacity free : %llu %llu ", capacity, free);
        return true;
    } else {
        int err = errno;
        HCP_Log(ERR, MODULE) << "Get FS mount size [statvfs()] has errors, errno: " << strerror(err) << HCPENDLOG;
        return false;
    }
#endif
        return false;
}

FileAggregator::~FileAggregator()
{
    ThreadPoolFactory::DestoryThreadPool(m_threadPoolKey);
    INFOLOG("Destruct FileAggregator, destroy thread pool %s", m_threadPoolKey.c_str());
    INFOLOG("aggregateDirMap size: %u", m_aggregateDirMap.GetSize());
    m_jsSqlPtr.clear();
    m_blobFileList.clear();
}

BackupRetCode FileAggregator::Start()
{
    if ((m_backupParams.commonParams.backupDataFormat == BackupDataFormat::AGGREGATE) ||
        FSBackupUtils::OnlyGenerateSqlite(m_backupParams.commonParams.genSqlite)) {
        uint32_t aggrThreadNum = m_backupParams.commonParams.aggregateThreadNum;
        INFOLOG("Start FileAggregator, create thread pool: %s size: %d", m_threadPoolKey.c_str(), aggrThreadNum);
        m_jsPtr = make_shared<JobScheduler>(*ThreadPoolFactory::GetThreadPoolInstance(m_threadPoolKey, aggrThreadNum));
        if (m_jsPtr == nullptr) {
            ERRLOG("Create thread pool failed");
            return BackupRetCode::FAILED;
        }

        /* Use only one thread for the sqliteops threadpool. This is to serialise the sqlite ops and avoid multiple
           threads work on same db file */
        for (uint16_t i = 0; i < MAX_JOB_SCHDULER; i++) {
            std::shared_ptr<Module::JobScheduler> tjsSqlPtr = make_shared<JobScheduler>(
                *ThreadPoolFactory::GetThreadPoolInstance(m_sqlThreadPoolKey + std::to_string(i), 1));
            if (tjsSqlPtr == nullptr) {
                ERRLOG("Create sqlite thread pool failed");
                return BackupRetCode::FAILED;
            }
            m_jsSqlPtr.push_back(tjsSqlPtr);
            std::shared_ptr<BlobFileList> tblobFileList = make_shared<BlobFileList>();
            m_blobFileList.push_back(tblobFileList);
        }
    } else {
        INFOLOG("Start FileAggregator");
    }

    m_started = true;
    return BackupRetCode::SUCCESS;
}

BackupRetCode FileAggregator::Abort()
{
    INFOLOG("Abort FileAggregator");
    m_abort = true;
    return BackupRetCode::SUCCESS;
}

bool FileAggregator::IsAbort() const
{
    if (m_abort || m_controlInfo->m_failed) {
        INFOLOG("abort %d failed %d", m_abort, m_controlInfo->m_failed.load());
        return true;
    }
    return false;
}

void FileAggregator::HandleComplete()
{
    BackupType backupType = m_backupParams.backupType;
    bool isBackup = ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC));
    if (m_backupParams.commonParams.useSubJobSqlite && isBackup && m_isUsedLocalDir) {
        std::string cmd = "cp -r " + m_sqliteDBRootPath + " " + m_backupParams.commonParams.metaPath;
        std::vector<std::string> output;
        std::vector<std::string> errOutput;
        INFOLOG("run cmd : %s", cmd.c_str());
        int ret = Module::runShellCmdWithOutput(INFO, MODULE_NAME, 0, cmd, { }, output, errOutput);
        if (ret != 0) {
            ERRLOG("run failed, cmd : %s, set m_controlInfo->m_failed", cmd.c_str());
            for (size_t i = 0; i < errOutput.size(); ++i) {
                ERRLOG("%s", errOutput[i].c_str());
            }
            m_controlInfo->m_failed = true;
        }
        FSBackupUtils::RemoveDir(FSBackupUtils::GetParentDirV2(m_sqliteDBRootPath));
    }
    FSBackupUtils::RemoveDir(m_sqliteDBAliasPath);
    INFOLOG("File aggregator complete, subJobId: %s", m_backupParams.commonParams.subJobId.c_str());
}

void FileAggregator::PollAggregateTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;

    if (!m_started) {
        if (IsAbort()) {
            WARNLOG("FileAggregator main thread abort!");
            return;
        }
        DBGLOG("m_started :%d", m_started);
        return;
    }

    while (m_jsPtr->Get(threadPoolRes, false)) {
        shared_ptr<FileAggregateTask> task = dynamic_pointer_cast<FileAggregateTask>(threadPoolRes);
        if (task == nullptr) {
            ERRLOG("task is nullptr");
            break;
        }

        if (task->m_result == SUCCESS) {
            HandleSuccessEvent(task);
        } else {
            HandleFailureEvent(task);
        }

        ++m_aggTaskConsume;
    }

    return;
}

bool FileAggregator::IsAggregateStarted() const
{
    return m_started;
}

void FileAggregator::PollSqlAggregateTask()
{
    shared_ptr<ExecutableItem> threadPoolRes;

    while (!m_started) {
        if (IsAbort()) {
            WARNLOG("FileAggregator main thread abort!");
            return;
        }
        DBGLOG("FileAggregator not started. waiting");
        Module::SleepFor(std::chrono::seconds(1)); /* Sleep for 1 second */
    }

    for (uint16_t i = 0; i < m_jsSqlPtr.size(); i++) {
        while (m_jsSqlPtr[i]->Get(threadPoolRes, true, QUEUE_TIMEOUT_MILLISECOND)) {
            shared_ptr<SqliteTask> task = dynamic_pointer_cast<SqliteTask>(threadPoolRes);
            if (task == nullptr) {
                ERRLOG("task is nullptr");
                break;
            }

            if (task->m_result == SUCCESS) {
                SqlHandleSuccessEvent(task);
            } else {
                SqlHandleFailureEvent(task);
            }

            (*task->m_sqliteTaskConsume)++;
        }
    }

    return;
}

void FileAggregator::HandleSuccessEvent(shared_ptr<FileAggregateTask> taskPtr) const
{
    if (taskPtr->m_event == AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX) {
        DBGLOG("AGGREGATE_FILES_AND_CREATE_INDEX event is success.");
        /* Add new FHs of an aggregated file */
        for (auto &fh : taskPtr->m_outputFileHandleList) {
            if (fh.m_block.m_seq != 0) {
                m_blockBufferMap->Add(fh.m_file->m_fileName, fh);
            }
        }
        /* Delete old FH's of non-aggregated (small) file */
        m_blockBufferMap->Delete(*taskPtr->m_inputFileHandleList);
        taskPtr->m_inputFileHandleList->clear();
        taskPtr->m_inputFileHandleList->shrink_to_fit();
        for (auto &fh : taskPtr->m_outputFileHandleList) {
            PushToWriteQueue(fh);
        }
        ++m_controlInfo->m_archiveFiles;
    } else if (taskPtr->m_event == AggregateEvent::UNAGGREGATE_FILES) {
        m_controlInfo->m_unaggregatedFiles += taskPtr->m_inputFileHandleList->size();
        DBGLOG("UNAGGREGATE_FILES event is success. Archive file: %s taskPtr->m_isDeleteBlobFile: %d"
            "m_controlInfo->m_unaggregatedFiles: %lld", taskPtr->m_blobfileHandle.m_file->m_onlyFileName.c_str(),
            taskPtr->m_isDeleteBlobFile, m_controlInfo->m_unaggregatedFiles.load());
         /* Add new FHs of an unaggregated files */
        for (auto &fh : taskPtr->m_outputFileHandleList) {
            if (fh.m_block.m_seq != 0) {
                m_blockBufferMap->Add(fh.m_file->m_fileName, fh);
            }
            PushToWriteQueue(fh);
        }
        /* Delete old FH's of aggregated (blob) file */
        if (taskPtr->m_isDeleteBlobFile) {
            DBGLOG("Delete Blob file %s", taskPtr->m_blobfileHandle.m_file->m_fileName.c_str());
            m_blockBufferMap->Delete(taskPtr->m_blobfileHandle.m_file->m_fileName);
        }
    }

    return;
}

void FileAggregator::HandleFailureEvent(shared_ptr<FileAggregateTask> taskPtr)
{
    if (taskPtr->m_event == AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX) {
        m_controlInfo->m_noOfFilesFailed += taskPtr->m_inputFileHandleList->size();
        ERRLOG("Aggregate task failed. event: %u, noOfFilesFailed: %u totalFailed: %llu", taskPtr->m_event,
            taskPtr->m_inputFileHandleList->size(), m_controlInfo->m_noOfFilesFailed.load());
        /* Delete old FH's of non-aggregated (small) file */
        m_blockBufferMap->Delete(*taskPtr->m_inputFileHandleList);
    } else if (taskPtr->m_event == AggregateEvent::UNAGGREGATE_FILES) {
        ERRLOG("UNAGGREGATE_FILES event is failed. Archive file: %s",
            taskPtr->m_blobfileHandle.m_file->m_onlyFileName.c_str());
        m_controlInfo->m_unaggregatedFaildFiles += taskPtr->m_inputFileHandleList->size();
        m_controlInfo->m_noOfFilesFailed += taskPtr->m_inputFileHandleList->size();
        ERRLOG("m_controlInfo->m_unaggregatedFaildFiles: %lld, totalFailed: %llu",
            m_controlInfo->m_unaggregatedFiles.load(), m_controlInfo->m_noOfFilesFailed.load());
        /* Delete old FH's of aggregated (blob) file */
        if (taskPtr->m_isDeleteBlobFile) {
            DBGLOG("Delete blob file %s", taskPtr->m_blobfileHandle.m_file->m_fileName.c_str());
            m_blockBufferMap->Delete(taskPtr->m_blobfileHandle.m_file->m_fileName);
        }
        return;
    }
    return;
}

void FileAggregator::SqlHandleSuccessEvent(shared_ptr<SqliteTask> taskPtr) const
{
    for (uint16_t i = 0; i < taskPtr->m_blobFileDetailsList.size(); i++) {
        taskPtr->m_blobFileDetailsList[i]->m_smallFileDescList.clear();
        taskPtr->m_blobFileDetailsList[i]->m_smallFileDescList.shrink_to_fit();
    }
    taskPtr->m_blobFileDetailsList.clear();
    taskPtr->m_blobFileDetailsList.shrink_to_fit();
    return;
}

void FileAggregator::SqlHandleFailureEvent(shared_ptr<SqliteTask> taskPtr) const
{
    uint32_t failedFileCount = 0;
    ERRLOG("Aggregate task failed. event: %u, errcode: %d", taskPtr->m_event, taskPtr->m_result);
    for (uint16_t i = 0; i < taskPtr->m_failedIndex.size(); i++) {
        uint16_t indx = taskPtr->m_failedIndex[i];
        vector<SmallFileDesc> inputFileList = taskPtr->m_blobFileDetailsList[indx]->m_smallFileDescList;
        for (size_t i = 0; i < inputFileList.size(); i++) {
            if (!inputFileList[i].IsFlagSet(IS_DIR)) {
                if (inputFileList[i].m_size == 0) {
                    // for empty files has been add 1 file copied. need to miuns if aggregate failed.
                    --m_controlInfo->m_noOfFilesFailed;
                }
                ++m_controlInfo->m_noOfFilesFailed;
                ERRLOG("create index failed. file handle inside: %s", inputFileList[i].m_onlyFileName.c_str());
            }
            failedFileCount++;
        }
        ERRLOG("Create sqlite db for dbFile: %s, arhiveFile: %s failed",
            taskPtr->m_blobFileDetailsList[indx]->m_dirPath.c_str(),
            taskPtr->m_blobFileDetailsList[indx]->archiveFileName.c_str());
    }
    ERRLOG("Aggregate task failed. event: %u, noOfFilesFailed: %u, totalFailed: %llu", taskPtr->m_event,
        failedFileCount, m_controlInfo->m_noOfFilesFailed.load());

    for (uint16_t i = 0; i < taskPtr->m_blobFileDetailsList.size(); i++) {
        taskPtr->m_blobFileDetailsList[i]->m_smallFileDescList.clear();
        taskPtr->m_blobFileDetailsList[i]->m_smallFileDescList.shrink_to_fit();
    }
    taskPtr->m_failedIndex.clear();
    taskPtr->m_blobFileDetailsList.clear();
    taskPtr->m_blobFileDetailsList.shrink_to_fit();
    return;
}

void FileAggregator::PushToWriteQueue(FileHandle &fileHandle) const
{
    DBGLOG("Push to write queue. File: %s", fileHandle.m_file->m_fileName.c_str());
    while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
        DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
        if (IsAbort()) {
            WARNLOG("FileAggregator main thread abort!");
            return;
        }
    }
}

void FileAggregator::CreateAggregateTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo)
{
    if (aggregateDirInfo->m_aggregatedFiles->size() > 0) {
        /* Encapsulate the params in a seperate arg to reduce params count !! */
        auto task = make_shared<FileAggregateTask>(
            AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX, m_backupParams,
            aggregateDirInfo->m_aggregatedFiles, aggregateDirInfo->m_dirName,
            m_writeQueue, m_blockBufferMap, m_controlInfo, m_sqliteDb, m_sqliteDBRootPath,
            m_idGenerator, m_sqliteTaskProduce, m_sqliteTaskConsume, m_blobFileList);
        DBGLOG("put aggregate task to js %s", aggregateDirInfo->m_dirName.c_str());
        if (m_jsPtr->Put(task) == false) {
            ERRLOG("put aggregate file task %s failed", aggregateDirInfo->m_dirName.c_str());
        } else {
            ++m_aggTaskProduce;
        }

        /* Create a new vector for next set of files */
        aggregateDirInfo->m_aggregatedFiles.reset();
        aggregateDirInfo->m_aggregatedFiles = make_shared<vector<FileHandle>>();
        m_aggregateDirMap.m_aggregatorFileSizeTotal -= aggregateDirInfo->m_archiveFileSize;
        m_aggregateDirMap.m_aggregatorBufferCountTotal -= aggregateDirInfo->m_archiveBufferCount;
        aggregateDirInfo->m_archiveFileSize = 0;
        aggregateDirInfo->m_archiveBufferCount = 0;
    }
}

void FileAggregator::CreateSqliteIndexTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo) const
{
    if ((aggregateDirInfo->m_nonAggregatedFiles->size() > 0) ||
        (aggregateDirInfo->m_totalFilesCount == aggregateDirInfo->m_readFailedFilesCount)) {
        string archiveFileName = "";
        CreateTaskForSqliteIndex(aggregateDirInfo->m_nonAggregatedFiles, aggregateDirInfo->m_dirName,
            archiveFileName, 0);
        DBGLOG("put sqliteIndex task to js %s", aggregateDirInfo->m_dirName.c_str());

        /* Create a new vector for next set of files */
        aggregateDirInfo->m_nonAggregatedFiles.reset();
        aggregateDirInfo->m_nonAggregatedFiles = make_shared<std::vector<FileHandle>>();
    }
}

void FileAggregator::CreateSqliteIndexTaskForEmptyDir(const string &dirName)
{
    shared_ptr<vector<FileHandle>> fileHandleList = make_shared<std::vector<FileHandle>>();
    string archiveFileName = "";
    CreateTaskForSqliteIndex(fileHandleList, dirName, archiveFileName, 0);
    DBGLOG("put sqliteIndex task to js %s", dirName.c_str());
}

void FileAggregator::CheckAndCreateAggregateTask(std::shared_ptr<AggregateDirInfo> aggregateDirInfo)
{
    DBGLOG("readCompletedFilesCount = %u, totalFilesCount = %u, "
        "archiveFileSize = %u, maxAggregateFileSize = %u, dirName = %s",
        aggregateDirInfo->m_readCompletedFilesCount,
        aggregateDirInfo->m_totalFilesCount,
        aggregateDirInfo->m_archiveFileSize,
        m_maxAggregateFileSize, aggregateDirInfo->m_dirName.c_str());

    if (aggregateDirInfo->m_readCompletedFilesCount >= aggregateDirInfo->m_totalFilesCount) {
        CreateAggregateTask(aggregateDirInfo);     /* Aggregate all small files and insert index into db */
        CreateSqliteIndexTask(aggregateDirInfo);   /* Insert index for all non aggregated files */
        m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName); /* Remove the dir info from aggregate map */
    } else {
        if ((aggregateDirInfo->m_archiveFileSize >= m_maxAggregateFileSize) ||
            ((aggregateDirInfo->m_aggregatedFiles!= nullptr) &&
            (aggregateDirInfo->m_aggregatedFiles->size() >= MAXFILESINSINGLEBLOBFILE))) {
            CreateAggregateTask(aggregateDirInfo); /* Aggregate all small files and insert index into db */
        }

        if ((aggregateDirInfo->m_nonAggregatedFiles != nullptr) &&
            (aggregateDirInfo->m_nonAggregatedFiles->size() >= MAX_FILES_IN_SINGLE_SQLITE_TASK)) {
            CreateSqliteIndexTask(aggregateDirInfo);
        }
    }
}

void FileAggregator::CreateSqliteIndexTaskForDir(FileHandle &fileHandle) const
{
    shared_ptr<vector<FileHandle>> fileHandleList = make_shared<std::vector<FileHandle>>();
    fileHandleList->push_back(fileHandle);
    string parentDir = fileHandle.m_file->m_fileName.substr(0,
        fileHandle.m_file->m_fileName.find_last_of("/"));

    /* The sqlite index for this dir will be added in the parent directory sqlite file */
    string archiveFileName = "";
    CreateTaskForSqliteIndex(fileHandleList, parentDir, archiveFileName, 0);
    DBGLOG("Put sqlite index task to js. Dir: %s, obskey: %s",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_obsKey.c_str());
}

void FileAggregator::CreateSqliteIndexForAllParentDirs(FileHandle &curFileHandle) const
{
    std::string obsKey = curFileHandle.m_file->m_obsKey;
    std::string parentDir = curFileHandle.m_file->m_dirName;
    DBGLOG("Create sqlite index of all parent dirs for %s, obskey: %s",
           curFileHandle.m_file->m_fileName.c_str(), obsKey.c_str());

#ifdef WIN32
    struct _stat st;
#else
    struct stat st;
#endif

    FileHandle fileHandle {};
    while (!parentDir.empty()) {
        fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
        fileHandle.m_file->SetFlag(IS_DIR);
        fileHandle.m_file->m_fileName = parentDir;
#ifndef WIN32
        if (lstat(parentDir.c_str(), &st) == SUCCESS) {
#else
        if (_stat(parentDir.c_str(), &st) == SUCCESS) { // Hint: If stat operation is needed here?
#endif
            fileHandle.m_file->m_atime = st.st_atime;
            fileHandle.m_file->m_mtime = st.st_mtime;
            fileHandle.m_file->m_ctime = st.st_ctime;
        } else {
            WARNLOG("Stat failed: %s", parentDir.c_str());
            fileHandle.m_file->m_atime = 0;
            fileHandle.m_file->m_mtime = 0;
            fileHandle.m_file->m_ctime = 0;
        }
        fileHandle.m_file->m_onlyFileName = parentDir.substr(parentDir.find_last_of("/") + 1, parentDir.length() - 1);
        if (!obsKey.empty()) {
            size_t pos = obsKey.substr(0, obsKey.size() - 1).find_last_of("/");
            // 目录路径包含了桶名，而对象key不包含，因此目录路径比对象key多一层，最后一层填充桶名
            fileHandle.m_file->m_obsKey = (pos == std::string::npos) ? parentDir.substr(1) : obsKey.substr(0, pos + 1);
        }
        CreateSqliteIndexTaskForDir(fileHandle);
        parentDir = parentDir.substr(0, parentDir.find_last_of("/"));
        obsKey = fileHandle.m_file->m_obsKey;
    }
}

void FileAggregator::HandleAggrFileSet() const
{
    FileHandle fileHandle {};
    fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    for (string eachEntry: m_backupParams.commonParams.aggregateFileSet) {
#ifdef WIN32
        eachEntry = Win32PathUtil::Win32ToPosix(eachEntry);
#endif
        DBGLOG("Each entry: %s", eachEntry.c_str());
        string parentDir = eachEntry.substr(0, eachEntry.find_last_of("/"));
        if (!parentDir.empty()) {
            fileHandle.m_file->m_dirName = parentDir;
            CreateSqliteIndexForAllParentDirs(fileHandle);
        }
    }
}

void FileAggregator::ProcessEmptyFileReadCompletion(FileHandle &fileHandle)
{
    DBGLOG("Enter ProcessEmptyFileReadCompletion: %s", fileHandle.m_file->m_fileName.c_str());

    shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("ProcessEmptyFileRead %s failed, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }
    aggregateDirInfo->m_readCompletedFilesCount++;
    aggregateDirInfo->m_nonAggregatedFiles->push_back(fileHandle);
    ++m_controlInfo->m_aggregatedFiles;
    ++m_controlInfo->m_noOfFilesCopied;
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);

    CheckAndCreateAggregateTask(aggregateDirInfo);
}

void FileAggregator::ProcessReadFailedFiles(FileHandle &fileHandle)
{
    DBGLOG("Enter ProcessReadFailedFiles: %s", fileHandle.m_file->m_fileName.c_str());

    shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("ProcessReadFailedFiles %s failed, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }

    aggregateDirInfo->m_readCompletedFilesCount++;
    aggregateDirInfo->m_readFailedFilesCount++;
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);

    /* Delete contents of this file from blockBuffer map */
    m_blockBufferMap->Delete(fileHandle.m_file->m_fileName);

    CheckAndCreateAggregateTask(aggregateDirInfo);
}

void FileAggregator::ProcessAggrFileReadCompletion(FileHandle &fileHandle)
{
    DBGLOG("Enter ProcessAggrFileReadCompletion: %s", fileHandle.m_file->m_fileName.c_str());
    uint32_t fileSize = 0;

    /* Combine all block buffers and push to Archive list */
    std::shared_ptr<BlockBufferMapQueue> bfrMapQueue = m_blockBufferMap->Get(fileHandle.m_file->m_fileName);
    if (bfrMapQueue == nullptr) {
        ERRLOG("Aggregate file task %s failed, blockBufferMap is empty", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }

    std::unique_lock<std::mutex> lk(bfrMapQueue->m_mtx);
    for (std::set<FileHandle>::iterator it = bfrMapQueue->m_set.begin(); it != bfrMapQueue->m_set.end(); ++it) {
        const FileHandle &fh = *it;
        fileSize += fh.m_block.m_size;
    }
    lk.unlock();

    shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("Aggregation of file: %s failed. aggregateDirInfo is null. Dir: %s",
            fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }

    if (aggregateDirInfo->m_archiveFileSize + fileSize > m_maxAggregateFileSize) {
        DBGLOG("Create aggregate task. archiveFileSize=%u, fileSize: %u, maxAggregateFileSize: %u",
            aggregateDirInfo->m_archiveFileSize, fileSize, m_maxAggregateFileSize);
        CreateAggregateTask(aggregateDirInfo);
    }

    aggregateDirInfo->m_archiveFileSize += fileSize;
    m_aggregateDirMap.m_aggregatorFileSizeTotal += fileSize;
    m_aggregateDirMap.m_aggregatorBufferCountTotal += fileHandle.m_file->m_blockStats.m_totalCnt;
    aggregateDirInfo->m_archiveBufferCount += fileHandle.m_file->m_blockStats.m_totalCnt;
    ++aggregateDirInfo->m_readCompletedFilesCount;
    aggregateDirInfo->m_aggregatedFiles->push_back(fileHandle);
    ++m_controlInfo->m_aggregatedFiles;
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    CheckAndCreateAggregateTask(aggregateDirInfo);
    // for host reader has 50Mb memory limit, if reader reach limit meanwhile aggregateDirInfo in dir maps not reach
    // aggregate threshold, it will stuck. so add this function here , if aggregator total size bigger than reader
    // limit, trigger aggregate task for those dirs which has enough file size ( bigger than average )
    // added for host
    CheckReaderBlock();
    return;
}

bool FileAggregator::IsAggrHang() const
{
    DBGLOG("Enter IsAggrHang, %u size in aggr, %u count buffer in aggr",
        m_aggregateDirMap.m_aggregatorFileSizeTotal, m_aggregateDirMap.m_aggregatorBufferCountTotal);
    if (m_backupParams.srcEngine == BackupIOEngine::POSIX || m_backupParams.srcEngine == BackupIOEngine::WIN32_IO) {
        std::shared_ptr<HostBackupAdvanceParams> srcAdvParams =
            dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.srcAdvParams);
        return m_aggregateDirMap.m_aggregatorFileSizeTotal >= srcAdvParams->maxMemory;
    }
    if (m_backupParams.srcEngine == BackupIOEngine::LIBNFS) {
        // 同时使用maxBufferSize和maxBufferCnt阻塞read，因此需要检查等待聚合中这两个值是否达到最大，判断是否聚合与read互相锁住
        return (m_aggregateDirMap.m_aggregatorFileSizeTotal >= m_backupParams.commonParams.maxBufferSize) ||
               (m_aggregateDirMap.m_aggregatorBufferCountTotal >= m_backupParams.commonParams.maxBufferCnt);
    }
    if (m_backupParams.srcEngine == BackupIOEngine::LIBSMB) {
        return m_aggregateDirMap.m_aggregatorFileSizeTotal >= m_backupParams.commonParams.maxBufferSize;
    }
    return m_aggregateDirMap.m_aggregatorFileSizeTotal >= MEMORY_THRESHOLD_HIGH;
}

void FileAggregator::CheckReaderBlock()
{
    if (!IsAggrHang()) {
        return;
    }
    INFOLOG("Aggregate reach reader memory or count block, force trigger aggregate task for dir size bigger than "
        "average, %u, %u, %u", m_aggregateDirMap.m_aggregatorFileSizeTotal,
        m_aggregateDirMap.m_aggregatorBufferCountTotal, m_aggregateDirMap.GetSize());
    std::vector<std::shared_ptr<AggregateDirInfo>> dirsToBeTrigger = m_aggregateDirMap.GetBiggerSizeDir(
        m_aggregateDirMap.m_aggregatorFileSizeTotal / m_aggregateDirMap.GetSize());
    for (auto dirptr : dirsToBeTrigger) {
        // dirs which has read completed has been processed in CheckAndCreateAggregateTask() function.
        // no need to handle here
        CreateAggregateTask(dirptr); /* Aggregate all small files and insert index into db */
        CreateSqliteIndexTask(dirptr);
    }
}

void FileAggregator::ProcessNonAggrFileRead(FileHandle &fileHandle)
{
    FileDescState srcState = fileHandle.m_file->GetSrcState();
    DBGLOG("Enter ProcessNonAggrFileRead: %s, mode: %u, isADS: %u, state:%d, PROCESSED_BY_AGGR_set: %d",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_mode,
        fileHandle.IsAdsFile(), (int)srcState, fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR));

    if (fileHandle.IsAdsFile()) { // ADS file should not enter this logic and inc failed cnt
        return;
    }

    if (!(srcState == FileDescState::READED || srcState == FileDescState::SRC_CLOSED) ||
        fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR)) {
        return;
    }

    shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("ProcessNonAggrFileRead %s failed, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
        ++m_controlInfo->m_noOfFilesFailed;
        return;
    }
    aggregateDirInfo->m_readCompletedFilesCount++;
    aggregateDirInfo->m_nonAggregatedFiles->push_back(fileHandle);
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);

    CheckAndCreateAggregateTask(aggregateDirInfo);
}

void FileAggregator::ProcessDirectory(FileHandle &fileHandle)
{
    BackupType backupType = m_backupParams.backupType;
    if (fileHandle.m_file->m_fileCount == 0) {
        /* This is an empty dir */
        if ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC)) {
            CreateSqliteIndexTaskForEmptyDir(fileHandle.m_file->m_fileName);

            if (fileHandle.m_file->m_fileName != ".") {
                CreateSqliteIndexTaskForDir(fileHandle);
            }
        }

        PushToWriteQueue(fileHandle);
        return;
    }

    /* This is an non-empty dir */
    shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_fileName);
    if (aggregateDirInfo == nullptr) {
        aggregateDirInfo = make_shared<AggregateDirInfo>();
        aggregateDirInfo->m_aggregatedFiles = make_shared<std::vector<FileHandle>>();
        aggregateDirInfo->m_nonAggregatedFiles = make_shared<std::vector<FileHandle>>();
        aggregateDirInfo->m_dirName = fileHandle.m_file->m_fileName;
        aggregateDirInfo->m_totalFilesCount = fileHandle.m_file->m_fileCount;
        DBGLOG("Insert dir: %s into aggregateDirMap. totalFilesCount: %u",
            fileHandle.m_file->m_fileName.c_str(), aggregateDirInfo->m_totalFilesCount);
        m_aggregateDirMap.Insert(fileHandle.m_file->m_fileName, aggregateDirInfo);
    } else {
        aggregateDirInfo->m_totalFilesCount += fileHandle.m_file->m_fileCount;
        DBGLOG("Added file count: %llu to dir: %s into aggregateDirMap. totalFilesCount: %u",
            fileHandle.m_file->m_fileCount, fileHandle.m_file->m_fileName.c_str(),
            aggregateDirInfo->m_totalFilesCount);
    }

    PushToWriteQueue(fileHandle);

    if ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC)) {
        if (fileHandle.m_file->m_fileName != "." && fileHandle.m_file->m_fileName != "/") { // skip root path entry
            CreateSqliteIndexTaskForDir(fileHandle);
            if (backupType == BackupType::BACKUP_INC) {
                CreateSqliteIndexForAllParentDirs(fileHandle);
            }
        }
    }
}

void FileAggregator::ProcessAggBackup(FileHandle &fileHandle)
{
    FileDescState srcState = fileHandle.m_file->GetSrcState();
    if ((srcState == FileDescState::READ_FAILED) && (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
        ProcessReadFailedFiles(fileHandle);
        return;
    }

    if (FSBackupUtils::IsSpecialFile(fileHandle.m_file->m_mode)) {
        ProcessNonAggrFileRead(fileHandle);
        PushToWriteQueue(fileHandle);
        return;
    }

    if (FSBackupUtils::OnlyGenerateSqlite(m_backupParams.commonParams.genSqlite) &&
        (m_backupParams.commonParams.backupDataFormat != BackupDataFormat::AGGREGATE)) {
        ProcessNonAggrFileRead(fileHandle);
        PushToWriteQueue(fileHandle);
        return;
    }

    if (fileHandle.m_file->m_size == 0) { /* This is an empty file */
        if ((!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
            ProcessEmptyFileReadCompletion(fileHandle);
        }
        return;
    }

    if (fileHandle.m_file->m_size <= m_maxFileSizeToAggregate) { /* This file needs to be aggregated */
        /* Reader可能更早把文件close了 */
        if ((srcState == FileDescState::READED || srcState == FileDescState::SRC_CLOSED) &&
            (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
            ProcessAggrFileReadCompletion(fileHandle);
        }
        return;
    }

    if ((srcState == FileDescState::READED || srcState == FileDescState::SRC_CLOSED) &&
        ((!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR)))) {
        ProcessNonAggrFileRead(fileHandle);
    }
    PushToWriteQueue(fileHandle);
}

void FileAggregator::ProcessFile(FileHandle &fileHandle)
{
    BackupType backupType = m_backupParams.backupType;
    if ((backupType == BackupType::BACKUP_FULL) || (backupType == BackupType::BACKUP_INC)) {
        ProcessAggBackup(fileHandle);
    } else if ((backupType == BackupType::RESTORE) || (backupType == BackupType::FILE_LEVEL_RESTORE))  {
        ProcessAggRestore(fileHandle);
    } else {
        ERRLOG("Invalid type: %d", (int)backupType);
    }
}

void FileAggregator::Aggregate(FileHandle &fileHandle)
{
    DBGLOG("Enter Aggregate. File: %s, size: %llu", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);

    while (!m_started) {
        if (IsAbort()) {
            ERRLOG("FileAggregator not started. Returning as aborted");
            return;
        }
        DBGLOG("FileAggregator not started. waiting");
        Module::SleepFor(std::chrono::seconds(1)); /* Sleep for 1 second */
    }

    /* FileSet needs to be processed only once */
    if (!m_isAggregateFileSetProcessed &&
        ((m_backupParams.backupType == BackupType::BACKUP_FULL) ||
        (m_backupParams.backupType == BackupType::BACKUP_INC))) {
        HandleAggrFileSet();
        m_isAggregateFileSetProcessed = true;
    }
    if (fileHandle.m_file->IsFlagSet(IS_DIR)) { /* This is a directory */
        ProcessDirectory(fileHandle);
    } else { /* This is a file */
        ProcessFile(fileHandle);
    }
}

void FileAggregator::ExecuteSqliteTasks()
{
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::unique_lock<std::mutex> lk(m_blobFileList[i]->m_mtx);
        std::queue<std::shared_ptr<BlobFileDetails>>& blobFileDetailsList = m_blobFileList[i]->m_blobFileDetailsList;
        if (blobFileDetailsList.size() < MAX_BLOB_FILE_PER_SQLITE_TASK) {
            lk.unlock();
            continue;
        }

        auto task = make_shared<SqliteTask>(SqliteEvent::SQL_TASK_PTR_CREATE_INDEX,
                    m_idGenerator, &m_sqliteTaskConsume, m_backupParams);
        while (!blobFileDetailsList.empty()) {
            auto it = blobFileDetailsList.front();
            blobFileDetailsList.pop();
            task->m_blobFileDetailsList.push_back(it);
        }
        lk.unlock();

        if (m_jsSqlPtr[i]->Put(task, true, JOB_SCHED_PUT_TIMEOUT_MILLISEC) == false) {
            DBGLOG("Put SQLITE task for the blob files timedout, retry later");
            lk.lock();
            for (size_t i = 0; i < task->m_blobFileDetailsList.size(); i++) {
                blobFileDetailsList.emplace(task->m_blobFileDetailsList[i]);
            }
            lk.unlock();
            continue;
        }
        m_sqliteTaskProduce++;
    }
}

void FileAggregator::CheckMemory()
{
    // Get the zip files we saved, who does not have any FHs to write to,
    // but simply waiting for new FH to come from reader
    // Free them and remove fom blockBufferMap

    while (!m_started) {
        if (IsAbort()) {
            ERRLOG("FileAggregator not started. Returning as aborted");
            return;
        }
        DBGLOG("FileAggregator not started. waiting");
        Module::SleepFor(std::chrono::seconds(1)); /* Sleep for 1 second */
    }

    std::unordered_map<std::string, std::shared_ptr<AggregateDirInfo>>::iterator it =
    m_aggregateDirMap.m_aggregateDirInfoMap.begin();
    for (; it != m_aggregateDirMap.m_aggregateDirInfoMap.end() ; it++)  {
        std::shared_ptr<AggregateDirInfo> dirMap;
        dirMap = it->second;
        std::shared_ptr<AggregateNormalInfo> normalInfo;
        std::unordered_map<std::string, std::shared_ptr<AggregateNormalInfo>>::iterator itInfo =
            dirMap->m_aggFileMap.m_aggregateFileInfoMap.begin();
        for (; itInfo != dirMap->m_aggFileMap.m_aggregateFileInfoMap.end(); itInfo++)  {
            std::string zipFileName;
            normalInfo = itInfo->second;
            zipFileName = itInfo->first;
            DBGLOG("zip FileName = %s", zipFileName.c_str());
            // this blob file read completed and no normal file handles
            if (normalInfo->IsFlagSet(IS_READ_COMPLETE) && normalInfo->aggregatedfileHandle.m_file != nullptr &&
                normalInfo->aggregatedfileHandle.m_file->GetUnAggTaskCnt() == 0 &&
                normalInfo->normalFileHandles != nullptr && (normalInfo->normalFileHandles->size() == 0) &&
                normalInfo->numOfNormalFiles > normalInfo->numOfFilesRestored &&
                ++normalInfo->emptyCnt > FREE_BUFFER_AFTER_NO_FILES_MAX_TIMES) {
                // need to clean up the zip file from buffer map
                DBGLOG("clean the zip file from buffer map FileName = %s, size: %d",
                    normalInfo->aggregatedfileHandle.m_file->m_fileName.c_str(),
                    normalInfo->aggregatedfileHandle.m_file->m_size);
                m_blockBufferMap->Delete(normalInfo->aggregatedfileHandle.m_file->m_fileName);
                normalInfo->aggregatedfileHandle.m_file.reset();
                normalInfo->ClearFlag(IS_READ_COMPLETE);
                normalInfo->SetFlag(NEED_PUSH_BLOB_FH);
            }
        }
    }
}

void FileAggregator::ProcessAggRestoreEmptyFile(FileHandle &fileHandle)
{
    std::string fileName = fileHandle.m_file->m_onlyFileName;
    DBGLOG("Enter. File Path: %s, OnlyFileName = %s, size: %d",
        fileHandle.m_file->m_fileName.c_str(), fileName.c_str(), fileHandle.m_file->m_size);
    // do we need to verify the file presence in db ??
    /* Push to writer queue to create the destination file */
    PushToWriteQueue(fileHandle); /* To open the dest file */
    fileHandle.m_block.m_seq = 1;
    PushToWriteQueue(fileHandle); /* To write data/metadata to dest file */
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    ++m_controlInfo->m_emptyFiles;
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("fileName %s, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
    } else {
        aggregateDirInfo->m_readCompletedFilesCount++;
        CleanUpTheDirMap(aggregateDirInfo);
    }
    DBGLOG("Exit. emptyFiles=%d", m_controlInfo->m_emptyFiles.load());
    return;
}

void FileAggregator::FillAndPushToReadQueue(FileHandle &fileHandle, const string aggfileName,
    uint64_t aggFileSize, shared_ptr<AggregateDirInfo> aggDirInfo, uint32_t numOfNormalFiles)
{
    fileHandle.m_file->ClearFlag(IS_DIR);

    // create the FH put in the read queue
    FileHandle blobfileHandle;
    blobfileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    blobfileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName + PATH_SEPERATOR + aggfileName;
    blobfileHandle.m_file->m_scannermode = "bm";
    blobfileHandle.m_file->m_dirName = fileHandle.m_file->m_dirName;
    blobfileHandle.m_file->m_onlyFileName = aggfileName;
    blobfileHandle.m_file->m_size    = aggFileSize;
    // this parameter used to idendity in aggregate queue after read is completed so that don't push to writeQueue
    blobfileHandle.m_file->SetFlag(AGGREGATE_GEN_FILE);
    PushToReadQueue(blobfileHandle);

    ++m_controlInfo->m_unarchiveFiles;
    DBGLOG("m_controlInfo->m_unarchiveFiles: %llu", m_controlInfo->m_unarchiveFiles.load());
    ++aggDirInfo->m_genFilesCount;
    // update the m_aggFileMap  with the nomral file FH and the aggregated/zip FH
    AddNormalnGenFileToAggregateFileMap(fileHandle, blobfileHandle, aggDirInfo, numOfNormalFiles);
}

void FileAggregator::FillAndPushToReadQueueOnly(FileHandle &fileHandle, const string aggfileName,
    shared_ptr<AggregateNormalInfo> aggFileInfo, shared_ptr<AggregateDirInfo> aggDirInfo)
{
    // create the FH put in the read queue
    FileHandle blobfileHandle;
    blobfileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    blobfileHandle.m_file->m_fileName = fileHandle.m_file->m_dirName + PATH_SEPERATOR + aggfileName;
    blobfileHandle.m_file->m_scannermode = "bm";
    blobfileHandle.m_file->m_dirName = fileHandle.m_file->m_dirName;
    blobfileHandle.m_file->m_onlyFileName = aggfileName;
    blobfileHandle.m_file->ClearFlag(IS_DIR);
    blobfileHandle.m_file->m_size = aggFileInfo->aggFileSize;
    // this parameter used to idendity in aggregatequeue after read is completed so that don't push to writeQueue
    blobfileHandle.m_file->SetFlag(AGGREGATE_GEN_FILE);
    PushToReadQueue(blobfileHandle);
    ++m_controlInfo->m_unarchiveFiles;
    ++aggDirInfo->m_genFilesCount;
    DBGLOG("m_controlInfo->m_unarchiveFiles: %llu aggDirInfo->m_genFilesCount: %u",
        m_controlInfo->m_unarchiveFiles.load(), aggDirInfo->m_genFilesCount);
    aggFileInfo->ClearFlag(NEED_PUSH_BLOB_FH);

    return;
}

void FileAggregator::AddNormalnGenFileToAggregateFileMap(FileHandle &fileHandle,
    FileHandle &zipfileHandle, std::shared_ptr<AggregateDirInfo> aggregateDirInfo, uint32_t numOfNormalFiles)
{
    std::shared_ptr<AggregateNormalInfo> aggFileInfo = aggregateDirInfo->m_aggFileMap.GetInfo(
        zipfileHandle.m_file->m_onlyFileName);
    DBGLOG("Enter AddNormalnGenFileToAggregateFileMap. File: %s, size: %d",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);
    if (aggFileInfo == nullptr) {
        DBGLOG("Inside aggFileInfo == nullptr.");
        aggFileInfo = make_shared<AggregateNormalInfo>();
        aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
        aggFileInfo->normalFileHandles->push_back(fileHandle);
        aggFileInfo->numOfNormalFiles = numOfNormalFiles;
        aggFileInfo->ClearFlag(IS_READ_COMPLETE);
        aggFileInfo->ClearFlag(NEED_PUSH_BLOB_FH);
        aggFileInfo->aggFileSize = zipfileHandle.m_file->m_size;
        aggFileInfo->numOfFilesRestored = 0;
        aggFileInfo->emptyCnt = 0;
        aggregateDirInfo->m_aggFileMap.Insert(zipfileHandle.m_file->m_onlyFileName, aggFileInfo);
    } else {
        DBGLOG("Inside  else of aggFileInfo == nullptr.");
        aggFileInfo->normalFileHandles->push_back(fileHandle);
        aggFileInfo->numOfNormalFiles = numOfNormalFiles;
    }
    ++m_controlInfo->m_aggrRestoreInMemoryFhCnt;
    return;
}

void FileAggregator::PushToReadQueue(FileHandle &fileHandle)
{
    DBGLOG("Enter PushToReadQueue. File: %s, size: %d", fileHandle.m_file->m_fileName.c_str(),
        fileHandle.m_file->m_size);
    m_readQueue->Push(fileHandle);
    return;
}

void FileAggregator::CreateUnAggregateTask(std::shared_ptr<AggregateNormalInfo> aggFileInfo,
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo, bool isAllFilesRcvd)
{
    std::string zipFileName = aggFileInfo->aggregatedfileHandle.m_file->m_onlyFileName;
    std::string dirName = aggFileInfo->aggregatedfileHandle.m_file->m_dirName;
    DBGLOG("Enter, zip filename: %s num of aggFileInfo->normalFileHandles: %u "
        "aggFileInfo->numOfNormalFiles: %d aggFileInfo->isReadCompleted: %d isAllFilesRcvd: %d",
        zipFileName.c_str(), aggFileInfo->normalFileHandles->size(), aggFileInfo->numOfNormalFiles,
        aggFileInfo->IsFlagSet(IS_READ_COMPLETE), isAllFilesRcvd);

    UnaggregatedTaskParms params;
    aggFileInfo->emptyCnt = 0;
    aggFileInfo->aggregatedfileHandle.m_file->IncUnAggTaskCnt();
    params.event = AggregateEvent::UNAGGREGATE_FILES;
    params.backupParams = m_backupParams;
    params.fileHandleList = aggFileInfo->normalFileHandles;
    params.dirPath = dirName;
    params.writeQueue = m_writeQueue;
    params.blockBufferMap = m_blockBufferMap;
    params.controlInfo = m_controlInfo;
    params.sqliteDb = m_sqliteDb;
    params.sqliteDBRootPath = m_sqliteDBRootPath;
    params.blobfileHandle = aggFileInfo->aggregatedfileHandle;
    params.isDeleteBlobFile = isAllFilesRcvd;
    auto task = make_shared<FileAggregateTask>(params);
    DBGLOG("put aggregate restore task to js for zip filename %s", zipFileName.c_str());
    if (m_jsPtr->Put(task) == false) {
        ERRLOG("put aggregate file task failed for zip file %s", zipFileName.c_str());
        return ;
    }
    // cleanup the maps
    uint32_t numOfFilesRestore = aggFileInfo->normalFileHandles->size();
    for (uint32_t i = 0; i < numOfFilesRestore; i++) {
        aggregateDirInfo->m_InfoMap.Erase((*aggFileInfo->normalFileHandles)[i].m_file->m_onlyFileName);
    }
    m_controlInfo->m_aggrRestoreInMemoryFhCnt -= numOfFilesRestore;
    aggFileInfo->normalFileHandles.reset();
    aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
    if (isAllFilesRcvd) {
        DBGLOG("CreateUnAggregateTask m_aggFileMap.Erase filename %s", zipFileName.c_str());
        aggregateDirInfo->m_aggFileMap.Erase(zipFileName);
    } else {
        aggFileInfo->numOfFilesRestored += numOfFilesRestore;
    }
    ++m_aggTaskProduce;
    CleanUpTheDirMap(aggregateDirInfo);
    DBGLOG("Exit, zip filename %s, total aggTask produce for now: %d", zipFileName.c_str(), m_aggTaskProduce.load());
    return;
}

void FileAggregator::FillTheInfoMap(std::vector<AggSqlRestoreInfo>& vecNormalFiles,
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo, const std::string &aggFileName, uint32_t &numOfNormalFiles)
{
    for (size_t i = 0; i< vecNormalFiles.size(); i++) {
        if (!aggregateDirInfo->m_InfoMap.Exists(vecNormalFiles[i].normalFileName)) {
            std::shared_ptr<AggRestoreInfo> info = make_shared<AggRestoreInfo>();
            info->blobFileName = aggFileName;
            info->normalFileOffset = vecNormalFiles[i].normalFileOffset;
            info->normalFileSize = vecNormalFiles[i].normalFileSize;
            DBGLOG("put nomal file: %s offset %u and normalfileSize %u aggregate file: %s info in the  m_InfoMap ",
                vecNormalFiles[i].normalFileName.c_str(), vecNormalFiles[i].normalFileOffset, info->normalFileSize,
                aggFileName.c_str());
            aggregateDirInfo->m_InfoMap.Insert(vecNormalFiles[i].normalFileName, info);
            numOfNormalFiles++;
        } else {
            DBGLOG("already part of the map for normal file: %s  aggregate filename: %s ",
                vecNormalFiles[i].normalFileName.c_str(), aggFileName.c_str());
        }
    }
    return;
}

/*
 * 恢复时，获取指定的sqlite文件
 * 输入的是sqliteDBRootPath的相对路径
 */
std::string FileAggregator::GetDbFile(std::string dirPath)
{
    if (dirPath == ".") {
        dirPath = "";
    }

    // 第1种场景：没有按备份子任务生成多个sqlite，只有一个 copymetadata.sqlite
    if (!m_backupParams.commonParams.useSubJobSqlite) {
        DBGLOG("use copymetadata.sqlite. useSubJobSqlite is false.");
        return dirPath + Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    }

    // 第2种场景：存在多个sqlite，但是又没有指定哪一个sqlite，需要遍历查找该目录所有的sqlite
    if (m_backupParams.commonParams.controlFile.empty()) {
        DBGLOG("Not unique sqlite.");
        return "";
    }

    // 第3种场景：存在多个sqlite，且通过controlFile指定了sqlite名称，并且该sqlite存在
    auto pos = m_backupParams.commonParams.controlFile.rfind(".txt");
    std::string dstDbName = m_backupParams.commonParams.controlFile.substr(0, pos) + ".sqlite";
    std::string dbFilePath = dirPath + Module::PATH_SEPARATOR + dstDbName;
    if (FSBackupUtils::Exists(m_sqliteDBRootPath + dbFilePath)) {
        DBGLOG("Exist, use sqlite: %s", dbFilePath.c_str());
        return dbFilePath;
    }

    /*
     * 第4种场景：存在多个sqlite，且通过controlFile指定了sqlite名称，并且该sqlite不存在，原因：
     * 对于少文件数的目录，多个以control文件命名的sqlite可能合并到copymetadata.sqlite
     */
    DBGLOG("use copymetadata.sqlite, cannnot find %s", dbFilePath.c_str());
    return dirPath + Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
}

int FileAggregator::GetRecordFromMultiSqliteByName(FileHandle &fileHandle, std::string& dbFile,
    std::vector<IndexDetails>& vecIndexInfo)
{
    std::string& dirPath = fileHandle.m_file->m_dirName;
    std::string& fileName = fileHandle.m_file->m_onlyFileName;
    std::string type = fileHandle.IsDir() ? "d" : "f";

    std::string fullPath = (dirPath == ".") ? m_sqliteDBRootPath : m_sqliteDBRootPath + dirPath;
    std::vector<std::string> fileList {};
    std::vector<std::string> dirList {};
    int ret = FSBackupUtils::GetFileWithSubDirListInDir(fullPath, fileList, dirList);
    if (ret != Module::SUCCESS) {
        ERRLOG("Get file list failed, dir: %s", fullPath.c_str());
        return ret;
    }

    IndexDetails indexInfo {};
    for (const auto &item : fileList) {
        dbFile = item.substr(m_sqliteDBRootPath.length());
        if (indexInfo.QueryIndexInfoByName(m_sqliteDb, dbFile, vecIndexInfo, fileName, type) != Module::SUCCESS) {
            ERRLOG("Query failed by name %s", fileName.c_str());
            return Module::FAILED;
        }
        // 文件名是唯一索引，只会有一条记录，找到退出
        if (!vecIndexInfo.empty()) {
            break;
        }
    }

    return Module::SUCCESS;
}

int FileAggregator::GetFileMetaByName(FileHandle &fileHandle)
{
    if (fileHandle.m_file->m_type != FileType::OBJECT) {
        return Module::SUCCESS;
    }

    if (!fileHandle.m_file->m_xattr.empty()) {
        return Module::SUCCESS;
    }

    DBGLOG("Query meta data from sqlite, key: %s", fileHandle.m_file->m_obsKey.c_str());

    std::string dbFile = GetDbFile(fileHandle.m_file->m_dirName);
    std::string& fileName = fileHandle.m_file->m_onlyFileName;
    std::vector<IndexDetails> vecIndexInfo {};
    int ret = Module::SUCCESS;
    if (dbFile.empty()) {
        ret = GetRecordFromMultiSqliteByName(fileHandle, dbFile, vecIndexInfo);
    } else {
        IndexDetails indexInfo {};
        ret = indexInfo.QueryIndexInfoByName(m_sqliteDb, dbFile, vecIndexInfo, fileName, "f");
    }
    if (ret != Module::SUCCESS) {
        ERRLOG("Query failed by name %s", fileName.c_str());
        return ret;
    }

    if (vecIndexInfo.empty()) {
        ERRLOG("Query no meta data by name %s", fileName.c_str());
        return Module::FAILED;
    }

    auto &metaInfo = vecIndexInfo.front();
    DBGLOG("vector size %d, sqlite meta data: %s", vecIndexInfo.size(), metaInfo.m_metaData.c_str());
    FileAttr fAttr {};
    Module::JsonHelper::JsonStringToStruct(metaInfo.m_metaData, fAttr);
    for (auto &meta : fAttr.meta) {
        fileHandle.m_file->m_xattr.push_back({meta.k, meta.v});
    }

    return Module::SUCCESS;
}

void FileAggregator::ProcessAggRestoreNonAggrFile(FileHandle &fileHandle)
{
    DBGLOG("fileName: %s, dirName: %s", fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_dirName.c_str());
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    if (fileHandle.IsAdsFile()) {
        DBGLOG("fileName %s is ADS file", fileHandle.m_file->m_fileName.c_str());
        return;
    }

    GetFileMetaByName(fileHandle);

    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("fileName %s, aggregateDirInfo is null", fileHandle.m_file->m_fileName.c_str());
        return;
    }
    aggregateDirInfo->m_readCompletedFilesCount++;
    CleanUpTheDirMap(aggregateDirInfo);
}

int FileAggregator::DeleteSqliteRecord(FileHandle& fileHandle)
{
    std::string dbFile {};
    std::string& fileName = fileHandle.m_file->m_onlyFileName;
    std::vector<IndexDetails> vecIndexInfo {};
    int ret = GetRecordFromMultiSqliteByName(fileHandle, dbFile, vecIndexInfo);
    if (ret != Module::SUCCESS) {
        ERRLOG("Get db file failed by %s", fileName.c_str());
        return ret;
    }

    if (vecIndexInfo.empty()) {
        ERRLOG("Query no record by name %s", fileName.c_str());
        return Module::FAILED;
    }

    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr = m_sqliteDb->QueryDbFromMap(dbFile);
    if (sqlInfoPtr == nullptr) {
        ERRLOG("Get sqlInfoPtr failed by %s", dbFile.c_str());
        return Module::FAILED;
    }
    // 前面查询是只读打开，而后面是写sqlite，因此先关闭，之后flags设置为读写打开
    m_sqliteDb->Disconnect(dbFile, sqlInfoPtr);

    IndexDetails indexInfo {};
    ret = indexInfo.DeleteIndexInfoByName(m_sqliteDb, dbFile, fileName);
    if (ret != Module::SUCCESS) {
        ERRLOG("Delete db record %s failed.", fileName.c_str());
        return ret;
    }

    DBGLOG("Delete record %s success.", fileName.c_str());
    return Module::SUCCESS;
}

void FileAggregator::CleanUpTheDirMap(std::shared_ptr<AggregateDirInfo> aggregateDirInfo)
{
    DBGLOG("Enter CleanUpTheDirMap");
    DBGLOG("readCompletedFilesCount = %u, totalFilesCount = %u "
        "m_genFilesCount= %u m_aggFileMap.size() = %u"
        "m_InfoMap.size()= %u dirName = %s",
        aggregateDirInfo->m_readCompletedFilesCount,
        aggregateDirInfo->m_totalFilesCount,
        aggregateDirInfo->m_genFilesCount,
        aggregateDirInfo->m_aggFileMap.Size(),
        aggregateDirInfo->m_InfoMap.Size(),
        aggregateDirInfo->m_dirName.c_str());

    if (aggregateDirInfo->m_readCompletedFilesCount >=
        (aggregateDirInfo->m_totalFilesCount + aggregateDirInfo->m_genFilesCount)) {
        // clean all the zip files
        // clean the m_aggFileMap
        for (size_t i = 0; i < aggregateDirInfo->m_aggFileMap.Size() ; i++) {
            shared_ptr<AggregateNormalInfo> info = aggregateDirInfo->m_aggFileMap.GetNPop();
            if ((info != nullptr) && (info->aggregatedfileHandle.m_file != nullptr)) {
                DBGLOG("zipfile name = %s numOfNormalFiles = %u isReadCompleted= %u ",
                    info->aggregatedfileHandle.m_file->m_fileName.c_str(),
                    info->numOfNormalFiles,
                    info->IsFlagSet(IS_READ_COMPLETE));
                CleanTheBlobFile(info);
            }
        }
        DBGLOG("Erase the dir from dirmap dirName = %s", aggregateDirInfo->m_dirName.c_str());
        m_aggregateDirMap.Erase(aggregateDirInfo->m_dirName); /* Remove the dir info from aggregate map */
    }
    DBGLOG("Exit CleanUpTheDirMap");
}

void FileAggregator::CleanTheBlobFile(shared_ptr<AggregateNormalInfo> info) const
{
    bool flag = false;
    static uint32_t protectCnt = 0;
    DBGLOG("Enter CleanTheBlobFile");
    if ((info->normalFileHandles != nullptr) && (info->normalFileHandles->size() > 0)) {
        flag = true;
        ERRLOG(" do we need to create the task, no need naaa");
    }
    // clean the zip file
    if (!flag) {
        DBGLOG("CleanTheBlobFile");
        while (info->aggregatedfileHandle.m_file != nullptr) {
            if (info->aggregatedfileHandle.m_file->GetUnAggTaskCnt() == 0) {
                DBGLOG("Delete Zip file %s", info->aggregatedfileHandle.m_file->m_fileName.c_str());
                m_blockBufferMap->Delete(info->aggregatedfileHandle.m_file->m_fileName);
                protectCnt = 0;
                break;
            } else {
                DBGLOG("Sleep CleanTheBlobFile");
                Module::SleepFor(std::chrono::seconds(1)); /* Sleep for 1 second */
                protectCnt++;
            }
            // just to over come infinite loop
            if (protectCnt >= FIVECOUNT) {
                protectCnt = 0;
                WARNLOG("protectCnt reached max so clean");
                INFOLOG("Delete Zip file %s", info->aggregatedfileHandle.m_file->m_fileName.c_str());
                m_blockBufferMap->Delete(info->aggregatedfileHandle.m_file->m_fileName);
                break;
            }
        }
    }
    DBGLOG("Exit CleanTheBlobFile");
}

void FileAggregator::ProcessAggRestore(FileHandle &fileHandle)
{
    DBGLOG("Enter. File Path: %s, size: %d, state: %d",
        fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size, fileHandle.m_file->GetSrcState());
    uint32_t mode = fileHandle.m_file->m_mode;

    if (FSBackupUtils::OnlyGenerateSqlite(m_backupParams.commonParams.genSqlite) &&
        (m_backupParams.commonParams.backupDataFormat != BackupDataFormat::AGGREGATE)) {
        if (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR)) {
            ProcessAggRestoreNonAggrFile(fileHandle);
        }
        PushToWriteQueue(fileHandle);
        return;
    }

    if (!fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
        GetFileMetaByName(fileHandle);
    }

    if (FSBackupUtils::IsSpecialFile(mode)) {
        if (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR)) {
            ProcessAggRestoreNonAggrFile(fileHandle);
        }
        PushToWriteQueue(fileHandle);
    } else if ((fileHandle.m_file->m_size == 0) && (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
        ProcessAggRestoreEmptyFile(fileHandle);
    } else if ((fileHandle.m_file->m_size <= m_maxFileSizeToAggregate) &&
               (!fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) &&
               (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
        // if file size is less than maxFileSizeToAggregate & not generated file
        if ((fileHandle.m_file->GetSrcState() == FileDescState::READED) ||
            (fileHandle.m_file->GetSrcState() == FileDescState::AGGREGATED)) {
            ProcessAggRestoreNormalFile(fileHandle);
        }
    } else { /* This file may be normal with more size or aggregated/generated/zip file */
        ProcessAggRestoreInteraction(fileHandle);
    }
    DBGLOG("Exit.");
    return;
}

void FileAggregator::ProcessAggRestoreInteraction(FileHandle &fileHandle)
{
    if (fileHandle.m_file->IsFlagSet(AGGREGATE_GEN_FILE)) {
        if (((fileHandle.m_file->GetSrcState() == FileDescState::READED) ||
                (fileHandle.m_file->GetSrcState() == FileDescState::SRC_CLOSED)) &&
            (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
            ProcessAggRestoreBlobFile(fileHandle);
        } else if ((fileHandle.m_file->GetSrcState() == FileDescState::READ_FAILED) &&
                   (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR))) {
            ERRLOG(" Readed zip fil failed.File Name: %s, size: %d",
                fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_file->m_size);
            ProcessAggRestoreReadFailedBlobFile(fileHandle);
        } else {
            DBGLOG("Partially Readed zip. File: %s, size: %d",
                fileHandle.m_file->m_fileName.c_str(),
                fileHandle.m_file->m_size);
        }
    } else {
        /* This logic for not aggregated files */
        if (!fileHandle.m_file->IsFlagSet(PROCESSED_BY_AGGR)) {
            ProcessAggRestoreNonAggrFile(fileHandle);
        }
        PushToWriteQueue(fileHandle);
    }
}
int FileAggregator::GetNormalFilesFromMultiSqliteByName(std::string& dirPath, std::string& fileName,
    AggSqlRestoreQueryInfo& info, std::vector<AggSqlRestoreInfo>& vecNormalFiles)
{
    std::string fullPath = (dirPath == ".") ? m_sqliteDBRootPath : m_sqliteDBRootPath + dirPath;
    std::vector<std::string> fileList {};
    std::vector<std::string> dirList {};
    int ret = FSBackupUtils::GetFileWithSubDirListInDir(fullPath, fileList, dirList);
    if (ret != Module::SUCCESS) {
        ERRLOG("Get file list failed, dir: %s", fullPath.c_str());
        return ret;
    }

    IndexDetails indexInfo {};
    for (const auto &item : fileList) {
        std::string dbFile = item.substr(m_sqliteDBRootPath.length());
        if (indexInfo.QueryNormalFilesByName(m_sqliteDb, fileName, dbFile, info, vecNormalFiles) != 0) {
            ERRLOG("Query failed by name %s", fileName.c_str());
            return Module::FAILED;
        }
        // 每个control文件一个恢复任务，对应一个sqlite，因此不会跨sqlite，只要在一个里面找到，就不需要再找其它的sqlite
        if (!vecNormalFiles.empty()) {
            break;
        }
    }

    return Module::SUCCESS;
}

void FileAggregator::ProcessAggRestoreNormalFile(FileHandle &fileHandle)
{
    std::string& fileName = fileHandle.m_file->m_onlyFileName;
    std::string aggfileName;
    std::shared_ptr<AggRestoreInfo> resInfo = nullptr;
    DBGLOG("Enter ProcessAggRestoreNormalFile. File Path: %s OnlyFileName = %s size: %d",
        fileHandle.m_file->m_fileName.c_str(), fileName.c_str(), fileHandle.m_file->m_size);
    fileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(fileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        m_controlInfo->m_unaggregatedFaildFiles += 1;
        m_controlInfo->m_noOfFilesFailed += 1;
        ERRLOG("ProcessAggRestoreNormalFile: %s aggregateDirInfo is null unaggregatedFaildFiles: %llu,"
            "totalFailed: %llu", fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_unaggregatedFaildFiles.load(),
            m_controlInfo->m_noOfFilesFailed.load());
        return;
    }
    aggregateDirInfo->m_readCompletedFilesCount++;
    // if the normal file is part of the m_InfoMap then just keep the FH in the m_aggFileMap
    if (aggregateDirInfo->m_InfoMap.Exists(fileName)) {
        resInfo = aggregateDirInfo->m_InfoMap.GetInfo(fileName);
        fileHandle.m_file->m_aggregateFileOffset = resInfo->normalFileOffset;
        fileHandle.m_file->m_size = resInfo->normalFileSize;
        NormalFileInFileMap(resInfo->blobFileName, fileHandle, aggregateDirInfo);
    } else {
        std::string dbFile = GetDbFile(fileHandle.m_file->m_dirName);
        uint32_t numOfNormalFiles = 0;
        AggSqlRestoreQueryInfo info {};
        std::vector<AggSqlRestoreInfo> vecNormalFiles {};
        int ret = Module::SUCCESS;
        if (dbFile.empty()) {
            ret = GetNormalFilesFromMultiSqliteByName(fileHandle.m_file->m_dirName, fileName, info, vecNormalFiles);
        } else {
            IndexDetails indexInfo {};
            ret = indexInfo.QueryNormalFilesByName(m_sqliteDb, fileName, dbFile, info, vecNormalFiles);
        }
        if (ret != Module::SUCCESS) {
            m_controlInfo->m_unaggregatedFaildFiles += 1;
            m_controlInfo->m_noOfFilesFailed += 1;
            ERRLOG("Query normal files failed for file: %s  unaggregatedFaildFiles: %llu, totalFailed: %llu",
                fileHandle.m_file->m_fileName.c_str(), m_controlInfo->m_unaggregatedFaildFiles.load(),
                m_controlInfo->m_noOfFilesFailed.load());
            CleanUpTheDirMap(aggregateDirInfo);
            return;
        }
        fileHandle.m_file->m_aggregateFileOffset = info.fileOffset;
        fileHandle.m_file->m_size = info.normalFileSize;
        FillTheInfoMap(vecNormalFiles, aggregateDirInfo, info.blobFileName, numOfNormalFiles);
        // create the FH put in the read queue
        FillAndPushToReadQueue(fileHandle, info.blobFileName, info.blobFileSize, aggregateDirInfo, numOfNormalFiles);
    }
    CleanUpTheDirMap(aggregateDirInfo);
    return;
}

void FileAggregator::NormalFileInFileMap(std::string zipFileName, FileHandle &fileHandle,
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo)
{
    std::shared_ptr<AggregateNormalInfo> aggFileInfo = aggregateDirInfo->m_aggFileMap.GetInfo(zipFileName);
    DBGLOG("Enter NormalFileInFileMap. zipfilename : %s Normal File: %s size: % offsetinzipfile %lld",
        zipFileName.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size,
        fileHandle.m_file->m_aggregateFileOffset);

    if (aggFileInfo == nullptr) {
        ERRLOG("this condition should not happen");
        return;
    } else if (!aggFileInfo->IsFlagSet(IS_BLOB_READ_FAILED)) {
        DBGLOG("Inside aggFileInfo->normalFileHandles.push_back");
        aggFileInfo->normalFileHandles->push_back(fileHandle);
        ++m_controlInfo->m_aggrRestoreInMemoryFhCnt;
    }
    // push the zip file handle to reader
    if ((aggFileInfo->IsFlagSet(NEED_PUSH_BLOB_FH)) && (!aggFileInfo->IsFlagSet(IS_READ_COMPLETE))) {
        FillAndPushToReadQueueOnly(fileHandle, zipFileName, aggFileInfo, aggregateDirInfo);
    }

    uint32_t numOfFilesRestore = aggFileInfo->normalFileHandles->size();
    // if the aggregated file /zip file read completed so create the task
    if ((aggFileInfo->IsFlagSet(IS_READ_COMPLETE)) &&
        ((aggFileInfo->aggregatedfileHandle.m_file->GetSrcState() == FileDescState::READED) ||
         (aggFileInfo->aggregatedfileHandle.m_file->GetSrcState() == FileDescState::SRC_CLOSED))) {
        bool flag;
        // if all the files of a zip file/ aggregated fil are restored
        flag = (aggFileInfo->numOfNormalFiles == (aggFileInfo->numOfFilesRestored + numOfFilesRestore));
        // if all the files in dir read completed
        flag = (flag || (aggregateDirInfo->m_readCompletedFilesCount) >=
            (aggregateDirInfo->m_totalFilesCount + aggregateDirInfo->m_genFilesCount));

        DBGLOG("call CreateUnAggregateTask numOfFilesRestore: %d", numOfFilesRestore);
        CreateUnAggregateTask(aggFileInfo, aggregateDirInfo, flag);
    } else if (aggFileInfo->IsFlagSet(IS_BLOB_READ_FAILED)) {
        DBGLOG("Blob file read failed. blobFilename: %s, received related Normal File: %s, size: %d",
            zipFileName.c_str(), fileHandle.m_file->m_fileName.c_str(), fileHandle.m_file->m_size);

        // need to consider the num of normal files related to aggregated/zip file failed here
        m_controlInfo->m_unaggregatedFaildFiles += 1;
        m_controlInfo->m_noOfFilesFailed += 1;
        DBGLOG("m_controlInfo->m_unaggregatedFaildFiles: %llu, totalFailed: %llu",
            m_controlInfo->m_unaggregatedFaildFiles.load(), m_controlInfo->m_noOfFilesFailed.load());
        m_controlInfo->m_aggrRestoreInMemoryFhCnt -= numOfFilesRestore;
        CleanUpTheDirMap(aggregateDirInfo);
    } else {
        DBGLOG(" just pushed the FH");
    }
    return;
}

bool FileAggregator::GetBlobFileHandleWithSingleBuffer(FileHandle &oldBlobFileHandle,
    FileHandle &newBlobFileHandle)
{
    uint64_t offset = 0;
    std::shared_ptr<BlockBufferMapQueue> bfrMapQueue = m_blockBufferMap->Get(oldBlobFileHandle.m_file->m_fileName);
    if (bfrMapQueue == nullptr) {
        ERRLOG("blockBufferMap is empty for blobFile: %s", oldBlobFileHandle.m_file->m_fileName.c_str());
        return false;
    }

    if (bfrMapQueue->Size() == 1) {
        DBGLOG("The blockBufferMap is already having only single block buffer for blob file: %s",
            oldBlobFileHandle.m_file->m_fileName.c_str());
        std::unique_lock<std::mutex> lk(bfrMapQueue->m_mtx);
        std::set<FileHandle>::iterator it = bfrMapQueue->m_set.begin();
        if (it == bfrMapQueue->m_set.end()) {
            ERRLOG("Blockbuffer map is empty for blob file: %s", oldBlobFileHandle.m_file->m_fileName.c_str());
            return false;
        }
        newBlobFileHandle = *it;
        return true;
    }

    DBGLOG("The blockBufferMap has more block buffers so converting to single block for blob file: %s",
        oldBlobFileHandle.m_file->m_fileName.c_str());
    uint8_t* shBuffer = new uint8_t[oldBlobFileHandle.m_file->m_size]();
    if (shBuffer == nullptr) {
        ERRLOG("Memory allocation failed for blob file: %s", oldBlobFileHandle.m_file->m_fileName.c_str());
        return false;
    }

    std::unique_lock<std::mutex> lk(bfrMapQueue->m_mtx);
    for (std::set<FileHandle>::iterator it = bfrMapQueue->m_set.begin(); it != bfrMapQueue->m_set.end(); ++it) {
        const FileHandle &eachBlock = *it;
        int ret = memcpy_s((shBuffer + offset), eachBlock.m_block.m_size, eachBlock.m_block.m_buffer,
            eachBlock.m_block.m_size);
        if (ret != 0) {
            ERRLOG("memcpy_s failed. ret: %d", ret);
            delete[] shBuffer;
            return false;
        }
        offset += eachBlock.m_block.m_size;
    }
    lk.unlock();

    newBlobFileHandle = oldBlobFileHandle; /* Copy old blob contents */
    // delete old and push the new buffer
    m_blockBufferMap->Delete(oldBlobFileHandle.m_file->m_fileName);
    newBlobFileHandle.m_block.m_seq = 1 ;
    newBlobFileHandle.m_block.m_size = newBlobFileHandle.m_file->m_size;
    newBlobFileHandle.m_block.m_offset = 0;
    newBlobFileHandle.m_block.m_buffer = shBuffer;
    m_blockBufferMap->Add(newBlobFileHandle.m_file->m_fileName, newBlobFileHandle);
    return true;
}

void FileAggregator::ProcessAggRestoreBlobFile(FileHandle &blobFileHandle)
{
    std::string& blobFileName = blobFileHandle.m_file->m_onlyFileName;
    bool flag = false;
    DBGLOG("Enter. File Path: %s, size: %d", blobFileHandle.m_file->m_fileName.c_str(), blobFileHandle.m_file->m_size);
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(blobFileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ++m_controlInfo->m_noOfFilesFailed;
        ERRLOG("ProcessAggRestoreBlobFile %s, aggregateDirInfo is null", blobFileHandle.m_file->m_fileName.c_str());
        return;
    }
    shared_ptr<AggregateNormalInfo> aggFileInfo = aggregateDirInfo->m_aggFileMap.GetInfo(blobFileName);
    if (aggFileInfo == nullptr) {
        ERRLOG("aggFileInfo is null for file %s.", blobFileName.c_str());
        return;
    }

    blobFileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    if (GetBlobFileHandleWithSingleBuffer(blobFileHandle, aggFileInfo->aggregatedfileHandle) != true) {
        ERRLOG("Get blob file handle failed. blob file: %s", blobFileHandle.m_file->m_fileName.c_str());
        return;
    }
    aggFileInfo->SetFlag(IS_READ_COMPLETE);
    aggregateDirInfo->m_readCompletedFilesCount++;
    uint32_t numOfFilesRestore = aggFileInfo->normalFileHandles->size();

    // if all the files of a zip file/ aggregated fil are restored
    flag = (aggFileInfo->numOfNormalFiles == (aggFileInfo->numOfFilesRestored + numOfFilesRestore));
    // if all the files in dir read completed
    flag = (flag || (aggregateDirInfo->m_readCompletedFilesCount) >=
        (aggregateDirInfo->m_totalFilesCount + aggregateDirInfo->m_genFilesCount));

    CreateUnAggregateTask(aggFileInfo, aggregateDirInfo, flag);
    DBGLOG("Exit. blobFileName %s", blobFileName.c_str());
    return ;
}

void FileAggregator::ProcessAggRestoreReadFailedBlobFile(FileHandle &zipFileHandle)
{
    std::string& zipFileName = zipFileHandle.m_file->m_onlyFileName;
    DBGLOG("Enter ProcessAggRestoreReadFailedBlobFile. File Path: %s, OnlyFileName = %s, size: %d",
        zipFileHandle.m_file->m_fileName.c_str(), zipFileName.c_str(), zipFileHandle.m_file->m_size);
    std::shared_ptr<AggregateDirInfo> aggregateDirInfo = m_aggregateDirMap.GetDirInfo(zipFileHandle.m_file->m_dirName);
    if (aggregateDirInfo == nullptr) {
        ERRLOG("ProcessAggRestoreReadFailedBlobFile %s, aggregateDirInfo is null",
            zipFileHandle.m_file->m_fileName.c_str());
        return;
    }
    shared_ptr<AggregateNormalInfo> aggFileInfo = aggregateDirInfo->m_aggFileMap.GetInfo(zipFileName);
    if (aggFileInfo == nullptr) {
        ERRLOG("aggFileInfo is null for file %s.", zipFileName.c_str());
        return;
    }

    aggFileInfo->SetFlag(IS_BLOB_READ_FAILED);
    zipFileHandle.m_file->SetFlag(PROCESSED_BY_AGGR);
    aggregateDirInfo->m_readCompletedFilesCount++;
    uint32_t numOfFilesRestore = aggFileInfo->normalFileHandles->size();
    DBGLOG("supposed to restore num of normal files. numOfFilesRestore: %llu", numOfFilesRestore);
    aggFileInfo->normalFileHandles.reset();
    aggFileInfo->normalFileHandles = make_shared<std::vector<FileHandle>>();
    // need to consider the num of normal files related to aggregated/zip file failed here
    m_controlInfo->m_unaggregatedFaildFiles += numOfFilesRestore;
    m_controlInfo->m_noOfFilesFailed += numOfFilesRestore;
    m_controlInfo->m_noOfFilesReadFailed++;
    m_controlInfo->m_aggrRestoreInMemoryFhCnt -= numOfFilesRestore;
    ERRLOG("m_controlInfo->m_unaggregatedFaildFiles: %llu, totalFailed: %llu",
        m_controlInfo->m_unaggregatedFaildFiles.load(), m_controlInfo->m_noOfFilesFailed.load());
    CleanUpTheDirMap(aggregateDirInfo);
    DBGLOG("Exit ProcessAggRestoreReadFailedBlobFile zipFileName %s", zipFileName.c_str());
}

void FileAggregator::CreateTaskForSqliteIndex(std::shared_ptr<std::vector<FileHandle>> fileHandleList,
    const std::string &dirName, std::string &archiveFileName, uint64_t fileSize) const
{
    auto blobFile = std::make_shared<BlobFileDetails>();
    blobFile->aggregatedFileSize = fileSize;
    blobFile->archiveFileName = archiveFileName;
    blobFile->m_dirPath =  dirName;
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;
    for (uint64_t i = 0; i < fileHandleList->size(); i++) {
        FileHandle& fileHandle = (*fileHandleList)[i];
        SmallFileDesc t_fileDesc;
        t_fileDesc.m_obsKey = fileHandle.m_file->m_obsKey;
        t_fileDesc.m_onlyFileName = fileHandle.m_file->m_onlyFileName;
        t_fileDesc.m_aggregateFileOffset = fileHandle.m_file->m_aggregateFileOffset;
        t_fileDesc.m_size = fileHandle.m_file->m_size;
        t_fileDesc.m_ctime = fileHandle.m_file->m_ctime;
        t_fileDesc.m_mtime = fileHandle.m_file->m_mtime;
        t_fileDesc.m_flag = fileHandle.m_file->m_flag;
#ifdef _OBS
        FileAttr fAttr {};
        for (auto &item : fileHandle.m_file->m_xattr) {
            ObsMetaData mdata {};
            mdata.k = item.first;
            mdata.v = item.second;
            fAttr.meta.emplace_back(mdata);
        }
        if (!Module::JsonHelper::StructToJsonString(fAttr, t_fileDesc.m_metaData)) {
            ERRLOG("Save meta data failed for %s", fileHandle.m_file->m_obsKey.c_str());
        }
#endif
        DBGLOG("smallFileDesc: %s", t_fileDesc.m_onlyFileName.c_str());
        blobFile->m_smallFileDescList.push_back(t_fileDesc);
    }

    /* Ensure to give an idx which is between 0 to 7 */
    uint16_t hashIndex = FSBackupUtils::GetHashIndexForSqliteTask(blobFile->m_dirPath);
    std::unique_lock<std::mutex> lk(m_blobFileList[hashIndex]->m_mtx);
    m_blobFileList[hashIndex]->m_blobFileDetailsList.emplace(blobFile);
    m_blobFileList[hashIndex]->m_numOfBlobsFilesInserted++;
    lk.unlock();
}

bool FileAggregator::IsBlobListEmpty()
{
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::lock_guard<std::mutex> lk(m_blobFileList[i]->m_mtx);
        if (m_blobFileList[i]->m_blobFileDetailsList.size() != 0) {
            return false;
        }
    }
    return true;
}

void FileAggregator::PrintSqliteTaskDistribution()
{
    INFOLOG("Hash Distribution of Sqlite job");
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        DBGLOG(" Slot[%u]--%u", i, m_blobFileList[i]->m_numOfBlobsFilesInserted);
    }
}

uint32_t FileAggregator::CanAcceptMoreWork()
{
    uint32_t count = 0;
    for (uint16_t i = 0; i < m_blobFileList.size(); i++) {
        std::lock_guard<std::mutex> lk(m_blobFileList[i]->m_mtx);
        count += m_blobFileList[i]->m_blobFileDetailsList.size();
    }

    if (count > MAX_SQL_TASK_RUNNING) {
        return false;
    }

    return true;
}
