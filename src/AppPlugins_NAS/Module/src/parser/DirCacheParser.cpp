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
#include "DirCacheParser.h"
#include "securec.h"
#include "Log.h"
#include "common/Thread.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "DCACHE_PARSER";
}

DirCacheParser::DirCacheParser(DirCacheParser::Params params) : FileParser(true)
{
    m_fileName = params.fileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = DCACHE_HEADER_TITLE;
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

DirCacheParser::DirCacheParser(std::string dcacheFileName) : FileParser(true)
{
    m_fileName = dcacheFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_readBufferSize = (DCACHE_MAX_READBUFF_SIZE * sizeof(DirCache));
}

DirCacheParser::~DirCacheParser()
{
    if (m_writeFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }

    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE DirCacheParser::OpenWrite()
{
    return WriteHeader();
}

CTRL_FILE_RETCODE DirCacheParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::GetHeader(Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::ReadHeader()
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadDCacheFileHeader();
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

CTRL_FILE_RETCODE DirCacheParser::ReadDCacheFileHeader()
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

CTRL_FILE_RETCODE DirCacheParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == DCACHE_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == DCACHE_TASKID) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_METADATA_SCOPE) {
        m_header.metaDataScope = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header For DCache Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string DirCacheParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case DCACHE_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case DCACHE_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case DCACHE_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case DCACHE_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case DCACHE_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case DCACHE_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case DCACHE_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case DCACHE_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case DCACHE_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case DCACHE_METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE DirCacheParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), DCACHE_HEADER_TITLE.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.metaDataScope.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    if ((strcmp(m_header.version.c_str(), DCACHE_HEADER_VERSION_V20.c_str()) != 0) &&
        (strcmp(m_header.version.c_str(), DCACHE_HEADER_VERSION_V10.c_str()) != 0)) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::WriteDirCache(DirCache &dcache, DCACHE_WRITE_INFO writeInfo)
{
    lock_guard<std::mutex> lk(m_lock);
    if (writeInfo == DCACHE_WRITE_INFO::DCACHE_ADD_TO_QUEUE) {
        if (m_header.version == DCACHE_HEADER_VERSION_V30) {
            m_dirCacheQueue.push(dcache);
        } else {
            m_dirCacheQueueV20.push(dcache);
        }
    } else if (writeInfo == DCACHE_WRITE_INFO::DCACHE_WRITE_TO_BUFFER) {
        WriteToBfrBasedOnVersion(dcache);
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

void DirCacheParser::ConvertDirCacheToV20(const DirCache &dcache, DirCacheV20 &dcacheV20)
{
    dcacheV20.m_inode          = dcache.m_inode;
    dcacheV20.m_mdataOffset    = dcache.m_mdataOffset;
    dcacheV20.m_fcacheOffset   = dcache.m_fcacheOffset;
    dcacheV20.m_hashTag        = dcache.m_dirPathHash.crc;
    dcacheV20.m_crc            = dcache.m_dirMetaHash.crc;
    dcacheV20.m_totalFiles     = dcache.m_totalFiles;
    dcacheV20.m_fileId         = dcache.m_fileId;
    dcacheV20.m_fcacheFileId   = dcache.m_fcacheFileId;
    dcacheV20.m_metaLength     = dcache.m_metaLength;
}

void DirCacheParser::PrintDirCache(DirCache& dcache)
{
    DBGLOG("Write dcahce file - %s", m_fileName.c_str());
    DBGLOG("Write dcache - inode: %u, mdataOffset: %u, fcacheOffset: %u, hashTag: %u, m_crc: %u, totalFiles: %u, ",
        " fileId: %u, fcacheFileId: %u, metaLength: %u",
        dcache.m_inode, dcache.m_mdataOffset, dcache.m_fcacheOffset, dcache.m_dirPathHash.crc, dcache.m_dirMetaHash.crc,
        dcache.m_totalFiles, dcache.m_fileId, dcache.m_fcacheFileId, dcache.m_metaLength);
}

int32_t DirCacheParser::WriteDirCacheEntries(std::queue<DirCache> &dcQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    int32_t count = 0;
    while (!dcQueue.empty()) {
        DirCache dcache = dcQueue.front();
        dcQueue.pop();
        count++;
        WriteToBfrBasedOnVersion(dcache);
    }
    return count;
}

int32_t DirCacheParser::WriteDirCacheEntries(
    std::priority_queue<DirCache, std::vector<DirCache>, Comparator> &dcQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    int32_t count = 0;
    while (!dcQueue.empty()) {
        DirCache dcache = dcQueue.top();
        dcQueue.pop();
        count++;
        WriteToBfrBasedOnVersion(dcache);
    }
    return count;
}

void DirCacheParser::WriteToBfrBasedOnVersion(DirCache &dcache)
{
    if (m_header.version == DCACHE_HEADER_VERSION_V30) {
        m_writeBuffer.write((char *)&dcache, sizeof(dcache));
    } else {
        DirCacheV20 dcacheV20 {};
        ConvertDirCacheToV20(dcache, dcacheV20);
        m_writeBuffer.write((char *)&dcacheV20, sizeof(dcacheV20));
    }
}

CTRL_FILE_RETCODE DirCacheParser::FlushToFile()
{
    if (m_header.version == DCACHE_HEADER_VERSION_V30) {
        while (!m_dirCacheQueue.empty()) {
            DirCache dcache = m_dirCacheQueue.top();
            m_dirCacheQueue.pop();
            m_writeBuffer.write((char *)&dcache, sizeof(dcache));
        }
    } else {
        while (!m_dirCacheQueueV20.empty()) {
            DirCache dcache = m_dirCacheQueueV20.top();
            m_dirCacheQueueV20.pop();
            DirCacheV20 dcacheV20 {};
            ConvertDirCacheToV20(dcache, dcacheV20);
            m_writeBuffer.write((char *)&dcacheV20, sizeof(dcacheV20));
        }
    }

    CTRL_FILE_RETCODE ret = WriteToFile(m_writeBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint64_t DirCacheParser::GetSize()
{
    lock_guard<std::mutex> lk(m_lock);
    if (m_header.version == DCACHE_HEADER_VERSION_V30) {
        return m_dirCacheQueue.size();
    } else {
        return m_dirCacheQueueV20.size();
    }
}

CTRL_FILE_RETCODE DirCacheParser::ReadDirCacheEntries(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead)
{
    lock_guard<std::mutex> lk(m_lock);
    if (m_header.version == DCACHE_HEADER_VERSION_V10) {
        return ReadDirCacheEntriesV10(dcQueue, numOfEntriesToRead);
    } else if (m_header.version == DCACHE_HEADER_VERSION_V20) {
        return ReadDirCacheEntriesV20(dcQueue, numOfEntriesToRead);
    }
    uint32_t len = numOfEntriesToRead * sizeof(DirCache);
    if (len > m_readBufferSize) {
        HCP_Log(WARN, MODULE) << "Cannot read " << numOfEntriesToRead
            << " entries, change to " << m_readBufferSize / sizeof(DirCache) << HCPENDLOG;
        len = m_readBufferSize;
        numOfEntriesToRead = m_readBufferSize / sizeof(DirCache);
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (m_readFd.eof()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read dcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *dcache = reinterpret_cast<DirCache *>(m_readBinaryBuffer);
    for (uint32_t i = 0; i < numOfEntriesToRead; i++) {
        DirCache dcache1(dcache);
        if (dcache->m_inode == 0 && dcache->m_mdataOffset == 0) {
            break;
        }
        dcQueue.push(dcache1);
        dcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::ReadDirCacheEntriesV20(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead)
{
    uint32_t len = numOfEntriesToRead * sizeof(DirCacheV20);
    if (len > m_readBufferSize) {
        HCP_Log(WARN, MODULE) << "Cannot read " << numOfEntriesToRead
            << " entries, change to " << m_readBufferSize / sizeof(DirCacheV20) << HCPENDLOG;
        len = m_readBufferSize;
        numOfEntriesToRead = m_readBufferSize / sizeof(DirCacheV20);
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (m_readFd.eof()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read dcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *dcache = reinterpret_cast<DirCacheV20 *>(m_readBinaryBuffer);
    for (uint32_t i = 0; i < numOfEntriesToRead; i++) {
        DirCache dcache1(dcache);
        if (dcache->m_inode == 0 && dcache->m_hashTag == 0) {
            break;
        }
        dcQueue.push(dcache1);
        dcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DirCacheParser::ReadDirCacheEntriesV10(std::queue<DirCache> &dcQueue, uint32_t numOfEntriesToRead)
{
    uint32_t len = numOfEntriesToRead * sizeof(DirCacheV10);
    if (len > m_readBufferSize) {
        HCP_Log(WARN, MODULE) << "Cannot read " << numOfEntriesToRead
            << " entries, change to " << m_readBufferSize / sizeof(DirCacheV10) << HCPENDLOG;
        len = m_readBufferSize;
        numOfEntriesToRead = m_readBufferSize / sizeof(DirCacheV10);
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (m_readFd.eof()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read dcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    auto *dcache = reinterpret_cast<DirCacheV10 *>(m_readBinaryBuffer);
    for (uint32_t i = 0; i < numOfEntriesToRead; i++) {
        DirCache dcache1(dcache);
        if (dcache->m_inode == 0 && dcache->m_hashTag == 0) {
            break;
        }
        dcQueue.push(dcache1);
        dcache++;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

std::string DirCacheParser::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}

std::string DirCacheParser::GetVersion()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_header.version;
}
