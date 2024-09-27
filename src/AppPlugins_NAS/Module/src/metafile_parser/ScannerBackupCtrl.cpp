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
#include "ScannerBackupCtrl.h"
using namespace std;

ScannerBackupCtrl::ScannerBackupCtrl(ScannerBackupCtrlParams params)
{
    m_ctlFileName = params.m_ctlFileName;
    m_maxEntryPerFile = params.maxEntriesPerFile;
    m_minEntriesPerFile = params.minEntriesPerFile;
    m_maxDataSize = params.maxDataSize;
    m_minDataSize = params.minDataSize;
    m_ctrlFileTimeElapsed = params.m_ctrlFileTimeElapsed;
    m_maxFileSize = NAS_CTRL_FILE_MAX_SIZE;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);

    m_header.title = NAS_SCANNERBACKUPCTRL_HEADER_TITLE;
    m_header.version = NAS_SCANNERBACKUPCTRL_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

ScannerBackupCtrl::ScannerBackupCtrl(std::string ctlFileName)
{
    m_ctlFileName = ctlFileName;
    m_ctrlFileParentDir = GetParentDirOfFile(m_ctlFileName);
}

ScannerBackupCtrl::~ScannerBackupCtrl()
{
    if (m_writeFd.is_open()) {
        // HCP_Log(DEBUG, CTL_MOD_NAME) << "Close called from  Destructor" << HCPENDLOG;
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);
    }
    if (m_readFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::Open(NAS_CTRL_FILE_OPEN_MODE mode)
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
            // HCP_Log(ERR, CTL_MOD_NAME) << "Read from stream is failed for " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ReadHeader();
        if (ValidateHeader() != NAS_CTRL_FILE_RET_SUCCESS) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "Header verification failed for " << m_ctlFileName << HCPENDLOG;
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

    // HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for :" << m_ctlFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    strmFd.open(m_ctlFileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(m_ctrlFileParentDir) != NAS_CTRL_FILE_RET_SUCCESS) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_ctlFileName << ", Parent dir not reachable";
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(m_ctlFileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            // HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_ctlFileName << ", ERR: "
                // << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::Close(NAS_CTRL_FILE_OPEN_MODE mode)
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
        m_writeBuffer << m_writeFileBuffer.str();
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

    // HCP_Log(ERR, CTL_MOD_NAME) << "Invalid close mode for :" << m_ctlFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::GetHeader(ScannerBackupCtrlHeader &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ReadEntry(ScannerBackupCtrlFileEntry &fileEntry,
    ScannerBackupCtrlDirEntry &dirEntry)
{
    string ctlFileLine {};
    vector<string> ctlFileLineSplit {};
    lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        // HCP_Log(ERR, CTL_MOD_NAME) << "Read failed as control file is not open: " << m_ctlFileName << HCPENDLOG;
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

    if (ctlFileLineSplit[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERBACKUPCTRL_ENTRY_TYPE_DIR) {
        TranslateDirEntry(ctlFileLineSplit, dirEntry);
    } else {
        TranslateFileEntry(ctlFileLineSplit, fileEntry);
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::WriteDirEntry(ScannerBackupCtrlDirEntry &dirEntry)
{
    string ctlFileLine {};
    lock_guard<std::mutex> lk(m_lock);
    dirEntry.m_fileCount = m_fileCount;
    uint32_t commaCount = GetCommaCountOfString(dirEntry.m_dirName);

    if (!m_writeFd.is_open()) {
        // HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as control file is not open: " << m_ctlFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if (dirEntry.m_mode != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        ctlFileLine = "d," +
                    dirEntry.m_mode + "," +
                    to_string(commaCount) + "," +
                    dirEntry.m_dirName + "," +
                    to_string(dirEntry.m_metaFileIndex) + "," +
                    to_string(dirEntry.metaFileOffset) + "," +
                    to_string(dirEntry.metaFileReadLen) + "," +
                    to_string(dirEntry.m_fileCount) + "," +
                    to_string(dirEntry.m_aclFlag) + "\n";
    } else {
        ctlFileLine = "d," +
                    dirEntry.m_mode + "," +
                    to_string(commaCount) + "," +
                    dirEntry.m_dirName + "," +
                    to_string(dirEntry.m_fileCount) + "\n";
    }
    m_writeBuffer << ctlFileLine;
    m_entries++;
    m_header.stats.noOfDirs++;
    m_writeBuffer << m_writeFileBuffer.str(); // Write and Append the m_writeFileBuffer  to the m_writeBuffer
    m_writeFileBuffer.str("");
    m_writeFileBuffer.clear();  // Once m_writeFileBuffer is written to m_writeBuffer, clear it .
    m_fileCount = 0;
    uint32_t bufSize = (uint32_t) m_writeBuffer.tellp();
    if (bufSize > m_maxFileSize || m_entries > m_maxEntryPerFile  || m_dataSize > m_maxDataSize) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::WriteFileEntry(ScannerBackupCtrlFileEntry &fileEntry)
{
    string ctlFileLine {};
    lock_guard<std::mutex> lk(m_lock);
    uint32_t commaCount = GetCommaCountOfString(fileEntry.m_fileName);

    if (!m_writeFd.is_open()) {
        // HCP_Log(ERR, CTL_MOD_NAME) << "Write failed as control file is not open: " << m_ctlFileName <<  HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if (fileEntry.m_mode != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        ctlFileLine = "f," +
                    fileEntry.m_mode + "," +
                    to_string(commaCount) + "," +
                    fileEntry.m_fileName + "," +
                    to_string(fileEntry.m_metaFileIndex) + "," +
                    to_string(fileEntry.metaFileOffset) + "," +
                    to_string(fileEntry.metaFileReadLen) + "," +
                    to_string(fileEntry.m_aclFlag) + "\n";
        if (fileEntry.m_mode != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_META_MODIFIED) {
            m_dataSize += fileEntry.m_fileSize; // Scanner to provide fileSize
            m_header.stats.dataSize += fileEntry.m_fileSize;
        }
    } else {
        ctlFileLine = "f," +
                    fileEntry.m_mode + "," +
                    to_string(commaCount) + "," +
                    fileEntry.m_fileName + "\n";
    }
    m_writeFileBuffer << ctlFileLine;
    m_entries++;
    m_header.stats.noOfFiles++;
    m_fileCount++;

    uint64_t bufSize = (uint64_t) m_writeFileBuffer.tellp() + (uint64_t) m_writeBuffer.tellp();
    if (bufSize > m_maxFileSize || m_entries > m_maxEntryPerFile  || m_dataSize > m_maxDataSize) {
        // The caller has to close this file and create a new file
        return NAS_CTRL_FILE_RET_MAX_LIMIT_REACHED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};

    while (headerLine < NAS_CTRL_FILE_NUMBER_THIRTEEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readBuffer, cltHeaderLine)) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (cltHeaderLine.empty()) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_ctlFileName << " line: " << cltHeaderLine;
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
NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::FillHeader2(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    switch (headerLine) {
        case METADATA_SCOPE :
            m_header.metaDataScope = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case DIR_COUNT :
            m_header.stats.noOfDirs = (uint64_t)atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        case FILE_COUNT :
            m_header.stats.noOfFiles = (uint64_t)atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        case DATA_SIZE :
            m_header.stats.dataSize = (uint64_t)atoll((cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE]).c_str());
            break;
        default :
            // HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            // << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_ctlFileName << HCPENDLOG;
            break;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    switch (headerLine) {
        case TITLE :
            m_header.title = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case HEADER_VERSION :
            m_header.version = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case TIMESTAMP :
            if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
                // HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_ctlFileName
                //     << " line: " << cltHeaderLine;
                return NAS_CTRL_FILE_RET_FAILED;
            }
            m_header.timestamp = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
                cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
            break;
        case TASKID :
            m_header.taskId = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case TASKTYPE :
            m_header.backupType = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case NASSERVER :
            m_header.nasServer = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case NASSHARE :
            m_header.nasSharePath = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case CTRL_PROTOCOL :
            m_header.proto = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        case CTRL_PROTOCOL_VERSION :
            m_header.protoVersion = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
            break;
        default :
            FillHeader2(headerLine, cltHeaderLineSplit, cltHeaderLine);
            break;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::WriteHeader()
{
    uint32_t headerLine = 0;
    stringstream headerBuffer {};
    NAS_CTRL_FILE_RETCODE ret;

    while (headerLine < NAS_CTRL_FILE_NUMBER_THIRTEEN) {
        string ctlHeaderLine = GetFileHeaderLine(headerLine);
        headerLine++;
        headerBuffer << ctlHeaderLine;
    }

    headerBuffer << "\n";
    WRITE_TO_FILE(headerBuffer, m_writeFd, !NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        // HCP_Log(ERR, CTL_MOD_NAME) << "Write Header for Ctrl file Failed: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string ScannerBackupCtrl::GetFileHeaderLine(uint32_t headerLine)
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
            // HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ValidateEntry(vector<std::string> &fileContents, std::string &line)
{
    if (fileContents.empty()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if (fileContents[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERBACKUPCTRL_ENTRY_TYPE_DIR) {
        if (ValidateDirEntry(fileContents, line) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else if (fileContents[NAS_CTRL_FILE_NUMBER_ZERO] == NAS_SCANNERBACKUPCTRL_ENTRY_TYPE_FILE) {
        if (ValidateFileEntry(fileContents, line) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else {
        // HCP_Log(ERR, CTL_MOD_NAME) << "Control entry neither file nor dir. Line: " << line
        //     << "File: " << m_ctlFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ValidateDirEntry(vector<std::string> &lineContents, std::string &line)
{
    uint32_t lineContentsVecSize = lineContents.size();
    uint32_t commaCount = (uint32_t)atoi(lineContents[NAS_CTRL_FILE_NUMBER_TWO].c_str());

    if (lineContents[NAS_CTRL_FILE_NUMBER_ONE] == NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_5) {
            // HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for dir has less columns. Line: "
            //     << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (lineContentsVecSize != (commaCount + NAS_CTRL_FILE_OFFSET_5)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for dir incorrect. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else {
        if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_9) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for dir has less columns. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (lineContentsVecSize != (commaCount + NAS_CTRL_FILE_OFFSET_9)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for dir incorrect. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ValidateFileEntry(vector<std::string> &lineContents, std::string &line)
{
    uint32_t lineContentsVecSize = lineContents.size();
    uint32_t commaCount = (uint32_t)atoi(lineContents[NAS_CTRL_FILE_NUMBER_TWO].c_str());

    if (lineContents[NAS_CTRL_FILE_NUMBER_ONE] == NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_4) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file has less columns. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (lineContentsVecSize != (commaCount + NAS_CTRL_FILE_OFFSET_4)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file incorrect. Line: "
                << line << "File: " << m_ctlFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    } else {
        if (m_header.version == NAS_SCANNERBACKUPCTRL_HEADER_VERSION_V10) {
            if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_7) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file has less columns. Line: "
                    << line << "File: " << m_ctlFileName << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
            if (lineContentsVecSize != (commaCount + NAS_CTRL_FILE_OFFSET_7)) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file incorrect. Line: "
                    << line << "File: " << m_ctlFileName << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
        } else {
            if (lineContentsVecSize < NAS_CTRL_FILE_OFFSET_8) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file has less columns. Line: "
                    << line << "File: " << m_ctlFileName << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
            if (lineContentsVecSize != (commaCount + NAS_CTRL_FILE_OFFSET_8)) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Control Entry for file incorrect. Line: "
                    << line << "File: " << m_ctlFileName << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
        }
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupCtrl::ValidateHeader()
{
    return NAS_CTRL_FILE_RET_SUCCESS;
}

std::string ScannerBackupCtrl::GetMetaFileName(uint16_t metaFileIndex)
{
    std::string metafileName = "meta_file_" + to_string(metaFileIndex);
    return metafileName;
}


void ScannerBackupCtrl::TranslateDirEntry(vector<string> &fileContents, ScannerBackupCtrlDirEntry &dirEntry)
{
    if (m_header.version == NAS_SCANNERBACKUPCTRL_HEADER_VERSION_V10) {
        TranslateDirEntryV10(fileContents, dirEntry);
        return;
    }
    dirEntry.m_mode = fileContents[NAS_CTRL_FILE_OFFSET_1];
    uint32_t offset = NAS_CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_OFFSET_2].c_str());

    dirEntry.m_dirName = ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[NAS_CTRL_FILE_NUMBER_ONE] != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        dirEntry.m_metaFileIndex = (uint16_t)atoi(fileContents[offset++].c_str());
        dirEntry.m_metaFileName = GetMetaFileName(dirEntry.m_metaFileIndex);
        dirEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        dirEntry.m_fileCount = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
    }
}

void ScannerBackupCtrl::TranslateDirEntryV10(vector<string> &fileContents, ScannerBackupCtrlDirEntry &dirEntry)
{
    dirEntry.m_mode = fileContents[NAS_CTRL_FILE_OFFSET_1];
    uint32_t offset = NAS_CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_OFFSET_2].c_str());

    dirEntry.m_dirName = ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[NAS_CTRL_FILE_NUMBER_ONE] != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        dirEntry.m_metaFileName = fileContents[offset++];
        dirEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        dirEntry.m_fileCount = (uint64_t)atoll((fileContents[offset++]).c_str());
        dirEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
        m_metaFileName = dirEntry.m_metaFileName;
    }
}

void ScannerBackupCtrl::TranslateFileEntry(vector<string> &fileContents, ScannerBackupCtrlFileEntry &fileEntry)
{
    if (m_header.version == NAS_SCANNERBACKUPCTRL_HEADER_VERSION_V10) {
        TranslateFileEntryV10(fileContents, fileEntry);
        return;
    }

    fileEntry.m_mode = fileContents[NAS_CTRL_FILE_OFFSET_1];
    uint32_t offset = NAS_CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_OFFSET_2].c_str());

    fileEntry.m_fileName = ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[NAS_CTRL_FILE_NUMBER_ONE] != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        fileEntry.m_metaFileIndex = (uint16_t)atoi(fileContents[offset++].c_str());
        fileEntry.m_metaFileName = GetMetaFileName(fileEntry.m_metaFileIndex);
        fileEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        fileEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        fileEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
    }
}

void ScannerBackupCtrl::TranslateFileEntryV10(vector<string> &fileContents, ScannerBackupCtrlFileEntry &fileEntry)
{
    fileEntry.m_mode = fileContents[NAS_CTRL_FILE_OFFSET_1];
    uint32_t offset = NAS_CTRL_FILE_OFFSET_3;
    uint32_t totCommaCnt = (uint32_t)atoi(fileContents[NAS_CTRL_FILE_OFFSET_2].c_str());

    fileEntry.m_fileName = ConstructStringName(offset, totCommaCnt, fileContents);
    if (fileContents[NAS_CTRL_FILE_NUMBER_ONE] != NAS_SCANNERBACKUPCTRL_ENTRY_MODE_DATA_DELETED) {
        fileEntry.metaFileOffset = (uint64_t)atoll((fileContents[offset++]).c_str());
        fileEntry.metaFileReadLen = (uint16_t)atoi((fileContents[offset++]).c_str());
        fileEntry.m_aclFlag = (uint32_t)atoi((fileContents[offset]).c_str());
        fileEntry.m_metaFileName = m_metaFileName;
    }
}

bool ScannerBackupCtrl::IsFileBufferEmpty()
{
    lock_guard<std::mutex> lk(m_lock);
    return (m_fileCount == 0);
}

uint32_t ScannerBackupCtrl::GetEntries()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_entries;
}

bool ScannerBackupCtrl::CheckCtrlFileTimeElapse()
{
    lock_guard<std::mutex> lk(m_lock);
    time_t curTime = GetCurrentTimeInSeconds();
    time_t diffTime = (curTime - m_ctrlFileCreationTime);

    if (diffTime >= m_ctrlFileTimeElapsed) {
        if (m_entries == 0) {
            return false;
        }
        if (m_entries < m_minEntriesPerFile && m_dataSize < m_minDataSize &&
            diffTime < NAS_SCANNERBACKUPCTRL_TEN_MIN_IN_SEC) {
            return false;
        }
        // HCP_Log(INFO, CTL_MOD_NAME) << "Control file time elapsed for file: " << m_ctlFileName << HCPENDLOG;
        return true;
    }
    return false;
}
