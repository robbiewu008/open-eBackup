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
#ifndef DP_WRITE_BUFFER_H
#define DP_WRITE_BUFFER_H
// std
#include <cstdint>

// Fuse
#define FUSE_USE_VERSION 34
#include <fuse_lowlevel.h>

// Vendors
#include <boost/utility/string_view.hpp>

// DataMove
#include <protocol/Utilities.h>

namespace DataMove {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11
namespace Fuse { // Ugly, but nested namespaces are not present in C++11

class WriteBufferRequest final {
public:
    WriteBufferRequest() = delete;

    explicit WriteBufferRequest(boost::string_view data) : m_data(data) { }

    std::uint64_t RequestHandler() const
    {
        return *reinterpret_cast<const std::uint64_t*>(m_data.data());
    }
    fuse_ino_t InodeNumber() const
    {
        return *reinterpret_cast<const fuse_ino_t*>(m_data.data() + sizeof(std::uint64_t));
    }
    uint64_t FileHandle() const
    {
        return *reinterpret_cast<const uint64_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t));
    }
    off_t Offset() const
    {
        return *reinterpret_cast<const off_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) +
                                                               sizeof(uint64_t));
    }
    boost::string_view Data() const
    {
        return { m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) + sizeof(uint64_t) + sizeof(off_t),
                 m_data.size() - sizeof(std::uint64_t) - sizeof(fuse_ino_t) - sizeof(uint64_t) - sizeof(off_t) };
    }

private:
    boost::string_view m_data;
};

class WriteBufferResponse final {
public:
    enum class ResponseType : std::uint8_t {
        SUCCESS,
        FAIL
    };

    WriteBufferResponse() = delete;

    explicit WriteBufferResponse(boost::string_view data) : m_data(data) { }

    std::uint64_t RequestHandler() const
    {
        return *reinterpret_cast<const std::uint64_t*>(m_data.data());
    }
    ResponseType Type() const
    {
        return *reinterpret_cast<const ResponseType*>(m_data.data() + sizeof(std::uint64_t));
    }
    int Error() const
    {
        return *reinterpret_cast<const int*>(m_data.data() + sizeof(std::uint64_t) + sizeof(ResponseType));
    }
    std::uint64_t Size() const
    {
        return *reinterpret_cast<const std::uint64_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(ResponseType));
    }

private:
    boost::string_view m_data;
};

}
}
}

#endif