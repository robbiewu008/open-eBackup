#include "servicecenter/thriftservice/detail/ThreadFactoryProxy.h"
#include "servicecenter/thriftservice/detail/ThreadProxy.h"

using namespace apache::thrift::concurrency;
namespace thriftservice {
namespace detail {
std::shared_ptr<Thread> ThreadFactoryProxy::newThread(std::shared_ptr<Runnable> runnable) const
{
    std::shared_ptr<ThreadProxy> result = std::make_shared<ThreadProxy>(isDetached(), runnable);
    runnable->thread(result);
    return result;
}
}
}

