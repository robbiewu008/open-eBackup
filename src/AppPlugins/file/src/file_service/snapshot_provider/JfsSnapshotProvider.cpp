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
#ifdef _AIX
#include "JfsSnapshotProvider.h"
#include <libgen.h>
#include "log/Log.h"
#include "common/Path.h"
#include "utils/PluginUtilities.h"
#include "system/System.hpp"


using namespace std;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "JfsSnapshotProvider";
    const string DEVICE_PREFIX = "/dev/";
    constexpr int VOLUME_SEGMENT_SIZE = 1;
    const string SUPPORT_JFS2_FS[] = {
        "jfs2"
    };
    constexpr uint8_t SNAP_ERRNO_SPACELESS = 3;
}

JfsSnapshotProvider::JfsSnapshotProvider(
    std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId, const std::string& snapshotMountRoot)
    : deviceMount(deviceMount), jobId(jobId), m_snapshotMountRoot(snapshotMountRoot)
{}

SnapshotResult JfsSnapshotProvider::CreateSnapshot(const std::string& filePath, bool isCrossVolume)
{
    SnapshotResult snapshotResult;
    if (filePath.empty()) {
        return snapshotResult;
    }
    string volumePath = filePath;
    if (!IsDir(volumePath)) {
        volumePath = GetDirName(volumePath);
        isCrossVolume = false;
    }
    // Get the LV list of the original volume and subvolume (it may not be supported by the original volume
    std::vector<LvmSnapshot> volumeInfo;
    bool isOriSupport = GetSupportSnapVolume(volumePath, isCrossVolume, volumeInfo);
    if (volumeInfo.size() == 0) {
        snapshotResult.snapShotStatus = SNAPSHOT_STATUS::UNSUPPORTED;
        WARNLOG("The Jfs logical volume where the path is located is empty, path: %s", filePath.c_str());
        return snapshotResult;
    }
    std::lock_guard<std::mutex> lock(mtxLock);
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    for (auto it = volumeInfo.begin(); it != volumeInfo.end(); ++it) {
        int ret = 0;
        std::shared_ptr<LvmSnapshot> snapshotPtr = CreateSnapshotByVolume((*it).m_oriDeviceVolume,
            (*it).m_oriDeviceMountPath, ret);
        INFOLOG("CreateSnapshotByVolume returned value is %d", ret);
        if (ret == SNAP_ERRNO_SPACELESS) {
            size_t pos = (*it).m_oriDeviceVolume.find('/');
            string vgName = (*it).m_oriDeviceVolume.substr(0, pos);
            ERRLOG("CreateSnapshotByVolume failed, lvm vg [%s] doesn't have enough space!", vgName.c_str());
            snapshotResult.spacelessVgs.emplace(vgName);
            continue;
        }
        if (snapshotPtr == nullptr) {
            ERRLOG("CreateSnapshotByVolume failed!");
            snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
            return snapshotResult;
        }
        INFOLOG("oriDeviceName:%s, oriDevicePath:%s, snapDeviceName:%s, snapDevicePath:%s",
            snapshotPtr->m_oriDeviceVolume.c_str(), snapshotPtr->m_oriDeviceMountPath.c_str(),
            snapshotPtr->m_snapDeviceVolume.c_str(), snapshotPtr->m_snapDeviceMountPath.c_str());
        snapshotResult.snapshotsMapper.emplace(snapshotPtr->m_oriDeviceMountPath,
                                               snapshotPtr->m_snapDeviceMountPath);
        snapshotResult.snapshotVolumeMapper.emplace(snapshotPtr->m_oriDeviceMountPath,
                                                    snapshotPtr->m_snapDeviceVolume);
        deviceVolumeSnapMap.emplace(snapshotPtr->m_oriDeviceVolume, snapshotPtr);
        snapshotResult.deviceVolumeSnapMap.emplace(snapshotPtr->m_oriDeviceMountPath, snapshotPtr->m_snapDeviceVolume);
    }
    return snapshotResult;
}

SnapshotResult JfsSnapshotProvider::QuerySnapshot(const std::string& filePath)
{
    SnapshotResult snapshotResult;
    return snapshotResult;
}

bool JfsSnapshotProvider::DeleteSnapshotByTag(const std::string &snapTag)
{
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(snapTag); // snapTag
    std::string cmd = "?/bin/lvmSnapshot.sh -dtag '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0) {
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "delete snapshot by tag success,snapTag:" << snapTag << HCPENDLOG;
    return true;
}

SnapshotDeleteResult JfsSnapshotProvider::DeleteAllSnapshots(const std::set<std::string>& snapshotInfos)
{
    std::lock_guard<std::mutex> lock(mtxLock);
    SnapshotDeleteResult snapshotResult;
    snapshotResult.status = true;
    DBGLOG("DeleteAllSnapshots,snapshot size:%d", snapshotInfos.size());
    std::string mountParentPath;
    mountParentPath.append(m_snapshotMountRoot).append("/").append(jobId);
    // umount all mountpoint
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(mountParentPath); // mountpath Prefix
    std::string cmd = "?/bin/jfsSnapshot.sh -umount '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0) {
        ERRLOG("umount snapshot failed, check path:%s", mountParentPath.c_str());
    }
    for (const string& volumeId : snapshotInfos) {
        bool status = DeleteSnapshotByVolume(volumeId);
        if (!status) {
            snapshotResult.status = false;
            snapshotResult.snapshots.emplace(volumeId);
        }
    }
    boost::system::error_code ec;
    boost::filesystem::remove_all(mountParentPath, ec);
    if (ec) {
        ERRLOG("Error deleting directory:%s, errno:%s", mountParentPath.c_str(), ec.message().c_str());
    }
    deviceVolumeSnapMap.clear();
    return snapshotResult;
}

bool JfsSnapshotProvider::MountSnapshot(const std::string &volumeDevice, const std::string& mountPath)
{
    HCP_Log(DEBUG, MODULE) << "MountSnapshot,snapshot volumeDevice:"<<
        volumeDevice << ",mountPath:"<< mountPath << HCPENDLOG;
    if (volumeDevice.empty() || mountPath.empty()) {
        HCP_Log(ERR, MODULE) << "volumeDevice or mountPath is emtpy:" << HCPENDLOG;
        return false;
    }
    if (strncmp(volumeDevice.c_str(), DEVICE_PREFIX.c_str(), DEVICE_PREFIX.length()) != 0) {
        HCP_Log(ERR, MODULE) << "Strange volumeDevice:" << volumeDevice << HCPENDLOG;
        return false;
    }
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeDevice);
    paramList.push_back(mountPath);
    std::string cmd = "?/bin/jfsSnapshot.sh -mount '?' '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0) {
        ERRLOG("MountSnapshot failed,volumeDevice:%s, mountPath:%s", volumeDevice.c_str(), mountPath.c_str());
        return false;
    }
    INFOLOG("MountSnapshot success,volumeDevice:%s, mountPath:%s", volumeDevice.c_str(), mountPath.c_str());
    return true;
}

bool JfsSnapshotProvider::IsSupportSnapshot(const std::string& file, std::string &lvPath, std::string &mountPoint)
{
    std::shared_ptr<FsDevice> fsDevice = deviceMount->FindDevice(file);
    if (fsDevice == nullptr) {
        return false;
    }
    bool isSupportJfs = false;
    int fsCnt = sizeof(SUPPORT_JFS2_FS) / sizeof(SUPPORT_JFS2_FS[0]);
    for (int i = 0; i < fsCnt; i++) {
        if (fsDevice->fsType == SUPPORT_JFS2_FS[i]) {
            isSupportJfs = true;
            break;
        }
    }
    if (!isSupportJfs) {
        HCP_Log(WARN, MODULE) << "Not support Jfs snapshot, file:" << file <<
            ",fsType:"<< fsDevice->fsType << HCPENDLOG;
        return false;
    }
    mountPoint = fsDevice->mountPoint;
    if (fsDevice->supportSnapCalled) {
        lvPath = fsDevice->lvPath;
        return fsDevice->isSupportSnapshot;
    }
    fsDevice->supportSnapCalled = true;
    // Query the valid LVM logical volume of the file system where the specified directory is located,
    // and the shell returns the result in the form: volumeName:mountPath
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(fsDevice->deviceName); // device /dev/lvName
    std::string cmd = "?/bin/jfsSnapshot.sh -ld '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0 || output.size() == 0) {
        WARNLOG("Not spport jfs snapshot, file: %s, device: %s", file.c_str(), fsDevice->deviceName.c_str());
        fsDevice->isSupportSnapshot = false;
        return false;
    }
    lvPath = output.front() + "/" + std::to_string(fsDevice->devNo);
    DBGLOG("Get jfs vg name success, file: %s, lvPath: %s", file.c_str(), lvPath.c_str());
    fsDevice->isSupportSnapshot = true;
    fsDevice->lvPath = lvPath;
    return true;
}

bool JfsSnapshotProvider::GetSupportSnapVolume(const std::string& file, bool isCrossVolume,
    std::vector<LvmSnapshot> &volumeInfo)
{
    std::string lvPath;
    std::string mountPoint;
    bool isOriSupport = IsSupportSnapshot(file, lvPath, mountPoint);
    if (isOriSupport) {
        LvmSnapshot snapshot(lvPath, mountPoint, "", "", "");
        volumeInfo.push_back(snapshot);
    }
    if (isCrossVolume) {
        GetSubVolumes(file, volumeInfo);
    }
    return isOriSupport;
}

void JfsSnapshotProvider::GetSubVolumes(const std::string& file,
    std::vector<LvmSnapshot> &volumeInfo)
{
    std::vector<std::shared_ptr<FsDevice>> outputEntryList;
    deviceMount->GetSubVolumes(file, outputEntryList);
    if (outputEntryList.empty()) {
        return;
    }
    std::string lvPath;
    std::string mountPoint;
    for (auto& it : outputEntryList) {
        bool isSupport = IsSupportSnapshot(it->mountPoint, lvPath, mountPoint);
        if (!isSupport) {
            continue;
        }
        LvmSnapshot snapshot(lvPath, mountPoint, "", "", "");
        volumeInfo.push_back(snapshot);
    }
}

int JfsSnapshotProvider::ExecShellCmd(const std::string& cmd,
    const std::vector<std::string>& paramList,
    std::vector<std::string>& shellOutput)
{
    std::vector<std::string> errOutput;
    HCP_Log(INFO, MODULE) << "execShellCmd param, cmd: " << cmd <<
        ", Module::CPath::GetInstance().GetRootPath() sh path: " <<
        Module::CPath::GetInstance().GetRootPath()<< HCPENDLOG;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, shellOutput, errOutput);
    INFOLOG("runshell ret:%d", ret);
    for (auto str : shellOutput) {
        INFOLOG("shellout:%s", str.c_str());
    }
    for (auto str : errOutput) {
        INFOLOG("errout:%s", str.c_str());
    }
    if (ret != 0) {
        std::string msg;
        for (auto &it : shellOutput) {
            msg += it + " ";
        }
        std::string errmsg;
        for (const auto &it : errOutput) {
            errmsg += it + " ";
        }
        HCP_Log(INFO, MODULE) << "run shell ret: " << ret << HCPENDLOG;
        HCP_Log(INFO, MODULE) << "run shell msg: " << msg << HCPENDLOG;
        HCP_Log(INFO, MODULE) << "run shell errmsg: " << errmsg<< HCPENDLOG;
    }
    return ret;
}

bool JfsSnapshotProvider::DeleteSnapshotByVolume(const std::string &snapVolumeName)
{
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(snapVolumeName);
    std::string cmd = "?/bin/jfsSnapshot.sh -dv '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0) {
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "delete snapshot by volume success,snapVolume:" << snapVolumeName << HCPENDLOG;
    return true;
}
// Determines whether a snapshot has been created, and returns the created snapshot.
std::shared_ptr<LvmSnapshot> JfsSnapshotProvider::CreateSnapshotByVolume(const std::string &volumeName,
    const std::string &volumePath, int& ret)
{
    HCP_Log(INFO, MODULE) << "CreateSnapshotByVolume,start create device volume:"
        << volumeName << HCPENDLOG;
    if (deviceVolumeSnapMap.count(volumeName) != 0) {
        return deviceVolumeSnapMap[volumeName];
    }
    INFOLOG("volumePath:%s", volumePath.c_str());
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumePath); // Original volume mount directory
    std::string cmd = "?/bin/jfsSnapshot.sh -cv '?'";
    ret = ExecShellCmd(cmd, paramList, output);
    if (ret != 0 || output.size() == 0) {
        INFOLOG("create jfs volume snapshot failed, volumeName:%s", volumeName);
        return nullptr;
    }
    vector<string> snapVolumeContents {};
    boost::algorithm::split(snapVolumeContents, output[0], boost::is_any_of(" "), boost::token_compress_on);
    if (snapVolumeContents.empty()) {
        INFOLOG("create volume snapshot failed, run shell output none");
        return nullptr;
    }
    string snapDeviceName = snapVolumeContents[snapVolumeContents.size()-1];
    string snapVolumePath;
    string volumeId = std::to_string(deviceMount->FindDevice(volumePath)->devNo);
    snapVolumePath.append(m_snapshotMountRoot).append("/").append(jobId).append("/")
        .append(volumeId).append(volumePath);
    INFOLOG("volumeName:%s, volumePath:%s, snapDevice:%s, snapPath:%s", volumeName.c_str(), volumePath.c_str(),
        snapDeviceName.c_str(), snapVolumePath.c_str());
    auto snapShot = std::make_shared<LvmSnapshot>(volumeName, volumePath, snapDeviceName, snapVolumePath, volumeId);
    INFOLOG("create jfs snapshot success:%s", volumeName);
    return snapShot;
}

std::string JfsSnapshotProvider::GetRealPath(const std::string& path) const
{
    std::string realPath;
    char normalizePath[PATH_MAX + 1] = { 0x00 };
    if (realpath(path.c_str(), normalizePath) != nullptr) {
        realPath = normalizePath;
    }
    return realPath;
}

std::string JfsSnapshotProvider::GetDirName(const std::string& path) const
{
    std::vector<char> copy(path.c_str(), path.c_str() + path.size() + 1);
    return dirname(copy.data());
}

bool JfsSnapshotProvider::IsDir(const std::string& path) const
{
    struct stat st {};
    return (lstat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode);
}
}
#endif
