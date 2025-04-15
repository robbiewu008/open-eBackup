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
#ifdef _WIN32

#ifndef VOLUMEBACKUP_WIN32_VIRTUALDISK_MOUNT_PROVIDER_HEADER
#define VOLUMEBACKUP_WIN32_VIRTUALDISK_MOUNT_PROVIDER_HEADER

#include "common/VolumeProtectMacros.h"
// external logger/json library
#include "common/JsonHelper.h"
#include "common/VolumeUtils.h"

#include "VolumeCopyMountProvider.h"

namespace volumeprotect {
namespace mount {

/**
 * Win32VirtualDiskMountProvider provides the functionality to mount/umount volume copy from a specified
 *   data path and meta path. This piece of code will attach the virtual disk (*.vhd/*.vhdx) file and assign
 *   a driver letter to it, it can alse be assigned a non-root path if it's a NTFS volume.
 * Each virtual disk is guaranteed to have only one MSR partition and one data partition, it can only be mounted
 *   by the Windows OS version that support GPT partition and virtual disk service.
 */
class Win32VirtualDiskMountProvider : public VolumeCopyMountProvider {
public:
    static std::unique_ptr<Win32VirtualDiskMountProvider> Build(
        const VolumeCopyMountConfig& volumeCopyMountConfig,
        const VolumeCopyMeta& volumeCopyMeta);

    Win32VirtualDiskMountProvider(
        const std::string& outputDirPath,
        const std::string& copyName,
        CopyFormat copyFormat,
        const std::string& virtualDiskFilePath,
        const std::string& mountTargetPath,
        const std::string& shareName = "");

    bool Mount() override;

    std::string GetMountRecordPath() const override;

private:
    void MountRollback();

private:
    std::string     m_outputDirPath;
    std::string     m_copyName;
    CopyFormat      m_copyFormat;
    std::string     m_virtualDiskFilePath;
    std::string     m_mountTargetPath;
    std::string     m_shareName;
};

class Win32VirtualDiskUmountProvider : public VolumeCopyUmountProvider {
public:
    static std::unique_ptr<Win32VirtualDiskUmountProvider> Build(const std::string& mountRecordJsonFilePath,
        const std::string& shareName = "");

    Win32VirtualDiskUmountProvider(const std::string& virtualDiskFilePath, const std::string& shareName = "");

    bool Umount() override;

private:
    std::string m_virtualDiskFilePath;
    std::string m_shareName;
};

}
}

#endif

#endif