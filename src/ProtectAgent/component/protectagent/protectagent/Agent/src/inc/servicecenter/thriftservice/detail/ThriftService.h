/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftServer.h
 * @brief  implement for thrift service
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef THRIFTSERVICE_H_
#define THRIFTSERVICE_H_

#include <mutex>
#include "servicecenter/thriftservice/include/IThriftService.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"
#include "servicecenter/thriftservice/include/IThriftClient.h"

namespace thriftservice {
namespace detail {
class ThriftService : public IThriftService {
public:
    virtual ~ThriftService();

    virtual bool Initailize();

    virtual bool Uninitailize();

    virtual std::shared_ptr<IThriftServer> RegisterServer(const std::string& host, int32_t port);

    virtual std::shared_ptr<IThriftServer> RegisterSslServer(const std::string& host, int32_t port,
        const std::shared_ptr<certificateservice::ICertificateHandler>& certificateHandler);

    virtual bool UnRegisterServer(const std::string& host, int32_t port);

    virtual std::shared_ptr<IThriftClient> RegisterClient(const ClientSocketOpt& opt);

    virtual std::shared_ptr<IThriftClient> RegisterSslClient(const ClientSocketOpt& opt,
        const std::shared_ptr<certificateservice::ICertificateHandler>& certificateHandler);

    virtual bool UnRegisterClient(const std::string& host, int32_t port);

private:
    std::mutex  m_lock;
    std::map<std::pair<std::string, int32_t>, std::shared_ptr<IThriftServer>> m_servers;
    std::map<std::pair<std::string, int32_t>, std::shared_ptr<IThriftClient>> m_clients;
};
}
}
#endif