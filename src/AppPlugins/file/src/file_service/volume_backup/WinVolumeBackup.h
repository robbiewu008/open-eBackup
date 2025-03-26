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
#ifndef WIN_VOLUME_BACKUP_H
#define WIN_VOLUME_BACKUP_H

#include "VolumeBackup.h"
#include "FileSystemUtil.h"
#include "ApplicationServiceDataType.h"

namespace FilePlugin {

enum class SystemBackupType {
    SYSTEM_BACKUP_TYEP_SYSTEM_STATE = 0,
    SYSTEM_BACKUP_TYPE_VOLUME = 1,
    SYSTEM_BACKUP_TYPE_BMR = 2,
    SYSTEM_BACKUP_TYPE_UNKNOWN
};

struct WinVolumeBackupProtectObjectVolumeName {
    std::string name;
    std::string displayName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(displayName, displayName)
    END_SERIAL_MEMEBER
};
struct WinVolumeBackupProtectObjectVolumeNameWrapper {
    std::vector<WinVolumeBackupProtectObjectVolumeName> paths;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
    END_SERIAL_MEMEBER
};

struct WinVolumeBackupProtectObjectExtend {
    std::string systemBackupType;
    std::string paths;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(systemBackupType, system_backup_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(paths, paths)
    END_SERIAL_MEMEBER
};

struct WinVolumeBackupCopy {
    SystemBackupType type { SystemBackupType::SYSTEM_BACKUP_TYPE_VOLUME };
    std::vector<Module::FileSystemUtil::Win32VolumesDetail> protectedVolumes;
};

class WinVolumeBackup : public VolumeBackup {
public:
    WinVolumeBackup() = default;
    ~WinVolumeBackup() override = default;
    void ClearResidualSnapshotsAndAlarm() override;
    bool InitProtectVolume() override;
    void CheckExistVolume(const std::vector<Module::FileSystemUtil::Win32VolumesDetail>& volumes,
        std::vector<std::string>& existVolumes, std::string& noExistVolumes);
    void GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes);
    volumeprotect::CopyFormat CopyFormat() override;
    bool ScanVolumesToGenerateTask() override;
    bool IsLimitedKernel() override;
    void FillVolumeInfo(const VolumeBackupSubJob& backupSubJob) override;
    void FillBackupConfig(volumeprotect::task::VolumeBackupConfig &backupParams) override;
    uint32_t GetSysVolumeSize() override;
    void DeleteSnapshot() override;
    bool RecordVolume() override;
    bool MergeVolumeInfo() override;
    bool FillCopyInfo(std::string& extendInfo) override;

protected:
    void FillVolumeBackupConfig(volumeprotect::task::VolumeBackupConfig& config);
    int StartVolumeBackup(const volumeprotect::task::VolumeBackupConfig& config);
    SystemBackupType GetSystemBackupType(const std::string& type);
    std::string GetValidVolumeName(const std::string& volumeName);
    bool PreProcess(std::string& snapShotDev) override;
    bool CheckIsEFI(const std::string& volmeName);

    std::string m_volumeName;
    std::string m_volumePath;
    std::string m_snapShotDev;
    std::vector<StringVolumeInfo> m_copiedVolumes;
};
}

#endif