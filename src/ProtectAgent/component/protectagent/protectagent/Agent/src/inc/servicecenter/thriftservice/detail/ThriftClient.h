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