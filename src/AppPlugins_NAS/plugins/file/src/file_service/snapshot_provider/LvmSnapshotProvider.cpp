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
#include "LvmSnapshotProvider.h"
#include <libgen.h>
#include "log/Log.h"
#include "common/Path.h"
#include "utils/PluginUtilities.h"
#include "system/System.hpp"
#include "config_reader/ConfigIniReader.h"

using namespace std;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "LvmSnapshotProvider";
    const string DEVICE_PREFIX = "/dev/";
    constexpr int VOLUME_SEGMENT_SIZE = 2;
    const string SUPPORT_LVM_FS[] = {
        "ext2",
        "ext3",
        "ext4",
        "xfs"
    };
    const std::string FILE_PLUGIN_CONFIG_KEY = "FilePluginConfig";
    constexpr uint8_t SNAP_ERRNO_SPACELESS = 3;
}

LvmSnapshotProvider::LvmSnapshotProvider(
    std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId, const std::string& snapshotMountRoot)
    : deviceMount(deviceMount), jobId(jobId), m_snapshotMountRoot(snapshotMountRoot)
{}

SnapshotResult LvmSnapshotProvider::CreateSnapshot(const std::string& filePath, bool isCrossVolume)
{
    SnapshotResult snapshotResult;
    if (filePath.empty()) {
        return snapshotResult;
    }
    std::string realPath = GetRealPath(filePath);
    if (realPath.empty()) {
        ERRLOG("CreateSnapshot failed, GetRealPath file: %s is not exist", filePath.c_str());
        return snapshotResult;
    }
    string volumePath = realPath;
    if (!IsDir(volumePath)) {
        volumePath = GetDirName(volumePath);
        isCrossVolume = false;
    }
    // Get the LV list of the original volume and subvolume (it may not be supported by the original volume,
    // but the subvolume supports snapshots)
    std::vector<LvmSnapshot> volumeInfo;
    bool isOriSupport = GetSupportSnapVolume(volumePath, isCrossVolume, volumeInfo);
    if (volumeInfo.size() == 0) {
        snapshotResult.snapShotStatus = SNAPSHOT_STATUS::UNSUPPORTED;
        WARNLOG("The LVM logical volume where the path is located is empty, path: %s", filePath.c_str());
        return snapshotResult;
    }
    std::lock_guard<std::mutex> lock(mtxLock);
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    for (auto it = volumeInfo.begin(); it != volumeInfo.end(); ++it) {
        int ret = 0;
        std::shared_ptr<LvmSnapshot> snapshotPtr = CreateSnapshotByVolume((*it).m_oriDeviceVolume,
            (*it).m_oriDeviceMountPath, ret);
        if (ret == SNAP_ERRNO_SPACELESS) {
            size_t pos = (*it).m_oriDeviceVolume.find('/');
            string vgName = (*it).m_oriDeviceVolume.substr(0, pos);
            ERRLOG("CreateSnapshotByVolume failed, lvm vg [%s] doesn't have enough space!", vgName.c_str());
            snapshotResult.spacelessVgs.emplace(vgName);
            continue;
        }
        if (snapshotPtr == nullptr) {
            ERRLOG("CreateSnapshot failed");
            snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
            return snapshotResult;
        }
        DBGLOG("snapshotsMapper emplace, realPath: %s, snapMountPath: %s",
            snapshotPtr->m_oriDeviceMountPath.c_str(), snapshotPtr->m_snapDeviceMountPath.c_str());
        snapshotResult.snapshotsMapper.emplace(snapshotPtr->m_oriDeviceMountPath,
                                               snapshotPtr->m_snapDeviceMountPath);
        snapshotResult.snapshotVolumeMapper.emplace(snapshotPtr->m_oriDeviceMountPath,
                                                    DEVICE_PREFIX + snapshotPtr->m_snapDeviceVolume);
        deviceVolumeSnapMap.emplace(snapshotPtr->m_oriDeviceVolume, snapshotPtr);
    }
    return snapshotResult;
}

SnapshotResult LvmSnapshotProvider::QuerySnapshot(const std::string& filePath)
{
    SnapshotResult snapshotResult;
    std::string realPath = GetRealPath(filePath);
    if (realPath.empty()) {
        HCP_Log(ERR, MODULE) << "QuerySnapshot failed,GetRealPath file:"<< filePath <<
            " is not exist" << HCPENDLOG;
        return snapshotResult;
    }
    string volumePath = realPath;
    if (!IsDir(volumePath)) {
        volumePath = GetDirName(volumePath);
    }
    // Get the LVM logical volume of the file system where volumePath is located, without cross-volume
    std::vector<LvmSnapshot> volumeInfo;
    bool isExistVolume = GetSupportSnapVolume(volumePath, false, volumeInfo);
    if (!isExistVolume || volumeInfo.size() == 0) {
        HCP_Log(WARN, MODULE) << "The LVM logical volume where the path is located is empty,path:"
            << filePath << HCPENDLOG;
        return snapshotResult;
    }
    std::string volumeName = volumeInfo[0].m_oriDeviceVolume;
    std::lock_guard<std::mutex> lock(mtxLock);
    if (deviceVolumeSnapMap.count(volumeName) == 0) {
        HCP_Log(WARN, MODULE) << "The LVM logical volume has not create snapshot,volumeName:"
            << volumeName << ",path:" << filePath << HCPENDLOG;
        return snapshotResult;
    }
    std::shared_ptr<LvmSnapshot> snapshotPtr = deviceVolumeSnapMap[volumeName];
    if (snapshotPtr == nullptr) {
        return snapshotResult;
    }
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    std::string snapMountPath = ConvertSnapMountPath(realPath, snapshotPtr);
    DBGLOG("snapshotsMapper emplace, realPath: %s, snapMountPath: %s", realPath.c_str(), snapMountPath.c_str());
    snapshotResult.snapshotsMapper.emplace(realPath, snapMountPath);
    return snapshotResult;
}

SnapshotDeleteResult LvmSnapshotProvider::DeleteAllSnapshots(const std::set<std::string>& snapshotInfos)
{
    std::lock_guard<std::mutex> lock(mtxLock);
    SnapshotDeleteResult snapshotResult;
    snapshotResult.status = true;
    DBGLOG("DeleteAllSnapshots,snapshot size: %d", snapshotInfos.size());
    std::string mountParentPath;
    mountParentPath.append(m_snapshotMountRoot).append("/").append(jobId);
    // Delete created snapshots
    for (const std::string& deviceName : snapshotInfos) {
        if (!DeleteSnapshotByVolume(deviceName)) {
            snapshotResult.status = false;
            snapshotResult.snapshots.emplace(deviceName);
        }
    }
    PluginUtils::Remove(mountParentPath);
    deviceVolumeSnapMap.clear();
    return snapshotResult;
}

bool LvmSnapshotProvider::MountSnapshot(const std::string &volumeDevice, const std::string& mountPath)
{
    if (volumeDevice.empty() || mountPath.empty()) {
        HCP_Log(ERR, MODULE) << "volumeDevice or mountPath is emtpy:" << HCPENDLOG;
        return false;
    }
    DBGLOG("MountSnapshot, snapshot volumeDevice: %s, mountPath:%s ", volumeDevice.c_str(), mountPath.c_str());
    if (strncmp(volumeDevice.c_str(), DEVICE_PREFIX.c_str(), DEVICE_PREFIX.length()) != 0) {
        HCP_Log(ERR, MODULE) << "Strange volumeDevice:" << volumeDevice << HCPENDLOG;
        return false;
    }
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeDevice);
    paramList.push_back(mountPath);
    std::string cmd = "?/bin/lvmSnapshot.sh -mount '?' '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0) {
        HCP_Log(ERR, MODULE) << "MountSnapshot failed,volumeDevice:" <<
            volumeDevice <<",mountPath:"<< mountPath << HCPENDLOG;
        return false;
    }
    return true;
}

bool LvmSnapshotProvider::IsSupportSnapshot(const std::string& file, std::string &lvPath, std::string &mountPoint)
{
    std::shared_ptr<FsDevice> fsDevice = deviceMount->FindDevice(file);
    if (fsDevice == nullptr) {
        return false;
    }
    bool isSupportLvm = false;
    int fsCnt = sizeof(SUPPORT_LVM_FS) / sizeof(SUPPORT_LVM_FS[0]);
    for (int i = 0; i < fsCnt; i++) {
        if (fsDevice->fsType == SUPPORT_LVM_FS[i]) {
            isSupportLvm = true;
            break;
        }
    }
    if (!isSupportLvm) {
        HCP_Log(WARN, MODULE) << "Not support lvm snapshot, file:" << file <<
            ",fsType:"<< fsDevice->fsType << HCPENDLOG;
        return false;
    }
    mountPoint = fsDevice->mountPoint;
    // Whether to call the lvs command
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
    paramList.push_back(fsDevice->deviceName); // device /dev/mapper/vg-lv
    std::string cmd = "?/bin/lvmSnapshot.sh -ld '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0 || output.size() == 0) {
        HCP_Log(WARN, MODULE) << "Not spport lvm snapshot, file:" << file <<
        ",device:"<< fsDevice->deviceName << HCPENDLOG;
        fsDevice->isSupportSnapshot = false;
        return false;
    }
    HCP_Log(DEBUG, MODULE) << "file:" << file << ",lvPath:" << output[0] << HCPENDLOG;
    if (output[0].empty()) {
        return false;
    }
    lvPath = output[0];
    fsDevice->isSupportSnapshot = true;
    fsDevice->lvPath = lvPath;
    return true;
}
bool LvmSnapshotProvider::GetSupportSnapVolume(const std::string& file, bool isCrossVolume,
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

void LvmSnapshotProvider::GetSubVolumes(const std::string& file,
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

int LvmSnapshotProvider::ExecShellCmd(const std::string& cmd,
    const std::vector<std::string>& paramList,
    std::vector<std::string>& shellOutput)
{
    std::vector<std::string> errOutput;
    HCP_Log(INFO, MODULE) << "execShellCmd param, cmd: " << cmd <<
        ", Module::CPath::GetInstance().GetRootPath() sh path: " <<
        Module::CPath::GetInstance().GetRootPath()<< HCPENDLOG;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, shellOutput, errOutput);
    if (ret != 0) {
        std::string msg;
        for (auto &it : shellOutput) {
            msg += it + " ";
        }
        std::string errmsg;
        for (const auto &it : errOutput) {
            errmsg += it + " ";
        }
        ERRLOG("run shell ret: %d", ret);
        ERRLOG("run shell msg: %s", msg.c_str());
        ERRLOG("run shell errmsg: %s", errmsg.c_str());
    }
    return ret;
}
bool LvmSnapshotProvider::DeleteSnapshotByTag(const std::string &snapTag)
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

bool LvmSnapshotProvider::DeleteSnapshotByVolume(const std::string &snapVolumeName)
{
    std::vector<std::string> paramList;
    INFOLOG("start to delete snapshot %s, %s",
        Module::CPath::GetInstance().GetRootPath().c_str(), snapVolumeName.c_str());
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(snapVolumeName);
    std::string cmd = "?/bin/lvmSnapshot.sh -dv '?'";
    int runRet = PluginUtils::RunShellCmd(cmd, paramList);
    if (runRet != 0) {
        return false;
    }
    INFOLOG("delete snapshot by volume success, snapVolume: %s", snapVolumeName.c_str());
    return true;
}

// Determines whether a snapshot has been created, and returns the created snapshot.
std::shared_ptr<LvmSnapshot> LvmSnapshotProvider::CreateSnapshotByVolume(const std::string &volumeName,
    const std::string &volumePath, int& ret)
{
    DBGLOG("CreateSnapshotByVolume, start create device volume: %s", volumeName.c_str());
    if (deviceVolumeSnapMap.count(volumeName) != 0) {
        return deviceVolumeSnapMap[volumeName];
    }
    std::string mountPathPrefix;
    string volumeId = deviceMount->GetVolumeId(volumePath);
    mountPathPrefix.append(m_snapshotMountRoot).append("/").append(jobId).append("/").append(volumeId);
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeName);
    paramList.push_back(mountPathPrefix);
    paramList.push_back(jobId); // snapTag
    paramList.push_back(volumePath); // Original volume mount directory
    std::string cmd = "?/bin/lvmSnapshot.sh -cv '?' '?' '?' '?'";
    ret = ExecShellCmd(cmd, paramList, output);
    if (ret != 0 || output.size() == 0) {
        ERRLOG("create lvm volume snapshot failed, volumeName: %s", volumeName.c_str());
        return nullptr;
    }
    vector<string> snapVolumeContents {};
    boost::algorithm::split(snapVolumeContents, output[0], boost::is_any_of(":"), boost::token_compress_on);
    if (snapVolumeContents.size() != VOLUME_SEGMENT_SIZE) {
        DBGLOG("create volume snapshot failed, run shell ouput: %d", snapVolumeContents.size());
        return nullptr;
    }
    auto snapShot = std::make_shared<LvmSnapshot>(volumeName, volumePath,
        snapVolumeContents[0], snapVolumeContents[1], "");
    DBGLOG("create lvm volume snapshot success");
    return snapShot;
}

std::shared_ptr<LvmSnapshot> LvmSnapshotProvider::CreateSnapshotByVolumeName(
    const std::string& snapshotMountRoot, const std::string& snapTag, const std::string& volumeName, int& errorCode)
{
    std::string snapshotPercent
        = Module::ConfigReader::getString(FILE_PLUGIN_CONFIG_KEY, "LVM_SNAPSHOT_CAPACITY_PERCENT");
    std::string mountPathPrefix;
    mountPathPrefix.append(snapshotMountRoot).append("/").append(snapTag);
    std::vector<std::string> paramList;
    INFOLOG("CreateSnapshotByVolumeName, volumeName: %s, snapPercent %s, mountPathPrefix %s, snapTag %s",
        volumeName.c_str(), snapshotPercent.c_str(), mountPathPrefix.c_str(), snapTag.c_str());
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeName);
    paramList.push_back(mountPathPrefix);
    paramList.push_back(snapTag); // snapTag
    paramList.push_back(volumeName); // 卷备份的卷可能没有被挂载，因此使用卷名作为挂载点
    paramList.push_back(snapshotPercent);  // 默认快照百分比大小
    std::string cmd = "?/bin/lvmSnapshot.sh -cv '?' '?' '?' '?' '?'";
    std::vector<std::string> output;
    errorCode = PluginUtils::RunShellCmd(cmd, paramList, output);
    if (errorCode != 0 || output.size() == 0) {
        ERRLOG("create lvm volume snapshot failed, volumeName: %s", volumeName.c_str());
        return nullptr;
    }
    vector<string> snapVolumeContents {};
    boost::algorithm::split(snapVolumeContents, output[0], boost::is_any_of(":"), boost::token_compress_on);
    if (snapVolumeContents.size() != VOLUME_SEGMENT_SIZE) {
        DBGLOG("create volume snapshot failed, run shell ouput: %d", snapVolumeContents.size());
        return nullptr;
    }
    auto snapShot = std::make_shared<LvmSnapshot>(volumeName, "",
        DEVICE_PREFIX + snapVolumeContents[0], snapVolumeContents[1], "");
    DBGLOG("create lvm volume snapshot success");
    return snapShot;
}

std::string LvmSnapshotProvider::GetLvmPath(const std::string& volumeDeviceName)
{
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeDeviceName);
    std::string cmd = "?/bin/lvmSnapshot.sh -lvpath '?'";
    std::vector<std::string> output;
    int errorCode = PluginUtils::RunShellCmd(cmd, paramList, output);
    if (errorCode != 0 || output.size() == 0) {
        ERRLOG("Get lvm path failed, volumeDeviceName: %s", volumeDeviceName.c_str());
        return "";
    }
    std::string lvmPath = output[0];
    DBGLOG("Get lvm path success, lvm path: %s, volumeDeviceName: %s", lvmPath.c_str(), volumeDeviceName.c_str());
    return lvmPath;
}

bool LvmSnapshotProvider::GetLogicalVolume(const std::string& path,  bool isCrossVolume,
                                           std::vector<LvmSnapshot> &volumeInfo)
{
    std::string crossVolume = isCrossVolume ? "true" : "false";
    // Query the valid LVM logical volume of the file system where the specified directory is located,
    // and the shell returns the result in the form: volumeName:mountPath
    std::vector<std::string> output;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(path);
    paramList.push_back(crossVolume);
    std::string cmd = "?/bin/lvmSnapshot.sh -lv '?' '?'";
    int runRet = ExecShellCmd(cmd, paramList, output);
    if (runRet != 0 || output.size() == 0) {
        HCP_Log(WARN, MODULE) << "GetLogicalVolume failed, path:" << path << HCPENDLOG;
        return false;
    }

    vector<string> volumeContents {};
    for (auto it = output.begin(); it != output.end(); ++it) {
        HCP_Log(DEBUG, MODULE) << "path:" << path << ",volumeInfo:" << *it << HCPENDLOG;
        boost::algorithm::split(volumeContents, *it, boost::is_any_of(":"), boost::token_compress_on);
        HCP_Log(DEBUG, MODULE) << "volumeContents size:"<<volumeContents.size() << HCPENDLOG;
        if (volumeContents.size() == VOLUME_SEGMENT_SIZE) {
            LvmSnapshot snapshot(volumeContents[0], volumeContents[1], "", "", "");
            volumeInfo.push_back(snapshot);
            HCP_Log(DEBUG, MODULE) << "volumeContent[0]:"<< volumeContents[0] <<
                "contents[1]:"<<volumeContents[1]<< HCPENDLOG;
        }
        volumeContents.clear();
    }
    return !volumeInfo.empty();
}

std::string LvmSnapshotProvider::ConcatPath(const std::string& first, const std::string& second) const
{
    if (second.empty()) {
        return first;
    }
    std::string secondPath = second;
    if (secondPath[0] == '/') {
        secondPath.erase(0, 1);
    }
    if (first[first.size() - 1] == '/') {
        return first + secondPath;
    }
    return first + "/" + secondPath;
}

std::string LvmSnapshotProvider::ConvertSnapMountPath(const std::string& originalPath,
                                                      const std::shared_ptr<LvmSnapshot>& snapshotInfo) const
{
    // Get the mount path of the original file, and the mount path of the snapshot
    std::string originMountPath = snapshotInfo->m_oriDeviceMountPath;
    std::string::size_type nPos = originalPath.find(originMountPath);
    std::string pathSufix;
    if (nPos != std::string::npos) {
        pathSufix = originalPath.substr(nPos + originMountPath.length(), originalPath.length());
    } else {
        HCP_Log(ERR, MODULE) << "ConvertSnapMountPath failed,originalPath file:"<< originalPath <<
            ",originMountPath:" << originMountPath << HCPENDLOG;
        return "";
    }
    // The splicing path (snapMountPath + pathSufix) is used as the snapshot path
    return ConcatPath(snapshotInfo->m_snapDeviceMountPath, pathSufix);
}

std::string LvmSnapshotProvider::GetRealPath(const std::string& path) const
{
    std::string realPath;
    char normalizePath[PATH_MAX + 1] = { 0x00 };
    if (realpath(path.c_str(), normalizePath) != nullptr) {
        realPath = normalizePath;
    }
    return realPath;
}

std::string LvmSnapshotProvider::GetDirName(const std::string& path) const
{
    std::vector<char> copy(path.c_str(), path.c_str() + path.size() + 1);
    return dirname(copy.data());
}

bool LvmSnapshotProvider::IsDir(const std::string& path) const
{
    struct stat st {};
    return (lstat(path.c_str(), &st) == 0) && S_ISDIR(st.st_mode);
}
}
