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
#ifndef SCAN_PATH_MAPPER_H
#define SCAN_PATH_MAPPER_H

#include <map>
#include <string>

class ScanPathMapper {
public:
#ifdef WIN32
    void AddWin32PathMapRule(const std::string& mappedDir, const std::string& prefix, char originDriver);
    std::pair<std::string, std::string> MappedWin32Entry(const std::string& originDir) const;
#endif
    void AddMapRule(const std::string& mappedDir, const std::string& prefix);
    std::string RecoverDir(const std::string& mappedDir) const;
    std::string MapDir(const std::string& originDir) const;

private:
#ifdef WIN32
    std::map<std::string, std::pair<std::string, std::string>> m_win32MappingRule;
#endif
    std::map<std::string, std::string> m_mappingRule;
};

#endif