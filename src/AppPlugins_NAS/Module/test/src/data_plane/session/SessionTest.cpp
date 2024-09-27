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
#include <session/Session.h>
#include <server/DataPlaneServer.h>

using Module::Protocol::Message;
using Module::Protocol::MessageHeader;
using Module::Protocol::MessageType;
using Module::ServerObjInfo;

namespace {
constexpr auto SERVER_ADDRESS = "127.0.0.1";
constexpr std::uint16_t PORT = 1234;
constexpr auto PORT_AS_STRING = "1234";
constexpr auto CERTIFICATE_PATH = "../resources/server.pem";
constexpr auto RSA_PATH = "../resources/server.pem";
constexpr auto CA_FILE_PATH = "../resources/server.pem";
constexpr auto KEY_FILE_PATH = "../resources/server.pem";
constexpr auto KMCSTORE_PATH = "../resources/server.pem";
constexpr auto KMCBACKUPSTORE_PATH = "../resources/server.pem";
constexpr auto ENCRYPWD_PATH = "../resources/server.pem";
constexpr auto ENCRYPWD_EMPTY_PATH = "../resources/server_empty.pem";
constexpr auto DH_PATH = "../resources/dh.pem";
constexpr auto TEST_DATA_MESSAGE = "kjshdhsbgjhc9823yr-92y2uhf2/fjh2o8r\\khbvuohv3uhvjkxbvyupvbev";
constexpr MessageType MESSAGE_TYPE = MessageType::FUSE;
const decltype(MessageHeader::messageLength) TEST_DATA_MESSAGE_LENGTH = std::strlen(TEST_DATA_MESSAGE);

std::shared_ptr<Message> CreateMessage(const std::string& messageData)
{
    const decltype(MessageHeader::messageLength) messageLength = messageData.size();

    auto message = std::make_shared<Message>();
    message->Reserve(sizeof(MessageHeader) + messageLength);
    message->AddData(reinterpret_cast<const char *>(&MESSAGE_TYPE), sizeof(MESSAGE_TYPE));
    message->AddData(reinterpret_cast<const char *>(&messageLength), sizeof(messageLength));
    message->AddData(messageData.data(), messageLength);

    return message;
}

class TestServer : public Module::DataPlaneServer {
public:
    TestServer(boost::asio::io_service& ioService, const ServerObjInfo& serverObj)
        : Module::DataPlaneServer(ioService, serverObj)
    {
    }

    std::size_t NumberOfAcceptedSessions() const
    {
        return m_acceptedSessions.size();
    }

    void StartDeadlineTimerOnFirstSession() const
    {
        if (m_acceptedSessions.empty()) {
            return;
        }

        m_acceptedSessions.front()->StartDeadlineTimer(boost::posix_time::seconds(1));
    }

    void StopDeadlineTimerOnFirstSession() const
    {
        if (m_acceptedSessions.empty()) {
            return;
        }

        m_acceptedSessions.front()->StopDeadlineTimer();
    }

protected:
    void HandleReceivedMessage(std::shared_ptr<Module::Session> session, std::shared_ptr<Message> message,
                               MessageType messageType) final
    {
        auto incomingMessage = std::string(message->Data().data(), message->Data().size());
        ASSERT_EQ(messageType, MessageType::FUSE);
        ASSERT_EQ(incomingMessage, std::string(TEST_DATA_MESSAGE));

        auto response = CreateMessage(incomingMessage);
        session->SendAsyncMessage(response, [this, session](const boost::system::error_code&, std::size_t) {
            session->ReceiveAsyncMessage([this, session](std::shared_ptr<Message> message, MessageType messageType) {
                HandleReceivedMessage(session, std::move(message), messageType);
            });
        });
    }

    void OnNewSessionAccepted(std::shared_ptr<Module::Session> session) final
    {
        session->SetOnDisconnectCallback(std::bind(&TestServer::OnSessionDisconnect, this, std::placeholders::_1));
        m_acceptedSessions.push_back(session);
    }

private:
    void OnSessionDisconnect(Module::Session *session)
    {
        for (auto acceptedSessionIter = m_acceptedSessions.begin(); acceptedSessionIter != m_acceptedSessions.end();
             ++acceptedSessionIter) {
            if (acceptedSessionIter->get() == session) {
                m_acceptedSessions.erase(acceptedSessionIter);
                return;
            }
        }
    }

    std::vector<std::shared_ptr<Module::Session>> m_acceptedSessions;
};
}

namespace Module {
TEST(Session, NewClientNoSsl)
{
    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    ASSERT_FALSE(session->IsSsl());
    ASSERT_FALSE(session->IsConnected());
    ASSERT_FALSE(session->Disconnect());
}

TEST(Session, NewClientSsl)
{
    boost::asio::io_context ioContext;
    boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
    ASSERT_NO_THROW(sslContext.load_verify_file(CERTIFICATE_PATH));
    auto session = Session::NewClient(ioContext, sslContext);
    ASSERT_TRUE(session->IsSsl());
    ASSERT_FALSE(session->IsConnected());
    ASSERT_FALSE(session->Disconnect());
}

TEST(Session, Move)
{
    boost::asio::io_context ioContext;
    boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
    ASSERT_NO_THROW(sslContext.load_verify_file(CERTIFICATE_PATH));
    auto session = Session::NewClient(ioContext, sslContext);
    auto movedSession(std::move(session));
    ASSERT_TRUE(movedSession->IsSsl());
    ASSERT_FALSE(movedSession->IsConnected());
    ASSERT_FALSE(movedSession->Disconnect());
}

TEST(Session, ConnectToNothing)
{
    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
        resolver.resolve("127.0.0.1", PORT_AS_STRING);
}

TEST(Session, HandshakeOnNothing)
{
    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    ASSERT_FALSE(session->Handshake());
}

TEST(Server, StartNoSsl)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    TestServer server(ioService, serverinfo);

    ASSERT_FALSE(ioService.stopped());
    ASSERT_FALSE(server.IsSsl());
    server.Stop();
    ASSERT_TRUE(ioService.stopped());
}

TEST(Server, StartSsl)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
    TestServer server(ioService, serverinfo);

    ASSERT_FALSE(ioService.stopped());
    ASSERT_TRUE(server.IsSsl());
    server.Stop();
    ASSERT_TRUE(ioService.stopped());
}

TEST(Server, NoCaFile)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
    TestServer server(ioService, serverinfo);
    ASSERT_FALSE(server.Init());
}

TEST(Server, NoEncryPwdFilePath)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
    serverinfo.authInfo.caFilePath = CA_FILE_PATH;
    TestServer server(ioService, serverinfo);
    ASSERT_FALSE(server.Init());
}

TEST(Server, NoCipherText)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
    serverinfo.authInfo.caFilePath = CA_FILE_PATH;
    serverinfo.authInfo.encryPwdFilePath = ENCRYPWD_EMPTY_PATH;
    TestServer server(ioService, serverinfo);
    ASSERT_FALSE(server.Init());
}

TEST(Server, NoKMC)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
    serverinfo.authInfo.caFilePath = CA_FILE_PATH;
    serverinfo.authInfo.encryPwdFilePath = ENCRYPWD_PATH;
    serverinfo.authInfo.kmcStoreFilePath = KMCSTORE_PATH;
    serverinfo.authInfo.kmcBackupStoreFilePath = KMCSTORE_PATH;
    TestServer server(ioService, serverinfo);
    ASSERT_FALSE(server.Init());
}

TEST(ServerPlusSession, ConnectionNoSsl)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    TestServer server(ioService, serverinfo);
    server.Init();

    std::thread serverThread(
        [&ioService]()
        {
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
                boost::asio::make_work_guard(ioService);
            ioService.run();
        });

    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
        resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);

    ASSERT_TRUE(session->Connect(endpoint));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 1);
    ASSERT_TRUE(session->Disconnect());
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 0);

    ioService.stop();
    serverThread.join();
}

// TEST(ServerPlusSession, ConnectionWithSsl)
// {
//     boost::asio::io_service ioService;
//     ServerObjInfo serverinfo;
//     serverinfo.port = PORT;
//     serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
//     serverinfo.authInfo.caFilePath = CA_FILE_PATH;
//     serverinfo.authInfo.encryPwdFilePath = ENCRYPWD_PATH;
//     serverinfo.authInfo.kmcStoreFilePath = KMCSTORE_PATH;
//     serverinfo.authInfo.kmcBackupStoreFilePath = KMCSTORE_PATH;
//     serverinfo.authInfo.keyFilePath = KMCSTORE_PATH;
//     TestServer server(ioService, serverinfo);
//     server.Init();

//     std::thread serverThread(
//         [&ioService]()
//         {
//             boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
//                 boost::asio::make_work_guard(ioService);
//             ioService.run();
//         });

//     boost::asio::io_context ioContext;
//     boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
//     ASSERT_NO_THROW(sslContext.load_verify_file(CERTIFICATE_PATH));
//     auto session = Session::NewClient(ioContext, sslContext);
//     boost::asio::ip::tcp::resolver resolver(ioContext);
//     boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
//         resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);

//     ASSERT_TRUE(session->Connect(endpoint));
//     ASSERT_TRUE(session->Handshake());
//     std::this_thread::sleep_for(std::chrono::milliseconds(2));
//     ASSERT_EQ(server.NumberOfAcceptedSessions(), 1);
//     ASSERT_TRUE(session->Disconnect());
//     std::this_thread::sleep_for(std::chrono::milliseconds(2));
//     ASSERT_EQ(server.NumberOfAcceptedSessions(), 0);

//     ioService.stop();
//     serverThread.join();
// }

TEST(ServerPlusSession, DeadlineTimerDisconnect)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    TestServer server(ioService, serverinfo);
    server.Init();

    std::thread serverThread(
        [&ioService]()
        {
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
                boost::asio::make_work_guard(ioService);
            ioService.run();
        });

    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
        resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);
    ASSERT_TRUE(session->Connect(endpoint));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 1);

    server.StartDeadlineTimerOnFirstSession();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    session->ReceiveMessage();
    ASSERT_FALSE(session->IsConnected());
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 0);

    ioService.stop();
    serverThread.join();
}

TEST(ServerPlusSession, DeadlineTimerStop)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    TestServer server(ioService, serverinfo);
    server.Init();

    std::thread serverThread(
        [&ioService]()
        {
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
                boost::asio::make_work_guard(ioService);
            ioService.run();
        });

    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
        resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);
    ASSERT_TRUE(session->Connect(endpoint));
    std::this_thread::sleep_for(std::chrono::milliseconds(2));
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 1);

    server.StartDeadlineTimerOnFirstSession();
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    server.StopDeadlineTimerOnFirstSession();
    std::this_thread::sleep_for(std::chrono::seconds(1));
    ASSERT_TRUE(session->IsConnected());
    ASSERT_EQ(server.NumberOfAcceptedSessions(), 1);

    ioService.stop();
    serverThread.join();
}

TEST(ServerPlusSession, SendAndReceiveNoSsl)
{
    boost::asio::io_service ioService;
    ServerObjInfo serverinfo;
    serverinfo.port = PORT;
    TestServer server(ioService, serverinfo);
    server.Init();

    std::thread serverThread(
        [&ioService]()
        {
            boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
                boost::asio::make_work_guard(ioService);
            ioService.run();
        });

    boost::asio::io_context ioContext;
    auto session = Session::NewClient(ioContext);
    boost::asio::ip::tcp::resolver resolver(ioContext);
    boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
        resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);
    ASSERT_TRUE(session->Connect(endpoint));
    session->TurnDelayOff();

    auto message = CreateMessage(TEST_DATA_MESSAGE);
    session->SendMessage(*message);
    auto response = session->ReceiveMessage();

    ASSERT_TRUE(response);

    auto responseMessage = std::string(response->Data().data(), response->Data().size());
    ASSERT_EQ(responseMessage, std::string(TEST_DATA_MESSAGE));

    session->Disconnect();

    ioService.stop();
    serverThread.join();
}

// TEST(ServerPlusSession, SendAndReceiveWithSsl)
// {
//     boost::asio::io_service ioService;
//     ServerObjInfo serverinfo;
//     serverinfo.port = PORT;
//     serverinfo.authInfo.certFilePath = CERTIFICATE_PATH;
//     serverinfo.authInfo.caFilePath = CA_FILE_PATH;
//     serverinfo.authInfo.encryPwdFilePath = ENCRYPWD_PATH;
//     serverinfo.authInfo.kmcStoreFilePath = KMCSTORE_PATH;
//     serverinfo.authInfo.kmcBackupStoreFilePath = KMCSTORE_PATH;
//     serverinfo.authInfo.keyFilePath = KMCSTORE_PATH;
//     TestServer server(ioService, serverinfo);
//     server.Init();

//     std::thread serverThread(
//         [&ioService]()
//         {
//             boost::asio::executor_work_guard<boost::asio::io_context::executor_type> executorWorkGuard =
//                 boost::asio::make_work_guard(ioService);
//             ioService.run();
//         });

//     boost::asio::io_context ioContext;
//     boost::asio::ssl::context sslContext(boost::asio::ssl::context::sslv23);
//     sslContext.load_verify_file(CERTIFICATE_PATH);
//     sslContext.use_certificate_file(CERTIFICATE_PATH,boost::asio::ssl::context::pem);
//     auto session = Session::NewClient(ioContext, sslContext);
//     boost::asio::ip::tcp::resolver resolver(ioContext);
//     boost::asio::ip::basic_resolver_results<boost::asio::ip::tcp> endpoint =
//         resolver.resolve(SERVER_ADDRESS, PORT_AS_STRING);
//     ASSERT_TRUE(session->Connect(endpoint));
//     ASSERT_TRUE(session->Handshake());
//     session->TurnDelayOff();

//     auto message = CreateMessage(TEST_DATA_MESSAGE);
//     session->SendMessage(*message);
//     auto response = session->ReceiveMessage();

//     ASSERT_TRUE(response);

//     auto responseMessage = std::string(response->Data().data(), response->Data().size());
//     ASSERT_EQ(responseMessage, std::string(TEST_DATA_MESSAGE));

//     session->Disconnect();

//     ioService.stop();
//     serverThread.join();
// }
}