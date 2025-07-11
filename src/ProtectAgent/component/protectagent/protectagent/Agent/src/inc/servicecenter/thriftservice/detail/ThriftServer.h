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
#ifndef THRIFTSERVER_H_
#define THRIFTSERVER_H_

#include <future>
#include "thrift/server/TServer.h"
#include "thrift/processor/TMultiplexedProcessor.h"
#include "thrift/transport/TNonblockingServerTransport.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"

namespace thriftservice {
namespace detail {
using namespace apache::thrift;
using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

class ThriftServer : public IThriftServer {
public:
    friend class ThriftFactory;
    ThriftServer() {}
    virtual ~ThriftServer();
    virtual bool Start();
    virtual bool Stop();
    bool RegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor);

private:
    void StartThread();

private:
    std::shared_ptr<TMultiplexedProcessor> m_processor;
    std::shared_ptr<TServer> m_server;
    std::shared_ptr<Thread> m_thread;
    std::shared_future<bool> m_future;
};
}
}

#endif