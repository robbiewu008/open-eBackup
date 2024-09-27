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
#include <boost/filesystem.hpp>
#include "Log.h"
#include "common/CloudServiceUtils.h"
#include "BucketLogParser.h"

using namespace Module;

namespace {
constexpr auto MODULE = "BUCKET_LOG_PARSER";
const uint64_t MAX_OBJECT_LIST_FILE_NUM = 1000;
}

ObjectOperation GetObjectOperation(const std::string& fieldContent)
{
    if (fieldContent.find("PUT") != std::string::npos) {
        return ObjectOperation::ADD_OR_MODIFY;
    }
    if (fieldContent.find("POST") != std::string::npos) {
        return ObjectOperation::ADD_OR_MODIFY;
    }
    if (fieldContent.find("DELETE") != std::string::npos) {
        return ObjectOperation::DELETE;
    }

    return ObjectOperation::UNKNOWN;
};

ObjectTarget GetObjectTarget(const std::string& fieldContent)
{
    if (fieldContent.find("OBJECT") != std::string::npos) {
        return ObjectTarget::OBJECT;
    }
    if (fieldContent.find("UPLOAD") != std::string::npos) {  // obs客户端大文件上传日志关键词
        return ObjectTarget::OBJECT;
    }
    if (fieldContent.find("METADATA") != std::string::npos) {
        return ObjectTarget::METADATA;
    }
    if (fieldContent.find("ACL") != std::string::npos) {
        return ObjectTarget::ACL;
    }

    return ObjectTarget::UNKNOWN;
};

bool PacificParseBucketLogOneLine(const std::string& lineContent, BucketLogInfo& bucketLogInfo)
{
    if (lineContent.empty()) {
        HCP_Log(ERR, MODULE) << "get empty line content" << HCPENDLOG;
        return false;
    }

    // 日志样例：
    // pacific:
    // 0000018C954AE689E3C17345F28F0FA8 log-test [04/Feb/2024:07:32:51 +0000] 8.42.99.169
    // 0000018C954AE689E3C17345F28F0FA8 082a63bf170703197199000006021000 REST.PUT.ACL + % ceshi 测试.txt
    // "PUT /log-test/%2B%20%25%20ceshi%20%E6%B5%8B%E8%AF%95.txt?acl HTTP/1.1" 200 - - - 14258 12008 "-"
    // "obs-browser-plus/3.23.9" - -

    static const std::string SEPARATOR = " ";
    static std::map<std::string, int> fieldIdMap = {
        {"bucketName", 1}, {"operateTime", 2}, {"operateTimeZone", 3}, {"objectOperation", 7}, {"objectName", 8}};
    static const int MAX_FIELD_ID = 9;

    std::vector<std::string> lineSplit;
    boost::algorithm::split(lineSplit, lineContent, boost::is_any_of(SEPARATOR), boost::token_compress_on);
    if (lineSplit.size() <= MAX_FIELD_ID) {
        HCP_Log(ERR, MODULE) << "line split size is too small, size " << lineSplit.size() << HCPENDLOG;
        return false;
    }

    bucketLogInfo.bucketName = lineSplit[fieldIdMap["bucketName"]];
    bucketLogInfo.operateTime = lineSplit[fieldIdMap["operateTime"]] + lineSplit[fieldIdMap["operateTimeZone"]];

    const std::string objectOperation = lineSplit[fieldIdMap["objectOperation"]];
    bucketLogInfo.objectOperation = GetObjectOperation(objectOperation);
    if (bucketLogInfo.objectOperation == ObjectOperation::UNKNOWN) {
        HCP_Log(DEBUG, MODULE) << lineContent << " contains unknown operation" << HCPENDLOG;
        return true;
    }
    bucketLogInfo.objectTarget = GetObjectTarget(objectOperation);
    if (bucketLogInfo.objectTarget == ObjectTarget::UNKNOWN) {
        HCP_Log(DEBUG, MODULE) << lineContent << " contains unknown target" << HCPENDLOG;
        return true;
    }

    for (std::size_t i = fieldIdMap["objectName"]; i < lineSplit.size(); ++i) {
        std::string objectNamePart = lineSplit[i];
        if (objectNamePart.find("\"PUT") == 0 || objectNamePart.find("\"DELETE") == 0 ||
            objectNamePart.find("\"POST") == 0) {
            break;
        }
        // pacific日志没有对特殊字符做任何处理，所以文件名中可能存在空格，需要做拼接，同时特殊字符需要做处理，与hcs一致
        // % 替换为 %25，与其它特殊字符的处理保持一致
        objectNamePart = std::regex_replace(objectNamePart, std::regex("%"), "%25");
        // 空格字符需要替换为 %20，便于后续解析
        if (i > fieldIdMap["objectName"]) {
            bucketLogInfo.objectName += "%20";
        }
        bucketLogInfo.objectName += objectNamePart;
    }
    bucketLogInfo.statusCode = "200";

    return true;
}

bool HWParseBucketLogOneLine(const std::string& lineContent, BucketLogInfo& bucketLogInfo)
{
    if (lineContent.empty()) {
        HCP_Log(ERR, MODULE) << "get empty line content" << HCPENDLOG;
        return false;
    }

    // 日志样例：
    // hcs:
    // 85248fa9872945188723fe7068537565 bucket1 [03/Feb/2024:23:50:22 +0000] 22.2.159.33
    // 85248fa9872945188723fe7068537565 0000018D71615D4F80058FBC87D5A7A6 REST.PUT.METADATA %2B+%25ceshi.txt
    // "PUT /bucket1/%2B%20%25ceshi.txt?metadata HTTP/1.1" 200 - - - 20 20 "-" "obs-browser-plus/3.22.9" - -
    // STANDARD - "-" f733150db0b849bba2554290869a124a

    static const std::string SEPARATOR = " ";
    static std::map<std::string, int> fieldIdMap = {
        {"bucketName", 1}, {"operateTime", 2}, {"operateTimeZone", 3}, {"objectOperation", 7}, {"objectName", 8},
        {"statusCode", 12}};
    static const int MAX_FIELD_ID = 12;

    std::vector<std::string> lineSplit;
    boost::algorithm::split(lineSplit, lineContent, boost::is_any_of(SEPARATOR), boost::token_compress_on);
    if (lineSplit.size() <= MAX_FIELD_ID) {
        HCP_Log(ERR, MODULE) << "line split size is too small, size " << lineSplit.size() << HCPENDLOG;
        return false;
    }

    bucketLogInfo.bucketName = lineSplit[fieldIdMap["bucketName"]];
    bucketLogInfo.operateTime = lineSplit[fieldIdMap["operateTime"]] + lineSplit[fieldIdMap["operateTimeZone"]];

    const std::string objectOperation = lineSplit[fieldIdMap["objectOperation"]];
    bucketLogInfo.objectOperation = GetObjectOperation(objectOperation);
    if (bucketLogInfo.objectOperation == ObjectOperation::UNKNOWN) {
        HCP_Log(DEBUG, MODULE) << lineContent << " contains unknown operation" << HCPENDLOG;
        return true;
    }
    bucketLogInfo.objectTarget = GetObjectTarget(objectOperation);
    if (bucketLogInfo.objectTarget == ObjectTarget::UNKNOWN) {
        HCP_Log(DEBUG, MODULE) << lineContent << " contains unknown target" << HCPENDLOG;
        return true;
    }

    bucketLogInfo.objectOperation = GetObjectOperation(lineSplit[fieldIdMap["objectOperation"]]);
    // + 替换为 %20（空格的ascii码）（hcs日志会将文件名中的空格替换为+，其它特殊字符则转换为以%为前缀的16进制ascii码）
    bucketLogInfo.objectName = std::regex_replace(lineSplit[fieldIdMap["objectName"]], std::regex(R"(\+)"), "%20");
    bucketLogInfo.statusCode = lineSplit[fieldIdMap["statusCode"]];

    return true;
}

using bucketLogLineParseFunc = std::function<bool(const std::string& lineContent, BucketLogInfo& bucketLogInfo)>;
std::map<StorageType, bucketLogLineParseFunc> parseFuncMap {
    {StorageType::PACIFIC, PacificParseBucketLogOneLine},
    {StorageType::HUAWEI, HWParseBucketLogOneLine}
};

bool BucketLogParser::TraverseAllLogFiles()
{
    if (parseFuncMap.count(m_storageType) == 0) {
        HCP_Log(ERR, MODULE) << "The current log type cannot be parsed, type "
            << static_cast<int>(m_storageType) << HCPENDLOG;
        return false;
    }

    m_fileNameListParser = std::make_unique<FileNameListParser>(m_fileNameListPath);
    if (m_fileNameListParser == nullptr) {
        HCP_Log(ERR, MODULE) << "Create FileNameListParser failed" << HCPENDLOG;
        return false;
    }
    if (m_fileNameListParser->Open(CTRL_FILE_OPEN_MODE::READ) != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(WARN, MODULE) << "Open file name list file falied, file name - " << m_fileNameListPath << HCPENDLOG;
        return true;
    }

    while (true) {
        std::string bucketLogFileName;
        CTRL_FILE_RETCODE fileNameListReadResult = m_fileNameListParser->ReadEntry(bucketLogFileName);
        if (fileNameListReadResult == CTRL_FILE_RETCODE::READ_EOF) {
            HCP_Log(INFO, MODULE) << "Read file name list " << m_fileNameListPath << " finished" << HCPENDLOG;
            break;
        }

        if (fileNameListReadResult != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Read bucket log file name list entry failed, file name "
                << bucketLogFileName << HCPENDLOG;
            return false;
        }

        m_fileName = m_bucketLogDir + "/" + bucketLogFileName;
        if (boost::filesystem::is_directory(m_fileName)) {
            HCP_Log(WARN, MODULE) << m_fileName << " is a directory, skip" << HCPENDLOG;
            continue;
        }
        if (ReadFile() != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Read file failed for log file " << m_fileName << HCPENDLOG;
            return false;
        }
    }

    if (!DeduplicateObjectListFile()) {
        HCP_Log(ERR, MODULE) << "DeduplicateObjectListFile failed" << HCPENDLOG;
        return false;
    }

    return true;
}

std::string BucketLogParser::HashFunc(const std::string& key)
{
    if (key.empty()) {
        return "default";
    }
    std::hash<std::string> hashFunc;
    return std::to_string(hashFunc(key) % MAX_OBJECT_LIST_FILE_NUM);
}

CTRL_FILE_RETCODE BucketLogParser::ReadFile()
{
    if (Open(CTRL_FILE_OPEN_MODE::READ) != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Open log file falied, file name - " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    while (true) {
        auto result = ReadLine();
        if (result == CTRL_FILE_RETCODE::FAILED) {
            HCP_Log(ERR, MODULE) << "Read line failed for log file " << m_fileName << HCPENDLOG;
            return CTRL_FILE_RETCODE::FAILED;
        }
        if (result == CTRL_FILE_RETCODE::READ_EOF) {
            HCP_Log(INFO, MODULE) << "Read log file " << m_fileName << " finished" << HCPENDLOG;
            break;
        }
    }

    if (Close(CTRL_FILE_OPEN_MODE::READ) != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Close log file falied, file name - " << m_fileName << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

CTRL_FILE_RETCODE BucketLogParser::ReadLine()
{
    if (!m_readFd.is_open()) {
        HCP_Log(ERR, MODULE) << "Read file is not open" << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    std::string lineContent;
    getline(m_readBuffer, lineContent);
    if (lineContent.empty()) {
        return CTRL_FILE_RETCODE::READ_EOF;
    }

    BucketLogInfo bucketLogInfo;
    if (!parseFuncMap[m_storageType](lineContent, bucketLogInfo)) {
        HCP_Log(ERR, MODULE) << "Parser bucket line failed, log file name " << m_fileName
            << ", line content " << lineContent << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }
    if (NeedSkip(bucketLogInfo)) {
        HCP_Log(DEBUG, MODULE) << "lineContent " << lineContent << " does not need backup" << HCPENDLOG;
        return CTRL_FILE_RETCODE::SUCCESS;
    }
    if (!WriteEntry(bucketLogInfo)) {
        HCP_Log(ERR, MODULE) << "Write entry failed, lineContent " << lineContent << HCPENDLOG;
        return CTRL_FILE_RETCODE::FAILED;
    }

    return CTRL_FILE_RETCODE::SUCCESS;
}

bool BucketLogParser::NeedSkip(const BucketLogInfo& bucketLogInfo)
{
    if (bucketLogInfo.bucketName.empty()) {
        HCP_Log(DEBUG, MODULE) << "Bucket name is empty" << HCPENDLOG;
        return true;
    }
    if (bucketLogInfo.objectName.empty()) {
        HCP_Log(DEBUG, MODULE) << "Object name is empty" << HCPENDLOG;
        return true;
    }
    if (bucketLogInfo.objectName == "-" && bucketLogInfo.objectTarget != ObjectTarget::OBJECT) {
        HCP_Log(DEBUG, MODULE) << "This action is object independent" << HCPENDLOG;
        return true;
    }
    // http状态码以4或5开头表示调用失败
    if (bucketLogInfo.statusCode.find("4") == 0 || bucketLogInfo.statusCode.find("5") == 0) {
        HCP_Log(DEBUG, MODULE) << "Request is failed, status code " << bucketLogInfo.statusCode << HCPENDLOG;
        return true;
    }

    return false;
}

bool BucketLogParser::FindObjectPrefix(const std::string& key, std::string& prefix)
{
    if (m_objectPrefixs.empty()) {
        HCP_Log(DEBUG, MODULE) << "Object prefix is empty, skip filter" << HCPENDLOG;
        prefix = "";
        return true;
    }
    
    for (const auto& objectPrefix : m_objectPrefixs) {
        if (key.find(objectPrefix) == 0) {
            HCP_Log(DEBUG, MODULE) << "Find prefix " << objectPrefix << HCPENDLOG;
            prefix = objectPrefix;
            return true;
        }
    }

    return false;
}

bool BucketLogParser::WriteEntry(const BucketLogInfo& bucketLogInfo)
{
    std::string originalObjectName; // 特殊字符需要还原后再计算hash，确保scanner模块在通过key计算hash时也能得到一样的值
    if (!ConvertSpecailChar2Normal(bucketLogInfo.objectName, originalObjectName)) {
        HCP_Log(ERR, MODULE) << "Convert special char failed, input is " << bucketLogInfo.objectName << HCPENDLOG;
        return false;
    }
    std::string prefix;
    if (!FindObjectPrefix(originalObjectName, prefix)) {
        HCP_Log(DEBUG, MODULE) << "ObjectName " << originalObjectName << " is not in backup prefix" << HCPENDLOG;
        return true;
    }

    const std::string prefixHash = HashFunc(prefix);
    const std::string objectNamehash = HashFunc(originalObjectName);
    const std::string hashKey = prefixHash + "/" + objectNamehash;
    if (m_objectListParserMap.count(hashKey) == 0 || m_objectListParserMap[hashKey] == nullptr) {
        ObjectListParserParams objectListParserParams;
        objectListParserParams.originalObjectListFilePath = m_originalObjectListDir + "/" + hashKey;
        objectListParserParams.uniqueObjectListFilePath = m_uniqueObjectListDir + "/" + hashKey;
        boost::filesystem::create_directories(m_originalObjectListDir + "/" + prefixHash);
        boost::filesystem::create_directories(m_uniqueObjectListDir + "/" + prefixHash);
        m_objectListParserMap[hashKey] = std::make_unique<ObjectListParser>(objectListParserParams);
        if (m_objectListParserMap[hashKey] == nullptr) {
            HCP_Log(ERR, MODULE) << "objectListParserMap is nullptr" << HCPENDLOG;
            return false;
        }
        if (m_objectListParserMap[hashKey]->Open(CTRL_FILE_OPEN_MODE::WRITE) != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Open object list write file failed, file id " << hashKey << HCPENDLOG;
            return false;
        }
    }

    if (m_objectListParserMap[hashKey]->WriteEntry(bucketLogInfo) != CTRL_FILE_RETCODE::SUCCESS) {
        HCP_Log(ERR, MODULE) << "Write entry from file " << m_fileName
            << " to object list file id " << hashKey << " failed" << HCPENDLOG;
        return false;
    }

    return true;
}

bool BucketLogParser::DeduplicateObjectListFile()
{
    for (const auto& pair : m_objectListParserMap) {
        if (pair.second == nullptr) {
            HCP_Log(WARN, MODULE) << "objectListParser is nullptr, hash key " << pair.first << HCPENDLOG;
            continue;
        }
        if (pair.second->Close(CTRL_FILE_OPEN_MODE::WRITE) != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(WARN, MODULE) << "Close original objectListParser failed, hash key " << pair.first << HCPENDLOG;
        }
        if (pair.second->Convert2UniqueObjectList() != CTRL_FILE_RETCODE::SUCCESS) {
            HCP_Log(ERR, MODULE) << "Convert2UniqueObjectList failed, hash key " << pair.first << HCPENDLOG;
            return false;
        }
    }

    return true;
}