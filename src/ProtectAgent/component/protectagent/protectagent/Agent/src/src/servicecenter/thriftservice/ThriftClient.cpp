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
#include <thrift/transport/TTransportException.h>
#include "common/Log.h"
#include "thriftservice/detail/ThriftClient.h"

using namespace apache::thrift::transport;
using namespace apache::thrift::protocol;
using namespace apache::thrift::async;
namespace thriftservice {
namespace detail {
ThriftClient::~ThriftClient()
{
    DBGLOG("ThriftClient Stop Enter");
    Stop();
}

bool ThriftClient::Start()
{
    DBGLOG("ThriftClient Start Enter");
    if (!m_transport) {
        return false;
    }
    try {
        m_transport->open();
    } catch (TTransportException& ex) {
        ERRLOG("Start thrift client failed, exception: %s.", ex.what());
        return false;
    } catch (const std::exception& ex) {
        ERRLOG("Start thrift client failed, Standard C++ Exception. %s", ex.what());
        return false;
    } catch (...) {
        ERRLOG("Start thrift client failed. Unknown exception.");
        return false;
    }
    return true;
}

bool ThriftClient::Stop()
{
    DBGLOG("ThriftClient Stop Enter");
    if (!m_transport) {
        return false;
    }

    try {
        if (m_transport->isOpen()) {
            m_transport->close();
        }
    } catch (TTransportException& ex) {
        ERRLOG("Stop thrift client failed, exception: %s.", ex.what());
        return false;
    } catch (const std::exception& ex) {
        ERRLOG("Stop thrift client failed, Standard C++ Exception. %s", ex.what());
        return false;
    } catch (...) {
        ERRLOG("Stop thrift client failed. Unknown exception.");
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