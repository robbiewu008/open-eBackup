#ifndef THREAD_FACTORY_PROXY_H_
#define THREAD_FACTORY_PROXY_H_
#include "thrift/concurrency/ThreadFactory.h"

namespace thriftservice {
namespace detail {
class ThreadFactoryProxy : public apache::thrift::concurrency::ThreadFactory {
public:
    using ThreadFactory::ThreadFactory;
    std::shared_ptr<apache::thrift::concurrency::Thread> newThread(
        std::shared_ptr<apache::thrift::concurrency::Runnable> runnable) const override;
};
}  // namespace detail
}  // namespace thriftservice
#endif