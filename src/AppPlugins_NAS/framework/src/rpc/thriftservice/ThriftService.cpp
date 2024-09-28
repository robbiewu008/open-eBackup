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
#include "ThriftService.h"
#include <csignal>
#include "ThriftFactory.h"
#include "ServiceFactory.h"


using namespace servicecenter;
using namespace certificateservice;

namespace thriftservice {
namespace detail {
#ifndef WIN32
static bool registerSerivce = ServiceFactory::GetInstance()->Register<ThriftService>("IThriftService");
#endif

ThriftService::~ThriftService()
{
}

// 根据thrift说明文档，使用openssl需要忽略SIGPIPE信号，否则连接中断进程收到SIGPIPE信号异常退出
bool ThriftService::Initailize()
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    return true;
}

bool ThriftService::Uninitailize()
{
    return true;
}

std::shared_ptr<IThriftServer> ThriftService::RegisterServer(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    if (m_servers.find(addr) == m_servers.end()) {
        m_servers[addr] = ThriftFactory::GetInstance()->GetServer(host, port);
    } else {
        return nullptr;
    }
    return m_servers[addr];
}

std::shared_ptr<IThriftServer> ThriftService::RegisterSslServer(const std::string& host, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    if (m_servers.find(addr) == m_servers.end()) {
        m_servers[addr] = ThriftFactory::GetInstance()->GetSslServer(host, port, certificateHandler);
    } else {
        return nullptr;
    }
    return m_servers[addr];
}

bool ThriftService::UnRegisterServer(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    auto it = m_servers.find(addr);
    if (it != m_servers.end()) {
        m_servers.erase(it);
    }
    return true;
}

std::shared_ptr<IThriftClient> ThriftService::RegisterClient(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    return ThriftFactory::GetInstance()->GetClient(host, port);
}

std::shared_ptr<IThriftClient> ThriftService::RegisterSslClient(const std::string& host, int32_t port,
    std::shared_ptr<ICertificateHandler> certificateHandler)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    return ThriftFactory::GetInstance()->GetSslClient(host, port, certificateHandler);
}

bool ThriftService::UnRegisterClient(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr {host, port};
    auto it = m_clients.find(addr);
    if (it != m_clients.end()) {
        m_clients.erase(it);
    }
    return true;
}
}
}
