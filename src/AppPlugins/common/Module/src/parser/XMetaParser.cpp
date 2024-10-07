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
#include "XMetaParser.h"
#ifndef WIN32
#include "define/GenericEndian.h"
#endif
#include "securec.h"
#include "define/Types.h"
#include "Log.h"
#include "common/Thread.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "XMETA_PARSER";
constexpr uint32_t FOUR_MB = (4 * 1024 * 1024);
}

XMetaParser::XMetaParser(const XMetaParser::Params& params) : FileParser(true)
{
    m_fileName = params.m_fileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = XMETA_HEADER_TITLE_FS;
    m_header.version = XMETA_HEADER_VERSION_V10;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
    if (m_isCacheEnable) {
        AllocReadCache();
    }
}

XMetaParser::XMetaParser(std::string metaFileName, bool isCacheEnable) : FileParser(true)
{
    m_fileName = metaFileName;
    m_isCacheEnable = isCacheEnable;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_readBufferSize = sizeof(FileMeta);
    if (m_isCacheEnable) {
        AllocReadCache();
    }
}

XMetaParser::~XMetaParser()
{
    if (m_writeFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }

    if (m_readFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::READ);
    }
    FreeReadCache();
}

CTRL_FILE_RETCODE XMetaParser::OpenWrite()
{
    CTRL_FILE_RETCODE ret;
    m_offset = 0;
    if (m_writeFd.tellp() == 0) {
        return WriteHeader();
    } else {
        m_offset = (uint64_t) m_writeFd.tellp();
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadHeader()
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadMetaFileHeader();
        if (ret == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        }
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

CTRL_FILE_RETCODE XMetaParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        std::string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    m_offset = (uint64_t) headerBuffer.tellp();
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header For XMetaParser Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeFd.flush();
    HCP_Log(INFO, MODULE) << "Write Header completed for metafile: " << m_fileName << HCPENDLOG;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), XMETA_HEADER_TITLE_FS.c_str()) != 0) ||
        (strcmp(m_header.version.c_str(), XMETA_HEADER_VERSION_V10.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.metaDataScope.empty()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::GetHeader(XMetaParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadMetaFileHeader()
{
    uint32_t headerLine = 0;
    vector<std::string> cltHeaderLineSplit {};
    if (!m_readFd) {
        HCP_Log(ERR, MODULE) << "ReadHeader readFd not proper: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        std::string cltHeaderLine {};
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

    std::string blankLine {};
    getline(m_readFd, blankLine); /* To skip the blank line after header */
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::FillHeader(uint32_t &headerLine, vector<std::string> &cltHeaderLineSplit,
    std::string &cltHeaderLine)
{
    if (headerLine == XMETA_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == XMETA_TASKID) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == XMETA_METADATA_SCOPE) {
        m_header.metaDataScope = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

std::string XMetaParser::GetFileHeaderLine(uint32_t headerLine)
{
    std::string ctlHeaderLine {};
    switch (headerLine) {
        case XMETA_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case XMETA_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case XMETA_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case XMETA_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case XMETA_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case XMETA_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case XMETA_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case XMETA_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case XMETA_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case XMETA_METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE XMetaParser::AllocReadCache()
{
    if (m_readCache != nullptr) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    m_readCache  = (char *)malloc(FOUR_MB);
    if (m_readCache == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to malloc for readCache" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int ret = memset_s(m_readCache, FOUR_MB, 0, FOUR_MB);
    if (ret != 0) {
        ERRLOG("failed memset m_readCache %p, ret %d", m_readCache, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readCacheOffsetStart = 0;
    m_readCacheOffsetEnd = 0;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::FreeReadCache()
{
    if (m_readCache == nullptr) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    free(m_readCache);
    m_readCache = nullptr;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReAllocReadCache(uint32_t len)
{
    free(m_readCache);
    m_readCache  = (char *)malloc(len);
    if (m_readCache == nullptr) {
        HCP_Log(ERR, MODULE) << "Failed to malloc for readCache" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int ret = memset_s(m_readCache, len, 0, len);
    if (ret != 0) {
        ERRLOG("failed memset m_readCache %p, ret %d", m_readCache, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::FillTlvsDetails(uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs)
{
    char *bufferPtr = m_readBinaryBuffer;
    numOfTlvs = le32toh(*(uint32_t *)bufferPtr);
    bufferPtr += sizeof(uint32_t);
    lenOfAllTlvs = le32toh(*(uint32_t *)bufferPtr);
    bufferPtr += sizeof(uint32_t);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadXMetaFieldFromBuffer(uint64_t offset, XMetaField &field, uint32_t &tlvLen)
{
    char *bufferPtr = m_readBinaryBuffer + offset;
    uint16_t tmpType = 0;
    memcpy_s(&tmpType, sizeof(uint16_t), bufferPtr, sizeof(uint16_t));
    XMETA_TYPE xMetaType = (XMETA_TYPE)le16toh(tmpType);
    if (xMetaType <= XMETA_TYPE::XMETA_TYPE_DEFAULT || xMetaType >= XMETA_TYPE::XMETA_TYPE_MAX_LENGTH) {
        HCP_Log(ERR, MODULE) << "Invalid xMetaType: "<< (int)xMetaType << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    field.m_xMetaType = xMetaType;
    bufferPtr += sizeof(uint16_t);
    uint32_t tmpLen = 0;
    memcpy_s(&tmpLen, sizeof(uint32_t), bufferPtr, sizeof(uint32_t));
    tlvLen = le32toh(tmpLen);
    bufferPtr += sizeof(uint32_t);
    if (tlvLen == 0) {
        ERRLOG("tlv length is zero");
        return CTRL_FILE_RETCODE::FAILED;
    }
    char* value = new char[tlvLen];
    if (memcpy_s(value, tlvLen, bufferPtr, tlvLen) != 0) {
        delete[] value;
        value = nullptr;
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    field.m_value.assign(value, tlvLen);
    delete[] value;
    value = nullptr;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadBuffer(uint32_t numOfTlvs, uint32_t lenOfAllTlvs, vector<XMetaField> &entry)
{
    uint16_t offset = 0;
    while (numOfTlvs > 0) {
        numOfTlvs--;
        XMetaField tlv {};
        uint32_t tlvLen = 0;
        if (ReadXMetaFieldFromBuffer(offset, tlv, tlvLen) != CTRL_FILE_RETCODE::SUCCESS) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "failed to read XMeta from buffer of length "
                << m_readBufferSize << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        entry.emplace_back(tlv);
        offset += sizeof(uint16_t) + sizeof(uint32_t) + tlvLen;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadDataWithoutCache(uint64_t offset, vector<XMetaField> &entry)
{
    uint32_t numOfTlvs = 0;
    uint32_t lenOfAllTlvs = 0;
    uint32_t readLen = sizeof(uint32_t) + sizeof(uint32_t);

    m_offset = offset;
    if (ValidateMetaFile(offset) != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate directory meta failed filename: "
            << m_fileName << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read xmetafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    FillTlvsDetails(numOfTlvs, lenOfAllTlvs);
    offset += readLen;
    readLen = lenOfAllTlvs;

    if (readLen > m_readBufferSize) {
        m_readBufferSize = readLen;
        if (m_readBinaryBuffer != nullptr) {
            free (m_readBinaryBuffer);
        }
        m_readBinaryBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBinaryBuffer == nullptr) {
            HCP_Log(ERR, MODULE) << "failed to malloc buffer of size " << m_readBufferSize << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadFromBinaryFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadBuffer(numOfTlvs, lenOfAllTlvs, entry);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Read xmeta from buffer failed " << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::RefreshReadCache(uint64_t offset)
{
    uint32_t numOfTlvs = 0;
    uint32_t lenOfAllTlvs = 0;
    uint32_t readLen = FOUR_MB;

    if (offset >= m_readCacheOffsetStart && (offset + sizeof(uint32_t) + sizeof(uint32_t)) <= m_readCacheOffsetEnd) {
        if (CTRL_FILE_RETCODE::SUCCESS != ReadXMetaSize(offset, numOfTlvs, lenOfAllTlvs)) {
            HCP_Log(ERR, MODULE) << "Invalid xmeta entry size in XMeta file: " << numOfTlvs
                << " offset: " << offset << " buffer: " << m_readBinaryBuffer << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (offset + lenOfAllTlvs <= m_readCacheOffsetEnd) {
            return CTRL_FILE_RETCODE::SUCCESS;
        }
        readLen = lenOfAllTlvs + sizeof(uint32_t) + sizeof(uint32_t);
        if (readLen < FOUR_MB) {
            readLen = FOUR_MB;
        } else if (ReAllocReadCache(readLen) != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Realocate xmeta buffer failed. length: "
                << readLen << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }

    CTRL_FILE_RETCODE ret = ReadData(offset, readLen, numOfTlvs, lenOfAllTlvs);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        if (lenOfAllTlvs == 0) {
            HCP_Log(ERR, MODULE) << "Read xmeta entry failed"  << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        readLen = lenOfAllTlvs + sizeof(uint32_t) + sizeof(uint32_t);
        if (readLen > FOUR_MB) {
            if (ReAllocReadCache(readLen) != CTRL_FILE_RETCODE::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Realocate xmeta buffer failed. length: "
                    << readLen << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
        } else {
            readLen = FOUR_MB;
        }
        ret = ReadData(offset, readLen, numOfTlvs, lenOfAllTlvs);
        return ret;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadData(uint64_t &offset, uint32_t &readLen,
    uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs)
{
    CTRL_FILE_RETCODE ret = FillReadCache(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to refresh cache, error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (offset >= m_readCacheOffsetStart && (offset + sizeof(uint32_t) + sizeof(uint32_t)) <= m_readCacheOffsetEnd) {
        if (CTRL_FILE_RETCODE::SUCCESS != ReadXMetaSize(offset, numOfTlvs, lenOfAllTlvs)) {
            HCP_Log(ERR, MODULE) << "Invalid xmeta entry size in XMeta file: " << numOfTlvs
                << " offset: " << offset << " buffer: " << m_readBinaryBuffer << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (offset + lenOfAllTlvs <= m_readCacheOffsetEnd) {
            return CTRL_FILE_RETCODE::SUCCESS;
        } else {
            HCP_Log(ERR, MODULE) << "Xmeta length exceeds read length.  lenOfAllTlvs: " << lenOfAllTlvs << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        return CTRL_FILE_RETCODE::FAILED;
    }
}

CTRL_FILE_RETCODE XMetaParser::FillReadCache(uint64_t offset, uint32_t readLen)
{
    int ret = memset_s(m_readCache, readLen, 0, readLen);
    if (ret != 0) {
        ERRLOG("failed memset m_readCache %p, size %u, ret %d", m_readCache, readLen, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readCacheOffsetStart = 0;
    m_readCacheOffsetEnd = 0;
    m_readFd.seekg(offset);
    if (m_readFd.fail()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.read(m_readCache, readLen);
    if ((!m_readFd.eof()) && (!m_readFd.good())) {
        m_readFd.close();
        CTRL_FILE_RETCODE ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
        if (ret != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_readFd.seekg(offset);
        if (m_readFd.fail()) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_readFd.read(m_readCache, readLen);
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            m_readFd.close();
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    if (m_readFd.eof() && m_readFd.fail()) {
        m_readCacheOffsetStart = offset;
        m_readCacheOffsetEnd = offset + m_readFd.gcount();
        m_readFd.clear();
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    m_readCacheOffsetStart = offset;
    m_readCacheOffsetEnd = offset + readLen;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ValidateMetaFile(uint64_t offset)
{
    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "metaFile given is not open" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_readFd.seekg(offset);
    if (m_readFd.fail()) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(WARN, MODULE) << "failed to seek in metafile"
            " to offset " << offset << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadXMetaFieldFromReadCache(uint64_t offset, XMetaField &field, uint32_t &tlvLen)
{
    char *bufferPtr = m_readCache + (offset - m_readCacheOffsetStart);
    XMETA_TYPE xMetaType = (XMETA_TYPE)(le16toh(*(uint16_t *)bufferPtr));
    if (xMetaType <= XMETA_TYPE::XMETA_TYPE_DEFAULT || xMetaType >= XMETA_TYPE::XMETA_TYPE_MAX_LENGTH) {
        HCP_Log(ERR, MODULE) << "Invalid xMetaType: "<< (int)xMetaType << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    field.m_xMetaType = xMetaType;
    bufferPtr += sizeof(uint16_t);
	tlvLen = le32toh(*(uint32_t *)bufferPtr);
    bufferPtr += sizeof(uint32_t);
    if (tlvLen == 0) {
        ERRLOG("tlv length is zero");
        return CTRL_FILE_RETCODE::FAILED;
    }
    char* value = new char[tlvLen];
    if (memcpy_s(value, tlvLen, bufferPtr, tlvLen) != 0) {
        delete[] value;
        value = nullptr;
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    field.m_value.assign(value, tlvLen);
    delete[] value;
    value = nullptr;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadXMetaSize(uint64_t offset, uint32_t &numOfTlvs, uint32_t &lenOfAllTlvs)
{
    char *bufferPtr = m_readCache + (offset - m_readCacheOffsetStart);
    numOfTlvs = le32toh(*(uint32_t *)bufferPtr);
    bufferPtr += sizeof(uint32_t);
    lenOfAllTlvs = le32toh(*(uint32_t *)bufferPtr);
    bufferPtr += sizeof(uint32_t);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE XMetaParser::ReadXMeta(vector<XMetaField> &entry, uint64_t offset)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t numOfTlvs = 0;
    uint32_t lenOfAllTlvs = 0;
    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "metaFile given is not open" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (!m_isCacheEnable) {
        if (ReadDataWithoutCache(offset, entry) != CTRL_FILE_RETCODE::SUCCESS) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(DirMeta) << " error= "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
                << " metafile: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    if (RefreshReadCache(offset) != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(DirMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    /**    --------------------------------------------
     *    | numOfTlvs | lenOfAllTlvs | T | L | V | ....|
     *      -------------------------------------------    */
    if (CTRL_FILE_RETCODE::SUCCESS != ReadXMetaSize(offset, numOfTlvs, lenOfAllTlvs)) {
        HCP_Log(ERR, MODULE) << "Invalid xmeta entry size in XMeta file: " << numOfTlvs
            << " offset: " << offset << " buffer: " << m_readBinaryBuffer << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    offset += sizeof(uint32_t) + sizeof(uint32_t);

    while (numOfTlvs > 0) {
        numOfTlvs--;
        XMetaField tlv {};
        uint32_t tlvLen = 0;
        if (ReadXMetaFieldFromReadCache(offset, tlv, tlvLen) != CTRL_FILE_RETCODE::SUCCESS) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "failed to read XMeta from buffer of length "
                << m_readBufferSize << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        entry.emplace_back(tlv);
        offset += sizeof(uint16_t) + sizeof(uint32_t) + tlvLen;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

uint64_t XMetaParser::WriteXMeta(vector<XMetaField> &entry)
{
    /**    --------------------------------------------
     *    | numOfTlvs | lenOfAllTlvs | T | L | V | ....|
     *      -------------------------------------------    */

    uint32_t lenOfAllTlvs = 0;
    std::stringstream tlvBuffer {};
    lock_guard<std::mutex> lk(m_lock);

    for (const XMetaField& field : entry) {
        uint16_t type = htole16(static_cast<uint16_t>(field.m_xMetaType));
        uint32_t len = htole32(field.m_value.size());
        if (field.m_value.size() > 0) {
            tlvBuffer.write(reinterpret_cast<char *>(&type), sizeof(uint16_t));
            tlvBuffer.write(reinterpret_cast<char *>(&len), sizeof(uint32_t));
            if (field.m_xMetaType == XMETA_TYPE::XMETA_TYPE_NFSFH) {
                tlvBuffer.write(field.m_value.c_str(), field.m_value.size());
            } else {
                tlvBuffer << field.m_value.c_str();
            }
            lenOfAllTlvs += sizeof(uint16_t) + sizeof(uint32_t) + field.m_value.size();
        }
    }
    uint32_t numOfTlvs = htole32(entry.size());
    uint32_t tLenOfAllTlvs = htole32(lenOfAllTlvs);
    m_writeBuffer.write(reinterpret_cast<char *>(&numOfTlvs), sizeof(uint32_t));
    m_writeBuffer.write(reinterpret_cast<char *>(&tLenOfAllTlvs), sizeof(uint32_t));
    m_writeBuffer << tlvBuffer.str();
    uint64_t totalLenWritten = sizeof(uint32_t) + sizeof(uint32_t) + lenOfAllTlvs;
    m_offset += totalLenWritten;
    return totalLenWritten;
}

void XMetaParser::PrintEntries(std::vector<XMetaField>& entry)
{
    DBGLOG("Write to xmeta - %s", m_fileName.c_str());
    for (auto tmp : entry) {
        DBGLOG("entry type: %d, entry value: %s", static_cast<int>(tmp.m_xMetaType), tmp.m_value.c_str());
    }
}

uint64_t XMetaParser::GetCurrentOffset()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_offset;
}

CTRL_FILE_RETCODE XMetaParser::FlushToFile()
{
    CTRL_FILE_RETCODE ret = WriteToFile(m_writeBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_writeFd.flush();
    HCP_Log(INFO, MODULE) << "Metafile flush data completed filename: " << m_fileName
        << " offset: " << m_writeFd.tellp() << " offset variable: " << m_offset << HCPENDLOG;
    return CTRL_FILE_RETCODE::SUCCESS;
}

std::string XMetaParser::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}

CTRL_FILE_RETCODE XMetaParser::ReadLenFromFile(uint32_t len)
{
    if (len > m_readBufferSize) {
        m_readBufferSize = len;
        if (m_readBinaryBuffer != nullptr) {
            free(m_readBinaryBuffer);
        }
        m_readBinaryBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBinaryBuffer == nullptr) {
            HCP_Log(ERR, MODULE) << "failed to malloc buffer of size " << m_readBufferSize << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    int ret = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (ret != 0) {
        ERRLOG("failed memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ReadFromBinaryFile(m_readFd.tellg(), len);
}
