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
#ifndef MODULE_FILE_NAME_LIST_PARSER_H
#define MODULE_FILE_NAME_LIST_PARSER_H

#include <string>
#include "FileParser.h"

namespace Module {

class FileNameListParser : public FileParser {
public:
    explicit FileNameListParser(const std::string& fileName) : FileParser(false)
    {
        m_fileName = fileName;
    }

    ~FileNameListParser()
    {
        if (m_writeFd.is_open()) {
            Close(CTRL_FILE_OPEN_MODE::WRITE);
        }
        if (m_readFd.is_open()) {
            Close(CTRL_FILE_OPEN_MODE::READ);
        }
    }

    CTRL_FILE_RETCODE WriteEntry(const std::string& fileName);

    CTRL_FILE_RETCODE ReadEntry(std::string& fileName);

private:
    CTRL_FILE_RETCODE OpenWrite() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    CTRL_FILE_RETCODE CloseWrite() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    CTRL_FILE_RETCODE FlushToFile() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    CTRL_FILE_RETCODE ReadHeader() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    CTRL_FILE_RETCODE ValidateHeader() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    CTRL_FILE_RETCODE WriteHeader() override
    {
        return CTRL_FILE_RETCODE::SUCCESS;
    }

    uint64_t m_entryCount = 0;
};

}

#endif // MODULE_FILE_NAME_LIST_PARSER_H
