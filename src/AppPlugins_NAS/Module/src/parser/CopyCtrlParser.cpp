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
#ifdef _AIX
#define __STDC_FORMAT_MACROS
#endif
#include "CopyCtrlParser.h"
#include "securec.h"
#include "Log.h"
#include <cinttypes>

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "COPY_CTRL_PARSER";
const int NUMBER1 = 1;
const int NUMBER2 = 2;
const int NUMBER4 = 4;
}

CopyCtrlParser::CopyCtrlParser(CopyCtrlParser::Params params) : FileParser(false)
{
    m_fileName = params.m_ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_minEntriesPerFile = params.minEntriesPerFile;
    m_maxDataSize = params.maxDataSize;
    m_minDataSize = params.minDataSize;
    m_ctrlFileTimeElapsed = params.m_ctrlFileTimeElapsed;
    m_maxFileSize = params.maxCtrlFileSize;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = CTRL_HEADER_TITLE;
    m_header.version = CTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

CopyCtrlParser::CopyCtrlParser(std::string ctlFileName) : FileParser(false)
{
    m_fileName = ctlFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
}

CopyCtrlParser::~CopyCtrlParser()
{
    if (m_writeFd.is_open()) {
        HCP_Log(DEBUG, MODULE) << "Close called from  Destructor" << HCPENDLOG;
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    }
    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE CopyCtrlParser::OpenWrite()
{
    m_ctrlFileCreationTime = ParserUtils::GetCurrentTimeInSeconds();
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::CloseWrite()
{
    m_writeBuffer << m_writeFileBuffer.str();
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::GetHeader(CopyCtrlParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::ReadEntry(CopyCtrlFileEntry &fileEntry,
    CopyCtrlDirEntry &dirEntry)
{
    string ctlFileLine {};
    vector<string> ctlFileLineSplit {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    do {
        getline(m_readBuffer, ctlFileLine);
        if (ctlFileLine.empty()) {
            return CTRL_FILE_RETCODE::READ_EOF;
        }

        boost::algorithm::split(ctlFileLineSplit, ctlFileLine, boost::is_any_of(","), boost::token_compress_off);
        if (ValidateEntry(ctlFileLineSplit, ctlFileLine) == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        } else {
            // 这里处理一种情况: 文件名中有字符导致换行， 与下一行一起读了返回
            HandleBreakLine(ctlFileLine);
            return ReadEntryWithLine(fileEntry, dirEntry, ctlFileLine);
        }
        ctlFileLine.clear();
        ctlFileLineSplit.clear();
    } while (true);

    if (ctlFileLineSplit[CTRL_FILE_NUMBER_ZERO] == CTRL_ENTRY_TYPE_DIR) {
        TranslateDirEntry(ctlFileLineSplit, dirEntry);
    } else {
        TranslateFileEntry(ctlFileLineSplit, fileEntry);
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

void CopyCtrlParser::HandleBreakLine(std::string& ctlFileLine)
{
    std::string tmpLine {};
    std::streampos currentPositon = m_readBuffer.tellg();
    while (true) {
        tmpLine.clear();
        currentPositon = m_readBuffer.tellg();
        getline(m_readBuffer, tmpLine);
        if (tmpLine.empty()) {
            return;
        }
        if (IsNormalEntry(tmpLine)) {
            // 读到正常的行， 把pos调整回原来的位置
            m_readBuffer.seekg(currentPositon);
            return;
        } else {
            // 说明是截断的行
            ctlFileLine += tmpLine;
        }
    }
    return;
}

bool CopyCtrlParser::IsNormalEntry(const std::string& fileLine)
{
    if (fileLine.size() < NUMBER4) {
        return false;
    }
    if (fileLine[0] != 'd' && fileLine[0] != 'f') {
        return false;
    }
    if (fileLine[NUMBER1] != ',') {
        return false;
    }
    std::string modeStr = fileLine.substr(NUMBER2, NUMBER2);
    if (modeStr != CTRL_ENTRY_MODE_DATA_MODIFIED &&
        modeStr != CTRL_ENTRY_MODE_META_MODIFIED &&
        modeStr != CTRL_ENTRY_MODE_BOTH_MODIFIED &&
        modeStr != CTRL_ENTRY_MODE_DATA_DELETED &&
        modeStr != CTRL_ENTRY_MODE_ONLY_FILE_MODIFIED &&
        modeStr != CTRL_ENTRY_MODE_NEW_FILE) {
        return false;
    }
    return true;
}

CTRL_FILE_RETCODE CopyCtrlParser::ReadEntryWithLine(CopyCtrlFileEntry &fileEntry, CopyCtrlDirEntry &dirEntry,
    const std::string& ctlFileLine)
{
    vector<string> ctlFileLineSplit {};
    if (ctlFileLine.empty()) {
        ERRLOG("entry empty");
        return CTRL_FILE_RETCODE::FAILED;
    }
    boost::algorithm::split(ctlFileLineSplit, ctlFileLine, boost::is_any_of(","), boost::token_compress_off);
    if (ValidateEntry(ctlFileLineSplit, ctlFileLine) != CTRL_FILE_RETCODE::SUCCESS) {
        ERRLOG("invalid fileEntry: %s", ctlFileLine.c_str());
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (ctlFileLineSplit[CTRL_FILE_NUMBER_ZERO] == CTRL_ENTRY_TYPE_DIR) {
        TranslateDirEntry(ctlFileLineSplit, dirEntry);
    } else {
        TranslateFileEntry(ctlFileLineSplit, fileEntry);
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

void CopyCtrlParser::PrintCopyCtrlDirEntry(CopyCtrlDirEntry& dirEntry)
{
    DBGLOG("Write to ctrl file - %s", m_fileName.c_str());
    DBGLOG("Write dirEntry - m_mode: %s, m_dirName: %s, m_metaFileName: %s, metaFileReadLen: %u, m_aclFlag: %u,"
        "metaFileOffset: %u, m_fileCount: %u, m_metaFileIndex: %u",
        dirEntry.m_mode.c_str(), dirEntry.m_dirName.c_str(), dirEntry.m_metaFileName.c_str(), dirEntry.metaFileReadLen,
        dirEntry.m_aclFlag, dirEntry.metaFileOffset, dirEntry.m_fileCount, dirEntry.m_metaFileIndex);
}

void CopyCtrlParser::PrintCopyCtrlFileEntry(CopyCtrlFileEntry& fileEntry)
{
    DBGLOG("Write to ctrl file - %s", m_fileName.c_str());
    DBGLOG("Write fileEntry - m_mode: %s, m_fileName: %s, m_metaFileName: %s, metaFileReadLen: %u, m_aclFlag: %u, metaFileOffset: %u, m_fileSize: %u, m_metaFileIndex: %u",
        fileEntry.m_mode.c_str(), fileEntry.m_fileName.c_str(), fileEntry.m_metaFileName.c_str(), fileEntry.metaFileReadLen, fileEntry.m_aclFlag, fileEntry.metaFileOffset, fileEntry.m_fileSize, fileEntry.m_metaFileIndex);
}

CTRL_FILE_RETCODE CopyCtrlParser::WriteDirEntry(CopyCtrlDirEntry &dirEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    dirEntry.m_fileCount = m_fileCount;
    uint32_t commaCount = ParserUtils::GetCommaCountOfString(dirEntry.m_dirName.c_str());

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as control file is not open: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int ret = memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE);
    if (ret != 0) {
        ERRLOG("failed to memset m_writeCtrlLine %p, ret %d", m_writeCtrlLine, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = 0;
    if (dirEntry.m_mode != CTRL_ENTRY_MODE_DATA_DELETED) {
        len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                        "d,%s,%" PRIu32 ",%s,%" PRIu16 ",%" PRIu64 ",%" PRIu16 ",%" PRIu64 ",%" PRIu32 "\n",
                        dirEntry.m_mode.c_str(),
                        commaCount,
                        dirEntry.m_dirName.c_str(),
                        dirEntry.m_metaFileIndex,
                        dirEntry.metaFileOffset,
                        dirEntry.metaFileReadLen,
                        dirEntry.m_fileCount,
                        dirEntry.m_aclFlag);
    } else {
        len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                        "d,%s,%" PRIu32 ",%s,%" PRIu64 "\n",
                        dirEntry.m_mode.c_str(),
                        commaCount,
                        dirEntry.m_dirName.c_str(),
                        dirEntry.m_fileCount);
    }
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "failed to prepare buffer: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;
    m_entries++;
    m_header.stats.noOfDirs++;
    m_writeBuffer << m_writeFileBuffer.str(); // Write and Append the m_writeFileBuffer  to the m_writeBuffer
    m_writeFileBuffer.str("");
    m_writeFileBuffer.clear();  // Once m_writeFileBuffer is written to m_writeBuffer, clear it .
    m_fileCount = 0;
    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile  || m_dataSize >= m_maxDataSize) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::WriteFileEntry(CopyCtrlFileEntry &fileEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = ParserUtils::GetCommaCountOfString(fileEntry.m_fileName.c_str());
    int ret = memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE);
    if (ret != 0) {
        ERRLOG("failed to memset m_writeCtrlLine %p, ret %d", m_writeCtrlLine, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as control file is not open: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (fileEntry.m_mode != CTRL_ENTRY_MODE_DATA_DELETED) {
        int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                            "f,%s,%" PRIu32 ",%s,%" PRIu16 ",%" PRIu64 ",%" PRIu16 ",%" PRIu32 "\n",
                            fileEntry.m_mode.c_str(),
                            commaCount,
                            fileEntry.m_fileName.c_str(),
                            fileEntry.m_metaFileIndex,
                            fileEntry.metaFileOffset,
                            fileEntry.metaFileReadLen,
                            fileEntry.m_aclFlag);
        if (len <= 0) {
            HCP_Log(ERR, MODULE) << "Failed to prepare buffer: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (fileEntry.m_mode != CTRL_ENTRY_MODE_META_MODIFIED) {
            m_dataSize += fileEntry.m_fileSize; // Scanner to provide fileSize
            m_header.stats.dataSize += fileEntry.m_fileSize;
        }
    } else {
        int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                            "f,%s,%" PRIu32 ",%s\n",
                            fileEntry.m_mode.c_str(),
                            commaCount,
                            fileEntry.m_fileName.c_str());
        if (len <= 0) {
            HCP_Log(ERR, MODULE) << "Failed to prepare buffer: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    m_writeFileBuffer << m_writeCtrlLine;
    m_entries++;
    m_header.stats.noOfFiles++;
    m_fileCount++;

    uint64_t bufSize = (uint64_t) m_writeFileBuffer.tellp() + (uint64_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile  || m_dataSize >= m_maxDataSize) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < CTRL_FILE_NUMBER_THIRTEEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readBuffer, cltHeaderLine)) {
            HCP_Log(ERR, MODULE) << "ReadHeader (getline) failed: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed, incomplete header: " << m_fileName << HCPENDLOG;
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
    getline(m_readBuffer, blankLine); /* To skip the blank line after header */
    return CTRL_FILE_RETCODE::SUCCESS;
}
CTRL_FILE_RETCODE CopyCtrlParser::FillHeader2(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    switch (headerLine) {
        case METADATA_SCOPE :
            m_header.metaDataScope = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case DIR_COUNT :
            m_header.stats.noOfDirs = (uint64_t)atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        case FILE_COUNT :
            m_header.stats.noOfFiles = (uint64_t)atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        case DATA_SIZE :
            m_header.stats.dataSize = (uint64_t)atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        default :
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
            break;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    switch (headerLine) {
        case TITLE :
            m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case HEADER_VERSION :
            m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case TIMESTAMP :
            if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
                HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                    << " line: " << cltHeaderLine;
                return CTRL_FILE_RETCODE::FAILED;
            }
            m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
                cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
            break;
        case TASKID :
            m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case TASKTYPE :
            m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case NASSERVER :
            m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case NASSHARE :
            m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case CTRL_PROTOCOL :
            m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        case CTRL_PROTOCOL_VERSION :
            m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
            break;
        default :
            FillHeader2(headerLine, cltHeaderLineSplit, cltHeaderLine);
            break;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_THIRTEEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, !CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header for Ctrl file Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string CopyCtrlParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case CTRL_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case CTRL_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        case DIR_COUNT:
            ctlHeaderLine = "DirectoryCount:" + to_string(m_header.stats.noOfDirs) + "\n";
            break;
        case FILE_COUNT:
            ctlHeaderLine = "FileCount:" + to_string(m_header.stats.noOfFiles) + "\n";
            break;
        case DATA_SIZE:
            ctlHeaderLine = "DataSize:" + to_string(m_header.stats.dataSize) + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE CopyCtrlParser::ValidateEntry(vector<std::string> &fileContents, const std::string &line)
{
    if (fileContents.empty()) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (fileContents[CTRL_FILE_NUMBER_ZERO] == CTRL_ENTRY_TYPE_DIR) {
        if (ValidateDirEntry(fileContents, line) != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else if (fileContents[CTRL_FILE_NUMBER_ZERO] == CTRL_ENTRY_TYPE_FILE) {
        if (ValidateFileEntry(fileContents, line) != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        HCP_Log(ERR, MODULE) << "Control entry neither file nor dir. Line: " << line
            << "File: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::ValidateDirEntry(vector<std::string> &lineContents, const std::string &line)
{
    uint32_t lineContentsVecSize = lineContents.size();
    uint32_t commaCount = (uint32_t)atoi(lineContents[CTRL_FILE_NUMBER_TWO].c_str());

    if (lineContents[CTRL_FILE_NUMBER_ONE] == CTRL_ENTRY_MODE_DATA_DELETED) {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_5) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (lineContentsVecSize != (commaCount + CTRL_FILE_OFFSET_5)) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir incorrect. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_9) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (lineContentsVecSize != (commaCount + CTRL_FILE_OFFSET_9)) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir incorrect. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::ValidateFileEntry(vector<std::string> &lineContents, const std::string &line)
{
    uint32_t lineContentsVecSize = lineContents.size();
    uint32_t commaCount = (uint32_t)atoi(lineContents[CTRL_FILE_NUMBER_TWO].c_str());

    if (lineContents[CTRL_FILE_NUMBER_ONE] == CTRL_ENTRY_MODE_DATA_DELETED) {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_4) {
            HCP_Log(ERR, MODULE) << "Control Entry for file has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (lineContentsVecSize != (commaCount + CTRL_FILE_OFFSET_4)) {
            HCP_Log(ERR, MODULE) << "Control Entry for file incorrect. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        if (m_header.version == CTRL_HEADER_VERSION_V10) {
            if (lineContentsVecSize < CTRL_FILE_OFFSET_7) {
                HCP_Log(ERR, MODULE) << "Control Entry for file has less columns. Line: "
                    << line << "File: " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
            if (lineContentsVecSize != (commaCount + CTRL_FILE_OFFSET_7)) {
                HCP_Log(ERR, MODULE) << "Control Entry for file incorrect. Line: "
                    << line << "File: " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
        } else {
            if (lineContentsVecSize < CTRL_FILE_OFFSET_8) {
                HCP_Log(ERR, MODULE) << "Control Entry for file has less columns. Line: "
                    << line << "File: " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
            if (lineContentsVecSize != (commaCount + CTRL_FILE_OFFSET_8)) {
                HCP_Log(ERR, MODULE) << "Control Entry for file incorrect. Line: "
                    << line << "File: " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
        }
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CopyCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), CTRL_HEADER_TITLE.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.metaDataScope.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }

    if ((strcmp(m_header.version.c_str(), CTRL_HEADER_VERSION.c_str()) != 0) &&
        (strcmp(m_header.version.c_str(), CTRL_HEADER_VERSION_V10.c_str()) != 0)) {
            return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

std::string CopyCtrlParser::GetMetaFileName(uint16_t metaFileIndex)
{
    std::string metafileName = "meta_file_" + to_string(metaFileIndex);
    return metafileName;
}


void CopyCtrlParser::TranslateDirEntry(vector<string> &fileContents, CopyCtrlDirEntry &dirEntry)
{
    if (m_header.version == CTRL_HEADER_VERSION_V10) {
        TranslateDirEntryV10(fileContents, dirEntry);
        return;
    }
    dirEntry.m_mode = fileContents[CTRL_FILE_OFFSET_1];
    uint32_t offset = CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[CTRL_FILE_OFFSET_2].c_str());

    dirEntry.m_dirName = ParserUtils::ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[CTRL_FILE_NUMBER_ONE] != CTRL_ENTRY_MODE_DATA_DELETED) {
        dirEntry.m_metaFileIndex = ParserUtils::Atou16((fileContents[offset++]).c_str());
        dirEntry.m_metaFileName = GetMetaFileName(dirEntry.m_metaFileIndex);
        dirEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        dirEntry.m_fileCount = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
    }
}

void CopyCtrlParser::TranslateDirEntryV10(vector<string> &fileContents, CopyCtrlDirEntry &dirEntry)
{
    dirEntry.m_mode = fileContents[CTRL_FILE_OFFSET_1];
    uint32_t offset = CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[CTRL_FILE_OFFSET_2].c_str());

    dirEntry.m_dirName = ParserUtils::ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[CTRL_FILE_NUMBER_ONE] != CTRL_ENTRY_MODE_DATA_DELETED) {
        dirEntry.m_metaFileName = fileContents[offset++];
        dirEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        dirEntry.m_fileCount = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());

        m_metaFileName = dirEntry.m_metaFileName;
    }
}

void CopyCtrlParser::TranslateFileEntry(vector<string> &fileContents, CopyCtrlFileEntry &fileEntry)
{
    if (m_header.version == CTRL_HEADER_VERSION_V10) {
        TranslateFileEntryV10(fileContents, fileEntry);
        return;
    }

    fileEntry.m_mode = fileContents[CTRL_FILE_OFFSET_1];
    uint32_t offset = CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[CTRL_FILE_OFFSET_2].c_str());

    fileEntry.m_fileName = ParserUtils::ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[CTRL_FILE_NUMBER_ONE] != CTRL_ENTRY_MODE_DATA_DELETED) {
        fileEntry.m_metaFileIndex = ParserUtils::Atou16((fileContents[offset++]).c_str());
        fileEntry.m_metaFileName = GetMetaFileName(fileEntry.m_metaFileIndex);
        fileEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        fileEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        fileEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
    }
}

void CopyCtrlParser::TranslateFileEntryV10(vector<string> &fileContents, CopyCtrlFileEntry &fileEntry)
{
    fileEntry.m_mode = fileContents[CTRL_FILE_OFFSET_1];
    uint32_t offset = CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[CTRL_FILE_OFFSET_2].c_str());

    fileEntry.m_fileName = ParserUtils::ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[CTRL_FILE_NUMBER_ONE] != CTRL_ENTRY_MODE_DATA_DELETED) {
        fileEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        fileEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        fileEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
        fileEntry.m_metaFileName = m_metaFileName;
    }
}

bool CopyCtrlParser::IsFileBufferEmpty()
{
    lock_guard<std::mutex> lk(m_lock);
    return (m_fileCount == 0);
}

uint32_t CopyCtrlParser::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

bool CopyCtrlParser::CheckCtrlFileTimeElapse()
{
    lock_guard<std::mutex> lk(m_lock);
    time_t curTime = ParserUtils::GetCurrentTimeInSeconds();
    time_t diffTime = (curTime - m_ctrlFileCreationTime);

    if (diffTime >= m_ctrlFileTimeElapsed) {
        if (m_entries == 0) {
            return false;
        }
        if (m_entries < m_minEntriesPerFile && m_dataSize < m_minDataSize &&
            diffTime < CTRL_TEN_MIN_IN_SEC) {
            return false;
        }
        HCP_Log(INFO, MODULE) << "Control file time elapsed for file: " << m_fileName << HCPENDLOG;
        return true;
    }
    return false;
}
