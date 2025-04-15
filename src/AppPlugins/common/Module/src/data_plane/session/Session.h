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
#ifndef DP_SESSION_H
#define DP_SESSION_H
// std
#include <memory>
#include <functional>
#include <utility>
#include <queue>
#include <mutex>

// Vendors
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/optional.hpp>
#include <boost/lockfree/queue.hpp>
#include <boost/utility/string_view.hpp>

#include "log/Log.h"
#include <protocol/Message.h>
#include <protocol/MessageHeader.h>

namespace Module {

class Session : public std::enable_shared_from_this<Session> {
public:
    explicit Session(boost::asio::io_context& ioContext);
    Session(boost::asio::io_context& ioContext,
            boost::asio::ssl::context& sslContext);

    explicit Session(boost::asio::ip::tcp::socket socket,
                     boost::asio::io_service& ioService);
    Session(boost::asio::ip::tcp::socket socket,
            boost::asio::ssl::context& sslContext,
            boost::asio::io_service& ioService);
    static std::shared_ptr<Session> NewClient(boost::asio::io_context& ioContext);
    static std::shared_ptr<Session> NewClient(boost::asio::io_context& ioContext,
                                              boost::asio::ssl::context& sslContext);
    static std::shared_ptr<Session> NewServer(boost::asio::ip::tcp::socket socket,
                                              boost::asio::io_service& ioService);
    static std::shared_ptr<Session> NewServer(boost::asio::ip::tcp::socket socket,
                                              boost::asio::ssl::context& sslContext,
                                              boost::asio::io_service& ioService);
    ~Session();

    void SetOnDisconnectCallback(const std::function<void(Session*)> onDisconnectCallback);

    bool IsSsl() const;
    bool IsConnected() const;

    void TurnDelayOff();
    void ConfigureTcpKeepAlive();

    bool Connect(const boost::asio::ip::tcp::resolver::results_type& endpoints);
    bool Disconnect();
    bool Handshake();
    void SendMessage(const Protocol::Message& message);
    void SendAsyncMessage(const std::shared_ptr<Protocol::Message>& message,
                          const std::function<void(const boost::system::error_code& error, std::size_t,
                          bool)>& finalHandler);
    boost::optional<Protocol::Message> ReceiveMessage();
    void ReceiveAsyncMessage(const std::function<void(std::shared_ptr<Protocol::Message>,
                                                      std::shared_ptr<Protocol::MessageHeader>, bool)>& finalHandler);

    void StartDeadlineTimer(const boost::posix_time::seconds& seconds);
    void StopDeadlineTimer();

    bool HandleError(const boost::system::error_code& error, boost::string_view message);

    uint16_t GetSequecenNumber();

private:
    bool VerifyCertificate(bool preverified, boost::asio::ssl::verify_context& context);

    void ClearQueues();

    // Reasoning:
    //   Queues are required to synchronize async write and read operations. Multiple boost::asio::async_write and
    //   boost::asio::async_read operations could be performed concurrently, but that will lead to wrong write/read
    //   byte order. To prevent this, we queue all operations and dequeue them one by one during the execution.
    //   When user requests to send/receive async message, we queue that operation, and if this operation is the only
    //   one in a queue - execute it. When operation is done (when boost::asio::async_write or boost::asio::async_read
    //   is successful), we check if there is anything left in a queue and execute it from the front. If we don't
    //   do that - queued operations except fot the first one will never be executed.
    // Why mutex:
    //   If we will use lock free queue (for example, boost::lockfree:queue), we will not be able to guarantee the
    //   state of the queue between operations. Consider this:
    //     1. Check if queue empty
    //     -- Between these two points we can't guarantee that another thread will not try to perform the same
    //        operations and achieve the same result. We could end up with two same type operations to be executed at
    //        once.
    //     2. Queue operation
    //     3. Start execution
    mutable std::mutex m_writeQueueMutex;
    std::queue<std::function<void()>> m_writeQueue;

    mutable std::mutex m_readQueueMutex;
    std::queue<std::function<void()>> m_readQueue;

    mutable std::recursive_mutex m_disconnectMutex;
    bool m_isServer;
    std::unique_ptr<boost::asio::ssl::stream<boost::asio::ip::tcp::socket>> m_sslSocket;
    std::unique_ptr<boost::asio::ip::tcp::socket> m_socket;

    boost::asio::deadline_timer m_deadlineTimer;
    std::function<void(Session*)> m_onDisconnectCallback;

    bool m_connected;
    uint16_t m_sequnceNumber = 0;
};
}

#endif
