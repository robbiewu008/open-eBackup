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
#include "VolumeUtils.h"
#include <string>

namespace {
#ifdef _WIN32
    constexpr auto SEPARATOR = "\\";
#else
    constexpr auto SEPARATOR = "/";
#endif
    constexpr auto VOLUME_COPY_META_JSON_FILENAME_EXTENSION = ".volumecopy.meta.json";
    constexpr auto SHA256_CHECKSUM_BINARY_FILENAME_EXTENSION = ".sha256.meta.bin";
    constexpr auto COPY_DATA_BIN_FILENAME_EXTENSION = ".copydata.bin";
    constexpr auto COPY_DATA_BIN_PARTED_FILENAME_EXTENSION = ".copydata.bin.part";
    constexpr auto COPY_DATA_IMAGE_FILENAME_EXTENSION = ".copydata.img";
    constexpr auto COPY_DATA_VHD_FILENAME_EXTENSION = ".copydata.vhd";
    constexpr auto COPY_DATA_VHDX_FILENAME_EXTENSION = ".copydata.vhdx";
    constexpr auto WRITER_BITMAP_FILENAME_EXTENSION = ".checkpoint.bin";
}

using namespace volumeprotect;

std::string common::GetChecksumBinPath(
    const std::string&  copyMetaDirPath,
    const std::string&  copyName,
    int                 sessionIndex)
{
    std::string filename = copyName + "." + std::to_string(sessionIndex) + SHA256_CHECKSUM_BINARY_FILENAME_EXTENSION;
    return copyMetaDirPath + SEPARATOR + filename;
}

std::string common::GetCopyDataFilePath(
    const std::string&  copyDataDirPath,
    const std::string&  copyName,
    CopyFormat          copyFormat,
    int                 sessionIndex)
{
    std::string suffix = COPY_DATA_BIN_FILENAME_EXTENSION;
    std::string filename;
    if (copyFormat == CopyFormat::BIN && sessionIndex == 0) {
        filename = copyName + COPY_DATA_BIN_FILENAME_EXTENSION;
    } else if (copyFormat == CopyFormat::IMAGE) {
        filename = copyName + COPY_DATA_IMAGE_FILENAME_EXTENSION;
    } else if (copyFormat == CopyFormat::BIN && sessionIndex != 0) {
        filename = copyName + COPY_DATA_BIN_PARTED_FILENAME_EXTENSION + std::to_string(sessionIndex);
#ifdef _WIN32
    } else if (copyFormat == CopyFormat::VHD_FIXED || copyFormat == CopyFormat::VHD_DYNAMIC) {
        filename = copyName + COPY_DATA_VHD_FILENAME_EXTENSION;
    } else if (copyFormat == CopyFormat::VHDX_FIXED || copyFormat == CopyFormat::VHDX_DYNAMIC) {
        filename = copyName + COPY_DATA_VHDX_FILENAME_EXTENSION;
    } else if (copyFormat == CopyFormat::FILE) {
        filename = copyName;
#endif
    }
    return copyDataDirPath + SEPARATOR + filename;
}

std::string common::GetWriterBitmapFilePath(
    const std::string&  copyMetaDirPath,
    const std::string&  copyName,
    int                 sessionIndex)
{
    std::string filename = copyName + "." + std::to_string(sessionIndex) + WRITER_BITMAP_FILENAME_EXTENSION;
    return copyMetaDirPath + SEPARATOR + filename;
}

std::string common::GetFileName(const std::string& fullpath)
{
    auto pos = fullpath.rfind(SEPARATOR);
    return pos == std::string::npos ? fullpath : fullpath.substr(pos + 1);
}

std::string common::GetParentDirectoryPath(const std::string& fullpath)
{
    std::string parentDirPath = fullpath;
    while (!parentDirPath.empty() && parentDirPath.back() == SEPARATOR[0]) {
        parentDirPath.pop_back();
    }
    auto pos = parentDirPath.rfind(SEPARATOR);
    return pos == std::string::npos ? "" : fullpath.substr(0, pos);
}

bool common::WriteVolumeCopyMeta(
    const std::string& copyMetaDirPath,
    const std::string& copyName,
    const VolumeCopyMeta& volumeCopyMeta)
{
    std::string filepath = copyMetaDirPath + SEPARATOR + copyName + VOLUME_COPY_META_JSON_FILENAME_EXTENSION;
    return JsonSerialize(volumeCopyMeta, filepath);
}

bool common::ReadVolumeCopyMeta(
    const std::string& copyMetaDirPath,
    const std::string& copyName,
    VolumeCopyMeta& volumeCopyMeta)
{
    std::string filepath = copyMetaDirPath + SEPARATOR + copyName + VOLUME_COPY_META_JSON_FILENAME_EXTENSION;
    return JsonDeserialize(volumeCopyMeta, filepath);
}