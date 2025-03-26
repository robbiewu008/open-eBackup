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

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLookupRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataLength = sizeof(param.m_requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);

    auto message = AllocateRequestBuffer(FuseMessageClass::LOOKUP,
                                         param,
                                         additionalDataLength + nameLength + sizeof(NULL_CHAR));
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
    DBGLOG("Lookup mesage size is %d.", sizeof(message));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLookupResponse(std::size_t requestHandler,
    LookupResponse::ResponseType type, const fuse_entry_param* params, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case LookupResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::LOOKUP, additionalDataLength +
                sizeof(paramsWrapper), msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case LookupResponse::ResponseType::NO_SUCH_FILE_OR_DIRECTORY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LOOKUP, additionalDataLength, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetAttributesRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber)
{
    constexpr static std::size_t additionalDataLength = sizeof(param.m_requestHandler) + sizeof(inodeNumber);

    auto message = AllocateRequestBuffer(FuseMessageClass::GETATTR, param, additionalDataLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetAttributesResponse(std::size_t requestHandler,
    GetAttributesResponse::ResponseType type, int error, const struct stat* attributes,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalDataLength = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case GetAttributesResponse::ResponseType::SUCCESS: {
            const Attributes attributesWrapper(*attributes);
            auto message = AllocateResponseBuffer(FuseMessageClass::GETATTR,
                                                  additionalDataLength + sizeof(attributesWrapper),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
            return message;
        }
        case GetAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETATTR,
                                                  additionalDataLength + sizeof(error),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetExtendedAttributesRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, std::size_t size, const char* name)
{
    constexpr static std::size_t sizeOfConstantAdditionalData = sizeof(param.m_requestHandler) + sizeof(inodeNumber)
        + sizeof(size);
    const std::size_t nameLength = std::strlen(name);
    std::size_t additionalDataLength = sizeOfConstantAdditionalData + nameLength;

    auto message = AllocateRequestBuffer(FuseMessageClass::GETXATTR, param, additionalDataLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(name, nameLength);

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewGetExtendedAttributesResponse(std::size_t requestHandler,
    GetExtendedAttributesResponse::ResponseType type, std::size_t size, int error, const std::string* data,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case GetExtendedAttributesResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR,
                                                  additionalDataSize + data->size() + sizeof(NULL_CHAR),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data->c_str(), data->size());
            message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
            return message;
        }
        case GetExtendedAttributesResponse::ResponseType::NO_AVAILABLE_DATA:
        case GetExtendedAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR,
                                                  additionalDataSize + sizeof(error),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
        case GetExtendedAttributesResponse::ResponseType::SIZE: {
            auto message = AllocateResponseBuffer(FuseMessageClass::GETXATTR,
                                                  additionalDataSize + sizeof(size),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadDirectoryRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, std::size_t size, std::size_t offset, bool plus)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
    sizeof(size) + sizeof(offset) + sizeof(plus);
    auto message = AllocateRequestBuffer(plus ? FuseMessageClass::READDIRPLUS : FuseMessageClass::READDIR,
                                         param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(reinterpret_cast<char*>(&plus), sizeof(plus));

    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadDirectoryResponse(std::size_t requestHandler,
    ReadDirectoryResponse::ResponseType type, bool plus, int error, std::size_t size, std::size_t offset,
    boost::string_view data, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalDataSizeSuccess = sizeof(requestHandler) + sizeof(type) + sizeof(plus) +
                                                             sizeof(size) + sizeof(offset);
    constexpr static std::size_t additionalDataSizeFail = sizeof(requestHandler) + sizeof(type) + sizeof(plus) +
                                                          sizeof(error);
    switch (type) {
        case ReadDirectoryResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(plus ? FuseMessageClass::READDIRPLUS : FuseMessageClass::READDIR,
                                                  additionalDataSizeSuccess + data.size(),
                                                  msgHeader);
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
                                                  additionalDataSizeFail,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&plus), sizeof(plus));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadLinkRequest(ReqMsgParam& param, fuse_ino_t inodeNumber)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber);
    auto message = AllocateRequestBuffer(FuseMessageClass::READLINK, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadLinkResponse(std::size_t requestHandler,
    ReadLinkResponse::ResponseType type, int error, const std::string* link,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalDataSize = sizeof(requestHandler) + sizeof(type);
    switch (type) {
        case ReadLinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READLINK,
                                                  additionalDataSize + link->size() + sizeof(NULL_CHAR),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(link->c_str(), link->size());
            message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
            return message;
        }
        case ReadLinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READLINK,
                                                  additionalDataSize + sizeof(error),
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewCreateRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, uint32_t mode, const char* name, struct UserGroup ug)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(parentInodeNumber) +
                                                      sizeof(mode) + sizeof(NULL_CHAR) + sizeof(UserGroup);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::CREATE, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(reinterpret_cast<const char*>(&ug), sizeof(ug));
    message->AddData(name, nameLength);
    message->AddData(&NULL_CHAR, sizeof(NULL_CHAR));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewCreateResponse(std::size_t requestHandler,
    CreateResponse::ResponseType type, int error, const fuse_entry_param* params, uint64_t fileHandle,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(FuseEntryParams) + sizeof(fileHandle);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case CreateResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::CREATE,
                                                  additionalSuccessDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
            return message;
        }
        case CreateResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::CREATE,
                                                  additionalFailDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSetAttributeRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, int fieldsToSet, const struct stat* attributes)
{
    const Attributes attributesWrapper(*attributes);
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(fieldsToSet) + sizeof(attributesWrapper);
    auto message = AllocateRequestBuffer(FuseMessageClass::SETATTR, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&fieldsToSet), sizeof(fieldsToSet));
    message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSetAttributeResponse(std::size_t requestHandler,
    SetAttributeResponse::ResponseType type, int error, const struct stat* attributes,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case SetAttributeResponse::ResponseType::SUCCESS: {
            const Attributes attributesWrapper(*attributes);
            constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(attributesWrapper);
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR,
                                                  additionalSuccessDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&attributesWrapper), sizeof(attributesWrapper));
            return message;
        }
        case SetAttributeResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR,
                                                  additionalFailDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
        case SetAttributeResponse::ResponseType::CTIME_ON_DELETED_ENTRY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SETATTR,
                                                  additionalFailDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReleaseRequest(ReqMsgParam& param,
    uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::RELEASE, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReleaseResponse(std::size_t requestHandler,
    ReleaseResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = additionalSuccessDataSize + sizeof(error);
    switch (type) {
        case ReleaseResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RELEASE,
                                                  additionalSuccessDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case ReleaseResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RELEASE,
                                                  additionalFailDataSize,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFlushRequest(ReqMsgParam& param,
    uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::FLUSH, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFlushResponse(std::size_t requestHandler,
    FlushResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = additionalSuccessDataSize + sizeof(error);
    switch (type) {
        case FlushResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FLUSH, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case FlushResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FLUSH, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewOpenRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, int flags)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
                                                      + sizeof(flags);
    auto message = AllocateRequestBuffer(FuseMessageClass::OPEN, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&flags), sizeof(flags));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewOpenResponse(std::size_t requestHandler,
    OpenResponse::ResponseType type, int error, uint64_t fileHandle, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             + sizeof(fileHandle);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case OpenResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::OPEN, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
            return message;
        }
        case OpenResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::OPEN, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewWriteBufferRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, uint64_t fileHandle, off_t offset, boost::string_view data)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(fileHandle) + sizeof(offset);
    const auto dataLength = data.size();
    auto message = AllocateRequestBuffer(FuseMessageClass::WRITE_BUF, param,
                                         additionalDataSize + dataLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(data.data(), dataLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewWriteBufferResponse(std::size_t requestHandler,
    WriteBufferResponse::ResponseType type, int error, std::uint64_t size,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(size);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case WriteBufferResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::WRITE_BUF, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
        case WriteBufferResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::WRITE_BUF, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadRequest(ReqMsgParam& param, fuse_ino_t inodeNumber,
    std::uint64_t size, off_t offset, uint64_t fileHandle)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(size) +sizeof(offset) + sizeof(fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::READ, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    message->AddData(reinterpret_cast<char*>(&offset), sizeof(offset));
    message->AddData(reinterpret_cast<char*>(&fileHandle), sizeof(fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewReadResponse(std::size_t requestHandler,
    ReadResponse::ResponseType type, int error, const std::string* data, std::size_t dataLength,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case ReadResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READ,
                                                  additionalSuccessDataSize + dataLength,
                                                  msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data->c_str(), dataLength);
            return message;
        }
        case ReadResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::READ, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewUnlinkRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::UNLINK, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewUnlinkResponse(std::size_t requestHandler,
    UnlinkResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case UnlinkResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::UNLINK, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case UnlinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::UNLINK, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRemoveDirectoryRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, const char* name)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(parentInodeNumber);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::RMDIR, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRemoveDirectoryResponse(std::size_t requestHandler,
    RemoveDirectoryResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case RemoveDirectoryResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RMDIR, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case RemoveDirectoryResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RMDIR, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMkNodRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, mode_t mode, const char* name, const struct UserGroup& ug)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) +
        sizeof(parentInodeNumber) + sizeof(mode) + sizeof(UserGroup);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::MKNOD, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(reinterpret_cast<const char*>(&ug), sizeof(ug));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMkNodResponse(std::size_t requestHandler,
    MkNodResponse::ResponseType type, int error, const fuse_entry_param* params,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    switch (type) {
        case MkNodResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                sizeof(FuseEntryParams);
            auto message = AllocateResponseBuffer(FuseMessageClass::MKNOD, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case MkNodResponse::ResponseType::FAIL: {
            constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
            auto message = AllocateResponseBuffer(FuseMessageClass::MKNOD, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMakeDirectoryRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, mode_t mode, const char* name, struct UserGroup ug)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) +
        sizeof(parentInodeNumber) + sizeof(mode) + sizeof(UserGroup);
    const auto nameLength = std::strlen(name);
    auto message = AllocateRequestBuffer(FuseMessageClass::MKDIR, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&mode), sizeof(mode));
    message->AddData(reinterpret_cast<const char*>(&ug), sizeof(ug));
    message->AddData(name, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewMakeDirectoryResponse(std::size_t requestHandler,
    MakeDirectoryResponse::ResponseType type, int error, const fuse_entry_param* params,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(FuseEntryParams);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case MakeDirectoryResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::MKDIR, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case MakeDirectoryResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::MKDIR, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLinkRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, fuse_ino_t newParentInodeNumber, const char* newName)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(inodeNumber) +
                                                      sizeof(newParentInodeNumber);
    const auto nameLength = std::strlen(newName);
    auto message = AllocateRequestBuffer(FuseMessageClass::LINK, param,
                                         additionalDataSize + nameLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&newParentInodeNumber), sizeof(newParentInodeNumber));
    message->AddData(newName, nameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewLinkResponse(std::size_t requestHandler,
    LinkResponse::ResponseType type, int error, const fuse_entry_param* params,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(FuseEntryParams);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case LinkResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::LINK, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case LinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LINK, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSymLinkRequest(ReqMsgParam& param,
    fuse_ino_t parentInodeNumber, const char* link, const char* name, const struct UserGroup& ug)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) + sizeof(parentInodeNumber) +
                                                      sizeof(AuxDataSize) + sizeof(AuxDataSize) + sizeof(UserGroup);
    const auto linkLength = static_cast<AuxDataSize>(std::strlen(link));
    const auto nameLength = static_cast<AuxDataSize>(std::strlen(name));
    auto message = AllocateRequestBuffer(FuseMessageClass::SYMLINK, param,
                                         additionalDataSize + nameLength + linkLength);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&parentInodeNumber), sizeof(parentInodeNumber));
    message->AddData(reinterpret_cast<const char*>(&linkLength), sizeof(linkLength));
    message->AddData(link, linkLength);
    message->AddData(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    message->AddData(name, nameLength);
    message->AddData(reinterpret_cast<const char*>(&ug), sizeof(ug));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewSymLinkResponse(std::size_t requestHandler,
    SymLinkResponse::ResponseType type, int error, const fuse_entry_param* params,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type) +
                                                             sizeof(FuseEntryParams);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case SymLinkResponse::ResponseType::SUCCESS: {
            const FuseEntryParams paramsWrapper(*params);
            auto message = AllocateResponseBuffer(FuseMessageClass::SYMLINK, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<const char*>(&paramsWrapper), sizeof(paramsWrapper));
            return message;
        }
        case SymLinkResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::SYMLINK, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRenameRequest(RenameMsgParam& renameParam)
{
    constexpr static std::size_t additionalDataSize = sizeof(renameParam.m_reqMsgParam.m_requestHandler) +
                                                      sizeof(renameParam.m_parentInodeNumber) +
                                                      sizeof(renameParam.m_newParentInodeNumber) +
                                                      sizeof(AuxDataSize) +
                                                      sizeof(AuxDataSize) +
                                                      sizeof(renameParam.m_flags);
    const auto nameLength = static_cast<AuxDataSize>(std::strlen(renameParam.m_oldName));
    const auto newNameLength = static_cast<AuxDataSize>(std::strlen(renameParam.m_newName));
    auto message = AllocateRequestBuffer(FuseMessageClass::RENAME, renameParam.m_reqMsgParam,
                                         additionalDataSize + nameLength + newNameLength);
    message->AddData(reinterpret_cast<char*>(&renameParam.m_reqMsgParam.m_requestHandler),
        sizeof(renameParam.m_reqMsgParam.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&renameParam.m_parentInodeNumber),
        sizeof(renameParam.m_parentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&renameParam.m_newParentInodeNumber),
        sizeof(renameParam.m_newParentInodeNumber));
    message->AddData(reinterpret_cast<char*>(&renameParam.m_flags), sizeof(renameParam.m_flags));
    message->AddData(reinterpret_cast<const char*>(&nameLength), sizeof(nameLength));
    message->AddData(renameParam.m_oldName, nameLength);
    message->AddData(reinterpret_cast<const char*>(&newNameLength), sizeof(newNameLength));
    message->AddData(renameParam.m_newName, newNameLength);
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewRenameResponse(std::size_t requestHandler,
    RenameResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case RenameResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RENAME, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case RenameResponse::ResponseType::FAIL:
        case RenameResponse::ResponseType::NO_PARENT_DIRECTORY:
        case RenameResponse::ResponseType::NO_NEW_PARENT_DIRECTORY: {
            auto message = AllocateResponseBuffer(FuseMessageClass::RENAME, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFAllocateRequest(AllocateReqParam& allocParam)
{
    constexpr static std::size_t additionalDataSize = sizeof(allocParam.m_reqMsgParam.m_requestHandler) +
        sizeof(allocParam.m_inodeNumber) + sizeof(allocParam.m_mode) + sizeof(allocParam.m_offset) +
        sizeof(allocParam.m_length) + sizeof(allocParam.m_fileHandle);
    auto message = AllocateRequestBuffer(FuseMessageClass::FALLOCATE, allocParam.m_reqMsgParam, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&allocParam.m_reqMsgParam.m_requestHandler),
        sizeof(allocParam.m_reqMsgParam.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&allocParam.m_inodeNumber), sizeof(allocParam.m_inodeNumber));
    message->AddData(reinterpret_cast<char*>(&allocParam.m_mode), sizeof(allocParam.m_mode));
    message->AddData(reinterpret_cast<char*>(&allocParam.m_offset), sizeof(allocParam.m_offset));
    message->AddData(reinterpret_cast<char*>(&allocParam.m_length), sizeof(allocParam.m_length));
    message->AddData(reinterpret_cast<char*>(&allocParam.m_fileHandle), sizeof(allocParam.m_fileHandle));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewFAllocateResponse(std::size_t requestHandler,
    FAllocateResponse::ResponseType type, int error, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    switch (type) {
        case FAllocateResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FALLOCATE, additionalSuccessDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            return message;
        }
        case FAllocateResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::FALLOCATE, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewListExtendedAttributesRequest(ReqMsgParam& param,
    fuse_ino_t inodeNumber, std::size_t size)
{
    constexpr static std::size_t additionalDataSize = sizeof(param.m_requestHandler) +
        sizeof(inodeNumber) + sizeof(size);
    auto message = AllocateRequestBuffer(FuseMessageClass::LISTXATTR, param, additionalDataSize);
    message->AddData(reinterpret_cast<char*>(&(param.m_requestHandler)), sizeof(param.m_requestHandler));
    message->AddData(reinterpret_cast<char*>(&inodeNumber), sizeof(inodeNumber));
    message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
    return message;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::NewListExtendedAttributesResponse(std::size_t requestHandler,
    ListExtendedAttributesResponse::ResponseType type, int error, std::size_t size, boost::string_view data,
    const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static std::size_t additionalSuccessDataSize = sizeof(requestHandler) + sizeof(type);
    constexpr static std::size_t additionalSizeDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(size);
    constexpr static std::size_t additionalFailDataSize = sizeof(requestHandler) + sizeof(type) + sizeof(error);
    const std::size_t dataLength = data.size();
    switch (type) {
        case ListExtendedAttributesResponse::ResponseType::SUCCESS: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalSuccessDataSize + dataLength,
                msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(data.data(), dataLength);
            return message;
        }
        case ListExtendedAttributesResponse::ResponseType::SIZE: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalSizeDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&size), sizeof(size));
            return message;
        }
        case ListExtendedAttributesResponse::ResponseType::FAIL: {
            auto message = AllocateResponseBuffer(FuseMessageClass::LISTXATTR, additionalFailDataSize, msgHeader);
            message->AddData(reinterpret_cast<char*>(&requestHandler), sizeof(requestHandler));
            message->AddData(reinterpret_cast<char*>(&type), sizeof(type));
            message->AddData(reinterpret_cast<char*>(&error), sizeof(error));
            return message;
        }
    }

    return nullptr;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::AllocateRequestBuffer(FuseMessageClass messageClass,
    ReqMsgParam& param, std::size_t additionalDataSize)
{
    constexpr static auto request = FuseMessageType::REQUEST;
    const AuxDataSize tokenSize = param.m_token.size();
    const AuxDataSize targetSourceSize = param.m_targetSource.size();
    const decltype(Module::Protocol::MessageHeader::messageLength) messageLength =
        FUSE_MESSAGE_REQUEST_HEADER_SIZE + tokenSize + targetSourceSize + additionalDataSize;
    DBGLOG("AllocateRequestBuffer size is %d.", messageLength);

    auto fuseMessage = std::make_shared<Module::Protocol::Message>();
    fuseMessage->Reserve(sizeof(Module::Protocol::MessageHeader) + messageLength);

    // Copy message header
    fuseMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    fuseMessage->AddData(reinterpret_cast<const char*>(&PROTOCOL_VERSION_V1), sizeof(PROTOCOL_VERSION_V1));
    fuseMessage->AddData(reinterpret_cast<const char*>(&param.m_sequecNumber), sizeof(param.m_sequecNumber));
    fuseMessage->AddData(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength));

    // Copy fuse message data
    fuseMessage->AddData(reinterpret_cast<char*>(&messageClass), sizeof(messageClass));
    fuseMessage->AddData(reinterpret_cast<const char*>(&request), sizeof(request));
    fuseMessage->AddData(reinterpret_cast<const char*>(&tokenSize), sizeof(tokenSize));
    fuseMessage->AddData(param.m_token.data(), tokenSize);
    fuseMessage->AddData(reinterpret_cast<const char*>(&targetSourceSize), sizeof(targetSourceSize));
    fuseMessage->AddData(param.m_targetSource.data(), targetSourceSize);
    return fuseMessage;
}

std::shared_ptr<Module::Protocol::Message> FuseMessage::AllocateResponseBuffer(FuseMessageClass messageClass,
    std::size_t additionalDataSize, const std::shared_ptr<MessageHeader>& msgHeader)
{
    constexpr static auto response = FuseMessageType::RESPONSE;
    const decltype(Module::Protocol::MessageHeader::messageLength) messageLength = FUSE_MESSAGE_RESPONSE_HEADER_SIZE
        + additionalDataSize;

    auto fuseMessage = std::make_shared<Module::Protocol::Message>();
    fuseMessage->Reserve(sizeof(Module::Protocol::MessageHeader) + messageLength);

    // Copy message header
    fuseMessage->AddData(reinterpret_cast<const char*>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    fuseMessage->AddData(reinterpret_cast<const char*>(&PROTOCOL_VERSION_V1), sizeof(PROTOCOL_VERSION_V1));
    fuseMessage->AddData(reinterpret_cast<const char*>(&msgHeader->sequenceNumber), sizeof(msgHeader->sequenceNumber));
    fuseMessage->AddData(reinterpret_cast<const char*>(&messageLength), sizeof(messageLength));

    // Copy fuse message data
    fuseMessage->AddData(reinterpret_cast<char*>(&messageClass), sizeof(messageClass));
    fuseMessage->AddData(reinterpret_cast<const char*>(&response), sizeof(response));

    return fuseMessage;
}

}
}
}
