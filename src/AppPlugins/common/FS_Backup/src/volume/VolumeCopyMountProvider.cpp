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
#include <memory>

// external logger/json library
#include "common/JsonHelper.h"
#include "log/Log.h"
#include "common/VolumeUtils.h"
#include "native/FileSystemAPI.h"
#include "securec.h"

#ifdef __linux__
#include "native/linux/LinuxDeviceMapperMountProvider.h"
#include "native/linux/LinuxLoopbackMountProvider.h"
#endif

#ifdef _WIN32
#include "native/win32/Win32VirtualDiskMountProvider.h"
#endif

#include "VolumeCopyMountProvider.h"

using namespace volumeprotect;
using namespace volumeprotect::mount;
using namespace volumeprotect::common;
using namespace volumeprotect::fsapi;

// used to read common 'copyFormat' field from all record json
struct VolumeCopyMountRecordCommon {
    int             copyFormat;             // cast to enum CopyFormat

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyFormat, copyFormat);
    END_SERIAL_MEMEBER
};

// implement InnerErrorLoggerTrait...
std::vector<std::string> InnerErrorLoggerTrait::GetErrors() const
{
    return m_errors;
}

std::string InnerErrorLoggerTrait::GetError() const
{
    std::string errors;
    for (const std::string& errorMessage : m_errors) {
        errors += errorMessage + "\n";
    }
    return errors;
}

void InnerErrorLoggerTrait::RecordError(const char* message, ...)
{
    va_list args;
    va_start(args, message);
    
    // Create a buffer to store the formatted string
    std::string formattedString(MAX_ERR_MSG_SIZE, '\0');
    
    // Ensure there's room for the null terminator
    errno_t err = vsnprintf_s(&formattedString[0], formattedString.size(), formattedString.size(), message, args);
    if (err < 0) {
        ERRLOG("vsnprintf_s failed, errno %u", err);
        va_end(args);
        return;
    }

    // Check if the message was truncated
    if (err >= static_cast<int>(formattedString.size())) {
        WARNLOG("Warning: message truncated. Expected size: %d, Actual size: %d", err, formattedString.size());
    }

    va_end(args);
    m_errors.emplace_back(std::move(formattedString)); // 使用 std::move 以避免不必要的拷贝
}

// implement VolumeCopyMountProvider...
std::unique_ptr<VolumeCopyMountProvider> VolumeCopyMountProvider::Build(
    const VolumeCopyMountConfig& mountConfig)
{
    VolumeCopyMeta volumeCopyMeta {};
    if (!common::ReadVolumeCopyMeta(mountConfig.copyMetaDirPath, mountConfig.copyName, volumeCopyMeta)) {
        ERRLOG("failed to read volume copy meta from %s, copy name %s",
            mountConfig.copyMetaDirPath.c_str(), mountConfig.copyName.c_str());
        return nullptr;
    }
    if (!fsapi::IsDirectoryExists(mountConfig.outputDirPath)) {
        ERRLOG("invalid output directory path %s", mountConfig.outputDirPath.c_str());
        return nullptr;
    }
    CopyFormat copyFormat = static_cast<CopyFormat>(volumeCopyMeta.copyFormat);
    switch (volumeCopyMeta.copyFormat) {
        case static_cast<int>(CopyFormat::BIN) : {
#ifdef __linux__
            return mem::static_unique_pointer_cast<VolumeCopyMountProvider>(
                LinuxDeviceMapperMountProvider::Build(mountConfig, volumeCopyMeta));
#else
            return nullptr;
#endif
        }
        case static_cast<int>(CopyFormat::IMAGE) : {
#ifdef __linux__
            return mem::static_unique_pointer_cast<VolumeCopyMountProvider>(
                LinuxLoopbackMountProvider::Build(mountConfig, volumeCopyMeta));
#else
            return nullptr;
#endif
        }
#ifdef _WIN32
        case static_cast<int>(CopyFormat::VHD_DYNAMIC) :
        case static_cast<int>(CopyFormat::VHD_FIXED) :
        case static_cast<int>(CopyFormat::VHDX_DYNAMIC) :
        case static_cast<int>(CopyFormat::VHDX_FIXED) : {
            return Win32VirtualDiskMountProvider::Build(mountConfig, volumeCopyMeta);
        }
#endif
        default: ERRLOG("unknown copy format type %d", copyFormat);
    }
    return nullptr;
}

bool VolumeCopyMountProvider::Mount()
{
    RECORD_INNER_ERROR("base class does not support mount, need implementation from derived class");
    return false;
}

std::string VolumeCopyMountProvider::GetMountRecordPath() const
{
    // return default empty string, need implementation from derived class
    return "";
}

// implement VolumeCopyMountProvider...
std::unique_ptr<VolumeCopyUmountProvider> VolumeCopyUmountProvider::Build(
    const std::string mountRecordJsonFilePath, const std::string& shareName)
{
    if (!fsapi::IsFileExists(mountRecordJsonFilePath)) {
        ERRLOG("umount json record file %s not exists", mountRecordJsonFilePath.c_str());
        return nullptr;
    }
    std::string outputDirPath = common::GetParentDirectoryPath(mountRecordJsonFilePath);

    VolumeCopyMountRecordCommon mountRecord {};
    if (!common::JsonDeserialize(mountRecord, mountRecordJsonFilePath)) {
        ERRLOG("unabled to open copy mount record %s to read, errno %u", mountRecordJsonFilePath.c_str(), errno);
        return nullptr;
    };
    switch (mountRecord.copyFormat) {
        case static_cast<int>(CopyFormat::BIN) : {
#ifdef __linux__
            return mem::static_unique_pointer_cast<VolumeCopyUmountProvider>(
                LinuxDeviceMapperUmountProvider::Build(mountRecordJsonFilePath, outputDirPath));
#else
            return nullptr;
#endif
        }
        case static_cast<int>(CopyFormat::IMAGE) : {
#ifdef __linux__
            return mem::static_unique_pointer_cast<VolumeCopyUmountProvider>(
                LinuxLoopbackUmountProvider::Build(mountRecordJsonFilePath, outputDirPath));
#else
            return nullptr;
#endif
        }
#ifdef _WIN32
        case static_cast<int>(CopyFormat::VHD_DYNAMIC) :
        case static_cast<int>(CopyFormat::VHD_FIXED) :
        case static_cast<int>(CopyFormat::VHDX_DYNAMIC) :
        case static_cast<int>(CopyFormat::VHDX_FIXED) : {
            return Win32VirtualDiskUmountProvider::Build(mountRecordJsonFilePath, shareName);
        }
#endif
        default: ERRLOG("unknown copy format type %d", mountRecord.copyFormat);
    }
    return nullptr;
}


bool VolumeCopyUmountProvider::Umount()
{
    RECORD_INNER_ERROR("base class does not support umount, need implementation from derived class");
    return false;
}
