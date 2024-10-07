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
#include "CheckPointParser.h"
#include "Log.h"

using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "CHECKPOINT_PARSER";
}

CheckPointParser::CheckPointParser(CheckPointParser::Params params) : FileParser(false)
{
    m_fileName = params.chkPntFileName;
    m_maxEntriesPerFile = params.maxEntriesPerFile;
    m_maxFileSize = params.maxFileSize;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = CHECKPOINT_HEADER_TITLE;
    m_header.version = CHECKPOINT_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

CheckPointParser::CheckPointParser(const std::string& chkPntFileName) : FileParser(false)
{
    m_fileName = chkPntFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
}

CheckPointParser::~CheckPointParser()
{
    if (m_writeFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::WRITE);

    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE CheckPointParser::OpenWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::GetHeader(CheckPointParser::Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

vector<string> CheckPointParser::ReadMultipleChkPntEntries(int maxReadCnt)
{
    string chkPntEntry {};
    vector<string> chkPntEntriesList {};
    int readCnt = 0;
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return chkPntEntriesList;
    }

    while ((readCnt < maxReadCnt) && (!m_readBuffer.eof())) {
        getline(m_readBuffer, chkPntEntry);
        chkPntEntriesList.push_back(chkPntEntry);
        readCnt++;
    }

    return chkPntEntriesList;
}

vector<string> CheckPointParser::ReadAllChkPntEntries()
{
    string chkPntEntry {};
    vector<string> chkPntEntriesList {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return chkPntEntriesList;
    }

    while (!m_readBuffer.eof()) {
        getline(m_readBuffer, chkPntEntry);
        chkPntEntriesList.push_back(chkPntEntry);
    }

    return chkPntEntriesList;
}

CTRL_FILE_RETCODE CheckPointParser::WriteChkPntEntry(const std::string& chkPntEntry)
{
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write failed as check point file is not open: "
            << m_fileName <<  HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << chkPntEntry;
    m_writeBuffer << endl;
    m_entries++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntriesPerFile) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::WriteHeader()
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
        HCP_Log(ERR, MODULE) << "Write Header for Check Point Ctrl file Failed: " << m_fileName;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string CheckPointParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case CHECKPOINT_HEADER_INFO::TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case CHECKPOINT_HEADER_INFO::METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE CheckPointParser::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> ctlHeaderLineSplit {};

    while (headerLine < CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine {};
        ctlHeaderLineSplit.clear();
        if (!getline(m_readBuffer, ctlHeaderLine)) {
            HCP_Log(ERR, MODULE) << "ReadHeader (getline) failed: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (ctlHeaderLine.empty()) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed, incomplete header: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        boost::algorithm::split(ctlHeaderLineSplit, ctlHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (ctlHeaderLineSplit.size() < CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed: " << m_fileName << " line: " << ctlHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (FillHeader(headerLine, ctlHeaderLineSplit, ctlHeaderLine) != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine);  // To skip the blank line after header
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::FillHeader(uint32_t &headerLine, std::vector<std::string> &ctlHeaderLineSplit,
    std::string &ctlHeaderLine)
{
    if (headerLine == CHECKPOINT_HEADER_INFO::TITLE) {
        m_header.title = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::HEADER_VERSION) {
        m_header.version = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::TIMESTAMP) {
        if (ctlHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << ctlHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            ctlHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + ctlHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::TASKID) {
        m_header.taskId = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::TASKTYPE) {
        m_header.backupType = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::NASSERVER) {
        m_header.nasServer = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::NASSHARE) {
        m_header.nasSharePath = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::PROTOCOL) {
        m_header.proto = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::PROTOCOL_VERSION) {
        m_header.protoVersion = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_INFO::METADATA_SCOPE) {
        m_header.metaDataScope = ctlHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << ctlHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE CheckPointParser::ValidateHeader()
{
    if ((m_header.title != CHECKPOINT_HEADER_TITLE) ||
        (m_header.version != CHECKPOINT_HEADER_VERSION) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.protoVersion.empty() || m_header.metaDataScope.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}
