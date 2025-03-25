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
#include <protocol/OsadAdminMessage.h>

// DataMove
#include <protocol/MessageHeader.h>

using Module::Protocol::MessageType;
using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
namespace DataMove {
namespace Protocol {

namespace {
constexpr MessageType MESSAGE_TYPE = MessageType::OSAD_ADMIN;
}


std::shared_ptr<Message> AllocateRequestBuffer(decltype(MessageHeader::messageLength) messageLength,
    uint16_t sequenceNum)
{
    auto osadAdminMessage = std::make_shared<Message>();
    osadAdminMessage->Reserve(sizeof(MessageHeader) + messageLength);

    // Add message header and allocat enough space
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&PROTOCOL_VERSION_V1), sizeof(PROTOCOL_VERSION_V1));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&sequenceNum), sizeof(sequenceNum));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength));

    return osadAdminMessage;
}

std::shared_ptr<Message> OsadAdminRequest::NewQuerySpeedRequest(const std::string& sourceId, bool isAvgSpeed,
    uint16_t sequenceNum)
{
    auto type = RequestType::QUERY_CURRENT_SPEED;
    if (isAvgSpeed) {
        type = RequestType::QUERY_AVG_SPEED;
    }
    const auto sourceIdSize = static_cast<AuxDataSize>(sourceId.size());
    auto messageBodySize = sizeof(type) + sizeof(sourceIdSize) + sourceIdSize;
    auto osadAdminMessage = AllocateRequestBuffer(messageBodySize, sequenceNum);
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&type), sizeof(type));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&sourceIdSize), sizeof(sourceIdSize));
    osadAdminMessage->AddData(sourceId.c_str(), sourceIdSize);
    return osadAdminMessage;
}

std::shared_ptr<Message> OsadAdminResponse::NewResponse(ResponseType type, const std::string& message,
    uint16_t sequenceNum)
{
    constexpr auto constantSize = sizeof(MessageHeader) + sizeof(type);
    const auto messageSize = message.size();

    auto osadAdminMessage = std::make_shared<Module::Protocol::Message>();
    osadAdminMessage->Reserve(constantSize + messageSize);

    // Copy message header
    const decltype(MessageHeader::messageLength) messageBodyLength = sizeof(type) + messageSize;
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&PROTOCOL_VERSION_V1), sizeof(PROTOCOL_VERSION_V1));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&sequenceNum), sizeof(sequenceNum));
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&messageBodyLength), sizeof(messageBodyLength));

    // Copy type and message
    osadAdminMessage->AddData(reinterpret_cast<const char*>(&type), sizeof(type));
    osadAdminMessage->AddData(message.c_str(), messageSize);

    return osadAdminMessage;
}

}
}
