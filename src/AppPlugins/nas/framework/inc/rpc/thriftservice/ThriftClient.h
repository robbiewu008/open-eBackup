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
#ifndef THRIFTCLIENT_H
#define THRIFTCLIENT_H

#include "IThriftClient.h"

namespace thriftservice {
namespace detail {
class ThriftClient : public IThriftClient {
public:
    friend class ThriftFactory;
    ~ThriftClient() override;
    bool Start() override;
    bool Stop() override;

protected:
    std::shared_ptr<apache::thrift::protocol::TProtocol> GetTProtocol() override;
    std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> GetSyncInfo() override;

private:
    std::shared_ptr<apache::thrift::transport::TTransport> m_transport;
    std::shared_ptr<apache::thrift::protocol::TProtocol> m_protocol;
    std::shared_ptr<apache::thrift::async::TConcurrentClientSyncInfo> m_syncInfo;
};
}
}

#endif