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
#ifndef VOLUMEBACKUP_NATIVE_FILESYSTEM_API_HEADER
#define VOLUMEBACKUP_NATIVE_FILESYSTEM_API_HEADER

#include "common/VolumeProtectMacros.h"
#include "VolumeProtector.h"

namespace volumeprotect {
/**
 * @brief native filesystem api wrapper
 */
namespace fsapi {

struct VOLUMEPROTECT_API SystemApiException : public std::exception {
    explicit SystemApiException(ErrCodeType errorCode);
    SystemApiException(const char* message, ErrCodeType errorCode);
    const char* what() const noexcept override;

    std::string m_message;
};

VOLUMEPROTECT_API bool         IsFileExists(const std::string& path);

VOLUMEPROTECT_API uint64_t     GetFileSize(const std::string& path);

VOLUMEPROTECT_API bool         IsDirectoryExists(const std::string& path);

VOLUMEPROTECT_API uint8_t*     ReadBinaryBuffer(const std::string& filepath, uint64_t length);

VOLUMEPROTECT_API bool         WriteBinaryBuffer(const std::string& filepath, const uint8_t* buffer, uint64_t length);

VOLUMEPROTECT_API bool         IsVolumeExists(const std::string& volumePath);

VOLUMEPROTECT_API uint64_t     ReadVolumeSize(const std::string& volumePath,
    const CopyFormat& copyFormat = CopyFormat::BIN);

VOLUMEPROTECT_API uint64_t     ReadFileSize(const std::string& volumePath);

VOLUMEPROTECT_API uint32_t     ProcessorsNum();

VOLUMEPROTECT_API bool         CreateEmptyFile(const std::string& dirPath, const std::string& filename);

VOLUMEPROTECT_API bool         RemoveFile(const std::string& dirPath, const std::string& filename);

#ifdef __linux__
uint64_t    ReadSectorSizeLinux(const std::string& devicePath);

bool        IsMountPoint(const std::string& dirPath);
// get the block device path of the mount point
std::string GetMountDevicePath(const std::string& mountTargetPath);
#endif

}
}

#endif