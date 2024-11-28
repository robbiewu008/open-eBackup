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
#ifndef BACKUP_S3_IO_H
#define BACKUP_S3_IO_H

#include "BackupIOInterface.h"
#include "S3System.hpp"
#include "UploadFile.h"
#include "GuardedFile.h"

namespace Module {
class BackupS3IO : public BackupIOInterface {
public:
    BackupS3IO(const IODeviceInfo &info);
    virtual ~BackupS3IO() override;

    bool Initialize() override;
    bool CreateDirectory(const std::string &directoryName) override;
    bool IsDirectory(const std::string &directoryName) override;
    bool IsFileExist(const std::string &fileName) override;
    bool GetDirectoryList(const std::string &directoryName, std::vector<std::string> &elementList) override;
    bool GetFileListInDirectory(const std::string &directoryName, std::vector<std::string> &elementList) override;
    bool GetObjectListWithMarker(const std::string &directoryName, std::string &marker, bool &isEnd,
        std::vector<std::string> &elementList, int maxSize = DEFAULT_MAX_SIZE) override;
    bool GetSpaceInfo(const std::string &pathName, uint64_t &capacity, uint64_t &free) override;

    bool Append(const std::string &remoteFile, const std::string &buffer) override;
    bool ReadFile(const std::string &remoteFile, std::string &buffer) override;
    bool ReadFile(const std::string &remoteFile, const std::string &localFile) override;
    bool DownloadFile(const std::string &remoteFile, const std::string &localFile, ReadFileCallback &handle) override;
    int32_t ReadFile(const std::string &remoteFile, BinaryData &data) override;
    int32_t ReadFile(const std::string &remoteFile, uint64_t offset, BinaryData &data) override;
    bool WriteFile(const std::string &remoteFile, const std::string &buffer) override;
    bool WriteFile(const std::string &localFile, const std::string &remoteFile, int threadCount,
        CallBackHandle &handle) override;
    bool WriteFile(const BinaryData &packageData, const std::string &remoteFile, CallBackHandle &handle) override;
    bool Delete(const std::string &remoteFile) override;
    bool DeleteAll(const std::string &directoryName, CallBackHandle &handle) override;
    void SetUpLoadRateLimit(uint64_t qos) override;
    void SetDownLoadRateLimit(uint64_t qos) override;

protected:
    void FormatFileName(std::string &s3path, const std::string &relativePath);
    bool GetFileContentLen(const std::string &fileName, uint64_t &ContentLen);
    bool QueryFileInfo(const std::string &remoteFile, std::size_t &dataSize, std::size_t &offset);
    bool UpdateFileInfo(const std::string &remoteFile, const std::size_t &offset);
    void DeleteFileInfo(const std::string &remoteFile);

private:
    struct FileInfo {
        std::size_t offset{0};
        std::size_t dataSize{0};
    };
    IODeviceInfo m_deviceInfo;
    std::unique_ptr<UploadFile> m_upload{nullptr};
    std::unique_ptr<GuardedFile> m_snapGf{nullptr};
    std::unique_ptr<S3SystemIO> m_s3IO{nullptr};
    std::map<std::string, FileInfo> m_fileCache;
    uint64_t m_UpLoadRateLimit {0};
    uint64_t m_DownLoadRateLimit {0};
};
}
#endif
