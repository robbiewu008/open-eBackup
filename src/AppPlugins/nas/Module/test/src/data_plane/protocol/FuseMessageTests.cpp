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
#include <protocol/MessageHeader.h>
#include <protocol/fuse/FuseMessage.h>
#include "Helpers.h"

using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
using Module::Protocol::MessageType;

namespace DataMove {
std::random_device g_device;
std::mt19937_64 g_generator(g_device());
TEST(FuseMessage, UpdateTokenInRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto parentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewLookupRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                 parentInodeNumber, ENTRY_NAME);
    Protocol::Fuse::FuseMessage::UpdateTokenInRequest(request, NEW_TOKEN);
    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Token(), NEW_TOKEN);
}

TEST(FuseMessage, LookupRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto parentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewLookupRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                 parentInodeNumber, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(fuse_ino_t) + ENTRY_NAME_LENGTH + sizeof(char) + FUSE_MESSAGE_REQUEST_HEADER_SIZE +
        TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LOOKUP);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto lookupRequest = fuseMessage.GetRequest<Protocol::Fuse::LookupRequest>();
    ASSERT_EQ(lookupRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(lookupRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(std::string(lookupRequest.Name()), ENTRY_NAME);
}

TEST(FuseMessage, LookupResponseSuccess)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewLookupResponse(
        requestHandler, Protocol::Fuse::LookupResponse::ResponseType::SUCCESS, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::LookupResponse::ResponseType) +
                                                           sizeof(FuseEntryParams) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LOOKUP);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto lookupResponse = fuseMessage.GetResponse<Protocol::Fuse::LookupResponse>();
    ASSERT_EQ(lookupResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(lookupResponse.Type(), Protocol::Fuse::LookupResponse::ResponseType::SUCCESS);
    const auto responseParams = lookupResponse.Params()->ToParams();
    CompareFuseEntryParams(responseParams, params);
}

TEST(FuseMessage, LookupResponseNoSuchFileOrDirectory)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewLookupResponse(
        requestHandler, Protocol::Fuse::LookupResponse::ResponseType::NO_SUCH_FILE_OR_DIRECTORY, &params);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::LookupResponse::ResponseType) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LOOKUP);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto lookupResponse = fuseMessage.GetResponse<Protocol::Fuse::LookupResponse>();
    ASSERT_EQ(lookupResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(lookupResponse.Type(), Protocol::Fuse::LookupResponse::ResponseType::NO_SUCH_FILE_OR_DIRECTORY);
}

TEST(FuseMessage, GetAttributeRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto inodeNumber = g_distribution(g_generator);
    auto request =
        Protocol::Fuse::FuseMessage::NewGetAttributesRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(fuse_ino_t) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto lookupRequest = fuseMessage.GetRequest<Protocol::Fuse::GetAttributesRequest>();
    ASSERT_EQ(lookupRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(lookupRequest.InodeNumber(), inodeNumber);
}

TEST(FuseMessage, GetAttributeResponseSuccess)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewGetAttributesResponse(
        requestHandler, Protocol::Fuse::GetAttributesResponse::ResponseType::SUCCESS, 0, &params.attr);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::GetAttributesResponse::ResponseType) +
                                                           sizeof(Attributes) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetAttributesResponse>();
    ASSERT_EQ(getAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getAttributesResponse.Type(), Protocol::Fuse::GetAttributesResponse::ResponseType::SUCCESS);
    const auto responseAttributes = getAttributesResponse.Attr()->ToStat();
    CompareAttributes(responseAttributes, params.attr);
}

TEST(FuseMessage, GetAttributeResponseFail)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewGetAttributesResponse(
        requestHandler, Protocol::Fuse::GetAttributesResponse::ResponseType::FAIL, ERROR, &params.attr);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::GetAttributesResponse::ResponseType) +
                                                           sizeof(int) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetAttributesResponse>();
    ASSERT_EQ(getAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getAttributesResponse.Type(), Protocol::Fuse::GetAttributesResponse::ResponseType::FAIL);
    ASSERT_EQ(getAttributesResponse.Error(), ERROR);
}

TEST(FuseMessage, GetExtendedAttributesRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto inodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewGetExtendedAttributesRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                                inodeNumber, EXPECTED_SIZE, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(fuse_ino_t) + sizeof(std::size_t) + ENTRY_NAME_LENGTH +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto getExtendedAttributesRequest = fuseMessage.GetRequest<Protocol::Fuse::GetExtendedAttributesRequest>();
    ASSERT_EQ(getExtendedAttributesRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(getExtendedAttributesRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(getExtendedAttributesRequest.Name(), ENTRY_NAME);
    ASSERT_EQ(getExtendedAttributesRequest.Size(), EXPECTED_SIZE);
}

TEST(FuseMessage, GetExtendedAttributesResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewGetExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::SUCCESS, 0, 0, &someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::GetExtendedAttributesResponse::ResponseType) + someData.size() +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE + sizeof(char);

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetExtendedAttributesResponse>();
    ASSERT_EQ(getExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getExtendedAttributesResponse.Type(),
              Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::SUCCESS);
    ASSERT_EQ(std::string(getExtendedAttributesResponse.Data().data(), getExtendedAttributesResponse.Data().size() - 1),
              someData);
}

TEST(FuseMessage, GetExtendedAttributesResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewGetExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::FAIL, 0, ERROR, &someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::GetExtendedAttributesResponse::ResponseType) + sizeof(int) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetExtendedAttributesResponse>();
    ASSERT_EQ(getExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getExtendedAttributesResponse.Type(), Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::FAIL);
    ASSERT_EQ(getExtendedAttributesResponse.Error(), ERROR);
}

TEST(FuseMessage, GetExtendedAttributesResponseSize)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewGetExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::SIZE, EXPECTED_SIZE, ERROR,
        &someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::GetExtendedAttributesResponse::ResponseType) +
        sizeof(std::size_t) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetExtendedAttributesResponse>();
    ASSERT_EQ(getExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getExtendedAttributesResponse.Type(), Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::SIZE);
    ASSERT_EQ(getExtendedAttributesResponse.Size(), EXPECTED_SIZE);
}

TEST(FuseMessage, GetExtendedAttributesResponseNoAvailableData)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewGetExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::NO_AVAILABLE_DATA, EXPECTED_SIZE,
        ERROR, &someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::GetExtendedAttributesResponse::ResponseType) + sizeof(int) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::GETXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto getExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::GetExtendedAttributesResponse>();
    ASSERT_EQ(getExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(getExtendedAttributesResponse.Type(),
              Protocol::Fuse::GetExtendedAttributesResponse::ResponseType::NO_AVAILABLE_DATA);
    ASSERT_EQ(getExtendedAttributesResponse.Error(), ERROR);
}

TEST(FuseMessage, ReadDirectoryRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto inodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewReadDirectoryRequest(
        TOKEN, TARGET_SOURCE, requestHandler, inodeNumber, EXPECTED_SIZE, EXPECTED_OFFSET, true);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(fuse_ino_t) + sizeof(std::size_t) + sizeof(std::size_t) + sizeof(bool) +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READDIRPLUS);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto readDirectoryRequest = fuseMessage.GetRequest<Protocol::Fuse::ReadDirectoryRequest>();
    ASSERT_EQ(readDirectoryRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(readDirectoryRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(readDirectoryRequest.Size(), EXPECTED_SIZE);
    ASSERT_EQ(readDirectoryRequest.Offset(), EXPECTED_OFFSET);
    ASSERT_EQ(readDirectoryRequest.Plus(), true);
}

TEST(FuseMessage, ReadDirectoryResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewReadDirectoryResponse(
        requestHandler, Protocol::Fuse::ReadDirectoryResponse::ResponseType::SUCCESS, false, ERROR, EXPECTED_SIZE,
        EXPECTED_OFFSET, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ReadDirectoryResponse::ResponseType) + sizeof(bool) +
        sizeof(std::size_t) + sizeof(std::size_t) + someData.size() + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadDirectoryResponse>();
    ASSERT_EQ(readDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readDirectoryResponse.Type(), Protocol::Fuse::ReadDirectoryResponse::ResponseType::SUCCESS);
    ASSERT_EQ(readDirectoryResponse.Plus(), false);
    ASSERT_EQ(readDirectoryResponse.Size(), EXPECTED_SIZE);
    ASSERT_EQ(readDirectoryResponse.Offset(), EXPECTED_OFFSET);
    ASSERT_EQ(std::string(readDirectoryResponse.Data()), someData);
}

TEST(FuseMessage, ReadDirectoryResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewReadDirectoryResponse(
        requestHandler, Protocol::Fuse::ReadDirectoryResponse::ResponseType::FAIL, false, ERROR, EXPECTED_SIZE,
        EXPECTED_OFFSET, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ReadDirectoryResponse::ResponseType) + sizeof(bool) + sizeof(int) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadDirectoryResponse>();
    ASSERT_EQ(readDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readDirectoryResponse.Type(), Protocol::Fuse::ReadDirectoryResponse::ResponseType::FAIL);
    ASSERT_EQ(readDirectoryResponse.Plus(), false);
    ASSERT_EQ(readDirectoryResponse.Error(), ERROR);
}

TEST(FuseMessage, ReadLinkRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto inodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewReadLinkRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(fuse_ino_t) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto readLinkRequest = fuseMessage.GetRequest<Protocol::Fuse::ReadLinkRequest>();
    ASSERT_EQ(readLinkRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(readLinkRequest.InodeNumber(), inodeNumber);
}

TEST(FuseMessage, ReadLinkResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewReadLinkResponse(
        requestHandler, Protocol::Fuse::ReadLinkResponse::ResponseType::SUCCESS, ERROR, &someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ReadLinkResponse::ResponseType) + someData.size() + sizeof(char) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readLinkResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadLinkResponse>();
    ASSERT_EQ(readLinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readLinkResponse.Type(), Protocol::Fuse::ReadLinkResponse::ResponseType::SUCCESS);
    ASSERT_EQ(readLinkResponse.Link(), someData);
}

TEST(FuseMessage, ReadLinkResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "jshdgkjsdhghjaskjf";
    auto response = Protocol::Fuse::FuseMessage::NewReadLinkResponse(
        requestHandler, Protocol::Fuse::ReadLinkResponse::ResponseType::FAIL, ERROR, &someData);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::ReadLinkResponse::ResponseType) +
                                                           sizeof(int) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readLinkResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadLinkResponse>();
    ASSERT_EQ(readLinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readLinkResponse.Type(), Protocol::Fuse::ReadLinkResponse::ResponseType::FAIL);
    ASSERT_EQ(readLinkResponse.Error(), ERROR);
}

TEST(FuseMessage, CreateRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto parentInodeNumber = g_distribution(g_generator);
    const std::uint32_t mode = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewCreateRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                 parentInodeNumber, mode, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(fuse_ino_t) + sizeof(mode) + sizeof(char) + ENTRY_NAME_LENGTH +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::CREATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto createRequest = fuseMessage.GetRequest<Protocol::Fuse::CreateRequest>();
    ASSERT_EQ(createRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(createRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(createRequest.Mode(), mode);
    ASSERT_EQ(std::string(createRequest.Name()), std::string(ENTRY_NAME));
}

TEST(FuseMessage, CreateResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewCreateResponse(
        requestHandler, Protocol::Fuse::CreateResponse::ResponseType::SUCCESS, ERROR, &params, fileHandle);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::CreateResponse::ResponseType) + sizeof(fileHandle) +
        sizeof(FuseEntryParams) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::CREATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto createResponse = fuseMessage.GetResponse<Protocol::Fuse::CreateResponse>();
    ASSERT_EQ(createResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(createResponse.Type(), Protocol::Fuse::CreateResponse::ResponseType::SUCCESS);
    CompareFuseEntryParams(createResponse.Params()->ToParams(), params);
    ASSERT_EQ(createResponse.FileHandle(), fileHandle);
}

TEST(FuseMessage, CreateResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewCreateResponse(
        requestHandler, Protocol::Fuse::CreateResponse::ResponseType::FAIL, ERROR, &params, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::CreateResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::CREATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto createResponse = fuseMessage.GetResponse<Protocol::Fuse::CreateResponse>();
    ASSERT_EQ(createResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(createResponse.Type(), Protocol::Fuse::CreateResponse::ResponseType::FAIL);
    ASSERT_EQ(createResponse.Error(), ERROR);
}

TEST(FuseMessage, SetAttributeRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto inodeNumber = g_distribution(g_generator);
    const int fieldsToSet = static_cast<uint64_t>(g_distribution(g_generator));
    const auto params = GetEntryParams();
    auto request = Protocol::Fuse::FuseMessage::NewSetAttributeRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                       inodeNumber, fieldsToSet, &params.attr);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(fuse_ino_t) + sizeof(fieldsToSet) + sizeof(Attributes) +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto setAttributeRequest = fuseMessage.GetRequest<Protocol::Fuse::SetAttributeRequest>();
    ASSERT_EQ(setAttributeRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(setAttributeRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(setAttributeRequest.FieldsToSet(), fieldsToSet);
    CompareAttributes(setAttributeRequest.Attr().ToStat(), params.attr);
}

TEST(FuseMessage, SetAttributeResponseSuccess)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewSetAttributeResponse(
        requestHandler, Protocol::Fuse::SetAttributeResponse::ResponseType::SUCCESS, 0, &params.attr);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(Attributes) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto setAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::SetAttributeResponse>();
    ASSERT_EQ(setAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(setAttributesResponse.Type(), Protocol::Fuse::SetAttributeResponse::ResponseType::SUCCESS);
    const auto responseAttributes = *setAttributesResponse.Attr();
    CompareAttributes(responseAttributes.ToStat(), params.attr);
}

TEST(FuseMessage, SetAttributeResponseFail)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewSetAttributeResponse(
        requestHandler, Protocol::Fuse::SetAttributeResponse::ResponseType::FAIL, ERROR, &params.attr);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto setAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::SetAttributeResponse>();
    ASSERT_EQ(setAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(setAttributesResponse.Type(), Protocol::Fuse::SetAttributeResponse::ResponseType::FAIL);
    ASSERT_EQ(setAttributesResponse.Error(), ERROR);
}

TEST(FuseMessage, SetAttributeResponseCtimeOnDeletedEntry)
{
    const auto params = GetEntryParams();
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewSetAttributeResponse(
        requestHandler, Protocol::Fuse::SetAttributeResponse::ResponseType::CTIME_ON_DELETED_ENTRY, ERROR,
        &params.attr);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SETATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto setAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::SetAttributeResponse>();
    ASSERT_EQ(setAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(setAttributesResponse.Type(), Protocol::Fuse::SetAttributeResponse::ResponseType::CTIME_ON_DELETED_ENTRY);
    ASSERT_EQ(setAttributesResponse.Error(), ERROR);
}

TEST(FuseMessage, ReleaseRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewReleaseRequest(TOKEN, TARGET_SOURCE, requestHandler, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(fileHandle) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RELEASE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto releaseRequest = fuseMessage.GetRequest<Protocol::Fuse::ReleaseRequest>();
    ASSERT_EQ(releaseRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(releaseRequest.FileHandle(), fileHandle);
}

TEST(FuseMessage, FlushRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewFlushRequest(TOKEN, TARGET_SOURCE, requestHandler, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(fileHandle) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FLUSH);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto releaseRequest = fuseMessage.GetRequest<Protocol::Fuse::ReleaseRequest>();
    ASSERT_EQ(releaseRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(releaseRequest.FileHandle(), fileHandle);
}

TEST(FuseMessage, ReleaseResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewReleaseResponse(
        requestHandler, Protocol::Fuse::ReleaseResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RELEASE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto releaseResponse = fuseMessage.GetResponse<Protocol::Fuse::ReleaseResponse>();
    ASSERT_EQ(releaseResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(releaseResponse.Type(), Protocol::Fuse::ReleaseResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, ReleaseResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewReleaseResponse(
        requestHandler, Protocol::Fuse::ReleaseResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RELEASE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto releaseResponse = fuseMessage.GetResponse<Protocol::Fuse::ReleaseResponse>();
    ASSERT_EQ(releaseResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(releaseResponse.Type(), Protocol::Fuse::ReleaseResponse::ResponseType::FAIL);
    ASSERT_EQ(releaseResponse.Error(), ERROR);
}

TEST(FuseMessage, FlushResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewFlushResponse(
        requestHandler, Protocol::Fuse::FlushResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FLUSH);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto flushResponse = fuseMessage.GetResponse<Protocol::Fuse::FlushResponse>();
    ASSERT_EQ(flushResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(flushResponse.Type(), Protocol::Fuse::FlushResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, FlushResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewFlushResponse(
        requestHandler, Protocol::Fuse::FlushResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FLUSH);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto flushResponse = fuseMessage.GetResponse<Protocol::Fuse::FlushResponse>();
    ASSERT_EQ(flushResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(flushResponse.Type(), Protocol::Fuse::FlushResponse::ResponseType::FAIL);
    ASSERT_EQ(flushResponse.Error(), ERROR);
}

TEST(FuseMessage, OpenRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t inodeNumber = g_distribution(g_generator);
    const int flags = static_cast<int>(g_distribution(g_generator));
    auto request =
        Protocol::Fuse::FuseMessage::NewOpenRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber, flags);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(inodeNumber) + sizeof(flags) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::OPEN);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto openRequest = fuseMessage.GetRequest<Protocol::Fuse::OpenRequest>();
    ASSERT_EQ(openRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(openRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(openRequest.Flags(), flags);
}

TEST(FuseMessage, OpenResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    auto response = Protocol::Fuse::FuseMessage::NewOpenResponse(
        requestHandler, Protocol::Fuse::OpenResponse::ResponseType::SUCCESS, ERROR, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(fileHandle) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::OPEN);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto openResponse = fuseMessage.GetResponse<Protocol::Fuse::OpenResponse>();
    ASSERT_EQ(openResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(openResponse.Type(), Protocol::Fuse::OpenResponse::ResponseType::SUCCESS);
    ASSERT_EQ(openResponse.FileHandle(), fileHandle);
}

TEST(FuseMessage, OpenResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    auto response = Protocol::Fuse::FuseMessage::NewOpenResponse(
        requestHandler, Protocol::Fuse::OpenResponse::ResponseType::FAIL, ERROR, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SetAttributeResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::OPEN);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto openResponse = fuseMessage.GetResponse<Protocol::Fuse::OpenResponse>();
    ASSERT_EQ(openResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(openResponse.Type(), Protocol::Fuse::OpenResponse::ResponseType::FAIL);
    ASSERT_EQ(openResponse.Error(), ERROR);
}

TEST(FuseMessage, WriteBufferRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t inodeNumber = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    const auto offset = static_cast<off_t>(g_distribution(g_generator));
    const std::string someData = "jahfjahf jkahfiqhrb97c6r7 t8rt 61 ;uiq;uwatyaw7ftaw7fiy w  rwyr87twfwgefu hsaeu "
                                 "fgYGF\\NKLJ WEIHJW\\ NKJ I/./SKFS84SF/SF;kjhsdufhg4f hsUHGUEy9826tu23bnr9";
    auto request = Protocol::Fuse::FuseMessage::NewWriteBufferRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber,
                                                                      fileHandle, offset, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(inodeNumber) + sizeof(fileHandle) + sizeof(offset) + someData.size() +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::WRITE_BUF);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto writeBufferRequest = fuseMessage.GetRequest<Protocol::Fuse::WriteBufferRequest>();
    ASSERT_EQ(writeBufferRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(writeBufferRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(writeBufferRequest.FileHandle(), fileHandle);
    ASSERT_EQ(writeBufferRequest.Offset(), offset);
    ASSERT_EQ(std::string(writeBufferRequest.Data().data(), writeBufferRequest.Data().size()), someData);
}

TEST(FuseMessage, WriteBufferResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto size = static_cast<std::uint64_t>(g_distribution(g_generator));
    auto response = Protocol::Fuse::FuseMessage::NewWriteBufferResponse(
        requestHandler, Protocol::Fuse::WriteBufferResponse::ResponseType::SUCCESS, ERROR, size);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::WriteBufferResponse::ResponseType) +
                                                           sizeof(size) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::WRITE_BUF);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto writeBufferResponse = fuseMessage.GetResponse<Protocol::Fuse::WriteBufferResponse>();
    ASSERT_EQ(writeBufferResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(writeBufferResponse.Type(), Protocol::Fuse::WriteBufferResponse::ResponseType::SUCCESS);
    ASSERT_EQ(writeBufferResponse.Size(), size);
}

TEST(FuseMessage, WriteBufferResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto size = static_cast<std::uint64_t>(g_distribution(g_generator));
    auto response = Protocol::Fuse::FuseMessage::NewWriteBufferResponse(
        requestHandler, Protocol::Fuse::WriteBufferResponse::ResponseType::FAIL, ERROR, size);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::WriteBufferResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::WRITE_BUF);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto writeBufferResponse = fuseMessage.GetResponse<Protocol::Fuse::WriteBufferResponse>();
    ASSERT_EQ(writeBufferResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(writeBufferResponse.Type(), Protocol::Fuse::WriteBufferResponse::ResponseType::FAIL);
    ASSERT_EQ(writeBufferResponse.Error(), ERROR);
}

TEST(FuseMessage, ReadRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const uint64_t fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    const auto size = static_cast<std::uint64_t>(g_distribution(g_generator));
    const auto offset = static_cast<off_t>(g_distribution(g_generator));
    auto request =
        Protocol::Fuse::FuseMessage::NewReadRequest(TOKEN, TARGET_SOURCE, requestHandler, size, offset, fileHandle);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(size) + sizeof(fileHandle) +
                                                           sizeof(offset) + FUSE_MESSAGE_REQUEST_HEADER_SIZE +
                                                           TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READ);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto readRequest = fuseMessage.GetRequest<Protocol::Fuse::ReadRequest>();
    ASSERT_EQ(readRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(readRequest.Size(), size);
    ASSERT_EQ(readRequest.FileHandle(), fileHandle);
    ASSERT_EQ(readRequest.Offset(), offset);
}

TEST(FuseMessage, ReadResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "sljhdfjsgh3uurv28 yt3hthw.t/w h348y7sufhssohdv-se=8712`usgvjh3479bjovbjvbdbcvm/czknc "
                                 "fgYGF\\NKLJ WEIHJW\\ NKJ I/./SKFS84SF/SF;kjhsdufhg4f hsUHGUEy9826tu23bnr9";
    auto response = Protocol::Fuse::FuseMessage::NewReadResponse(
        requestHandler, Protocol::Fuse::ReadResponse::ResponseType::SUCCESS, ERROR, &someData, someData.size());

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::ReadResponse::ResponseType) +
                                                           someData.size() + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READ);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadResponse>();
    ASSERT_EQ(readResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readResponse.Type(), Protocol::Fuse::ReadResponse::ResponseType::SUCCESS);
    ASSERT_EQ(std::string(readResponse.Data().data(), readResponse.Data().size()), someData);
}

TEST(FuseMessage, ReadResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const std::string someData = "sljhdfjsgh3uurv28 yt3hthw.t/w h348y7sufhssohdv-se=8712`usgvjh3479bjovbjvbdbcvm/czknc "
                                 "fgYGF\\NKLJ WEIHJW\\ NKJ I/./SKFS84SF/SF;kjhsdufhg4f hsUHGUEy9826tu23bnr9";
    auto response = Protocol::Fuse::FuseMessage::NewReadResponse(
        requestHandler, Protocol::Fuse::ReadResponse::ResponseType::FAIL, ERROR, &someData, someData.size());

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::ReadResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::READ);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto readResponse = fuseMessage.GetResponse<Protocol::Fuse::ReadResponse>();
    ASSERT_EQ(readResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(readResponse.Type(), Protocol::Fuse::ReadResponse::ResponseType::FAIL);
    ASSERT_EQ(readResponse.Error(), ERROR);
}

TEST(FuseMessage, UnlinkRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t parentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewUnlinkRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                 parentInodeNumber, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(parentInodeNumber) +
                                                           ENTRY_NAME_LENGTH + FUSE_MESSAGE_REQUEST_HEADER_SIZE +
                                                           TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::UNLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto unlinkRequest = fuseMessage.GetRequest<Protocol::Fuse::UnlinkRequest>();
    ASSERT_EQ(unlinkRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(unlinkRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(std::string(unlinkRequest.Name()), std::string(ENTRY_NAME));
}

TEST(FuseMessage, UnlinkResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewUnlinkResponse(
        requestHandler, Protocol::Fuse::UnlinkResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::UnlinkResponse::ResponseType) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::UNLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto unlinkResponse = fuseMessage.GetResponse<Protocol::Fuse::UnlinkResponse>();
    ASSERT_EQ(unlinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(unlinkResponse.Type(), Protocol::Fuse::UnlinkResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, UnlinkResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewUnlinkResponse(
        requestHandler, Protocol::Fuse::UnlinkResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::UnlinkResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::UNLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto unlinkResponse = fuseMessage.GetResponse<Protocol::Fuse::UnlinkResponse>();
    ASSERT_EQ(unlinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(unlinkResponse.Type(), Protocol::Fuse::UnlinkResponse::ResponseType::FAIL);
    ASSERT_EQ(unlinkResponse.Error(), ERROR);
}

TEST(FuseMessage, RemoveDirectoryRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t parentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewRemoveDirectoryRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                          parentInodeNumber, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(parentInodeNumber) +
                                                           ENTRY_NAME_LENGTH + FUSE_MESSAGE_REQUEST_HEADER_SIZE +
                                                           TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RMDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto removeDirectoryRequest = fuseMessage.GetRequest<Protocol::Fuse::RemoveDirectoryRequest>();
    ASSERT_EQ(removeDirectoryRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(removeDirectoryRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(std::string(removeDirectoryRequest.Name()), std::string(ENTRY_NAME));
}

TEST(FuseMessage, RemoveDirectoryResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewRemoveDirectoryResponse(
        requestHandler, Protocol::Fuse::RemoveDirectoryResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::UnlinkResponse::ResponseType) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RMDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto removeDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::RemoveDirectoryResponse>();
    ASSERT_EQ(removeDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(removeDirectoryResponse.Type(), Protocol::Fuse::RemoveDirectoryResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, RemoveDirectoryResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewRemoveDirectoryResponse(
        requestHandler, Protocol::Fuse::RemoveDirectoryResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::UnlinkResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RMDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto removeDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::RemoveDirectoryResponse>();
    ASSERT_EQ(removeDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(removeDirectoryResponse.Type(), Protocol::Fuse::RemoveDirectoryResponse::ResponseType::FAIL);
    ASSERT_EQ(removeDirectoryResponse.Error(), ERROR);
}

TEST(FuseMessage, MakeDirectoryRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t parentInodeNumber = g_distribution(g_generator);
    const auto mode = static_cast<mode_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewMakeDirectoryRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                        parentInodeNumber, mode, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(parentInodeNumber) + sizeof(mode) + ENTRY_NAME_LENGTH +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::MKDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto makeDirectoryRequest = fuseMessage.GetRequest<Protocol::Fuse::MakeDirectoryRequest>();
    ASSERT_EQ(makeDirectoryRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(makeDirectoryRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(makeDirectoryRequest.Mode(), mode);
    ASSERT_EQ(std::string(makeDirectoryRequest.Name()), std::string(ENTRY_NAME));
}

TEST(FuseMessage, MakeDirectoryResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewMakeDirectoryResponse(
        requestHandler, Protocol::Fuse::MakeDirectoryResponse::ResponseType::SUCCESS, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::MakeDirectoryResponse::ResponseType) +
                                                           sizeof(FuseEntryParams) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::MKDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto makeDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::MakeDirectoryResponse>();
    ASSERT_EQ(makeDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(makeDirectoryResponse.Type(), Protocol::Fuse::MakeDirectoryResponse::ResponseType::SUCCESS);
    CompareFuseEntryParams(makeDirectoryResponse.Params()->ToParams(), params);
}

TEST(FuseMessage, MakeDirectoryResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewMakeDirectoryResponse(
        requestHandler, Protocol::Fuse::MakeDirectoryResponse::ResponseType::FAIL, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::MakeDirectoryResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::MKDIR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto makeDirectoryResponse = fuseMessage.GetResponse<Protocol::Fuse::MakeDirectoryResponse>();
    ASSERT_EQ(makeDirectoryResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(makeDirectoryResponse.Type(), Protocol::Fuse::MakeDirectoryResponse::ResponseType::FAIL);
    ASSERT_EQ(makeDirectoryResponse.Error(), ERROR);
}

TEST(FuseMessage, LinkRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t inodeNumber = g_distribution(g_generator);
    const fuse_ino_t newParentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewLinkRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber,
                                                               newParentInodeNumber, ENTRY_NAME);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(inodeNumber) + sizeof(newParentInodeNumber) + ENTRY_NAME_LENGTH +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto linkRequest = fuseMessage.GetRequest<Protocol::Fuse::LinkRequest>();
    ASSERT_EQ(linkRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(linkRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(linkRequest.NewParentInodeNumber(), newParentInodeNumber);
    ASSERT_EQ(std::string(linkRequest.NewName()), std::string(ENTRY_NAME));
}

TEST(FuseMessage, LinkResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewLinkResponse(
        requestHandler, Protocol::Fuse::LinkResponse::ResponseType::SUCCESS, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::MakeDirectoryResponse::ResponseType) +
                                                           sizeof(FuseEntryParams) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto linkResponse = fuseMessage.GetResponse<Protocol::Fuse::LinkResponse>();
    ASSERT_EQ(linkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(linkResponse.Type(), Protocol::Fuse::LinkResponse::ResponseType::SUCCESS);
    CompareFuseEntryParams(linkResponse.Params()->ToParams(), params);
}

TEST(FuseMessage, LinkResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewLinkResponse(
        requestHandler, Protocol::Fuse::LinkResponse::ResponseType::FAIL, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::MakeDirectoryResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto linkResponse = fuseMessage.GetResponse<Protocol::Fuse::LinkResponse>();
    ASSERT_EQ(linkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(linkResponse.Type(), Protocol::Fuse::LinkResponse::ResponseType::FAIL);
    ASSERT_EQ(linkResponse.Error(), ERROR);
}

TEST(FuseMessage, SymLinkRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t parentInodeNumber = g_distribution(g_generator);
    auto request = Protocol::Fuse::FuseMessage::NewSymLinkRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                  parentInodeNumber, ENTRY_NAME, ENTRY_NAME_TWO);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(parentInodeNumber) + sizeof(AuxDataSize) + sizeof(AuxDataSize) +
        ENTRY_NAME_LENGTH + ENTRY_NAME_TWO_LENGTH + FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
        TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SYMLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto symLinkRequest = fuseMessage.GetRequest<Protocol::Fuse::SymLinkRequest>();
    ASSERT_EQ(symLinkRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(symLinkRequest.ParentInodeNumber(), parentInodeNumber);
    const auto linkSize = symLinkRequest.LinkSize();
    ASSERT_EQ(linkSize, ENTRY_NAME_LENGTH);
    ASSERT_EQ(std::string(symLinkRequest.Link(linkSize)), std::string(ENTRY_NAME));
    const auto nameSize = symLinkRequest.NameSize(linkSize);
    ASSERT_EQ(nameSize, ENTRY_NAME_TWO_LENGTH);
    ASSERT_EQ(std::string(symLinkRequest.Name(linkSize, nameSize)), std::string(ENTRY_NAME_TWO));
}

TEST(FuseMessage, SymLinkResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewSymLinkResponse(
        requestHandler, Protocol::Fuse::SymLinkResponse::ResponseType::SUCCESS, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SymLinkResponse::ResponseType) +
                                                           sizeof(FuseEntryParams) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SYMLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto symLinkResponse = fuseMessage.GetResponse<Protocol::Fuse::SymLinkResponse>();
    ASSERT_EQ(symLinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(symLinkResponse.Type(), Protocol::Fuse::SymLinkResponse::ResponseType::SUCCESS);
    CompareFuseEntryParams(symLinkResponse.Params()->ToParams(), params);
}

TEST(FuseMessage, SymLinkResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto params = GetEntryParams();
    auto response = Protocol::Fuse::FuseMessage::NewSymLinkResponse(
        requestHandler, Protocol::Fuse::SymLinkResponse::ResponseType::FAIL, ERROR, &params);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::SymLinkResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::SYMLINK);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto symLinkResponse = fuseMessage.GetResponse<Protocol::Fuse::SymLinkResponse>();
    ASSERT_EQ(symLinkResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(symLinkResponse.Type(), Protocol::Fuse::SymLinkResponse::ResponseType::FAIL);
    ASSERT_EQ(symLinkResponse.Error(), ERROR);
}

TEST(FuseMessage, RenameRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t parentInodeNumber = g_distribution(g_generator);
    const fuse_ino_t newParentInodeNumber = g_distribution(g_generator);
    const auto flags = static_cast<std::uint32_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewRenameRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                 parentInodeNumber, newParentInodeNumber, flags,
                                                                 ENTRY_NAME, ENTRY_NAME_TWO);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(parentInodeNumber) + sizeof(newParentInodeNumber) + sizeof(flags) +
        sizeof(AuxDataSize) + sizeof(AuxDataSize) + ENTRY_NAME_LENGTH + ENTRY_NAME_TWO_LENGTH +
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RENAME);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto renameRequest = fuseMessage.GetRequest<Protocol::Fuse::RenameRequest>();
    ASSERT_EQ(renameRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(renameRequest.ParentInodeNumber(), parentInodeNumber);
    ASSERT_EQ(renameRequest.NewParentInodeNumber(), newParentInodeNumber);
    const auto nameSize = renameRequest.NameSize();
    ASSERT_EQ(nameSize, ENTRY_NAME_LENGTH);
    ASSERT_EQ(std::string(renameRequest.Name(nameSize)), std::string(ENTRY_NAME));
    const auto newNameSize = renameRequest.NewNameSize(nameSize);
    ASSERT_EQ(newNameSize, ENTRY_NAME_TWO_LENGTH);
    ASSERT_EQ(std::string(renameRequest.NewName(nameSize, newNameSize)), std::string(ENTRY_NAME_TWO));
}

TEST(FuseMessage, RenameResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewRenameResponse(
        requestHandler, Protocol::Fuse::RenameResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::RenameResponse::ResponseType) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RENAME);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto renameResponse = fuseMessage.GetResponse<Protocol::Fuse::RenameResponse>();
    ASSERT_EQ(renameResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(renameResponse.Type(), Protocol::Fuse::RenameResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, RenameResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewRenameResponse(
        requestHandler, Protocol::Fuse::RenameResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::RenameResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::RENAME);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto renameResponse = fuseMessage.GetResponse<Protocol::Fuse::RenameResponse>();
    ASSERT_EQ(renameResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(renameResponse.Type(), Protocol::Fuse::RenameResponse::ResponseType::FAIL);
    ASSERT_EQ(renameResponse.Error(), ERROR);
}

TEST(FuseMessage, FAllocateRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t inodeNumber = g_distribution(g_generator);
    const auto mode = static_cast<int>(g_distribution(g_generator));
    const auto offset = static_cast<off_t>(g_distribution(g_generator));
    const auto length = static_cast<off_t>(g_distribution(g_generator));
    const auto fileHandle = static_cast<uint64_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewFAllocateRequest(TOKEN, TARGET_SOURCE, requestHandler, inodeNumber,
                                                                    mode, offset, length, fileHandle);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(inodeNumber) + sizeof(mode) + sizeof(offset) + sizeof(length) +
        sizeof(fileHandle) + FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH + TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FALLOCATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto fAllocateRequest = fuseMessage.GetRequest<Protocol::Fuse::FAllocateRequest>();
    ASSERT_EQ(fAllocateRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(fAllocateRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(fAllocateRequest.Mode(), mode);
    ASSERT_EQ(fAllocateRequest.Offset(), offset);
    ASSERT_EQ(fAllocateRequest.Length(), length);
    ASSERT_EQ(fAllocateRequest.FileHandle(), fileHandle);
}

TEST(FuseMessage, FAllocateResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewFAllocateResponse(
        requestHandler, Protocol::Fuse::FAllocateResponse::ResponseType::SUCCESS, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::FAllocateResponse::ResponseType) +
                                                           FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FALLOCATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto fAllocateResponse = fuseMessage.GetResponse<Protocol::Fuse::FAllocateResponse>();
    ASSERT_EQ(fAllocateResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(fAllocateResponse.Type(), Protocol::Fuse::FAllocateResponse::ResponseType::SUCCESS);
}

TEST(FuseMessage, FAllocateResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    auto response = Protocol::Fuse::FuseMessage::NewFAllocateResponse(
        requestHandler, Protocol::Fuse::FAllocateResponse::ResponseType::FAIL, ERROR);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) +
                                                           sizeof(Protocol::Fuse::FAllocateResponse::ResponseType) +
                                                           sizeof(ERROR) + FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::FALLOCATE);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto fAllocateResponse = fuseMessage.GetResponse<Protocol::Fuse::FAllocateResponse>();
    ASSERT_EQ(fAllocateResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(fAllocateResponse.Type(), Protocol::Fuse::FAllocateResponse::ResponseType::FAIL);
    ASSERT_EQ(fAllocateResponse.Error(), ERROR);
}

TEST(FuseMessage, ListExtendedAttributesRequest)
{
    const auto requestHandler = g_distribution(g_generator);
    const fuse_ino_t inodeNumber = g_distribution(g_generator);
    const auto size = static_cast<std::size_t>(g_distribution(g_generator));
    auto request = Protocol::Fuse::FuseMessage::NewListExtendedAttributesRequest(TOKEN, TARGET_SOURCE, requestHandler,
                                                                                 inodeNumber, size);

    decltype(MessageHeader::messageLength) messageLength = sizeof(std::size_t) + sizeof(inodeNumber) + sizeof(size) +
                                                           FUSE_MESSAGE_REQUEST_HEADER_SIZE + TOKEN_LENGTH +
                                                           TARGET_SOURCE_LENGTH;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(request->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    request->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(request);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LISTXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::REQUEST);
    ASSERT_EQ(fuseMessage.Token(), TOKEN);
    ASSERT_EQ(fuseMessage.TargetSource(), TARGET_SOURCE);
    ASSERT_EQ(fuseMessage.Data(), request);

    auto listExtendedAttributesRequest = fuseMessage.GetRequest<Protocol::Fuse::ListExtendedAttributesRequest>();
    ASSERT_EQ(listExtendedAttributesRequest.RequestHandler(), requestHandler);
    ASSERT_EQ(listExtendedAttributesRequest.InodeNumber(), inodeNumber);
    ASSERT_EQ(listExtendedAttributesRequest.Size(), size);
}

TEST(FuseMessage, ListExtendedAttributesResponseSuccess)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto size = static_cast<std::size_t>(g_distribution(g_generator));
    const std::string someData = "ljshdgjshf0892vyt82yb[9t8'32u'ot9vu30t9v2unt=v23 [toia983u";
    auto response = Protocol::Fuse::FuseMessage::NewListExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::SUCCESS, ERROR, size, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ListExtendedAttributesResponse::ResponseType) + someData.size() +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LISTXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto listExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::ListExtendedAttributesResponse>();
    ASSERT_EQ(listExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(listExtendedAttributesResponse.Type(),
              Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::SUCCESS);
    ASSERT_EQ(std::string(listExtendedAttributesResponse.Data().data(), listExtendedAttributesResponse.Data().size()),
              someData);
}

TEST(FuseMessage, ListExtendedAttributesResponseSize)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto size = static_cast<std::size_t>(g_distribution(g_generator));
    const std::string someData = "ljshdgjshf0892vyt82yb[9t8'32u'ot9vu30t9v2unt=v23 [toia983u";
    auto response = Protocol::Fuse::FuseMessage::NewListExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::SIZE, ERROR, size, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ListExtendedAttributesResponse::ResponseType) + sizeof(size) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LISTXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto listExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::ListExtendedAttributesResponse>();
    ASSERT_EQ(listExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(listExtendedAttributesResponse.Type(),
              Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::SIZE);
    ASSERT_EQ(listExtendedAttributesResponse.Size(), size);
}

TEST(FuseMessage, ListExtendedAttributesResponseFail)
{
    const auto requestHandler = g_distribution(g_generator);
    const auto size = static_cast<std::size_t>(g_distribution(g_generator));
    const std::string someData = "ljshdgjshf0892vyt82yb[9t8'32u'ot9vu30t9v2unt=v23 [toia983u";
    auto response = Protocol::Fuse::FuseMessage::NewListExtendedAttributesResponse(
        requestHandler, Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::FAIL, ERROR, size, someData);

    decltype(MessageHeader::messageLength) messageLength =
        sizeof(std::size_t) + sizeof(Protocol::Fuse::ListExtendedAttributesResponse::ResponseType) + sizeof(ERROR) +
        FUSE_MESSAGE_RESPONSE_HEADER_SIZE;

    auto *messageHeader = reinterpret_cast<MessageHeader *>(response->Data().data());
    ASSERT_EQ(messageHeader->type, MessageType::FUSE);
    ASSERT_EQ(messageHeader->messageLength, messageLength);

    response->RemoveHeader();
    Protocol::Fuse::FuseMessage fuseMessage(response);
    ASSERT_EQ(fuseMessage.Class(), Protocol::Fuse::FuseMessageClass::LISTXATTR);
    ASSERT_EQ(fuseMessage.Type(), Protocol::Fuse::FuseMessageType::RESPONSE);
    ASSERT_TRUE(fuseMessage.Token().empty());
    ASSERT_TRUE(fuseMessage.TargetSource().empty());
    ASSERT_EQ(fuseMessage.Data(), response);

    auto listExtendedAttributesResponse = fuseMessage.GetResponse<Protocol::Fuse::ListExtendedAttributesResponse>();
    ASSERT_EQ(listExtendedAttributesResponse.RequestHandler(), requestHandler);
    ASSERT_EQ(listExtendedAttributesResponse.Type(),
              Protocol::Fuse::ListExtendedAttributesResponse::ResponseType::FAIL);
    ASSERT_EQ(listExtendedAttributesResponse.Error(), ERROR);
}
}
