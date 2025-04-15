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
#ifndef APPPLUGIN_FILE_VSSSNAPSHOTPROVIDER_H
#define APPPLUGIN_FILE_VSSSNAPSHOTPROVIDER_H
#include <memory>
#include <mutex>
#include "SnapshotProvider.h"
#include "DeviceMount.h"
#include "VssClient.h"
#include "define/Defines.h"

namespace FilePlugin {
class AGENT_API VssSnapshotProvider : public SnapshotProvider {
public:
	VssSnapshotProvider(
        std::shared_ptr<DeviceMount> deviceMount, const std::string &jobId, const std::string& snapshotMountRoot);
    ~VssSnapshotProvider() override {};
    SnapshotResult CreateSnapshot(const std::string& filePath, bool isCrossVolume,
        const std::string& snapshotPercent) override;
    SnapshotResult QuerySnapshot(const std::string& filePath) override;
    SnapshotDeleteResult DeleteAllSnapshots(const std::set<std::string>& snapshotInfos = {}) override;
    bool MountSnapshot(const std::string &volumeDevice, const std::string& mountPath) override;
private:
    bool IsSupportSnapshot(const std::string& file, std::string &lvPath, std::string &mountPoint);
    bool GetSupportSnapVolume(const std::string& file, bool isCrossVolume,
    std::vector<LvmSnapshot> &volumeInfo);
    void GetSubVolumes(const std::string& file, std::vector<LvmSnapshot> &volumeInfo);
    std::string GetRealPath(const std::string& path);
    bool IsDir(const std::string& path);
    bool SpaceRemainRat(const std::string &path, const std::string &snapshotPercent);
    std::shared_ptr<LvmSnapshot> CreateSnapshotByVolume(const std::string &volumeName,
        const std::string &volumePath, int& ret, const std::string &snapshotPercent);
    std::string ConvertSnapMountPath(const std::string& originalPath,
        const std::shared_ptr<LvmSnapshot>& snapshotInfo);
    std::string ConcatPath(const std::string& first, const std::string& second);
    bool DeleteSnapshotByVolume(const std::string &snapVolumeName);
    bool DeleteSnapshotByTag(const std::string &snapTag);
private:
    // key:文件对应的原始volume:deviceVolume(eg: volumeGroup/volume1), value: 快照信息
    std::map<std::string, std::shared_ptr<LvmSnapshot>> deviceVolumeSnapMap;
    std::mutex mtxLock {};
    std::shared_ptr<DeviceMount> deviceMount;
    std::shared_ptr<Win32VSSWrapper::VssClient> vssclient;
    std::string jobId;
    std::string m_snapshotMountRoot; // snapshot mount root point
};
}

#endif // APPPLUGIN_FILE_VSSSNAPSHOTPROVIDER_H
