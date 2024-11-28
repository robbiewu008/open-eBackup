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
#ifndef BACKUP_IO_INTERFACE_H
#define BACKUP_IO_INTERFACE_H

#include <string>
#include <vector>
#include <functional>
#include "LayoutCommon.h"

namespace Module {
enum class MediaType {
    OBS = 1,
    FS
};

constexpr static int DEFAULT_MAX_SIZE = 100;

class BackupIOInterface {
public:
    virtual ~BackupIOInterface() = default;

    virtual bool Initialize() = 0;
    virtual bool CreateDirectory(const std::string &directoryName) = 0;
    virtual bool IsDirectory(const std::string &directoryName) = 0;
    virtual bool IsFileExist(const std::string &fileName) = 0;
    virtual bool GetDirectoryList(const std::string &directoryName, std::vector<std::string> &elementList) = 0;
    virtual bool GetFileListInDirectory(const std::string &directoryName, std::vector<std::string> &elementList) = 0;
    virtual bool GetObjectListWithMarker(const std::string &directoryName, std::string &marker, bool &isEnd,
        std::vector<std::string> &elementList, int maxSize = DEFAULT_MAX_SIZE) = 0;
    virtual bool GetSpaceInfo(const std::string &pathName, uint64_t &capacity, uint64_t &free) = 0;

    virtual bool Append(const std::string &remoteFile, const std::string& buffer) = 0;
    virtual bool WriteFile(const std::string &remoteFile, const std::string& buffer) = 0;
    virtual bool WriteFile(const std::string &localFile, const std::string &remoteFile, int threadCount,
        CallBackHandle &handle) = 0;
    virtual bool WriteFile(const BinaryData &packageData, const std::string &remoteFile, CallBackHandle &handle) = 0;

    virtual bool ReadFile(const std::string &remoteFile, std::string &buffer) = 0;
    virtual bool ReadFile(const std::string &remoteFile, const std::string &localFile) = 0;
    virtual bool DownloadFile(
        const std::string &remoteFile, const std::string &localFile, ReadFileCallback &handle) = 0;
    virtual int32_t ReadFile(const std::string &remoteFile, BinaryData &data) = 0;
    virtual int32_t ReadFile(const std::string &remoteFile, uint64_t offset, BinaryData &data) = 0;

    virtual bool Delete(const std::string &remoteFile) = 0;
    virtual bool DeleteAll(const std::string &directoryName, CallBackHandle &handle) = 0;

    virtual void SetUpLoadRateLimit(uint64_t qos) = 0;
    virtual void SetDownLoadRateLimit(uint64_t qos) = 0;

    int32_t GetS3ObjNameMaxLength() const {return m_s3ObjNameMaxLength;}
    int32_t GetMediaType() const {return m_mediaType;}
    std::string GetWorkSpace() const {return m_workSpace;}
    void SetS3ObjNameMaxLength(const uint32_t length) {m_s3ObjNameMaxLength = length;}
    void SetMediaType(const uint32_t mediaType) {m_mediaType = mediaType;}
    void SetWorkSpace(const std::string &workSpace) {m_workSpace = workSpace;}

    void WriteParallelReset() { m_writeParallelCount = 0; }
    int GetWriteParallelCount() const { return m_writeParallelCount; }
protected:
    int m_writeParallelCount {0};
private:
    int32_t m_s3ObjNameMaxLength {1024}; // s3 maximum object length supported.
    int32_t m_mediaType {static_cast<int32_t>(MediaType::OBS)};
    std::string m_workSpace; // remote work space
};
}
#endif