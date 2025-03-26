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
#include "define/Defines.h"
#include "define/Types.h"
#include "securec.h"
#include "Log.h"
#include "common/Thread.h"
#ifndef WIN32
#include "define/GenericEndian.h"
#endif
#include "AdsParser.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "ADS_PARSER";
constexpr uint32_t FOUR_MB = (4 * 1024 * 1024);
}

AdsParser::AdsParser(const AdsParser::Params& params) : FileParser(true)
{
    m_fileName = params.m_fileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_header.title = ADS_HEADER_FILE;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.backupType = params.backupType;
}

AdsParser::AdsParser(const std::string& AdsFileName) : FileParser(true)
{
    m_fileName = AdsFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_isCacheEnable = false;
    m_readBufferSize = sizeof(AdsStruct);
}

AdsParser::~AdsParser()
{
    if (m_writeFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }

    if (m_readFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::READ);
    }
    FreeReadCache();
}

CTRL_FILE_RETCODE AdsParser::OpenWrite()
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

CTRL_FILE_RETCODE AdsParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::ReadHeader()
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadMetaFileHeader();
        if (ret == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        }
        m_readFd.sync();
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed. readFD is not good file: "
                << m_fileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.close();
            ret = FileOpen<std::ifstream>(m_readFd, ios::in | ios::binary);
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

CTRL_FILE_RETCODE AdsParser::WriteHeader()
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
        HCP_Log(ERR, MODULE) << "Write Header For AdsParser Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeFd.flush();
    HCP_Log(DEBUG, MODULE) << "Write Header completed for metafile: " << m_fileName << HCPENDLOG;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), ADS_HEADER_FILE.c_str()) != 0) ||
        (strcmp(m_header.version.c_str(), ADS_HEADER_VERSION_V10.c_str()) != 0) ||
        m_header.timestamp.empty() ||
        m_header.taskId.empty() ||
        m_header.backupType.empty() ||
        m_header.fileCount == 0 ||
        m_header.streamCount == 0 ||
        m_header.dataSize == 0) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::GetHeader(AdsParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::FillHeader(uint32_t headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine   )
{
    if (headerLine == ADS_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == ADS_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == ADS_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == ADS_TASKID) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == ADS_BACKUP_TYPE) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == ADS_FILE_COUNT) {
        m_header.fileCount = std::stoul(cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]);
    } else if (headerLine == ADS_STREAM_COUNT) {
        m_header.streamCount = std::stoul(cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]);
    } else if (headerLine == ADS_DATA_SIZE) {
        m_header.dataSize = std::stoul(cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]);
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::ReadMetaFileHeader()
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

        try {
            boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        } catch (const std::exception& e) {
            HCP_Log(ERR, MODULE) << "split Header failed" << e.what()<< HCPENDLOG;
        }

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

string AdsParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case ADS_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case ADS_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case ADS_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case ADS_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case ADS_BACKUP_TYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case ADS_FILE_COUNT:
            ctlHeaderLine = "FileCount:" + std::to_string(m_header.fileCount) + "\n";
            break;
        case ADS_STREAM_COUNT:
            ctlHeaderLine = "StreamCount:" + std::to_string(m_header.streamCount) + "\n";
            break;
        case ADS_DATA_SIZE:
            ctlHeaderLine = "DataSize:" + std::to_string(m_header.dataSize) + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: "<< headerLine << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

size_t AdsParser::ReadAdsEntry(uint64_t offset,
                               uint8_t* buffer,
                               uint32_t& nextIndex,
                               uint64_t& nextOffset,
                               bool& isLast)
{
    lock_guard<std::mutex> lk(m_lock);
    int currentTellg = m_readFd.tellg();
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    m_offset = offset;
    if (!ValidateAdsEntryOfferset(offset)) {
        return 0;
    }
    currentTellg = m_readFd.tellg();
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer,size %u, ret %d", m_readBufferSize, result);
        return 0;
    }
    if (!ReadadsEntryOffsetFromBinaryFile(offset)) {
        return 0;
    }
    AdsStruct adsEntry {};
    ret = ReadadsEntryFromBuffer(adsEntry);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read File MetaParser from buffer of length "
            << sizeof(FileMeta) << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
        return 0;
    }
    memset_s(buffer, adsEntry.currentStreamLength, 0, adsEntry.currentStreamLength);
    // read stream data
    if (!ValidateCurrentStreamOffset(adsEntry)) {
        return 0;
    }
    offset = adsEntry.currentStreamOffset;

    if (!ReadCurrentStreamLengthFromBinaryFile(offset, adsEntry.currentStreamLength)) {
        return 0;
    }

    char* bufferCpy = m_readBinaryBuffer;
    ret = CTRL_FILE_RETCODE::SUCCESS;
    if (memcpy_s(buffer, adsEntry.currentStreamLength, bufferCpy, adsEntry.currentStreamLength) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return 0;
    }
    nextIndex = adsEntry.nextIndex;
    nextOffset = adsEntry.nextOffset;
    isLast = adsEntry.isLastStruct;
    return static_cast<size_t>(adsEntry.currentStreamLength);
}

bool AdsParser::ReadCurrentStreamLengthFromBinaryFile(uint64_t offset, uint32_t readLen)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    ret = ReadFromBinaryFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return false;
    }
    return true;
}

bool AdsParser::ValidateAdsEntryOfferset(uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    ret = ValidateMetaFile(offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate directory meta failed filename: "
            << m_fileName << " offset: " << offset << HCPENDLOG;
        return false;
    }
    return true;
}

bool AdsParser::ReadadsEntryOffsetFromBinaryFile(uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    ret = ReadFromBinaryFile(offset, sizeof(AdsStruct));
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(AdsStruct) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return false;
    }
    return true;
}

bool AdsParser::ValidateCurrentStreamOffset(AdsStruct adsEntry)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    ret = ValidateMetaFile(adsEntry.currentStreamOffset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate directory meta failed filename: "
            << m_fileName << " offset: " << adsEntry.currentStreamOffset << HCPENDLOG;
        return false;
    }
    return true;
}

CTRL_FILE_RETCODE AdsParser::ReadadsEntryFromBuffer(AdsStruct &adsEntry)
{
    char *bufferCpy = m_readBinaryBuffer;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    if (memcpy_s(&adsEntry, sizeof(AdsStruct), bufferCpy, sizeof(AdsStruct)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(adsEntry);
    return CTRL_FILE_RETCODE::SUCCESS;
}

void AdsParser::TranslateToHostEndian(AdsStruct& adsEntry)
{
    adsEntry.currentBlock = le32toh(adsEntry.currentBlock);
    adsEntry.nextIndex = le32toh(adsEntry.nextIndex);
    adsEntry.numOfStreams = le32toh(adsEntry.numOfStreams);
    adsEntry.currentStreamIndex = le32toh(adsEntry.currentStreamIndex);
    adsEntry.nextOffset = le64toh(adsEntry.nextOffset);
    adsEntry.currentStreamLength = le32toh(adsEntry.currentStreamLength);
    adsEntry.currentStreamOffset = le64toh(adsEntry.currentStreamOffset);
}

uint16_t AdsParser::WriteAdsEntry(AdsStruct& adsEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    adsEntry.nextIndex = 0;
    adsEntry.currentStreamOffset = m_offset + sizeof(adsEntry);
    adsEntry.nextOffset = m_offset + sizeof(adsEntry) + adsEntry.currentStreamLength;
    char *adsEntryPtr = reinterpret_cast<char *>(&adsEntry);
    TranslateToLittleEndian(adsEntry);
    m_writeBuffer.write(reinterpret_cast<char *>(&adsEntry), sizeof(adsEntry));
    uint16_t adsLength = sizeof(adsEntry);
    m_offset += adsLength;
    m_writeBuffer.write(reinterpret_cast<char *>(&adsEntry.streamData[0]), adsEntry.currentStreamLength);
    m_offset += adsEntry.currentStreamLength;

    if (m_writeBuffer.tellp() > BINARY_MAX_BUFFER_SIZE) {
        FlushToFile();
    }
    return sizeof(uint32_t) + adsLength;
}

void AdsParser::TranslateToLittleEndian(AdsStruct& adsEntry)
{
    adsEntry.currentBlock = htole32(adsEntry.currentBlock);
    adsEntry.nextIndex = htole32(adsEntry.nextIndex);
    adsEntry.numOfStreams = htole32(adsEntry.numOfStreams);
    adsEntry.currentStreamIndex = htole32(adsEntry.currentStreamIndex);
    adsEntry.nextOffset = htole64(adsEntry.nextOffset);
    adsEntry.currentStreamLength = htole32(adsEntry.currentStreamLength);
    adsEntry.currentStreamOffset = htole64(adsEntry.currentStreamOffset);
}

CTRL_FILE_RETCODE AdsParser::FlushToFile()
{
    CTRL_FILE_RETCODE ret = WriteToFile(m_writeBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_writeFd.flush();
    HCP_Log(DEBUG, MODULE) << "Metafile flush data completed filename: " << m_fileName
        << " offset: " << m_writeFd.tellp() << " offset variable: " << m_offset << HCPENDLOG;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE AdsParser::ValidateMetaFile(uint64_t offset)
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

string AdsParser::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}

CTRL_FILE_RETCODE AdsParser::FreeReadCache()
{
    if (m_readCache == nullptr) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    free(m_readCache);
    m_readCache = nullptr;
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint64_t AdsParser::GetCurrentOffset()
{
    return m_offset;
}