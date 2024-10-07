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
#include <protocol/AuthorizationMessage.h>
#include <protocol/MessageHeader.h>
#include "log/Log.h"

namespace Module {
namespace Protocol {
namespace {
// Sizes
constexpr std::size_t AUTHORIZATION_MESSAGE_SIZE_OF_SIZE = sizeof(AuthorizationMessage::AuthorizationMessageSize);
}

AuthorizationMessage::AuthorizationMessage(const std::shared_ptr<Message>& message)
    : m_type(*reinterpret_cast<const AuthorizationMessageType*>(message->Data().data())),
    m_data(message)
{ }

AuthorizationMessageType AuthorizationMessage::Type() const
{
    return m_type;
}

boost::string_view AuthorizationMessage::Login() const
{
    switch (m_type) {
        case AuthorizationMessageType::REQUEST_LOGIN:
            {
                const auto* loginSize = reinterpret_cast<AuthorizationMessageSize*>(m_data->Data().data() +
                                                                                    sizeof(AuthorizationMessageType));
                return {m_data->Data().data() + sizeof(AuthorizationMessageType) + AUTHORIZATION_MESSAGE_SIZE_OF_SIZE,
                        *loginSize};
            }
        case AuthorizationMessageType::RESPONSE_LOGIN:
        case AuthorizationMessageType::REQUEST_KEEP_ALIVE:
        case AuthorizationMessageType::RESPONSE_KEEP_ALIVE:
        default:
            return m_emptyString;
    }
}

boost::string_view AuthorizationMessage::Token() const
{
    switch (m_type) {
        case AuthorizationMessageType::REQUEST_LOGIN:
        default:
            return m_emptyString;
        case AuthorizationMessageType::RESPONSE_LOGIN:
        case AuthorizationMessageType::REQUEST_KEEP_ALIVE:
        case AuthorizationMessageType::RESPONSE_KEEP_ALIVE:
            return { m_data->Data().data() + sizeof(AuthorizationMessageType),
                     m_data->Data().size() - sizeof(AuthorizationMessageType) };
    }
}

const std::shared_ptr<Message>& AuthorizationMessage::Data() const
{
    return m_data;
}

std::shared_ptr<Message> AuthorizationMessage::NewLoginRequest()
{
    auto message = AllocateBuffer(AuthorizationMessageType::REQUEST_LOGIN, 0);
    return message;
}

std::shared_ptr<Message> AuthorizationMessage::NewLoginResponse(const std::string& token)
{
    const auto tokenSize = static_cast<AuthorizationMessageSize>(token.size());
    auto message = AllocateBuffer(AuthorizationMessageType::RESPONSE_LOGIN, tokenSize);

    // Copy data
    message->AddData(token.data(), tokenSize);

    return message;
}

std::shared_ptr<Message> AuthorizationMessage::NewKeepAliveRequest(const std::string& token)
{
    const auto tokenSize = static_cast<AuthorizationMessageSize>(token.size());
    auto message = AllocateBuffer(AuthorizationMessageType::REQUEST_KEEP_ALIVE, tokenSize);

    // Copy data
    message->AddData(token.data(), tokenSize);

    return message;
}

std::shared_ptr<Message> AuthorizationMessage::NewKeepAliveResponse(const std::string& token)
{
    const auto tokenSize = static_cast<AuthorizationMessageSize>(token.size());
    auto message = AllocateBuffer(AuthorizationMessageType::RESPONSE_KEEP_ALIVE, tokenSize);

    // Copy data
    message->AddData(token.data(), tokenSize);

    return message;
}

std::shared_ptr<Message> AuthorizationMessage::AllocateBuffer(AuthorizationMessageType type,
                                                              AuthorizationMessageSize dataLength)
{
    constexpr static MessageType messageType = MessageType::AUTHORIZATION;

    auto authorizationMessage = std::make_shared<Message>();
    decltype(MessageHeader::messageLength) messageLength = sizeof(AuthorizationMessageType);
    messageLength += dataLength;
    authorizationMessage->Reserve(sizeof(MessageHeader) + messageLength);
    // Copy message header
    authorizationMessage->AddData(reinterpret_cast<const char*>(&messageType), sizeof(messageType));
    authorizationMessage->AddData(reinterpret_cast<char*>(&messageLength), sizeof(messageLength));
    // Copy message type
    authorizationMessage->AddData(reinterpret_cast<char*>(&type), sizeof(type));

    return authorizationMessage;
}

}
}
