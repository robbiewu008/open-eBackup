/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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