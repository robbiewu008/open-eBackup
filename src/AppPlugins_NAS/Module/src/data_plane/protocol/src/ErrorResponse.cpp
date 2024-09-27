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
#include <protocol/ErrorResponse.h>

#include <protocol/MessageHeader.h>

namespace Module {
namespace Protocol {

ErrorResponse::ErrorResponse(const std::shared_ptr<Protocol::Message>& message)
    : m_type(*reinterpret_cast<const ErrorType*>(message->Data().data())),
    m_requestHandler(*reinterpret_cast<const std::size_t*>(message->Data().data() + sizeof(m_type))),
    m_data(message)
{ }

ErrorResponse::ErrorType ErrorResponse::Type() const
{
    return m_type;
}

std::size_t ErrorResponse::RequestHandler() const
{
    return m_requestHandler;
}

boost::string_view ErrorResponse::Message() const
{
    return { m_data->Data().data() + sizeof(m_type) + sizeof(m_requestHandler),
             m_data->Data().size() - sizeof(m_type) - sizeof(m_requestHandler) };
}

const std::shared_ptr<Protocol::Message>& ErrorResponse::Data() const
{
    return m_data;
}

std::shared_ptr<Protocol::Message> ErrorResponse::NewErrorResponse(ErrorType type,
                                                                   std::size_t requestHandler,
                                                                   const std::string& message)
{
    constexpr static MessageType messageType = MessageType::ERROR;
    decltype(MessageHeader::messageLength) messageLength = sizeof(type) + sizeof(requestHandler) + message.size();

    auto protocolMessage = std::make_shared<Protocol::Message>();
    protocolMessage->Reserve(sizeof(MessageHeader) + messageLength);

    // Copy message header
    protocolMessage->AddData(reinterpret_cast<const char*>(&messageType), sizeof(messageType));
    protocolMessage->AddData(reinterpret_cast<char*>(&messageLength), sizeof(messageLength));

    // Copy error type
    protocolMessage->AddData(reinterpret_cast<char*>(&type), sizeof(type));
    protocolMessage->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    // Copy error message
    protocolMessage->AddData(message.data(), message.size());

    return protocolMessage;
}

}
}
