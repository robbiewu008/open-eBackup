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
#include "ZfsSnapshotProvider.h"
#include <libgen.h>
#include "log/Log.h"
#include "common/Path.h"
#include "system/System.hpp"
#include "constant/PluginConstants.h"
#include "utils/PluginUtilities.h"

using namespace std;

namespace FilePlugin {
namespace {
    constexpr auto MODULE = "ZfsSnapshotProvider";
    constexpr int VOLUME_SEGMENT_SIZE = 2;
    const string ZFS = "zfs";
    constexpr uint8_t SNAP_ERRNO_SPACELESS = 3;
    const std::string ZFS_SNAPSHOT_SEP_CHAR = "@";
}

ZfsSnapshotProvider::ZfsSnapshotProvider(shared_ptr<DeviceMount> deviceMount, const string &jobId)
    : m_deviceMount(deviceMount), m_jobId(jobId) {}

SnapshotResult ZfsSnapshotProvider::CreateSnapshot(const string& filePath, bool isCrossVolume)
{
    SnapshotResult snapshotResult;
    if (filePath.empty()) {
        return snapshotResult;
    }
    string realPath = PluginUtils::GetRealPath(filePath);
    if (realPath.empty()) {
        ERRLOG("CreateSnapshot failed, GetRealPath file: %s is not exist", filePath.c_str());
        return snapshotResult;
    }
    string fileDirPath = realPath;
    if (!PluginUtils::IsDir(fileDirPath)) {
        fileDirPath = PluginUtils::GetDirName(realPath);
        isCrossVolume = false;
    }
    // Get the fs/volume list of the original fs/volume and subfs/subvolume (it may not be supported by the
    // original fs/volume, but the subfs/subvolume supports snapshots)
    vector<LvmSnapshot> volumeInfo;
    GetZfsVolumesInfoR(fileDirPath, isCrossVolume, volumeInfo);
    if (volumeInfo.empty()) {
        snapshotResult.snapShotStatus = SNAPSHOT_STATUS::UNSUPPORTED;
        WARNLOG("The ZFS logical volume where the path is located is empty, path: %s", filePath.c_str());
        return snapshotResult;
    }
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    for (auto it = volumeInfo.begin(); it != volumeInfo.end(); ++it) {
        int ret = 0;
        shared_ptr<LvmSnapshot> snapshotPtr = CreateSnapshotByVolume(
            it->m_oriDeviceVolume, it->m_oriDeviceMountPath, ret);
        if (ret == SNAP_ERRNO_SPACELESS) {
            size_t pos = it->m_oriDeviceVolume.find('/');
            string zpoolName = it->m_oriDeviceVolume.substr(0, pos);
            ERRLOG("CreateSnapshotByVolume failed, zfs zpool [%s] doesn't have enough space!", zpoolName.c_str());
            snapshotResult.spacelessVgs.emplace(zpoolName);
            continue;
        }
        if (snapshotPtr == nullptr) {
            ERRLOG("CreateSnapshot failed");
            snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
            return snapshotResult;
        }
        string originalMntPoint = snapshotPtr->m_oriDeviceMountPath;
        string snapshotDevice = snapshotPtr->m_snapDeviceVolume;
        DBGLOG("Create snapshot success, realPath: %s, snapVol: %s", originalMntPoint.c_str(), snapshotDevice.c_str());
        snapshotResult.snapshotVolumeMapper.emplace(originalMntPoint, snapshotDevice);
        deviceVolumeSnapshotMap.emplace(snapshotPtr->m_oriDeviceVolume, snapshotPtr);
    }
    return snapshotResult;
}

SnapshotResult ZfsSnapshotProvider::QuerySnapshot(const string& filePath)
{
    ERRLOG("ZfsSnapshotProvider does not provide QuerySnapshot API");
    (void)filePath;
    SnapshotResult snapshotResult;
    return snapshotResult;
}

SnapshotDeleteResult ZfsSnapshotProvider::DeleteAllSnapshots(const set<string>& snapshotInfos)
{
    SnapshotDeleteResult snapshotResult;
    snapshotResult.status = true;
    DBGLOG("DeleteAllSnapshots, snapshot size: %d", snapshotInfos.size());
    // Delete created snapshots
    for (const string& deviceName : snapshotInfos) {
        if (!DeleteSnapshotByVolume(deviceName)) {
            snapshotResult.status = false;
            snapshotResult.snapshots.emplace(deviceName);
        }
    }
    deviceVolumeSnapshotMap.clear();
    return snapshotResult;
}

bool ZfsSnapshotProvider::MountSnapshot(const string &volumeDevice, const string& mountPath)
{
    return true;
}

void ZfsSnapshotProvider::GetZfsVolumesInfoR(const string& file, bool isCrossVolume, vector<LvmSnapshot> &volumeInfo)
{
    GetZfsVolumesInfo(file, volumeInfo);
    if (!isCrossVolume) {
        return;
    }
    vector<shared_ptr<FsDevice>> outputEntryList;
    m_deviceMount->GetSubVolumes(file, outputEntryList);
    if (outputEntryList.empty()) {
        return;
    }
    for (auto& subDevice : outputEntryList) {
        GetZfsVolumesInfo(subDevice->mountPoint, volumeInfo);
    }
}

void ZfsSnapshotProvider::GetZfsVolumesInfo(const string& file, vector<LvmSnapshot> &volumeInfo)
{
    shared_ptr<FsDevice> fsDevice = m_deviceMount->FindDevice(file);
    if (fsDevice == nullptr) {
        return;
    }
    if (fsDevice->fsType != ZFS) {
        WARNLOG("Not support zfs snapshot, file: %s, fsType: %s", file.c_str(), fsDevice->fsType.c_str());
        return;
    }
    LvmSnapshot snapshot(fsDevice->deviceName, fsDevice->mountPoint, "", "", "");
    volumeInfo.push_back(snapshot);
    return;
}

bool ZfsSnapshotProvider::DeleteSnapshotByTag(const string &snapTag)
{
    vector<string> output;
    vector<string> errput;
    vector<string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(snapTag); // snapTag
    string cmd = "?/bin/zfsSnapshot.sh -dtag '?'";
    int ret = Module::runShellCmdWithOutput(INFO, "ZfsSnapshotProvider", 0, cmd, paramList, output, errput);
    if (ret != 0) {
        ERRLOG("Delete snapshot by tag failed: %s, ret: %d", cmd.c_str(), ret);
        std::for_each(output.begin(), output.end(), [&] (const std::string& v) { ERRLOG("output: %s", v.c_str()); });
        std::for_each(errput.begin(), errput.end(), [&] (const std::string& v) { ERRLOG("errput: %s", v.c_str()); });
        return false;
    }
    DBGLOG("Delete snapshot by tag success, snapshot tag: %s", snapTag.c_str());
    return true;
}

bool ZfsSnapshotProvider::DeleteSnapshotByVolume(const string &snapVolumeName)
{
    vector<string> output;
    vector<string> errput;
    vector<string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(snapVolumeName);
    string cmd = "?/bin/zfsSnapshot.sh -dv '?'";
    int ret = Module::runShellCmdWithOutput(INFO, "ZfsSnapshotProvider", 0, cmd, paramList, output, errput);
    if (ret != 0) {
        ERRLOG("Delete snapshot by volume failed: %s, ret: %d", cmd.c_str(), ret);
        std::for_each(output.begin(), output.end(), [&] (const std::string& v) { ERRLOG("output: %s", v.c_str()); });
        std::for_each(errput.begin(), errput.end(), [&] (const std::string& v) { ERRLOG("errput: %s", v.c_str()); });
        return false;
    }
    DBGLOG("Delete snapshot by tag success, snapshot volume: %s", snapVolumeName.c_str());
    return true;
}

// Determines whether a snapshot has been created, and returns the created snapshot.
shared_ptr<LvmSnapshot> ZfsSnapshotProvider::CreateSnapshotByVolume(
    const string &volumeName, const string &volumeMntPoint, int& ret)
{
    DBGLOG("CreateSnapshotByVolume, start create device volume: %s", volumeName.c_str());
    if (deviceVolumeSnapshotMap.count(volumeName) != 0) {
        return deviceVolumeSnapshotMap[volumeName];
    }
    vector<string> output;
    vector<string> errput;
    vector<string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(volumeName);
    paramList.push_back(m_jobId); // snapTag
    string cmd = "?/bin/zfsSnapshot.sh -cv '?' '?'";
    ret = Module::runShellCmdWithOutput(INFO, "ZfsSnapshotProvider", 0, cmd, paramList, output, errput);
    if (ret != 0) {
        ERRLOG("Delete snapshot by volume failed: %s, ret: %d", cmd.c_str(), ret);
        std::for_each(output.begin(), output.end(), [&] (const std::string& v) { ERRLOG("output: %s", v.c_str()); });
        std::for_each(errput.begin(), errput.end(), [&] (const std::string& v) { ERRLOG("errput: %s", v.c_str()); });
        return nullptr;
    }
    string snapshotVolumeName = volumeName + ZFS_SNAPSHOT_SEP_CHAR + m_jobId;
    auto snapShot = make_shared<LvmSnapshot>(volumeName, volumeMntPoint, snapshotVolumeName, "", "");
    DBGLOG("create zfs volume snapshot success");
    return snapShot;
}
}
