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
#include <random>
#include "ScannerDirCache.h"

using namespace std;
using namespace ScannerDirCache;

CacheFile::CacheFile(Params params)
{
    m_dcacheFileName = params.fileName;
    m_dcacheFileParentDir = GetParentDirOfFile(m_dcacheFileName);

    m_header.title = NAS_SCANNERBACKUPDCACHE_HEADER_TITLE;
    m_header.version = NAS_SCANNERBACKUPDCACHE_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

CacheFile::CacheFile(string dcacheFileName)
{
    m_dcacheFileName = dcacheFileName;
    m_dcacheFileParentDir = GetParentDirOfFile(m_dcacheFileName);
    m_readBufferSize = (NAS_SCANNERBACKUPDCACHE_MAX_READBUFF_SIZE * sizeof(Cache));
}

CacheFile::~CacheFile()
{
    if (m_writeFd.is_open()) {
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);
    }

    if (m_readFd.is_open()) {
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
    }
}

NAS_CTRL_FILE_RETCODE CacheFile::Open(NAS_CTRL_FILE_OPEN_MODE mode)
{
    NAS_CTRL_FILE_RETCODE ret;
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        m_readBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBuffer == nullptr) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "readBuffer malloc failed, Error: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " file: " << m_dcacheFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            free(m_readBuffer);
            m_readBuffer = nullptr;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ReadDCacheFileHeader();
        if (ValidateHeader() != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Header verification failed for: " << m_dcacheFileName << HCPENDLOG;
            free(m_readBuffer);
            m_readBuffer = nullptr;
            m_readFd.close();
            return NAS_CTRL_FILE_RET_FAILED;
        }
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::binary | std::ios::app);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ret = WriteHeader();
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for :" << m_dcacheFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE CacheFile::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    strmFd.open(m_dcacheFileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(m_dcacheFileParentDir) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_dcacheFileName << ", Parent dir not reachable";
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(m_dcacheFileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_dcacheFileName << ", ERR: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::Close(NAS_CTRL_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (!m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        NAS_CTRL_FILE_RETCODE ret = FlushToFile();
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_writeFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (m_readBuffer) {
            free(m_readBuffer);
        }
        if (!m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        m_readFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid close mode for:" << m_dcacheFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

NAS_CTRL_FILE_RETCODE CacheFile::GetHeader(Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::ReadDCacheFileHeader()
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadHeader();
        if (ret == NAS_CTRL_FILE_RET_SUCCESS) {
            break;
        }
        m_readFd.sync();
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed. readFD is not good file: "
                << m_dcacheFileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.close();
            ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_CTRL_FILE_RET_FAILED;
            }
        } else {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed. readFD is good file: "
                << m_dcacheFileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.seekg(0);
        }
        retryCnt++;
        uint32_t sleepDuration = GetRandomNumber(NAS_HDR_RETRY_SLEEP_DUR_MIN_MSEC,
            NAS_HDR_RETRY_SLEEP_DUR_MAX_MSEC);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
    } while (retryCnt < NAS_CTRL_FILE_SERVER_RETRY_CNT);
    return ret;
}

NAS_CTRL_FILE_RETCODE CacheFile::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};
    if (!m_readFd) {
        HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader readFd not proper: " << m_dcacheFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    while (headerLine < NAS_CTRL_FILE_NUMBER_TEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readFd, cltHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_dcacheFileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_dcacheFileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_dcacheFileName << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (FillHeader(headerLine, cltHeaderLineSplit, cltHeaderLine) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readFd, blankLine); /* To skip the blank line after header */
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == DCACHE_TITLE) {
        m_header.title = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_dcacheFileName
                << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == DCACHE_TASKID) {
        m_header.taskId = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == DCACHE_METADATA_SCOPE) {
        m_header.metaDataScope = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_dcacheFileName << HCPENDLOG;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};
    NAS_CTRL_FILE_RETCODE ret;

    while (headerLine < NAS_CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    WRITE_TO_FILE(headerBuffer, m_writeFd, NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write Header For DCache Failed: " << m_dcacheFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string CacheFile::GetFileHeaderLine(uint32_t headerLine)
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
            HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE CacheFile::ValidateHeader()
{
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::WriteDirCache(Cache &dcache, DCACHE_WRITE_INFO writeInfo)
{
    lock_guard<std::mutex> lk(m_lock);
    if (writeInfo == DCACHE_ADD_TO_QUEUE) {
        this->m_dirCacheQueue.push(dcache);
    } else if (writeInfo == DCACHE_WRITE_TO_BUFFER) {
        m_writeBuffer.write((char *)&dcache, sizeof(dcache));
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

int32_t CacheFile::WriteDirCacheEntries(std::queue<Cache> &dcQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    int32_t count = 0;
    while (!dcQueue.empty()) {
        Cache dcache = dcQueue.front();
        dcQueue.pop();
        count++;
        m_writeBuffer.write((char *)&dcache, sizeof(dcache));
    }
    return count;
}

int32_t CacheFile::WriteDirCacheEntries(
    std::priority_queue<Cache, std::vector<Cache>, Comparator> &dcQueue)
{
    lock_guard<std::mutex> lk(m_lock);
    int32_t count = 0;
    while (!dcQueue.empty()) {
        Cache dcache = dcQueue.top();
        dcQueue.pop();
        count++;
        m_writeBuffer.write((char *)&dcache, sizeof(dcache));
    }
    return count;
}

NAS_CTRL_FILE_RETCODE CacheFile::FlushToFile()
{
    while (!m_dirCacheQueue.empty()) {
        Cache dcache = m_dirCacheQueue.top();
        m_dirCacheQueue.pop();
        m_writeBuffer.write((char *)&dcache, sizeof(dcache));
    }

    NAS_CTRL_FILE_RETCODE ret;
    WRITE_TO_FILE(m_writeBuffer, m_writeFd, NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    return NAS_CTRL_FILE_RET_SUCCESS;
}

uint64_t CacheFile::GetSize()
{
    lock_guard<std::mutex> lk(m_lock);
    return this->m_dirCacheQueue.size();
}

NAS_CTRL_FILE_RETCODE CacheFile::ReadDirCacheEntries(std::queue<Cache> &dcQueue, uint32_t numOfEntriesToRead)
{
    lock_guard<std::mutex> lk(m_lock);
    if (m_header.version == NAS_SCANNERBACKUPDCACHE_HEADER_VERSION_V10) {
        return ReadDirCacheEntriesV10(dcQueue, numOfEntriesToRead);
    }
    uint32_t len = numOfEntriesToRead * sizeof(Cache);
    if (len > m_readBufferSize) {
        HCP_Log(WARN, CTL_MOD_NAME) << "Cannot read " << numOfEntriesToRead
            << " entries, change to " << m_readBufferSize/sizeof(Cache) << HCPENDLOG;
        len = m_readBufferSize;
        numOfEntriesToRead = m_readBufferSize / sizeof(Cache);
    }
    int result = memset_s(m_readBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "readBuffer memset_s failed" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (m_readFd.eof()) {
        return NAS_CTRL_FILE_RET_SUCCESS;
    }
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    READ_FROM_BINARY_FILE(m_readBuffer, m_readFd.tellg(), len, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read dcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_dcacheFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    auto *dcache = reinterpret_cast<Cache *>(m_readBuffer);
    for (uint32_t i = 0; i < numOfEntriesToRead; i++) {
        Cache dcache1(dcache);
        if (dcache->m_inode == 0 && dcache->m_hashTag == 0) {
            break;
        }
        dcQueue.push(dcache1);
        dcache++;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CacheFile::ReadDirCacheEntriesV10(std::queue<Cache> &dcQueue, uint32_t numOfEntriesToRead)
{
    uint32_t len = numOfEntriesToRead * sizeof(CacheV10);
    if (len > m_readBufferSize) {
        HCP_Log(WARN, CTL_MOD_NAME) << "Cannot read " << numOfEntriesToRead
            << " entries, change to " << m_readBufferSize/sizeof(CacheV10) << HCPENDLOG;
        len = m_readBufferSize;
        numOfEntriesToRead = m_readBufferSize / sizeof(CacheV10);
    }
    int result = memset_s(m_readBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "readBuffer memset_s failed" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (m_readFd.eof()) {
        return NAS_CTRL_FILE_RET_SUCCESS;
    }
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    READ_FROM_BINARY_FILE(m_readBuffer, m_readFd.tellg(), len, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read dcachefile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " fileName: " << m_dcacheFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    auto *dcache = reinterpret_cast<CacheV10 *>(m_readBuffer);
    for (uint32_t i = 0; i < numOfEntriesToRead; i++) {
        Cache dcache1(dcache);
        if (dcache->m_inode == 0 && dcache->m_hashTag == 0) {
            break;
        }
        dcQueue.push(dcache1);
        dcache++;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

std::string CacheFile::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_dcacheFileName;
}

uint32_t CacheFile::GetRandomNumber(uint32_t minNum, uint32_t maxNum)
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(minNum, maxNum);
    uint32_t randomNum = (uint32_t)distrib(gen);
    return randomNum;
}