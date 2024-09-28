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
#include <repository_handlers/mock/FileSystemMock.h>
#include <cstring>

using namespace HDT_TEST;

const std::string UUID = "1c0b87cf-18c5-4b90-8753-838d02f8d25c";

std::string Stub_FileSystemReadSnapshot()
{
    SnapshotInfo snapshotInfo;
    VolSnapInfo volSnapshotInfo;
    volSnapshotInfo.m_volUuid = UUID;
    snapshotInfo.m_volSnapList.push_back(volSnapshotInfo);
    std::string snapshotInfoStr;
    Module::JsonHelper::StructToJsonString(snapshotInfo, snapshotInfoStr);
    return snapshotInfoStr;
}

namespace HDT_TEST {
size_t FileSystemMock::Read(std::string &buf, size_t count)
{
    std::string return_value = Stub_FileSystemReadSnapshot();
    buf = return_value;
    return return_value.length();
}

bool FileSystemMock::Flush(bool sync)
{
    return true;
}

int32_t FileSystemMock::Truncate(const uint64_t &size)
{
    return SUCCESS;
}

bool FileSystemMock::Remove(const std::string &fileName)
{
    return true;
}

size_t FileSystemMock::Write(const std::string &str)
{
    return str.length();
}

bool FileSystemMock::CreateDirectory(const std::string &dirName)
{
    return SUCCESS;
}

bool FileSystemMock::Exists(const std::string &fileName)
{
    if (fileName.find("volumes_block_bitmap") > fileName.length()) {
        return true;
    }
    m_fileName = "/tmp/volumes_block_bitmap.info";
    return FileSystemHandler::Exists(m_fileName);
}

int32_t FileSystemMock::Open(const std::string &fileName, const std::string &mode)
{
    if (fileName.find("volumes_block_bitmap") > fileName.length()) {
        return 0;
    }
    m_fileName = "/tmp/volumes_block_bitmap.info";
    return FileSystemHandler::Open(m_fileName, mode);
}

int64_t FileSystemMock::Seek(size_t offset, int origin)
{
    if (m_fileName.find("volumes_block_bitmap") > m_fileName.length()) {
        return 0;
    }
    return FileSystemHandler::Seek(offset, origin);
}

size_t FileSystemMock::Write(const std::shared_ptr<uint8_t[]> &buf, size_t count)
{
    if (m_fileName.find("volumes_block_bitmap") > m_fileName.length()) {
        return count;
    }
    return FileSystemHandler::Write(buf, count);
}

int32_t FileSystemMock::Close()
{
    if (m_fileName.find("volumes_block_bitmap") > m_fileName.length()) {
        return 0;
    }
    return FileSystemHandler::Close();
}

size_t FileSystemMock::Read(std::shared_ptr<uint8_t[]> buf, size_t count)
{
    if (m_fileName.find("volumes_block_bitmap") > m_fileName.length()) {
        std::string snapInfo = Stub_FileSystemReadSnapshot();
        buf = std::make_unique<uint8_t[]>(snapInfo.length());
        memcpy(buf.get(), &snapInfo[0], snapInfo.length());
        return count;
    }
    return FileSystemHandler::Read(buf, count);
}

size_t FileSystemMock::FileSize(const std::string &fileName)
{
    if (fileName.find("volumes_block_bitmap") > fileName.length()) {
        std::string snapInfo = Stub_FileSystemReadSnapshot();
        return snapInfo.length();
    }
    return FileSystemHandler::FileSize("/tmp/volumes_block_bitmap.info");
}
}
