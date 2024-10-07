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
#include <protocol/fuse/FuseMessage.h>

// std
#include <cstring>
#include <protocol/MessageHeader.h>
#include "log/Log.h"

namespace DataMove {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11
namespace Fuse { // Ugly, but nested namespaces are not present in C++11

namespace {
constexpr char NULL_CHAR = '\0';

constexpr Module::Protocol::MessageType MESSAGE_TYPE = Module::Protocol::MessageType::FUSE;

constexpr std::size_t FUSE_MESSAGE_REQUEST_HEADER_SIZE =
    sizeof(FuseMessageClass) + // Size of a fuse message class
    sizeof(FuseMessageType) +  // Size of a fuse message type
    sizeof(AuxDataSize) +      // Size of a size of a token
    sizeof(AuxDataSize);       // Size of a size of a target source

constexpr std::size_t FUSE_MESSAGE_RESPONSE_HEADER_SIZE =
    sizeof(FuseMessageClass) + // Size of a fuse message class
    sizeof(FuseMessageType);   // Size of a fuse message type
}

std::string FuseMessage::Token() const
{
    switch (m_messageType) {
        case FuseMessageType::REQUEST:
            return { m_data->Data().data() + TOKEN_OFFSET,
                     *reinterpret_cast<AuxDataSize*>(m_data->Data().data() + TOKEN_SIZE_OFFSET) };
        case FuseMessageType::RESPONSE:
            return m_emptyString;
    }

    return m_emptyString;
}

std::string FuseMessage::TargetSource() const
{
    switch (m_messageType) {
        case FuseMessageType::REQUEST: {
            const auto tokenSize = *reinterpret_cast<AuxDataSize*>(m_data->Data().data() + TOKEN_SIZE_OFFSET);
            const auto targetSourceSizeOffset = TOKEN_OFFSET + tokenSize;
            return { m_data->Data().data() + targetSourceSizeOffset + sizeof(AuxDataSize),
                      *reinterpret_cast<AuxDataSize*>(m_data->Data().data() + targetSourceSizeOffset) };
        }
        case FuseMessageType::RESPONSE:
            return m_emptyString;
    }

    return m_emptyString;
}

void FuseMessage::UpdateTokenInRequest(const std::shared_ptr<Module::Protocol::Message>& request,
    const std::string& newToken)
{
    constexpr static std::size_t tokenOffsetOfRequestWithHeader = sizeof(Module::Protocol::MessageHeader)
     + TOKEN_OFFSET;
    auto& requestData = request->Data();
    for (std::size_t i = 0; i < newToken.size(); ++i) {
        requestData[tokenOffsetOfRequestWithHeader + i] = newToken[i];
    }
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLookupRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);

    auto message = AllocateRequestBuffer(FuseMessageClass::LOOKUP,
                                         token,
                                         targetSource,
                                         additionalDataLength + nameLength + sizeof(NULL_CHAR));
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
    INFOLOG("Lookup mesage size is %d.", sizeof(message));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLookupResponse(std::size_t requestHandler,
    LookupResponse::ResponseType type, const fuse_entry_param* params)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case LookupResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::LOOKUP, additionalDataLength +
                sizeof(paramsWrapper));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case LookupResponse::ResponseType::NO_SUCH_FILE_OR_DIRECTORY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LOOKUP, additionalDataLength);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetAttributesRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(inodeNumber);

    auto message = AllocateRequestBuffer(FuseMessageClass::GETATTR, token, targetSource, additionalDataLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetAttributesResponse(std::size_t requestHandler,
    GetAttributesResponse::ResponseType type, int error, const struct stat* attributes)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case GetAttributesResponse::ResponseType::SUCCESS: {
            const Attributes attributesWrapper(*attributes);
            auto message = AllocateResponseBuffer(FuseMessageClass::GETATTR,
                                                  additionalDataLength + sizeof(attributesWrapper));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
            return message;
        }
        case GetAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETATTR, additionalDataLength + sizeof(error));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetExtendedAttributesRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, std::size_t size,
    const char* name)
{
    constexpr static std::size_t sizeOfConstantAdditionalData = sizeof(requestHandler) + sizeof(inodeNumber)
        + sizeof(size);
    const std::size_t nameLength = std::strlen(name);
    std::size_t additionalDataLength = sizeOfConstantAdditionalData + nameLength;

    auto message = AllocateRequestBuffer(FuseMessageClass::GETXATTR, token, targetSource, additionalDataLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(name, nameLength);

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetExtendedAttributesResponse(std::size_t requestHandler,
    GetExtendedAttributesResponse::ResponseType type, std::size_t size, int error, const std::string* data)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case GetExtendedAttributesResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR,
                                                  additionalDataSize + data->size() + sizeof(NULL_CHAR));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data->c_str(), data->size());
            message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
            return message;
        }
        case GetExtendedAttributesResponse::ResponseType::NO_AVAILABLE_DATA:
        case GetExtendedAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR, additionalDataSize + sizeof(error));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
        case GetExtendedAttributesResponse::ResponseType::SIZE: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR, additionalDataSize + sizeof(size));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadDirectoryRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, std::size_t size,
    std::size_t offset, bool plus)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) + sizeof(size) +
                                                      sizeof(offset) + sizeof(plus);
    auto message = AllocateRequestBuffer(plus ? FuseMessageClass::READDIRPLUS : FuseMessageClass::READDIR,
                                         token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(reinterpret_cast<char*>(&plus), sizeof(plus));

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadDirectoryResponse(std::size_t requestHandler,
    ReadDirectoryResponse::ResponseType type, bool plus, int error, std::size_t size, std::size_t offset,
    boost::string_view data)
{
    constexpr static std::size_t additionalDataSizeSuccess = sizeof(requestHandler) + sizeof(type) + sizeof(plus) +
                                                             sizeof(size) + sizeof(offset);
    constexpr static std::size_t additionalDataSizeFail = sizeof(requestHandler) + sizeof(type) + sizeof(plus) +
                                                          sizeof(error);
    switch (type) {
        case ReadDirectoryResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(plus ? FuseMessageClass::READDIRPLUS : FuseMessageClass::READDIR,
                                                  additionalDataSizeSuccess + data.size());
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&plus), sizeof(plus));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
            message->AddData(data.data(), data.size());
            return message;
        }
        case ReadDirectoryResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(plus ? FuseMessageClass::READDIRPLUS : FuseMessageClass::READDIR,
                                                  additionalDataSizeFail);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&plus), sizeof(plus));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadLinkRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber);
    auto message = AllocateRequestBuffer(FuseMessageClass::READLINK, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadLinkResponse(std::size_t requestHandler,
    ReadLinkResponse::ResponseType type, int error, const std::string* link)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case ReadLinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READLINK,
                                                  additionalDataSize + link->size() + sizeof(NULL_CHAR));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(link->c_str(), link->size());
            message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
            return message;
        }
        case ReadLinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READLINK, additionalDataSize + sizeof(error));
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewCreateRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, uint32_t mode,
    const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber) +
                                                      sizeof(mode) + sizeof(NULL_CHAR);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::CREATE, token, targetSource,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(name, nameLength);
    message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewCreateResponse(std::size_t requestHandler,
    CreateResponse::ResponseType type, int error, const fuse_entry_param* params, uint64_t fileHandle)
{
    const FuseEntryParams paramsWrapper(*params);
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(paramsWrapper) + sizeof(fileHandle);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case CreateResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::CREATE,
                                                  additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
            return message;
        }
        case CreateResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::CREATE, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSetAttributeRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, int fieldsToSet,
    const struct stat* attributes)
{
    const Attributes attributesWrapper(*attributes);
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(fieldsToSet) + sizeof(attributesWrapper);
    auto message = AllocateRequestBuffer(FuseMessageClass::SETATTR, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&fieldsToSet), sizeof(fieldsToSet));
    message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSetAttributeResponse(std::size_t requestHandler,
    SetAttributeResponse::ResponseType type, int error, const struct stat* attributes)
{
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case SetAttributeResponse::ResponseType::SUCCESS: {
            const Attributes attributesWrapper(*attributes);
            constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(attributesWrapper);
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR,
                                                  additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
            return message;
        }
        case SetAttributeResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
        case SetAttributeResponse::ResponseType::CTIME_ON_DELETED_ENTRY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReleaseRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::RELEASE, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReleaseResponse(std::size_t requestHandler,
    ReleaseResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = additionalSuccessDataSize + sizeof(error);
    switch (type) {
        case ReleaseResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RELEASE, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case ReleaseResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RELEASE, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFlushRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::FLUSH, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFlushResponse(std::size_t requestHandler,
    FlushResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = additionalSuccessDataSize + sizeof(error);
    switch (type) {
        case FlushResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FLUSH, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case FlushResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FLUSH, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewOpenRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, int flags)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) +
                                                      + sizeof(flags);
    auto message = AllocateRequestBuffer(FuseMessageClass::OPEN, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&flags), sizeof(flags));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewOpenResponse(std::size_t requestHandler,
    OpenResponse::ResponseType type, int error, uint64_t fileHandle)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             + sizeof(fileHandle);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case OpenResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::OPEN, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
            return message;
        }
        case OpenResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::OPEN, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewWriteBufferRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, uint64_t fileHandle,
    off_t offset, boost::string_view data)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(fileHandle) + sizeof(offset);
    const auto dataLength = data.size();
    auto message = AllocateRequestBuffer(FuseMessageClass::WRITE_BUF, token, targetSource,
                                         additionalDataSize + dataLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(data.data(), dataLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewWriteBufferResponse(std::size_t requestHandler,
    WriteBufferResponse::ResponseType type, int error, std::uint64_t size)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(size);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case WriteBufferResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::WRITE_BUF, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
        case WriteBufferResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::WRITE_BUF, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, std::uint64_t size, off_t offset, uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(size) +
                                                      sizeof(offset) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::READ, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadResponse(std::size_t requestHandler,
    ReadResponse::ResponseType type, int error, const std::string* data, std::size_t dataLength)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case ReadResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READ,
                                                  additionalSuccessDataSize + dataLength);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data->c_str(), dataLength);
            return message;
        }
        case ReadResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READ, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewUnlinkRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::UNLINK, token, targetSource,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewUnlinkResponse(std::size_t requestHandler,
    UnlinkResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case UnlinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::UNLINK, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case UnlinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::UNLINK, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRemoveDirectoryRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::RMDIR, token, targetSource,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRemoveDirectoryResponse(std::size_t requestHandler,
    RemoveDirectoryResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case RemoveDirectoryResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RMDIR, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case RemoveDirectoryResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RMDIR, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMakeDirectoryRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, mode_t mode,
    const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber) + sizeof(mode);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::MKDIR, token, targetSource,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMakeDirectoryResponse(std::size_t requestHandler,
    MakeDirectoryResponse::ResponseType type, int error, const fuse_entry_param* params)
{
    const FuseEntryParams paramsWrapper(*params);
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(paramsWrapper);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case MakeDirectoryResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::MKDIR, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case MakeDirectoryResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::MKDIR, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLinkRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber,
    fuse_ino_t newParentInodeNumber, const char* newName)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(newParentInodeNumber);
    const auto nameLength = std::strlen(newName);
    auto message = AllocateRequestBuffer(FuseMessageClass::LINK, token, targetSource,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&newParentInodeNumber), sizeof(newParentInodeNumber));
    message->AddData(newName, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLinkResponse(std::size_t requestHandler,
    LinkResponse::ResponseType type, int error, const fuse_entry_param* params)
{
    const FuseEntryParams paramsWrapper(*params);
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(paramsWrapper);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case LinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LINK, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case LinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LINK, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSymLinkRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber, const char* link,
    const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber) +
                                                      sizeof(AuxDataSize) + sizeof(AuxDataSize);
    const auto linkLength = static_cast<AuxDataSize>(std::strlen(link));
    const auto nameLength = static_cast<AuxDataSize>(std::strlen(name));
    auto message = AllocateRequestBuffer(FuseMessageClass::SYMLINK, token, targetSource,
                                         additionalDataSize + nameLength + linkLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<const char*>(&linkLength), sizeof(linkLength));
    message->AddData(link, linkLength);
    message->AddData(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSymLinkResponse(std::size_t requestHandler,
    SymLinkResponse::ResponseType type, int error, const fuse_entry_param* params)
{
    const FuseEntryParams paramsWrapper(*params);
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(paramsWrapper);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case SymLinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SYMLINK, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case SymLinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SYMLINK, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRenameRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t parentInodeNumber,
    fuse_ino_t newParentInodeNumber, std::uint32_t flags, const char* name, const char* newName)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(parentInodeNumber) +
                                                      sizeof(newParentInodeNumber) +  sizeof(AuxDataSize) +
                                                      sizeof(AuxDataSize) + sizeof(flags);
    const auto nameLength = static_cast<AuxDataSize>(std::strlen(name));
    const auto newNameLength = static_cast<AuxDataSize>(std::strlen(newName));
    auto message = AllocateRequestBuffer(FuseMessageClass::RENAME, token, targetSource,
                                         additionalDataSize + nameLength + newNameLength);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&newParentInodeNumber), sizeof(newParentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&flags), sizeof(flags));
    message->AddData(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    message->AddData(name, nameLength);
    message->AddData(reinterpret_cast<const char*>(&newNameLength), sizeof(newNameLength));
    message->AddData(newName, newNameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRenameResponse(std::size_t requestHandler,
    RenameResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case RenameResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RENAME, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case RenameResponse::ResponseType::FAIL:
        case RenameResponse::ResponseType::NO_PARENT_DIRECTORY:
        case RenameResponse::ResponseType::NO_NEW_PARENT_DIRECTORY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RENAME, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFAllocateRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, int mode, off_t offset,
    off_t length, uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) + sizeof(mode) +
                                                      sizeof(offset) +  sizeof(length) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::FALLOCATE, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(reinterpret_cast<char*>(&length), sizeof(length));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFAllocateResponse(std::size_t requestHandler,
    FAllocateResponse::ResponseType type, int error)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case FAllocateResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FALLOCATE, additionalSuccessDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case FAllocateResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FALLOCATE, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewListExtendedAttributesRequest(const std::string& token,
    const std::string& targetSource, std::size_t requestHandler, fuse_ino_t inodeNumber, std::size_t size)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(inodeNumber) + sizeof(size);
    auto message = AllocateRequestBuffer(FuseMessageClass::LISTXATTR, token, targetSource, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewListExtendedAttributesResponse(std::size_t requestHandler,
    ListExtendedAttributesResponse::ResponseType type, int error, std::size_t size, boost::string_view data)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalSizeDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(size);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    const std::size_t dataLength = data.size();
    switch (type) {
        case ListExtendedAttributesResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalSuccessDataSize + dataLength);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data.data(), dataLength);
            return message;
        }
        case ListExtendedAttributesResponse::ResponseType::SIZE: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalSizeDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
        case ListExtendedAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalFailDataSize);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::AllocateRequestBuffer(FuseMessageClass messageClass,
    const std::string& token, const std::string& targetSource, std::size_t additionalDataSize)
{
    constexpr static auto request = FuseMessageType::REQUEST;
    const AuxDataSize tokenSize = token.size();
    const AuxDataSize targetSourceSize = targetSource.size();
    const decltype(Module::Protocol::MessageHeader::messageLength) messageLength =
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + tokenSize + targetSourceSize + additionalDataSize;
    INFOLOG("AllocateRequestBuffer size is %d.", messageLength);

    auto fuseMessage = std::make_shared<Module::Protocol::Message>();
    fuseMessage->Reserve(sizeof(Module::Protocol::MessageHeader) + messageLength);

    // Copy message header
    fuseMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    fuseMessage->AddData(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength));

    // Copy fuse message data
    fuseMessage->AddData(reinterpret_cast<char*>(&messageClass), sizeof(messageClass));
    fuseMessage->AddData(reinterpret_cast<const char*>(&request), sizeof(request));
    fuseMessage->AddData(reinterpret_cast<const char*>(&tokenSize), sizeof(tokenSize));
    fuseMessage->AddData(token.data(), tokenSize);
    fuseMessage->AddData(reinterpret_cast<const char*>(&targetSourceSize), sizeof(targetSourceSize));
    fuseMessage->AddData(targetSource.data(), targetSourceSize);

    return fuseMessage;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::AllocateResponseBuffer(FuseMessageClass messageClass,
    std::size_t additionalDataSize)
{
    constexpr static auto response = FuseMessageType::RESPONSE;
    const decltype(Module::Protocol::MessageHeader::messageLength) messageLength = FUSE_MESSAGE_RESPONSE_HEADER_SIZE
        + additionalDataSize;

    auto fuseMessage = std::make_shared<Module::Protocol::Message>();
    fuseMessage->Reserve(sizeof(Module::Protocol::MessageHeader) + messageLength);

    // Copy message header
    fuseMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    fuseMessage->AddData(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength));

    // Copy fuse message data
    fuseMessage->AddData(reinterpret_cast<char*>(&messageClass), sizeof(messageClass));
    fuseMessage->AddData(reinterpret_cast<const char*>(&response), sizeof(response));

    return fuseMessage;
}

}
}
}
