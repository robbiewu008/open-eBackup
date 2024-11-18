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
#include "ThriftServer.h"
#include "log/Log.h"

using namespace std;
using namespace apache::thrift::transport;
using namespace apache::thrift::server;
using namespace apache::thrift::concurrency;

namespace {
    constexpr auto MODULE = "ThriftServer";
}

namespace thriftservice {
namespace detail {
ThriftServer::~ThriftServer()
{
    INFOLOG("Thrift Server Destruct!");
    Stop();
}

bool ThriftServer::Start()
{
    DBGLOG("ThriftServer Start");
    if (!m_thread) {
        return false;
    }
    StartThread();
    m_future.wait();
    return m_future.get();
}

void ThriftServer::StartThread()
{
    if (m_thread->getState() == Thread::uninitialized) {
        try {
            HCP_Log(DEBUG, MODULE) << "ThriftServer Start Thread" << HCPENDLOG;
            m_thread->start();
        } catch (TTransportException& ex) {
            HCP_Log(ERR, MODULE) << "start thrift server failed, exception: " << ex.what() << HCPENDLOG;
        } catch (...) {
            HCP_Log(ERR, MODULE) << "start thrift server failed. Unknown exception" << HCPENDLOG;
        }
    }
}

bool ThriftServer::Stop()
{
    INFOLOG("ThriftServer Stop Thread");
    try {
        if (m_server) {
            m_server->stop();
            m_server.reset();
        }
        if (m_thread) {
            m_thread->join();
        }
    } catch (TTransportException& ex) {
        HCP_Log(ERR, MODULE) << "start thrift server failed, exception: " << ex.what() << HCPENDLOG;
        return false;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "start thrift server failed, unknown exception." << HCPENDLOG;
        return false;
    }
    return true;
}

bool ThriftServer::RegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor)
{
    if (!m_processor) {
        return false;
    }
    m_processor->registerProcessor(name, processor);
    return true;
}
}
}
