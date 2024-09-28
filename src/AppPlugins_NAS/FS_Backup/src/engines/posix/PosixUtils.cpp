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
#include "PosixUtils.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "log/Log.h"
#include "system/System.hpp"

using namespace std;
using namespace FS_Backup;
using namespace Module;

namespace {
    constexpr auto ERROR_MESSAGE_BUFFER_MAX = 1024;
    const std::string SLASH = "/";
    const std::string BACKSLASH = "\\";
    const std::string DOUBLESLASH = "//";
    const int MAX_RENAME_INDEX = 100;
};

void PosixUtils::RecurseCreateDirectory(const string path)
{
    DBGLOG("Enter RecurseCreateDirectory:%s", path.c_str());
    if (path.empty() || path[0] != '/') {
        return;
    }
    struct stat sb;
    if (stat(path.c_str(), &sb) == 0) {
        if (S_ISDIR(sb.st_mode)) {
            return;
        } else {
            remove(path.c_str());
        }
    }
    RecurseCreateDirectory(GetParentDirName(path));
    DBGLOG("create directory: %s", path.c_str());
    mkdir(path.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
}

int PosixUtils::RecurseCreateDirectoryWithErr(const std::string& path)
{
    DBGLOG("Enter RecurseCreateDirectory:%s", path.c_str());
    if (path.empty() || path[0] != '/') {
        return 0;
    }
    std::string fullPath = path;
    if (fullPath[fullPath.size() - 1] != '/') {
        fullPath += "/";
    }
    uint32_t beginPos = 1;
    uint32_t endPos = fullPath.size();
    for (uint32_t i = beginPos; i < endPos; i++) {
        if (fullPath[i] != '/') {
            continue;
        }
        std::string curPath = fullPath.substr(0, i);
        struct stat sb;
        if (stat(curPath.c_str(), &sb) == 0) {
            if (S_ISDIR(sb.st_mode)) {
                continue;
            }
            WARNLOG("remove file: %s", curPath.c_str());
            remove(curPath.c_str());
        }
        DBGLOG("create directory: %s", curPath.c_str());
        int ret = mkdir(curPath.c_str(), S_IRUSR | S_IWUSR | S_IXUSR);
        int err = errno;
        if (ret != 0 && err != EEXIST) {
            return err;
        }
    }
    return 0;
}

string PosixUtils::GetParentDirName(const string path)
{
    return path.substr(0, path.find_last_of("/"));
}

SpecialFileType PosixUtils::IsSpecialFile(uint32_t mode)
{
    if (S_ISLNK(mode)) {
        return SpecialFileType::SLINK;
    }
    if (S_ISBLK(mode)) {
        return SpecialFileType::BLK;
    }
    if (S_ISCHR(mode)) {
        return SpecialFileType::CHR;
    }
    if (S_ISFIFO(mode)) {
        return SpecialFileType::FIFO;
    }
    return SpecialFileType::REG;
}

std::string PosixUtils::ParseErrorMessage(uint32_t errCode)
{
    std::string errorMessage = hcp_strerror(errCode);
    /* if failed to obtain meaningful error string, using error code instead */
    if (errorMessage.empty()) {
        errorMessage = std::string("PosixErrorCode ") + std::to_string(errCode);
    }
    std::replace(errorMessage.begin(), errorMessage.end(), '\r', ' ');
    std::replace(errorMessage.begin(), errorMessage.end(), '\n', ' ');
    return errorMessage;
}

std::string PosixUtils::PathConcat(
    const std::string& forwardPath,
    const std::string& ctrlEntryPath,
    const std::string& trimPrefixPath)
{
    std::string newCtrlEntryPath = ctrlEntryPath;
    if (newCtrlEntryPath.find(trimPrefixPath) == 0) {
        newCtrlEntryPath = newCtrlEntryPath.substr(trimPrefixPath.length());
    }
    std::string path = forwardPath + SLASH + newCtrlEntryPath;
    std::size_t pos = 0;
    while ((pos = path.find(DOUBLESLASH)) != std::string::npos) {
        path.replace(pos, DOUBLESLASH.length(), SLASH);
    }
    return path;
}

static std::string GenerateRenameSuffix(const std::string& path, int index)
{
    std::string indexSuffix = " (" + std::to_string(index) + ")";
    auto slashPos = path.rfind("/");
    std::string newPath = path.substr(0, slashPos);
    std::string newName = path.substr(slashPos + 1);
    auto dotPos = newName.rfind(".");
    if (dotPos == std::string::npos) {
        newName += indexSuffix;
    } else {
        newName = newName.substr(0, dotPos) + indexSuffix + newName.substr(dotPos);
    }
    newPath += newName;
    return newPath;
}

std::string PosixUtils::GetUnusedNewPath(const std::string& path)
{
    std::string newPath = path;
    int index = 0;
    struct stat st;
    while (::stat(newPath.c_str(), &st) == 0 && index < MAX_RENAME_INDEX) {
        newPath = GenerateRenameSuffix(path, index++);
    }
    return newPath;
}
