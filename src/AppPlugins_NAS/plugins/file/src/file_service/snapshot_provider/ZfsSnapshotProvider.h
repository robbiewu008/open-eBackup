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
#ifndef APPPLUGIN_FILE_ZFSSNAPSHOTPROVIDER_H
#define APPPLUGIN_FILE_ZFSSNAPSHOTPROVIDER_H
#include <memory>
#include <mutex>
#include "SnapshotProvider.h"
#include "DeviceMount.h"

namespace FilePlugin {
class ZfsSnapshotProvider : public SnapshotProvider {
public:
    ZfsSnapshotProvider(std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId);
    ~ZfsSnapshotProvider() override {};
    SnapshotResult CreateSnapshot(const std::string& filePath, bool isCrossVolume) override;
    SnapshotDeleteResult DeleteAllSnapshots(const std::set<std::string>& snapshotInfos = {}) override;
    bool MountSnapshot(const std::string &volumeDevice, const std::string& mountPath) override;
    SnapshotResult QuerySnapshot(const std::string& filePath) override;

private:
    void GetZfsVolumesInfoR(const std::string& file, bool isCrossVolume, std::vector<LvmSnapshot>& volumeInfo);
    void GetZfsVolumesInfo(const std::string& file, std::vector<LvmSnapshot>& volumeInfo);
    std::shared_ptr<LvmSnapshot> CreateSnapshotByVolume(
        const std::string &volumeName, const std::string &volumeMntPoint, int& ret);
    bool DeleteSnapshotByVolume(const std::string &snapVolumeName);
    bool DeleteSnapshotByTag(const std::string &snapTag);
private:
    // key:文件对应的原始volume:deviceVolume(eg: mpool/fs), value: 快照信息
    std::map<std::string, std::shared_ptr<LvmSnapshot>> deviceVolumeSnapshotMap;
    std::shared_ptr<DeviceMount> m_deviceMount;
    std::string m_jobId;
    std::string m_snapshotMountRoot; // snapshot mount root point
};
}

#endif // APPPLUGIN_FILE_ZFSSNAPSHOTPROVIDER_H
