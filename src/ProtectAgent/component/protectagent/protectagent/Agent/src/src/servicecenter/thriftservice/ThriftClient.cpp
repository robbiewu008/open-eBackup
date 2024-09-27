/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.cpp
 * @brief  implement for IThriftClient
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
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