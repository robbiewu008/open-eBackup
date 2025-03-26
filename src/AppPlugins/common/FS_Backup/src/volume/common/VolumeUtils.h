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
#ifndef VOLUMEBACKUP_BACKUP_UTIL_H
#define VOLUMEBACKUP_BACKUP_UTIL_H

#include "common/VolumeProtectMacros.h"
// external logger/json library
#include "common/JsonHelper.h"
#include "log/Log.h"
#include "VolumeProtector.h"

namespace volumeprotect {

// volume data in [offset, offset + length) store in the file
struct CopySegment {
    std::string                 copyDataFile;           // name of the copy file
    std::string                 checksumBinFile;        // name of checksum binary file
    int                         index;                  // session index
    uint64_t                    offset;                 // volume offset
    uint64_t                    length;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyDataFile, copyDataFile);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(checksumBinFile, checksumBinFile);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(index, index);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(offset, offset);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(length, length);
    END_SERIAL_MEMEBER
};

struct VolumeCopyMeta {
    std::string                 copyName;
    int                         backupType;     // cast BackupType to int
    int                         copyFormat;     // cast CopyFormat to int
    uint64_t                    volumeSize;     // volume size in bytes
    uint32_t                    blockSize;      // block size in bytes
    std::string                 volumePath;
    std::vector<CopySegment>    segments;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyName, copyName);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(backupType, backupType);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(copyFormat, copyFormat);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeSize, volumeSize);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumePath, volumePath);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(blockSize, blockSize);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(segments, segments);
    END_SERIAL_MEMEBER
};

/**
 * @brief common utils
 */
namespace common {

std::string GetChecksumBinPath(
    const std::string&  copyMetaDirPath,
    const std::string&  copyName,
    int                 sessionIndex
);

std::string GetCopyDataFilePath(
    const std::string&  copyDataDirPath,
    const std::string&  copyName,
    CopyFormat          copyFormat,
    int                 sessionIndex
);

std::string GetWriterBitmapFilePath(
    const std::string&  copyMetaDirPath,
    const std::string&  copyName,
    int                 sessionIndex
);

std::string GetFileName(const std::string& fullpath);

std::string GetParentDirectoryPath(const std::string& fullpath);

bool VOLUMEPROTECT_API WriteVolumeCopyMeta(
    const std::string& copyMetaDirPath,
    const std::string& copyName,
    const VolumeCopyMeta& volumeCopyMeta);

bool VOLUMEPROTECT_API ReadVolumeCopyMeta(
    const std::string& copyMetaDirPath,
    const std::string& copyName,
    VolumeCopyMeta& volumeCopyMeta);

template<typename T>
bool JsonSerialize(const T& record, const std::string& filepath)
{
    std::string jsonContent;
    T recordTmp = record;
    if (!Module::JsonHelper::StructToJsonString(recordTmp, jsonContent)) {
        WARNLOG("StructToJsonString failed");
    }
    try {
        std::ofstream file(filepath, std::ios::trunc);
        if (!file.is_open()) {
            ERRLOG("failed to open file %s to write json %s", filepath.c_str(), jsonContent.c_str());
            return false;
        }
        file << jsonContent;
        file.close();
    } catch (const std::exception& e) {
        ERRLOG("failed to write json %s, exception: %s", filepath.c_str(), e.what());
        return false;
    } catch (...) {
        ERRLOG("failed to write json %s, exception caught", filepath.c_str());
        return false;
    }
    return true;
}

template<typename T>
bool JsonDeserialize(T& record, const std::string& filepath)
{
    std::ifstream file(filepath);
    try {
        std::ifstream file(filepath);
        std::string jsonContent;
        if (!file.is_open()) {
            ERRLOG("failed to open file %s to read json %s", filepath.c_str(), jsonContent.c_str());
            return false;
        }
        getline(file, jsonContent);
        file >> jsonContent;
        file.close();
        if (!Module::JsonHelper::JsonStringToStruct(jsonContent, record)) {
            WARNLOG("JsonStringToStruct failed, %s", jsonContent.c_str());
        }
    } catch (const std::exception& e) {
        ERRLOG("failed to read json %s, exception: %s", filepath.c_str(), e.what());
        return false;
    } catch (...) {
        ERRLOG("failed to read json %s, exception caught", filepath.c_str());
        return false;
    }
    return true;
}

}
}

#endif