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
#include "FileMap.h"

using namespace std;

FileMap::FileMap(const FileMapParams &params)
{
    m_fileMapName = params.fileMapName;
    m_maxEntryPerFile = params.maxEntriesPerFile;

    m_header.title = params.title;
    m_header.version = NAS_FILEMAP_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
}

FileMap::FileMap(const string &fileMapName)
{
    m_fileMapName = fileMapName;
}

FileMap::~FileMap()
{
    if (m_writeFd.is_open())
        Close(NAS_FILEMAP_OPEN_MODE_WRITE);

    if (m_readFd.is_open())
        Close(NAS_FILEMAP_OPEN_MODE_READ);
}

NAS_FILEMAP_RETCODE FileMap::OpenRead()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (m_readFd.is_open()) {
        return NAS_FILEMAP_RET_SUCCESS;
    }
    ret = FileOpen<std::ifstream>(m_readFd, m_fileMapName, ios::in);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_FILEMAP_RET_FAILED;
    }
    READ_FROM_AGGR_FILE(m_readBuffer, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, MAP_MOD) << "Read from stream is failed for " << m_fileMapName << HCPENDLOG;
        return NAS_FILEMAP_RET_FAILED;
    }
    ReadHeader();
    if (ValidateHeader() != NAS_FILEMAP_RET_SUCCESS) {
        HCP_Log(ERR, MAP_MOD) << "Header verification failed for " << m_fileMapName << HCPENDLOG;
        m_readFd.close();
        return NAS_FILEMAP_RET_FAILED;
    }

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::OpenWrite()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (m_writeFd.is_open()) {
        return NAS_FILEMAP_RET_SUCCESS;
    }
    ret = FileOpen<std::ofstream>(m_writeFd, m_fileMapName, ios::out | ios::app);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_FILEMAP_RET_FAILED;
    }
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::OpenUpdate()
{
    NAS_CTRL_FILE_RETCODE ret;

    if (!m_readFd.is_open()) {
        ret = FileOpen<std::ifstream>(m_readFd, m_fileMapName, ios::in);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_FILEMAP_RET_FAILED;
        }
    }
    READ_FROM_AGGR_FILE(m_readBuffer, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, MAP_MOD) << "Read from stream failed for " << m_fileMapName << HCPENDLOG;
        return NAS_FILEMAP_RET_FAILED;
    }
    ReadHeader();
    if (ValidateHeader() != NAS_FILEMAP_RET_SUCCESS) {
        HCP_Log(ERR, MAP_MOD) << "Header verification failed for " << m_fileMapName << HCPENDLOG;
        m_readFd.close();
        return NAS_FILEMAP_RET_FAILED;
    }
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Open(NAS_FILEMAP_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_FILEMAP_OPEN_MODE_READ) {
        return OpenRead();
    }

    if (mode == NAS_FILEMAP_OPEN_MODE_WRITE) {
        return OpenWrite();
    }

    if (mode == NAS_FILEMAP_OPEN_MODE_UPDATE) {
        return OpenUpdate();
    }

    HCP_Log(ERR, MAP_MOD) << "Invalid open mode for " << m_fileMapName << HCPENDLOG;
    return NAS_FILEMAP_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE FileMap::FileOpen(FileStream &strmFd, string fileName, ios::openmode fileMode)
{
    strmFd.open(fileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(GetParentDirOfFile(fileName)) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Open file failed " << fileName
                << " Parent dir not reachable" << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(fileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MAP_MOD) << "Open file failed " << fileName << " ERR "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Close(NAS_FILEMAP_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_CTRL_FILE_RETCODE ret;

    if (mode == NAS_FILEMAP_OPEN_MODE_WRITE) {
        if (m_writeFd.is_open()) {
            if (WriteHeader() != NAS_FILEMAP_RET_SUCCESS) {
                return NAS_FILEMAP_RET_FAILED;
            }
            WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_FILEMAP_RET_FAILED;
            }
            m_writeFd.close();
        }
        return NAS_FILEMAP_RET_SUCCESS;
    }

    if (mode == NAS_FILEMAP_OPEN_MODE_READ) {
        if (m_readFd.is_open()) {
            m_readFd.close();
        }
        return NAS_FILEMAP_RET_SUCCESS;
    }

    if (mode == NAS_FILEMAP_OPEN_MODE_UPDATE) {
        if (m_readFd.is_open()) {
            m_readFd.close();
        }
        if (!m_writeFd.is_open()) {
            ret = FileOpen<std::ofstream>(m_writeFd, m_fileMapName, ios::out | ios::app);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_FILEMAP_RET_FAILED;
            }
        }
        if (m_writeFd.is_open()) {
            if (WriteHeader() != NAS_FILEMAP_RET_SUCCESS) {
                return NAS_FILEMAP_RET_FAILED;
            }
            WRITE_TO_AGGR_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_FILEMAP_RET_FAILED;
            }
            m_writeFd.close();
        }
        return NAS_FILEMAP_RET_SUCCESS;
    }

    HCP_Log(ERR, MAP_MOD) << "Invalid close mode for :" << m_fileMapName << HCPENDLOG;
    return NAS_FILEMAP_RET_FAILED;
}

NAS_FILEMAP_RETCODE FileMap::GetHeader(FileMapHeader &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_FILEMAP_RET_FAILED;
    }
    header = m_header;
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::ReadEntry(FileMapEntry &fileMapEntry)
{
    string line;
    vector<string> lineContents;
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MAP_MOD) << "Read failed as file map is not open " << m_fileMapName << HCPENDLOG;
        return NAS_FILEMAP_RET_FAILED;
    }

    do {
        getline(m_readBuffer, line);
        if (line.empty()) {
            return NAS_FILEMAP_RET_READ_EOF;
        }
        boost::algorithm::split(lineContents, line, boost::is_any_of(","), boost::token_compress_on);
        if (lineContents.size() == NAS_FILE_MAP_ENTRY_SIZE) {
            break;
        }
        line.clear();
        lineContents.clear();
    } while (true);

    HCP_Log(DEBUG, MAP_MOD) << line << HCPENDLOG;

    TranslateToFileMapEntry(lineContents, fileMapEntry);

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::WriteEntry(FileMapEntry &fileMapEntry)
{
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MAP_MOD) << "Write failed as file map is not open "
            << m_fileMapName <<  HCPENDLOG;
        return NAS_FILEMAP_RET_FAILED;
    }

    m_fileMap[fileMapEntry.absPath].insert(fileMapEntry.mapFileName);

    m_header.stats.noOfFileEntries++;
    m_entries++;

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::UpdateEntry(FileMapEntry &fileMapEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    fileMapEntry;
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> headerLineSplit {};

    while (headerLine < FILEMAP_RESERVED_1) {
        string mapFileHeaderLine {};
        headerLineSplit.clear();
        if (!getline(m_readBuffer, mapFileHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
        if (mapFileHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
        boost::algorithm::split(headerLineSplit, mapFileHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (headerLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_fileMapName << " line: " << mapFileHeaderLine;
            return NAS_FILEMAP_RET_FAILED;
        }
        if (FillHeader(headerLine, headerLineSplit, mapFileHeaderLine) != NAS_FILEMAP_RET_SUCCESS) {
            return NAS_FILEMAP_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine); /* To skip the blank line after header */
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::FillHeader(uint32_t &headerLine, vector<string> &headerLineSplit,
    string &mapFileHeaderLine)
{
    if (headerLine == FILEMAP_TITLE) {
        m_header.title = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_HEADER_VERSION) {
        m_header.version = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_TIMESTAMP) {
        if (headerLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_fileMapName
                << " line: " << mapFileHeaderLine;
            return NAS_FILEMAP_RET_FAILED;
        }
        m_header.timestamp = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE] + ":" +
            headerLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + headerLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == FILEMAP_TASKID) {
        m_header.taskId = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_NASSERVER) {
        m_header.nasServer = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_NASSHARE) {
        m_header.nasSharePath = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_PROTOCOL) {
        m_header.proto = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_PROTOCOL_VERSION) {
        m_header.protoVersion = headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE];
    } else if (headerLine == FILEMAP_DIR_COUNT) {
        m_header.stats.noOfDirEntries = (uint64_t)atoll((headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE]).c_str());
    } else if (headerLine == FILEMAP_FILE_COUNT) {
        m_header.stats.noOfFileEntries = (uint64_t)atoll((headerLineSplit[FILEMAP_HEADER_ENTRY_OFFSET_VALUE]).c_str());
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << mapFileHeaderLine << " fileName: " << m_fileMapName << HCPENDLOG;
    }
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer;
    NAS_CTRL_FILE_RETCODE ret;
    while (headerLine < FILEMAP_RESERVED_1) {
        string mapFileHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << mapFileHeaderLine;
    }
    headerBuffer << "\n";
    WRITE_TO_AGGR_FILE(headerBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, MAP_MOD) << "Write Header for map file failed " << m_fileMapName << HCPENDLOG;
        return NAS_FILEMAP_RET_FAILED;
    }
    return NAS_FILEMAP_RET_SUCCESS;
}

string FileMap::GetFileHeaderLine(uint32_t headerLine)
{
    string mapFileHeaderLine;
    switch (headerLine) {
        case FILEMAP_TITLE:
            mapFileHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case FILEMAP_HEADER_VERSION:
            mapFileHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case FILEMAP_TIMESTAMP:
            mapFileHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case FILEMAP_TASKID:
            mapFileHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case FILEMAP_NASSERVER:
            mapFileHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case FILEMAP_NASSHARE:
            mapFileHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case FILEMAP_PROTOCOL:
            mapFileHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case FILEMAP_PROTOCOL_VERSION:
            mapFileHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case FILEMAP_DIR_COUNT:
            mapFileHeaderLine = "DirectoryCount:" + to_string(m_header.stats.noOfDirEntries) + "\n";
            break;
        case FILEMAP_FILE_COUNT:
            mapFileHeaderLine = "FileCount:" + to_string(m_header.stats.noOfFileEntries) + "\n";
            break;
        default:
            HCP_Log(ERR, MAP_MOD) << "No of values for header exceeded. Line "
                << headerLine << HCPENDLOG;
            break;
    }
    return mapFileHeaderLine;
}

NAS_FILEMAP_RETCODE FileMap::ValidateHeader()
{
    if ((strcmp(m_header.version.c_str(), NAS_FILEMAP_HEADER_VERSION.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.nasServer.empty() ||
        m_header.nasSharePath.empty() || m_header.proto.empty() || m_header.protoVersion.empty()) {
            return NAS_FILEMAP_RET_FAILED;
    }
    return NAS_FILEMAP_RET_SUCCESS;
}

string FileMap::TranslateFromFileMapEntry(FileMapEntry &fileMapEntry)
{
    return fileMapEntry.absPath + NAS_FILEMAP_ENTRY_DELIMITER +
           fileMapEntry.absPath + "\n";
}

void FileMap::TranslateToFileMapEntry(vector<string> &fileContents, FileMapEntry &fileMapEntry)
{
    fileMapEntry.absPath = fileContents[FILEMAP_ENTRY_DIRNAME];
    fileMapEntry.mapFileName = fileContents[FILEMAP_ENTRY_FILENAME];
}

uint32_t FileMap::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

NAS_FILEMAP_RETCODE FileMap::Merge(unordered_map<string, unordered_set<string>> &fileMap)
{
    for (auto itm = fileMap.begin(); itm != fileMap.end(); itm++) {
        m_header.stats.noOfFileEntries += itm->second.size();
        if (m_fileMap.count(itm->first) == 0) {
            m_fileMap[itm->first] = itm->second;
            m_header.stats.noOfDirEntries++;
        } else {
            for (auto itf = itm->second.begin(); itf != itm->second.end(); itf++) {
                m_fileMap[itm->first].insert(*itf);
            }
        }
    }

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Merge(vector<string> &fileMapList)
{
    NAS_FILEMAP_RETCODE ret;

    for (auto tmpFileMapName : fileMapList) {
        unique_ptr<FileMap> tmpFileMap = make_unique<FileMap>(tmpFileMapName);
        unordered_map<string, unordered_set<string>> tmpFileInmemoryMap;

        ret = tmpFileMap->Open(NAS_FILEMAP_OPEN_MODE_READ);
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Read map file failed " << tmpFileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }

        ret = tmpFileMap->Load();
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << tmpFileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }

        ret = tmpFileMap->GetFileMap(tmpFileInmemoryMap);
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Get map file failed " << tmpFileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }

        ret = Merge(tmpFileInmemoryMap);
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Get map file failed " << tmpFileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }

        tmpFileMap->Close(NAS_FILEMAP_OPEN_MODE_READ);
    }

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Merge(vector<string> &fileMapList, FILEMAP_MERGE_TYPE mergeType)
{
    lock_guard<std::mutex> lk(m_lock);
    NAS_FILEMAP_RETCODE ret;

    if (mergeType == FILEMAP_MERGE_NEW) {
        ret = Merge(fileMapList);
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
        ret = Unload();
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
    }

    if (mergeType == FILEMAP_MERGE_UPDATE) {
        ret = Load();
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
        ret = Merge(fileMapList);
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
        ret = Unload();
        if (ret != NAS_FILEMAP_RET_SUCCESS) {
            HCP_Log(ERR, MAP_MOD) << "Load map file failed " << m_fileMapName << HCPENDLOG;
            return NAS_FILEMAP_RET_FAILED;
        }
    }

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Load()
{
    lock_guard<std::mutex> lk(m_lock);
    FileMapEntry fileMapEntry;

    if (!m_readFd.is_open()) {
        return NAS_FILEMAP_RET_FAILED;
    }

    while (ReadEntry(fileMapEntry) != NAS_FILEMAP_RET_READ_EOF) {
        if (m_fileMap.count(fileMapEntry.absPath) == 0) {
            m_header.stats.noOfDirEntries++;
        }
        m_header.stats.noOfFileEntries++;
        m_fileMap[fileMapEntry.absPath].insert(fileMapEntry.mapFileName);
    }

    m_readBuffer.str("");
    m_readBuffer.clear();

    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::Unload()
{
    lock_guard<std::mutex> lk(m_lock);
    FileMapEntry fileMapEntry;

    if (!m_writeFd.is_open()) {
        return NAS_FILEMAP_RET_FAILED;
    }

    for (auto itm = m_fileMap.begin(); itm != m_fileMap.end(); itm++) {
        fileMapEntry.absPath = itm->first;
        for (auto itf = itm->second.begin(); itf != itm->second.end(); itf++) {
            fileMapEntry.mapFileName = *itf;
            m_writeBuffer << TranslateFromFileMapEntry(fileMapEntry);
        }
    }

    m_fileMap.clear();
    return NAS_FILEMAP_RET_SUCCESS;
}

NAS_FILEMAP_RETCODE FileMap::GetFileMap(unordered_map<string, unordered_set<string>> &fileMap)
{
    lock_guard<std::mutex> lk(m_lock);

    fileMap = m_fileMap;
    return NAS_FILEMAP_RET_SUCCESS;
}