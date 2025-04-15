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
#include "BackupFailedCtrlParser.h"
#include "Log.h"

using namespace std;
using namespace Module;

BackupFailedCtrlParser::BackupFailedCtrlParser() : FileParser(false)
{}

BackupFailedCtrlParser::~BackupFailedCtrlParser()
{
    if (m_writeFd.is_open()) {
        DBGLOG("close called from Destructor");
        WriteToFile(m_writeBuffer, false);
    }
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::WriteHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::OpenWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::CloseWrite()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::FlushToFile()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::ValidateHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::ReadHeader()
{
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::WriteFailedFile(string fileName)
{
    m_writeBuffer << fileName << "\n";
    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BackupFailedCtrlParser::ReadFailedFile(string& fileName)
{
    if (!m_readFd.is_open()) {
        ERRLOG("Read failed as control file is not open: %s", m_fileName.c_str());
        return CTRL_FILE_RETCODE::FAILED;
    }
    getline(m_readBuffer, fileName);
    if (fileName.empty()) {
        return CTRL_FILE_RETCODE::READ_EOF;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}
