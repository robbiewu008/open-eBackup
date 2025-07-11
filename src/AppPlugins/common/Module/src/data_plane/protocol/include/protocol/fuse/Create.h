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
#ifndef DP_CREATE_REQUEST_H
#define DP_CREATE_REQUEST_H
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

class CreateRequest final {
public:
    CreateRequest() = delete;

    explicit CreateRequest(boost::string_view data) : m_data(data) { }

    std::uint64_t RequestHandler() const
    {
        return *reinterpret_cast<const std::uint64_t*>(m_data.data());
    }
    fuse_ino_t ParentInodeNumber() const
    {
        return *reinterpret_cast<const fuse_ino_t*>(m_data.data() + sizeof(std::uint64_t));
    }
    std::uint32_t Mode() const
    {
        return *reinterpret_cast<const std::uint32_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t));
    }
    const UserGroup* Uidgid() const
    {
        return reinterpret_cast<const UserGroup*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) +
            sizeof(std::uint32_t));
    }
    const char* Name() const
    {
        return m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) + sizeof(std::uint32_t) + sizeof(UserGroup);
    }

private:
    boost::string_view m_data;
};

class CreateResponse final {
public:
    enum class ResponseType : std::uint8_t {
        SUCCESS,
        FAIL
    };

    CreateResponse() = delete;

    explicit CreateResponse(boost::string_view data) : m_data(data) { }

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
    const FuseEntryParams* Params() const
    {
        return reinterpret_cast<const FuseEntryParams*>(m_data.data() + sizeof(std::uint64_t) + sizeof(ResponseType));
    }
    uint64_t FileHandle() const
    {
        return *reinterpret_cast<const uint64_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(ResponseType) +
                                                             sizeof(FuseEntryParams));
    }

private:
    boost::string_view m_data;
};

}
}
}
#endif
