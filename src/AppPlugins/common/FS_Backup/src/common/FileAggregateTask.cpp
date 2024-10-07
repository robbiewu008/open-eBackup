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
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>
#include <codecvt>
#include <sys/stat.h>
#include "common/Thread.h"
#include "log/Log.h"
#include "FSBackupUtils.h"
#include "FileSystemUtil.h"
#include "FileAggregateTask.h"

using namespace std;
using namespace FS_Backup;

namespace {
    const string INDEX_DB_FILE_NAME = "copymetadata.sqlite";
    const int QUEUE_TIMEOUT_MILLISECOND = 200;
    constexpr int SUCCESS = 0;
    constexpr int FAILED  = -1;
    const uint32_t KB_IN_BYTES = 1024;
    const int CREATE_DB_FLAGS = SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX;
    const uint64_t RETRY_TIMEOUT = 30000;
    const uint32_t RETRY_COUNT = 10;
    const int32_t SQLITE_TASK_FAIL_CODE = -5;
}

void FileAggregateTask::Exec()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    switch (m_event) {
        case AggregateEvent::AGGREGATE_FILES_AND_CREATE_INDEX: {
            DoAggrFilesAndCreateIndex();
            break;
        }
        case AggregateEvent::UNAGGREGATE_FILES: {
            HandleUnaggregation();
            break;
        }

        default:
            break;
    }
}

string FileAggregateTask::GetUniqueIdStr() const
{
    return to_string(m_idGenerator->GenerateId());
}

string FileAggregateTask::GenerateArchiveFileName()
{
    string filename = GetUniqueIdStr() + ".dpa.emei.blob";
    return filename;
}

void FileAggregateTask::PushArchiveFileToWriter(FileHandle& fileHandle)
{
    DBGLOG("Enter PushArchiveFileToWriter: %s", fileHandle.m_file->m_fileName.c_str());
    uint32_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t fileSize = fileHandle.m_file->m_size;
    uint64_t fullBlockNum = fileSize / blockSize;
    uint32_t remainSize = fileSize % blockSize;

    fileHandle.m_file->m_blockStats.m_totalCnt = (remainSize == 0) ? fullBlockNum : fullBlockNum + 1;
    DBGLOG("total blocks: %u, file size: %llu",
        fileHandle.m_file->m_blockStats.m_totalCnt.load(), fileHandle.m_file->m_size);

    /* One FH for creation of file in the destination. Small file will open while write data,so don't need to push */
    m_outputFileHandleList.push_back(fileHandle);

    /* Form new FH after aggregation */
    uint8_t *buffer = fileHandle.m_block.m_buffer;
    for (uint64_t i = 0; i < fullBlockNum; i++) {
        FileHandle tmpFileHandle;
        tmpFileHandle.m_block.m_buffer = new uint8_t[blockSize];
        tmpFileHandle.m_block.m_size = blockSize;
        tmpFileHandle.m_block.m_offset = blockSize * i;
        tmpFileHandle.m_block.m_seq = i + 1;

        if (memcpy_s(tmpFileHandle.m_block.m_buffer, blockSize, buffer, blockSize) != 0) {
            ERRLOG("memcpy failed");
        }

        buffer += blockSize;
        tmpFileHandle.m_file = fileHandle.m_file;
        m_outputFileHandleList.push_back(tmpFileHandle);
    }

    if (remainSize != 0) {
        FileHandle tmpFileHandle;
        tmpFileHandle.m_block.m_buffer = new uint8_t[remainSize];
        tmpFileHandle.m_block.m_size = remainSize;
        tmpFileHandle.m_block.m_offset = blockSize * fullBlockNum;
        tmpFileHandle.m_block.m_seq = fullBlockNum + 1;

        if (memcpy_s(tmpFileHandle.m_block.m_buffer, remainSize, buffer, remainSize) != 0) {
            ERRLOG("memcpy failed");
        }

        tmpFileHandle.m_file = fileHandle.m_file;
        m_outputFileHandleList.push_back(tmpFileHandle);
    }
}

void FileAggregateTask::DeleteFileList(string &parentDir, vector<string> &blobFileList) const
{
    DBGLOG("delete file list in %s", parentDir.c_str());
    for (string &eachBlobFile : blobFileList) {
        DBGLOG("Deleting blob file: %s", eachBlobFile.c_str());

#ifdef _NAS
        std::string blobFilePath = parentDir + Module::PATH_SEPARATOR + eachBlobFile;
        FileHandle fileHandle;
        fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
        fileHandle.m_file->m_fileName = blobFilePath;
        fileHandle.m_file->SetDstState(FileDescState::LINK_DEL);

        while (!m_writeQueue->WaitAndPush(fileHandle, QUEUE_TIMEOUT_MILLISECOND)) {
            DBGLOG("Wait and push timeout. File: %s", fileHandle.m_file->m_fileName.c_str());
        }
#else
        std::string dataPath = dynamic_pointer_cast<HostBackupAdvanceParams>(m_backupParams.dstAdvParams)->dataPath;
        std::string blobFileFullPath = FSBackupUtils::JoinPath(
            FSBackupUtils::JoinPath(dataPath, parentDir), eachBlobFile);
        if (!FSBackupUtils::RemoveFile(blobFileFullPath)) {
            ERRLOG("failed to delete previous blob file: %s", blobFileFullPath.c_str());
        }
#endif
    }
}

// 备份时，生成指定文件名的sqlite，此时文件不存在，在插件记录前会创建
std::string FileAggregateTask::GetDbFile(std::string &dirPath)
{
    std::string dbFile = (dirPath == ".") ? "" : dirPath;

    if (!m_backupParams.commonParams.useSubJobSqlite || m_backupParams.commonParams.controlFile.empty()) {
        dbFile += Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    } else {
        auto pos = m_backupParams.commonParams.controlFile.rfind(".txt");
        std::string dstDbName = m_backupParams.commonParams.controlFile.substr(0, pos) + ".sqlite";
        dbFile += Module::PATH_SEPARATOR + dstDbName;
    }

    return dbFile;
}

int FileAggregateTask::GetNormalFilesFromMultiSqliteByName(std::vector<std::string> fileList, std::string& fileName,
    AggSqlRestoreQueryInfo& info, std::vector<AggSqlRestoreInfo>& vecNormalFiles)
{
    IndexDetails indexInfo {};
    for (const auto &item : fileList) {
        std::string dbFile = item.substr(m_sqliteDBRootPath.length());
        if (indexInfo.QueryNormalFilesByName(m_sqliteDb, fileName, dbFile, info, vecNormalFiles) != 0) {
            ERRLOG("Query failed by name %s", fileName.c_str());
            return Module::FAILED;
        }
        if (!vecNormalFiles.empty()) {
            break;
        }
    }

    return Module::SUCCESS;
}

void FileAggregateTask::DeleteOldBlobFiles()
{
    std::set<std::string> normalFileList;
    for (FileHandle &eachFile : (*m_inputFileHandleList)) {
        normalFileList.emplace(eachFile.m_file->m_onlyFileName);
        DBGLOG("Added file: %s to normalFileList", eachFile.m_file->m_onlyFileName.c_str());
    }

    std::string dirName = (*m_inputFileHandleList)[0].m_file->m_dirName;
    std::string dirFullPath = (dirName == ".") ? m_sqliteDBRootPath : m_sqliteDBRootPath + dirName;
    std::vector<std::string> fileList {};
    std::vector<std::string> dirList {};
    int ret = FSBackupUtils::GetFileWithSubDirListInDir(dirFullPath, fileList, dirList);
    if (ret != Module::SUCCESS) {
        ERRLOG("Get file list failed, dir: %s", dirFullPath.c_str());
        return;
    }

    std::string dbFile = GetDbFile((*m_inputFileHandleList)[0].m_file->m_dirName);
    std::vector<std::string> blobFileList;
    std::vector<AggSqlRestoreInfo> vecNormalFiles;
    AggSqlRestoreQueryInfo sqlQueryInfo;
    IndexDetails indexInfo {};
    while (!normalFileList.empty()) {
        auto itr = normalFileList.begin();
        std::string fileName = *itr;
        normalFileList.erase(fileName);
        DBGLOG("Deleted file: %s from normalFileList", fileName.c_str());
        if (!FSBackupUtils::Exists(m_sqliteDBRootPath + dbFile)) {
            ret = GetNormalFilesFromMultiSqliteByName(fileList, fileName, sqlQueryInfo, vecNormalFiles);
        } else {
            ret = indexInfo.QueryNormalFilesByName(m_sqliteDb, fileName, dbFile, sqlQueryInfo, vecNormalFiles);
        }
        if (ret != 0) {
            DBGLOG("Query %s not success in dbFile: %s", fileName.c_str(), dbFile.c_str());
            continue;
        }

        blobFileList.push_back(sqlQueryInfo.blobFileName);
        for (auto &eachFile : vecNormalFiles) {
            if (normalFileList.erase(eachFile.normalFileName) != 0) {
                DBGLOG("Deleted file: %s from normal file list.", eachFile.normalFileName.c_str());
            }
        }
    }

    DeleteFileList((*m_inputFileHandleList)[0].m_file->m_dirName, blobFileList);
}

void FileAggregateTask::CreateTaskForSqliteIndex(FileHandle &fileHandle, std::string &archiveFileName)
{
    auto blobFile = std::make_shared<BlobFileDetails>();
    blobFile->aggregatedFileSize = fileHandle.m_file->m_size;
    blobFile->archiveFileName = archiveFileName;
    blobFile->m_dirPath =  m_dirPath;
    blobFile->m_sqliteDb = m_sqliteDb;
    blobFile->m_sqliteDBRootPath = m_sqliteDBRootPath;
    for (uint64_t i = 0; i < m_inputFileHandleList->size(); i++) {
        FileHandle& smallFile = (*m_inputFileHandleList)[i];
        SmallFileDesc t_fileDesc;
        t_fileDesc.m_obsKey = smallFile.m_file->m_obsKey;
        t_fileDesc.m_onlyFileName = smallFile.m_file->m_onlyFileName;
        t_fileDesc.m_aggregateFileOffset = smallFile.m_file->m_aggregateFileOffset;
        t_fileDesc.m_size = smallFile.m_file->m_size;
        t_fileDesc.m_ctime = smallFile.m_file->m_ctime;
        t_fileDesc.m_mtime = smallFile.m_file->m_mtime;
        t_fileDesc.m_flag = smallFile.m_file->m_flag;
#ifdef _OBS
        FileAttr fAttr {};
        for (auto &item : smallFile.m_file->m_xattr) {
            ObsMetaData mdata {};
            mdata.k = item.first;
            mdata.v = item.second;
            fAttr.meta.emplace_back(mdata);
        }
        if (!Module::JsonHelper::StructToJsonString(fAttr, t_fileDesc.m_metaData)) {
            ERRLOG("Save meta data failed for %s", smallFile.m_file->m_obsKey.c_str());
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

void FileAggregateTask::DoAggrFilesAndCreateIndex()
{
    uint32_t maxAggregateFileSize = m_backupParams.commonParams.maxAggregateFileSize * KB_IN_BYTES;

    FileHandle fileHandle;
    fileHandle.m_file = make_shared<FileDesc>(m_backupParams.srcEngine, m_backupParams.dstEngine);
    fileHandle.m_file->m_dirName = m_dirPath;
    fileHandle.m_file->m_onlyFileName = GenerateArchiveFileName();
    fileHandle.m_file->m_fileName = m_dirPath + Module::PATH_SEPARATOR + fileHandle.m_file->m_onlyFileName;
    fileHandle.m_file->ClearFlag(IS_DIR);
    fileHandle.m_file->m_size = maxAggregateFileSize;
    fileHandle.m_file->m_originalFileCount = 0;
    fileHandle.m_block.m_buffer = new uint8_t[maxAggregateFileSize];

    uint8_t *archiveBuffer = fileHandle.m_block.m_buffer;
    int32_t ret = 0;
    int32_t offset = 0;
    int32_t fileIndex = 0;
    m_result = SUCCESS;

    for (FileHandle &eachFile : (*m_inputFileHandleList)) {
        /* Update the offset of this file's content inside the archive file */
        (*m_inputFileHandleList)[fileIndex++].m_file->m_aggregateFileOffset = offset;

        std::shared_ptr<BlockBufferMapQueue> bfrMapQueue = m_blockBufferMap->Get(eachFile.m_file->m_fileName);
        if (bfrMapQueue == nullptr) {
            ERRLOG("Aggregate file task %s failed, blockBufferMap is empty", eachFile.m_file->m_fileName.c_str());
            m_result = FAILED;
            delete[] fileHandle.m_block.m_buffer;
            return;
        }

        std::unique_lock<std::mutex> lk(bfrMapQueue->m_mtx);
        for (std::set<FileHandle>::iterator it = bfrMapQueue->m_set.begin(); it != bfrMapQueue->m_set.end(); ++it) {
            const FileHandle &eachBlock = *it;
            ret = memcpy_s((archiveBuffer + offset), eachBlock.m_block.m_size, eachBlock.m_block.m_buffer,
                eachBlock.m_block.m_size);
            if (ret != 0) {
                ERRLOG("memcpy_s failed. ret: %d", ret);
                m_result = FAILED;
                delete[] fileHandle.m_block.m_buffer;
                return;
            }

            offset += eachBlock.m_block.m_size;
        }
        lk.unlock();
        ++fileHandle.m_file->m_originalFileCount;
    }

    fileHandle.m_file->m_size = offset; /* Assign the actual size */
    DBGLOG("Allocated size: %u, Actual size: %u", maxAggregateFileSize, offset);

    if (m_backupParams.commonParams.isReExecutedTask) {
        DeleteOldBlobFiles();
    }

    CreateTaskForSqliteIndex(fileHandle, fileHandle.m_file->m_onlyFileName);
    PushArchiveFileToWriter(fileHandle);
    delete[] fileHandle.m_block.m_buffer;  // 释放临时聚合文件内存
}

int32_t SqliteTask::CreateSqliteDb(std::shared_ptr<BlobFileDetails> blobFileDetails, string &dbFile)
{
    sqlite3_stmt *sqlStmt = nullptr;
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr { nullptr };
    string dbFullPath = blobFileDetails->m_sqliteDb->GetMetaPath() + dbFile;
    sqlInfoPtr = blobFileDetails->m_sqliteDb->PrepareDb(dbFile, CREATE_DB_FLAGS);
    if (sqlInfoPtr == nullptr) {
        ERRLOG("PrepareDb failed for dbFile: %s", dbFullPath.c_str());
        return FAILED;
    }

    /* Allow only one thread to operate on the same DB file */
    std::lock_guard<std::mutex> lock(sqlInfoPtr->m_sqliteCoreInfoMutex);
    int ret = FAILED;
    uint32_t retryCnt = 0;
    while ((ret != SUCCESS) && (retryCnt++ < RETRY_COUNT)) {
        /* Begin transaction & prepare the sql stmt */
        ret = blobFileDetails->m_sqliteDb->PrepareSqlStmt(dbFile, &sqlStmt, sqlInfoPtr);
        if (ret != SUCCESS) {
            ERRLOG("PrepareSqlStmt failed: %d for dbFile: %s, retrying: %d", ret, dbFullPath.c_str(), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            int32_t retLocal = blobFileDetails->m_sqliteDb->DeleteAndPrepareDb(dbFile, sqlInfoPtr, CREATE_DB_FLAGS);
            if ((retLocal != SUCCESS) || (sqlInfoPtr == nullptr)) {
                ERRLOG("DeleteAndPrepareDb failed: %d for dbFile: %s", ret, dbFullPath.c_str());
                return FAILED;
            }
            continue;
        }
        /* Bind & Step */
        ret = InsertIndexInfo(blobFileDetails, sqlStmt, sqlInfoPtr);
        if (ret != SUCCESS) {
            blobFileDetails->m_sqliteDb->FinalizeSqlStmt(sqlStmt, sqlInfoPtr);
            ERRLOG("InsertIndexInfo failed: %d for dbFile: %s, retrying: %d", ret, dbFullPath.c_str(), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            int32_t retLocal = blobFileDetails->m_sqliteDb->DeleteAndPrepareDb(dbFile, sqlInfoPtr, CREATE_DB_FLAGS);
            if ((retLocal != SUCCESS) || (sqlInfoPtr == nullptr)) {
                ERRLOG("DeleteAndPrepareDb failed: %d for dbFile: %s", ret, dbFullPath.c_str());
                return FAILED;
            }
            continue;
        }

        ret = blobFileDetails->m_sqliteDb->FinalizeSqlStmt(sqlStmt, sqlInfoPtr);
        if (ret != SUCCESS) {
            ERRLOG("FinalizeSqlStmt failed: %d for dbFile: %s, (retrying:%d)", ret, dbFullPath.c_str(), retryCnt);
            Module::SleepFor(std::chrono::milliseconds(RETRY_TIMEOUT));
            int32_t retLocal = blobFileDetails->m_sqliteDb->DeleteAndPrepareDb(dbFile, sqlInfoPtr, CREATE_DB_FLAGS);
            if ((retLocal != SUCCESS) || (sqlInfoPtr == nullptr)) {
                ERRLOG("DeleteAndPrepareDb failed: %d for dbFile: %s", ret, dbFullPath.c_str());
                return FAILED;
            }
            continue;
        }
    }

    return ret;
}

// 备份时，生成指定文件名的sqlite，此时文件不存在，在插件记录前会创建
std::string SqliteTask::GetDbFile(std::string &dirPath)
{
    string dbFile = (dirPath == ".") ? "" : dirPath;

    if (!m_backupParams.commonParams.useSubJobSqlite || m_backupParams.commonParams.controlFile.empty()) {
        dbFile += Module::PATH_SEPARATOR + INDEX_DB_FILE_NAME;
    } else {
        auto pos = m_backupParams.commonParams.controlFile.rfind(".txt");
        std::string dstDbName = m_backupParams.commonParams.controlFile.substr(0, pos) + ".sqlite";
        dbFile += Module::PATH_SEPARATOR + dstDbName;
    }

    return dbFile;
}

std::string SqliteTask::GetObsKey(const std::string &key, const std::string &type)
{
    if (key.empty()) {
        return "";
    }

    std::string delimiter = Module::PATH_SEPARATOR;
    size_t pos = key.rfind(delimiter);
    if (pos == std::string::npos) {
        return key;
    }

    if (key.substr(pos) == delimiter) {
        // 以 delimiter 结尾, 查找倒数第二个 delimiter
        pos = key.substr(0, pos).rfind(delimiter);
        if (pos == std::string::npos) {
            pos = 0;
        } else {
            pos = pos + delimiter.size();
        }
    } else {
        pos = pos + delimiter.size();
    }

    std::string subKeyStr = key.substr(pos);
    if (type == "d") {
        // 目录结尾的"/"不需要展示
        size_t subPos = subKeyStr.rfind(delimiter);
        if ((subPos != std::string::npos) && (subKeyStr.substr(subPos) == delimiter)) {
            subKeyStr = subKeyStr.substr(0, subPos);
        }
    }

    return subKeyStr;
}

void SqliteTask::DoCreateSqliteIndex(std::shared_ptr<BlobFileDetails> blobFileDetails, uint16_t &index)
{
    int64_t time1 = FSBackupUtils::GetMilliSecond();
    if (blobFileDetails->m_dirPath != ".") {
        std::string fullPath = blobFileDetails->m_sqliteDBRootPath + Module::PATH_SEPARATOR +
                               blobFileDetails->m_dirPath;
        FSBackupUtils::RecurseCreateDirectory(fullPath);
    } else {
        FSBackupUtils::RecurseCreateDirectory(blobFileDetails->m_sqliteDBRootPath);
    }
    int64_t time2 = FSBackupUtils::GetMilliSecond();
    string dbFile = GetDbFile(blobFileDetails->m_dirPath);
    DBGLOG("Dir creation time: %llu ms, dbFile %s", (time2 - time1), dbFile.c_str());

    int32_t ret = CreateSqliteDb(blobFileDetails, dbFile);
    if (ret != SUCCESS) {
        string dbFullPath = blobFileDetails->m_sqliteDb->GetMetaPath() + dbFile;
        ERRLOG("Create sqlite db for dbFile: %s failed", dbFullPath.c_str());
        m_result = -SQLITE_TASK_FAIL_CODE;
        m_failedIndex.push_back(index);
    }
    int64_t time3 = FSBackupUtils::GetMilliSecond();
    DBGLOG("Sqlite insertion time for %u records: %llu ms, archiveFileName: %s, dir: %s",
        blobFileDetails->m_smallFileDescList.size(), (time3 - time2),
        blobFileDetails->archiveFileName.c_str(), blobFileDetails->m_dirPath.c_str());
}

int32_t SqliteTask::InsertIndexInfo(std::shared_ptr<BlobFileDetails> blobFileDetails, sqlite3_stmt *sqlStmt,
    std::shared_ptr<SQLiteCoreInfo> sqlInfoPtr)
{
    IndexDetails indexInfo;
    uint32_t len = blobFileDetails->m_smallFileDescList.size();
    std::vector<SmallFileDesc>& inputFileHandleList = blobFileDetails->m_smallFileDescList;
    for (uint32_t i = 0; i < len; i++) {
        DBGLOG("File: %s, archiveFileName: %s", inputFileHandleList[i].m_onlyFileName.c_str(),
            blobFileDetails->archiveFileName.c_str());
        if (inputFileHandleList[i].m_onlyFileName.empty()) {
            continue;
        }

        indexInfo.m_uuid = GetUniqueIdStr();
        indexInfo.m_type = inputFileHandleList[i].IsFlagSet(IS_DIR) ? "d" : "f";
        indexInfo.m_metaData = inputFileHandleList[i].m_metaData;
        indexInfo.m_actualFileName = inputFileHandleList[i].m_onlyFileName;
        indexInfo.m_actualFileSize = inputFileHandleList[i].m_size;
        indexInfo.m_metaFileName = "";
        indexInfo.m_offset = inputFileHandleList[i].m_aggregateFileOffset;
        indexInfo.m_createTime = inputFileHandleList[i].m_ctime;
        indexInfo.m_modifyTime = inputFileHandleList[i].m_mtime;
        indexInfo.m_aggregateFileName = blobFileDetails->archiveFileName;
        indexInfo.m_aggregatedFileSize = blobFileDetails->aggregatedFileSize;
        indexInfo.m_resType = GetObsKey(inputFileHandleList[i].m_obsKey, indexInfo.m_type);

        int32_t ret = indexInfo.InsertIndexInfo(blobFileDetails->m_sqliteDb, sqlStmt, sqlInfoPtr);
        if (ret != SUCCESS) {
            ERRLOG("InsertIndexInfo failed. file: %s, idx: %u, len: %u, ret: %d archiveFileName:%s .",
                indexInfo.m_actualFileName.c_str(), i, len, ret, blobFileDetails->archiveFileName.c_str());
            return FAILED;
        }
    }
    return SUCCESS;
}

void FileAggregateTask::HandleUnaggregation()
{
    DBGLOG("Enter HandleUnaggregation. File: %s, m_size: %llu",
        m_blobfileHandle.m_file->m_onlyFileName.c_str(), m_blobfileHandle.m_file->m_size);

    uint8_t *archiveBuffer = m_blobfileHandle.m_block.m_buffer;
    if (archiveBuffer == nullptr) {
        ERRLOG("blobfileHandle buffer is null. blobFile: %s", m_blobfileHandle.m_file->m_onlyFileName.c_str());
        m_result = FAILED;
        return;
    }

    for (uint32_t i = 0 ; i < m_inputFileHandleList->size(); i++) {
        FileHandle &fileHandle = (*m_inputFileHandleList)[i];
        uint64_t offsetInAggr = fileHandle.m_file->m_aggregateFileOffset;
        uint64_t lsize = fileHandle.m_file->m_size;
        DBGLOG("original file name: %s file size: %lld offset in zip file: %lld",
            fileHandle.m_file->m_onlyFileName.c_str(), lsize, offsetInAggr);
        uint8_t* buffer;
#ifdef WIN32
        buffer = new uint8_t[lsize]();
#else
        if (S_ISLNK(fileHandle.m_file->m_mode)) {
            buffer = new uint8_t [lsize + 1]();
        } else {
            buffer = new uint8_t [lsize]();
        }
#endif
        if (buffer == nullptr) {
            m_result = FAILED;
            ERRLOG("Memory allocation failed for file %s ", fileHandle.m_file->m_onlyFileName.c_str());
            return;
        }
        int ret = memcpy_s(buffer, lsize, (archiveBuffer + offsetInAggr), lsize);
        if (ret != 0) {
            ERRLOG(" memcpy_s failed for original file name: %s file size: %lld offset in zip file: %lld",
                fileHandle.m_file->m_onlyFileName.c_str(), lsize, offsetInAggr);
            ERRLOG("from zip file %s, ret: %d", m_blobfileHandle.m_file->m_fileName.c_str(), ret);
            m_result = FAILED;
            return;
        }
#ifndef WIN32
        if (S_ISLNK(fileHandle.m_file->m_mode)) {
            buffer[lsize] = '\0';
        }
#endif
        CreateFileHandles(buffer, fileHandle);
    }

    m_blobfileHandle.m_file->DecUnAggTaskCnt();
    m_result = SUCCESS;
    return;
}

void FileAggregateTask::CreateFileHandles(uint8_t *buffer, FileHandle& fileHandle)
{
    if (fileHandle.m_file->m_size <= m_backupParams.commonParams.blockSize) {
        return CreateNPushToWriteQueue(buffer, fileHandle, true);
    } else {
        return CreateNPushToWriteQueue(buffer, fileHandle);
    }
}

void FileAggregateTask::CreateNPushToWriteQueue(uint8_t* buffer, FileHandle& fileHandle,
    bool isSingleBlk)
{
    DBGLOG("Normal file m_size: %llu", fileHandle.m_file->m_size);
    string fileName = fileHandle.m_file->m_fileName;
    uint64_t fileSize = fileHandle.m_file->m_size;
    uint32_t blockSize = m_backupParams.commonParams.blockSize;
    uint64_t fullBlockNum = fileSize / blockSize;
    uint32_t remainSize = fileSize % blockSize;
    fileHandle.m_file->m_blockStats.m_totalCnt = (remainSize == 0) ? fullBlockNum : fullBlockNum + 1;
    /* Push to m_outputFileHandleList to create the destination file */
    m_outputFileHandleList.push_back(fileHandle);
    DBGLOG("file size: %llu, blockSize: %u, fullBlockNum: %llu, remainSize: %u",
        fileHandle.m_file->m_size, blockSize, fullBlockNum, remainSize);
    if (isSingleBlk) {
        if ((fullBlockNum == 0 && remainSize != 0) || (fullBlockNum == 1 && remainSize == 0)) {
            fileHandle.m_block.m_size = fileSize;
            fileHandle.m_block.m_offset = 0;
            fileHandle.m_block.m_seq = 1;
            fileHandle.m_block.m_buffer = buffer;
            m_outputFileHandleList.push_back(fileHandle);
        } else {
            ERRLOG("something wrong for =%s", fileName.c_str());
            delete[] buffer;
        }
        return;
    }

    // copy the data to different file handles & buffer maps and push the FH to writeQueue
    for (uint64_t i = 0; i < fullBlockNum; i++) {
        fileHandle.m_block.m_buffer = new uint8_t[blockSize];
        fileHandle.m_block.m_size = blockSize;
        fileHandle.m_block.m_offset = blockSize * i;
        fileHandle.m_block.m_seq = i + 1;
        if (memcpy_s(fileHandle.m_block.m_buffer, blockSize, (buffer + fileHandle.m_block.m_offset), blockSize) != 0) {
                ERRLOG("memcpy_s failed for file %s  blocknum %llu ", fileName.c_str(), i);
                delete[] buffer;
                return ;
        }
        m_outputFileHandleList.push_back(fileHandle);
    }
    if (remainSize != 0) {
        fileHandle.m_block.m_buffer = new uint8_t[remainSize];
        fileHandle.m_block.m_size = remainSize;
        fileHandle.m_block.m_offset = blockSize * fullBlockNum;
        fileHandle.m_block.m_seq = fullBlockNum + 1;
        if (memcpy_s(fileHandle.m_block.m_buffer, remainSize, (buffer + fileHandle.m_block.m_offset),
            remainSize) != 0) {
                ERRLOG("memcpy_s failed for file %s  blocknum %llu ", fileName.c_str(), fullBlockNum);
                delete[] buffer;
                return ;
        }
        m_outputFileHandleList.push_back(fileHandle);
    }
    delete[] buffer;
}

void SqliteTask::Exec()
{
    HCPTSP::getInstance().reset(m_backupParams.commonParams.reqID);
    switch (m_event) {
        case SqliteEvent::SQL_TASK_PTR_CREATE_INDEX: {
            DoCreateSqliteIndexBlobList();
            break;
        }

        default:
            break;
    }
}

string SqliteTask::GetUniqueIdStr()
{
    return to_string(m_idGenerator->GenerateId());
}

void SqliteTask::DoCreateSqliteIndexBlobList()
{
    m_result = SUCCESS;
    uint16_t len = m_blobFileDetailsList.size();
    for (uint16_t i = 0; i < len; i++) {
        DoCreateSqliteIndex(m_blobFileDetailsList[i], i);
    }
}
