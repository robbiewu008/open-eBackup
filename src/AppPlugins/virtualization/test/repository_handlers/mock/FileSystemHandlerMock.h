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
#ifndef __FILESYSTEM_HANDLER_H__
#define __FILESYSTEM_HANDLER_H__

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <repository_handlers/RepositoryHandler.h>
#include "repository_handlers/filesystem/FileSystemHandler.h"

namespace HDT_TEST {
class FileSystemHandlerMock : public VirtPlugin::RepositoryHandler {
public:
    MOCK_METHOD(int32_t, Open, (const std::string &fileName, const std::string &mode), (override));
    MOCK_METHOD(int32_t, Truncate, (const uint64_t &size), (override));
    MOCK_METHOD(int32_t, Close, (), (override));
    MOCK_METHOD(size_t, Read, (std::shared_ptr<uint8_t[]> buf, size_t count), (override));
    MOCK_METHOD(size_t, Read, (std::string &buf, size_t count), (override));
    MOCK_METHOD(size_t, Write, (const std::shared_ptr<uint8_t[]> &buf, size_t count), (override));
    MOCK_METHOD(size_t, Write, (const std::string &str), (override));
    MOCK_METHOD(size_t, Append, (std::shared_ptr<uint8_t[]> buf, size_t count), (override));
    MOCK_METHOD(int64_t, Seek, (size_t offset, int origin), (override));
    MOCK_METHOD(int64_t, Tell, (), (override));
    MOCK_METHOD(size_t, FileSize, (const std::string &fileName), (override));
    MOCK_METHOD(bool, Flush, (bool sync), (override));
    MOCK_METHOD(bool, Exists, (const std::string &fileName), (override));
    MOCK_METHOD(bool, Rename, (const std::string &oldName, const std::string &newName), (override));
    MOCK_METHOD(bool, CopyFile, (const std::string &srcName, const std::string &destName), (override));
    MOCK_METHOD(bool, IsDirectory, (const std::string& path), (override));
    MOCK_METHOD(bool, IsRegularFile, (const std::string& fileName), (override));
    MOCK_METHOD(bool, Remove, (const std::string &fileName), (override));
    MOCK_METHOD(bool, RemoveAll, (const std::string &dirName), (override));
    MOCK_METHOD(bool, CreateDirectory, (const std::string &dirName), (override));
    MOCK_METHOD(void, GetFiles, (std::string pathName, std::vector<std::string> &files), (override));
};
}

#endif //__FILESYSTEM_HANDLER_H__