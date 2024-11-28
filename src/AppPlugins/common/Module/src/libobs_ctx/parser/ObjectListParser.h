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
#ifndef MODULE_OBJECT_LIST_PARSER_H
#define MODULE_OBJECT_LIST_PARSER_H

#include <string>
#include "FileParser.h"
#include "common/CloudServiceCommonStruct.h"

namespace Module {

struct ObjectListParserParams {
    std::string originalObjectListFilePath; // 写入和读取时都使用
    std::string uniqueObjectListFilePath; // 写入时使用，读取时不使用
};

class ObjectListParser : public FileParser {
public:
    explicit ObjectListParser(const ObjectListParserParams &params)
        : FileParser(false), m_uniqueObjectListFilePath(params.uniqueObjectListFilePath)
    {
        m_fileName = params.originalObjectListFilePath;
    }
    ~ObjectListParser();

    CTRL_FILE_RETCODE WriteEntry(const BucketLogInfo& bucketLogInfo);

    CTRL_FILE_RETCODE ReadEntry(BucketLogInfo& bucketLogInfo);

    CTRL_FILE_RETCODE Convert2UniqueObjectList() const;

private:
    CTRL_FILE_RETCODE ValidateEntry(const std::vector<std::string>& fields);

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

    std::string m_uniqueObjectListFilePath;
    uint64_t m_entryCount = 0;
};

}

#endif // MODULE_OBJECT_LIST_PARSER_H
