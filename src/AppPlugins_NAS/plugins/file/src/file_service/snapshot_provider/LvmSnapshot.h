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
#ifndef APPPLUGIN_FILE_SNAPSHOT_H
#define APPPLUGIN_FILE_SNAPSHOT_H
#include <unordered_map>
#include <string>
#include <vector>
#include <set>
/**
 * Represents metadata of a snapshot.
 */
namespace FilePlugin {
class LvmSnapshot {
public:
    LvmSnapshot(std::string oriDeviceVolume, std::string oriDeviceMountPath,
        std::string snapDeviceVolume, std::string snapDeviceMountPath, std::string snapshotId)
        :m_oriDeviceVolume (oriDeviceVolume),
        m_oriDeviceMountPath (oriDeviceMountPath),
        m_snapDeviceVolume (snapDeviceVolume),
        m_snapDeviceMountPath (snapDeviceMountPath),
        m_snapshotId (snapshotId) {}

    std::string m_oriDeviceVolume;
    std::string m_oriDeviceMountPath;
    std::string m_snapDeviceVolume;
    std::string m_snapDeviceMountPath;
    std::string m_snapshotId;
};

enum class SNAPSHOT_STATUS {
    FAILED = -1,    // supported or partial_supported but create snapshot failed
    SUCCESS = 0,    // all volumes or partial volumes are supported snapshot and create snapshot success
    UNSUPPORTED = 1 // all volumes are not supported
};
struct SnapshotResult {
    SNAPSHOT_STATUS snapShotStatus = SNAPSHOT_STATUS::FAILED;
    /**
    * key-打快照的目录
    * value-快照后的映射路径
    */
    std::unordered_map<std::string, std::string> snapshotsMapper;
    /**
    * key-打快照的目录
    * value- snapshot volumeDevice /dev/vg/lv
    */
    std::unordered_map<std::string, std::string> snapshotVolumeMapper;
    /**
    * key   -打快照的目录
    * value -快照卷ID
    */
    std::unordered_map<std::string, std::string> deviceVolumeSnapMap;
    std::set<std::string> spacelessVgs;
};

struct SnapshotDeleteResult {
    bool status = true;
    std::set<std::string> snapshots; // 删除失败的快照
};
}

#endif // APPPLUGIN_FILE_SNAPSHOT_H
