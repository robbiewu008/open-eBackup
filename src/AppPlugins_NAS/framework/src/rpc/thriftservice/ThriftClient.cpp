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
#include "ThriftClient.h"
#include "thrift/transport/TTransportException.h"
#include "log/Log.h"

using namespace apache::thrift::protocol;
using namespace apache::thrift::async;
using namespace apache::thrift::transport;

namespace {
    constexpr auto MODULE = "ThriftClient";
}

namespace thriftservice {
namespace detail {
ThriftClient::~ThriftClient()
{
    Stop();
}

bool ThriftClient::Start()
{
    HCP_Log(DEBUG, MODULE) << "Enter ThriftClient start" << HCPENDLOG;
    if (!m_transport) {
        return false;
    }
    try {
        m_transport->open();
    } catch (TTransportException& ex) {
        HCP_Log(ERR, MODULE) << "Start thrift client failed, exception: " << ex.what() << HCPENDLOG;
        return false;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "Start thrift client failed, Standard C++ Exception: " << ex.what() << HCPENDLOG;
        return false;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "Start thrift client failed. Unknown exception" << HCPENDLOG;
        return false;
    }
    return true;
}

bool ThriftClient::Stop()
{
    HCP_Log(DEBUG, MODULE) << "Enter ThriftClient stop" << HCPENDLOG;
    if (!m_transport) {
        return false;
    }
    try {
        if (m_transport->isOpen()) {
            m_transport->close();
        }
    } catch (TTransportException& ex) {
        HCP_Log(ERR, MODULE) << "Stop thrift client failed, exception: " <<  ex.what() << HCPENDLOG;
        return false;
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "Stop thrift client failed, Standard C++ Exception: " << ex.what() << HCPENDLOG;
        return false;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "Stop thrift client failed. Unknown exception" << HCPENDLOG;
        return false;
    }
    return true;
}

std::shared_ptr<TProtocol> ThriftClient::GetTProtocol()
{
    return m_protocol;
}

std::shared_ptr<TConcurrentClientSyncInfo> ThriftClient::GetSyncInfo()
{
    if (!m_syncInfo) {
        m_syncInfo = std::make_shared<TConcurrentClientSyncInfo>();
    }
    return m_syncInfo;
}
}
}
