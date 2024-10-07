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
#ifndef VHD_HANDLER_H
#define VHD_HANDLER_H

#ifdef WIN32
#include <windows.h>
#include <virtdisk.h>
#include <repository_handlers/RepositoryHandler.h>
#include <functional>

namespace VirtPlugin {
class VhdHandler : public RepositoryHandler {
public:
    VhdHandler() = default;
    ~VhdHandler() = default;

    int32_t CreateVHD();
    int32_t Open(const std::string &fileName, const std::string &mode) override;
    int32_t Truncate(const uint64_t &size) override;
    int32_t Close() override;
    size_t Read(std::shared_ptr<uint8_t[]> buf, size_t count) override;
    size_t Read(std::string &buf, size_t count) override;
    size_t Write(const std::shared_ptr<uint8_t[]> &buf, size_t count) override;
    size_t Write(const std::string &str) override;
    size_t Append(std::shared_ptr<uint8_t[]> buf, size_t count) override;
    int64_t Tell() override;
    int64_t Seek(size_t offset, int origin = SEEK_SET) override;
    size_t FileSize(const std::string &fileName) override;
    bool Flush(bool sync = true) override;
    bool Exists(const std::string &fileName) override;
    bool Rename(const std::string &oldName, const std::string &newName) override;
    virtual bool CopyFile(const std::string &srcName, const std::string &destName) override;
    virtual bool IsDirectory(const std::string &path) override;
    virtual bool IsRegularFile(const std::string &fileName) override;
    bool Remove(const std::string &fileName) override;
    bool RemoveAll(const std::string &dirName) override;
    bool CreateDirectory(const std::string &dirName) override;
    void GetFiles(std::string pathName, std::vector<std::string> &files) override;

private:
    bool OpenVHDDisk(const std::string &fileName);
    bool AttachVHDDisk(ATTACH_VIRTUAL_DISK_FLAG attachFlags);

private:
    std::string m_fileName;
    std::string m_mode;
    size_t m_offset = 0;
    HANDLE m_vhdHandle = INVALID_HANDLE_VALUE;
};
}
#endif
#endif