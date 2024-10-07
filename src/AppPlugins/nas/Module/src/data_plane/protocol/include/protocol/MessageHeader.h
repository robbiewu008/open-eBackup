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
#ifndef DP_MESSAGE_HEADER_H
#define DP_MESSAGE_HEADER_H
// std
#include <cstdint>

namespace Module {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11

enum class MessageType : std::uint32_t {
    AUTHORIZATION,
    FUSE,
    ERROR,
    FILE_ADMIN
};

/*!
 * \brief
 *
 * \details Message type is 32bits length + 32bits for message length. It is done for proper memory alignment.
 */
struct MessageHeader {
public:
    MessageType type;
    std::uint32_t messageLength;
};

}
}

#endif