#ifndef THREAD_PROXY_H_
#define THREAD_PROXY_H_
#include <future>
#include "thrift/concurrency/Thread.h"
namespace thriftservice {
namespace detail {
class ThreadProxy : public apache::thrift::concurrency::Thread {
public:
    using Thread::Thread;
    static void threadMainProxy(std::shared_ptr<Thread> thread);
    void SetPromise(std::shared_ptr<std::promise<bool>> p);
    void Notify();

protected:
    virtual apache::thrift::concurrency::Thread::thread_funct_t getThreadFunc() const override;
    std::shared_ptr<std::promise<bool>> m_promise;
};
}  // namespace detail
}  // namespace thriftservice
#endif