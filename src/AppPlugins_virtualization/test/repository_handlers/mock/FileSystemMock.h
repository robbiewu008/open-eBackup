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
#ifndef __FILESYSTEM_MOCK_H__
#define __FILESYSTEM_MOCK_H__

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "common/Structs.h"
#include <repository_handlers/RepositoryHandler.h>
#include "repository_handlers/filesystem/FileSystemHandler.h"

using namespace VirtPlugin;

namespace HDT_TEST {

class FileSystemMock : public VirtPlugin::FileSystemHandler {
public:
    size_t Read(std::string &buf, size_t count) override;
    size_t FileSize(const std::string &fileName) override;
    bool Flush(bool sync = true) override;
    int32_t Truncate(const uint64_t &size) override;
    bool Remove(const std::string &fileName) override;
    size_t Write(const std::string &str) override;
    bool CreateDirectory(const std::string &dirName) override;
    size_t Read(std::shared_ptr<uint8_t[]> buf, size_t count) override;
    bool Exists(const std::string &fileName) override;
    int32_t Open(const std::string &fileName, const std::string &mode) override;
    int64_t Seek(size_t offset, int origin) override;
    // size_t Write(std::shared_ptr<uint8_t[]> buf, size_t count) override;
    size_t Write(const std::shared_ptr<uint8_t[]> &buf, size_t count) override;
    int32_t Close() override;

private:
    std::string m_fileName;
};
}
#endif