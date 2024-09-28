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
#include <random>
#include "ScannerBackupMeta.h"

using namespace std;
using namespace NasScanner;

namespace {
    constexpr int MP_SUCCESS = 0;
    constexpr int MP_FAILED = -1;
}

ScannerBackupMeta::ScannerBackupMeta(ScannerMetaCtrlParams params)
{
    m_metaFileName = params.m_fileName;
    m_metaFileParentDir = GetParentDirOfFile(m_metaFileName);

    m_header.title = NAS_SCANNERBACKUPMETA_HEADER_TITLE;
    m_header.version = NAS_SCANNERBACKUPMETA_HEADER_VERSION;
    m_header.timestamp = to_simple_string(boost::posix_time::second_clock::local_time());
    m_header.taskId = params.taskId;
    m_header.nasServer = params.nasServer;
    m_header.nasSharePath = params.nasSharePath;
    m_header.proto = params.proto;
    m_header.protoVersion = params.protoVersion;
    m_header.backupType = params.backupType;
    m_header.metaDataScope = params.metaDataScope;
}

ScannerBackupMeta::ScannerBackupMeta(string metaFileName)
{
    m_metaFileName = metaFileName;
    m_metaFileParentDir = GetParentDirOfFile(m_metaFileName);
    m_readBufferSize = SCANNER_PATH_LEN_MAX + ACL_MAX_LEN + sizeof(FileMetaReadWrite);
}

ScannerBackupMeta::~ScannerBackupMeta()
{
    if (m_writeFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_WRITE);

    if (m_readFd.is_open())
        Close(NAS_CTRL_FILE_OPEN_MODE_READ);
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::Open(NAS_CTRL_FILE_OPEN_MODE mode)
{
    NAS_CTRL_FILE_RETCODE ret;
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        m_readBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBuffer == nullptr) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Failed to malloc buffer of size " << m_readBufferSize << " file: "
                << m_metaFileName << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        ReadMetaFileHeader();
        if (ValidateHeader() != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Header verification failed for: " << m_metaFileName << HCPENDLOG;
            m_readFd.close();
            return NAS_CTRL_FILE_RET_FAILED;
        }
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::binary | std::ios::app);
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_offset = 0;
        if (m_writeFd.tellp() == 0) {
            ret = WriteHeader();
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_CTRL_FILE_RET_FAILED;
            }
        } else {
            m_offset = (uint64_t) m_writeFd.tellp();
        }
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid open mode for :" << m_metaFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

template<class FileStream>
NAS_CTRL_FILE_RETCODE ScannerBackupMeta::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    strmFd.open(m_metaFileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (CheckParentDirIsReachable(m_metaFileParentDir) != NAS_CTRL_FILE_RET_SUCCESS) {
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_metaFileName << ", Parent dir not reachable";
            return NAS_CTRL_FILE_RET_FAILED;
        }
        strmFd.open(m_metaFileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "Open file failed: " << m_metaFileName << ", ERR: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::Close(NAS_CTRL_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == NAS_CTRL_FILE_OPEN_MODE_WRITE) {
        if (!m_writeFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        NAS_CTRL_FILE_RETCODE ret = FlushToFile();
        if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_writeFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    if (mode == NAS_CTRL_FILE_OPEN_MODE_READ) {
        if (m_readBuffer != nullptr) {
            free(m_readBuffer);
            m_readBuffer = nullptr;
        }
        if (!m_readFd.is_open()) {
            return NAS_CTRL_FILE_RET_SUCCESS;
        }
        m_readFd.close();
        return NAS_CTRL_FILE_RET_SUCCESS;
    }

    HCP_Log(ERR, CTL_MOD_NAME) << "Invalid close mode for :" << m_metaFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_FAILED;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::GetHeader(ScannerMetaCtrlHeader &header)
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_readFd.is_open()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    header = m_header;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadMetaFileHeader()
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    int retryCnt = 0;

    do {
        ret = ReadHeader();
        if (ret == NAS_CTRL_FILE_RET_SUCCESS) {
            break;
        }
        m_readFd.sync();
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed. readFD is not good file: "
                << m_metaFileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.close();
            ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
            if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
                return NAS_CTRL_FILE_RET_FAILED;
            }
        } else {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed. readFD is good file: "
                << m_metaFileName << " retry count: " << retryCnt << HCPENDLOG;
            m_readFd.seekg(0);
        }
        retryCnt++;
        uint32_t sleepDuration = GetRandomNumber(NAS_HDR_RETRY_SLEEP_DUR_MIN_MSEC,
            NAS_HDR_RETRY_SLEEP_DUR_MAX_MSEC);
        std::this_thread::sleep_for(std::chrono::milliseconds(sleepDuration));
    } while (retryCnt < NAS_CTRL_FILE_SERVER_RETRY_CNT);
    return ret;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadHeader()
{
    uint32_t headerLine = 0;
    vector<string> cltHeaderLineSplit {};
    if (!m_readFd) {
        HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader readFd not proper: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    while (headerLine < NAS_CTRL_FILE_NUMBER_TEN) {
        string cltHeaderLine {};
        cltHeaderLineSplit.clear();
        if (!getline(m_readFd, cltHeaderLine)) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader (getline) failed: " << m_metaFileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (cltHeaderLine.empty()) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed, incomplete header: " << m_metaFileName
                << " Line Num: " << headerLine << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        boost::algorithm::split(cltHeaderLineSplit, cltHeaderLine, boost::is_any_of(":"), boost::token_compress_on);
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_TWO) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed: " << m_metaFileName << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (FillHeader(headerLine, cltHeaderLineSplit, cltHeaderLine) != NAS_CTRL_FILE_RET_SUCCESS) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        headerLine++;
    }

    string blankLine {};
    getline(m_readFd, blankLine); /* To skip the blank line after header */
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::FillHeader(uint32_t &headerLine, vector<string> &cltHeaderLineSplit,
    string &cltHeaderLine)
{
    if (headerLine == META_TITLE) {
        m_header.title = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_HEADER_VERSION) {
        m_header.version = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_TIMESTAMP) {
        if (cltHeaderLineSplit.size() < NAS_CTRL_FILE_NUMBER_FOUR) {
            HCP_Log(ERR, CTL_MOD_NAME) << "ReadHeader failed timestamp failed: " << m_metaFileName
                << " line: " << cltHeaderLine;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        m_header.timestamp = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE] + ":" +
            cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_TWO] + ":" + cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_THREE];
    } else if (headerLine == META_TASKID) {
        m_header.taskId = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_TASKTYPE) {
        m_header.backupType = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_NASSERVER) {
        m_header.nasServer = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_NASSHARE) {
        m_header.nasSharePath = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_PROTOCOL) {
        m_header.proto = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_PROTOCOL_VERSION) {
        m_header.protoVersion = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else if (headerLine == META_METADATA_SCOPE) {
        m_header.metaDataScope = cltHeaderLineSplit[NAS_CTRL_FILE_NUMBER_ONE];
    } else {
        HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. headerLine: " << headerLine
            << " cltHeaderLine: " << cltHeaderLine << " fileName: " << m_metaFileName << HCPENDLOG;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::WriteHeader()
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
    m_offset = (uint64_t) headerBuffer.tellp();
    WRITE_TO_FILE(headerBuffer, m_writeFd, NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Write Header For Meta Failed: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    m_writeFd.flush();
    HCP_Log(INFO, CTL_MOD_NAME) << "Write Header completed for metafile: " << m_metaFileName << HCPENDLOG;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

string ScannerBackupMeta::GetFileHeaderLine(uint32_t headerLine)
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
        case META_TASKID:
            ctlHeaderLine = "TaskId:" + m_header.taskId + "\n";
            break;
        case META_TASKTYPE:
            ctlHeaderLine = "BackupType:" + m_header.backupType + "\n";
            break;
        case META_NASSERVER:
            ctlHeaderLine = "NasServer:" + m_header.nasServer + "\n";
            break;
        case META_NASSHARE:
            ctlHeaderLine = "NasShare:" + m_header.nasSharePath + "\n";
            break;
        case META_PROTOCOL:
            ctlHeaderLine = "NasProtocol:" + m_header.proto + "\n";
            break;
        case META_PROTOCOL_VERSION:
            ctlHeaderLine = "NasProtocolVersion:" + m_header.protoVersion + "\n";
            break;
        case META_METADATA_SCOPE:
            ctlHeaderLine = "MetadataScope:" + m_header.metaDataScope + "\n";
            break;
        default:
            HCP_Log(INFO, CTL_MOD_NAME) << "No of values for header exceeded. Line: " << HCPENDLOG;
            break;
    }
    return ctlHeaderLine;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ValidateHeader()
{
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ValidateMetaFile(uint64_t offset, uint16_t readLen)
{
    if (!m_readFd.is_open()) {
        HCP_Log(ERR, CTL_MOD_NAME) << "metaFile given is not open" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    m_readFd.seekg(offset);
    if (m_readFd.fail()) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(WARN, CTL_MOD_NAME) << "failed to seek in metafile"
            " to offset " << offset << " error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (readLen <= 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "readLen is invalid " << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadDirMetaFromBuffer(DirectoryMeta &dirMeta,
    uint64_t offset, uint16_t readLen)
{
    DirectoryMetaReadWrite dirMetaRw {};
    char *bufferCpy = nullptr;
    char name[SCANNER_PATH_LEN_MAX];
    int32_t aclOffset = 0;
    bufferCpy = m_readBuffer;
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    if (memcpy_s(&dirMetaRw, sizeof(DirectoryMetaReadWrite), bufferCpy, sizeof(DirectoryMetaReadWrite)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    bufferCpy += sizeof(DirectoryMetaReadWrite);

    if ((dirMetaRw.m_pathLen <= 0) || (dirMetaRw.m_pathLen > SCANNER_PATH_LEN_MAX)) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Invalid string length: " << dirMetaRw.m_pathLen
            << " offset: " << offset << " readLen: " << readLen <<
            " pathlen: " << dirMetaRw.m_pathLen << " inode: " <<
            dirMetaRw.m_inode << " buffer: " << m_readBuffer << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if (memcpy_s(&name, SCANNER_PATH_LEN_MAX, bufferCpy, dirMetaRw.m_pathLen) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    bufferCpy += dirMetaRw.m_pathLen;
    aclOffset = static_cast<int32_t>(sizeof(DirectoryMetaReadWrite)) + static_cast<int32_t>(dirMetaRw.m_pathLen);

    TranslateDirMeta(dirMeta, dirMetaRw, METARW_TO_META);
    dirMeta.m_path.assign(reinterpret_cast<char *>(&name), dirMetaRw.m_pathLen);

    ret = ReadAcl(offset, dirMeta, readLen, aclOffset);
    if (ret != MP_SUCCESS) {
        return ret;
    }
    ret = CheckDirMetaValidity(dirMeta);
    if (ret == MP_FAILED) {
        HCP_Log(ERR, CTL_MOD_NAME) << "DirMeta received from file is invalid offset: "
            << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadDirectoryMeta(DirectoryMeta &dirMeta, uint16_t readLen, uint64_t offset)
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    ret = ValidateMetaFile(offset, readLen);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Validate directory meta failed filename: " <<
            m_metaFileName<<" readlen " <<
            readLen << " offset: " << offset << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (readLen > m_readBufferSize) {
        m_readBufferSize = readLen;
        if (m_readBuffer != nullptr) {
            free (m_readBuffer);
        }
        m_readBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBuffer == nullptr) {
            HCP_Log(ERR, CTL_MOD_NAME) << "failed to malloc buffer of size " << m_readBufferSize << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    if (memset_s(m_readBuffer, m_readBufferSize, 0, m_readBufferSize) != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to memset buffer of size " << m_readBufferSize << HCPENDLOG;
        ret = NAS_CTRL_FILE_RET_FAILED;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    m_readFd.seekg(offset);
    READ_FROM_BINARY_FILE(m_readBuffer, offset, readLen, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    ret = ReadDirMetaFromBuffer(dirMeta, offset, readLen);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read DirMeta from buffer of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " offset: " << offset
            << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return ret;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadAcl(uint64_t offset, DirectoryMeta &dirMeta,
    uint16_t readLen, int32_t aclOffset)
{
    char aclText[ACL_MAX_LEN];
    char *bufferCpy = m_readBuffer;
    bufferCpy += aclOffset;
    int ret = memset_s(aclText, ACL_MAX_LEN, 0, ACL_MAX_LEN);
    if (ret != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "aclText memset_s failed " << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (dirMeta.m_aclSize > 0) {
        if (dirMeta.m_aclSize > ACL_MAX_LEN) {
            HCP_Log(ERR, CTL_MOD_NAME) << "AclSize out of bounds offset: "
                << offset << " readLen: " << readLen << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (memcpy_s(&aclText, dirMeta.m_aclSize, bufferCpy, dirMeta.m_aclSize) != 0) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        std::string strAcl(aclText);
        dirMeta.m_aclText = strAcl;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::CheckDirMetaValidity(DirectoryMeta &dirMeta)
{
    if (dirMeta.m_mtime == 0 && dirMeta.m_path != ".") {
        return NAS_CTRL_FILE_RET_FAILED;
    } else {
        return NAS_CTRL_FILE_RET_SUCCESS;
    }
}

uint16_t ScannerBackupMeta::WriteDirectoryMeta(DirectoryMeta &dirMeta)
{
    DirectoryMetaReadWrite dirMetaRw {};
    uint16_t metaLength = 0;
    lock_guard<std::mutex> lk(m_lock);

    TranslateDirMeta(dirMeta, dirMetaRw, META_TO_METARW);
    m_writeBuffer.write(reinterpret_cast<char *>(&dirMetaRw), sizeof(dirMetaRw));
    m_writeBuffer << dirMeta.m_path.c_str();
    if (dirMeta.m_aclSize > 0) {
        m_writeBuffer << dirMeta.m_aclText.c_str();
    }
    metaLength = sizeof(dirMetaRw) + dirMeta.m_aclSize + dirMetaRw.m_pathLen;
    m_offset += metaLength;
    return metaLength;
}

uint16_t ScannerBackupMeta::WriteFileMeta(FileMeta &fMeta)
{
    FileMetaReadWrite fMetaRw {};
    uint16_t metaLength = 0;
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    ret = TranslateFileMeta(fMeta, fMetaRw, META_TO_METARW);
    if (ret != MP_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Failed to translate fMeta" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    m_writeBuffer.write(reinterpret_cast<char *>(&fMetaRw), sizeof(fMetaRw));
    m_writeBuffer << fMeta.m_name.c_str();
    if (fMeta.m_aclSize > 0) {
        m_writeBuffer << fMeta.m_aclText.c_str();
    }
    metaLength = sizeof(fMetaRw) + fMetaRw.m_aclSize + fMetaRw.m_nameLen;
    m_offset += metaLength;
    return metaLength;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFileMetaFromBuffer(FileMeta &fMeta,
    uint64_t offset, uint16_t readLen)
{
    FileMetaReadWrite fMetaRw {};
    char *bufferCpy = m_readBuffer;
    int32_t aclOffset = 0;
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    char name[SCANNER_FILE_NAME_LEN_MAX];
    if (memcpy_s(&fMetaRw, sizeof(FileMetaReadWrite), bufferCpy, sizeof(FileMetaReadWrite)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << "offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    bufferCpy += sizeof(FileMetaReadWrite);

    if ((fMetaRw.m_nameLen <= 0) || (fMetaRw.m_nameLen > SCANNER_FILE_NAME_LEN_MAX)) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Invalid string length: " << fMetaRw.m_nameLen
            << " offset: " << offset << " readLen: " << readLen <<
            " inode: " << fMetaRw.m_inode << " metafname: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if (memcpy_s(&name, SCANNER_FILE_NAME_LEN_MAX, bufferCpy, fMetaRw.m_nameLen) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    bufferCpy += fMetaRw.m_nameLen;
    ret = TranslateFileMeta(fMeta, fMetaRw, METARW_TO_META);
    if (ret != MP_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Failed to translate fmeta" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    fMeta.m_name.assign(reinterpret_cast<char *>(&name), fMetaRw.m_nameLen);
    aclOffset = (int32_t) sizeof(FileMetaReadWrite) + (int32_t) fMetaRw.m_nameLen;
    ret = ReadAcl(0, fMeta, fMetaRw.m_aclSize, aclOffset);
    if (ret != MP_SUCCESS) {
        return ret;
    }
    ret = CheckFileMetaValidity(fMeta);
    if (ret == MP_FAILED) {
        HCP_Log(ERR, CTL_MOD_NAME) << "FileMeta received from file is invalid offset: "
            << offset << " readLen: " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFileMeta(FileMeta &fMeta, uint16_t readLen, uint64_t offset)
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    lock_guard<std::mutex> lk(m_lock);

    ret = ValidateMetaFile(offset, readLen);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Validate file meta failed filename: "<< m_metaFileName<<" readlen " <<
            readLen << " offset: " << offset << HCPENDLOG;
        return ret;
    }
    if (readLen > m_readBufferSize) {
        m_readBufferSize = readLen;
        if (m_readBuffer != nullptr) {
            free (m_readBuffer);
        }
        m_readBuffer = (char *)malloc(m_readBufferSize);
        if (m_readBuffer == nullptr) {
            HCP_Log(ERR, CTL_MOD_NAME) << "failed to malloc buffer of size " << m_readBufferSize << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
    }
    if (memset_s(m_readBuffer, m_readBufferSize, 0, m_readBufferSize) != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to memset buffer of size " << readLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    READ_FROM_BINARY_FILE(m_readBuffer, offset, readLen, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read metafile of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << "offset: " << offset
            << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    ret = ReadFileMetaFromBuffer(fMeta, offset, readLen);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read File Meta from buffer of length " << readLen << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << "offset: " << offset
            << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return ret;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadAcl(uint64_t offset, FileMeta &fMeta,
    uint16_t readLen, int32_t aclOffset)
{
    char aclText[ACL_MAX_LEN];
    char *bufferCpy = m_readBuffer;
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    bufferCpy += aclOffset;
    int retCode = memset_s(aclText, ACL_MAX_LEN, 0, ACL_MAX_LEN);
    if (retCode != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "aclText memset_s failed" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (fMeta.m_aclSize > 0) {
        if (fMeta.m_aclSize > ACL_MAX_LEN) {
            HCP_Log(ERR, CTL_MOD_NAME) << "AclSize out of bounds offset: "
                << offset << " readLen: " << readLen << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        if (memcpy_s(&aclText, fMeta.m_aclSize, bufferCpy, fMeta.m_aclSize) != 0) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
                << " offset: " << offset << " readLen: " << readLen << HCPENDLOG;
            return NAS_CTRL_FILE_RET_FAILED;
        }
        std::string strAcl(aclText);
        fMeta.m_aclText = strAcl;
    }

    return ret;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::CheckFileMetaValidity(FileMeta &fMeta)
{
    if (fMeta.m_ctime == 0) {
        return NAS_CTRL_FILE_RET_FAILED;
    } else {
        return NAS_CTRL_FILE_RET_SUCCESS;
    }
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::TranslateDirMeta(DirectoryMeta &dirMeta,
    DirectoryMetaReadWrite &dirMetaRw, int32_t option)
{
    if (option == META_TO_METARW) {
        dirMetaRw.m_mode = dirMeta.m_mode;
        dirMetaRw.m_attr = dirMeta.m_attr;
        dirMetaRw.m_uid = dirMeta.m_uid;
        dirMetaRw.m_gid = dirMeta.m_gid;
        dirMetaRw.m_size = dirMeta.m_size;
        dirMetaRw.m_inode = dirMeta.m_inode;
        dirMetaRw.m_atime = dirMeta.m_atime;
        dirMetaRw.m_mtime = dirMeta.m_mtime;
        dirMetaRw.m_ctime = dirMeta.m_ctime;
        dirMetaRw.m_btime = dirMeta.m_btime;
        dirMetaRw.m_aclSize = dirMeta.m_aclSize;
        dirMetaRw.m_pathLen = dirMeta.m_path.length();
    } else {
        dirMeta.m_mode = dirMetaRw.m_mode;
        dirMeta.m_attr = dirMetaRw.m_attr;
        dirMeta.m_uid = dirMetaRw.m_uid;
        dirMeta.m_gid = dirMetaRw.m_gid;
        dirMeta.m_size = dirMetaRw.m_size;
        dirMeta.m_inode = dirMetaRw.m_inode;
        dirMeta.m_atime = dirMetaRw.m_atime;
        dirMeta.m_mtime = dirMetaRw.m_mtime;
        dirMeta.m_ctime = dirMetaRw.m_ctime;
        dirMeta.m_btime = dirMetaRw.m_btime;
        dirMeta.m_aclSize = dirMetaRw.m_aclSize;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::TranslateFileMeta(FileMeta &fMeta, FileMetaReadWrite &fMetaRw, int32_t option)
{
    if (option == META_TO_METARW) {
        fMetaRw.m_mode = fMeta.m_mode;
        fMetaRw.m_attr = fMeta.m_attr;
        fMetaRw.m_nlink = fMeta.m_nlink;
        fMetaRw.m_uid = fMeta.m_uid;
        fMetaRw.m_gid = fMeta.m_gid;
        fMetaRw.m_size = fMeta.m_size;
        fMetaRw.m_inode = fMeta.m_inode;
        fMetaRw.m_rdev = fMeta.m_rdev;
        fMetaRw.m_mtime = fMeta.m_mtime;
        fMetaRw.m_ctime = fMeta.m_ctime;
        fMetaRw.m_atime = fMeta.m_atime;
        fMetaRw.m_btime = fMeta.m_btime;
        fMetaRw.m_aclSize = fMeta.m_aclSize;
        fMetaRw.m_nameLen = fMeta.m_name.length();
#ifdef _NAS
        /* m_fh is only used to cache nfs filehandle in Nfs Scanner */
        fMetaRw.m_fh.len = fMeta.m_fh.len;
        if (fMetaRw.m_fh.len > 0) {
            if (memcpy_s(fMetaRw.m_fh.value, fMetaRw.m_fh.len, fMeta.m_fh.value, fMetaRw.m_fh.len) != 0) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Memcpy failed" << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
        }
#endif
    } else {
        fMeta.m_mode = fMetaRw.m_mode;
        fMeta.m_attr = fMetaRw.m_attr;
        fMeta.m_nlink = fMetaRw.m_nlink;
        fMeta.m_uid = fMetaRw.m_uid;
        fMeta.m_gid = fMetaRw.m_gid;
        fMeta.m_size = fMetaRw.m_size;
        fMeta.m_inode = fMetaRw.m_inode;
        fMeta.m_rdev = fMetaRw.m_rdev;
        fMeta.m_mtime = fMetaRw.m_mtime;
        fMeta.m_ctime = fMetaRw.m_ctime;
        fMeta.m_atime = fMetaRw.m_atime;
        fMeta.m_btime = fMetaRw.m_btime;
        fMeta.m_aclSize = fMetaRw.m_aclSize;
#ifdef _NAS
        /* m_fh is only used to cache nfs filehandle in Nfs Scanner */
        fMeta.m_fh.len = fMetaRw.m_fh.len;
        if (fMeta.m_fh.len > 0) {
            if (memcpy_s(fMeta.m_fh.value, fMeta.m_fh.len, fMetaRw.m_fh.value, fMeta.m_fh.len) != 0) {
                HCP_Log(ERR, CTL_MOD_NAME) << "Memcpy failed" << HCPENDLOG;
                return NAS_CTRL_FILE_RET_FAILED;
            }
        }
#endif
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

uint64_t ScannerBackupMeta::GetCurrentOffset()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_offset;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::FlushToFile()
{
    NAS_CTRL_FILE_RETCODE ret;
    WRITE_TO_FILE(m_writeBuffer, m_writeFd, NAS_CTRL_BINARY_FILE, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    m_writeBuffer.str("");
    m_writeBuffer.clear();
    m_writeFd.flush();
    HCP_Log(INFO, CTL_MOD_NAME) << "Metafile flush data completed filename: " << m_metaFileName
        << " offset: " << m_writeFd.tellp() << " offset variable: " << m_offset << HCPENDLOG;
    return NAS_CTRL_FILE_RET_SUCCESS;
}

std::string ScannerBackupMeta::GetFileName()
{
    lock_guard<std::mutex> lk(m_lock);
    return m_metaFileName;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFileMetaFromBuffer(FileMeta &fMeta)
{
    FileMetaReadWrite fMetaRw {};
    char *bufferCpy = m_readBuffer;
    int len = 0;
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    char name[SCANNER_FILE_NAME_LEN_MAX];
    if (memcpy_s(&fMetaRw, sizeof(FileMetaReadWrite), bufferCpy, sizeof(FileMetaReadWrite)) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed, ERR: "<< strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    if ((fMetaRw.m_nameLen <= 0) || (fMetaRw.m_nameLen > SCANNER_FILE_NAME_LEN_MAX)) {
        HCP_Log(DEBUG, CTL_MOD_NAME) << "Invalid string length: " << fMetaRw.m_nameLen
            << " inode: " << fMetaRw.m_inode << " metafname: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    len = fMetaRw.m_nameLen + fMetaRw.m_aclSize;
    if (ReadFromFile(len) != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    bufferCpy = m_readBuffer;
    if (memcpy_s(&name, SCANNER_FILE_NAME_LEN_MAX, bufferCpy, fMetaRw.m_nameLen) != 0) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "memcpy failed  error= " << strerror_r(errno, errMsg, ERROR_MSG_SIZE)
            << " readLen: " << fMetaRw.m_nameLen << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }

    bufferCpy += sizeof(FileMetaReadWrite);

    ret = TranslateFileMeta(fMeta, fMetaRw, METARW_TO_META);
    if (ret != MP_SUCCESS) {
        HCP_Log(ERR, CTL_MOD_NAME) << "Failed to translate fmeta" << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    fMeta.m_name.assign(reinterpret_cast<char *>(&name), fMetaRw.m_nameLen);
    ret = ReadAcl(0, fMeta, fMetaRw.m_aclSize, fMetaRw.m_nameLen);
    if (ret != MP_SUCCESS) {
        return ret;
    }
    ret = CheckFileMetaValidity(fMeta);
    if (ret == MP_FAILED) {
        HCP_Log(ERR, CTL_MOD_NAME) << "FileMeta received from file is invalid " << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFromFile(int len)
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    if (memset_s(m_readBuffer, m_readBufferSize, 0, m_readBufferSize) != 0) {
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to memset buffer of size " << len << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    READ_FROM_BINARY_FILE(m_readBuffer, m_readFd.tellg(), len, m_readFd, ret);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(ERR, CTL_MOD_NAME) << "failed to read metafile of length " << len << " error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return NAS_CTRL_FILE_RET_SUCCESS;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFileMeta(FileMeta &fMeta)
{
    NAS_CTRL_FILE_RETCODE ret = NAS_CTRL_FILE_RET_SUCCESS;
    lock_guard<std::mutex> lk(m_lock);
    if (ReadFromFile(sizeof(FileMetaReadWrite)) != NAS_CTRL_FILE_RET_SUCCESS) {
        return NAS_CTRL_FILE_RET_FAILED;
    }
    if (m_readFd.eof()) {
        return NAS_CTRL_FILE_RET_FAILED;
    }

    ret = ReadFileMetaFromBuffer(fMeta);
    if (ret != NAS_CTRL_FILE_RET_SUCCESS) {
        char errMsg[ERROR_MSG_SIZE];
        HCP_Log(DEBUG, CTL_MOD_NAME) << "failed to read File Meta from buffer error= "
            << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << " metafile: " << m_metaFileName << HCPENDLOG;
        return NAS_CTRL_FILE_RET_FAILED;
    }
    return ret;
}

NAS_CTRL_FILE_RETCODE ScannerBackupMeta::ReadFileMetaEntries(std::queue<FileMeta> &fileMetaQueue, int maxCount)
{
    FileMeta fmeta {};
    int count = 0;
    while (count < maxCount) {
        if (ReadFileMeta(fmeta) == NAS_CTRL_FILE_RET_FAILED) {
            return NAS_CTRL_FILE_RET_FAILED;
        }
        fileMetaQueue.push(fmeta);
        count++;
    }
    return NAS_CTRL_FILE_RET_FAILED;
}

uint32_t ScannerBackupMeta::GetRandomNumber(uint32_t minNum, uint32_t maxNum)
{
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> distrib(minNum, maxNum);
    uint32_t randomNum = (uint32_t)distrib(gen);
    return randomNum;
}
