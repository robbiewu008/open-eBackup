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
#ifdef WIN32
#include "VssSnapshotProvider.h"
#include <regex>
#include <filesystem>
#include "FileSystemUtil.h"
#include "shlwapi.h"
#include "log/Log.h"
#include <iostream>
#include "common/Thread.h"

using namespace Module;
using namespace FileSystemUtil;
using namespace Win32VSSWrapper;
using namespace std;
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "VssSnapshotProvider";
    const string DEVICE_PREFIX = "/dev/";
    const int NUM0 = 0;
    const int TRYNUMES = 3;
    const int NUM60 = 60;
    constexpr int VOLUME_SEGMENT_SIZE = 2;
    const string SUPPORT_LVM_FS[] = {
        "NTFS",
    };
    constexpr uint8_t SNAP_ERRNO_SPACELESS = 3;
    constexpr uint8_t SPACE_PERCENTAGE = 5;
}

VssSnapshotProvider::VssSnapshotProvider(
    std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId, const std::string& snapshotMountRoot)
    : deviceMount(deviceMount), jobId(jobId), m_snapshotMountRoot(snapshotMountRoot)
{}

SnapshotResult VssSnapshotProvider::CreateSnapshot(const std::string& filePath, bool isCrossVolume)
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
        isCrossVolume = false;
    }
    // Get the LV list of the original volume and subvolume
    std::vector<LvmSnapshot> volumeInfo;
    bool isOriSupport = GetSupportSnapVolume(volumePath, isCrossVolume, volumeInfo);
    if (volumeInfo.empty()) {
        snapshotResult.snapShotStatus = SNAPSHOT_STATUS::UNSUPPORTED;
        WARNLOG("The logical volume where the path is located is empty, path: %s", filePath.c_str());
        return snapshotResult;
    }
    std::lock_guard<std::mutex> lock(mtxLock);
    snapshotResult.snapShotStatus = SNAPSHOT_STATUS::SUCCESS;
    for (const LvmSnapshot& volume : volumeInfo) {
        int ret = 0;
        std::shared_ptr<LvmSnapshot> snapshotPtr = CreateSnapshotByVolume(volume.m_oriDeviceVolume,
            volume.m_oriDeviceMountPath, ret);
        if (ret == SNAP_ERRNO_SPACELESS) {
            string vgName = deviceMount->FindDevice(volume.m_oriDeviceMountPath)->mountPoint;
            ERRLOG("CreateSnapshotByVolume failed, lvm vg [%s] doesn't have enough space!", vgName.c_str());
            snapshotResult.spacelessVgs.emplace(vgName);
            continue;
        }
        if (snapshotPtr == nullptr) {
            ERRLOG("CreateSnapshotByVolume failed");
            snapshotResult.snapShotStatus = SNAPSHOT_STATUS::FAILED;
            return snapshotResult;
        }
        DBGLOG("snapshotsMapper emplace, realPath: %s, snapMountPath: %s",
            snapshotPtr->m_oriDeviceMountPath.c_str(), snapshotPtr->m_snapDeviceMountPath.c_str());
        INFOLOG("origin path:%s, volume mount path:%s, snap volume id:%s", snapshotPtr->m_oriDeviceMountPath.c_str(),
            snapshotPtr->m_snapDeviceMountPath.c_str(), snapshotPtr->m_snapshotId.c_str());
        snapshotResult.snapshotsMapper.emplace(snapshotPtr->m_oriDeviceMountPath, snapshotPtr->m_snapDeviceMountPath);
        snapshotResult.snapshotVolumeMapper.emplace(snapshotPtr->m_oriDeviceMountPath,
                                                    snapshotPtr->m_snapDeviceVolume);
        deviceVolumeSnapMap.emplace(snapshotPtr->m_oriDeviceVolume, snapshotPtr);
        snapshotResult.deviceVolumeSnapMap.emplace(snapshotPtr->m_oriDeviceMountPath, snapshotPtr->m_snapshotId);
    }
    return snapshotResult;
}

SnapshotResult VssSnapshotProvider::QuerySnapshot(const std::string& filePath)
{
    SnapshotResult snapshotResult;
    std::string realPath = GetRealPath(filePath);
    if (realPath.empty()) {
        HCP_Log(ERR, MODULE) << "QuerySnapshot failed,GetRealPath file:"<< filePath <<
            " is not exist" << HCPENDLOG;
        return snapshotResult;
    }
    string volumePath = realPath;
    // Get the LVM logical volume of the file system where volumePath is located, without cross-volume
    std::vector<LvmSnapshot> volumeInfo;
    bool isExistVolume = GetSupportSnapVolume(volumePath, false, volumeInfo);
    if (!isExistVolume || volumeInfo.empty()) {
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

SnapshotDeleteResult VssSnapshotProvider::DeleteAllSnapshots(const std::set<std::string>& snapshotInfos)
{
    std::lock_guard<std::mutex> lock(mtxLock);
    SnapshotDeleteResult snapshotResult;
    snapshotResult.status = true;
    DBGLOG("DeleteAllSnapshots, snapshot size:%d", snapshotInfos.size());
    for (const std::string& snapId : snapshotInfos) {
        if (!DeleteSnapshotByVolume(snapId)) {
            ERRLOG("snapId:%s delete failed", snapId.c_str());
            snapshotResult.snapshots.emplace(snapId);
            snapshotResult.status = false;
            continue;
        }
        INFOLOG("snapId:%s has been deleted", snapId.c_str());
    }
    std::string mountParentPath;
    mountParentPath.append(m_snapshotMountRoot).append("\\").append(jobId);
    bool ret = true;
    try {
        ret = std::filesystem::remove_all(mountParentPath);
    }
    catch (const std::exception& e) {
        ERRLOG("remove %s failed, exception", mountParentPath.c_str());
    }
    if (!ret) {
        ERRLOG("delete vss snapshot dir failed:%s", mountParentPath.c_str());
    }
    return snapshotResult;
}

bool VssSnapshotProvider::MountSnapshot(const std::string &volumeDevice, const std::string& mountPath)
{
    HCP_Log(INFO, MODULE) << "MountSnapshot,snapshot volumeDevice:"<<
        volumeDevice << ",mountPath:"<< mountPath << HCPENDLOG;
    if (volumeDevice.empty() || mountPath.empty()) {
        HCP_Log(ERR, MODULE) << "volumeDevice or mountPath is emtpy:" << HCPENDLOG;
        return false;
    }
    VssClient client;
    if (!client.ExposeSnapshotLocally(volumeDevice, mountPath)) {
        HCP_Log(ERR, MODULE) << "MountSnapshot failed, volumeDevice:" << volumeDevice
            << ",mountPath:" << mountPath << HCPENDLOG;
        return false;
    }
    return true;
}

bool VssSnapshotProvider::IsSupportSnapshot(const std::string& file, std::string &lvPath, std::string &mountPoint)
{
    /*
    1. 判断文件系统类型是否支持
    */
    std::shared_ptr<FsDevice> fsDevice = deviceMount->FindDevice(file);
    if (fsDevice == nullptr) {
        return false;
    }
    bool isSupportVss = false;
    int fsCnt = sizeof(SUPPORT_LVM_FS) / sizeof(SUPPORT_LVM_FS[0]);
    for (int i = 0; i < fsCnt; i++) {
        if (fsDevice->fsType == SUPPORT_LVM_FS[i]) {
            isSupportVss = true;
            break;
        }
    }
    if (!isSupportVss) {
        HCP_Log(WARN, MODULE) << "Not support vss snapshot, file:" << file <<
            ",fsType:"<< fsDevice->fsType << HCPENDLOG;
        return false;
    }
    mountPoint = fsDevice->mountPoint;
    lvPath = std::to_string(fsDevice->devNo);
    return true;
}

bool VssSnapshotProvider::GetSupportSnapVolume(const std::string& file, bool isCrossVolume,
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

void VssSnapshotProvider::GetSubVolumes(const std::string& file,
    std::vector<LvmSnapshot> &volumeInfo)
{
    std::vector<std::shared_ptr<FsDevice>> outputEntryList;
    deviceMount->GetSubVolumes(file, outputEntryList);
    if (outputEntryList.empty()) {
        DBGLOG("path:%s has no subvolumes", file.c_str());
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
        DBGLOG("get subsnapshot mountPoint:%s", mountPoint.c_str());
        volumeInfo.push_back(snapshot);
    }
}

bool VssSnapshotProvider::DeleteSnapshotByTag(const std::string &snapTag)
{
    return true;
}

bool VssSnapshotProvider::DeleteSnapshotByVolume(const std::string &snapVolumeName)
{
    HCP_Log(DEBUG, MODULE) << "delete snapshot by volume success,snapVolume:" << snapVolumeName << HCPENDLOG;
    VssClient client;
    if (!client.DeleteSnapshot(snapVolumeName)) {
        HCP_Log(ERROR, MODULE) << "delete snapshot by volume failed,snapVolume:" << snapVolumeName << HCPENDLOG;
        return false;
    }
    return true;
}

bool VssSnapshotProvider::SpaceRemainRat(const std::string &path)
{
    ULARGE_INTEGER totalBytes;
    ULARGE_INTEGER freeBytes;
    if (!GetDiskFreeSpaceExW(Utf8ToUtf16(path).c_str(), nullptr, &totalBytes, &freeBytes)) {
        ERRLOG("Get disk free space failed of path:%s", path.c_str());
        return false;
    }
    uint64_t per = freeBytes.QuadPart * 100 / totalBytes.QuadPart;
    return per < SPACE_PERCENTAGE ? false : true;
}

// Determines whether a snapshot has been created, and returns the created snapshot.
std::shared_ptr<LvmSnapshot> VssSnapshotProvider::CreateSnapshotByVolume(const std::string &volumeName,
    const std::string &volumePath, int& ret)
{
    INFOLOG("CreateSnapshotByVolume,start create device volume: %s", volumePath.c_str());
    if (deviceVolumeSnapMap.count(volumeName) != 0) {
        return deviceVolumeSnapMap[volumeName];
    }
    if (volumePath.empty()) {
        DBGLOG("%s is null", volumePath.c_str());
        return nullptr;
    }
    if (!SpaceRemainRat(volumePath)) {
        DBGLOG("volumePath %s space is not enough", volumePath.c_str());
        ret = SNAP_ERRNO_SPACELESS;
        return nullptr;
    }
    std::vector<std::string> volumePathList {volumePath};
    std::optional<SnapshotSetResult> snapRes;
    bool snapCreateRes = false;
    VssClient client;
    for (int i = NUM0; i < TRYNUMES; i++) {
        snapRes = client.CreateSnapshots(volumePathList);
        if (snapRes) {
            INFOLOG("Create Snapshot for %s success", volumePath.c_str());
            snapCreateRes = true;
            break;
        }
        Module::SleepFor(std::chrono::seconds(NUM60));
        INFOLOG("Create Snapshot for %s failed, try again and wait for 60s", volumePath.c_str());
    }
    if (!snapCreateRes) {
        ERRLOG("Create Snapshot for %s failed", volumePath.c_str());
        return nullptr;
    }
    std::string driverLetter = volumePath.substr(0, 1) + volumePath.substr(2);
    // 盘符转为小写，防止细粒度恢复盘符被置为大写
    driverLetter[0] = driverLetter[0] + 'a' - 'A';
    DBGLOG("driverLetter:%s", driverLetter.c_str());
    std::string mountPoint;
    string volumeId =  snapRes.value().SnapshotIDList()[0];
    mountPoint.append(m_snapshotMountRoot).append("\\").append(jobId).append("\\").
        append(volumeName).append("\\").append(driverLetter);
    HCP_Log(DEBUG, MODULE) << "mountPoint:" << mountPoint << HCPENDLOG;
    if (!std::filesystem::create_directories(mountPoint)) {
        HCP_Log(ERROR, MODULE) << "create mountPoint Directory failed" << HCPENDLOG;
    }
    std::string snapVolumeName = Utf16ToUtf8(client.GetSnapshotProperty(volumeId)
        .value().SnapshotDeviceObjectW()).append("\\");
    MountSnapshot(volumeId, mountPoint);
    auto snapShot = std::make_shared<LvmSnapshot>(volumeName, volumePath, snapVolumeName, mountPoint, volumeId);
    HCP_Log(DEBUG, MODULE) << "create lvm volume snapshot success:" << HCPENDLOG;
    return snapShot;
}

std::string VssSnapshotProvider::ConcatPath(const std::string& first, const std::string& second)
{
    if (second.empty()) {
        return first;
    }
    std::string secondPath = second;
    if (secondPath[0] == '\\') {
        secondPath.erase(0, 1);
    }
    if (first[first.size() - 1] == '\\') {
        return first + secondPath;
    }
    return first + "\\" + secondPath;
}

std::string VssSnapshotProvider::ConvertSnapMountPath(const std::string& originalPath,
                                                      const std::shared_ptr<LvmSnapshot>& snapshotInfo)
{
    // Get the mount path of the original file, and the mount path of the snapshot
    std::string originMountPath = snapshotInfo->m_oriDeviceMountPath;
    std::string::size_type nPos = originalPath.find(originMountPath);
    std::string pathSufix;
    if (nPos != std::string::npos) {
        pathSufix = originalPath.substr(nPos + originMountPath.length(), originalPath.length());
    } else {
        // HCP_Log(ERR, MODULE) << "ConvertSnapMountPath failed,originalPath file:"<< originalPath <<
        //     ",originMountPath:" << originMountPath << HCPENDLOG;
        return "";
    }
    // The splicing path (snapMountPath + pathSufix) is used as the snapshot path
    return ConcatPath(snapshotInfo->m_snapDeviceMountPath, pathSufix);
}

std::string VssSnapshotProvider::GetRealPath(const std::string& path)
{
    return GetFullPath(path);
}

bool VssSnapshotProvider::IsDir(const std::string& path)
{
    return std::filesystem::is_directory(path);
}
}
#endif
