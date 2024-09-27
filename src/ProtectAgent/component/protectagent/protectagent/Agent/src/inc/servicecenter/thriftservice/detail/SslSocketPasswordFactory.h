/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file SslSocketPasswordFactory.h
 * @brief  Implement for TSSLSocketFactory
 * @version 1.1.0
 * @date 2021-11-02
 * @author caomin 00511255
 */
#ifndef SSL_SOCKET_PASSWORD_FACTORY_H_
#define SSL_SOCKET_PASSWORD_FACTORY_H_

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include "servicecenter/certificateservice/include/ICertificateHandler.h"

using namespace apache::thrift::transport;
namespace thriftservice {
namespace detail {
class SslSocketPasswordFactory : public TSSLSocketFactory {
public:
    friend class ThriftFactory;
    SslSocketPasswordFactory(SSLProtocol protocol = TLSv1_2);
    virtual ~SslSocketPasswordFactory() = default;
    bool LoadServerCertificate();
    bool LoadClientCertificate();

protected:
    void getPassword(std::string& password, int size) override;

private:
    std::shared_ptr<certificateservice::ICertificateHandler> m_handler;
};
}  // namespace detail
}  // namespace thriftservice

#endif