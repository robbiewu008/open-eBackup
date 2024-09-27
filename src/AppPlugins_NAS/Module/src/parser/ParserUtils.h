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
#ifndef MODULE_FILE_PARSER_UTILS_H
#define MODULE_FILE_PARSER_UTILS_H

#include <cstdint>
#include <string>
#include <vector>
#include "ParserStructs.h"
#include "define/Defines.h"

namespace Module {

class AGENT_API ParserUtils {
public:
    explicit ParserUtils() {}
    virtual ~ParserUtils() {}
    static std::string GetParentDirOfFile(std::string filePath);
    static bool CheckParentDirIsReachable(std::string path);
    static uint32_t GetRandomNumber(uint32_t minNum, uint32_t maxNum);
    static std::string ConstructStringName(uint32_t &offset, uint32_t &totCommaCnt, std::vector<std::string> &lineContents);
    static uint32_t GetCommaCountOfString(const char *str);
    static uint16_t Atou16(const char *s);
    static time_t GetCurrentTimeInSeconds();
    static std::vector<std::pair<std::string, std::string>> ParseXattr(const std::vector<Module::XMetaField> &xmetalist);
    static std::vector<std::pair<uint64_t, uint64_t>> ParseSparseInfo(const std::vector<Module::XMetaField> &xmetalist);

    /* parse ACL for Linux/NFS */
    static std::string ParseDefaultAcl(const std::vector<Module::XMetaField> &xmetalist);
    static std::string ParseAccessAcl(const std::vector<Module::XMetaField> &xmetalist);

    static std::string ParseObjectBucketName(const std::string& path);
    static std::string ParseObjectKey(const std::vector<XMetaField> &xmetalist);
    static std::string ParseObjectPath(const std::string& path, const std::vector<XMetaField> &xmetalist);

    /* parse Windows Security Descriptor ACE string */
    static std::string ParseSecurityDescriptor(const std::vector<XMetaField> &xmetalist);
    /* parse target print name of Windows symbolic link */
    static std::string ParseSymbolicLinkTargetPath(const std::vector<XMetaField> &xmetalist);
    /* parse target print name of Windows juntion point */
    static std::string ParseJunctionPointTargetPath(const std::vector<XMetaField> &xmetalist);
};

}
#endif