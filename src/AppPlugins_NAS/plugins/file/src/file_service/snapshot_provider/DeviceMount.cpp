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
#include "DeviceMount.h"
#ifdef WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#elif defined(SOLARIS)
#include <sys/stat.h>
#include <sys/mntent.h>
#include <sys/mnttab.h>
#else
#include <mntent.h>
#include <sys/stat.h>
#endif
#include <string>
#include <map>
#include <algorithm>
#include <cctype>
#include <future>
#include <functional>
#include "log/Log.h"
#include "system/System.hpp"
#include "FileSystemUtil.h"
#include "Thread.h"
#include "securec.h"
#include "constant/PluginConstants.h"

using namespace std;
using namespace Module;
using namespace FileSystemUtil;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "DeviceMount";
    constexpr int VOLUME_SEGMENT_SIZE = 4;
    constexpr int NFS_LEN = 3;
    constexpr int DFINFO_SIZE = 2;
    constexpr int ONE_HUNDRED = 100;
    constexpr int STAT_TIMEOUT_SECOND = 3;
    const string CIFS = "CIFS";
    const std::wstring WPATH_PREFIX = LR"(\\?\)";
    const int MNTENT_BUFFER_MAX = 4096;
}

DeviceMount::DeviceMount() {}

#ifndef WIN32
struct NonBlockStatResult {
    // in
    std::string             path;
    int                     timeoutSec  { STAT_TIMEOUT_SECOND };
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
    int timeout = stRetPtr->timeoutSec;
    int timer = 0;
    std::thread t = std::thread(&BlockingStat, stRetPtr);
    Module::SleepFor(std::chrono::milliseconds(ONE_HUNDRED));
    while (!stRetPtr->returned) {
        Module::SleepFor(std::chrono::seconds(1));
        timer++;
        if (timer >= timeout) {
            t.detach();
            WARNLOG("Non blocking stat timeout for path: %s, time: %d", stRetPtr->path.c_str(), timeout);
            return false;
        }
    }
    t.join();
    return stRetPtr->valid;
}
#endif

std::string DeviceMount::GetVolumeId(const std::string& path)
{
    std::shared_ptr<FsDevice> fsDevicePtr = FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("%s don't have mount volume", path.c_str());
        return "";
    }
    std::string volumeId = std::to_string(fsDevicePtr->devNo);
    DBGLOG("get volumeId! path:%s, volumeId:%s", path.c_str(), volumeId.c_str());
    return volumeId;
}

#ifdef WIN32
void DeviceMount::GetWinVolumeId(std::string path, std::optional<uint64_t>& volumeId)
{
    wstring wPath = Utf8ToUtf16(path);
    BY_HANDLE_FILE_INFORMATION handleFileInformation{};
    // resolve UNC prefix
    if (wPath.length() <= WPATH_PREFIX.length() || wPath.find(WPATH_PREFIX) != 0) {
        /* already have prefix */
        wPath = WPATH_PREFIX + wPath;
    }
    HANDLE hFile = ::CreateFileW(wPath.c_str(),
        GENERIC_READ,
        FILE_SHARE_READ,
        nullptr,
        OPEN_EXISTING,
        FILE_FLAG_BACKUP_SEMANTICS,
        0);
    if (hFile == INVALID_HANDLE_VALUE) {
        return;
    }
    if (::GetFileInformationByHandle(hFile, &handleFileInformation) == 0) {
        ::CloseHandle(hFile);
        return;
    }
    ::CloseHandle(hFile);
    volumeId = std::make_optional<uint64_t>(static_cast<uint64_t>(handleFileInformation.dwVolumeSerialNumber));
    return;
}

void DeviceMount::GetNetVolume()
{
    string mountPath = R"(A:\)";
    for (char chDrive = 'A'; chDrive <= 'Z'; chDrive++) {
        mountPath[0] = chDrive;
        uint64_t devId;
        UINT driverType = GetDriveTypeW(Utf8ToUtf16(mountPath).c_str());
        if (driverType != DRIVE_REMOTE) {
            continue;
        }
        INFOLOG("get info for mountPath: %s", mountPath.c_str());
        std::optional<uint64_t> volumeId = std::nullopt;
        int retryCnt = 0;
        // for net drive with problem , sync call may stuck for minutes, cause job failure
        std::thread t = std::thread(&DeviceMount::GetWinVolumeId, this, mountPath, std::ref(volumeId));
        // for those good volume, 0.1s is enough to return
        Module::SleepFor(std::chrono::milliseconds(ONE_HUNDRED));
        while (!volumeId) {
            WARNLOG("hit retry for %d times, %s", retryCnt, mountPath);
            // not return in 100ms , probly err net device
            Module::SleepFor(std::chrono::seconds(1));
            retryCnt++;
            // not return in 3s. it has to be err net device, thread detach and try next
            if (retryCnt > 3) {
                t.detach();
                break;
            }
        }
        if (!volumeId) {
            ERRLOG("Invalid volume: %s", mountPath.c_str());
            continue;
        }
        t.join();
        devId = volumeId.value();
        string volumeIdStr = std::to_string(devId);
        auto volPtr = std::make_shared<FsDevice>(devId, mountPath, CIFS, "");
        if (m_fsDeviceMap.find(devId) == m_fsDeviceMap.end()) {
            m_fsDeviceMap[devId] = volPtr;
            DBGLOG("Load net device volume, devId: %s, mountPath:%s, volumeType:CIFS",
                volumeIdStr.c_str(), mountPath.c_str());
        }
    }
}

bool DeviceMount::LoadDevice()
{
    DBGLOG("Enter LoadDevice");
    std::optional<std::vector<Win32VolumesDetail>> volumesDetails = GetWin32VolumeList();
    if (!volumesDetails) {
        ERRLOG("LoadDevice failed, cannot get win32 volume list");
        return false;
    }
    for (Win32VolumesDetail& volumeDetail : volumesDetails.value()) {
        auto volumePathList = volumeDetail.GetVolumePathList();
        if (!volumePathList) {
            ERRLOG("this volume has no mountpoint");
            continue;
        }
        std::vector<std::string> mountPaths = volumePathList.value();
        if (mountPaths.empty()) {
            DBGLOG("there has no mountPaths");
            continue;
        }
        std::string mountPath = mountPaths[0];
        std::string volName = volumeDetail.VolumeName();
        std::optional<uint64_t> volumeId = std::nullopt;
        GetWinVolumeId(mountPath, volumeId);
        if (!volumeId) {
            DBGLOG("mountPath:%s, stat failed, can get its volumeId", mountPath.c_str());
            continue;
        }
        uint64_t devId = volumeId.value();
        std::string volType = volumeDetail.GetVolumeType().value();
        string volumeIdStr = std::to_string(devId);
        auto volPtr = std::make_shared<FsDevice>(devId, mountPath, volType, volName);
        if (m_fsDeviceMap.find(devId) == m_fsDeviceMap.end()) {
            m_fsDeviceMap[devId] = volPtr;
            DBGLOG("Load Device devId: %s, volumeName: %s, mountPath:%s, volumeType:%s",
                volumeIdStr.c_str(), volName.c_str(), mountPath.c_str(), volType.c_str());
        }
    }
    GetNetVolume();
    return true;
}

std::shared_ptr<FsDevice> DeviceMount::FindDevice(const std::string& path)
{
    std::optional<StatResult> statResult = Stat(path);
    if (!statResult) {
        HCP_Log(ERR, MODULE) << "findDevice failed, file:"<< path <<
            " is not exist" << HCPENDLOG;
        return nullptr;
    }
    std::optional<uint64_t> winVolumeId = std::nullopt;
    GetWinVolumeId(path, winVolumeId);
    if (!winVolumeId) {
        ERRLOG("cannot get win volume Id of %s", path.c_str());
        return nullptr;
    }
    uint64_t volumeId = winVolumeId.value();
    if (m_fsDeviceMap.count(volumeId) != 0) {
        HCP_Log(DEBUG, MODULE) << "findDevice success, file:"<< path <<
            ",devNo:"<< volumeId << HCPENDLOG;
        return m_fsDeviceMap[volumeId];
    }
    HCP_Log(INFO, MODULE) << "findDevice failed, file:"<< path <<
            ",deviceID:" << volumeId << "not found" << HCPENDLOG;
    return nullptr;
}

bool DeviceMount::GetInValidMountPoints(std::set<std::string>& invalidMountPoints)
{
    return true;
}

std::string DeviceMount::GetFsType(const std::string file)
{
    std::shared_ptr<FsDevice> fsDevicePtr = FindDevice(file);
    if (fsDevicePtr == nullptr) {
        ERRLOG("%s don't have mount volume", file.c_str());
        return "";
    }
    return fsDevicePtr->fsType;
}

bool DeviceMount::GetSubVolumes(std::string path, std::vector<std::shared_ptr<FsDevice>>& outputEntryList) // win
{
    std::shared_ptr<FsDevice> fsDevicePtr = FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("FindDevice for path: %s failed", path.c_str());
        return false;
    }
    for (auto it : m_fsDeviceMap) {
        if (fsDevicePtr->devNo == it.first) {
            continue;
        }
        std::shared_ptr<FsDevice> volPtr = it.second;
        // 避免path为c:\mnt\aaa，子卷为c:\mnt\aaamount，这种情况
        if (path.back() != '\\') {
            path = path + "\\";
        }
        // 判断此卷挂载点是否为当前目录的子目录
        int oriMntPointLen = path.length();
        if (strncmp(volPtr->mountPoint.c_str(), path.c_str(), oriMntPointLen) != 0) {
            continue;
        }
        int volMountPointLen = volPtr->mountPoint.length();
        if (volMountPointLen > oriMntPointLen) {
            DBGLOG("Get subvolume: %s, for path: %s", volPtr->mountPoint.c_str(), path.c_str());
            outputEntryList.push_back(volPtr);
        }
    }
    return true;
}

DeviceMount::~DeviceMount()
{
    m_fsDeviceMap.clear();
}

#elif defined(__linux__)

bool DeviceMount::LoadDevice()
{
    DBGLOG("Enter LoadDevice");
    FILE *mntfp;
    if ((mntfp = setmntent("/proc/mounts", "r")) == nullptr) {
        HCP_Log(ERR, MODULE) << "LoadDevice failed,/proc/mounts read failed" << HCPENDLOG;
        return false;
    }
    lock_guard<std::mutex> lock(m_mtxLock);
    struct mntent mnt;
    char mntentBuffer[MNTENT_BUFFER_MAX];
    memset_s(&mnt, sizeof(mnt), 0, sizeof(mnt));
    memset_s(mntentBuffer, sizeof(mntentBuffer), 0, sizeof(mntentBuffer));
    while (getmntent_r(mntfp, &mnt, mntentBuffer, sizeof(mntentBuffer)) != nullptr) {
        if (strcmp(mnt.mnt_type, "rootfs") == 0) {
            continue;
        }
        std::shared_ptr<NonBlockStatResult> stRetPtr =
            std::make_shared<NonBlockStatResult>(NonBlockStatResult{mnt.mnt_dir, STAT_TIMEOUT_SECOND});
        if (!NonBlockingStat(stRetPtr)) {
            WARNLOG("Non blocking stat path: %s failed", mnt.mnt_dir);
            m_invalidMountPoints.emplace(string(mnt.mnt_dir));
            continue;
        }
        struct stat st = stRetPtr->st;
        if (m_fsDeviceMap.count(st.st_dev) != 0) {
            m_fsDeviceMap[st.st_dev]->mountPoints.emplace(mnt.mnt_dir);
        } else {
            std::set<std::string> mountPoints = { mnt.mnt_dir };
            shared_ptr<FsDevice> volPtr = make_shared<FsDevice>(st.st_dev, mountPoints, mnt.mnt_type, mnt.mnt_fsname);
            m_fsDeviceMap.emplace(st.st_dev, volPtr);
        }

        std::hash<std::string> hashFn;
        uint64_t hashId = hashFn(std::to_string(st.st_dev) + mnt.mnt_dir);
        DBGLOG("hashId: %llu, devNo: %d, deviceName: %s, mountPoint: %s, fsType: %s, mntOpts: %s",
            hashId, st.st_dev, mnt.mnt_fsname, mnt.mnt_dir, mnt.mnt_type, mnt.mnt_opts);
        shared_ptr<FsDevice> volPtr = make_shared<FsDevice>(hashId, mnt.mnt_dir, mnt.mnt_type, mnt.mnt_fsname);
        if (m_fsDeviceMntMap.count(hashId) == 0) {
            m_fsDeviceMntMap.emplace(hashId, volPtr);
        }
    }
    endmntent(mntfp);
    DBGLOG("Exit LoadDevice");
    return true;
}

string DeviceMount::GetFsType(string file)
{
    shared_ptr<FsDevice> fsDevicePtr = FindDevice(file);
    if (fsDevicePtr == nullptr) {
        ERRLOG("%s don't have mount volume", file.c_str());
        return "";
    }
    return fsDevicePtr->fsType;
}

bool DeviceMount::GetSubVolumes(string path, vector<shared_ptr<FsDevice>>& outputEntryList) // linux
{
    shared_ptr<FsDevice> fsDevicePtr = FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("FindDevice for path: %s failed", path.c_str());
        return false;
    }
    for (auto it : m_fsDeviceMntMap) {
        if (fsDevicePtr->devNo == it.first) {
            continue;
        }
        shared_ptr<FsDevice> volPtr = it.second;
        // 当前目录根目录，所有卷挂载点都为当前目录子目录
        if (path == "/") {
            DBGLOG("Get subvolume: %s, for path: %s", volPtr->mountPoint.c_str(), path.c_str());
            outputEntryList.push_back(volPtr);
            continue;
        }
        // 避免path为/mnt/aaa，子卷为/mnt/aaamount，这种情况
        if (path.back() != '/') {
            path = path + "/";
        }
        // 判断此卷挂载点是否为当前目录的子目录
        int oriMntPointLen = path.length();
        if (strncmp(volPtr->mountPoint.c_str(), path.c_str(), oriMntPointLen) != 0) {
            continue;
        }
        int volMountPointLen = volPtr->mountPoint.length();
        if (volMountPointLen > oriMntPointLen) {
            DBGLOG("Get subvolume: %s, for path: %s", volPtr->mountPoint.c_str(), path.c_str());
            outputEntryList.push_back(volPtr);
        }
    }
    return true;
}

shared_ptr<FsDevice> DeviceMount::FindDevice(const string& file)
{
    struct stat statp;
    if (lstat(file.c_str(), &statp) != 0) {
        ERRLOG("findDevice failed, file: %s is not exist", file.c_str());
        return nullptr;
    }
    // Find all equipment list
    DBGLOG("file: %s, devId: %lu", file.c_str(), statp.st_dev);
    if (m_fsDeviceMap.count(statp.st_dev) == 0) {
        ERRLOG("FindDevice failed file: %s, device: %lu", file.c_str(), statp.st_dev);
        return nullptr;
    }
    auto mntPoints = m_fsDeviceMap[statp.st_dev]->mountPoints;
    for (auto iter = mntPoints.crbegin(); iter != mntPoints.crend(); ++iter) {
        DBGLOG("file: %s, devId: %lu, mntPoint: %s", file.c_str(), statp.st_dev, iter->c_str());
        if (file.size() < iter->size() || (file.size() == iter->size() && *iter != file)) {
            continue;
        }
        if (*iter == dir_sep || *iter == file || file.substr(0, iter->size() + 1) == *iter + dir_sep) {
            std::string mntPnt = *iter;
            std::hash<std::string> hashFn;
            uint64_t hashId = hashFn(std::to_string(statp.st_dev) + mntPnt);
            if (m_fsDeviceMntMap.count(hashId)) {
                DBGLOG("FindDevice success file: %s, device: %lu, hashId: %llu", file.c_str(), statp.st_dev, hashId);
                return m_fsDeviceMntMap[hashId];
            }
            ERRLOG("FindDevice failed file: %s, device: %lu, hashId: %llu", file.c_str(), statp.st_dev, hashId);
            return nullptr;
        }
    }
    ERRLOG("FindDevice failed file: %s, device: %lu", file.c_str(), statp.st_dev);
    return nullptr;
}

bool DeviceMount::GetInValidMountPoints(set<string>& invalidMountPoints)
{
    if (m_invalidMountPoints.empty()) {
        return false;
    }
    invalidMountPoints = m_invalidMountPoints;
    return true;
}

DeviceMount::~DeviceMount()
{
    m_fsDeviceMap.clear();
}

#else

#if defined(SOLARIS)
bool DeviceMount::LoadDevice()
{
    DBGLOG("Enter LoadDevice");
    FILE *fp = std::fopen("/etc/mnttab", "r");
    if (fp == nullptr) {
        ERRLOG("LoadDevice failed, /etc/mnttab read failed");
        return false;
    }
    struct mnttab mnt;
    while (getmntent(fp, &mnt) == 0) {
        std::shared_ptr<NonBlockStatResult> stRetPtr =
            std::make_shared<NonBlockStatResult>(NonBlockStatResult{mnt.mnt_mountp, STAT_TIMEOUT_SECOND});
        if (!NonBlockingStat(stRetPtr)) {
            WARNLOG("Non blocking stat path: %s failed", mnt.mnt_mountp);
            m_invalidMountPoints.emplace(string(mnt.mnt_mountp));
            continue;
        }
        struct stat st = stRetPtr->st;
        if (m_fsDeviceMap.count(st.st_dev) != 0) {
            continue;
        }
        shared_ptr<FsDevice> ptr = make_shared<FsDevice>(st.st_dev, mnt.mnt_mountp, mnt.mnt_fstype, mnt.mnt_special);
        DBGLOG("devNo: %d, deviceName: %s, mountPoint: %s, fsType: %s, mntOpts: %s",
            st.st_dev, mnt.mnt_special, mnt.mnt_mountp, mnt.mnt_fstype, mnt.mnt_mntopts);
        m_fsDeviceMap.emplace(st.st_dev, ptr);
    }
    std::fclose(fp);
    DBGLOG("Exit LoadDevice");
    return true;
}
#endif

#ifdef _AIX

/*
 * used to check if a device path obtained from the second col of "mount" command is a valid device
 */
static bool IsValidDevicePath(const std::string& devicePath)
{
    if (!devicePath.empty() && devicePath.front() == '/') {
        return true;
    } else {
        DBGLOG("skip invalid device path : %s", devicePath.c_str());
        return false;
    }
}

bool DeviceMount::LoadDevice()
{
    INFOLOG("Enter LoadDevice");
    string cmd = "mount";
    vector<string> output;
    vector<string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput);
    if (ret != 0) {
        INFOLOG("mount failed, cannot get all mounted volume");
        return false;
    }
    INFOLOG("output size:%d", output.size());
    if (output.empty()) {
        INFOLOG("has no valid mounted volume");
        return false;
    }
    for (auto& mountedDevice : output) {
        vector<string> device;
        INFOLOG("mount information:%s", mountedDevice.c_str());
        boost::split(device, mountedDevice, boost::is_any_of(" "), boost::token_compress_on);
        struct stat statp;
        if (device.size() < VOLUME_SEGMENT_SIZE) {
            DBGLOG("this line of mount don't have enough information");
            continue;
        }
        string deviceName = device[1];
        string mountPath = device[2];
        string fsType = device[3];
        if (!IsValidDevicePath(deviceName)) {
            continue;
        }
        DBGLOG("deviceName:%s, mountPath:%s, fsType:%s", deviceName.c_str(), mountPath.c_str(), fsType.c_str());
        if (lstat(mountPath.c_str(), &statp) != 0) {
            ERRLOG("stat failed, path:%s", mountPath.c_str());
            continue;
        }
        uint64_t volumeId = statp.st_dev;
        shared_ptr<FsDevice> volPtr = make_shared<FsDevice>(volumeId, mountPath, fsType, deviceName);
        INFOLOG("volumeId:%lu, mountPath:%s, fsType:%s, deviceName:%s", volumeId, mountPath.c_str(),
            fsType.c_str(), deviceName.c_str());
        if (m_fsDeviceMap.count(volumeId) == 0) {
            INFOLOG("mountPath:%s", mountPath.c_str());
            m_fsDeviceMap[volumeId] = volPtr;
        }
    }
    return true;
}

#endif

string DeviceMount::GetFsType(string file)
{
    shared_ptr<FsDevice> fsDevicePtr = FindDevice(file);
    if (fsDevicePtr == nullptr) {
        ERRLOG("%s don't have mount volume", file.c_str());
        return "";
    }
    return fsDevicePtr->fsType;
}

bool DeviceMount::GetSubVolumes(string path, vector<shared_ptr<FsDevice>>& outputEntryList) // solaris aix
{
    shared_ptr<FsDevice> fsDevicePtr = FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("FindDevice for path: %s failed", path.c_str());
        return false;
    }
    for (auto it : m_fsDeviceMap) {
        if (fsDevicePtr->devNo == it.first) {
            continue;
        }
        shared_ptr<FsDevice> volPtr = it.second;
        // 当前目录根目录，所有卷挂载点都为当前目录子目录
        if (path == "/") {
            DBGLOG("Get subvolume: %s, for path: %s", volPtr->mountPoint.c_str(), path.c_str());
            outputEntryList.push_back(volPtr);
            continue;
        }
        // 避免path为/mnt/aaa，子卷为/mnt/aaamount，这种情况
        if (path.back() != '/') {
            path = path + "/";
        }
        // 判断此卷挂载点是否为当前目录的子目录
        int oriMntPointLen = path.length();
        if (strncmp(volPtr->mountPoint.c_str(), path.c_str(), oriMntPointLen) != 0) {
            continue;
        }
        int volMountPointLen = volPtr->mountPoint.length();
        if (volMountPointLen > oriMntPointLen) {
            DBGLOG("Get subvolume: %s, for path: %s", volPtr->mountPoint.c_str(), path.c_str());
            outputEntryList.push_back(volPtr);
        }
    }
    return true;
}

shared_ptr<FsDevice> DeviceMount::FindDevice(const string& file)
{
    struct stat statp;
    if (lstat(file.c_str(), &statp) != 0) {
        HCP_Log(ERR, MODULE) << "findDevice failed, file:"<< file <<
            " is not exist" << HCPENDLOG;
        return nullptr;
    }
    // Find all equipment list
    DBGLOG("file:%s, devId:%lu", file.c_str(), statp.st_dev);
    if (m_fsDeviceMap.count(statp.st_dev) != 0) {
        DBGLOG("FindDevice success file:%s, device:%lu", file.c_str(), statp.st_dev);
        return m_fsDeviceMap[statp.st_dev];
    }
    ERRLOG("FindDevice failed file:%s, device:%lu", file.c_str(), statp.st_dev);
    return nullptr;
}

bool DeviceMount::GetInValidMountPoints(set<string>& invalidMountPoints)
{
    if (m_invalidMountPoints.empty()) {
        return false;
    }
    invalidMountPoints = m_invalidMountPoints;
    return true;
}

DeviceMount::~DeviceMount()
{
    m_fsDeviceMap.clear();
}
#endif

bool DeviceMount::CheckWhetherMountPoint(const std::string& path)
{
    shared_ptr<FsDevice> fsDevicePtr = FindDevice(path);
    if (fsDevicePtr == nullptr) {
        ERRLOG("FindDevice for path: %s failed", path.c_str());
        return false;
    }
    DBGLOG("path: %s, the mount point of fs which the path belong to: %s",
        path.c_str(), fsDevicePtr->mountPoint.c_str());
    return fsDevicePtr->mountPoint == path;
}

}
