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
#include "PosixHandler.h"
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <cstring>
#ifdef __linux__
    #include <fcntl.h>
    #include <mntent.h>
#endif
#include <sys/types.h>
#include "ProtectPluginFactory.h"
#include "ApplicationServiceDataType.h"
#include "PluginConstants.h"
#include "PluginNasTypes.h"
#include "ErrorCode.h"
#include "utils/PluginUtilities.h"
#include "system/System.hpp"
#include "common/Thread.h"

namespace {
    constexpr auto STAT_TIMEOUT_MILLSEC = 100;
    constexpr auto NUM10 = 10;
}

using namespace std;

namespace {
    constexpr auto MODULE = "PosixHandler";
}

namespace FilePlugin {

static AutoRegAppManager<PosixHandler> g_autoReg { ResourceType::UNIX };

bool CheckSpecialResource(string file)
{
    const vector<string> specialRes = {
        "/proc",
        "/dev",
        "/run"
    };
    int specialResCnt = specialRes.size();
    for (int i = 0; i < specialResCnt; i++) {
        if (file == specialRes[i]) {
            return true;
        }
    }
    return false;
}

void PosixHandler::ListNativeResource(
    FileResourceInfo& resourceInfo,
    const ListResourceParam& listResourceParam)
{
    int pageNo = listResourceParam.pageNo;
    int pageSize = listResourceParam.pageSize;
    string convertPath = listResourceParam.path;
    if (convertPath.empty()) {
        convertPath = "/";
    }
    string parentPath = NormalizeDirectoryPath(convertPath);
    auto startTime = chrono::steady_clock::now();
    BrowseFolderByPage(resourceInfo, parentPath, pageNo, pageSize);
    auto endTime = chrono::steady_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(endTime - startTime);
    HCP_Log(DEBUG, MODULE) << "ListNativeResource, cost=" << duration.count() <<"ms" << HCPENDLOG;
    return;
}


struct NonBlockStatResult {
    // in
    std::string             path;
    int                     timeoutMillsec  { STAT_TIMEOUT_MILLSEC };
    // out
    bool                    returned    { false };
    bool                    valid       { false };
    struct stat             st          {};
};
 
static void BlockingStat(std::shared_ptr<NonBlockStatResult> stRetPtr)
{
    std::string path = stRetPtr->path;
    struct stat st;
    if (::stat(path.c_str(), &st) == 0) {
        stRetPtr->valid = true;
        stRetPtr->st = st;
    }
    stRetPtr->returned = true;
}
 
static bool NonBlockingStat(std::shared_ptr<NonBlockStatResult> stRetPtr)
{
    int timeout = stRetPtr->timeoutMillsec;
    int timer = 0;
    std::thread t = std::thread(&BlockingStat, stRetPtr);
    Module::SleepFor(std::chrono::milliseconds(NUM10));
    while (!stRetPtr->returned) {
        Module::SleepFor(std::chrono::milliseconds(NUM10));
        timer += NUM10;
        if (timer >= timeout) {
            t.detach();
            WARNLOG("Non blocking stat timeout for path: %s, time: %d", stRetPtr->path.c_str(), timeout);
            return false;
        }
    }
    t.join();
    return stRetPtr->valid;
}

void PosixHandler::BrowseFolderByPage(
    FileResourceInfo& resourceInfo, const std::string& parentPath, int pageNo, int pageSize)
{
    DIR* dir = opendir(parentPath.c_str());
    if (dir == nullptr) {
        int err = errno;
        HCP_Log(ERR, MODULE) << "PosixHandler opendir failed,dir:"<< parentPath <<
        ",errno:"<< err << ":" << strerror(err) <<  HCPENDLOG;
        uint32_t errorCode = (err == EACCES) ? E_RESOURCE_NO_ACCESS_RIGHT : E_RESOURCE_DIR_NOT_EXIST;
        ThrowAppException(errorCode, strerror(err));
        return;
    }
    struct dirent* dirinfo;
    int countBegin = pageNo * pageSize - 1;
    int countEnd = countBegin + pageSize;
    int totalNum = 0;
    while ((dirinfo = readdir(dir)) != nullptr) {
        if (strcmp(dirinfo->d_name, ".") == 0 || strcmp(dirinfo->d_name, "..") == 0) {
            continue;
        }
        string absPath = parentPath + dirinfo->d_name;
        if (CheckSpecialResource(absPath)) {
            DBGLOG("skip special resource %s", absPath.c_str());
            continue;
        }
        auto stRetPtr = std::make_shared<NonBlockStatResult>(NonBlockStatResult{ absPath, STAT_TIMEOUT_MILLSEC });
        if (!NonBlockingStat(stRetPtr)) {
            WARNLOG("Non blocking stat path: %s failed", absPath.c_str());
            continue;
        }
        if (totalNum <= countBegin || totalNum > countEnd) {
            ++totalNum;
            continue;
        }
        NasShareResourceInfo resourceDetailInfo;
        resourceDetailInfo.path = absPath;
        resourceDetailInfo.modifyTime = PluginUtils::FormatTimeToStrBySetting(
            stRetPtr->st.st_mtime, "%Y-%m-%d %H:%M:%S");
        resourceDetailInfo.size = stRetPtr->st.st_size;
        if (S_ISDIR(stRetPtr->st.st_mode)) {
            resourceDetailInfo.type = "d";
            resourceDetailInfo.hasChildren = true;
        } else {
            resourceDetailInfo.type = "f";
            resourceDetailInfo.hasChildren = false;
        }
        resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
        ++totalNum;
    }
    resourceInfo.totalCount = totalNum;
    closedir(dir);
}

std::string PosixHandler::NormalizeDirectoryPath(std::string path)
{
    char normalizePath[PATH_MAX + 1] = { 0x00 };
    if (realpath(path.c_str(), normalizePath) == nullptr) {
        int err = errno;
        string errMsg = strerror(err);
        HCP_Log(ERR, MODULE) << "PosixHandler realpath failed,path:"<< path <<
        ",errMsg:" << errMsg<< HCPENDLOG;
        if (err == EACCES) {
            ThrowAppException(E_RESOURCE_NO_ACCESS_RIGHT, errMsg);
        } else {
            ThrowAppException(E_RESOURCE_DIR_NOT_EXIST, errMsg);
        }
    }
    path = normalizePath;
    string::size_type slash_pos = path.find_last_of(dir_sep);
    if (slash_pos != path.size() - 1) {
        path += "/";
    }
    return path;
}

void PosixHandler::ListAggregateResource(
    FileResourceInfo& resourceInfo,
    const ListResourceParam& listResourceParam)
{
    WARNLOG("PosixHandler ListAggregateResource not implemented!");
    return;
}

// https://sites.uclouvain.be/SystInfo/usr/include/mntent.h.html
static std::vector<std::string> GetAllMountPoints(const std::string& devPath)
{
    DBGLOG("GetAllMountPoints %s", devPath.c_str());
    std::vector<std::string> mountPoints {};
#ifdef __linux__
    FILE* mountFile = ::setmntent("/proc/mounts", "r");
    if (mountFile == nullptr) {
        ERRLOG("failed to open /proc/mounts, errno = %d", errno);
        return mountPoints;
    }
    struct mntent* entry;
    while ((entry = ::getmntent(mountFile)) != nullptr) {
        std::string entryDevPath = entry->mnt_fsname;
        std::string entryMountPath = entry->mnt_dir;
        DBGLOG("detect mount points pairs %s => %s", entryDevPath.c_str(), entryMountPath.c_str());
        if (entryDevPath == devPath) {
            mountPoints.push_back(entryMountPath);
        }
    }
    ::endmntent(mountFile);
#else
    WARNLOG("ListVolumeResource not implemented on this platform!");
#endif
    return mountPoints;
}

static std::string GetAllMountPointsJoinStr(const std::string& devicePath)
{
    std::vector<std::string> mountPoints =  GetAllMountPoints(devicePath);
    const std::string separator = ",";
    std::string joinStr;
    for (const std::string& path : mountPoints) {
        joinStr += (path + separator);
    }
    if (!joinStr.empty() && joinStr.back() == separator.front()) {
        joinStr.pop_back();
    }
    return joinStr;
}

static bool CheckLvscanCommand()
{
    std::vector<std::string> paramList;
    std::vector<std::string> stdOutput;
    std::vector<std::string> errOutput;
    std::string stdoutMsg;
    std::string stderrMsg;
    std::string cmd = "lvscan";
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, stdOutput, errOutput);
    for (const std::string& it : stdOutput) {
        stdoutMsg += it + " ";
    }
    for (const std::string& it : errOutput) {
        stderrMsg += it + " ";
    }
    if (ret != 0) { // exec script failed
        ERRLOG("run shell ret: %d", ret);
        ERRLOG("run shell stdoutMsg: %s", stdoutMsg.c_str());
        ERRLOG("run shell stderrMsg: %s", stderrMsg.c_str());
        return false;
    }
    return true;
}

static std::vector<std::string> FilterOutLvmOnSwap(std::vector<std::string>& lvmVolumes)
{
    std::string cmd = R"(blkid -s TYPE | grep 'TYPE="swap"' | awk -F ': TYPE' '{print $1}' | grep "/dev/mapper")";
    std::vector<std::string> paramList;
    std::vector<std::string> stdOutput;
    std::vector<std::string> errOutput;
    std::vector<std::string> finalLvmVolumes;
    std::string stdoutMsg;
    std::string stderrMsg;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, stdOutput, errOutput);
    for (const std::string& it : stdOutput) {
        stdoutMsg += it + " ";
    }
    for (const std::string& it : errOutput) {
        stderrMsg += it + " ";
    }
    if (ret != 0) { // exec script failed, 如果不存在swap的lvm分区，也会返回非0
        WARNLOG("run shell ret: %d", ret);
        WARNLOG("run shell stdoutMsg: %s", stdoutMsg.c_str());
        WARNLOG("run shell stderrMsg: %s", stderrMsg.c_str());
    }
    for (const std::string& lvmVolume : lvmVolumes) {
        if (std::find(stdOutput.begin(), stdOutput.end(), lvmVolume) != stdOutput.end()) {
            INFOLOG("detect swapon lvm %s", lvmVolume.c_str());
            continue;
        }
        finalLvmVolumes.push_back(lvmVolume);
    }
    return finalLvmVolumes;
}

void PosixHandler::ListVolumeResource(
    FileResourceInfo& resourceInfo,
    const ListResourceParam& listResourceParam)
{
#ifdef __linux__
    int pageNo = listResourceParam.pageNo;
    int pageSize = listResourceParam.pageSize;
    // listResourceParam.path is useless in this case
    if (!CheckLvscanCommand()) {
        ERRLOG("no lvscan command found!");
        ThrowAppException(E_RESOURCE_AGENT_LVM_NOT_SUPPORT, "no lvscan command found");
        return;
    }
    std::string cmd = "lvscan | awk -F ' ' '{if($2 != \"Snapshot\" && $2 !=\"Original\"){ print$2;}"
        "else if($2 == \"Original\"){print $3}}' | awk -F \"'\" '{print $2}'"
        " | xargs dmsetup info | grep Name: | awk '{print \"/dev/mapper/\"$2}'";
    std::vector<std::string> paramList;
    std::vector<std::string> stdOutput;
    std::vector<std::string> errOutput;
    std::string stdoutMsg;
    std::string stderrMsg;
    INFOLOG("exec query lvm volumes scripts, cmd: %s", cmd.c_str());
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, stdOutput, errOutput);
    for (const std::string& it : stdOutput) {
        stdoutMsg += it + " ";
    }
    for (const std::string& it : errOutput) {
        stderrMsg += it + " ";
    }
 
    if (ret != 0) { // exec script failed
        ERRLOG("run shell ret: %d", ret);
        ERRLOG("run shell stdoutMsg: %s", stdoutMsg.c_str());
        ERRLOG("run shell stderrMsg: %s", stderrMsg.c_str());
        ThrowAppException(E_RESOURCE_TYPE_INVALID, stderrMsg);
        return;
    }
    std::vector<std::string> lvmVolumes = FilterOutLvmOnSwap(stdOutput);
    // parse lvs command output
    for (const std::string& lvmBlockDevicePath: lvmVolumes) {
        DBGLOG("detected non-snapshot lvm volume: %s", lvmBlockDevicePath.c_str());
        NasShareResourceInfo resourceDetailInfo;
        resourceDetailInfo.path = lvmBlockDevicePath;
        resourceDetailInfo.modifyTime = "0";
        resourceDetailInfo.size = PluginUtils::GetVolumeSize(lvmBlockDevicePath);
        // block file and has no children
        resourceDetailInfo.type = "f";
        resourceDetailInfo.hasChildren = false;
        resourceDetailInfo.volumeMountPoints = GetAllMountPointsJoinStr(lvmBlockDevicePath);
        resourceInfo.resourceDetailVec.push_back(resourceDetailInfo);
    }
#else
    WARNLOG("ListVolumeResource not implemented on this platform!");
#endif
}

}
