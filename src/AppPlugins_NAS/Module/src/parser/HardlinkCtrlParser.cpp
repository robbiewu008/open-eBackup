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
#include "HardlinkCtrlParser.h"
#include "Log.h"
#include "securec.h"
#include <cinttypes>

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "HARDLINK_CTRL_PARSER";
}

HardlinkCtrlParser::HardlinkCtrlParser(Params params) : FileParser(false)
{
    m_fileName = params.ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_minEntriesPerFile = params.minEntriesPerFile;
    m_maxDataSize = params.maxDataSize;
    m_minDataSize = params.minDataSize;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);

    m_header.title = HARDLINKCTRL_HEADER_TITLE;
    m_header.version = HARDLINKCTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_maxFileSize = params.maxCtrlFileSize;;

    DBGLOG("Hardlink params - maxEntriesPerFile: %u, minEntriesPerFile: %u, m_ctrlFileTimeElapsed: %u,",
        " maxDataSize: %u, minDataSize: %u ,m_ctlFileName: %s, taskId: %s, backupType: %s, nasServer: %s,",
        " nasSharePath: %s, proto: %s, protoVersion: %s",
        params.maxEntriesPerFile, params.minEntriesPerFile, params.ctrlFileTimeElapsed, params.maxDataSize,
        params.minDataSize, params.ctlFileName.c_str(), params.taskId.c_str(), params.backupType.c_str(),
        params.nasServer.c_str(), params.nasSharePath.c_str(),
        params.proto.c_str(), params.protoVersion.c_str());
}

HardlinkCtrlParser::HardlinkCtrlParser(std::string ctlFileName) : FileParser(false)
{
    m_fileName = ctlFileName;
    m_fileParentDir = ParserUtils::GetParentDirOfFile(m_fileName);
}

HardlinkCtrlParser::~HardlinkCtrlParser()
{
    if (m_writeFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::WRITE);
    if (m_readFd.is_open())
        Close(CTRL_FILE_OPEN_MODE::READ);
}

CTRL_FILE_RETCODE HardlinkCtrlParser::OpenWrite()
{
    m_ctrlFileCreationTime = ParserUtils::GetCurrentTimeInSeconds();
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::GetHeader(Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    header = m_header;
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::ReadEntry(HardlinkCtrlEntry &linkEntry, HardlinkCtrlInodeEntry &inodeEntry)
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
        }
        ctlFileLine.clear();
        ctlFileLineSplit.clear();
    } while (true);

    if (ctlFileLineSplit[CTRL_FILE_NUMBER_ZERO] == HARDLINKCTRL_ENTRY_TYPE_INODE) {
        inodeEntry.linkCount = (uint32_t)atoi((ctlFileLineSplit[CTRL_FILE_OFFSET_1]).c_str());
        inodeEntry.inode = (uint64_t)atoll((ctlFileLineSplit[CTRL_FILE_OFFSET_2]).c_str());
    } else {
        TranslateLinkEntry(ctlFileLineSplit, linkEntry);
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

void HardlinkCtrlParser::TranslateLinkEntry(vector<std::string> &fileContents, HardlinkCtrlEntry &linkEntry)
{
    uint32_t offset = CTRL_FILE_OFFSET_2;
    uint32_t totDirCommaCnt = (uint32_t)atoi(fileContents[CTRL_FILE_OFFSET_1].c_str());

    linkEntry.dirName = ParserUtils::ConstructStringName(offset, totDirCommaCnt, fileContents);

    uint32_t totFileCommaCnt = (uint32_t)atoi(fileContents[offset].c_str());
    ++offset;

    linkEntry.fileName = ParserUtils::ConstructStringName(offset, totFileCommaCnt, fileContents);
    linkEntry.metaFileName = fileContents[offset++];
    linkEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
    linkEntry.metaFileReadLen = (uint64_t)atoll((fileContents[offset++]).c_str());
    linkEntry.aclFlag = (uint32_t)atoi((fileContents[offset++]).c_str());
    linkEntry.m_hardLinkFilesCnt = (uint32_t)atoi((fileContents[offset]).c_str());
}

CTRL_FILE_RETCODE HardlinkCtrlParser::WriteEntry(HardlinkCtrlEntry &linkEntry)
{
    lock_guard<std::mutex> lk(m_lock);
    uint32_t dirCommaCount = ParserUtils::GetCommaCountOfString(linkEntry.dirName.c_str());
    uint32_t fileCommaCount = ParserUtils::GetCommaCountOfString(linkEntry.fileName.c_str());

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
                        "f,%" PRIu32 ",%s,%" PRIu32 ",%s,%s,%" PRIu64 ",%" PRIu64 ",%" PRIu32 ",%" PRIu32 "\n",
                        dirCommaCount,
                        linkEntry.dirName.c_str(),
                        fileCommaCount,
                        linkEntry.fileName.c_str(),
                        linkEntry.metaFileName.c_str(),
                        linkEntry.metaFileOffset,
                        linkEntry.metaFileReadLen,
                        linkEntry.aclFlag,
                        linkEntry.m_hardLinkFilesCnt);
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Prepare buffer failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;

    m_entries++;
    m_header.stats.noOfFiles++;
    m_dataSize += linkEntry.fileSize; // Scanner to provide fileSize
    m_header.stats.dataSize += linkEntry.fileSize;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile  || m_dataSize >= m_maxDataSize) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::WriteInodeEntry(HardlinkCtrlInodeEntry &inodeEntry)
{
    lock_guard<std::mutex> lk(m_lock);

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
                        "i,%" PRIu32 ",%" PRIu64 "\n",
                        inodeEntry.linkCount,
                        inodeEntry.inode);
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Prepare buffer failed: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    m_writeBuffer << m_writeCtrlLine;
    m_entries++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntryPerFile) {
        // The caller has to close this file and create a new file
        return CTRL_FILE_RETCODE::LIMIT_REACHED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

void HardlinkCtrlParser::PrintHardlinkEntry(HardlinkCtrlEntry& linkEntry)
{
    DBGLOG("Write to hardlink ctrl - %s", m_fileName.c_str());
    DBGLOG("Write hardlink entry - metaFileName: %s, dirName: %s, fileName: %s, aclFlag: %u, metaFileOffset: %u,",
        " metaFileReadLen: %u , fileSize: %u, hardLinkFilesCnt: %u",
        linkEntry.metaFileName.c_str(), linkEntry.dirName.c_str(), linkEntry.fileName.c_str(), linkEntry.aclFlag,
        linkEntry.metaFileOffset, linkEntry.fileSize, linkEntry.m_hardLinkFilesCnt);
}

void HardlinkCtrlParser::PrintHardlinkInodeEntry(HardlinkCtrlInodeEntry& linkInodeEntry)
{
    DBGLOG("Write to hardlink ctrl - %s", m_fileName.c_str());
    DBGLOG("Write hardlink inode entry - linkCount: %u, inode: %u", linkInodeEntry.linkCount, linkInodeEntry.inode);
}

CTRL_FILE_RETCODE HardlinkCtrlParser::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < CTRL_FILE_NUMBER_ELEVEN) {
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

CTRL_FILE_RETCODE HardlinkCtrlParser::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == HLINK_TITLE) {
        m_header.title = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, MODULE) << "ReadHeader failed timestamp failed: " << m_fileName
                << " line: " << cltHeaderLine;
            return CTRL_FILE_RETCODE::FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == HLINK_TASKID) {
        m_header.taskId = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_FILE_COUNT) {
        m_header.stats.noOfFiles = (uint64_t)atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
    } else if (headerLine == HLINK_DATA_SIZE) {
        m_header.stats.dataSize = (uint64_t)atoll((cltHeaderLineSplit[CTRL_FILE_NUMBER_ONE]).c_str());
    } else {
        HCP_Log(INFO, MODULE) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_fileName << HCPENDLOG;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::WriteHeader()
{
    uint32_t headerLine = 0;
    std::stringstream headerBuffer {};
    
    while (headerLine < CTRL_FILE_NUMBER_ELEVEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    CTRL_FILE_RETCODE ret = WriteToFile(headerBuffer, !CTRL_BINARY_FILE);
    if (ret != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write Header for Hard Link Ctrl file Failed: "  << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

string HardlinkCtrlParser::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case HLINK_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case HLINK_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case HLINK_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case HLINK_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case HLINK_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case HLINK_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case HLINK_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case HLINK_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case HLINK_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case HLINK_FILE_COUNT:
            ctlHeaderLine = "FileCount:" + to_string(m_header.stats.noOfFiles) + "\n";
            break;
        case HLINK_DATA_SIZE:
            ctlHeaderLine = "DataSize:" + to_string(m_header.stats.dataSize) + "\n";
            break;
        default:
            HCP_Log(INFO, MODULE) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::ValidateEntry(vector<std::string> &fileContents, std::string &line)
{
    if (fileContents.empty()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    uint32_t lineContentsVecSize = fileContents.size();

    if (fileContents[CTRL_FILE_NUMBER_ZERO] == HARDLINKCTRL_ENTRY_TYPE_INODE) {
        if (lineContentsVecSize != CTRL_FILE_OFFSET_3) {
            HCP_Log(ERR, MODULE)
                << "Hardlink control Entry for inode incorrect. Line:" << line <<
                    "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else if (fileContents[CTRL_FILE_NUMBER_ZERO] == HARDLINKCTRL_ENTRY_TYPE_FILE) {
        if (lineContentsVecSize < CTRL_FILE_OFFSET_10) {
            HCP_Log(ERR, MODULE) << "Hardlink Entry for file has less columns. Line: "
                << line << "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        uint32_t dirCommaCount = (uint32_t)atoi(fileContents[CTRL_FILE_NUMBER_ONE].c_str());
        uint32_t fileCommaCntIdx = CTRL_FILE_OFFSET_3 +
            (uint32_t)atoi(fileContents[CTRL_FILE_NUMBER_ONE].c_str());
        uint32_t fileCommaCount = (uint32_t)atoi(fileContents[fileCommaCntIdx].c_str());
        if (lineContentsVecSize != (dirCommaCount + fileCommaCount + CTRL_FILE_OFFSET_10)) {
            HCP_Log(ERR, MODULE) << "Hardlink control Entry for file incorrect. Line: " << line <<
                    "File: " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    } else {
        HCP_Log(ERR, MODULE)
            << "Hardlink control entry neither file nor dir. Line: " << line <<
                "File: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
    if ((strcmp(m_header.title.c_str(), HARDLINKCTRL_HEADER_TITLE.c_str()) != 0) ||
        (strcmp(m_header.version.c_str(), HARDLINKCTRL_HEADER_VERSION.c_str()) != 0) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.protoVersion.empty()) {
            return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

uint32_t HardlinkCtrlParser::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

bool HardlinkCtrlParser::CheckCtrlFileTimeElapse()
{
    lock_guard<std::mutex> lk(m_lock);
    time_t curTime = ParserUtils::GetCurrentTimeInSeconds();
    time_t diffTime = (curTime - m_ctrlFileCreationTime);

    if (diffTime >= m_ctrlFileTimeElapsed) {
        if (m_entries == 0) {
            return false;
        }
        if (m_entries < m_minEntriesPerFile && m_dataSize < m_minDataSize &&
            diffTime < HARDLINKCTRL_TEN_MIN_IN_SEC) {
            return false;
        }
        HCP_Log(INFO, MODULE) << "Control file time elapsed for file: " << m_fileName << HCPENDLOG;
        return true;
    }
    return false;
}

CTRL_FILE_RETCODE HardlinkCtrlParser::ReadFileForCheckPoint(std::queue<HardlinkFileCache> &hardLinkFCacheQue,
    vector<pair<string, uint32_t>> &hardlinkFilesCntList)
{
    uint64_t inode = 0;

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read failed as control file is not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    do {
        HardlinkCtrlEntry linkEntry {};
        HardlinkCtrlInodeEntry inodeEntry {};
        CTRL_FILE_RETCODE ret = ReadEntry(linkEntry, inodeEntry);
        if (ret == CTRL_FILE_RETCODE::READ_EOF) {
            break;
        }

        if (inodeEntry.inode != 0) {
            inode = inodeEntry.inode;
            continue;
        }

        HardlinkFileCache hardLinkFCache {};
        hardLinkFCache.m_inode = inode;
        hardLinkFCache.m_mdataOffset = linkEntry.metaFileOffset;
        hardLinkFCache.m_metaLength = linkEntry.metaFileReadLen;
        hardLinkFCache.m_dirName = linkEntry.dirName;
        hardLinkFCache.m_fileName =  linkEntry.fileName;
        hardLinkFCache.m_aclFlag = linkEntry.aclFlag;
        hardLinkFCache.m_metafileName = linkEntry.metaFileName;
        hardlinkFilesCntList.emplace_back(make_pair(linkEntry.dirName, linkEntry.m_hardLinkFilesCnt));

        hardLinkFCacheQue.push(hardLinkFCache);
    } while (true);

    return CTRL_FILE_RETCODE::SUCCESS;
}