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
#ifndef SCAN_FILTER_UTIL_H
#define SCAN_FILTER_UTIL_H

#include <vector>
#include <string>
#include <regex>
#include "ScanFilter.h"

class ScanFilterUtil {
public:
    static std::vector<std::string> ParentDirs(const std::string &path, const std::string& pathSpliter);
    static std::string DirPath(const std::string &path, const std::string& pathSpliter);
    static std::vector<std::string> DirPathUnion(std::vector<std::string> dirList, const std::string& separator);
    static bool CanCover(const std::string dirPath, const std::string subItemPath, const std::string& pathSpliter);
    static bool CanCover(const std::vector<std::string> dirPathList, const std::string subItemPath,
        const std::string& pathSpliter);

    static bool FileFiltersMatched(const std::vector<FilterItem>& fileFilters, const std::string &filePath);
    static bool DirFiltersMatched(const std::vector<FilterItem>& dirFilters, const std::string &dirPath);
    static bool DirFiltersMatched(const std::vector<FilterItem>& dirFilters, const std::string &dirPath,
        uint8_t &filterFlag);

    static bool ContainsWildcard(const std::string &path);
    static std::regex WildcardToRegex(const std::string &wildcard, bool caseSensitive = true);
    static std::string FilterFlagToString(uint8_t filterFlag);
    static std::string FilterTypeToString(FILTER_TYPE filterType);
    static std::string FormatToStandardPosixPath(const std::string& path);
    static std::string FormatToStandardWin32Path(const std::string& path);

    static std::string PosixPathToNasPath(NAS_PROTOCOL protocal, const std::string &posixPath);
    static std::string NasPathToPosixPath(NAS_PROTOCOL protocol, const std::string &nasPath);
    static std::string LowerCase(const std::string& str);
};

#endif