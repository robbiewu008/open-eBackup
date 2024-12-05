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
#include <session/Session.h>

// std
#include <vector>

// Vendors
#include <openssl/x509.h>
#include "log/Log.h"

namespace {

constexpr auto FAILED_TO_SEND_MESSAGE     = "Failed to send message";
constexpr auto FAILED_TO_READ_HEADER      = "Failed to read message header";
constexpr auto FAILED_TO_READ_MESSAGE     = "Failed to read message";

// --- Send functions ---
template <typename SocketType>
void SendMessage(Module::Session& session,
                 SocketType& socket,
                 const Module::Protocol::Message& message)
{
    // Reusable error
    boost::system::error_code error;

    // Send message
    boost::asio::write(*socket, boost::asio::buffer(message.Data().data(), message.Data().size()), error);
    if (session.HandleError(error, FAILED_TO_SEND_MESSAGE)) {
        return;
    }
}

template <typename SocketType>
void SendAsync(const std::shared_ptr<Module::Session>& session,
               SocketType& socket,
               const std::shared_ptr<Module::Protocol::Message>& message,
               const std::function<void(const boost::system::error_code& error, std::size_t, bool)>& finalHandler)
{
    boost::asio::async_write(*socket,
        boost::asio::buffer(message->Data().data(), message->Data().size()),
        [session, message, finalHandler] (const boost::system::error_code& error, std::size_t length) {
            if (session->HandleError(error, FAILED_TO_SEND_MESSAGE)) {
                ERRLOG("Failed to send message.");
                finalHandler(error, length, true);
                return;
            }
            // Session should be alive up until this point.
            // If user want to extend life ot a session, they should capture it in callback.
            (void) session;
            finalHandler(error, length, false);
        });
}

// --- Receive functions ---
template <typename SocketType>
boost::optional<Module::Protocol::Message> ReceiveMessage(Module::Session& session, SocketType& socket)
{
    // Reusable error
    boost::system::error_code error;

    // Read message header
    Module::Protocol::MessageHeader header{};
    boost::asio::read(*socket, boost::asio::buffer(&header, sizeof(header)), error);
    if (session.HandleError(error, FAILED_TO_READ_HEADER)) {
        ERRLOG("Failed to receive message.");
        return boost::none;
    }

    // Receive message
    Module::Protocol::Message message;
    message.Resize(header.messageLength);
    boost::asio::read(*socket, boost::asio::buffer(message.Data().data(), header.messageLength), error);
    if (session.HandleError(error, FAILED_TO_READ_MESSAGE)) {
        return boost::none;
    }

    return message;
}

template <typename SocketType>
void ReceiveAsync(const std::shared_ptr<Module::Session>& session,
                  SocketType& socket,
                  const std::function<void(std::shared_ptr<Module::Protocol::Message>,
                  std::shared_ptr<Module::Protocol::MessageHeader>, bool)>& finalHandler)
{
    // Read message header
    auto header = std::make_shared<Module::Protocol::MessageHeader>();
    auto message = std::make_shared<Module::Protocol::Message>();
    boost::asio::async_read(
        *socket,
        boost::asio::buffer(header.get(), sizeof(*header)),
        [session, &socket, message, header,  finalHandler] (const boost::system::error_code& error, std::size_t) {
            if (session->HandleError(error, FAILED_TO_READ_HEADER)) {
                ERRLOG("Failed to receive async message.");
                finalHandler(message, header, true);
                return;
            }
            // Read message body
            message->Resize(header->messageLength);
            boost::asio::async_read(
                *socket,
                boost::asio::buffer(message->Data().data(), header->messageLength),
                [session, message, header, finalHandler](const boost::system::error_code& error, std::size_t) {
                    if (session->HandleError(error, FAILED_TO_READ_MESSAGE)) {
                        return;
                    }

                    // Session should be alive up until this point.
                    // If user want to extend life ot a session, they should capture it in callback.
                    (void) session;
                    finalHandler(message, header, false);
                });
        });
}

}

namespace Module {

std::shared_ptr<Session> Session::NewClient(boost::asio::io_context& ioContext)
{
    return std::make_shared<Session>(ioContext);
}

std::shared_ptr<Session> Session::NewClient(boost::asio::io_context& ioContext,
                                            boost::asio::ssl::context& sslContext)
{
    return std::make_shared<Session>(ioContext, sslContext);
}

std::shared_ptr<Session> Session::NewServer(boost::asio::ip::tcp::socket socket,
                                            boost::asio::io_service& ioService)
{
    return std::make_shared<Session>(std::move(socket), ioService);
}

std::shared_ptr<Session> Session::NewServer(boost::asio::ip::tcp::socket socket,
                                            boost::asio::ssl::context& sslContext,
                                            boost::asio::io_service& ioService)
{
    return std::make_shared<Session>(std::move(socket), sslContext, ioService);
}

Session::~Session()
{
    Disconnect();
}

void Session::SetOnDisconnectCallback(const std::function<void(Session*)> onDisconnectCallback)
{
    m_onDisconnectCallback = std::move(onDisconnectCallback);
}

bool Session::IsSsl() const
{
    return m_sslSocket != nullptr;
}

bool Session::IsConnected() const
{
    return m_connected;
}

void Session::TurnDelayOff()
{
    // Turn option "NO_DELAY" on.
    //
    // Quote:
    // "Most TCP implementations implement the Nagle algorithm, where a TCP connection can only have one outstanding
    // small segment that has not yet been acknowledged. This causes TCP to delay sending any more packets until it
    // receives an acknowledgement or until it can bundle up more data and send a full size segment.
    // Applications that use request/response workloads should use the setsockopt() call to enable the TCP_NODELAY
    // option. For example, the telnet and rlogin utilities, Network File System (NFS), and web servers,
    // already use the TCP_NODELAY option to disable nagle."
    // (https://www.ibm.com/docs/en/aix/7.3?topic=tuning-tcp-nodelay-tcp-nagle-limit-options)
    //
    // Nagle algorithm: https://en.wikipedia.org/wiki/Nagle%27s_algorithm
    //
    // TCP connection have a buffer, from which data is sent using Nagle algo. This algorithm does not work well
    // with request-response type of connections, since requests and responses could be tiny, which will lead to delay
    // between putting data to a buffer and sending said buffer. Delay could be 40+ms for each request and response.
    // So we turn off Nagle algo by turning "NO_DELAY" option on.
    IsSsl() ? m_sslSocket->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true))
            : m_socket->lowest_layer().set_option(boost::asio::ip::tcp::no_delay(true));
}

bool Session::Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints)
{
    boost::asio::ip::tcp::socket::lowest_layer_type& lowestLayer = IsSsl() ? m_sslSocket->lowest_layer() :
                                                                   m_socket->lowest_layer();
    boost::system::error_code error;
    boost::asio::connect(lowestLayer, endpoints, error);
    if (error) {
        ERRLOG("[Connection: {%zu}] Failed to connect: {%s}", reinterpret_cast<std::size_t>(this),
            error.message().c_str());
        return false;
    }

    INFOLOG("New connection established(id: %zu)", reinterpret_cast<std::size_t>(this));

    TurnDelayOff();

    m_connected = true;

    return true;
}

bool Session::Disconnect()
{
    m_deadlineTimer.cancel();
    if (m_connected && m_sslSocket != nullptr) {
        boost::system::error_code error;
        m_sslSocket->lowest_layer().cancel(error);
        m_sslSocket->shutdown(error);
        m_sslSocket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        m_sslSocket->lowest_layer().close(error);
        m_connected = false;
        ClearQueues();
        INFOLOG("[Connection: {%zu}] ended successfully", reinterpret_cast<std::size_t>(this));
        if (m_onDisconnectCallback) {
            m_onDisconnectCallback(this);
        }
        return true;
    }
    if (m_connected && m_socket != nullptr) {
        boost::system::error_code error;
        m_socket->lowest_layer().cancel(error);
        m_socket->lowest_layer().shutdown(boost::asio::ip::tcp::socket::shutdown_both, error);
        m_socket->lowest_layer().close(error);
        m_connected = false;
        ClearQueues();
        INFOLOG("[Connection: {%zu}] ended successfully", reinterpret_cast<std::size_t>(this));
        if (m_onDisconnectCallback) {
            m_onDisconnectCallback(this);
        }
        return true;
    }

    return false;
}

bool Session::Handshake()
{
    if (m_sslSocket == nullptr && m_socket == nullptr) {
        ERRLOG("[Connection: {}] is not initialized", reinterpret_cast<std::size_t>(this));
        return false;
    }

    if (!IsSsl()) {
        ERRLOG("[Connection: {}] does not use SLL, no reason to perform handshake",
            reinterpret_cast<std::size_t>(this));
        return false;
    }

    boost::system::error_code error;
    m_sslSocket->handshake(m_isServer ? boost::asio::ssl::stream_base::server
                                      : boost::asio::ssl::stream_base::client,
                           error);

    if (Session::HandleError(error, "Handshake failed")) {
        return false;
    }

    return true;
}

void Session::SendMessage(const Protocol::Message& message)
{
    if (IsSsl()) {
        ::SendMessage(*this, m_sslSocket, message);
    } else {
        ::SendMessage(*this, m_socket, message);
    }
}

void Session::SendAsyncMessage(const std::shared_ptr<Protocol::Message>& message,
                               const std::function<void(const boost::system::error_code&, std::size_t,
                               bool)>& finalHandler)
{
    auto self = shared_from_this();
    // Wrap final handler in a way that we will check if there is anything in the queue to queue it up to perform new
    // send operation.
    auto finalHandlerWrapper = [self, finalHandler](const boost::system::error_code& error, std::size_t length,
                                                    bool networkErrorFlag) {
        if (networkErrorFlag) {
            ERRLOG("Network error occurs.");
            finalHandler(error, length, networkErrorFlag);
            return;
        }
        std::function<void()> operation;
        {
            std::lock_guard<std::mutex> lockGuard(self->m_writeQueueMutex);
            self->m_writeQueue.pop();
            if (!self->m_writeQueue.empty()) {
                operation = self->m_writeQueue.front();
            }
        }

        if (operation) {
            operation();
        }

        finalHandler(error, length, networkErrorFlag);
    };
    std::function<void()> writeOperation;
    if (IsSsl()) {
        writeOperation = [self, message, finalHandlerWrapper]() {
            ::SendAsync(self, self->m_sslSocket, message, finalHandlerWrapper);
        };
    } else {
        writeOperation = [self, message, finalHandlerWrapper]() {
            ::SendAsync(self, self->m_socket, message, finalHandlerWrapper);
        };
    }

    auto execute = false;
    {
        std::lock_guard<std::mutex> lockGuard(m_writeQueueMutex);
        if (m_writeQueue.empty()) {
            execute = true;
        }
        m_writeQueue.push(writeOperation);
    }
    if (execute) {
        writeOperation();
    }
}

boost::optional<Protocol::Message> Session::ReceiveMessage()
{
    if (IsSsl()) {
        return ::ReceiveMessage(*this, m_sslSocket);
    } else {
        return ::ReceiveMessage(*this, m_socket);
    }
}

void Session::ReceiveAsyncMessage(
    const std::function<void(std::shared_ptr<Protocol::Message>, std::shared_ptr<Protocol::MessageHeader>,
    bool)>& finalHandler)
{
    auto self = shared_from_this();

    // Wrap final handler in a way that we will check if there is anything in the queue to queue it up to perform new
    // receive operation.
    auto finalHandlerWrapper = [self, finalHandler](std::shared_ptr<Protocol::Message> message,
                                                    std::shared_ptr<Protocol::MessageHeader> msgHeader,
                                                    bool networkErrorFlag) {
        if (networkErrorFlag) {
            ERRLOG("Network error occurs.");
            finalHandler(std::move(message), msgHeader, networkErrorFlag);
            return;
        }
        std::function<void()> operation;
        {
            std::lock_guard<std::mutex> lockGuard(self->m_readQueueMutex);
            self->m_readQueue.pop();
            if (!self->m_readQueue.empty()) {
                operation = self->m_readQueue.front();
            }
        }
        if (operation) {
            operation();
        }

        finalHandler(std::move(message), msgHeader, networkErrorFlag);
    };

    std::function<void()> readOperation;
    if (IsSsl()) {
        readOperation = [self, finalHandlerWrapper]() {
            ::ReceiveAsync(self, self->m_sslSocket, finalHandlerWrapper);
        };
    } else {
        readOperation = [self, finalHandlerWrapper]() {
            ::ReceiveAsync(self, self->m_socket, finalHandlerWrapper);
        };
    }

    auto execute = false;
    {
        std::lock_guard<std::mutex> lockGuard(m_readQueueMutex);
        if (m_readQueue.empty()) {
            execute = true;
        }
        m_readQueue.push(readOperation);
    }
    if (execute) {
        readOperation();
    }
}

void Session::StartDeadlineTimer(const boost::posix_time::seconds& seconds)
{
    m_deadlineTimer.cancel();
    m_deadlineTimer.expires_from_now(seconds);
    m_deadlineTimer.async_wait([this](const boost::system::error_code& error) {
        if (!error) {
            // Timer expired
            ERRLOG("[Connection: %zu] Deadline timer expired, disconnecting",
                reinterpret_cast<std::size_t>(this));
            Disconnect();
        }
    });
}

void Session::StopDeadlineTimer()
{
    INFOLOG("[Connection: %zu] Deadline timer was stopped", reinterpret_cast<std::size_t>(this));
    m_deadlineTimer.cancel();
}

bool Session::HandleError(const boost::system::error_code& error, boost::string_view message)
{
    switch (error.value()) {
        case boost::system::errc::errc_t::success:
            return false;
        case boost::asio::error::eof:
        case boost::asio::error::connection_reset:
        case boost::asio::error::connection_aborted:
        case boost::asio::error::network_down:
        case boost::asio::error::network_reset:
        case boost::asio::error::operation_aborted:
        case boost::asio::error::broken_pipe:
        case boost::asio::ssl::error::stream_truncated:
            ERRLOG("[Connection: %zu] error, error(%d, %s), msg(%s).", reinterpret_cast<std::size_t>(this),
                error.value(), error.message().c_str(), message.data());
            Disconnect();
            return true;
        default:
            ERRLOG("[Connection: %zu] error value(%d, %s): msg(%s)", reinterpret_cast<std::size_t>(this),
                error.value(), error.message().c_str(), message.data());
            Disconnect();
            return true;
    }
}

Session::Session(boost::asio::io_context& ioContext)
    : m_isServer(false),
    m_socket(new boost::asio::ip::tcp::socket(ioContext)),
    m_deadlineTimer(ioContext),
    m_connected(false) // When we are creating a new session for client, it is not connected, and we need to do it
                         // manually.
{
    DBGLOG("New client session with no SSL");
}

Session::Session(boost::asio::io_context& ioContext,
                 boost::asio::ssl::context& sslContext)
    : m_isServer(false),
    m_sslSocket(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(ioContext, sslContext)),
    m_deadlineTimer(ioContext),
    m_connected(false) // When we are creating a new session for client, it is not connected, and we need to do it
                         // manually.
{
    DBGLOG("New client session with SSL");
    m_sslSocket->set_verify_mode(boost::asio::ssl::verify_peer);
    m_sslSocket->set_verify_callback(std::bind(&Session::VerifyCertificate,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2));
}

Session::Session(boost::asio::ip::tcp::socket socket,
                 boost::asio::io_service& ioService)
    : m_isServer(true),
    m_socket(new boost::asio::ip::tcp::socket(std::move(socket))),
    m_deadlineTimer(ioService),
    m_connected(true) // When we are creating a new session for a server, it is already connected, so we do need to
                        // do it manually.
{
    DBGLOG("New server session with no SSL");
}

Session::Session(boost::asio::ip::tcp::socket socket,
                 boost::asio::ssl::context& sslContext,
                 boost::asio::io_service& ioService)
    : m_isServer(true),
    m_sslSocket(new boost::asio::ssl::stream<boost::asio::ip::tcp::socket>(std::move(socket), sslContext)),
    m_deadlineTimer(ioService),
    m_connected(true) // When we are creating a new session for a server, it is already connected, so we do need to
                        // do it manually.
{
    DBGLOG("New server session with SSL");
    m_sslSocket->set_verify_mode(boost::asio::ssl::verify_peer);
    m_sslSocket->set_verify_callback(std::bind(&Session::VerifyCertificate,
                                               this,
                                               std::placeholders::_1,
                                               std::placeholders::_2));
}

bool Session::VerifyCertificate(bool preverified, boost::asio::ssl::verify_context& context)
{
    int bufferSize = 256;
    char subjectName[bufferSize];
    X509* cert = X509_STORE_CTX_get_current_cert(context.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subjectName, bufferSize);
    INFOLOG("Verified certificate of %s", subjectName);
    return preverified;
}

void Session::ClearQueues()
{
    std::lock_guard<std::mutex> readLockGuard(m_readQueueMutex);
    std::lock_guard<std::mutex> writeLockGuard(m_writeQueueMutex);

    // Since queues will have handlers that contains std::shared_ptr on the same session, we need to clear them
    // to prevent memory leakage.
    // We could pop each entry from the queue until it become empty, but assigning an empty queue achieves the same
    // result - all entries are going to be destroyed alongside with old queue.
    m_readQueue = {};
    m_writeQueue = {};
}

uint16_t Session::GetSequecenNumber()
{
    m_sequnceNumber++;
    return m_sequnceNumber;
}
}
