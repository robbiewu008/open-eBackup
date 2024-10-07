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
#ifndef DP_FUSE_MESSAGE_H
#define DP_FUSE_MESSAGE_H
// std
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// Vendors
#include <boost/utility/string_view.hpp>

#include <protocol/Utilities.h>
#include <protocol/Message.h>
#include <protocol/fuse/Flush.h>
#include <protocol/fuse/Lookup.h>
#include <protocol/fuse/GetAttributes.h>
#include <protocol/fuse/GetExtendedAttributes.h>
#include <protocol/fuse/ReadDirectory.h>
#include <protocol/fuse/ReadLink.h>
#include <protocol/fuse/Create.h>
#include <protocol/fuse/SetAttribute.h>
#include <protocol/fuse/Release.h>
#include <protocol/fuse/Open.h>
#include <protocol/fuse/WriteBuffer.h>
#include <protocol/fuse/Read.h>
#include <protocol/fuse/Unlink.h>
#include <protocol/fuse/RemoveDirectory.h>
#include <protocol/fuse/MakeDirectory.h>
#include <protocol/fuse/Link.h>
#include <protocol/fuse/SymLink.h>
#include <protocol/fuse/Rename.h>
#include <protocol/fuse/FAllocate.h>
#include <protocol/fuse/ListExtendedAttributes.h>

namespace DataMove {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11
namespace Fuse { // Ugly, but nested namespaces are not present in C++11

enum class FuseMessageClass : std::uint16_t {
    INIT,    // Not used
    DESTROY, // Not used
    LOOKUP,
    FORGET,  // Not used
    GETATTR,
    SETATTR,
    READLINK,
    MKNOD,   // Not used
    MKDIR,
    UNLINK,
    RMDIR,
    SYMLINK,
    RENAME,
    LINK,
    OPEN,
    READ,
    WRITE, // Not used
    FLUSH, // Not used
    RELEASE,
    FSYNC, // Not used
    OPENDIR, // Not used
    READDIR,
    RELEASEDIR, // Not used
    FSYNCDIR, // Not used
    STATFS, // Not used
    SETXATTR, // Not used
    GETXATTR,
    LISTXATTR,
    REMOVEXATTR, // Not used
    ACCESS, // Not used
    CREATE,
    GETLK, // Not used
    SETLK, // Not used
    BMAP, // Not used
    IOCTL, // Not used
    POLL, // Not used
    WRITE_BUF,
    RETRIEVE_REPLY, // Not used
    FORGET_MULTI, // Not used
    FLOCK, // Not used
    FALLOCATE,
    READDIRPLUS,
    COPY_FILE_RANGE, // Not used
    LSEEK, // Not used
};

enum class FuseMessageType : std::uint8_t {
    REQUEST,
    RESPONSE
};

class FuseMessage final {
public:
    FuseMessage() = delete;
    explicit FuseMessage(const std::shared_ptr<Module::Protocol::Message>& message)
        : m_messageClass(*reinterpret_cast<const FuseMessageClass*>(message->Data().data())),
        m_messageType(*reinterpret_cast<const FuseMessageType*>(message->Data().data() + sizeof(FuseMessageClass))),
        m_data(message)
    { }

    FuseMessageClass Class() const { return m_messageClass; }
    FuseMessageType Type() const { return m_messageType; };
    std::string Token() const;
    std::string TargetSource() const;

    const std::shared_ptr<Module::Protocol::Message>& Data() const { return m_data; }

    /*!
     * \brief Updates token in a request.
     *
     * \param request Request to update.
     * \param newToken New token that is going to be copied into the provided request. `newToken.size()` should be
     *                 the same as the old one, otherwise some data could be overwritten if 'newToken.size()' is bigger.
     */
    static void UpdateTokenInRequest(const std::shared_ptr<Module::Protocol::Message>& request,
     const std::string& newToken);

    static std::shared_ptr<Module::Protocol::Message> NewLookupRequest(const std::string& token,
                                                     const std::string& targetSource,
                                                     std::uint64_t requestHandler,
                                                     fuse_ino_t parentInodeNumber,
                                                     const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewLookupResponse(std::uint64_t requestHandler,
                                                      LookupResponse::ResponseType type,
                                                      const fuse_entry_param* params);

    static std::shared_ptr<Module::Protocol::Message> NewGetAttributesRequest(const std::string& token,
                                                            const std::string& targetSource,
                                                            std::uint64_t requestHandler,
                                                            fuse_ino_t inodeNumber);
    static std::shared_ptr<Module::Protocol::Message> NewGetAttributesResponse(std::uint64_t requestHandler,
                                                             GetAttributesResponse::ResponseType type,
                                                             int error,
                                                             const struct stat* attributes);
    static std::shared_ptr<Module::Protocol::Message> NewGetExtendedAttributesRequest(const std::string& token,
                                                                    const std::string& targetSource,
                                                                    std::uint64_t requestHandler,
                                                                    fuse_ino_t inodeNumber,
                                                                    std::uint64_t size,
                                                                    const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewGetExtendedAttributesResponse(std::uint64_t requestHandler,
                                                                     GetExtendedAttributesResponse::ResponseType type,
                                                                     std::uint64_t size,
                                                                     int error,
                                                                     const std::string* data);
    static std::shared_ptr<Module::Protocol::Message> NewReadDirectoryRequest(const std::string& token,
                                                            const std::string& targetSource,
                                                            std::uint64_t requestHandler,
                                                            fuse_ino_t inodeNumber,
                                                            std::uint64_t size,
                                                            std::uint64_t offset,
                                                            bool plus);
    static std::shared_ptr<Module::Protocol::Message> NewReadDirectoryResponse(std::uint64_t requestHandler,
                                                             ReadDirectoryResponse::ResponseType type,
                                                             bool plus,
                                                             int error,
                                                             std::uint64_t size,
                                                             std::uint64_t offset,
                                                             boost::string_view data);
    static std::shared_ptr<Module::Protocol::Message> NewReadLinkRequest(const std::string& token,
                                                       const std::string& targetSource,
                                                       std::uint64_t requestHandler,
                                                       fuse_ino_t inodeNumber);
    static std::shared_ptr<Module::Protocol::Message> NewReadLinkResponse(std::uint64_t requestHandler,
                                                        ReadLinkResponse::ResponseType type,
                                                        int error,
                                                        const std::string* link);
    static std::shared_ptr<Module::Protocol::Message> NewCreateRequest(const std::string& token,
                                                     const std::string& targetSource,
                                                     std::uint64_t requestHandler,
                                                     fuse_ino_t parentInodeNumber,
                                                     std::uint32_t mode,
                                                     const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewCreateResponse(std::uint64_t requestHandler,
                                                      CreateResponse::ResponseType type,
                                                      int error,
                                                      const fuse_entry_param* params,
                                                      uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewSetAttributeRequest(const std::string& token,
                                                           const std::string& targetSource,
                                                           std::uint64_t requestHandler,
                                                           fuse_ino_t inodeNumber,
                                                           int fieldsToSet,
                                                           const struct stat* attributes);
    static std::shared_ptr<Module::Protocol::Message> NewSetAttributeResponse(std::uint64_t requestHandler,
                                                            SetAttributeResponse::ResponseType type,
                                                            int error,
                                                            const struct stat* attributes);
    static std::shared_ptr<Module::Protocol::Message> NewReleaseRequest(const std::string& token,
                                                      const std::string& targetSource,
                                                      std::uint64_t requestHandler,
                                                      uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewReleaseResponse(std::uint64_t requestHandler,
                                                        ReleaseResponse::ResponseType type,
                                                       int error);
    static std::shared_ptr<Module::Protocol::Message> NewFlushRequest(const std::string& token,
                                                      const std::string& targetSource,
                                                      std::uint64_t requestHandler,
                                                      uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewFlushResponse(std::uint64_t requestHandler,
                                                       FlushResponse::ResponseType type,
                                                       int error);
    static std::shared_ptr<Module::Protocol::Message> NewOpenRequest(const std::string& token,
                                                   const std::string& targetSource,
                                                   std::uint64_t requestHandler,
                                                   fuse_ino_t inodeNumber,
                                                   int flags);
    static std::shared_ptr<Module::Protocol::Message> NewOpenResponse(std::uint64_t requestHandler,
                                                    OpenResponse::ResponseType type,
                                                    int error,
                                                    uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewWriteBufferRequest(const std::string& token,
                                                          const std::string& targetSource,
                                                          std::uint64_t requestHandler,
                                                          fuse_ino_t inodeNumber,
                                                          uint64_t fileHandle,
                                                          off_t offset,
                                                          boost::string_view data);
    static std::shared_ptr<Module::Protocol::Message> NewWriteBufferResponse(std::uint64_t requestHandler,
                                                           WriteBufferResponse::ResponseType type,
                                                           int error,
                                                           std::uint64_t size);
    static std::shared_ptr<Module::Protocol::Message> NewReadRequest(const std::string& token,
                                                   const std::string& targetSource,
                                                   std::uint64_t requestHandler,
                                                   std::uint64_t size,
                                                   off_t offset,
                                                   uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewReadResponse(std::uint64_t requestHandler,
                                                    ReadResponse::ResponseType type,
                                                    int error,
                                                    const std::string* data,
                                                    std::uint64_t dataLength);
    static std::shared_ptr<Module::Protocol::Message> NewUnlinkRequest(const std::string& token,
                                                     const std::string& targetSource,
                                                     std::uint64_t requestHandler,
                                                     fuse_ino_t parentInodeNumber,
                                                     const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewUnlinkResponse(std::uint64_t requestHandler,
                                                      UnlinkResponse::ResponseType type,
                                                      int error);
    static std::shared_ptr<Module::Protocol::Message> NewRemoveDirectoryRequest(const std::string& token,
                                                              const std::string& targetSource,
                                                              std::uint64_t requestHandler,
                                                              fuse_ino_t parentInodeNumber,
                                                              const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewRemoveDirectoryResponse(std::uint64_t requestHandler,
                                                               RemoveDirectoryResponse::ResponseType type,
                                                               int error);
    static std::shared_ptr<Module::Protocol::Message> NewMakeDirectoryRequest(const std::string& token,
                                                            const std::string& targetSource,
                                                            std::uint64_t requestHandler,
                                                            fuse_ino_t parentInodeNumber,
                                                            mode_t mode,
                                                            const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewMakeDirectoryResponse(std::uint64_t requestHandler,
                                                             MakeDirectoryResponse::ResponseType type,
                                                             int error,
                                                             const fuse_entry_param* params);
    static std::shared_ptr<Module::Protocol::Message> NewLinkRequest(const std::string& token,
                                                   const std::string& targetSource,
                                                   std::uint64_t requestHandler,
                                                   fuse_ino_t inodeNumber,
                                                   fuse_ino_t newParentInodeNumber,
                                                   const char* newName);
    static std::shared_ptr<Module::Protocol::Message> NewLinkResponse(std::uint64_t requestHandler,
                                                    LinkResponse::ResponseType type,
                                                    int error,
                                                    const fuse_entry_param* params);
    static std::shared_ptr<Module::Protocol::Message> NewSymLinkRequest(const std::string& token,
                                                      const std::string& targetSource,
                                                      std::uint64_t requestHandler,
                                                      fuse_ino_t parentInodeNumber,
                                                      const char* link,
                                                      const char* name);
    static std::shared_ptr<Module::Protocol::Message> NewSymLinkResponse(std::uint64_t requestHandler,
                                                       SymLinkResponse::ResponseType type,
                                                       int error,
                                                       const fuse_entry_param* params);
    static std::shared_ptr<Module::Protocol::Message> NewRenameRequest(const std::string& token,
                                                     const std::string& targetSource,
                                                     std::uint64_t requestHandler,
                                                     fuse_ino_t parentInodeNumber,
                                                     fuse_ino_t newParentInodeNumber,
                                                     std::uint32_t flags,
                                                     const char* name,
                                                     const char* newName);
    static std::shared_ptr<Module::Protocol::Message> NewRenameResponse(std::uint64_t requestHandler,
                                                      RenameResponse::ResponseType type,
                                                      int error);
    static std::shared_ptr<Module::Protocol::Message> NewFAllocateRequest(const std::string& token,
                                                        const std::string& targetSource,
                                                        std::uint64_t requestHandler,
                                                        fuse_ino_t inodeNumber,
                                                        int mode,
                                                        off_t offset,
                                                        off_t length,
                                                        uint64_t fileHandle);
    static std::shared_ptr<Module::Protocol::Message> NewFAllocateResponse(std::uint64_t requestHandler,
                                                         FAllocateResponse::ResponseType type,
                                                         int error);
    static std::shared_ptr<Module::Protocol::Message> NewListExtendedAttributesRequest(const std::string& token,
                                                                     const std::string& targetSource,
                                                                     std::uint64_t requestHandler,
                                                                     fuse_ino_t inodeNumber,
                                                                     std::uint64_t size);
    static std::shared_ptr<Module::Protocol::Message> NewListExtendedAttributesResponse(std::uint64_t requestHandler,
                                                                      ListExtendedAttributesResponse::ResponseType type,
                                                                      int error,
                                                                      std::uint64_t size,
                                                                      boost::string_view data);

    template <typename MessageType>
    MessageType GetRequest() const;
    template <typename MessageType>
    MessageType GetResponse() const;

private:
    constexpr static std::uint64_t TOKEN_SIZE_OFFSET = sizeof(FuseMessageClass) + sizeof(FuseMessageType);
    constexpr static std::uint64_t TOKEN_OFFSET = TOKEN_SIZE_OFFSET + sizeof(AuxDataSize);

    static std::shared_ptr<Module::Protocol::Message> AllocateRequestBuffer(FuseMessageClass messageClass,
                                                          const std::string& token,
                                                          const std::string& targetSource,
                                                          std::uint64_t additionalDataSize);
    static std::shared_ptr<Module::Protocol::Message> AllocateResponseBuffer(FuseMessageClass messageClass,
                                                           std::uint64_t additionalDataSize);

    const FuseMessageClass m_messageClass;
    const FuseMessageType m_messageType;
    std::shared_ptr<Module::Protocol::Message> m_data;
    const std::string m_emptyString;
};

template <typename MessageType>
MessageType FuseMessage::GetRequest() const
{
    const auto tokenSize = *reinterpret_cast<AuxDataSize*>(m_data->Data().data() + TOKEN_SIZE_OFFSET);
    const auto targetSourceSizeOffset = TOKEN_OFFSET + tokenSize;
    const auto targetSourceSize = *reinterpret_cast<AuxDataSize*>(m_data->Data().data() + targetSourceSizeOffset);
    const auto dataOffset = targetSourceSizeOffset + sizeof(AuxDataSize) + targetSourceSize;
    return MessageType({ m_data->Data().data() + dataOffset, m_data->Data().size() - dataOffset });
}

template <typename MessageType>
MessageType FuseMessage::GetResponse() const
{
    return MessageType({ m_data->Data().data() + TOKEN_SIZE_OFFSET, m_data->Data().size() - TOKEN_SIZE_OFFSET });
}

}
}
}

#endif