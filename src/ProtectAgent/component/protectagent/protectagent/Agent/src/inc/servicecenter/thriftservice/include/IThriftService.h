/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IThriftService.h
 * @brief  Base for thrift service
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef ITHRIFTSERVICE_H_
#define ITHRIFTSERVICE_H_

#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/TProcessor.h>
#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/thriftservice/include/IThriftClient.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"
#include "servicecenter/certificateservice/include/ICertificateHandler.h"

namespace thriftservice {
class IThriftService : public servicecenter::IService {
public:
    virtual ~IThriftService() = default;

    virtual std::shared_ptr<IThriftClient> RegisterClient(const ClientSocketOpt& opt) = 0;

    virtual std::shared_ptr<IThriftClient> RegisterSslClient(const ClientSocketOpt& opt,
        const std::shared_ptr<certificateservice::ICertificateHandler>& certificateHandler) = 0;

    virtual bool UnRegisterClient(const std::string& host, int32_t port) = 0;

    // 可能存在多张网卡下绑定特定网卡的情况
    virtual std::shared_ptr<IThriftServer> RegisterServer(const std::string& host, int32_t port) = 0;

    virtual std::shared_ptr<IThriftServer> RegisterSslServer(const std::string& host, int32_t port,
        const std::shared_ptr<certificateservice::ICertificateHandler>& certificateHandler) = 0;

    virtual bool UnRegisterServer(const std::string& host, int32_t port) = 0;
};
}

#endif