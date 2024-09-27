#ifndef ITHRIFTSERVER_H_
#define ITHRIFTSERVER_H_

#include <thrift/TProcessor.h>

using namespace apache::thrift;

namespace thriftservice {
class IThriftServer {
public:
    virtual ~IThriftServer(){};
    virtual bool Start() = 0;
    virtual bool Stop() = 0;
    virtual bool RegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor) = 0;
};
}
#endif