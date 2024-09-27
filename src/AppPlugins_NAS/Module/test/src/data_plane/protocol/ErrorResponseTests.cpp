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
#include <random>
#include <limits>

// Vendors
#include <gtest/gtest.h>

// DataMove
#include <protocol/MessageHeader.h>
#include <protocol/ErrorResponse.h>

using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
using Module::Protocol::MessageType;

namespace {
constexpr auto ERROR_MESSAGE = "Error message";
const auto ERROR_MESSAGE_LENGTH = std::strlen(ERROR_MESSAGE);
}

namespace Module {
void TestNewErrorResponse(Protocol::ErrorResponse::ErrorType errorType, std::size_t requestHandler)
{
    auto errorResponseMessage = Protocol::ErrorResponse::NewErrorResponse(errorType, requestHandler, ERROR_MESSAGE);

    auto *messageHeader = reinterpret_cast<MessageHeader *>(errorResponseMessage->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::ERROR);
    ASSERT_EQ(messageHeader->messageLength, sizeof(errorType) + sizeof(requestHandler) + ERROR_MESSAGE_LENGTH);

    errorResponseMessage->RemoveHeader();
    Protocol::ErrorResponse errorResponse(errorResponseMessage);
    ASSERT_EQ(errorResponse.Type(), errorType);
    ASSERT_EQ(errorResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(errorResponse.Message(), ERROR_MESSAGE);
    ASSERT_EQ(errorResponse.Data(), errorResponseMessage);
}

TEST(ErrorResponse, NewErrorResponse)
{
    std::random_device gDevice;
    std::mt19937_64 gGenerator(gDevice());
    std::uniform_int_distribution<std::mt19937_64::result_type> distribution(
        0, std::numeric_limits<std::mt19937_64::result_type>::max());
    const auto requestHandler = static_cast<std::size_t>(distribution(gGenerator));

    for (auto i = static_cast<std::size_t>(Protocol::ErrorResponse::ErrorType::RESPONSE_RECEIVED);
         i < static_cast<std::size_t>(Protocol::ErrorResponse::ErrorType::INVALID_TARGET_SOURCE); ++i) {
        TestNewErrorResponse(static_cast<Protocol::ErrorResponse::ErrorType>(i), requestHandler);
    }
}
}
