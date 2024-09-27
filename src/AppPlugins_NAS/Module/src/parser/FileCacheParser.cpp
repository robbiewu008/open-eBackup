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
#include "FileCacheParser.h"
#include "securec.h"
#include "Log.h"
#include "common/Thread.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "FCACHE_PARSER";
}

FileCacheParser::FileCacheParser(FileCacheParser::Params params) : FileParser(true)
{
    m_fileName = params.fileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = FCACHE_HEADER_TITLE;
    m_header.version = params.version;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

FileCacheParser::FileCacheParser(std::string fcacheFileName) : FileParser(true)
{
    m_fileName = fcacheFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_readBufferSize = CTRL_READ_BUFFER_SIZE * sizeof(FileCache);
}

FileCacheParser::~FileCacheParser()
{
    if (m_writeFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }

    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE FileCacheParser::OpenWrite()
{
    m_currWriteOffset = 0;
    if (m_writeFd.tellp() == 0) {
        return WriteHeader();
    } else {
        m_currWriteOffset = (uint64_t) m_writeFd.tellp();
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::GetHeader(FileCacheParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::ReadHeader()
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadFCacheFileHeader();
        if (ret == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        }
        m_readFd.sync();
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed. readFD is not good file: "
                << m_fileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.close();
            ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
        } else {
            HCP_Log(ERR, MODULE) << "ReadHeader failed. readFD is good file: "
                << m_fileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.seekg(0);
        }
        retryCnt++;
        uint32_t sleepDuration = ParserUtils::GetRandomNumber(HDR_RETRY_SLEEP_DUR_MIN_MSEC,
            HDR_RETRY_SLEEP_DUR_MAX_MSEC);
        Module::SleepFor(std::chrono::milliseconds(sleepDuration));
    } while (retryCnt < CTRL_FILE_SERVER_RETRY_CNT);
    return ret;
}

CTRL_FILE_RETCODE FileCacheParser::ReadFCacheFileHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};
    if (!m_readFd) {
        HCP_Log(ERR, MODULE) << "ReadHeader readFd not proper: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readFd, cltHeaderLine)) {
            HCP_Log(ERR, MODULE) << "ReadHeader (getline) failed: " << m_fileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed, incomplete header: " << m_fileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed: " << m_fileName << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (FillHeader(headerLine, cltHeaderLineSplit, cltHeaderLine) != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readFd, blankLine); /* To skip the blank line after header */
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == FCACHE_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == FCACHE_TASKID) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == FCACHE_METADATA_SCOPE) {
        m_header.metaDataScope = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    m_currWriteOffset = (uint64_t) headerBuffer.tellp();
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header For Fcache Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string FileCacheParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case FCACHE_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case FCACHE_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case FCACHE_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case FCACHE_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case FCACHE_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case FCACHE_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case FCACHE_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case FCACHE_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case FCACHE_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case FCACHE_METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE FileCacheParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), FCACHE_HEADER_TITLE.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.metaDataScope.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    if ((strcmp(m_header.version.c_str(), FCACHE_HEADER_VERSION_V20.c_str()) != 0) &&
        (strcmp(m_header.version.c_str(), FCACHE_HEADER_VERSION_V10.c_str()) != 0)) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::WriteFileCache(FileCache &fcache)
{
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (m_header.version == FCACHE_HEADER_VERSION_V30) {
        m_writeBuffer.write((char *)&fcache, sizeof(fcache));
        m_currWriteOffset += sizeof(fcache);
    } else {
        FileCacheV20 fcacheV20 {};
        ConvertFcacheToV20(fcache, fcacheV20);
        m_writeBuffer.write((char *)&fcacheV20, sizeof(fcacheV20));
        m_currWriteOffset += sizeof(fcacheV20);
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

void FileCacheParser::ConvertFcacheToV20(const FileCache &fcache, FileCacheV20 &fcacheV20)
{
    fcacheV20.m_inode          = fcache.m_inode;
    fcacheV20.m_mdataOffset    = fcache.m_mdataOffset;
    fcacheV20.m_hashTag        = fcache.m_filePathHash.crc;
    fcacheV20.m_crc            = fcache.m_fileMetaHash.crc;
    fcacheV20.m_fileId         = fcache.m_fileId;
    fcacheV20.m_metaLength     = fcache.m_metaLength;
    fcacheV20.m_compareFlag    = fcache.m_compareFlag;
}

CTRL_FILE_RETCODE FileCacheParser::WriteFileCacheEntries(std::queue<FileCache> &fileCacheQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    while (!fileCacheQueue.empty()) {
        FileCache fcache = fileCacheQueue.front();
        fileCacheQueue.pop();
        if (m_header.version == FCACHE_HEADER_VERSION_V30) {
            m_writeBuffer.write((char *)&fcache, sizeof(fcache));
            m_currWriteOffset += sizeof(fcache);
        } else {
            FileCacheV20 fcacheV20 {};
            ConvertFcacheToV20(fcache, fcacheV20);
            m_writeBuffer.write((char *)&fcacheV20, sizeof(fcacheV20));
            m_currWriteOffset += sizeof(fcacheV20);
        }
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::WriteFileCacheEntries(
    std::priority_queue<FileCache, std::vector<FileCache>, Comparator> &fileCacheQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    while (!fileCacheQueue.empty()) {
        FileCache fcache = fileCacheQueue.top();
        fileCacheQueue.pop();
        m_writeBuffer.write((char *)&fcache, sizeof(fcache));
        m_currWriteOffset += sizeof(fcache);
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::WriteFileCacheEntries(
    std::priority_queue<FileCache, std::vector<FileCache>, ComparatorV20> &fileCacheQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    while (!fileCacheQueue.empty()) {
        FileCache fcache = fileCacheQueue.top();
        fileCacheQueue.pop();
        FileCacheV20 fcacheV20 {};
        ConvertFcacheToV20(fcache, fcacheV20);
        m_writeBuffer.write((char *)&fcacheV20, sizeof(fcacheV20));
        m_currWriteOffset += sizeof(fcacheV20);
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

void FileCacheParser::PrintFileCache(const FileCache& fileCache)
{
    DBGLOG("Write to fcache - %s", m_fileName.c_str());
    DBGLOG("Write fileCache - inode: %u, mdataoffset: %u, hastag: %u, crc: %u, fileId: %u, metalength: %u, compareFilg: %u",
        fileCache.m_inode, fileCache.m_mdataOffset, fileCache.m_filePathHash.crc, fileCache.m_fileMetaHash.crc, fileCache.m_fileId, fileCache.m_metaLength, fileCache.m_compareFlag);
}

CTRL_FILE_RETCODE FileCacheParser::FlushToFile()
{
    CTRL_FILE_RETCODE ret = WriteToFile(m_writeBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_writeFd.flush();
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint64_t FileCacheParser::GetCurrentOffset()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_currWriteOffset;
}

CTRL_FILE_RETCODE FileCacheParser::ReadFileCacheEntries(std::queue<FileCache> &fcQueue, uint64_t offset,
    uint32_t totalCount, uint16_t metaFileIndex)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (totalCount == 0) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    return ReadEntries(fcQueue, totalCount, metaFileIndex);
}

CTRL_FILE_RETCODE FileCacheParser::ReadFileCacheEntries(std::queue<FileCache> &fcQueue, uint32_t maxEntries,
    uint16_t metaFileIndex)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (m_readFd.eof()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    return ReadEntries(fcQueue, maxEntries, metaFileIndex);
}

CTRL_FILE_RETCODE FileCacheParser::ReadEntries(std::queue<FileCache> &fcQueue,
    uint32_t maxEntries, uint16_t metaFileIndex)
{
    if (m_header.version == FCACHE_HEADER_VERSION_V10) {
        return ReadEntriesV10(fcQueue, maxEntries, metaFileIndex);
    } else if (m_header.version == FCACHE_HEADER_VERSION_V20) {
        return ReadEntriesV20(fcQueue, maxEntries);
    }

    uint32_t len = maxEntries * sizeof(FileCache);
    if (len > m_readBufferSize) {
        len = m_readBufferSize;
        maxEntries = CTRL_READ_BUFFER_SIZE;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read fcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *fcache = reinterpret_cast<FileCache *>(m_readBinaryBuffer);
    for (uint64_t i = 0; i < maxEntries; i++) {
        FileCache fcache1(fcache);
        if (fcache->m_inode == 0 && fcache->m_mdataOffset == 0) {
            break;
        }
        fcQueue.push(fcache1);
        fcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::ReadEntriesV20(std::queue<FileCache> &fcQueue, uint32_t maxEntries)
{
    uint32_t len = maxEntries * sizeof(FileCacheV20);
    if (len > m_readBufferSize) {
        len = m_readBufferSize;
        maxEntries = CTRL_READ_BUFFER_SIZE;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read fcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *fcache = reinterpret_cast<FileCacheV20 *>(m_readBinaryBuffer);
    for (uint64_t i = 0; i < maxEntries; i++) {
        FileCache fcache1(fcache);
        if (fcache->m_inode == 0 && fcache->m_hashTag == 0) {
            break;
        }
        fcQueue.push(fcache1);
        fcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileCacheParser::ReadEntriesV10(std::queue<FileCache> &fcQueue,
    uint32_t maxEntries, uint16_t metaFileIndex)
{
    uint32_t len = maxEntries * sizeof(FileCacheV10);
    if (len > m_readBufferSize) {
        len = m_readBufferSize;
        maxEntries = CTRL_READ_BUFFER_SIZE;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read fcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *fcache = reinterpret_cast<FileCacheV10 *>(m_readBinaryBuffer);
    for (uint64_t i = 0; i < maxEntries; i++) {
        FileCache fcache1(fcache);
        if (fcache->m_inode == 0 && fcache->m_hashTag == 0) {
            break;
        }
        fcache1.m_fileId = metaFileIndex;
        fcQueue.push(fcache1);
        fcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

std::string FileCacheParser::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}
