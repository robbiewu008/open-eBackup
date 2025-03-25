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
#ifndef CTRL_FILE_FILTER_H
#define CTRL_FILE_FILTER_H

#include <string>
#include <vector>
#include "ScanConfig.h"

class CtrlFileFilter {
public:
    CtrlFileFilter() = default;
    CtrlFileFilter(const std::vector<std::string> &dirList,
        const std::vector<std::string> &fileList, CtrlFilterType filterType = CtrlFilterType::INCLUDE);
    void IncludeDir(std::string dirPath);
    void IncludeFile(std::string filePath);
    bool Accept(std::string path);
    void LogStatus() const;
    bool Enabled() const;
    bool PrefixMatch(const std::string &path);
    bool BinaryMatch(const std::string &path);
    bool AcceptDir(const std::string &dirPath);
    bool AcceptFile(const std::string &filePath);

private:
    void AddIncludePath(const std::string& path);
    void AddMatchPath(const std::string& path);
    void CheckExistAndPush(std::vector<std::string> &pathList, const std::string &path);
    void CheckAndResolvePath(std::string &path);
private:
    bool m_enabled {false};
    CtrlFilterType m_filterType { CtrlFilterType::INCLUDE };
    std::vector<std::string> m_matchList {};
    std::vector<std::string> m_includeList {};
};

#endif
