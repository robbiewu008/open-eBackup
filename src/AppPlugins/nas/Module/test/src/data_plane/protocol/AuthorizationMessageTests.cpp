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
#include <gtest/gtest.h>

// DataMove
#include <protocol/AuthorizationMessage.h>
#include <protocol/MessageHeader.h>

using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
using Module::Protocol::MessageType;

namespace {
constexpr auto TOKEN = "slkjhfslkjfajhflkajkjaflkanskjnajkfna";
const auto TOKEN_LENGTH = std::strlen(TOKEN);
}

namespace Module {
TEST(AuthorizationMessage, NewLoginRequest)
{
    auto loginRequest = Protocol::AuthorizationMessage::NewLoginRequest();

    Protocol::AuthorizationMessage authorizationMessage(loginRequest);
    ASSERT_EQ(authorizationMessage.Type(), Protocol::AuthorizationMessageType::REQUEST_LOGIN);
    ASSERT_EQ(authorizationMessage.Token(), "");
    ASSERT_EQ(authorizationMessage.Data(), loginRequest);
}

TEST(AuthorizationMessage, NewLoginResponse)
{
    auto loginResponse = Protocol::AuthorizationMessage::NewLoginResponse(TOKEN);

    decltype(MessageHeader::messageLength) messageLength = sizeof(Protocol::AuthorizationMessageType);
    messageLength += TOKEN_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(loginResponse->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::AUTHORIZATION);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    loginResponse->RemoveHeader();
    Protocol::AuthorizationMessage authorizationMessage(loginResponse);
    ASSERT_EQ(authorizationMessage.Type(), Protocol::AuthorizationMessageType::RESPONSE_LOGIN);
    ASSERT_EQ(authorizationMessage.Login(), "");
    ASSERT_EQ(authorizationMessage.Token(), TOKEN);
    ASSERT_EQ(authorizationMessage.Data(), loginResponse);
}

TEST(AuthorizationMessage, NewKeepAliveRequest)
{
    auto tokenRequest = Protocol::AuthorizationMessage::NewKeepAliveRequest(TOKEN);

    decltype(MessageHeader::messageLength) messageLength = sizeof(Protocol::AuthorizationMessageType);
    messageLength += TOKEN_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(tokenRequest->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::AUTHORIZATION);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    tokenRequest->RemoveHeader();
    Protocol::AuthorizationMessage authorizationMessage(tokenRequest);
    ASSERT_EQ(authorizationMessage.Type(), Protocol::AuthorizationMessageType::REQUEST_KEEP_ALIVE);
    ASSERT_EQ(authorizationMessage.Login(), "");
    ASSERT_EQ(authorizationMessage.Token(), TOKEN);
    ASSERT_EQ(authorizationMessage.Data(), tokenRequest);
}

TEST(AuthorizationMessage, NewKeepAliveResponse)
{
    auto tokenResponse = Protocol::AuthorizationMessage::NewKeepAliveResponse(TOKEN);

    decltype(MessageHeader::messageLength) messageLength = sizeof(Protocol::AuthorizationMessageType);
    messageLength += TOKEN_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(tokenResponse->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::AUTHORIZATION);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    tokenResponse->RemoveHeader();
    Protocol::AuthorizationMessage authorizationMessage(tokenResponse);
    ASSERT_EQ(authorizationMessage.Type(), Protocol::AuthorizationMessageType::RESPONSE_KEEP_ALIVE);
    ASSERT_EQ(authorizationMessage.Login(), "");
    ASSERT_EQ(authorizationMessage.Token(), TOKEN);
    ASSERT_EQ(authorizationMessage.Data(), tokenResponse);
}
}
