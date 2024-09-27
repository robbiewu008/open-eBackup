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
#include "Win32Handler.h"
#include <iostream>
#include <sys/stat.h>
#include <cstring>
#include <string>
#include "ProtectPluginFactory.h"
#include "ApplicationServiceDataType.h"
#include "common/FileSystemUtil.h"
#include "PluginConstants.h"
#include "PluginNasTypes.h"
#include "ErrorCode.h"

using namespace std;
using namespace Module;
using namespace Module::FileSystemUtil;

namespace {
    constexpr auto MODULE = "Win32Handler";
    constexpr auto WIN32_MAX_PATH_LEN = 4096;
    constexpr auto BACKSLASH = "\\";
    constexpr auto SLASH = "/";
    const std::string FILE_ITEM_TYPE = "f";
    const std::string DIRECTORY_ITEM_TYPE = "d";
    const std::string UNC_PATH_PREFIX = R"(\\?\)";
}
namespace FilePlugin {
    
static AutoRegAppManager<Win32Handler> g_autoReg { ResourceType::WINDOWS };

void Win32Handler::ListNativeResource(FileResourceInfo& resourceInfo,
    const ListResourceParam& listResourceParam)
{
    int pageNo = listResourceParam.pageNo;
    int pageSize = listResourceParam.pageSize;
    string convertPath = listResourceParam.path;

    if (convertPath == SLASH || convertPath == BACKSLASH) {   // 兼容PM传来的"/"
        convertPath = "";
    }
    if (convertPath.empty()) {
        /* browse drivers */
        vector<std::string> drivers = FileSystemUtil::GetWin32DriverList();   // 返回驱动器
        for (const std::string &driver : drivers) {
            if (!IsDriverBackupable(driver)) {
                continue;
            }
            NasShareResourceInfo resourceDetailInfo;
            resourceDetailInfo.path = driver;
            resourceDetailInfo.type = "d"; /* show driver path as directory */
            resourceDetailInfo.hasChildren = true;
            resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
            ++resourceInfo.totalCount;
        }
        return;
    }
    /* browse files */
    GetReourceList(resourceInfo, pageNo, pageSize, convertPath);  // 返回指定路径的资源
}

void Win32Handler::GetReourceList(
    FileResourceInfo& resourceInfo, int pageNo, int pageSize, const std::string& dirPath)
{
    DBGLOG("list item in directory: %s", dirPath.c_str());
    auto startTime = chrono::steady_clock::now();
    BrowseFolderByPage(resourceInfo, dirPath, pageNo, pageSize);
    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    DBGLOG("GetReourceList, duration = %d", duration.count());
    return;
}

void Win32Handler::ListAggregateResource(
    FileResourceInfo& resourceInfo,
    const ListResourceParam& listResourceParam)
{
    ERRLOG("Win32Handler ListAggregateResource not implemented!");
    return;
}
 
void Win32Handler::ListVolumeResource(FileResourceInfo& resourceInfo, const ListResourceParam& listResourceParam)
{
    WARNLOG("Win32Handler ListVolumeResource not implemented!");
    return;
}

/*
 * Symbolic link & junction point treated as link file,
 * while device mount point treated as directory
 */
static std::string GetSubItemType(const StatResult& statResult)
{
    if (!statResult.IsReparsePoint()) {
        /* common file */
        return statResult.IsDirectory() ? DIRECTORY_ITEM_TYPE : FILE_ITEM_TYPE;
    }
    if (statResult.IsSymbolicLink() || statResult.IsJunctionPoint()) {
        return FILE_ITEM_TYPE;
    }
    if (statResult.IsMountedDevice()) {
        return DIRECTORY_ITEM_TYPE;
    }
    const std::string fullpath = statResult.CanicalPath();
    WARNLOG("unknown reparse point: %s", fullpath.c_str());
    return statResult.IsDirectory() ? DIRECTORY_ITEM_TYPE : FILE_ITEM_TYPE;
}

void Win32Handler::BrowseFolderByPage(FileResourceInfo& resourceInfo,
    const std::string& dirPath, int pageNo, int pageSize)
{
    int countBegin = pageNo * pageSize - 1;
    int countEnd = countBegin + pageSize;
    int totalNum = 0;
    int error;
    std::optional<OpenDirEntry> openDirEntry = FileSystemUtil::OpenDir(dirPath);
    if (!openDirEntry) {
        error = ::GetLastError();
        uint32_t errorCode = (error == ERROR_ACCESS_DENIED) ? E_RESOURCE_NO_ACCESS_RIGHT : E_RESOURCE_DIR_NOT_EXIST;
        std::string errMsg = (error == ERROR_ACCESS_DENIED) ? "RESOURCE_NO_ACCESS_RIGHT" : "RESOURCE_DIR_NOT_EXIST";
        ERRLOG("open dirent failed, path = %s, error = %d, mssage = %s", dirPath.c_str(), error, errMsg.c_str());
        ThrowAppException(errorCode, errMsg.c_str());
        return;
    }
    do {
        const std::string entryName = openDirEntry->Name();
        DBGLOG("list directory %s find entry %s", dirPath.c_str(), entryName.c_str());
        if (entryName == "." || entryName == "..") {
            continue;
        }
        const std::string fullpath = openDirEntry->FullPath();
        std::optional<StatResult> subStatResult = Stat(openDirEntry->FullPath());
        if (!subStatResult) {
            error = ::GetLastError();
            ERRLOG("stat path %s fail, error = %d", fullpath.c_str(), error);
            continue;
        }
        if (totalNum <= countBegin || totalNum > countEnd) {
            ++totalNum;
            continue;
        }
        NasShareResourceInfo resourceDetailInfo {};
        resourceDetailInfo.path = fullpath;
        resourceDetailInfo.size = openDirEntry->Size();
        resourceDetailInfo.type = GetSubItemType(subStatResult.value());
        resourceDetailInfo.hasChildren = openDirEntry->IsDirectory() ? true : false;
        resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
        ++totalNum;
    } while (openDirEntry->Next());
    INFOLOG("total num: %d", totalNum);
    resourceInfo.totalCount = totalNum;
    return;
}

bool Win32Handler::IsDriverBackupable(const std::string& driverPath)
{
    /* Only accept driver root path, so ANSI version works */
    UINT driverType = ::GetDriveTypeA(driverPath.c_str());
    if (driverType == DRIVE_UNKNOWN) {
        WARNLOG("driver path %s is an unknown driver, skip.", driverPath.c_str());
        return false;
    }
    if (driverType == DRIVE_CDROM) {
        WARNLOG("driver path %s is a CD-ROM driver, skip.", driverPath.c_str());
        return false;
    }
    return true;
}

}

