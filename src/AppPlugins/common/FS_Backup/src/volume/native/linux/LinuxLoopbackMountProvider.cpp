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
#ifdef __linux__

#include "log/Log.h"

// linux/fs.h have BLOCK_SIZE defined, conflicted with define/Defines.h
#ifdef BLOCK_SIZE
#undef BLOCK_SIZE
#endif

#include <cerrno>
#include <fcntl.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <mntent.h>
#include <dirent.h>
#include <cstdlib>
#include <unistd.h>

#include "native/FileSystemAPI.h"
#include "native/linux/LoopDeviceControl.h"
#include "VolumeCopyMountProvider.h"
#include "system/System.hpp"
#include "common/Path.h"
#include "native/linux/LinuxLoopbackMountProvider.h"

using namespace volumeprotect;
using namespace volumeprotect::common;
using namespace volumeprotect::mount;
using namespace volumeprotect::fsapi;

namespace {
    const std::string SEPARATOR = "/";
    const std::string IMAGE_COPY_MOUNT_RECORD_FILE_SUFFIX = ".image.mount.record.json";
    const std::string LOOPBACK_DEVICE_PATH_PREFIX = "/dev/loop";
    const std::string LOOPBACK_DEVICE_CREATION_RECORD_SUFFIX = ".loop.record";
    const int MP_SUCCESS = 0;
    const std::string MODULE = "LinuxLoopbackMountProvider";
    const int MAX_UMOUNT_TRY = 3;
    const int UMOUNT_SLEEP = 300;
}

// serialize to $copyName.image.mount.record.json
struct LinuxImageCopyMountRecord {
    int             copyFormat;
    std::string     loopbackDevicePath;
    std::string     mountTargetPath;
    std::string     mountFsType;
    std::string     mountOptions;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyFormat, copyFormat);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(loopbackDevicePath, loopbackDevicePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountTargetPath, mountTargetPath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountFsType, mountFsType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mountOptions, mountOptions);
    END_SERIAL_MEMEBER
};

std::unique_ptr<LinuxLoopbackMountProvider> LinuxLoopbackMountProvider::Build(
    const VolumeCopyMountConfig& volumeCopyMountConfig,
    const VolumeCopyMeta& volumeCopyMeta)
{
    LinuxLoopbackMountProviderParams params {};
    params.outputDirPath = volumeCopyMountConfig.outputDirPath;
    params.copyName = volumeCopyMeta.copyName;
    if (volumeCopyMeta.segments.empty()) {
        ERRLOG("illegal volume copy meta, image file segments list empty");
        return nullptr;
    }
    params.imageFilePath = volumeCopyMountConfig.copyDataDirPath
        + SEPARATOR + volumeCopyMeta.segments.front().copyDataFile;
    params.mountTargetPath = volumeCopyMountConfig.mountTargetPath;
    params.mountFsType = volumeCopyMountConfig.mountFsType;
    params.mountOptions = volumeCopyMountConfig.mountOptions;
    params.readOnly = volumeCopyMountConfig.readOnly;
    return exstd::make_unique<LinuxLoopbackMountProvider>(params);
}

LinuxLoopbackMountProvider::LinuxLoopbackMountProvider(const LinuxLoopbackMountProviderParams& params)
    : m_outputDirPath(params.outputDirPath),
    m_copyName(params.copyName),
    m_imageFilePath(params.imageFilePath),
    m_mountTargetPath(params.mountTargetPath),
    m_readOnly(params.readOnly),
    m_mountFsType(params.mountFsType),
    m_mountOptions(params.mountOptions)
{}

std::string LinuxLoopbackMountProvider::GetMountRecordPath() const
{
    return m_outputDirPath + SEPARATOR + m_copyName + IMAGE_COPY_MOUNT_RECORD_FILE_SUFFIX;
}

static bool RunMountShellCmd(
    const std::string& type,
    const std::string& options,
    const std::string& device,
    const std::string& target)
{
    std::string cmd = "mount -t ? -o ? ? ?";
    std::vector<std::string> paramList { type, options, device, target };
    std::vector<std::string> output {};
    std::vector<std::string> erroutput {};
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, erroutput) != MP_SUCCESS) {
        std::string msg;
        std::string errmsg;
        for (auto &it : output) {
            msg += it + " ";
        }
        ERRLOG("Run shell msg: %s", Module::WipeSensitiveDataForLog(msg).c_str());
        for (auto &it : erroutput) {
            errmsg += it + " ";
        }
        ERRLOG("Run shell error msg: %s", Module::WipeSensitiveDataForLog(errmsg).c_str());
        return false;
    }
    return true;
}

static bool RunUmountShellCmd(const std::string& device)
{
    std::string cmd = "umount -lf ?";
    std::vector<std::string> paramList { device };
    std::vector<std::string> output {};
    std::vector<std::string> erroutput {};
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, erroutput) != MP_SUCCESS) {
        std::string msg;
        std::string errmsg;
        for (auto &it : output) {
            msg += it + " ";
        }
        ERRLOG("Run shell msg: %s", Module::WipeSensitiveDataForLog(msg).c_str());
        for (auto &it : erroutput) {
            errmsg += it + " ";
        }
        ERRLOG("Run shell error msg: %s", Module::WipeSensitiveDataForLog(errmsg).c_str());
        return false;
    }
    return true;
}

bool LinuxLoopbackMountProvider::CheckAndTuneUuid(const std::string& loopDevicePath)
{
    if (m_mountFsType != "xfs") {
        return true;
    }
    INFOLOG("enter CheckAndTuneUuid for loop device %s", loopDevicePath.c_str());
    std::string cmd = "?/bin/lvmSnapshot.sh -csnapid '?'";
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(loopDevicePath);
    std::vector<std::string> output {};
    std::vector<std::string> erroutput {};
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, erroutput) != MP_SUCCESS) {
        std::string msg;
        std::string errmsg;
        for (auto &it : output) {
            msg += it + " ";
        }
        ERRLOG("Run shell msg: %s", Module::WipeSensitiveDataForLog(msg).c_str());
        for (auto &it : erroutput) {
            errmsg += it + " ";
        }
        ERRLOG("Run shell error msg: %s", Module::WipeSensitiveDataForLog(errmsg).c_str());
        return false;
    }
    INFOLOG("exit CheckAndTuneUuid for loop device %s", loopDevicePath.c_str());
    return true;
}

// mount using *nix loopback device
bool LinuxLoopbackMountProvider::Mount()
{
    // 1. attach loopback device
    std::string loopDevicePath;
    if (!loopback::Attach(m_imageFilePath, loopDevicePath, m_readOnly ? O_RDONLY : O_RDWR)) {
        RECORD_INNER_ERROR("failed to attach read only loopback device from %s, readonly %u, errno %u",
            m_imageFilePath.c_str(), m_readOnly, errno);
        return false;
    }
    // keep checkpoint for loopback device creation
    std::string loopDeviceNumber = loopDevicePath.substr(LOOPBACK_DEVICE_PATH_PREFIX.length());
    std::string loopbackDeviceCheckpointName = loopDeviceNumber + LOOPBACK_DEVICE_CREATION_RECORD_SUFFIX;
    if (!fsapi::CreateEmptyFile(m_outputDirPath, loopbackDeviceCheckpointName)) {
        RECORD_INNER_ERROR("failed to create checkpoint file %s", loopbackDeviceCheckpointName.c_str());
    }

    // 2. mount block device
    if (!CheckAndTuneUuid(loopDevicePath)) {
        RECORD_INNER_ERROR("tunefs %s failed, type %s, errno %u", loopDevicePath.c_str(), m_mountFsType.c_str(), errno);
        PosixLoopbackMountRollback(loopDevicePath);
        return false;
    }
    if (!::RunMountShellCmd(m_mountFsType, m_mountOptions, loopDevicePath, m_mountTargetPath)) {
        RECORD_INNER_ERROR("mount %s to %s failed, type %s, option %s, errno %u",
            loopDevicePath.c_str(), m_mountTargetPath.c_str(), m_mountFsType.c_str(), m_mountOptions.c_str(), errno);
        PosixLoopbackMountRollback(loopDevicePath);
        return false;
    }

    // 3. save mount record to output directory
    LinuxImageCopyMountRecord mountRecord {};
    mountRecord.copyFormat = static_cast<int>(CopyFormat::IMAGE);
    mountRecord.loopbackDevicePath = loopDevicePath;
    mountRecord.mountTargetPath = m_mountTargetPath;
    mountRecord.mountFsType = m_mountFsType;
    mountRecord.mountOptions = m_mountOptions;
    std::string filepath = GetMountRecordPath();
    if (!common::JsonSerialize(mountRecord, filepath)) {
        RECORD_INNER_ERROR("failed to save image copy mount record to %s, errno %u", filepath.c_str(), errno);
        PosixLoopbackMountRollback(loopDevicePath);
        return false;
    }
    return true;
}

bool LinuxLoopbackMountProvider::PosixLoopbackMountRollback(const std::string& loopbackDevicePath)
{
    if (loopbackDevicePath.empty()) {
        // no loopback device attached, no mounts, return directly
        return true;
    }
    if (fsapi::IsMountPoint(m_mountTargetPath) && fsapi::GetMountDevicePath(m_mountTargetPath) != loopbackDevicePath) {
        // moint point used by other application, return directly
        return true;
    }
    LinuxLoopbackUmountProvider umountProvider(m_outputDirPath, m_mountTargetPath, loopbackDevicePath);
    if (!umountProvider.Umount()) {
        RECORD_INNER_ERROR("failed to clear loopback mount residue");
        return false;
    }
    return true;
}

std::unique_ptr<LinuxLoopbackUmountProvider> LinuxLoopbackUmountProvider::Build(
    const std::string& mountRecordJsonFilePath,
    const std::string& outputDirPath)
{
    LinuxImageCopyMountRecord mountRecord {};
    if (!common::JsonDeserialize(mountRecord, mountRecordJsonFilePath)) {
        ERRLOG("unabled to open copy mount record %s to read, errno %u", mountRecordJsonFilePath.c_str(), errno);
        return nullptr;
    };
    return exstd::make_unique<LinuxLoopbackUmountProvider>(
        outputDirPath,
        mountRecord.mountTargetPath,
        mountRecord.loopbackDevicePath);
}

LinuxLoopbackUmountProvider::LinuxLoopbackUmountProvider(
    const std::string& outputDirPath, const std::string& mountTargetPath, const std::string& loopbackDevicePath)
    : m_outputDirPath(outputDirPath), m_mountTargetPath(mountTargetPath), m_loopbackDevicePath(loopbackDevicePath)
{}

bool LinuxLoopbackUmountProvider::Umount()
{
    // 1. umount filesystem
    if (!m_mountTargetPath.empty()
        && fsapi::IsMountPoint(m_mountTargetPath)
        && ::umount2(m_mountTargetPath.c_str(), MNT_FORCE | MNT_DETACH) != 0) {
        RECORD_INNER_ERROR("failed to umount target %s, errno %u", m_mountTargetPath.c_str(), errno);
        return false;
    }
    for (int i = 0; i < MAX_UMOUNT_TRY; i++) {
        INFOLOG("run post umount hook (%d) for device %s", i, m_loopbackDevicePath.c_str());
        ::RunUmountShellCmd(m_loopbackDevicePath.c_str());
        std::this_thread::sleep_for(std::chrono::milliseconds(UMOUNT_SLEEP));
    }
    // 2. detach loopback device
    if (!m_loopbackDevicePath.empty()
        && loopback::Attached(m_loopbackDevicePath)
        && !loopback::Detach(m_loopbackDevicePath)) {
        RECORD_INNER_ERROR("failed to detach loopback device %s, errno %u", m_loopbackDevicePath.c_str(), errno);
        return false;
    }
    if (m_loopbackDevicePath.find(LOOPBACK_DEVICE_PATH_PREFIX) == 0) {
        std::string loopDeviceNumber = m_loopbackDevicePath.substr(LOOPBACK_DEVICE_PATH_PREFIX.length());
        if (!fsapi::RemoveFile(m_outputDirPath, loopDeviceNumber + LOOPBACK_DEVICE_CREATION_RECORD_SUFFIX)) {
            WARNLOG("failed to remove loopback record checkpoint");
        }
    }
    return true;
}

#endif