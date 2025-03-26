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
#ifndef LINUX_VOLUME_BACKUP_H
#define LINUX_VOLUME_BACKUP_H

#include "VolumeBackup.h"

namespace FilePlugin {

class LinuxVolumeBackup : public VolumeBackup {
public:
    LinuxVolumeBackup() = default;
    ~LinuxVolumeBackup() override = default;

protected:
    bool GenerateSubJobHook() override;
    bool TearDownVolumeHook() override;
    volumeprotect::CopyFormat CopyFormat() override;
    void GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes) override;
    bool SaveVolumeMountEntriesJson() override;
    bool InitProtectVolume() override;
    void ClearResidualSnapshotsAndAlarm() override;
    bool CreateSnapshot(const std::string& volumeName, std::string& snapDevVol) override;
    void DeleteSnapshot() override;
    bool VolumeExists(const std::string& volumeName);
    bool PreProcess(std::string& snapShotDev) override;
    bool ScanVolumesToGenerateTask() override;
    bool IsLimitedKernel() override;
    void FillVolumeInfo(const VolumeBackupSubJob& backupSubJob) override;
    void FillBackupConfig(volumeprotect::task::VolumeBackupConfig &backupParams) override;
    uint32_t GetSysVolumeSize() override;
    std::string GetMountedVolumeFsType(const std::string& volumePath) const;

private:
    int InitSystemVolume();
    int GetSysVolumeList();
    int GetSystemInfo();
    int CheckSystemVolume();
    std::string GetMountTypeVersion(const std::string& mountType, const std::string& volumePath) const;
    bool CopyBoot();
    std::string LoadSnapshotParentPath() const;
    void CheckExistVolume(const std::vector<std::string> &volumeNames, std::vector<std::string> &existVolumes,
        std::string &notExistVolumes);

    VolumeInfo m_volumeInfomation;
    std::vector<VolumeInfo> m_sysVolumeList; /* 原始的系统卷列表，非用户输入 */
};
}

#endif