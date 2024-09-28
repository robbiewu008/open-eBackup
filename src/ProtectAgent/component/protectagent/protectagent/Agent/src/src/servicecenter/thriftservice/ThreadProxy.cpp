#include "thrift/transport/TTransportException.h"
#include "common/Log.h"
#include "servicecenter/thriftservice/detail/ThreadProxy.h"

using namespace apache::thrift::concurrency;
using namespace apache::thrift::transport;
namespace thriftservice {
namespace detail {
void ThreadProxy::threadMainProxy(std::shared_ptr<Thread> thread)
{
    try {
        Thread::threadMain(thread);
    } catch(TTransportException ex) {
        ERRLOG("thrift server main thread TTransportException:%d, errmsg:%s", ex.getType(), ex.what());
    } catch(...) {
        ERRLOG("thrift server main thread error");
    }
    if (thread->getState() != stopping && thread->getState() != stopped) {
        thread->setState(stopping);
    }
    auto ptr = std::dynamic_pointer_cast<ThreadProxy>(thread);
    if (ptr) {
        ptr->Notify();
    }
}

void ThreadProxy::SetPromise(std::shared_ptr<std::promise<bool>> p)
{
    m_promise = p;
}

Thread::thread_funct_t ThreadProxy::getThreadFunc() const
{
    return threadMainProxy;
}

void ThreadProxy::Notify()
{
    m_promise->set_value(false);
}
}
}
