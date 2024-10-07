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
#include "BackupIndex.h"
using namespace std;
using namespace Module;
using namespace NasScanner;

BackupIndex::BackupIndex(const BackupIndexParams &params)
{
    m_indexFileName        = params.indexFileName;
    m_indexFileTimeElapsed = params.indexFileTimeElapsed;

    m_header.title         = NAS_BACKUPINDEX_HEADER_TITLE;
    m_header.version       = NAS_BACKUPINDEX_HEADER_VERSION;
    m_header.timestamp     = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId        = params.taskId;
    m_header.nasServer     = params.nasServer;
    m_header.nasSharePath  = params.nasSharePath;
    m_header.proto         = params.proto;
    m_header.protoVersion  = params.proto_version;
    m_header.backupType    = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

BackupIndex::BackupIndex(string &indexFileName)
{
    m_indexFileName      = indexFileName;
}

BackupIndex::~BackupIndex()
{
    HCP_Log(DEBUG, INDEX_MOD) << "Destruct file " << m_indexFileName << HCPENDLOG;
    if (m_writeFd.is_open()) {
        Close(NAS_INDEX_FILE_OPEN_MODE_WRITE);
    }
    if (m_readFd.is_open()) {
        Close(NAS_INDEX_FILE_OPEN_MODE_READ);
    }
}

NAS_INDEX_FILE_RETCODE BackupIndex::OpenRead()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (m_readFd.is_open()) {
        return NAS_INDEX_FILE_RET_SUCCESS;
    }
    ret = FileOpen<std::ifstream>(m_readFd, m_indexFileName, std::ios::in);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }
    READ_FROM_AGGR_FILE(m_readBuffer, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Read from stream failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    ReadHeader();

    if (ValidateHeader() != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Header verification failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::OpenWrite()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (m_writeFd.is_open()) {
        return NAS_INDEX_FILE_RET_SUCCESS;
    }
    ret = FileOpen<std::ofstream>(m_writeFd, m_indexFileName, std::ios::out | std::ios::app);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }
    m_indexFileCreationTime = GetCurrentTimeInSeconds();

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::OpenReadWrite()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (!m_readFd.is_open()) {
        ret = FileOpen<std::ifstream>(m_readFd, m_indexFileName, std::ios::in);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_INDEX_FILE_RET_FAILED;
        }
    }
    READ_FROM_AGGR_FILE(m_readBuffer, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Read from stream failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    ReadHeader();

    if (ValidateHeader() != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Header verification failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Open(NAS_INDEX_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_INDEX_FILE_OPEN_MODE_READ) {
        return OpenRead();
    }

    if (mode == NAS_INDEX_FILE_OPEN_MODE_WRITE) {
        return OpenWrite();
    }

    if (mode == NAS_INDEX_FILE_OPEN_MODE_UPDATE) {
        return OpenReadWrite();
    }

    return NAS_INDEX_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE BackupIndex::FileOpen(FileStream &strmFd, string fileName, std::ios::openmode fileMode)
{
    strmFd.open(fileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(GetParentDirOfFile(fileName)) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "Open file failed for " << fileName
                << " Parent dir not reachable" << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(fileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, INDEX_MOD) << "Open file failed for " << fileName
                << " ERR " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Close(NAS_INDEX_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_CTRL_FILE_RETCODE ret;

    if (mode == NAS_INDEX_FILE_OPEN_MODE_WRITE) {
        if (m_writeFd.is_open()) {
            if (WriteHeader() != NAS_INDEX_FILE_RET_SUCCESS) {
                return NAS_INDEX_FILE_RET_FAILED;
            }
            WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_INDEX_FILE_RET_FAILED;
            }
            m_writeFd.close();
        }
        return NAS_INDEX_FILE_RET_SUCCESS;
    }

    if (mode == NAS_INDEX_FILE_OPEN_MODE_READ) {
        if (!m_readFd.is_open()) {
            return NAS_INDEX_FILE_RET_SUCCESS;
        }
        m_readFd.close();
        return NAS_INDEX_FILE_RET_SUCCESS;
    }

    if (mode == NAS_INDEX_FILE_OPEN_MODE_UPDATE) {
        if (m_readFd.is_open()) {
            m_readFd.close();
        }
        if (!m_writeFd.is_open()) {
            ret = FileOpen<std::ofstream>(m_writeFd, m_indexFileName, std::ios::out | std::ios::app);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_INDEX_FILE_RET_FAILED;
            }
        }
        if (!m_writeFd.is_open()) {
            if (WriteHeader() != NAS_INDEX_FILE_RET_SUCCESS) {
                return NAS_INDEX_FILE_RET_FAILED;
            }
            WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_INDEX_FILE_RET_FAILED;
            }
            m_writeFd.close();
        }
        return NAS_INDEX_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, INDEX_MOD) << "Invalid close mode for " << m_indexFileName << HCPENDLOG;
    return NAS_INDEX_FILE_RET_FAILED;
}

NAS_INDEX_FILE_RETCODE BackupIndex::ReadEntries(INDEX_ENTRY_OP_TYPE opType)
{
    INDEX_ENTRY_TYPE entryType;
    BackupIndexFileMetaInfo fileMetaInfo;
    BackupIndexDirEntry dirEntry;
    BackupIndexArchiveEntry archiveEntry;
    BackupIndexFileEntry fileEntry;

    while (ReadEntry(dirEntry, archiveEntry, fileEntry, entryType) == NAS_INDEX_FILE_RET_SUCCESS) {
        if (entryType == INDEX_ENTRY_TYPE_ARCHIVE) {
            continue;
        }
        fileMetaInfo.metaDataLen = fileEntry.metaDataLen;
        if (opType == INDEX_ENTRY_OP_FULL) {
            m_archiveMap[archiveEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
        } else {
            m_archiveMapInc[archiveEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
            m_incFileSet.insert(fileEntry.fileName);
        }
        m_fileEntryCount++;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::ReadEntry(
    BackupIndexDirEntry &dirEntry, BackupIndexArchiveEntry &archiveEntry, BackupIndexFileEntry &fileEntry,
    INDEX_ENTRY_TYPE &entryType)
{
    string indexFileLine;
    vector<string> indexFileLineSplit;

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Read failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    getline(m_readBuffer, indexFileLine);

    if (indexFileLine == NAS_BACKUPINDEX_SECTION_DELIMITER) {
        return NAS_INDEX_FILE_RET_READ_NEXT_SECTION;
    }

    if (indexFileLine.empty()) {
        return NAS_INDEX_FILE_RET_READ_EOF;
    }

    boost::algorithm::split(indexFileLineSplit, indexFileLine, boost::is_any_of(","), boost::token_compress_on);
    indexFileLine.clear();
    indexFileLineSplit.clear();

    if (indexFileLineSplit.size() == NAS_INDEX_DIR_ENTRY_SIZE) {
        TranslateToDirEntry(indexFileLineSplit, dirEntry);
        entryType = INDEX_ENTRY_TYPE_DIR;
    } else if (indexFileLineSplit.size() == NAS_INDEX_ARCHIVE_ENTRY_SIZE) {
        TranslateToArchiveEntry(indexFileLineSplit, archiveEntry);
        entryType = INDEX_ENTRY_TYPE_ARCHIVE;
    } else if (indexFileLineSplit.size() == NAS_INDEX_FILE_ENTRY_SIZE) {
        TranslateToFileEntry(indexFileLineSplit, fileEntry);
        entryType = INDEX_ENTRY_TYPE_FILE;
    } else {
        return NAS_INDEX_FILE_RET_FAILED;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Load()
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_INDEX_FILE_RETCODE ret;
    INDEX_ENTRY_TYPE entryType;
    BackupIndexDirEntry dirEntry;
    BackupIndexArchiveEntry archiveEntry;
    BackupIndexFileEntry fileEntry;

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Read failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    /* first entry is dir */
    ret = ReadEntry(dirEntry, archiveEntry, fileEntry, entryType);
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Read dir entry failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    if (entryType != INDEX_ENTRY_TYPE_DIR) {
        HCP_Log(ERR, INDEX_MOD) << "Dir entry verification failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }
    m_dirEntry = dirEntry;

    /* rest entries are old files */
    ret = ReadEntries(INDEX_ENTRY_OP_FULL);
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "File entries(full) read failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    if (ReadEntry(dirEntry, archiveEntry, fileEntry, entryType) == NAS_INDEX_FILE_RET_READ_NEXT_SECTION) {
        /* if got another section, continue to read rest entries which wrote during inc backup */
        ret = ReadEntries(INDEX_ENTRY_OP_INC);
        if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "File entries(inc) read failed for " << m_indexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
    }

    m_readBuffer.str("");
    m_readBuffer.clear();

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Release()
{
    m_archiveMap.clear();
    m_archiveMapInc.clear();
    m_archiveDeleteList.clear();
    m_archivePartialDeleteList.clear();
    m_incFileSet.clear();

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::UnloadArchiveMap()
{
    NAS_CTRL_FILE_RETCODE ret;
    BackupIndexFileEntry fileEntry;
    BackupIndexArchiveEntry archiveEntry;

    for (auto ita = m_archiveMap.begin(); ita != m_archiveMap.end(); ita++) {
        archiveEntry.archiveName = ita->first;
        m_writeBuffer << TranslateFromArchiveEntry(archiveEntry);
        for (auto itf = ita->second.begin(); itf != ita->second.end(); itf++) {
            fileEntry.fileName        = itf->first;
            fileEntry.metaDataLen = itf->second.metaDataLen;
            m_writeBuffer << TranslateFromFileEntry(fileEntry);
        }
    }

    WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_archiveMap.clear();

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::UnloadArchiveMapInc()
{
    NAS_CTRL_FILE_RETCODE ret;
    BackupIndexFileEntry fileEntry;
    BackupIndexArchiveEntry archiveEntry;

    m_writeBuffer << NAS_BACKUPINDEX_SECTION_DELIMITER << "\n";
    for (auto ita = m_archiveMapInc.begin(); ita != m_archiveMapInc.end(); ita++) {
        archiveEntry.archiveName = ita->first;
        m_writeBuffer << TranslateFromArchiveEntry(archiveEntry);
        for (auto itf = ita->second.begin(); itf != ita->second.end(); itf++) {
            fileEntry.fileName        = itf->first;
            fileEntry.metaDataLen = itf->second.metaDataLen;
            m_writeBuffer << TranslateFromFileEntry(fileEntry);
        }
    }

    WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_archiveMapInc.clear();

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Unload()
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_INDEX_FILE_RETCODE ret;

    ret = WriteDir();
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }

    ret = UnloadArchiveMap();
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }

    if (m_archiveMapInc.empty()) {
        return NAS_INDEX_FILE_RET_SUCCESS;
    }

    ret = UnloadArchiveMapInc();
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteDirEntry(BackupIndexDirEntry &dirEntry)
{
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Write failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    m_dirEntry = dirEntry;
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteDirectoryMeta(DirectoryMeta &dirMeta)
{
    dirMeta;
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteFileEntry(
    BackupIndexFileEntry &fileEntry, INDEX_ENTRY_OP_TYPE opType)
{
    lock_guard<std::mutex> lk(m_lock);
    BackupIndexFileMetaInfo fileMetaInfo;

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Write failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    fileMetaInfo.metaDataLen = fileEntry.metaDataLen;
    if (opType == INDEX_ENTRY_OP_FULL) {
        m_archiveMap[fileEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
    } else {
        m_archiveMapInc[fileEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteFileEntries(
    vector<BackupIndexFileEntry> &fileEntries, INDEX_ENTRY_OP_TYPE opType)
{
    lock_guard<std::mutex> lk(m_lock);
    BackupIndexFileMetaInfo fileMetaInfo;

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Write failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    for (auto &fileEntry : fileEntries) {
        fileMetaInfo.metaDataLen = fileEntry.metaDataLen;
        if (opType == INDEX_ENTRY_OP_FULL) {
            m_archiveMap[fileEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
        } else {
            m_archiveMapInc[fileEntry.archiveName][fileEntry.fileName] = fileMetaInfo;
        }
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

bool BackupIndex::Exist(const string &fileName)
{
    lock_guard<std::mutex> lk(m_lock);

    for (auto ita = m_archiveMap.begin(); ita != m_archiveMap.end(); ita++) {
        for (auto itf = ita->second.begin(); itf != ita->second.end(); itf++) {
            if (itf->first == fileName) {
                return true;
            }
        }
    }
    return false;
}

NAS_INDEX_FILE_RETCODE BackupIndex::GetDeleteList(const unordered_set<string> &deleteFiles,
    unordered_set<string> &archiveDeleteList, unordered_map<string, unordered_set<string>> &archivePartialDeleteList)
{
    lock_guard<std::mutex> lk(m_lock);

    for (auto ita = m_archiveMap.begin(); ita != m_archiveMap.end(); ita++) {
        unordered_set<string> deleteFileList;
        for (auto itf = ita->second.begin(); itf != ita->second.end(); itf++) {
            if (deleteFiles.count(itf->first) > 0) {
                deleteFileList.insert(itf->first);
                itf->second.toBeDelete = true;
            }
        }
        if (ita->second.size() == deleteFileList.size()) {
            m_archiveDeleteList.insert(ita->first);
        } else {
            m_archivePartialDeleteList[ita->first] = deleteFileList;
        }
    }

    archiveDeleteList = m_archiveDeleteList;
    archivePartialDeleteList = m_archivePartialDeleteList;
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::GetArchiveMap(
    unordered_map<string, unordered_map<string, BackupIndexFileMetaInfo>> &archiveMap)
{
    lock_guard<std::mutex> lk(m_lock);
    archiveMap = m_archiveMap;
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::GetDirEntry(BackupIndexDirEntry &dirEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    dirEntry = m_dirEntry;
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Merge(
    unordered_map<string, unordered_map<string, BackupIndexFileMetaInfo>> &archiveMap)
{
    for (auto ita = archiveMap.begin(); ita != archiveMap.end(); ita++) {
        if (m_archiveMap.count(ita->first) > 0) {
            return NAS_INDEX_FILE_RET_FAILED;
        }
        m_archiveMap[ita->first] = ita->second;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Merge(vector<string> &tmpIndexFileList)
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_INDEX_FILE_RETCODE ret;

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, INDEX_MOD) << "Write failed as file is not open " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }

    for (auto &tmpIndexFileName : tmpIndexFileList) {
        unordered_map<string, unordered_map<string, BackupIndexFileMetaInfo>> archiveMap;
        unique_ptr<BackupIndex> tmpIndexFile = std::make_unique<BackupIndex>(tmpIndexFileName);
        ret = tmpIndexFile->Open(NAS_INDEX_FILE_OPEN_MODE_READ);
        if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "Open failed for " << tmpIndexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        ret = tmpIndexFile->Load();
        if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "Load failed for " << tmpIndexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        if (IsFileBufferEmpty()) {
            HCP_Log(DEBUG, INDEX_MOD) << "Empty file entries for " << tmpIndexFileName << HCPENDLOG;
            continue;
        }
        ret = tmpIndexFile->GetArchiveMap(archiveMap);
        if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "Get archive map failed for " << tmpIndexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        ret = Merge(archiveMap);
        if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
            HCP_Log(ERR, INDEX_MOD) << "Merge archive map failed for " << tmpIndexFileName <<  HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        tmpIndexFile->Close(NAS_INDEX_FILE_OPEN_MODE_READ);
    }

    ret = Unload();
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Dump failed for " << m_indexFileName << HCPENDLOG;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::Synthetic()
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_INDEX_FILE_RETCODE ret;

    for (auto ita = m_archiveMap.begin(); ita != m_archiveMap.end();) {
        if (m_archiveDeleteList.count(ita->first) > 0) {
            ita = m_archiveMap.erase(ita);
        } else if (m_archivePartialDeleteList.count(ita->first) > 0) {
            for (auto itf = ita->second.begin(); itf != ita->second.end();) {
                if (m_archivePartialDeleteList[ita->first].count(itf->first)) {
                    itf = ita->second.erase(itf);
                } else {
                    itf++;
                }
            }
        } else {
            for (auto itf = ita->second.begin(); itf != ita->second.end();) {
                if (m_incFileSet.count(itf->first) > 0) {
                    itf = ita->second.erase(itf);
                } else {
                    itf++;
                }
            }
        }
    }

    ret = Unload();
    if (ret != NAS_INDEX_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Dump failed for " << m_indexFileName << HCPENDLOG;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

string BackupIndex::GetFileHeaderLine(INDEX_HEADER_INFO headerLine)
{
    string indexHeaderLine;
    switch (headerLine) {
        case INDEX_TITLE:
            indexHeaderLine = "Title:"              + m_header.title + "\n";
            break;
        case INDEX_HEADER_VERSION:
            indexHeaderLine = "Version:"            + m_header.version + "\n";
            break;
        case INDEX_TIMESTAMP:
            indexHeaderLine = "Timestamp:"          + m_header.timestamp + "\n";
            break;
        case INDEX_TASKID:
            indexHeaderLine = "TaskId:"             + m_header.taskId + "\n";
            break;
        case INDEX_TASKTYPE:
            indexHeaderLine = "BackupType:"         + m_header.backupType + "\n";
            break;
        case INDEX_NASSERVER:
            indexHeaderLine = "NasServer:"          + m_header.nasServer + "\n";
            break;
        case INDEX_NASSHARE:
            indexHeaderLine = "NasShare:"           + m_header.nasSharePath + "\n";
            break;
        case INDEX_PROTOCOL:
            indexHeaderLine = "NasProtocol:"        + m_header.proto + "\n";
            break;
        case INDEX_PROTOCOL_VERSION:
            indexHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case INDEX_METADATA_SCOPE:
            indexHeaderLine = "MetadataScope:"      + m_header.metaDataScope + "\n";
            break;
        case INDEX_ARCHIVE_COUNT:
            indexHeaderLine = "ArchiveCount:"       + to_string(m_header.stats.noOfArchives) + "\n";
            break;
        case INDEX_FILE_COUNT:
            indexHeaderLine = "FileCount:"          + to_string(m_header.stats.noOfFiles) + "\n";
            break;
        default:
            HCP_Log(ERR, INDEX_MOD) << "No of values for header exceeded. Line " << headerLine << HCPENDLOG;
            break;
    }
    return indexHeaderLine;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer;
    string indexHeaderLine;
    NAS_CTRL_FILE_RETCODE ret;

    m_header.stats.noOfArchives = m_archiveMap.size() + m_archiveMapInc.size();
    for (auto ita = m_archiveMap.begin(); ita != m_archiveMap.end(); ita++) {
        m_header.stats.noOfFiles += ita->second.size();
    }
    for (auto ita = m_archiveMapInc.begin(); ita != m_archiveMapInc.end(); ita++) {
        m_header.stats.noOfFiles += ita->second.size();
    }

    while (headerLine < INDEX_RESERVED_1) {
        indexHeaderLine = GetFileHeaderLine(static_cast<INDEX_HEADER_INFO>(headerLine));
        headerLine++;
        headerBuffer << indexHeaderLine;
    }

    headerBuffer << "\n";
    WRITE_TO_AGGR_FILE(headerBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, INDEX_MOD) << "Write Header failed for " << m_indexFileName << HCPENDLOG;
        return NAS_INDEX_FILE_RET_FAILED;
    }
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::WriteDir()
{
    NAS_CTRL_FILE_RETCODE ret;

    m_writeBuffer << TranslateFromDirEntry(m_dirEntry);
    WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_INDEX_FILE_RET_FAILED;
    }

    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> indexHeaderLineSplit {};

    while (headerLine < INDEX_RESERVED_1) {
        string indexHeaderLine {};
        indexHeaderLineSplit.clear();
        if (!getline(m_readBuffer, indexHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_indexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        if (indexHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_indexFileName << HCPENDLOG;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        boost::algorithm::split(indexHeaderLineSplit, indexHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (indexHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_indexFileName << " line: " << indexHeaderLine;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        if (FillHeader(headerLine, indexHeaderLineSplit, indexHeaderLine) != NAS_INDEX_FILE_RET_SUCCESS) {
            return NAS_INDEX_FILE_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine); /* To skip the blank line after header */
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::FillHeader(uint32_t &headerLine, vector<string> &indexHeaderLineSplit,
    string &indexHeaderLine)
{
    if (headerLine == INDEX_TITLE) {
        m_header.title = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_HEADER_VERSION) {
        m_header.version = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_TIMESTAMP) {
        if (indexHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_indexFileName
                << " line: " << indexHeaderLine;
            return NAS_INDEX_FILE_RET_FAILED;
        }
        m_header.timestamp = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE] + ":" +
            indexHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + indexHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == INDEX_TASKID) {
        m_header.taskId = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_TASKTYPE) {
        m_header.backupType = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_NASSERVER) {
        m_header.nasServer = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_NASSHARE) {
        m_header.nasSharePath = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_PROTOCOL) {
        m_header.proto = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_PROTOCOL_VERSION) {
        m_header.protoVersion = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_METADATA_SCOPE) {
        m_header.metaDataScope = indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == INDEX_ARCHIVE_COUNT) {
        m_header.stats.noOfArchives = (uint64_t)atoll((indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE]).c_str());
    } else if (headerLine == INDEX_FILE_COUNT) {
        m_header.stats.noOfFiles = (uint64_t)atoll((indexHeaderLineSplit[INDEX_HEADER_ENTRY_OFFSET_VALUE]).c_str());
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << indexHeaderLine << " fileName: " << m_indexFileName << HCPENDLOG;
    }
    return NAS_INDEX_FILE_RET_SUCCESS;
}

NAS_INDEX_FILE_RETCODE BackupIndex::ValidateHeader()
{
    return NAS_INDEX_FILE_RET_SUCCESS;
}

string BackupIndex::TranslateFromDirEntry(BackupIndexDirEntry &dirEntry)
{
    return dirEntry.dirName + NAS_BACKUPINDEX_ENTRY_DELIMITER +                     /* DIR_OFFSET_DIRNAME */
           dirEntry.metaFileName + NAS_BACKUPINDEX_ENTRY_DELIMITER +                /* DIR_OFFSET_METAFILE_NAME */
           to_string(dirEntry.metaFileOffset) + NAS_BACKUPINDEX_ENTRY_DELIMITER +   /* DIR_OFFSET_METAFILE_OFFSET */
           to_string(dirEntry.metaFileReadLen) + NAS_BACKUPINDEX_ENTRY_DELIMITER +  /* DIR_OFFSET_METAFILE_LEN */
           to_string(dirEntry.aclFlag) + "\n";                                      /* DIR_OFFSET_ACL_FLAG */
}

void BackupIndex::TranslateToDirEntry(const vector<string> &fileContents, BackupIndexDirEntry &dirEntry)
{
    dirEntry.dirName =              fileContents[INDEX_ENTRY_DIR_OFFSET_DIRNAME];
    dirEntry.metaFileName =         fileContents[INDEX_ENTRY_DIR_OFFSET_METAFILE_NAME];
    dirEntry.metaFileOffset = (uint64_t)atoll(fileContents[INDEX_ENTRY_DIR_OFFSET_METAFILE_OFFSET].c_str());
    dirEntry.metaFileReadLen = (uint16_t)atoi(fileContents[INDEX_ENTRY_DIR_OFFSET_METAFILE_LEN].c_str());
    dirEntry.aclFlag =         (uint32_t)atoi(fileContents[INDEX_ENTRY_DIR_OFFSET_ACL_FLAG].c_str());
}

string BackupIndex::TranslateFromArchiveEntry(BackupIndexArchiveEntry &archiveEntry)
{
    return archiveEntry.archiveName + "\n"; /* ARCHIVE_OFFSET_ARCHIVENAME */
}

void BackupIndex::TranslateToArchiveEntry(const vector<string> &archiveContents, BackupIndexArchiveEntry &archiveEntry)
{
    archiveEntry.archiveName = archiveContents[INDEX_ENTRY_ARCHIVE_OFFSET_ARCHIVENAME];
}

string BackupIndex::TranslateFromFileEntry(BackupIndexFileEntry &fileEntry)
{
    return fileEntry.fileName + NAS_BACKUPINDEX_ENTRY_DELIMITER + /* FILE_OFFSET_DIRNAME */
           to_string(fileEntry.metaDataLen) + "\n";           /* FILE_OFFSET_METAFILE_LEN */
}

void BackupIndex::TranslateToFileEntry(const vector<string> &fileContents, BackupIndexFileEntry &fileEntry)
{
    fileEntry.fileName        = fileContents[INDEX_ENTRY_FILE_OFFSET_DIRNAME];
    fileEntry.metaDataLen     = (uint16_t)atoi(fileContents[INDEX_ENTRY_FILE_OFFSET_METAFILE_LEN].c_str());
}

bool BackupIndex::IsFileBufferEmpty()
{
    lock_guard<std::mutex> lk(m_lock);
    return (m_fileEntryCount == 0);
}

uint32_t BackupIndex::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

bool BackupIndex::CheckIndexFileTimeElapse()
{
    lock_guard<std::mutex> lk(m_lock);
    time_t curTime = GetCurrentTimeInSeconds();
    if ((curTime - m_indexFileCreationTime) >= m_indexFileTimeElapsed) {
        HCP_Log(INFO, INDEX_MOD) << "Index file time elapsed for file " << m_indexFileName << HCPENDLOG;
        return true;
    }
    return false;
}
