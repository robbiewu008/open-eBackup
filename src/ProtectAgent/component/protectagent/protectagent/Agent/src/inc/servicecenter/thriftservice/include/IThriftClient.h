#ifndef ITHRIFTCLIENT_H_
#define ITHRIFTCLIENT_H_

#include <memory>
#include <thrift/protocol/TProtocol.h>
#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/async/TConcurrentClientSyncInfo.h>

namespace thriftservice {
constexpr uint32_t PLUGIN_CONNECT_MIN_COUNT = 1;
constexpr uint32_t PLUGIN_CONNECT_MAX_COUNT = 60;
constexpr uint32_t PLUGIN_CONNECT_DEFAULT_COUNT = 10;
constexpr uint32_t THRIFT_TIMEOUT_MIN = 2 * 60 * 1000;
constexpr uint32_t THRIFT_TIMEOUT_MAX = 10 * 60 * 1000;
constexpr uint32_t THRIFT_TIMEOUT_DEFAULT = 5 * 60 * 1000;

constexpr uint32_t THRIFT_CLIENT_TIMEOUT_MIN = 30 * 1000;
constexpr uint32_t THRIFT_CLIENT_TIMEOUT_MAX = 6 * 60 * 60 * 1000;
constexpr uint32_t THRIFT_CLIENT_TIMEOUT_DEFAULT = 6 * 60 * 1000;

constexpr uint32_t THRIFT_MSG_TYPE_COMMON = 0;
constexpr uint32_t THRIFT_MSG_TYPE_HEARTBEAT = 1;

struct ClientSocketOpt {
    std::string host;
    int32_t port = 0;
    uint32_t connTimeout = THRIFT_TIMEOUT_DEFAULT;
    uint32_t recvTimeout = THRIFT_TIMEOUT_DEFAULT;
    uint32_t sendTimeout = THRIFT_TIMEOUT_DEFAULT;
    bool bKeepAlive = true;
    int32_t msgType = 0;       // type 0: common msg, 1: heartbeat
};
class IThriftClient {
public:
    virtual ~IThriftClient(){};

    template<typename T>
    std::shared_ptr<T> GetClientIf(const std::string& service)
    {
        return std::make_shared<T>(
            std::make_shared<apache::thrift::protocol::TMultiplexedProtocol>(GetTProtocol(), service));
    }
    template<typename T>
    std::shared_ptr<T> GetConcurrentClientIf(const std::string& service)
    {
        return std::make_shared<T>(
            std::make_shared<apache::thrift::protocol::TMultiplexedProtocol>(GetTProtocol(), service), GetSyncInfo());
    }

    virtual bool Start() = 0;
    virtual bool Stop() = 0;

protected:
    virtual std::shared_ptr<apache::thrift::protocol::TProtocol> GetTProtocol() = 0;
    virtual std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> GetSyncInfo() = 0;
};
}  // namespace thriftservice

#endif