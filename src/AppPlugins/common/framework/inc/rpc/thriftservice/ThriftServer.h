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
#ifndef THRIFTSERVER_H
#define THRIFTSERVER_H

#include <future>
#include <thrift/server/TServer.h>
#include <thrift/processor/TMultiplexedProcessor.h>
#include <thrift/transport/TNonblockingServerTransport.h>
#include "IThriftServer.h"

namespace thriftservice {
namespace detail {

class ThriftServer : public IThriftServer {
public:
    friend class ThriftFactory;
    ThriftServer() {}

    ~ThriftServer() override;
    bool Start() override;
    bool Stop() override;
    bool RegisterProcessor(
        const std::string& name, std::shared_ptr<apache::thrift::TProcessor> processor) override;

private:
    void StartThread();

private:
    std::shared_ptr<apache::thrift::TMultiplexedProcessor> m_processor = nullptr;
    std::shared_ptr<apache::thrift::server::TServer> m_server  = nullptr;
    std::shared_ptr<apache::thrift::concurrency::Thread> m_thread   = nullptr;
    std::shared_future<bool> m_future;
};
}
}

#endif