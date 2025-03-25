/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2020-2022. All rights reserved.
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
