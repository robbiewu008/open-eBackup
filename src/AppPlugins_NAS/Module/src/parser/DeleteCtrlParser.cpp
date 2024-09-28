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
#include "DeleteCtrlParser.h"
#include "Log.h"
#include "securec.h"
#include <cinttypes>

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "DEL_CTRL_PARSER";
constexpr uint32_t DEL_CTRL_FILE_MAX_SIZE = (8 * 1024 * 1024);   /* 8 MB */
}

DeleteCtrlParser::DeleteCtrlParser(DeleteCtrlParser::Params params) : FileParser(false)
{
    m_fileName = params.m_ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_maxFileSize = DEL_CTRL_FILE_MAX_SIZE;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = DELETECTRL_HEADER_TITLE;
    m_header.version = DELETECTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
}

DeleteCtrlParser::DeleteCtrlParser(std::string ctlFileName) : FileParser(false)
{
    m_fileName = ctlFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
}

DeleteCtrlParser::~DeleteCtrlParser()
{
    if (m_writeFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::WRITE);

    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE DeleteCtrlParser::OpenWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::GetHeader(DeleteCtrlParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::ReadEntry(DeleteCtrlEntry &deleteEntry, string &fileName)
{
    string line {};
    vector<string> lineContents {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    do {
        getline(m_readBuffer, line);
        if (line.empty()) {
            return CTRL_FILE_RETCODE::READ_EOF;
        }
        boost::algorithm::split(lineContents, line, boost::is_any_of(","), boost::token_compress_off);
        if (ValidateEntry(lineContents, line) == CTRL_FILE_RETCODE::SUCCESS) {
            break;
        }
        line.clear();
        lineContents.clear();
    } while (true);

    HCP_Log(DEBUG, MODULE) << line << HCPENDLOG;

    if (lineContents[CTRL_FILE_NUMBER_ZERO] == DELETECTRL_ENTRY_TYPE_DIR) {
        TranslateDirEntry(lineContents, deleteEntry);
    } else {
        uint32_t offset = CTRL_FILE_OFFSET_2;
        uint32_t totCommaCnt = (uint32_t) atoi(lineContents[CTRL_FILE_OFFSET_1].c_str());
        fileName = ParserUtils::ConstructStringName(offset, totCommaCnt, lineContents);
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::WriteDirEntry(DeleteCtrlEntry &deleteEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = ParserUtils::GetCommaCountOfString(deleteEntry.m_absPath.c_str());

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as control file is not open: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int ret = memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE);
    if (ret != 0) {
        ERRLOG("failed to memset m_writeCtrlLine %p, ret %d", m_writeCtrlLine, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE, 
                        "d,%" PRIu32 ",%s,%d\n",
                        commaCount,
                        deleteEntry.m_absPath.c_str(),
                        deleteEntry.m_isDel);
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Prepare buffer failed: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;
    m_entries++;
    if (deleteEntry.m_isDel) {
        m_header.stats.noOfDelDirs++;
    }

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::WriteFileEntry(string &fileName)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = ParserUtils::GetCommaCountOfString(fileName.c_str());

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as control file is not open: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    int ret = memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE);
    if (ret != 0) {
        ERRLOG("failed to memset m_writeCtrlLine %p, ret %d", m_writeCtrlLine, ret);
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE,
                        "f,%" PRIu32 ",%s\n",
                        commaCount,
                        fileName.c_str());
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Prepare buffer failed: " << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;
    m_entries++;
    m_header.stats.noOfDelFiles++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile) {
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

void DeleteCtrlParser::PrintDirEntry(DeleteCtrlEntry& deleteEntry)
{
    DBGLOG("Write to delete ctrl - %s", m_fileName.c_str());
    DBGLOG("Write dirEntry - absPath : %s, isDel: %d, isDir, %d", deleteEntry.m_absPath.c_str(),
        deleteEntry.m_isDel, deleteEntry.m_isDir);
}

void DeleteCtrlParser::PrintFileEntry(string fileName)
{
    DBGLOG("Write to delete ctrl - %s", m_fileName.c_str());
    DBGLOG("Write fileEntry - fileName: %s", fileName.c_str());
}

CTRL_FILE_RETCODE DeleteCtrlParser::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
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

CTRL_FILE_RETCODE DeleteCtrlParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TITLE)) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_HEADER_VERSION)) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TIMESTAMP)) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TASKID)) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_NASSERVER)) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_NASSHARE)) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_PROTOCOL)) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_PROTOCOL_VERSION)) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_DIRS_TO_DELETE_COUNT)) {
        m_header.stats.noOfDelDirs = (uint64_t) atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
    } else if (headerLine == static_cast<uint32_t>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_FILE_TO_DELETE_COUNT)) {
        m_header.stats.noOfDelFiles = (uint64_t) atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }
    headerBuffer << "\n";
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, !CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header for Bkup Delete Ctrl file Failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string DeleteCtrlParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TITLE):
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_HEADER_VERSION):
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TIMESTAMP):
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_TASKID):
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_NASSERVER):
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_NASSHARE):
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_PROTOCOL):
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_PROTOCOL_VERSION):
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_DIRS_TO_DELETE_COUNT):
            ctlHeaderLine = "DirectoryDeleteCount:" + to_string(m_header.stats.noOfDelDirs) + "\n";
            break;
        case  static_cast<int>(DELETE_CTRL_HEADER_INFO::DELETE_CTRL_FILE_TO_DELETE_COUNT):
            ctlHeaderLine = "FileDeleteCount:" + to_string(m_header.stats.noOfDelFiles) + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE DeleteCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), DELETECTRL_HEADER_TITLE.c_str()) != 0) ||
        (strcmp(m_header.version.c_str(), DELETECTRL_HEADER_VERSION.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.nasServer.empty() ||
        m_header.nasSharePath.empty() || m_header.proto.empty() || m_header.protoVersion.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE DeleteCtrlParser::ValidateEntry(vector<string> lineContents, string line)
{
    if (lineContents.empty() || line.empty()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    uint32_t lineContentsVecSize = lineContents.size();

    if (lineContents[CTRL_FILE_NUMBER_ZERO] == DELETECTRL_ENTRY_TYPE_DIR) {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_4) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        uint32_t commaCount = (uint32_t) atoi(lineContents[CTRL_FILE_NUMBER_ONE].c_str());
        if (lineContentsVecSize != (CTRL_FILE_OFFSET_4 + commaCount)) {
            HCP_Log(ERR, MODULE) << "Control Entry for dir incorrect. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else if (lineContents[CTRL_FILE_NUMBER_ZERO] == DELETECTRL_ENTRY_TYPE_FILE) {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_3) {
            HCP_Log(ERR, MODULE) << "Control Entry for file has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        uint32_t commaCount = (uint32_t) atoi(lineContents[CTRL_FILE_NUMBER_ONE].c_str());
        if (lineContentsVecSize != (CTRL_FILE_OFFSET_3 + commaCount)) {
            HCP_Log(ERR, MODULE) << "Control Entry for file incorrect. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        HCP_Log(ERR, MODULE) << "Control entry neither file nor dir. Line: " << line
            << "File: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

void DeleteCtrlParser::TranslateDirEntry(vector<string> &fileContents, DeleteCtrlEntry &deleteEntry)
{
    deleteEntry.m_isDir = true;

    uint32_t offset = CTRL_FILE_OFFSET_2;
    uint32_t totCommaCnt = (uint32_t) atoi(fileContents[CTRL_FILE_OFFSET_1].c_str());
    deleteEntry.m_absPath = ParserUtils::ConstructStringName(offset, totCommaCnt, fileContents);

    if (atoi(fileContents[offset].c_str()))
        deleteEntry.m_isDel = true;
    else
        deleteEntry.m_isDel = false;
}

uint32_t DeleteCtrlParser::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

std::string DeleteCtrlParser::GetCtrlFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_fileName;
}
