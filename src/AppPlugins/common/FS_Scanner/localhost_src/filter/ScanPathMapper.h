/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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