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
#include <server/DataPlaneServer.h>
#include <curl_http/CurlHttpClient.h>
#include <functional>
#include <fstream>

#include <session/Session.h>
#include "log/Log.h"

namespace Module {

DataPlaneServer::DataPlaneServer(boost::asio::io_service& ioService,
    const ServerObjInfo& serverObj)
    : m_ioService(ioService),
    m_acceptor(ioService, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverObj.port)),
    m_sslContext(boost::asio::ssl::context::tlsv12_server)
{
    m_authInfo = serverObj.authInfo;
    if (!m_authInfo.certFilePath.empty()) {
        m_isSsl = true;
    } else {
        m_isSsl = false;
    }
}

bool DataPlaneServer::Init()
{
    if (m_isSsl) {
        DBGLOG("Creating server with SSL");
        m_sslContext.set_options(boost::asio::ssl::context::default_workarounds |
                                 boost::asio::ssl::context::no_sslv2 |
                                 boost::asio::ssl::context::single_dh_use);
        try {
            m_sslContext.load_verify_file(m_authInfo.caFilePath);
        } catch (std::exception& exception) {
            ERRLOG("Failed to load ca %s file", m_authInfo.caFilePath.c_str());
            return false;
        }
        try {
            m_sslContext.use_certificate_file(m_authInfo.certFilePath, boost::asio::ssl::context::pem);
        } catch (std::exception& exception) {
            ERRLOG("Failed to load certificate %s file", m_authInfo.certFilePath.c_str());
            return false;
        }
        if (!LoadEncryPassword()) {
            ERRLOG("Load Password failed.");
            return false;
        }
        try {
            m_sslContext.use_rsa_private_key_file(m_authInfo.keyFilePath, boost::asio::ssl::context::pem);
        } catch (std::exception& exception) {
            ERRLOG("Failed to load internal.pem file");
            return false;
        }
    } else {
        DBGLOG("Creating server with no SSL");
    }
    HandleAccept();
    return true;
}

bool DataPlaneServer::LoadEncryPassword()
{
    std::ifstream stream;
    stream.open(m_authInfo.encryPwdFilePath, std::ios::in);
    if (!stream.is_open()) {
        ERRLOG("Open %s failed.", m_authInfo.encryPwdFilePath.c_str());
        return false;
    }
    std::string encryStr;
    std::getline(stream, encryStr);
    stream.close();
    if (encryStr.empty()) {
        ERRLOG("CipherText is empty string.");
        return false;
    }
    std::unique_ptr<Module::CurlKmcManagerInterface> kmcInstance = std::make_unique<Module::CurlKmcManagerInterface>();
    if (kmcInstance.get() == nullptr) {
        ERRLOG("kmcInstance is null.");
        return false;
    }
    if (!kmcInstance->InitKmc(m_authInfo.kmcStoreFilePath, m_authInfo.kmcBackupStoreFilePath)) {
        ERRLOG("Init kmc failed.");
        return false;
    }
    std::string decryPwd;
    if (kmcInstance->Decrypt(decryPwd, encryStr) != 0) {
        ERRLOG("Decrypt failed.");
        return false;
    }
    m_sslContext.set_password_callback(std::bind(&DataPlaneServer::GetPassword, decryPwd));
    return true;
}

std::string DataPlaneServer::GetPassword(const std::string& password)
{
    return password;
}

bool DataPlaneServer::IsSsl() const
{
    return m_isSsl;
}

void DataPlaneServer::Stop()
{
    m_ioService.stop();
}

void DataPlaneServer::HandleAccept()
{
    DBGLOG("Waiting for a new connection");
    m_acceptor.async_accept(
        [this](const boost::system::error_code& error, boost::asio::ip::tcp::socket socket) {
            if (error) {
                HandleAccept();
                return;
            }
            std::shared_ptr<Session> session;
            if (IsSsl()) {
                INFOLOG("SSL is on.");
                session = Session::NewServer(std::move(socket), m_sslContext, m_ioService);
                if (!session->Handshake()) {
                    ERRLOG("Failed to perform handshake");
                    return;
                }
            } else {
                session = Session::NewServer(std::move(socket), m_ioService);
                INFOLOG("SSL is off.");
            }
            session->TurnDelayOff();
            INFOLOG("[Session: %zu] New incoming session established",
                reinterpret_cast<std::size_t>(session.get()));
            OnNewSessionAccepted(session);
            session->ReceiveAsyncMessage(
                [this, session](std::shared_ptr<Protocol::Message> message,
                                std::shared_ptr<Protocol::MessageHeader> msgHeader, bool networkErrorFlag) {
                    if (networkErrorFlag) {
                        ERRLOG("Failed to receive message.");
                        return;
                    }
                    HandleReceivedMessage(session, std::move(message), msgHeader);
                });
            HandleAccept();
        });
}

uint16_t DataPlaneServer::GetLocalServerPort()
{
    return m_acceptor.local_endpoint().port();
}

}