/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.h
 * @brief  Implement for thrift client
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef THRIFTCLIENT_H_
#define THRIFTCLIENT_H_

#include "servicecenter/thriftservice/include/IThriftClient.h"

namespace thriftservice {
namespace detail {
class ThriftClient : public IThriftClient {
public:
    friend class ThriftFactory;
    virtual ~ThriftClient();
    virtual bool Start();
    virtual bool Stop();

protected:
    virtual std::shared_ptr<apache::thrift::protocol::TProtocol> GetTProtocol();
    virtual std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> GetSyncInfo();
private:
    std::shared_ptr<apache::thrift::transport::TTransport> m_transport;
    std::shared_ptr<apache::thrift::protocol::TProtocol> m_protocol;
    std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> m_syncInfo;
};
}
}

#endif