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
#include "LinuxVolumeLivemount.h"

using namespace FilePlugin;

namespace {
    constexpr auto MODULE = "VolumeLivemount";
    const std::string VOLUME_LIVEMOUNT_CACHE_DIR_NAME = "volumelivemount";
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
    const std::string SYS_BOOT_VOLUME = "boot";
}

bool LinuxVolumeLivemount::PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo)
{
    m_nasShareMountTarget = PluginUtils::PathJoin(
        VOLUME_LIVEMOUNT_PATH_ROOT, m_cloneCopyId, "share");
    INFOLOG("create volume livemount share mount target %s", m_nasShareMountTarget.c_str());
    m_volumesMountTargetRoot = PluginUtils::PathJoin(
        VOLUME_LIVEMOUNT_PATH_ROOT, m_cloneCopyId, "volumes");
    INFOLOG("create volume livemount volumes mount target root %s", m_volumesMountTargetRoot.c_str());
    m_volumesMountRecordRoot = PluginUtils::PathJoin(
        PluginUtils::GetPathName(m_cacheRepo->path[0]), "volumelivemount", m_cloneCopyId, "records");
    INFOLOG("create volume livemount volumes mount record root %s", m_volumesMountRecordRoot.c_str());
    if (!PluginUtils::CreateDirectory(m_nasShareMountTarget)
        || !PluginUtils::CreateDirectory(m_volumesMountTargetRoot)
        || !PluginUtils::CreateDirectory(m_volumesMountRecordRoot)) {
        ERRLOG("failed to create basic volume livemount directory");
        return false;
    }
    m_dataPathRoot = m_nasShareMountTarget;
    INFOLOG("using data path %s, cloneCopyId %s, path: %s", m_dataPathRoot.c_str(), m_cloneCopyId.c_str(),
        extendInfo.dstPath.c_str());
    return true;
}

bool LinuxVolumeLivemount::MountVolumes()
{
    bool ret = true;
    INFOLOG("get volume directories in %s", m_dataPathRoot.c_str());
    std::vector<std::string> volumeNameList = GetAllvolumeNameList(m_dataPathRoot);
    for (const std::string& volumeName : volumeNameList) {
        std::string volumeDirPath = PluginUtils::PathJoin(m_dataPathRoot, volumeName);
        if (volumeName == SYS_BOOT_VOLUME) {
            INFOLOG("skip mount system boot volume path %s", volumeDirPath.c_str());
            continue;
        }
        INFOLOG("peform volume mount (%s) from data dir %s", volumeName.c_str(), volumeDirPath.c_str());
        if (!MountSingleVolumeCopy(volumeName, volumeDirPath)) {
            ERRLOG("failed to mount volume (%s) from data dir %s", volumeName.c_str(), volumeDirPath.c_str());
            ret = false;
        }
    }
    // hint:: no need to set to transactional, livemount allow partial success
    return ret;
}

bool LinuxVolumeLivemount::MountSingleVolumeCopy(const std::string& volumeName, const std::string& volumeDirPath)
{
    std::string volumeMountTarget = PluginUtils::PathJoin(m_volumesMountTargetRoot, volumeName);
    std::string outputRecordDirPath = PluginUtils::PathJoin(m_volumesMountRecordRoot, volumeName);
    std::string volumeMountArgsEntriesJsonPath = PluginUtils::PathJoin(volumeDirPath, VOLUME_MOUNT_ENTRIES_JSON_NAME);
    std::string volumeMountRecordPath;

    INFOLOG("mount volume (%s) from data dir %s, using target dir %s and record dir %s",
        volumeName.c_str(), volumeDirPath.c_str(), volumeMountTarget.c_str(), outputRecordDirPath.c_str());
    VolumeMountEntries volumeMountEntries {};
    if (!JsonFileTool::ReadFromFile(volumeMountArgsEntriesJsonPath, volumeMountEntries)) {
        ERRLOG("failed to read volume (%s) mount args entries json from %s",
            volumeName.c_str(), volumeMountArgsEntriesJsonPath.c_str());
        return false;
    }
    if (volumeMountEntries.mountEntries.empty()) {
        WARNLOG("volume (%s) mount args entries empty, won't mount, skip", volumeName.c_str());
        ReportJobLabel(
            JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_livemount_volume_no_mount_point",
            volumeMountEntries.volumePath);
        return true;
    }
    
    if (!MountSingleVolumeReadWrite(
        volumeName,
        volumeDirPath,
        volumeMountTarget,
        outputRecordDirPath,
        volumeMountRecordPath)) {
        ReportJobLabel(
            JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_livemount_volume_mount_failed",
            volumeMountEntries.volumePath, volumeMountTarget);
        return false;
    }
    m_mountedRecords.push_back(volumeMountRecordPath);
    ReportJobLabel(
        JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_mount_success",
        volumeMountEntries.volumePath, volumeMountTarget);
    return true;
}

bool LinuxVolumeLivemount::MountShare()
{
    std::string driveInfo;
    if (!MountNasShare(*m_dataRepo.get(), m_nasShareMountTarget, m_livemountPara->extendInfo, driveInfo)) {
        return false;
    }
    return true;
}