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
#ifndef THRIFTFACTORY_H
#define THRIFTFACTORY_H

#include <string>
#include <future>
#include <thrift/server/TServer.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/concurrency/ThreadFactory.h>
#include <thrift/transport/TNonblockingServerTransport.h>

#include "IThriftServer.h"
#include "IThriftClient.h"
#include "ICertificateHandler.h"
#include "SslSocketPasswordFactory.h"

namespace thriftservice {
namespace detail {

class ThriftFactory {
public:
    ThriftFactory();
    ~ThriftFactory();
    static ThriftFactory* GetInstance();
    std::shared_ptr<IThriftServer> GetServer(const std::string& host, int32_t port);
    std::shared_ptr<IThriftClient> GetClient(const std::string& host, int32_t port);
    std::shared_ptr<IThriftServer> GetSslServer(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler);
    std::shared_ptr<IThriftClient> GetSslClient(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler);

private:
    std::shared_ptr<apache::thrift::concurrency::Thread> GetThread(
        std::shared_ptr<apache::thrift::concurrency::Runnable> runnable, std::shared_ptr<std::promise<bool>> promise);
    std::shared_ptr<apache::thrift::transport::TNonblockingServerTransport> CreateTransport(
        const std::string& host, int32_t port);
    std::shared_ptr<apache::thrift::transport::TNonblockingServerTransport> CreateSslTransport(const std::string& host,
        int32_t port, std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler);

    std::shared_ptr<SslSocketPasswordFactory> CreateSocketFactory(
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler);

    std::shared_ptr<apache::thrift::transport::TSocket> CreateSocket(const std::string& host, int32_t port);
    std::shared_ptr<apache::thrift::transport::TSocket> CreateSslSocket(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler);

    std::shared_ptr<IThriftServer> GetServerImpl(
        std::shared_ptr<apache::thrift::transport::TNonblockingServerTransport> transport);
    std::shared_ptr<IThriftClient> GetClientImpl(std::shared_ptr<apache::thrift::transport::TTransport> socket);
private:
    std::shared_ptr<apache::thrift::concurrency::ThreadFactory> m_factory;
};
}
}

#endif
