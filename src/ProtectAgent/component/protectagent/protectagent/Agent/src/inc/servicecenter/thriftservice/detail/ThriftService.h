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