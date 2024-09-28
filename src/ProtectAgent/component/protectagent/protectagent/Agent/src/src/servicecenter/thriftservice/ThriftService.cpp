#include <thriftservice/detail/ThriftService.h>
#include <thriftservice/detail/ThriftFactory.h>
#include <servicefactory/include/ServiceFactory.h>
#include <csignal>

using namespace servicecenter;
using namespace certificateservice;
namespace thriftservice {
namespace detail {
ThriftService::~ThriftService()
{
}

// 根据thrift说明文档，使用openssl需要忽略SIGPIPE信号，否则连接中断进程收到SIGPIPE信号异常退出
bool ThriftService::Initailize()
{
#ifndef WIN32
    signal(SIGPIPE, SIG_IGN);
#endif
    return true;
}

bool ThriftService::Uninitailize()
{
    return true;
}

std::shared_ptr<IThriftServer> ThriftService::RegisterServer(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr = {host, port};
    if (m_servers.find(addr) == m_servers.end()) {
        m_servers[addr] = ThriftFactory::GetInstance()->GetServer(host, port);
    } else {
        return nullptr;
    }
    return m_servers[addr];
}

std::shared_ptr<IThriftServer> ThriftService::RegisterSslServer(const std::string& host, int32_t port,
    const std::shared_ptr<ICertificateHandler>& certificateHandler)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr = {host, port};
    if (m_servers.find(addr) == m_servers.end()) {
        m_servers[addr] = ThriftFactory::GetInstance()->GetSslServer(host, port, certificateHandler);
    } else {
        return nullptr;
    }
    return m_servers[addr];
}

bool ThriftService::UnRegisterServer(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr = {host, port};
    auto it = m_servers.find(addr);
    if (it != m_servers.end()) {
        m_servers.erase(it);
    }
    return true;
}

std::shared_ptr<IThriftClient> ThriftService::RegisterClient(const ClientSocketOpt& opt)
{
    std::lock_guard<std::mutex> lock(m_lock);
    return ThriftFactory::GetInstance()->GetClient(opt);
}

std::shared_ptr<IThriftClient> ThriftService::RegisterSslClient(const ClientSocketOpt& opt,
    const std::shared_ptr<ICertificateHandler>& certificateHandler)
{
    std::lock_guard<std::mutex> lock(m_lock);
    return ThriftFactory::GetInstance()->GetSslClient(opt, certificateHandler);
}

bool ThriftService::UnRegisterClient(const std::string& host, int32_t port)
{
    std::lock_guard<std::mutex> lock(m_lock);
    std::pair<std::string, int32_t> addr = {host, port};
    auto it = m_clients.find(addr);
    if (it != m_clients.end()) {
        m_clients.erase(it);
    }
    return true;
}
}
}
