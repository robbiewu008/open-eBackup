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
#ifndef MODULE_BUCKET_LOG_PARSER_H
#define MODULE_BUCKET_LOG_PARSER_H

#include <string>
#include <mutex>
#include <unordered_map>
#include "FileParser.h"
#include "common/CloudServiceCommonStruct.h"
#include "FileNameListParser.h"
#include "ObjectListParser.h"

namespace Module {

struct BucketLogParserParams {
    StorageType storageType = StorageType::OTHER;
    std::vector<std::string> objectPrefixs; // 当前一次解析只解析单个桶的日志，所以前缀也是属于单个桶的
    std::string bucketLogDir;
    std::string originalObjectListDir;
    std::string uniqueObjectListDir;
    std::string fileNameListPath;
};

class BucketLogParser : public FileParser {
public:
    explicit BucketLogParser(const BucketLogParserParams& params)
        : FileParser(false), m_storageType(params.storageType),
          m_objectPrefixs(params.objectPrefixs), m_bucketLogDir(params.bucketLogDir),
          m_originalObjectListDir(params.originalObjectListDir), m_uniqueObjectListDir(params.uniqueObjectListDir),
          m_fileNameListPath(params.fileNameListPath)
    {}

    ~BucketLogParser()
    {
        if (m_writeFd.is_open()) {
            Close(CTRL_FILE_OPEN_MODE::WRITE);
        }
        if (m_readFd.is_open()) {
            Close(CTRL_FILE_OPEN_MODE::READ);
        }
    }

    bool TraverseAllLogFiles();

    static std::string HashFunc(const std::string& key);

private:
    CTRL_FILE_RETCODE ReadFile();

    CTRL_FILE_RETCODE ReadLine();

    bool NeedSkip(const BucketLogInfo& bucketLogInfo);

    bool FindObjectPrefix(const std::string& key, std::string& prefix);

    bool WriteEntry(const BucketLogInfo& bucketLogInfo);

    bool DeduplicateObjectListFile();

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

    StorageType m_storageType;
    std::vector<std::string> m_objectPrefixs;
    std::string m_bucketLogDir;
    std::string m_originalObjectListDir;
    std::string m_uniqueObjectListDir;
    std::string m_fileNameListPath;

    // <hash(前缀)/hash(对象名), objectlist文件读写句柄>
    std::unordered_map<std::string, std::unique_ptr<ObjectListParser>> m_objectListParserMap;
    std::unique_ptr<FileNameListParser> m_fileNameListParser = nullptr;
    uint32_t m_objectListFileCount = 0;
    uint64_t m_objectListEntryCount = 0;
};

}

#endif // MODULE_BUCKET_LOG_PARSER_H
