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
#ifndef WIN_VOLUME_RESTORE_H
#define WIN_VOLUME_RESTORE_H

#include "VolumeRestore.h"
#include "ApplicationServiceDataType.h"

namespace FilePlugin {
enum class State {
    STATE_NONE = 0,
    /* 前置任务 */
    STATE_PRE_INIT,
    STATE_PRE_CHECK_BEFORE_RESTORE,  // 恢复前检查
    STATE_PRE_VOLUME_MAPPING,        // 卷的映射关系
    STATE_PRE_DISMOUNT_VOL,          // 解挂载卷
};

class WinVolumeRestore : public VolumeRestore {
public:
    WinVolumeRestore() = default;
    ~WinVolumeRestore() override = default;

    std::vector<WinVolumeInfo> GetVolumesFromCopy();
protected:
    int PrerequisiteJobInner() override;
    int PostJobInner() override;
    bool InitRepoInfo() override;
    bool ScanVolumesToGenerateTask() override;
    int ExecuteTearDownVolume() override;

    void PreInitStateHandles();

    int PrerequisiteJobInit();
    int CheckBeforeRestore();
    int VolumeMapping();
    int DismountVolumes();
    void HandleRestoreErrorCode() override;
    bool CheckBMR(const BareMetalRestoreParam& param) override;
    void PostProcess() override;
    bool SetBCDFile();
    bool SetPartitionActive();
    bool SetBCDFileForBios();
    bool GetDriveLetterForEfiAndSystemVolume();
    bool HasEFIPartitionInCopy();

private:
    std::vector<WinVolumeInfo> GetVolumesFromEnv();
    bool IsWinPE();
    bool AutoVolumeMapping(const std::vector<WinVolumeInfo> &copyVols, const std::vector<WinVolumeInfo> &envVols);
    bool ManualVolumeMapping(const std::vector<WinVolumeInfo> &copyVols, const std::vector<WinVolumeInfo> &envVols);
    bool SaveVolumeMapping();
    bool LoadVolumeMapping();
    int DismountVolume(const std::wstring &wDevicePath);
    bool RestartSystem();

private:
    uint32_t m_bootVolPartNumber;
    std::string m_volMappingPath;
    std::string m_volMetaPath;
    std::wstring m_bootDriveLetter;
    std::wstring m_sysReservedDriveLetter;
    std::wstring m_sysVolumeLetter;
    std::unordered_map<std::wstring, WinVolumeInfo> m_volumesMap;
};
}  // namespace FilePlugin

#endif