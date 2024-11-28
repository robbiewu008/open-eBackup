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
#include "ScannerCheckPoint.h"

using namespace std;

ScannerCheckPoint::ScannerCheckPoint(ScannerCheckPointParams params)
{
    m_chkPntFileName = params.chkPntFileName;
    m_maxEntriesPerFile = NAS_SCANNER_CHECKPOINT_MAX_ENTRIES_PER_FILE;
    m_maxFileSize = NAS_SCANNER_CHECKPOINT_MAX_FILE_SIZE;
    m_chkPntFileParentDir = GetParentDirOfFile(m_chkPntFileName);

    m_header.title = NAS_SCANNER_CHECKPOINT_HEADER_TITLE;
    m_header.version = NAS_SCANNER_CHECKPOINT_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

ScannerCheckPoint::ScannerCheckPoint(string chkPntFileName)
{
    m_chkPntFileName = chkPntFileName;
    m_chkPntFileParentDir = GetParentDirOfFile(m_chkPntFileName);
}

ScannerCheckPoint::~ScannerCheckPoint()
{
    if (m_writeFd.is_open()) {
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);
    }

    if (m_readFd.is_open()) {
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
    }
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::Open(NAS_CTRL_FILE_OPEN_MODE mode)
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
            HCP_Log(ERR, CTL_MOD_NAME) << "Read from stream is failed for " << m_chkPntFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ReadHeader();
        if (ValidateHeader() != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Header verification failed for " << m_chkPntFileName << HCPENDLOG;
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
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for : " << m_chkPntFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE ScannerCheckPoint::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    strmFd.open(m_chkPntFileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(m_chkPntFileParentDir) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_chkPntFileName << ", Parent dir not reachable";
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(m_chkPntFileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_chkPntFileName << ", ERR: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::Close(NAS_CTRL_FILE_OPEN_MODE mode)
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

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid close mode for :" << m_chkPntFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::GetHeader(ScannerCheckPointHeader &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

vector<string> ScannerCheckPoint::ReadMultipleChkPntEntries(int maxReadCnt)
{
    string chkPntEntry {};
    vector<string> chkPntEntriesList {};
    int readCnt = 0;
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Read failed as control file is not open: " << m_chkPntFileName << HCPENDLOG;
        return chkPntEntriesList;
    }

    while ((readCnt < maxReadCnt) && (!m_readBuffer.eof())) {
        getline(m_readBuffer, chkPntEntry);
        chkPntEntriesList.push_back(chkPntEntry);
        readCnt++;
    }

    return chkPntEntriesList;
}

vector<string> ScannerCheckPoint::ReadAllChkPntEntries()
{
    string chkPntEntry {};
    vector<string> chkPntEntriesList {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Read failed as control file is not open: " << m_chkPntFileName << HCPENDLOG;
        return chkPntEntriesList;
    }

    while (!m_readBuffer.eof()) {
        std::getline(m_readBuffer, chkPntEntry);
        chkPntEntriesList.push_back(chkPntEntry);
    }

    return chkPntEntriesList;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::WriteChkPntEntry(string chkPntEntry)
{
    lock_guard<std::mutex> lk(m_lock);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as check point file is not open: "
            << m_chkPntFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    m_writeBuffer << chkPntEntry;
    m_writeBuffer << std::endl;
    m_entries++;

    uint32_t bufSize = static_cast<uint32_t>(m_writeBuffer.tellp());
    if (bufSize >= m_maxFileSize || m_entries >= m_maxEntriesPerFile) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::WriteHeader()
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
    WRITE_TO_FILE(headerBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write Header for Check Point Ctrl file Failed: " << m_chkPntFileName;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string ScannerCheckPoint::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case CHECKPOINT_TITLE:
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case CHECKPOINT_HEADER_VERSION:
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case CHECKPOINT_TIMESTAMP:
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case CHECKPOINT_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case CHECKPOINT_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case CHECKPOINT_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case CHECKPOINT_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case CHECKPOINT_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case CHECKPOINT_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case CHECKPOINT_METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> ctlHeaderLineSplit {};

    while (headerLine < NAS_CTRL_FILE_NUMBER_TEN) {
        string ctlHeaderLine {};
        ctlHeaderLineSplit.clear();
        if (!getline(m_readBuffer, ctlHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_chkPntFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (ctlHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_chkPntFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        boost::algorithm::split(ctlHeaderLineSplit, ctlHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (ctlHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_chkPntFileName << " line: " << ctlHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (FillHeader(headerLine, ctlHeaderLineSplit, ctlHeaderLine) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readBuffer, blankLine);  // To skip the blank line after header
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::FillHeader(uint32_t &headerLine, vector<string> &ctlHeaderLineSplit,
    string &ctlHeaderLine)
{
    if (headerLine == CHECKPOINT_TITLE) {
        m_header.title = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_HEADER_VERSION) {
        m_header.version = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_TIMESTAMP) {
        if (ctlHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_chkPntFileName
                << " line: " << ctlHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_header.timestamp = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
            ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == CHECKPOINT_TASKID) {
        m_header.taskId = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_TASKTYPE) {
        m_header.backupType = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_NASSERVER) {
        m_header.nasServer = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_NASSHARE) {
        m_header.nasSharePath = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_PROTOCOL) {
        m_header.proto = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_PROTOCOL_VERSION) {
        m_header.protoVersion = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == CHECKPOINT_METADATA_SCOPE) {
        m_header.metaDataScope = ctlHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << ctlHeaderLine << " fileName: " << m_chkPntFileName << HCPENDLOG;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerCheckPoint::ValidateHeader()
{
    if ((m_header.title != NAS_SCANNER_CHECKPOINT_HEADER_TITLE) ||
        (m_header.version != NAS_SCANNER_CHECKPOINT_HEADER_VERSION) ||
        m_header.timestamp.empty() || m_header.taskId.empty() || m_header.backupType.empty() ||
        m_header.nasServer.empty() || m_header.nasSharePath.empty() || m_header.proto.empty() ||
        m_header.protoVersion.empty() || m_header.metaDataScope.empty()) {
            return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}
