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
#include "ScannerHardLinkCtrl.h"

using namespace std;
using namespace ScannerHardLinkCtrl;

CtrlFile::CtrlFile(Params params)
{
    m_ctlFileName = params.ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_minEntriesPerFile = params.minEntriesPerFile;
    m_maxDataSize = params.maxDataSize;
    m_minDataSize = params.minDataSize;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);

    m_header.title = NAS_SCANNERHARDLINKCTRL_HEADER_TITLE;
    m_header.version = NAS_SCANNERHARDLINKCTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_maxFileSize = NAS_CTRL_FILE_MAX_SIZE;
}

CtrlFile::CtrlFile(std::string ctlFileName)
{
    m_ctlFileName = ctlFileName;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);
}

CtrlFile::~CtrlFile()
{
    if (m_writeFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);
    if (m_readFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
}

NAS_CTRL_FILE_RETCODE CtrlFile::Open(NAS_CTRL_FILE_OPEN_MODE mode)
{
    NAS_CTRL_FILE_RETCODE ret;
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        ret = FileOpen<std::ifstream>(m_readFd, std::ios::in);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        READ_FROM_FILE(m_readBuffer, m_readFd, ret);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Read from stream is failed for " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ReadHeader();
        if (ValidateHeader() != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Header verification failed for " << m_ctlFileName << HCPENDLOG;
            m_readFd.close();
            return NAS_CTRL_FILE_RET_FAILED;
        }
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::app);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_ctrlFileCreationTime = GetCurrentTimeInSeconds();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for :" << m_ctlFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE CtrlFile::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    strmFd.open(m_ctlFileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(m_ctrlFileParentDir) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_ctlFileName << ", Parent dir not reachable";
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(m_ctlFileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_ctlFileName << ", ERR: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::Close(NAS_CTRL_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (!m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        NAS_CTRL_FILE_RETCODE ret = WriteHeader();
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        WRITE_TO_FILE(m_writeBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_writeFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (!m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        m_readFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid close mode for :" << m_ctlFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

NAS_CTRL_FILE_RETCODE CtrlFile::GetHeader(Header &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::ReadEntry(LinkEntry &linkEntry, InodeEntry &inodeEntry)
{
    string ctlFileLine {};
    vector<string> ctlFileLineSplit {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Read failed as control file is not open: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    do {
        getline(m_readBuffer, ctlFileLine);
        if (ctlFileLine.empty()) {
            return NAS_CTRL_FILE_RET_READ_EOF;
        }
        boost::algorithm::split(ctlFileLineSplit, ctlFileLine, boost::is_any_of(","), boost::token_compress_off);
        if (ValidateEntry(ctlFileLineSplit, ctlFileLine) == NAS_CTRL_FILE_RET_SUCCESS) {
            break;
        }
        ctlFileLine.clear();
        ctlFileLineSplit.clear();
    } while (true);

    if (ctlFileLineSplit[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERHARDLINKCTRL_ENTRY_TYPE_INODE) {
        inodeEntry.linkCount = (uint32_t)atoi((ctlFileLineSplit[NAS_CTRL_FILE_OFFSET_1]).c_str());
        inodeEntry.inode = (uint64_t)atoll((ctlFileLineSplit[NAS_CTRL_FILE_OFFSET_2]).c_str());
    } else {
        TranslateLinkEntry(ctlFileLineSplit, linkEntry);
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

void CtrlFile::TranslateLinkEntry(vector<std::string> &fileContents, LinkEntry &linkEntry)
{
    uint32_t offset = NAS_CTRL_FILE_OFFSET_2;
    uint32_t totDirCommaCnt = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_OFFSET_1].c_str());

    linkEntry.dirName = ConstructStringName(offset, totDirCommaCnt, fileContents);

    uint32_t totFileCommaCnt = (uint32_t)atoi(fileContents[offset].c_str());
    ++offset;

    linkEntry.fileName = ConstructStringName(offset, totFileCommaCnt, fileContents);
    linkEntry.metaFileName = fileContents[offset++];
    linkEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
    linkEntry.metaFileReadLen = (uint64_t)atoll((fileContents[offset++]).c_str());
    linkEntry.aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
}

NAS_CTRL_FILE_RETCODE CtrlFile::WriteEntry(LinkEntry &linkEntry)
{
    string ctlFileLine {};
    lock_guard<std::mutex> lk(m_lock);
    uint32_t dirCommaCount = GetCommaCountOfString(linkEntry.dirName);
    uint32_t fileCommaCount = GetCommaCountOfString(linkEntry.fileName);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as control file is not open: " << m_ctlFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    ctlFileLine = "f," +
                to_string(dirCommaCount) + "," +
                linkEntry.dirName + "," +
                to_string(fileCommaCount) + "," +
                linkEntry.fileName + "," +
                linkEntry.metaFileName + "," +
                to_string(linkEntry.metaFileOffset) + "," +
                to_string(linkEntry.metaFileReadLen) + "," +
                to_string(linkEntry.aclFlag) + "\n";

    m_writeBuffer << ctlFileLine;
    m_entries++;
    m_header.stats.noOfFiles++;
    m_dataSize += linkEntry.fileSize; // Scanner to provide fileSize
    m_header.stats.dataSize += linkEntry.fileSize;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize > m_maxFileSize || m_entries > m_maxEntryPerFile  || m_dataSize > m_maxDataSize) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::WriteInodeEntry(InodeEntry &inodeEntry)
{
    string ctlFileLine {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as control file is not open: " << m_ctlFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    ctlFileLine = "i," +
                to_string(inodeEntry.linkCount) + "," +
                to_string(inodeEntry.inode) + "\n";

    m_writeBuffer << ctlFileLine;
    m_entries++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize > m_maxFileSize || m_entries > m_maxEntryPerFile) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < NAS_CTRL_FILE_NUMBER_ELEVEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readBuffer, cltHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_ctlFileName << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (FillHeader(headerLine, cltHeaderLineSplit, cltHeaderLine) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine); /* To skip the blank line after header */
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == HLINK_TITLE) {
        m_header.title = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_ctlFileName
                << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == HLINK_TASKID) {
        m_header.taskId = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == HLINK_FILE_COUNT) {
        m_header.stats.noOfFiles = (uint64_t)atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
    } else if (headerLine == HLINK_DATA_SIZE) {
        m_header.stats.dataSize = (uint64_t)atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_ctlFileName << HCPENDLOG;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::WriteHeader()
{
    uint32_t headerLine = 0;
    std::stringstream headerBuffer {};
    NAS_CTRL_FILE_RETCODE ret;
    while (headerLine < NAS_CTRL_FILE_NUMBER_ELEVEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    WRITE_TO_FILE(headerBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write Header for Hard Link Ctrl file Failed: "  << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string CtrlFile::GetFileHeaderLine(uint32_t headerLine)
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
            HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE CtrlFile::ValidateEntry(vector<std::string> &fileContents, std::string &line)
{
    if (fileContents.empty()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    uint32_t lineContentsVecSize = fileContents.size();

    if (fileContents[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERHARDLINKCTRL_ENTRY_TYPE_INODE) {
        if (lineContentsVecSize != NAS_CTRL_FILE_OFFSET_3) {
            HCP_Log(ERR, CTL_MOD_NAME)
                << "Hardlink control Entry for inode incorrect. Line:" << line <<
                    "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else if (fileContents[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERHARDLINKCTRL_ENTRY_TYPE_FILE) {
        if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_9) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Hardlink Entry for file has less columns. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        uint32_t dirCommaCount = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_NUMBER_ONE].c_str());
        uint32_t fileCommaCntIdx = NAS_CTRL_FILE_OFFSET_3 +
            (uint32_t)atoi(fileContents[NAS_CTRL_FILE_NUMBER_ONE].c_str());
        uint32_t fileCommaCount = (uint32_t)atoi(fileContents[fileCommaCntIdx].c_str());
        if (lineContentsVecSize != (dirCommaCount + fileCommaCount + NAS_CTRL_FILE_OFFSET_9)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Hardlink control Entry for file incorrect. Line: " << line <<
                    "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else {
        HCP_Log(ERR, CTL_MOD_NAME)
            << "Hardlink control entry neither file nor dir. Line: " << line <<
                "File: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE CtrlFile::ValidateHeader()
{
    return NAS_CTRL_FILE_RET_SUCCESS;
}

uint32_t CtrlFile::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

bool CtrlFile::CheckCtrlFileTimeElapse()
{
    lock_guard<std::mutex> lk(m_lock);
    time_t curTime = GetCurrentTimeInSeconds();
    time_t diffTime = (curTime - m_ctrlFileCreationTime);

    if (diffTime >= m_ctrlFileTimeElapsed) {
        if (m_entries == 0) {
            return false;
        }
        if (m_entries < m_minEntriesPerFile && m_dataSize < m_minDataSize &&
            diffTime < NAS_SCANNERHARDLINKCTRL_TEN_MIN_IN_SEC) {
            return false;
        }
        HCP_Log(INFO, CTL_MOD_NAME) << "Control file time elapsed for file: " << m_ctlFileName << HCPENDLOG;
        return true;
    }
    return false;
}
