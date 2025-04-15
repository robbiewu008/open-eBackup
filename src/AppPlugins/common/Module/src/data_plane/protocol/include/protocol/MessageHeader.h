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

enum class MessageType : std::uint8_t {
    AUTHORIZATION,
    FUSE,
    ERROR,
    FILE_ADMIN,
    OSAD_ADMIN
};

enum class ProtocolVersion : std::uint8_t {
    V1
};

/*!
 * \brief
 *
 * \details Message type is 8bits length + 32bits for message length. It is done for proper memory alignment.
 */
struct MessageHeader {
public:
    MessageType type;
    ProtocolVersion version;
    std::uint16_t sequenceNumber;
    std::uint32_t messageLength;
};
}
}
constexpr Module::Protocol::ProtocolVersion PROTOCOL_VERSION_V1 = Module::Protocol::ProtocolVersion::V1;
const uint16_t SEQUENCE_NUMBER_ZERO = 0;
#endif