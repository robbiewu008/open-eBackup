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
#ifndef DP_ERROR_RESPONSE_H
#define DP_ERROR_RESPONSE_H
// std
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// Vendors
#include <boost/utility/string_view.hpp>

#include <protocol/Message.h>
#include <protocol/MessageHeader.h>

namespace Module {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11

/*!
 * \brief
 * \details Data is stored like this: { [ErrorType] [Message] }
 */
class ErrorResponse {
public:
    enum class ErrorType : std::uint8_t {
        RESPONSE_RECEIVED,
        INVALID_TOKEN,
        INVALID_CREDENTIALS,
        INVALID_TARGET_SOURCE
    };

    ErrorResponse() = delete;
    explicit ErrorResponse(const std::shared_ptr<Protocol::Message>& message);

    ErrorType Type() const;
    std::size_t RequestHandler() const;
    boost::string_view Message() const;

    const std::shared_ptr<Protocol::Message>& Data() const;

    static std::shared_ptr<Protocol::Message> NewErrorResponse(ErrorType type,
                                                               std::size_t requestHandler,
                                                               const std::string& message,
                                                               const std::shared_ptr<MessageHeader>& reqMsgHeader);

private:
    const ErrorType m_type;
    const std::size_t m_requestHandler;
    std::shared_ptr<Protocol::Message> m_data;
};

}
}

#endif