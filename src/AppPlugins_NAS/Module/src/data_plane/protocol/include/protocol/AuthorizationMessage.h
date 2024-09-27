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
#ifndef DP_AUTH_MESSAGE_H
#define DP_AUTH_MESSAGE_H

// std
#include <cstdint>
#include <string>
#include <vector>
#include <memory>

// Vendors
#include <boost/utility/string_view.hpp>


#include <protocol/Message.h>

namespace Module {
namespace Protocol { // Ugly, but nested namespaces are not present in C++11

enum class AuthorizationMessageType : std::uint8_t {
    REQUEST_LOGIN,
    RESPONSE_LOGIN,
    REQUEST_KEEP_ALIVE,
    RESPONSE_KEEP_ALIVE
};

/*!
 * \brief
 *
 * \details Data is stored like this: { [AuthorizationMessageType] [Other data] }.\n
 * Other data could store different things depending on the AuthorizationMessageType:\n
 * - RESPONSE_LOGIN:      [Token]                                               \n
 * - REQUEST_KEEP_ALIVE:  [Token]                                               \n
 * - RESPONSE_KEEP_ALIVE: [Token]
 */
class AuthorizationMessage {
public:
    using AuthorizationMessageSize = std::uint16_t;

    AuthorizationMessage() = delete;
    explicit AuthorizationMessage(const std::shared_ptr<Message>& message);

    AuthorizationMessageType Type() const;
    boost::string_view Login() const;
    boost::string_view Token() const;

    const std::shared_ptr<Message>& Data() const;
    static std::shared_ptr<Message> NewLoginRequest();
    static std::shared_ptr<Message> NewLoginResponse(const std::string& token);
    static std::shared_ptr<Message> NewKeepAliveRequest(const std::string& token);
    static std::shared_ptr<Message> NewKeepAliveResponse(const std::string& token);

private:
    static std::shared_ptr<Message> AllocateBuffer(AuthorizationMessageType type,
                                                   AuthorizationMessageSize dataLength);

    const AuthorizationMessageType m_type;
    std::shared_ptr<Message> m_data;
    const std::string m_emptyString;
};

}
}

#endif