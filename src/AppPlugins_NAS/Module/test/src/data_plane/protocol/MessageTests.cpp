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
#include <cstring>

// Vendors
#include <gtest/gtest.h>

// DataMove
#include <protocol/MessageHeader.h>
#include <protocol/Message.h>

using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
using Module::Protocol::MessageType;

namespace {
constexpr std::size_t MESSAGE_SIZE = 1024 * 1024 * 10;  // 10MB
constexpr auto TEST_MESSAGE_DATA = "Some test data will be used in tests";
const std::size_t TEST_MESSAGE_DATA_LENGTH = std::strlen(TEST_MESSAGE_DATA);
}

namespace Module {
TEST(Message, Reserve)
{
    // const case for coverage
    const Message constMessage;
    ASSERT_EQ(constMessage.Data().size(), 0);
    ASSERT_EQ(constMessage.Data().capacity(), 0);

    Message message;
    ASSERT_EQ(message.Data().size(), 0);
    ASSERT_EQ(message.Data().capacity(), 0);

    message.Reserve(MESSAGE_SIZE);
    ASSERT_EQ(message.Data().size(), 0);
    ASSERT_EQ(message.Data().capacity(), MESSAGE_SIZE);
}

TEST(Message, Resize)
{
    // const case for coverage
    const Message constMessage;
    ASSERT_EQ(constMessage.Data().size(), 0);
    ASSERT_EQ(constMessage.Data().capacity(), 0);

    Message message;
    ASSERT_EQ(message.Data().size(), 0);
    ASSERT_EQ(message.Data().capacity(), 0);

    message.Resize(MESSAGE_SIZE);
    ASSERT_EQ(message.Data().size(), MESSAGE_SIZE);
    ASSERT_EQ(message.Data().capacity(), MESSAGE_SIZE);
}

TEST(Message, AddData)
{
    Message message;

    message.AddData(TEST_MESSAGE_DATA, TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(message.Data().size(), TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(std::string(message.Data().data(), message.Data().size()),
              std::string(TEST_MESSAGE_DATA, TEST_MESSAGE_DATA_LENGTH));

    // mutable case for coverage
    message.Data().clear();
    ASSERT_EQ(message.Data().size(), 0);

    message.AddData(const_cast<char *>(TEST_MESSAGE_DATA), TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(message.Data().size(), TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(std::string(message.Data().data(), message.Data().size()),
              std::string(TEST_MESSAGE_DATA, TEST_MESSAGE_DATA_LENGTH));
}

TEST(Message, AddDataToReserved)
{
    Message message;

    message.Reserve(MESSAGE_SIZE);
    message.AddData(TEST_MESSAGE_DATA, TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(message.Data().size(), TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(message.Data().capacity(), MESSAGE_SIZE);
}

TEST(Message, AddDataToResized)
{
    Message message;

    message.Resize(MESSAGE_SIZE);
    message.AddData(TEST_MESSAGE_DATA, TEST_MESSAGE_DATA_LENGTH);
    ASSERT_EQ(message.Data().size(), MESSAGE_SIZE + TEST_MESSAGE_DATA_LENGTH);
    ASSERT_TRUE(message.Data().capacity() > MESSAGE_SIZE);
}

TEST(Message, RemoveHeader)
{
    Message message;
    message.Reserve(sizeof(MessageHeader));

    // Copy message header
    constexpr static MessageType messageType = MessageType::ERROR;
    decltype(MessageHeader::messageLength) messageLength = 0;

    message.AddData(reinterpret_cast<const char *>(&messageType), sizeof(messageType));
    message.AddData(reinterpret_cast<char *>(&messageLength), sizeof(messageLength));
    ASSERT_EQ(message.Data().size(), sizeof(MessageHeader));

    auto *messageHeader = reinterpret_cast<MessageHeader *>(message.Data().data());
    ASSERT_EQ(messageHeader->type, messageType);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    message.RemoveHeader();
    ASSERT_EQ(message.Data().size(), 0);

    message.RemoveHeader();
    ASSERT_EQ(message.Data().size(), 0);
}

}
