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

#include "native/win32/Win32VirtualDiskMountProvider.h"
#include "common/VolumeUtils.h"
#include "native/FileSystemAPI.h"
#include "native/win32/Win32RawIO.h"
#include "log/Log.h"

using namespace volumeprotect;
using namespace volumeprotect::mount;
using namespace volumeprotect::rawio;
using namespace volumeprotect::common;
using namespace volumeprotect::fsapi;

namespace {
    const std::string SEPARATOR = "\\";
    const std::string VIRTUAL_DISK_COPY_MOUNT_RECORD_FILE_SUFFIX = ".vhd.mount.record.json";
}

// serialize to $copyName.image.mount.record.json
struct Win32VirtualDiskCopyMountRecord {
    int             copyFormat;             // cast to enum CopyFormat
    std::string     virtualDiskFilePath;
    std::string     mountTargetPath;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyFormat, copyFormat);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(virtualDiskFilePath, virtualDiskFilePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountTargetPath, mountTargetPath);
    END_SERIAL_MEMEBER
};

// implement Win32VirtualDiskMountProvider...
std::unique_ptr<Win32VirtualDiskMountProvider> Win32VirtualDiskMountProvider::Build(
    const VolumeCopyMountConfig& volumeCopyMountConfig,
    const VolumeCopyMeta& volumeCopyMeta)
{
    CopyFormat copyFormat = static_cast<CopyFormat>(volumeCopyMeta.copyFormat);
    if (copyFormat != CopyFormat::VHD_DYNAMIC && copyFormat != CopyFormat::VHD_FIXED
        && copyFormat != CopyFormat::VHDX_DYNAMIC && copyFormat != CopyFormat::VHDX_FIXED) {
        ERRLOG("unsupport copy format %d for win32 virtual disk mount provider!", volumeCopyMeta.copyFormat);
        return nullptr;
    }
    if (!fsapi::IsDirectoryExists(volumeCopyMountConfig.copyDataDirPath)) {
        ERRLOG("invalid copy data directory path %s", volumeCopyMountConfig.copyDataDirPath.c_str());
        return nullptr;
    }
    if (volumeCopyMeta.segments.empty()) {
        ERRLOG("illegal volume copy meta, image file segments list empty");
        return nullptr;
    }
    std::string virtualDiskFilePath = volumeCopyMountConfig.copyDataDirPath
        + SEPARATOR + volumeCopyMeta.segments.front().copyDataFile;
    return exstd::make_unique<Win32VirtualDiskMountProvider>(
        volumeCopyMountConfig.outputDirPath,
        volumeCopyMountConfig.copyName,
        copyFormat,
        virtualDiskFilePath,
        volumeCopyMountConfig.mountTargetPath,
        volumeCopyMountConfig.shareName);
}

Win32VirtualDiskMountProvider::Win32VirtualDiskMountProvider(
    const std::string& outputDirPath,
    const std::string& copyName,
    CopyFormat copyFormat,
    const std::string& virtualDiskFilePath,
    const std::string& mountTargetPath,
    const std::string& shareName)
    : m_outputDirPath(outputDirPath),
    m_copyName(copyName),
    m_copyFormat(copyFormat),
    m_virtualDiskFilePath(virtualDiskFilePath),
    m_mountTargetPath(mountTargetPath),
    m_shareName(shareName)
{}

bool Win32VirtualDiskMountProvider::Mount()
{
    std::string physicalDrivePath;
    std::string volumeDevicePath;
    std::string volumeGuidName;
    ErrCodeType errorCode = ERROR_SUCCESS;
    // serialize mountRecord ahead
    std::string filepath = GetMountRecordPath();
    Win32VirtualDiskCopyMountRecord mountRecord {};
    mountRecord.copyFormat = static_cast<int>(m_copyFormat);
    mountRecord.mountTargetPath = m_mountTargetPath;
    mountRecord.virtualDiskFilePath = m_virtualDiskFilePath;
    if (!common::JsonSerialize(mountRecord, filepath)) {
        RECORD_INNER_ERROR("failed to save image copy mount record to %s, errno %u", filepath.c_str(), errno);
        return false;
    }
    if (!rawio::win32::VirtualDiskAttached(m_virtualDiskFilePath, m_shareName)
        && !rawio::win32::AttachVirtualDiskCopy(m_virtualDiskFilePath, errorCode)) {
        RECORD_INNER_ERROR("failed to attach virtualdisk file %s, error %u", m_virtualDiskFilePath.c_str(), errorCode);
        return false;
    }
    if (!rawio::win32::GetVirtualDiskPhysicalDrivePath(m_virtualDiskFilePath, physicalDrivePath, errorCode)) {
        RECORD_INNER_ERROR("failed to get virtual disk physical drive path from %s, error %u",
            m_virtualDiskFilePath.c_str(), errorCode);
        MountRollback();
        return false;
    }
    if (!rawio::win32::GetCopyVolumeDevicePath(physicalDrivePath, volumeDevicePath, errorCode)) {
        RECORD_INNER_ERROR("failed to get volume device path from %s, error %u",
            physicalDrivePath.c_str(), errorCode);
        MountRollback();
        return false;
    }
    if (!rawio::win32::GetVolumeGuidNameByVolumeDevicePath(volumeDevicePath, volumeGuidName, errorCode)) {
        RECORD_INNER_ERROR("failed to get volume guid name, device path : %s, error %u",
            volumeDevicePath.c_str(), errorCode);
        MountRollback();
        return false;
    }
    if (!rawio::win32::AddVolumeMountPoint(volumeGuidName, m_mountTargetPath, errorCode)) {
        RECORD_INNER_ERROR("failed to assign mount point %s for volume %s, path %d, error %u",
            m_mountTargetPath.c_str(), volumeGuidName.c_str(), volumeDevicePath.c_str(), ::GetLastError());
        MountRollback();
        return false;
    }
    return true;
}

std::string Win32VirtualDiskMountProvider::GetMountRecordPath() const
{
    return m_outputDirPath + SEPARATOR + m_copyName + VIRTUAL_DISK_COPY_MOUNT_RECORD_FILE_SUFFIX;
}

// virtual disk that has been attached need to be detached
void Win32VirtualDiskMountProvider::MountRollback()
{
    ErrCodeType errorCode = ERROR_SUCCESS;
    if (rawio::win32::VirtualDiskAttached(m_virtualDiskFilePath, m_shareName)
        && !rawio::win32::DetachVirtualDiskCopy(m_virtualDiskFilePath, errorCode)) {
        RECORD_INNER_ERROR("failed to detach virtual disk %s, error %u", m_virtualDiskFilePath.c_str(), errorCode);
    }
    return;
}

// implement Win32VirtualDiskUmountProvider...
std::unique_ptr<Win32VirtualDiskUmountProvider> Win32VirtualDiskUmountProvider::Build(
    const std::string& mountRecordJsonFilePath, const std::string& shareName)
{
    Win32VirtualDiskCopyMountRecord mountRecord {};
    if (!common::JsonDeserialize(mountRecord, mountRecordJsonFilePath)) {
        ERRLOG("unabled to open copy mount record %s to read, errno %u", mountRecordJsonFilePath.c_str(), errno);
        return nullptr;
    };
    return exstd::make_unique<Win32VirtualDiskUmountProvider>(mountRecord.virtualDiskFilePath, shareName);
}

Win32VirtualDiskUmountProvider::Win32VirtualDiskUmountProvider(const std::string& virtualDiskFilePath, const std::string& shareName)
    : m_virtualDiskFilePath(virtualDiskFilePath), m_shareName(shareName)
{}

bool Win32VirtualDiskUmountProvider::Umount()
{
    // HINT:: if volume is in use, need to wait for virtual disk to be detached?
    ErrCodeType errorCode = ERROR_SUCCESS;
    if (rawio::win32::VirtualDiskAttached(m_virtualDiskFilePath, m_shareName)
        && !rawio::win32::DetachVirtualDiskCopy(m_virtualDiskFilePath, errorCode)) {
        RECORD_INNER_ERROR("failed to detach virtual disk %s, error %u", m_virtualDiskFilePath.c_str(), errorCode);
        return false;
    }
    return true;
}

#endif