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
#include "BackupMtimeCtrl.h"
using namespace std;

namespace {
    constexpr uint32_t NAS_MTIME_CTRL_FILE_MAX_SIZE = (32 * 1024 * 1024);   /* 32 MB */
}

BackupMtimeCtrl::BackupMtimeCtrl(BackupMtimeCtrlParams params)
{
    m_ctlFileName = params.m_ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_maxFileSize = NAS_MTIME_CTRL_FILE_MAX_SIZE;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);

    m_header.title = NAS_BACKUPMTIMECTRL_HEADER_TITLE;
    m_header.version = NAS_BACKUPMTIMECTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
}

BackupMtimeCtrl::BackupMtimeCtrl(std::string ctlFileName)
{
    m_ctlFileName = ctlFileName;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);
}

BackupMtimeCtrl::~BackupMtimeCtrl()
{
    if (m_writeFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);

    if (m_readFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::Open(NAS_CTRL_FILE_OPEN_MODE mode)
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
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for :" << m_ctlFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
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

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::Close(NAS_CTRL_FILE_OPEN_MODE mode)
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

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::GetHeader(BackupMtimeCtrlHeader &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::ValidateEntry(vector<string> lineContents, string line)
{
    if (lineContents.empty() || line.empty()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    uint32_t lineContentsVecSize = lineContents.size();
    if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_11) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for dir has less columns. Line: "
            << line << "File: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    uint32_t commaCount = (uint32_t) atoi(lineContents[NAS_CTRL_FILE_NUMBER_ZERO].c_str());
    if (lineContentsVecSize != (NAS_CTRL_FILE_OFFSET_11 + commaCount)) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Mtime file has Wrong Entries, Line: "
            << line << ", File: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::ReadEntry(BackupMtimeCtrlEntry &mtimeEntry)
{
    string line {};
    vector<string> lineContents {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Read failed as control file is not open: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    do {
        getline(m_readBuffer, line);
        if (line.empty()) {
            return NAS_CTRL_FILE_RET_READ_EOF;
        }
        boost::algorithm::split(lineContents, line, boost::is_any_of(","), boost::token_compress_off);
        if (ValidateEntry(lineContents, line) == NAS_CTRL_FILE_RET_SUCCESS) {
            break;
        }
        line.clear();
        lineContents.clear();
    } while (true);

    HCP_Log(DEBUG, CTL_MOD_NAME) << line << HCPENDLOG;
    TranslateEntry(lineContents, mtimeEntry);

    return NAS_CTRL_FILE_RET_SUCCESS;
}

void BackupMtimeCtrl::TranslateEntry(vector<string> &lineContents, BackupMtimeCtrlEntry &mtimeEntry)
{
    uint32_t offset = NAS_CTRL_FILE_OFFSET_1;
    uint32_t totCommaCnt = (uint32_t) atoi(lineContents[NAS_CTRL_FILE_OFFSET_0].c_str());

    mtimeEntry.m_absPath = ConstructStringName(offset, totCommaCnt, lineContents);
	#ifndef WIN32
    mtimeEntry.m_mode = (mode_t) atoi(lineContents[offset++].c_str());
	#endif
    mtimeEntry.m_attr = (uint32_t) atoi(lineContents[offset++].c_str());
    mtimeEntry.m_uid = (uint32_t) atoi(lineContents[offset++].c_str());
    mtimeEntry.m_gid = (uint32_t) atoi(lineContents[offset++].c_str());
    mtimeEntry.m_ctime = (uint64_t) stoll(lineContents[offset++]);
    mtimeEntry.m_atime = (uint64_t) stoll(lineContents[offset++]);
    mtimeEntry.m_mtime = (uint64_t) stoll(lineContents[offset++]);
    mtimeEntry.m_btime = (uint64_t) stoll(lineContents[offset]);
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::WriteEntry(BackupMtimeCtrlEntry &mtimeEntry)
{
    string ctlFileLine {};
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = GetCommaCountOfString(mtimeEntry.m_absPath);

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as control file is not open: " << m_ctlFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    ctlFileLine = to_string(commaCount) + "," +
                mtimeEntry.m_absPath + "," +
				#ifndef WIN32
                to_string(mtimeEntry.m_mode) + "," +
				#endif
                to_string(mtimeEntry.m_attr) + "," +
                to_string(mtimeEntry.m_uid) + "," +
                to_string(mtimeEntry.m_gid) + "," +
                to_string(mtimeEntry.m_ctime) + "," +
                to_string(mtimeEntry.m_atime) + "," +
                to_string(mtimeEntry.m_mtime) + "," +
                to_string(mtimeEntry.m_btime) + "\n";

    m_writeBuffer << ctlFileLine;
    m_entries++;
    m_header.stats.noOfDirs++;

    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize > m_maxFileSize || m_entries > m_maxEntryPerFile) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < NAS_CTRL_FILE_NUMBER_TEN) {
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

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TITLE)) {
        m_header.title = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_HEADER_VERSION)) {
        m_header.version = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TIMESTAMP)) {
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_ctlFileName
                << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKID)) {
        m_header.taskId = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKTYPE)) {
        m_header.backupType = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSERVER)) {
        m_header.nasServer = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSHARE)) {
        m_header.nasSharePath = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL)) {
        m_header.proto = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL_VERSION)) {
        m_header.protoVersion = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == static_cast<uint32_t>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_DIR_MTIME_COUNT)) {
        m_header.stats.noOfDirs = (uint64_t) atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_ctlFileName << HCPENDLOG;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::WriteHeader()
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
        HCP_Log(ERR, CTL_MOD_NAME) << "Write Header for Bkup Mtime Ctrl file Failed: " << m_ctlFileName;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string BackupMtimeCtrl::GetFileHeaderLine(uint32_t headerLine)
{
    string ctlHeaderLine {};
    switch (headerLine) {
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TITLE):
            ctlHeaderLine = "Title:" + m_header.title + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_HEADER_VERSION):
            ctlHeaderLine = "Version:" + m_header.version + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TIMESTAMP):
            ctlHeaderLine = "Timestamp:" + m_header.timestamp + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKID):
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_TASKTYPE):
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSERVER):
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_NASSHARE):
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL):
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_PROTOCOL_VERSION):
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case static_cast<int>(MTIME_CTRL_HEADER_INFO::MTIME_CTRL_DIR_MTIME_COUNT):
            ctlHeaderLine = "DirectoryMtimeSetCount:" + to_string(m_header.stats.noOfDirs) + "\n";
            break;
        default:
            HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE BackupMtimeCtrl::ValidateHeader()
{
    return NAS_CTRL_FILE_RET_SUCCESS;
}

uint32_t BackupMtimeCtrl::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

std::string BackupMtimeCtrl::GetCtrlFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_ctlFileName;
}
