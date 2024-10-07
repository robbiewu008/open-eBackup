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
#ifndef ARCHIVE_CLIENT_BASE_H
#define ARCHIVE_CLIENT_BASE_H

#include <string>
#include <vector>

class ArchiveRequest {
public:
    std::string m_jobId;
    std::string m_copyId;
    std::string m_fileSystemId;
    std::string m_fileName; // 输入的文件名
    uint64_t m_offset {0};  // 输入的偏移量
    uint64_t m_size {0};    // 输入的长度
    int m_readSizeType {0};
    int m_rspSizeType {0};
    uint8_t* m_buffer { nullptr };
};

class ArchiveResponse {
public:
    std::string m_fileName;
    uint64_t m_offset {0}; // 读取的偏移量
    uint64_t m_size {0};   // 读取到的实际长度
    int m_readEnd {0};     // 文件是否读取完  1：文件已获取完  0：文件尚未获取完
};

class ArchiveClientBase {
public:
    ArchiveClientBase() {}
    virtual ~ArchiveClientBase() {}
    virtual int GetFileData(const ArchiveRequest& req, ArchiveResponse& rsp) = 0;
};

#endif // ENGINE_BASE_H