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
#ifndef VOLUMEBACKUP_LINUX_IMAGE_MOUNT_PROVIDER_HEADER
#define VOLUMEBACKUP_LINUX_IMAGE_MOUNT_PROVIDER_HEADER

#ifdef __linux__

#include "VolumeUtils.h"
#include "VolumeCopyMountProvider.h"

namespace volumeprotect {
namespace mount {

struct LinuxLoopbackMountProviderParams {
    std::string     outputDirPath;
    std::string     copyName;
    std::string     imageFilePath;
    std::string     mountTargetPath;
    bool            readOnly;
    std::string     mountFsType;
    std::string     mountOptions;
};

/**
 * LinuxLoopbackMountProvider provides the functionality to mount volume copy with CopyFormat::IMAGE.
 * For *nix platform, this piece of code will create a loopback device from the volume image file and mount it
 * For Windows platform, mounting image whose filesystem is not UDF/ISO-9660 requiring 3rd utilities like ImDisk.
 */
class LinuxLoopbackMountProvider : public VolumeCopyMountProvider {
public:
    static std::unique_ptr<LinuxLoopbackMountProvider> Build(
        const VolumeCopyMountConfig& volumeCopyMountConfig,
        const VolumeCopyMeta& volumeCopyMeta);

    explicit LinuxLoopbackMountProvider(const LinuxLoopbackMountProviderParams& params);

    bool Mount() override;

    std::string GetMountRecordPath() const override;

    ~LinuxLoopbackMountProvider() = default;

    bool CheckAndTuneUuid(const std::string& loopDevicePath);

private:
    bool PosixLoopbackMountRollback(const std::string& loopbackDevicePath);

private:
    std::string     m_outputDirPath;
    std::string     m_copyName;
    std::string     m_imageFilePath;
    std::string     m_mountTargetPath;
    bool            m_readOnly;

    // [optional] used for *nix system
    std::string     m_mountFsType;
    std::string     m_mountOptions;
};

/**
 * LinuxLoopbackMountProvider provides the functionality to umount volume copy with CopyFormat::IMAGE.
 * For *nix platform, this piece of code will umount the mount point and detach the loopback device.
 * For Windows platform, umouting image whose filesystem is not UDF/ISO-9660 requiring 3rd utilities like ImDisk.
 */
class LinuxLoopbackUmountProvider : public VolumeCopyUmountProvider {
public:
    static std::unique_ptr<LinuxLoopbackUmountProvider> Build(
        const std::string& mountRecordJsonFilePath,
        const std::string& outputDirPath);

    LinuxLoopbackUmountProvider(
    	const std::string& outputDirPath,
        const std::string& mountTargetPath,
        const std::string& loopbackDevicePath);

    ~LinuxLoopbackUmountProvider() = default;

    bool Umount() override;

private:
    std::string     m_outputDirPath;
    std::string     m_mountTargetPath;
    std::string     m_loopbackDevicePath;
};

}
}

#endif

#endif