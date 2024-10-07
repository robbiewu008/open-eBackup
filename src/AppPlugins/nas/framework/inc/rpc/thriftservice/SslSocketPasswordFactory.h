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
#ifndef SSLSOCKETPASSWORDFACTORY_H_
#define SSLSOCKETPASSWORDFACTORY_H_

#include <thrift/transport/TSocket.h>
#include <thrift/transport/TSSLSocket.h>
#include "ICertificateHandler.h"

namespace  thriftservice {
namespace detail {
class SslSocketPasswordFactory : public apache::thrift::transport::TSSLSocketFactory {
public:
    friend class ThriftFactory;
    explicit SslSocketPasswordFactory(
        apache::thrift::transport::SSLProtocol protocol = apache::thrift::transport::TLSv1_2);
    virtual ~SslSocketPasswordFactory() = default;
    bool LoadServerCertificate();
    bool LoadClientCertificate();

protected:
    void getPassword(std::string& password, int size) override;

private:
    std::shared_ptr<certificateservice::ICertificateHandler> m_handler;
};
}
}

#endif