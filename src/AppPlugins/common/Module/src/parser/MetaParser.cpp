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
#include "MetaParser.h"
#include "define/Defines.h"
#include "define/Types.h"
#ifndef WIN32
#include "define/GenericEndian.h"
#endif
#include "securec.h"
#include "Log.h"
#include "common/Thread.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "META_PARSER";
constexpr uint32_t SCANNER_PATH_LEN_MAX = 4096;
constexpr uint32_t ACL_MAX_LEN = 65535;
constexpr uint32_t SCANNER_FILE_NAME_LEN_MAX = 256;
constexpr uint32_t FOUR_MB = (4 * 1024 * 1024);
}

MetaParser::MetaParser(MetaParser::Params params) : FileParser(true)
{
    m_fileName = params.m_fileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_header.title = META_HEADER_TITLE_FS;
    m_header.version = META_HEADER_VERSION_V20;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
}

MetaParser::MetaParser(string metaFileName) : FileParser(true)
{
    m_fileName = metaFileName;
    m_isCacheEnable = false;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_readBufferSize = sizeof(FileMeta);
}

MetaParser::MetaParser(string metaFileName, bool isCacheEnable) : FileParser(true)
{
    m_fileName = metaFileName;
    m_isCacheEnable = isCacheEnable;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
    m_readBufferSize = sizeof(FileMeta);
    if (m_isCacheEnable) {
        AllocReadCache();
    }
}

MetaParser::~MetaParser()
{
    if (m_writeFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::WRITE);

    if (m_readFd.is_open()) {
        Close(CTRL_FILE_OPEN_MODE::READ);
    }
    FreeReadCache();
}

CTRL_FILE_RETCODE MetaParser::OpenWrite()
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

CTRL_FILE_RETCODE MetaParser::OpenForWrite()
{
    CTRL_FILE_RETCODE ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::binary | std::ios::in);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

void MetaParser::CloseForWrite()
{
    m_writeFd.close();
    return;
}

CTRL_FILE_RETCODE MetaParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadHeader()
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
        Module::SleepFor(chrono::milliseconds(sleepDuration));
    } while (retryCnt < CTRL_FILE_SERVER_RETRY_CNT);
    return ret;
}

CTRL_FILE_RETCODE MetaParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_THREE) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    m_offset = (uint64_t) headerBuffer.tellp();
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header For MetaParser Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeFd.flush();
    HCP_Log(INFO, MODULE) << "Write Header completed for metafile: " << m_fileName << HCPENDLOG;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), META_HEADER_TITLE_FS.c_str()) == 0) &&
        (strcmp(m_header.version.c_str(), META_HEADER_VERSION_V20.c_str()) == 0) &&
        !m_header.timestamp.empty()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    if ((strcmp(m_header.title.c_str(), META_HEADER_TITLE_NAS.c_str()) == 0) &&
        (strcmp(m_header.version.c_str(), META_HEADER_VERSION_V10.c_str()) == 0) &&
        !m_header.timestamp.empty()) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    return CTRL_FILE_RETCODE::FAILED;
}

CTRL_FILE_RETCODE MetaParser::GetHeader(MetaParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadMetaFileHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};
    if (!m_readFd) {
        HCP_Log(ERR, MODULE) << "ReadHeader readFd not proper: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    while (headerLine < CTRL_FILE_NUMBER_THREE) {
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

CTRL_FILE_RETCODE MetaParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == META_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string MetaParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case META_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case META_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case META_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE MetaParser::ValidateMetaFile(uint64_t offset)
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

CTRL_FILE_RETCODE MetaParser::ValidateMetaFile(uint64_t offset, uint16_t readLen)
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

    m_readFd.seekg(offset + readLen);
    if (m_readFd.fail()) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(WARN, MODULE) << "failed to seek in metafile"
            " to offset " << (offset + readLen) << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadDirMetaFromReadCache(DirMeta &dirMeta, uint64_t offset)
{
    char *bufferCpy = m_readCache + (offset - m_readCacheOffsetStart);
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    DirMeta dirMetaInLe {};
    if (memcpy_s(&dirMetaInLe, sizeof(DirMeta), bufferCpy, sizeof(DirMeta)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(dirMetaInLe, dirMeta);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadAcl(uint64_t offset, DirMetaV10 &dirMeta,
    uint16_t readLen, int32_t aclOffset)
{
    char aclText[ACL_MAX_LEN];
    char *bufferCpy = m_readBinaryBuffer;
    bufferCpy += aclOffset;
    int ret = memset_s(aclText, ACL_MAX_LEN, 0, ACL_MAX_LEN);
    if (ret != 0) {
        ERRLOG("failed to memset aclText %p, ret %d", aclText, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (dirMeta.m_aclSize > 0) {
        if (dirMeta.m_aclSize > ACL_MAX_LEN) {
            HCP_Log(ERR, MODULE) << "AclSize out of bounds offset: "
                << offset << " readLen: " << readLen << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (memcpy_s(&aclText, dirMeta.m_aclSize, bufferCpy, dirMeta.m_aclSize) != 0) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        string strAcl(aclText);
        dirMeta.m_aclText = strAcl;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadAcl(uint64_t offset, FileMetaV10 &fMeta,
    uint16_t readLen, int32_t aclOffset)
{
    char aclText[ACL_MAX_LEN];
    char *bufferCpy = m_readBinaryBuffer;
    bufferCpy += aclOffset;
    int ret = memset_s(aclText, ACL_MAX_LEN, 0, ACL_MAX_LEN);
    if (ret != 0) {
        ERRLOG("failed to memset aclText %p, ret %d", aclText, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (fMeta.m_aclSize > 0) {
        if (fMeta.m_aclSize > ACL_MAX_LEN) {
            HCP_Log(ERR, MODULE) << "AclSize out of bounds offset: "
                << offset << " readLen: " << readLen << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (memcpy_s(&aclText, fMeta.m_aclSize, bufferCpy, fMeta.m_aclSize) != 0) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        string strAcl(aclText);
        fMeta.m_aclText = strAcl;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::CheckDirMetaV10Validity(DirMetaV10 &dirMeta)
{
    if (dirMeta.m_mtime == 0 && dirMeta.m_path != ".") {
        return CTRL_FILE_RETCODE::FAILED;
    } else {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
}

CTRL_FILE_RETCODE MetaParser::CheckFileMetaV10Validity(FileMetaV10 &fMeta)
{
    if (fMeta.m_ctime == 0) {
        return CTRL_FILE_RETCODE::FAILED;
    } else {
        return CTRL_FILE_RETCODE::SUCCESS;
    }
}

CTRL_FILE_RETCODE MetaParser::ReadDirMetaV10FromBuffer(DirMetaV10 &dirMeta,
    uint64_t offset, uint16_t readLen)
{
    DirectoryMetaReadWrite dirMetaRw {};
    char *bufferCpy = nullptr;
    char name[SCANNER_PATH_LEN_MAX];
    int32_t aclOffset = 0;
    bufferCpy = m_readBinaryBuffer;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    if (memcpy_s(&dirMetaRw, sizeof(DirectoryMetaReadWrite), bufferCpy, sizeof(DirectoryMetaReadWrite)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    bufferCpy += sizeof(DirectoryMetaReadWrite);

    uint16_t pathLen = le16toh(dirMetaRw.m_pathLen);
    if ((le16toh(pathLen) <= 0) || (le16toh(pathLen) > SCANNER_PATH_LEN_MAX)) {
        HCP_Log(ERR, MODULE) << "Invalid string length: " << pathLen
            << " offset: " << offset << " readLen: " << readLen <<
            " pathlen: " << pathLen << " inode: " <<
            dirMetaRw.m_inode << " buffer: " << m_readBinaryBuffer << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (memcpy_s(&name, SCANNER_PATH_LEN_MAX, bufferCpy, pathLen) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    bufferCpy += pathLen;
    aclOffset = static_cast<int32_t>(sizeof(DirectoryMetaReadWrite)) + static_cast<int32_t>(pathLen);

    TranslateDirMetaV10(dirMeta, dirMetaRw);
    dirMeta.m_path.assign(reinterpret_cast<char *>(&name), pathLen);

    ret = ReadAcl(offset, dirMeta, readLen, aclOffset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return ret;
    }
    ret = CheckDirMetaV10Validity(dirMeta);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        HCP_Log(ERR, MODULE) << "DirMeta received from file is invalid offset: "
            << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::AllocReadCache()
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
        ERRLOG("failed to memset m_readCache %p, ret %d", m_readCache, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readCacheOffsetStart = 0;
    m_readCacheOffsetEnd = 0;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::FreeReadCache()
{
    if (m_readCache == nullptr) {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    free(m_readCache);
    m_readCache = nullptr;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::RefreshReadCache(uint64_t offset, uint64_t len)
{
    if (offset >= m_readCacheOffsetStart && (offset + len) <= m_readCacheOffsetEnd) {
        m_cacheHitCnt++;
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    m_cacheMissCnt++;
    CTRL_FILE_RETCODE ret = FillReadCache(offset, FOUR_MB);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to refresh cache, error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (offset >= m_readCacheOffsetStart && (offset + len) <= m_readCacheOffsetEnd) {
        return CTRL_FILE_RETCODE::SUCCESS;
    } else {
        return CTRL_FILE_RETCODE::FAILED;
    }
}

CTRL_FILE_RETCODE MetaParser::FillReadCache(uint64_t offset, uint32_t readLen)
{
    int ret = memset_s(m_readCache, readLen, 0, readLen);
    if (ret != 0) {
        ERRLOG("memset m_readCache failed %p, size %u, ret %d", m_readCache, readLen, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    if (m_readFd.fail()) {
        m_readCacheOffsetStart = 0;
        m_readCacheOffsetEnd = 0;
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.read(m_readCache, readLen);
    m_readFd.sync();
    if ((!m_readFd.eof()) && (!m_readFd.good())) {
        m_readFd.close();
        CTRL_FILE_RETCODE ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
        if (ret != CTRL_FILE_RETCODE::SUCCESS) {
            m_readCacheOffsetStart = 0;
            m_readCacheOffsetEnd = 0;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_readFd.seekg(offset);
        if (m_readFd.fail()) {
            m_readCacheOffsetStart = 0;
            m_readCacheOffsetEnd = 0;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_readFd.read(m_readCache, readLen);
        m_readFd.sync();
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            m_readFd.close();
            m_readCacheOffsetStart = 0;
            m_readCacheOffsetEnd = 0;
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

CTRL_FILE_RETCODE MetaParser::ReadDirectoryMetaWithoutCache(DirMeta &dirMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
 
    ret = ValidateMetaFile(offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate directory meta failed filename: "
            << m_fileName << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    ret = ReadFromBinaryFile(offset, sizeof(DirMeta));
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(DirMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadDirMetaFromBuffer(dirMeta, offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read DirMeta from buffer of length " << sizeof(DirMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ret;
}

CTRL_FILE_RETCODE MetaParser::ReadDirMetaFromBuffer(DirMeta &dirMeta, uint64_t offset)
{
    char *bufferCpy = nullptr;
    bufferCpy = m_readBinaryBuffer;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    DirMeta dirMetaInLe {};
    if (memcpy_s(&dirMetaInLe, sizeof(DirMeta), bufferCpy, sizeof(DirMeta)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(dirMetaInLe, dirMeta);
    bufferCpy += sizeof(DirMeta);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadDirectoryMeta(DirMeta &dirMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "metaFile given is not open" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (!m_isCacheEnable) {
        return ReadDirectoryMetaWithoutCache(dirMeta, offset);
    }

    ret = RefreshReadCache(offset, sizeof(DirMeta));
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(DirMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadDirMetaFromReadCache(dirMeta, offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read DirMeta from buffer of length " << sizeof(DirMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ret;
}

CTRL_FILE_RETCODE MetaParser::ReadDirectoryMetaV10(DirMetaV10 &dirMeta, uint16_t readLen, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    ret = ValidateMetaFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate directory meta failed filename: "
            << m_fileName << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
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
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    ret = ReadFromBinaryFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadDirMetaV10FromBuffer(dirMeta, offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read DirMeta from buffer of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ret;
}

uint16_t MetaParser::WriteDirectoryMeta(DirMeta &dirMeta)
{
    lock_guard<std::mutex> lk(m_lock);
    DirMeta dirMetaInLe {};
    TranslateToLittleEndian(dirMeta, dirMetaInLe);
    m_writeBuffer.write(reinterpret_cast<char *>(&dirMetaInLe), sizeof(dirMetaInLe));
    uint16_t metaLength = sizeof(dirMetaInLe);
    m_offset += metaLength;
    return metaLength;
}

void MetaParser::PrintFileMeta(const FileMeta& fileMeta)
{
    DBGLOG("Write to metafile - %s", m_fileName.c_str());
    DBGLOG("Write filemeta - type: %u, m_attr %u, m_mode %u, m_nlink %u, m_uid %u, m_gid %u, m_inode %u, m_size %u,"
        "m_err %u, m_rdev %u, m_mtime %u, m_ctime %u, m_atime %u"
        "m_btime %u, m_blksize %u, m_blocks %u, m_xMetaFileIndex %u, m_xMetaFileOffset %u",
        fileMeta.type, fileMeta.m_attr, fileMeta.m_mode, fileMeta.m_nlink, fileMeta.m_uid,
        fileMeta.m_gid, fileMeta.m_inode, fileMeta.m_size, fileMeta.m_err, fileMeta.m_rdev,
        fileMeta.m_mtime, fileMeta.m_ctime, fileMeta.m_atime, fileMeta.m_btime, fileMeta.m_blksize,
        fileMeta.m_blocks, fileMeta.m_xMetaFileIndex, fileMeta.m_xMetaFileOffset);
}

void MetaParser::PrintDirectoryMeta(const DirMeta& dirMeta)
{
    DBGLOG("Write to metafile - %s", m_fileName.c_str());
    DBGLOG("Write dirMeta - type: %u, m_attr %u, m_mode %u, m_uid %u, m_gid %u, m_inode %u, m_size %u, m_mtime %u,"
        "m_ctime %u, m_atime %u, m_btime %u, m_xMetaFileIndex %u, m_xMetaFileOffset %u",
        dirMeta.type, dirMeta.m_attr, dirMeta.m_mode, dirMeta.m_uid, dirMeta.m_gid, dirMeta.m_inode,
        dirMeta.m_size, dirMeta.m_mtime, dirMeta.m_atime, dirMeta.m_ctime, dirMeta.m_btime,
        dirMeta.m_xMetaFileIndex, dirMeta.m_xMetaFileOffset);
}

void MetaParser::TranslateToLittleEndian(DirMeta &dirMeta, DirMeta &dirMetaInLe)
{
    dirMetaInLe.type              = htole16(dirMeta.type);
    dirMetaInLe.m_attr            = htole32(dirMeta.m_attr);
    dirMetaInLe.m_mode            = htole32(dirMeta.m_mode);
    dirMetaInLe.m_uid             = htole32(dirMeta.m_uid);
    dirMetaInLe.m_gid             = htole32(dirMeta.m_gid);
    dirMetaInLe.m_inode           = htole64(dirMeta.m_inode);
    dirMetaInLe.m_size            = htole64(dirMeta.m_size);
    dirMetaInLe.m_mtime           = htole64(dirMeta.m_mtime);
    dirMetaInLe.m_atime           = htole64(dirMeta.m_atime);
    dirMetaInLe.m_ctime           = htole64(dirMeta.m_ctime);
    dirMetaInLe.m_btime           = htole64(dirMeta.m_btime);
    dirMetaInLe.m_hardLinkFilesCnt  = htole64(dirMeta.m_hardLinkFilesCnt);
    dirMetaInLe.m_subDirsCnt      = htole64(dirMeta.m_subDirsCnt);
    dirMetaInLe.m_xMetaFileIndex  = htole64(dirMeta.m_xMetaFileIndex);
    dirMetaInLe.m_xMetaFileOffset = htole64(dirMeta.m_xMetaFileOffset);
}

void MetaParser::TranslateToHostEndian(DirMeta &dirMeta, DirMeta &dirMetaInHost)
{
    dirMetaInHost.type              = le16toh(dirMeta.type);
    dirMetaInHost.m_attr            = le32toh(dirMeta.m_attr);
    dirMetaInHost.m_mode            = le32toh(dirMeta.m_mode);
    dirMetaInHost.m_uid             = le32toh(dirMeta.m_uid);
    dirMetaInHost.m_gid             = le32toh(dirMeta.m_gid);
    dirMetaInHost.m_inode           = le64toh(dirMeta.m_inode);
    dirMetaInHost.m_size            = le64toh(dirMeta.m_size);
    dirMetaInHost.m_mtime           = le64toh(dirMeta.m_mtime);
    dirMetaInHost.m_atime           = le64toh(dirMeta.m_atime);
    dirMetaInHost.m_ctime           = le64toh(dirMeta.m_ctime);
    dirMetaInHost.m_btime           = le64toh(dirMeta.m_btime);
    dirMetaInHost.m_hardLinkFilesCnt  = le64toh(dirMeta.m_hardLinkFilesCnt);
    dirMetaInHost.m_subDirsCnt      = le64toh(dirMeta.m_subDirsCnt);
    dirMetaInHost.m_xMetaFileIndex  = le64toh(dirMeta.m_xMetaFileIndex);
    dirMetaInHost.m_xMetaFileOffset = le64toh(dirMeta.m_xMetaFileOffset);
}

void MetaParser::TranslateDirMetaV10(DirMetaV10 &dirMeta, DirectoryMetaReadWrite &dirMetaRw)
{
    dirMeta.m_mode    = le32toh(dirMetaRw.m_mode);
    dirMeta.m_attr    = le32toh(dirMetaRw.m_attr);
    dirMeta.m_uid     = le32toh(dirMetaRw.m_uid);
    dirMeta.m_gid     = le32toh(dirMetaRw.m_gid);
    dirMeta.m_size    = le64toh(dirMetaRw.m_size);
    dirMeta.m_inode   = le64toh(dirMetaRw.m_inode);
    dirMeta.m_atime   = le64toh(dirMetaRw.m_atime);
    dirMeta.m_mtime   = le64toh(dirMetaRw.m_mtime);
    dirMeta.m_ctime   = le64toh(dirMetaRw.m_ctime);
    dirMeta.m_btime   = le64toh(dirMetaRw.m_btime);
    dirMeta.m_aclSize = le16toh(dirMetaRw.m_aclSize);
}

CTRL_FILE_RETCODE MetaParser::TranslateFileMetaV10(FileMetaV10 &fMeta, FileMetaReadWrite &fMetaRw)
{
    fMeta.m_mode    = le32toh(fMetaRw.m_mode);
    fMeta.m_attr    = le32toh(fMetaRw.m_attr);
    fMeta.m_nlink   = le32toh(fMetaRw.m_nlink);
    fMeta.m_uid     = le32toh(fMetaRw.m_uid);
    fMeta.m_gid     = le32toh(fMetaRw.m_gid);
    fMeta.m_size    = le64toh(fMetaRw.m_size);
    fMeta.m_inode   = le64toh(fMetaRw.m_inode);
    fMeta.m_rdev    = le64toh(fMetaRw.m_rdev);
    fMeta.m_mtime   = le64toh(fMetaRw.m_mtime);
    fMeta.m_ctime   = le64toh(fMetaRw.m_ctime);
    fMeta.m_atime   = le64toh(fMetaRw.m_atime);
    fMeta.m_btime   = le64toh(fMetaRw.m_btime);
    fMeta.m_aclSize = le16toh(fMetaRw.m_aclSize);
    fMeta.m_fh.len  = le32toh(fMetaRw.m_fh.len);
    if (fMeta.m_fh.len > 0) {
        if (memcpy_s(fMeta.m_fh.value, fMeta.m_fh.len, fMetaRw.m_fh.value, fMeta.m_fh.len) != 0) {
            HCP_Log(ERR, MODULE) << "Memcpy failed" << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint16_t MetaParser::WriteFileMeta(FileMeta &fMeta)
{
    lock_guard<std::mutex> lk(m_lock);
    FileMeta fMetaInLe {};
    TranslateToLittleEndian(fMeta, fMetaInLe);
    m_writeBuffer.write(reinterpret_cast<char *>(&fMetaInLe), sizeof(fMetaInLe));
    uint16_t metaLength = sizeof(fMetaInLe);
    m_offset += metaLength;
    return metaLength;
}

void MetaParser::TranslateToLittleEndian(FileMeta &fMeta, FileMeta &fMetaInLe)
{
    fMetaInLe.type              = htole16(fMeta.type);
    fMetaInLe.m_attr            = htole32(fMeta.m_attr);
    fMetaInLe.m_mode            = htole32(fMeta.m_mode);
    fMetaInLe.m_nlink           = htole32(fMeta.m_nlink);
    fMetaInLe.m_uid             = htole32(fMeta.m_uid);
    fMetaInLe.m_gid             = htole32(fMeta.m_gid);
    fMetaInLe.m_inode           = htole64(fMeta.m_inode);
    fMetaInLe.m_size            = htole64(fMeta.m_size);
    fMetaInLe.m_err             = htole64(fMeta.m_err);
    fMetaInLe.m_rdev            = htole64(fMeta.m_rdev);
    fMetaInLe.m_mtime           = htole64(fMeta.m_mtime);
    fMetaInLe.m_ctime           = htole64(fMeta.m_ctime);
    fMetaInLe.m_atime           = htole64(fMeta.m_atime);
    fMetaInLe.m_btime           = htole64(fMeta.m_btime);
    fMetaInLe.m_blksize         = htole64(fMeta.m_blksize);
    fMetaInLe.m_blocks          = htole64(fMeta.m_blocks);
    fMetaInLe.m_xMetaFileIndex  = htole64(fMeta.m_xMetaFileIndex);
    fMetaInLe.m_xMetaFileOffset = htole64(fMeta.m_xMetaFileOffset);
}

void MetaParser::TranslateToHostEndian(FileMeta &fMeta, FileMeta &fMetaInHost)
{
    fMetaInHost.type              = le16toh(fMeta.type);
    fMetaInHost.m_attr            = le32toh(fMeta.m_attr);
    fMetaInHost.m_mode            = le32toh(fMeta.m_mode);
    fMetaInHost.m_nlink           = le32toh(fMeta.m_nlink);
    fMetaInHost.m_uid             = le32toh(fMeta.m_uid);
    fMetaInHost.m_gid             = le32toh(fMeta.m_gid);
    fMetaInHost.m_inode           = le64toh(fMeta.m_inode);
    fMetaInHost.m_size            = le64toh(fMeta.m_size);
    fMetaInHost.m_err             = le64toh(fMeta.m_err);
    fMetaInHost.m_rdev            = le64toh(fMeta.m_rdev);
    fMetaInHost.m_mtime           = le64toh(fMeta.m_mtime);
    fMetaInHost.m_ctime           = le64toh(fMeta.m_ctime);
    fMetaInHost.m_atime           = le64toh(fMeta.m_atime);
    fMetaInHost.m_btime           = le64toh(fMeta.m_btime);
    fMetaInHost.m_blksize         = le64toh(fMeta.m_blksize);
    fMetaInHost.m_blocks          = le64toh(fMeta.m_blocks);
    fMetaInHost.m_xMetaFileIndex  = le64toh(fMeta.m_xMetaFileIndex);
    fMetaInHost.m_xMetaFileOffset = le64toh(fMeta.m_xMetaFileOffset);
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaFromReadCache(FileMeta &fMeta, uint64_t offset)
{
    char *bufferCpy = m_readCache + (offset - m_readCacheOffsetStart);
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    FileMeta fMetaInLe {};
    if (memcpy_s(&fMetaInLe, sizeof(FileMeta), bufferCpy, sizeof(FileMeta)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(fMetaInLe, fMeta);
    bufferCpy += sizeof(FileMeta);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaWithoutCache(FileMeta &fMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
 
    ret = ValidateMetaFile(offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate file meta failed filename: "<< m_fileName
            << " offset: " << offset << HCPENDLOG;
        return ret;
    }
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadFromBinaryFile(offset, sizeof(FileMeta));
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(FileMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << "offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadFileMetaFromBuffer(fMeta, offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read File MetaParser from buffer of length "
            << sizeof(FileMeta) << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaFromBuffer(FileMeta &fMeta, uint64_t offset)
{
    char *bufferCpy = m_readBinaryBuffer;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    FileMeta fMetaInLe {};
    if (memcpy_s(&fMetaInLe, sizeof(FileMeta), bufferCpy, sizeof(FileMeta)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(fMetaInLe, fMeta);
    bufferCpy += sizeof(FileMeta);
    return CTRL_FILE_RETCODE::SUCCESS;
}
 

CTRL_FILE_RETCODE MetaParser::ReadFileMeta(FileMeta &fMeta, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "metaFile given is not open" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (!m_isCacheEnable) {
        return ReadFileMetaWithoutCache(fMeta, offset);
    }

    ret = RefreshReadCache(offset, sizeof(FileMeta));
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << sizeof(FileMeta) << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << "offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadFileMetaFromReadCache(fMeta, offset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read File MetaParser from buffer of length "
            << sizeof(FileMeta) << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::UpdateFileMeta(const FileMeta &fMeta, uint64_t offset)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_writeFd.is_open()) {
        ERRLOG("metaFile given is not open.");
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeFd.seekp(offset, std::ios::beg);
    m_writeFd.write(reinterpret_cast<const char*>(&fMeta), sizeof(FileMeta));
    m_writeFd.flush();
    if (m_writeFd.fail()) {
        ERRLOG("failed to write data.");
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaV10(FileMetaV10 &fMeta, uint16_t readLen, uint64_t offset)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    ret = ValidateMetaFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Validate file meta failed filename: "
            << m_fileName << " offset: " << offset << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
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
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_readFd.seekg(offset);
    ret = ReadFromBinaryFile(offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    ret = ReadFileMetaV10FromBuffer(fMeta, offset, readLen);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read File Meta from buffer of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ret;
}

uint64_t MetaParser::GetCurrentOffset()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_offset;
}

CTRL_FILE_RETCODE MetaParser::FlushToFile()
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

string MetaParser::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}

string MetaParser::GetFileVersion()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_header.version;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaV10FromBuffer(FileMetaV10 &fMeta, uint64_t offset, uint16_t readLen)
{
    FileMetaReadWrite fMetaRw {};
    char *bufferCpy = m_readBinaryBuffer;
    int32_t aclOffset = 0;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    char name[SCANNER_FILE_NAME_LEN_MAX];
    if (memcpy_s(&fMetaRw, sizeof(FileMetaReadWrite), bufferCpy, sizeof(FileMetaReadWrite)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    bufferCpy += sizeof(FileMetaReadWrite);

    uint16_t nameLen = le16toh(fMetaRw.m_nameLen);
    if ((nameLen <= 0) || (nameLen > SCANNER_FILE_NAME_LEN_MAX)) {
        HCP_Log(ERR, MODULE) << "Invalid string length: " << nameLen
            << " offset: " << offset << " readLen: " << readLen <<
            " inode: " << fMetaRw.m_inode << " metafname: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (memcpy_s(&name, SCANNER_FILE_NAME_LEN_MAX, bufferCpy, nameLen) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    bufferCpy += nameLen;
    ret = TranslateFileMetaV10(fMeta, fMetaRw);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Failed to translate fmeta" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    fMeta.m_name.assign(reinterpret_cast<char *>(&name), nameLen);
    aclOffset = (int32_t) sizeof(FileMetaReadWrite) + (int32_t) nameLen;
    ret = ReadAcl(0, fMeta, fMetaRw.m_aclSize, aclOffset);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        return ret;
    }
    ret = CheckFileMetaV10Validity(fMeta);
    if (ret == CTRL_FILE_RETCODE::FAILED) {
        HCP_Log(ERR, MODULE) << "FileMeta received from file is invalid offset: "
            << offset << " readLen: " << readLen << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaFromBuffer(FileMeta &fMeta)
{
    char *bufferCpy = m_readBinaryBuffer;
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    FileMeta fMetaInLe {};
    if (memcpy_s(&fMetaInLe, sizeof(FileMeta), bufferCpy, sizeof(FileMeta)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    TranslateToHostEndian(fMetaInLe, fMeta);
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadLenFromFile(uint32_t len)
{
    int result = memset_s(m_readBinaryBuffer, m_readBufferSize, 0, m_readBufferSize);
    if (result != 0) {
        ERRLOG("failed to memset m_readBinaryBuffer %p, size %u, ret %d", m_readBinaryBuffer, m_readBufferSize, result);
        return CTRL_FILE_RETCODE::FAILED;
    }
    CTRL_FILE_RETCODE ret = ReadFromBinaryFile(m_readFd.tellg(), len);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, MODULE) << "failed to read metafile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMeta(FileMeta &fMeta)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    lock_guard<std::mutex> lk(m_lock);
    if (ReadLenFromFile(sizeof(FileMeta)) != CTRL_FILE_RETCODE::SUCCESS) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (m_readFd.eof()) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    ret = ReadFileMetaFromBuffer(fMeta);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(DEBUG, MODULE) << "failed to read File MetaParser from buffer error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " metafile: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return ret;
}

CTRL_FILE_RETCODE MetaParser::ReadFileMetaEntries(std::queue<FileMeta> &fileMetaQueue, int maxCount)
{
    FileMeta fmeta {};
    int count = 0;
    while (count < maxCount) {
        if (ReadFileMeta(fmeta) == CTRL_FILE_RETCODE::FAILED) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        fileMetaQueue.push(fmeta);
        count++;
    }
    return CTRL_FILE_RETCODE::FAILED;
}

uint64_t MetaParser::GetCacheHitCount()
{
    return m_cacheHitCnt;
}

uint64_t MetaParser::GetCacheMissCount()
{
    return m_cacheMissCnt;
}
