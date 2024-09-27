#include "thriftservice/detail/ThriftServer.h"
#include "common/Log.h"

namespace thriftservice {
namespace detail {
ThriftServer::~ThriftServer()
{
    Stop();
}

bool ThriftServer::Start()
{
    DBGLOG("ThriftServer Start Enter");
    if (!m_thread) {
        return false;
    }
    StartThread();
    m_future.wait();
    return m_future.get();
}

void ThriftServer::StartThread()
{
    if (m_thread->getState() == Thread::uninitialized) {
        try {
            DBGLOG("ThriftServer Start Thread");
            m_thread->start();
        } catch (TTransportException& ex) {
            ERRLOG("start thrift server failed, exception: %s.", ex.what());
        } catch (...) {
            ERRLOG("start thrift server failed. Unknown exception.");
        }
    }
}

bool ThriftServer::Stop()
{
    DBGLOG("ThriftServer Stop Thread");
    try {
        if (m_server) {
            m_server->stop();
            m_server.reset();
        }
        if (m_thread) {
            m_thread->join();
        }
    } catch (TTransportException& ex) {
            ERRLOG("start thrift server failed, exception: %s.", ex.what());
            return false;
    } catch (...) {
        ERRLOG("start thrift server failed. Unknown exception.");
        return false;
    }

    return true;
}

bool ThriftServer::RegisterProcessor(const std::string& name, std::shared_ptr<TProcessor> processor)
{
    if (!m_processor) {
        ERRLOG("Register processor failed for %s.", name.c_str());
        return false;
    }
    m_processor->registerProcessor(name, processor);
    INFOLOG("Register processor sucess for %s", name.c_str());
    return true;
}
}
}
