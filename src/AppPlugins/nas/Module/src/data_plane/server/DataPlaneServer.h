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
#ifndef DP_SERVER_H
#define DP_SERVER_H
// std
#include <memory>

// Vendors
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>

#include <session/Session.h>
#include <protocol/MessageHeader.h>
#include <protocol/Message.h>
#include <common/DataPlaneCommon.h>

namespace Module {
class DataPlaneServer {
public:
    explicit DataPlaneServer(boost::asio::io_service& ioService,
                    const ServerObjInfo& serverObj);
    virtual ~DataPlaneServer() = default;

    bool IsSsl() const;
    bool Init();
    bool LoadEncryPassword();

    static std::string GetPassword(const std::string& password);
    virtual void Stop();
    uint16_t GetLocalServerPort();

protected:
    virtual void HandleReceivedMessage(std::shared_ptr<Session> session,
                                       std::shared_ptr<Protocol::Message> message,
                                       Protocol::MessageType messageType) = 0;

    virtual void OnNewSessionAccepted(std::shared_ptr<Session> session) = 0;

private:
    void HandleAccept();
    boost::asio::ip::tcp::acceptor m_acceptor;
    boost::asio::io_service& m_ioService;
    bool m_isSsl;
    boost::asio::ssl::context m_sslContext;
    AuthObjInfo m_authInfo;
};

}

#endif