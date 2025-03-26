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
#include "ThriftFactory.h"
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TNonblockingSSLServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include "ThriftServer.h"
#include "ThriftClient.h"
#include "TServerEventHandlerImpl.h"
#include "ThreadProxy.h"
#include "ThreadFactoryProxy.h"
#include "log/Log.h"

using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace certificateservice;
using namespace std;

namespace thriftservice {
namespace detail {
constexpr uint32_t MAX_CONNECTION = 10;
constexpr uint32_t THRIFT_MAX_FRAME_SIZE = 1024 * 1024;
constexpr uint32_t THRIFT_WRITE_BUFFER_DEFAULT_SIZE = 1024 * 1024;
// thrift client call agent thrift server timeout, discussed to set to 120s
constexpr uint32_t MAX_THRIFT_COMMUNICATION_TIME = 120 * 1000;
constexpr uint32_t THRIFT_COMMUNICATION_TIME_30S = 30 * 1000;
constexpr auto MODULE = "ThriftFactory";

ThriftFactory::ThriftFactory()
{
    m_factory = std::make_shared<ThreadFactoryProxy>();
    m_factory->setDetached(false);
}

ThriftFactory::~ThriftFactory()
{
}

ThriftFactory* ThriftFactory::GetInstance()
{
    static ThriftFactory instance;
    return &instance;
}

std::shared_ptr<IThriftServer> ThriftFactory::GetServer(const std::string& host, int32_t port)
{
    auto transport = CreateTransport(host, port);
    auto transportSocket = std::dynamic_pointer_cast<TNonblockingServerSocket>(transport);
    if (transportSocket) {
            transportSocket->setSendTimeout(MAX_THRIFT_COMMUNICATION_TIME);
            transportSocket->setRecvTimeout(MAX_THRIFT_COMMUNICATION_TIME);
            transportSocket->setKeepAlive(true);
    }
    return GetServerImpl(transport);
}

std::shared_ptr<IThriftServer> ThriftFactory::GetSslServer(const std::string& host, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto transport = CreateSslTransport(host, port, certificateHandler);
    if (transport != nullptr) {
        return GetServerImpl(transport);
    }
    return nullptr;
}

std::shared_ptr<IThriftClient> ThriftFactory::GetClient(const std::string& host, int32_t port)
{
    std::shared_ptr<TTransport> socket = CreateSocket(host, port);
    return GetClientImpl(socket);
}

std::shared_ptr<IThriftClient> ThriftFactory::GetSslClient(const std::string& /* host */, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::string hostName;
    certificateHandler->GetCertificateConfig(CertificateConfig::HOST_NAME, hostName);
    std::shared_ptr<TTransport> socket = CreateSslSocket(hostName, port, certificateHandler);
    if (socket != nullptr) {
        return GetClientImpl(socket);
    }
    return nullptr;
}

std::shared_ptr<SslSocketPasswordFactory> ThriftFactory::CreateSocketFactory(
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::shared_ptr<SslSocketPasswordFactory> pServerSocketFactory = std::make_shared<SslSocketPasswordFactory>();
    pServerSocketFactory->m_handler = certificateHandler;
    return pServerSocketFactory;
}

std::shared_ptr<TNonblockingServerTransport> ThriftFactory::CreateTransport(const std::string& host, int32_t port)
{
    return std::make_shared<TNonblockingServerSocket>(host, port);
}

std::shared_ptr<TNonblockingServerTransport> ThriftFactory::CreateSslTransport(const std::string& host, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto factory = CreateSocketFactory(certificateHandler);
    if (factory->LoadServerCertificate()) {
        return std::make_shared<TNonblockingSSLServerSocket>(host, port, factory);
    } else {
        return nullptr;
    }
}

std::shared_ptr<TSocket> ThriftFactory::CreateSocket(const std::string& host, int32_t port)
{
    return std::make_shared<TSocket>(host, port);
}

std::shared_ptr<TSocket> ThriftFactory::CreateSslSocket(const std::string& host, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto factory = CreateSocketFactory(certificateHandler);
    if (factory->LoadClientCertificate()) {
        return factory->createSocket(host, port);
    } else {
        ERRLOG("CreateSslSocket failed");
        return nullptr;
    }
}

std::shared_ptr<Thread> ThriftFactory::GetThread(std::shared_ptr<Runnable> runnable,
    std::shared_ptr<std::promise<bool>> prom)
{
    std::shared_ptr<Thread> th = m_factory->newThread(runnable);
    auto thProxy = std::dynamic_pointer_cast<ThreadProxy>(th);
    if (thProxy) {
        thProxy->SetPromise(prom);
    }
    return th;
}

std::shared_ptr<IThriftServer> ThriftFactory::GetServerImpl(std::shared_ptr<TNonblockingServerTransport> transport)
{
    std::shared_ptr<ThriftServer> ret = std::make_shared<ThriftServer>();
    ret->m_processor = std::make_shared<TMultiplexedProcessor>();
    auto server = std::make_shared<TNonblockingServer>(ret->m_processor, std::make_shared<TBinaryProtocolFactory>(),
        transport);
    server->setOverloadAction(T_OVERLOAD_DRAIN_TASK_QUEUE);
    server->setNumIOThreads(MAX_CONNECTION);
    server->setResizeBufferEveryN(0);
    server->setIdleReadBufferLimit(0);
    server->setIdleWriteBufferLimit(0);
    server->setMaxFrameSize(THRIFT_MAX_FRAME_SIZE);
    server->setWriteBufferDefaultSize(THRIFT_WRITE_BUFFER_DEFAULT_SIZE);
    std::shared_ptr<std::promise<bool>> promise = std::make_shared<std::promise<bool>>();
    std::shared_ptr<TServerEventHandlerImpl> handler = std::make_shared<TServerEventHandlerImpl>();
    handler->SetPromise(promise);
    server->setServerEventHandler(handler);

    std::shared_ptr<apache::thrift::concurrency::Runnable> serverThreadRunner(server);
    ret->m_thread = GetThread(serverThreadRunner, promise);
    ret->m_server = server;
    ret->m_future = promise->get_future();
    return ret;
}

std::shared_ptr<IThriftClient> ThriftFactory::GetClientImpl(std::shared_ptr<TTransport> socket)
{
    auto tsocket = std::dynamic_pointer_cast<TSocket>(socket);
    if (tsocket) {
        tsocket->setConnTimeout(THRIFT_COMMUNICATION_TIME_30S);
        tsocket->setRecvTimeout(MAX_THRIFT_COMMUNICATION_TIME);
        tsocket->setSendTimeout(MAX_THRIFT_COMMUNICATION_TIME);
        tsocket->setKeepAlive(true);
    }
    std::shared_ptr<ThriftClient> ret = std::make_shared<ThriftClient>();
    ret->m_transport = std::make_shared<TFramedTransport>(socket);
    ret->m_protocol = std::make_shared<TBinaryProtocol>(ret->m_transport);
    return ret;
}
}
}