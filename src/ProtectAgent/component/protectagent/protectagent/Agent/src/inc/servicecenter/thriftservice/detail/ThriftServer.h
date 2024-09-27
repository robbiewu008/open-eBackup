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