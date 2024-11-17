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
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include <thriftservice/detail/ThriftFactory.h>
#include <future>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TNonblockingServerSocket.h>
#include <thrift/transport/TNonblockingSSLServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/server/TNonblockingServer.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <common/Log.h>
#include "common/ConfigXmlParse.h"
#include "servicecenter/thriftservice/detail/ThriftServer.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "servicecenter/thriftservice/detail/ThriftClient.h"
#include "servicecenter/thriftservice/detail/ThreadFactoryProxy.h"
#include "servicecenter/thriftservice/detail/ThreadProxy.h"
#include "servicecenter/thriftservice/detail/TServerEventHandlerImpl.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace certificateservice;

namespace thriftservice {
namespace detail {
constexpr uint32_t THRIFT_MAX_FRAME_SIZE = 1024 * 1024;
constexpr uint32_t THRIFT_WRITE_BUFFER_DEFAULT_SIZE = 1024 * 1024;

static void SetFDCloseOnExec(int32_t fd)
{
#ifndef WIN32
    DBGLOG("SetFDCloseOnExec enter handler = %d\n.", fd);
    mp_int32 iFlag = fcntl(fd, F_GETFD);
    if (iFlag == MP_FAILED) {
        ERRLOG("fcntl failed! handler = %d\n.", fd);
        return;
    }
    iFlag = iFlag | FD_CLOEXEC;
    auto iRet = fcntl(fd, F_SETFD, iFlag);
    if (iRet == MP_FAILED) {
        ERRLOG("fcntl failed! handler = %d\n.", fd);
        return;
    }
#endif
}

ThriftFactory::ThriftFactory()
{
    m_factory = std::make_shared<ThreadFactoryProxy>();
    m_factory->setDetached(false);
}

ThriftFactory::~ThriftFactory()
{}

ThriftFactory* ThriftFactory::GetInstance()
{
    static ThriftFactory instance;
    return &instance;
}

EXTER_ATTACK std::shared_ptr<IThriftServer> ThriftFactory::GetServer(const std::string& host, int32_t port)
{
    auto transport = CreateTransport(host, port);
    auto transportSocket = std::dynamic_pointer_cast<TNonblockingServerSocket>(transport);
    if (transportSocket) {
        uint32_t timeout = GetThriftTimeout();
        transportSocket->setSendTimeout(timeout);
        transportSocket->setRecvTimeout(timeout);
        transportSocket->setKeepAlive(true);
    }
    return GetServerImpl(transport);
}

EXTER_ATTACK std::shared_ptr<IThriftServer> ThriftFactory::GetSslServer(
    const std::string& host, int32_t port, std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto transport = CreateSslTransport(host, port, certificateHandler);
    auto transportSocket = std::dynamic_pointer_cast<TNonblockingServerSocket>(transport);
    if (transportSocket) {
        uint32_t timeout = GetThriftTimeout();
        transportSocket->setSendTimeout(timeout);
        transportSocket->setRecvTimeout(timeout);
        transportSocket->setKeepAlive(true);
    }
    return GetServerImpl(transport);
}

std::shared_ptr<IThriftClient> ThriftFactory::GetClient(const ClientSocketOpt& opt)
{
    std::shared_ptr<TTransport> socket = CreateSocket(opt.host, opt.port);
    return GetClientImpl(socket, opt);
}

// 客户端使用证书域名连接agent插件，主机host文件需要添加证书域名和对应agent服务ip，thrift默认安全策略是使用域名连接
std::shared_ptr<IThriftClient> ThriftFactory::GetSslClient(
    const ClientSocketOpt& opt, std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::string hostname;
    certificateHandler->GetCertificateConfig(CertificateConfig::HOST_NAME, hostname);
    std::shared_ptr<TTransport> socket = CreateSslSocket(hostname, opt.port, certificateHandler);
    if (socket) {
        return GetClientImpl(socket, opt);
    }
    return nullptr;
}

std::shared_ptr<Thread> ThriftFactory::GetThread(
    std::shared_ptr<Runnable> runnable, std::shared_ptr<std::promise<bool>> p)
{
    std::shared_ptr<Thread> th = m_factory->newThread(runnable);
    auto thProxy = std::dynamic_pointer_cast<ThreadProxy>(th);
    if (thProxy) {
        thProxy->SetPromise(p);
    }
    return th;
}

std::shared_ptr<SslSocketPasswordFactory> ThriftFactory::CreateSocketFactory(
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::shared_ptr<SslSocketPasswordFactory> pServerSocketFactory = std::make_shared<SslSocketPasswordFactory>(
        TLSv1_2);
    pServerSocketFactory->m_handler = certificateHandler;
    return pServerSocketFactory;
}

std::shared_ptr<TNonblockingServerTransport> ThriftFactory::CreateTransport(const std::string& host, int32_t port)
{
    auto socket = std::make_shared<TNonblockingServerSocket>(host, port);
    socket->setListenCallback(SetFDCloseOnExec);
    return socket;
}

std::shared_ptr<TNonblockingServerTransport> ThriftFactory::CreateSslTransport(
    const std::string& host, int32_t port, std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto factory = CreateSocketFactory(certificateHandler);
    if (factory->LoadServerCertificate()) {
        auto socket = std::make_shared<TNonblockingSSLServerSocket>(host, port, factory);
        socket->setListenCallback(SetFDCloseOnExec);
        return socket;
    } else {
        return nullptr;
    }
}

std::shared_ptr<TSocket> ThriftFactory::CreateSocket(const std::string& host, int32_t port)
{
    return std::make_shared<TSocket>(host, port);
}

std::shared_ptr<TSocket> ThriftFactory::CreateSslSocket(
    const std::string& host, int32_t port, std::shared_ptr<ICertificateHandler> certificateHandler)
{
    auto factory = CreateSocketFactory(certificateHandler);
    if (factory->LoadClientCertificate()) {
        return factory->createSocket(host, port);
    } else {
        return nullptr;
    }
}

std::shared_ptr<IThriftServer> ThriftFactory::GetServerImpl(std::shared_ptr<TNonblockingServerTransport> transport)
{
    std::shared_ptr<ThriftServer> ret = std::make_shared<ThriftServer>();
    ret->m_processor = std::make_shared<TMultiplexedProcessor>();
    auto server = std::make_shared<TNonblockingServer>(
        ret->m_processor, std::make_shared<TBinaryProtocolFactory>(), transport);
    server->setOverloadAction(T_OVERLOAD_DRAIN_TASK_QUEUE);
    server->setNumIOThreads(GetPluginMaxConnectCount());
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

std::shared_ptr<IThriftClient> ThriftFactory::GetClientImpl(
    std::shared_ptr<TTransport> socket, const ClientSocketOpt& opt)
{
    auto tsocket = std::dynamic_pointer_cast<TSocket>(socket);
    if (tsocket) {
        tsocket->setConnTimeout(opt.connTimeout);
        tsocket->setRecvTimeout(opt.recvTimeout);
        tsocket->setSendTimeout(opt.sendTimeout);
        tsocket->setKeepAlive(opt.bKeepAlive);
    }
    std::shared_ptr<ThriftClient> ret = std::make_shared<ThriftClient>();
    ret->m_transport = std::make_shared<TFramedTransport>(socket);
    ret->m_protocol = std::make_shared<TBinaryProtocol>(ret->m_transport);
    return ret;
}

uint32_t ThriftFactory::GetThriftTimeout()
{
    mp_int32 thriftTimeout = 0;
    auto iRet = CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, THRIFT_TIME_OUT, thriftTimeout);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Get the THRIFT_TIME_OUT from agent_cfg.xml failed.");
        return THRIFT_TIMEOUT_DEFAULT;
    }
    if (thriftTimeout > THRIFT_TIMEOUT_MAX || thriftTimeout < THRIFT_TIMEOUT_MIN) {
        WARNLOG("The THRIFT_TIME_OUT(%d) from agent_cfg.xml file is in wrong range.", thriftTimeout);
        return THRIFT_TIMEOUT_DEFAULT;
    }
    return (uint32_t)thriftTimeout;
}

uint32_t ThriftFactory::GetPluginMaxConnectCount()
{
    mp_int32 maxConnection = 0;
    auto iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_FRAME_THRIFT_SECTION, MAX_PLUGIN_CONNECTION_COUNT, maxConnection);
    if (iRet != MP_SUCCESS) {
        WARNLOG("Get the MAX_PLUGIN_CONNECTION_COUNT from agent_cfg.xml failed.");
        return PLUGIN_CONNECT_DEFAULT_COUNT;
    }
    if (maxConnection > PLUGIN_CONNECT_MAX_COUNT || maxConnection < PLUGIN_CONNECT_MIN_COUNT) {
        WARNLOG("The MAX_PLUGIN_CONNECTION_COUNT(%d) from agent_cfg.xml file is in wrong range.", maxConnection);
        return PLUGIN_CONNECT_DEFAULT_COUNT;
    }
    return (uint32_t)maxConnection;
}
}  // namespace detail
}  // namespace thriftservice