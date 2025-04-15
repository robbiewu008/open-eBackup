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
#ifndef APPPLUGIN_FILE_LVMSNAPSHOTPROVIDER_H
#define APPPLUGIN_FILE_LVMSNAPSHOTPROVIDER_H
#include <memory>
#include <mutex>
#include "SnapshotProvider.h"
#include "DeviceMount.h"

namespace FilePlugin {
class LvmSnapshotProvider : public SnapshotProvider {
public:
    LvmSnapshotProvider() {}
    LvmSnapshotProvider(
        std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId, const std::string& snapshotMountRoot);
    ~LvmSnapshotProvider() override {};
    SnapshotResult CreateSnapshot(const std::string& filePath, bool isCrossVolume,
        const std::string& snapshotPercent) override;
    SnapshotResult QuerySnapshot(const std::string& filePath) override;
    SnapshotDeleteResult DeleteAllSnapshots(const std::set<std::string>& snapshotInfos = {}) override;
    bool MountSnapshot(const std::string &volumeDevice, const std::string& mountPath) override;
    static bool DeleteSnapshotByVolume(const std::string& snapVolumeName);
    static std::shared_ptr<LvmSnapshot> CreateSnapshotByVolumeName(const std::string& snapshotMountRoot,
        const std::string& snapTag, const std::string& volumeName, const std::string& snapshotPercent, int& errorCode);
    static std::string GetLvmPath(const std::string& volumeDeviceName);

private:
    bool IsSupportSnapshot(const std::string& file, std::string &lvPath, std::string &mountPoint);
    bool GetSupportSnapVolume(const std::string& file, bool isCrossVolume,
    std::vector<LvmSnapshot> &volumeInfo);
    void GetSubVolumes(const std::string& file, std::vector<LvmSnapshot> &volumeInfo);
    std::string GetRealPath(const std::string& path) const;
    std::string GetDirName(const std::string& path) const;
    bool IsDir(const std::string& path) const;
    bool GetLogicalVolume(const std::string& path,  bool isCrossVolume,
                           std::vector<LvmSnapshot> &volumeInfo);
    std::shared_ptr<LvmSnapshot> CreateSnapshotByVolume(const std::string &volumeName,
        const std::string &volumePath, int& ret, const std::string &snapshotPercent);
    std::string ConvertSnapMountPath(const std::string& originalPath,
        const std::shared_ptr<LvmSnapshot>& snapshotInfo) const;
    std::string ConcatPath(const std::string& first, const std::string& second) const;
    bool DeleteSnapshotByTag(const std::string &snapTag);
    int ExecShellCmd(const std::string& cmd,
                    const std::vector<std::string>& paramList,
                    std::vector<std::string>& shellOutput);
private:
    // key:文件对应的原始volume:deviceVolume(eg: volumeGroup/volume1), value: 快照信息
    std::map<std::string, std::shared_ptr<LvmSnapshot>> deviceVolumeSnapMap;
    std::mutex mtxLock {};
    std::shared_ptr<DeviceMount> deviceMount;
    std::string jobId;
    std::string m_snapshotMountRoot; // snapshot mount root point
};
}

#endif // APPPLUGIN_FILE_LVMSNAPSHOTPROVIDER_H
