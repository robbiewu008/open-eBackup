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
#include "Log.h"
#include "FileNameListParser.h"

using namespace Module;

namespace {
constexpr auto MODULE = "FILENAME_LIST_PARSER";
const int PRINT_COUNT = 10000;
}

CTRL_FILE_RETCODE FileNameListParser::WriteEntry(const std::string& fileName)
{
    std::lock_guard<std::mutex> lk(m_lock);

    if (memset_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, 0, CTRL_WRITE_LINE_SIZE) != 0) {
        return CTRL_FILE_RETCODE::FAILED;
    }

    if (!m_writeFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Write file not open, file name " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    int len = snprintf_s(m_writeCtrlLine, CTRL_WRITE_LINE_SIZE, CTRL_WRITE_LINE_SIZE, "%s\n", fileName.c_str());
    if (len <= 0) {
        HCP_Log(ERR, MODULE) << "Failed to prepare buffer: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    m_writeBuffer << m_writeCtrlLine;

    ++m_entryCount;
    if (m_entryCount % PRINT_COUNT == 0) {
        HCP_Log(INFO, MODULE) << m_fileName << " has written " << m_entryCount << " entries" << HCPENDLOG;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE FileNameListParser::ReadEntry(std::string& fileName)
{
    std::lock_guard<std::mutex> lk(m_lock);

    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read file not open: " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    getline(m_readBuffer, fileName);
    if (fileName.empty()) {
        return CTRL_FILE_RETCODE::READ_EOF;
    }

    ++m_entryCount;
    if (m_entryCount % PRINT_COUNT == 0) {
        HCP_Log(INFO, MODULE) << m_fileName << " has read " << m_entryCount << " entries" << HCPENDLOG;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}