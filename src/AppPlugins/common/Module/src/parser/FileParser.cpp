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
#include "FileParser.h"
#include "Log.h"

using namespace std;
using namespace Module;

namespace {
constexpr auto MODULE = "FILE_PARSER";
}

template<class FileStream>
CTRL_FILE_RETCODE FileParser::FileOpen(FileStream &strmFd, std::ios::openmode fileMode)
{
    DBGLOG("Open file %s", m_fileName.c_str());
    strmFd.open(m_fileName.c_str(), fileMode);
    if (!strmFd.is_open()) {
        if (!ParserUtils::CheckParentDirIsReachable(m_fileParentDir)) {
            HCP_Log(ERR, MODULE) << "Open file failed: " << m_fileName << ", Parent dir not reachable";
            return CTRL_FILE_RETCODE::FAILED;
        }
        strmFd.open(m_fileName.c_str(), fileMode);
        if (!strmFd.is_open()) {
            char errMsg[ERROR_MSG_SIZE];
            HCP_Log(ERR, MODULE) << "Open file failed: " << m_fileName << ", ERR: "
                << strerror_r(errno, errMsg, ERROR_MSG_SIZE) << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileParser::WriteToAggrFile(bool isBinaryFile)
{
    (void)isBinaryFile;
    m_writeFd << m_writeBuffer.str();
    m_writeFd.seekp(0, ios::end);
    if (!m_writeFd.good()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileParser::ReadFromAggrFile()
{
    m_readBuffer << m_readFd.rdbuf();
    if (!m_readBuffer.good()) {
        return CTRL_FILE_RETCODE::FAILED;
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileParser::WriteToFile(std::stringstream& writeBuffer, bool isBinaryFile)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    m_writeFd << writeBuffer.str();
    // DTS2023101812316
    if (!m_writeFd.good()) {
        m_writeFd.close();
        std::ios::openmode fileOpenMode = std::ios::out | std::ios::app;
        if (isBinaryFile) {
            fileOpenMode |= std::ios::binary;
        }
        ret = FileOpen<std::ofstream>(m_writeFd, fileOpenMode);
        if (ret == CTRL_FILE_RETCODE::SUCCESS) {
            m_writeFd << writeBuffer.str();
            m_writeFd.seekp(0, ios::end);
            if (!m_writeFd.good()) {
                m_writeFd.close();
                ret = CTRL_FILE_RETCODE::FAILED;
            }
        }
    }
    return ret;
}

CTRL_FILE_RETCODE FileParser::ReadFromFile()
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    m_readBuffer << m_readFd.rdbuf();
    if (!m_readBuffer.good()) {
        m_readFd.close();
        m_readBuffer.clear(m_readBuffer.goodbit);
        m_readBuffer.str("");
        ret = FileOpen<std::ifstream>(m_readFd, std::ios::in);
        if (ret == CTRL_FILE_RETCODE::SUCCESS) {
            m_readBuffer << m_readFd.rdbuf();
            if (!m_readBuffer.good()) {
                m_readFd.close();
                m_readBuffer.str("");
                ret = CTRL_FILE_RETCODE::FAILED;
            }
        }
    }
    return ret;
}

CTRL_FILE_RETCODE FileParser::ReadFromBinaryFile(uint64_t offset, uint32_t readLen)
{
    CTRL_FILE_RETCODE ret = CTRL_FILE_RETCODE::SUCCESS;
    m_readFd.read(m_readBinaryBuffer, readLen);
    if (m_readFd.eof() && m_readFd.fail()) {
        m_readFd.clear();
        return CTRL_FILE_RETCODE::READ_EOF;
    }
    if ((!m_readFd.eof()) && (!m_readFd.good())) {
        m_readFd.close();
        ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
        if (ret != CTRL_FILE_RETCODE::SUCCESS) {
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (offset != 0) {
            m_readFd.seekg(offset);
            if (m_readFd.fail()) {
                return CTRL_FILE_RETCODE::FAILED;
            }
        }
        m_readFd.read(m_readBinaryBuffer, readLen);
        if ((!m_readFd.eof()) && (!m_readFd.good())) {
            m_readFd.close();
            return CTRL_FILE_RETCODE::FAILED;
        }
    }
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileParser::Open(CTRL_FILE_OPEN_MODE mode)
{
    CTRL_FILE_RETCODE ret;
    lock_guard<std::mutex> lk(m_lock);
    if (mode == CTRL_FILE_OPEN_MODE::READ) {
        if (m_readFd.is_open()) {
            return CTRL_FILE_RETCODE::SUCCESS;
        }
        if (m_binaryFlag) {
            m_readBinaryBuffer = (char *)malloc(m_readBufferSize);
            if (m_readBinaryBuffer == nullptr) {
                HCP_Log(ERR, MODULE) << "Failed to malloc buffer of size " << m_readBufferSize << " file: "
                    << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
            ret = FileOpen<std::ifstream>(m_readFd, std::ios::in | std::ios::binary);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                free(m_readBinaryBuffer);
                m_readBinaryBuffer = nullptr;
                return CTRL_FILE_RETCODE::FAILED;
            }
        } else {
            ret = FileOpen<std::ifstream>(m_readFd, std::ios::in);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
            ret = ReadFromFile();
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Read from stream is failed for " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
        }
        ReadHeader();
        if (ValidateHeader() != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Header verification failed for: " << m_fileName << HCPENDLOG;
            if (m_binaryFlag) {
                free(m_readBinaryBuffer);
                m_readBinaryBuffer = nullptr;
            }
            m_readFd.close();
            return CTRL_FILE_RETCODE::FAILED;
        }
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    if (mode == CTRL_FILE_OPEN_MODE::WRITE) {
        if (m_writeFd.is_open()) {
            return CTRL_FILE_RETCODE::SUCCESS;
        }
        if (m_binaryFlag) {
            ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::binary | std::ios::app);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
            m_writeFd.seekp(0, ios::end);
            return OpenWrite();
        } else {
            m_writeCtrlLine = (char *)malloc(CTRL_WRITE_LINE_SIZE);
            if (m_writeCtrlLine == nullptr) {
                HCP_Log(ERR, MODULE) << "Malloc failed, sz: " <<
                    CTRL_WRITE_LINE_SIZE << "file: " << m_fileName << HCPENDLOG;
                return CTRL_FILE_RETCODE::FAILED;
            }
            ret = FileOpen<std::ofstream>(m_writeFd, std::ios::out | std::ios::app);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                free(m_writeCtrlLine);
                m_writeCtrlLine = nullptr;
                return CTRL_FILE_RETCODE::FAILED;
            }
            m_writeFd.seekp(0, ios::end);
            return OpenWrite();
        }
    }

    HCP_Log(ERR, MODULE) << "Invalid open mode for :" << m_fileName << HCPENDLOG;
    return CTRL_FILE_RETCODE::FAILED;
}

CTRL_FILE_RETCODE FileParser::Close(CTRL_FILE_OPEN_MODE mode)
{
    lock_guard<std::mutex> lk(m_lock);

    if (mode == CTRL_FILE_OPEN_MODE::WRITE) {
        if (m_writeCtrlLine) {
            free(m_writeCtrlLine);
            m_writeCtrlLine = nullptr;
        }
        if (!m_writeFd.is_open()) {
            return CTRL_FILE_RETCODE::SUCCESS;
        }
        if (m_binaryFlag) {
            CTRL_FILE_RETCODE ret = FlushToFile();
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
        } else {
            CTRL_FILE_RETCODE ret = WriteHeader();
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
            ret = CloseWrite();
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
            ret = WriteToFile(m_writeBuffer, m_binaryFlag);
            if (ret != CTRL_FILE_RETCODE::SUCCESS) {
                return CTRL_FILE_RETCODE::FAILED;
            }
        }
        m_writeFd.close();
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    if (mode == CTRL_FILE_OPEN_MODE::READ) {
        if (m_binaryFlag) {
            if (m_readBinaryBuffer) {
                free(m_readBinaryBuffer);
                m_readBinaryBuffer = nullptr;
            }
        }
        if (!m_readFd.is_open()) {
            return CTRL_FILE_RETCODE::SUCCESS;
        }
        m_readFd.close();
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    HCP_Log(ERR, MODULE) << "Invalid close mode for :" << m_fileName << HCPENDLOG;
    return CTRL_FILE_RETCODE::FAILED;
}

void FileParser::CloseWriteFD()
{
    lock_guard<std::mutex> lk(m_lock);
    if (!m_writeFd.is_open()) {
        return;
    }
    m_writeFd.close();
}