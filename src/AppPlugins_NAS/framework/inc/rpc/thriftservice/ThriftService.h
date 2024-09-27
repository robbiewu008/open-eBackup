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
#ifndef THRIFTSERVICE_H
#define THRIFTSERVICE_H

#include <mutex>
#include "IThriftService.h"
#include "IThriftServer.h"
#include "IThriftClient.h"

namespace thriftservice {
namespace detail {
class ThriftService : public IThriftService {
public:
    ~ThriftService() override;

    bool Initailize() override;

    bool Uninitailize() override;

    std::shared_ptr<IThriftServer> RegisterServer(const std::string& host, int32_t port) override;

    std::shared_ptr<IThriftServer> RegisterSslServer(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler) override;

    bool UnRegisterServer(const std::string& host, int32_t port) override;

    std::shared_ptr<IThriftClient> RegisterClient(const std::string& host, int32_t port) override;

    std::shared_ptr<IThriftClient> RegisterSslClient(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler) override;

    bool UnRegisterClient(const std::string& host, int32_t port) override;

private:
    std::mutex  m_lock;
    std::map<std::pair<std::string, int32_t>, std::shared_ptr<IThriftServer>> m_servers;
    std::map<std::pair<std::string, int32_t>, std::shared_ptr<IThriftClient>> m_clients;
};
}
}
#endif