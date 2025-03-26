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
#ifndef LINUX_VOLUME_RESTORE_H
#define LINUX_VOLUME_RESTORE_H

#include "VolumeRestore.h"

namespace FilePlugin {

class LinuxVolumeRestore : public VolumeRestore {
public:
    LinuxVolumeRestore() = default;
    ~LinuxVolumeRestore() override = default;

protected:
    int PrerequisiteJobInner() override;
    int PostJobInner() override;
    bool BareMetalRestore() override;
    bool IsSystemVolume(const std::string &volumeName) const override;
    int ExecuteTearDownVolume() override;
    void HandleRestoreErrorCode() override;
    bool CheckBMR(const BareMetalRestoreParam& param) override;

private:
    bool CheckBMRCompatible();
    bool GetUmountVolumeInfo();
    bool UmountVolume(const std::string &mountPoint);
    void ReadMountInfoFromFile(const std::string &infoFilePath);
    bool WriteMountInfoToFile(const std::string &infoFilePath);
    bool IsSystemVolumeMountPath(const std::string &mountPath) const;
    bool BareMetalRestoreRepairGrub();
    bool RebootSystem();
    bool MountInitalVolume();
    bool MountVolume();
    bool ModifyVolumeInfo(const std::string &deviceName, const ::std::string &attribute);
    bool CompareFilesystemVersion(const std::string &infoFilePath, const std::string &dataDstPath, std::string &oriVolumeFilesystemType);
    bool GetOriVolumeFilesystemInfo(const std::string &infoFilePath, std::string &oriVolumeFilesystemVersion, std::string &oriVolumeFilesystemType);
    bool GetTarVolumeFilesystemInfo(const std::string &dataDstPath, std::string &targetVolumeFilesystemVersion, std::string &oriVolumeFilesystemType);
    bool CompareVersion(std::string& version1, std::string& version2);
    bool InitDiskMapInfo(std::string &diskMapInfoSet);

private:
    std::vector<MountInfo> m_mountInfoSet;
};
}  // namespace FilePlugin

#endif