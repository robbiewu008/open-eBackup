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
#ifndef DP_RENAME_H
#define DP_RENAME_H

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

class RenameRequest final {
public:
    RenameRequest() = delete;

    explicit RenameRequest(boost::string_view data) : m_data(data) { }

    std::uint64_t RequestHandler() const
    {
        return *reinterpret_cast<const std::uint64_t*>(m_data.data());
    }
    fuse_ino_t ParentInodeNumber() const
    {
        return *reinterpret_cast<const fuse_ino_t*>(m_data.data() + sizeof(std::uint64_t));
    }
    fuse_ino_t NewParentInodeNumber() const
    {
        return *reinterpret_cast<const fuse_ino_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t));
    }
    std::uint32_t Flags() const
    {
        return *reinterpret_cast<const std::uint32_t*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) +
                                                                       sizeof(fuse_ino_t));
    }
    AuxDataSize NameSize() const
    {
        return *reinterpret_cast<const AuxDataSize*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) +
                                                                     sizeof(fuse_ino_t) + sizeof(std::uint32_t));
    }
    boost::string_view Name(AuxDataSize nameSize) const
    {
        return { m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) + sizeof(fuse_ino_t) +
                                 sizeof(std::uint32_t) + sizeof(AuxDataSize),
                 nameSize };
    }
    AuxDataSize NewNameSize(AuxDataSize nameSize) const
    {
        return *reinterpret_cast<const AuxDataSize*>(m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) +
                                                                     sizeof(fuse_ino_t) + sizeof(std::uint32_t) +
                                                                     sizeof(AuxDataSize) + nameSize);
    }
    boost::string_view NewName(AuxDataSize nameSize, AuxDataSize newNameSize) const
    {
        return { m_data.data() + sizeof(std::uint64_t) + sizeof(fuse_ino_t) + sizeof(fuse_ino_t) +
                                 sizeof(std::uint32_t) + sizeof(AuxDataSize) + nameSize + sizeof(AuxDataSize),
                 newNameSize };
    }

private:
    boost::string_view m_data;
};

class RenameResponse final {
public:
    enum class ResponseType : std::uint8_t {
        SUCCESS,
        FAIL,
        NO_PARENT_DIRECTORY,
        NO_NEW_PARENT_DIRECTORY
    };

    RenameResponse() = delete;

    explicit RenameResponse(boost::string_view data) : m_data(data) { }

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

private:
    boost::string_view m_data;
};

}
}
}

#endif