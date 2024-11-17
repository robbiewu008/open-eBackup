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
#ifndef ITHRIFTSERVICE_H
#define ITHRIFTSERVICE_H

#include <thrift/protocol/TMultiplexedProtocol.h>
#include <thrift/TProcessor.h>
#include "servicefactory/IService.h"
#include "IThriftClient.h"
#include "IThriftServer.h"
#include "ICertificateHandler.h"

namespace thriftservice {
class IThriftService : public servicecenter::IService {
public:
    virtual ~IThriftService() = default;

    virtual std::shared_ptr<IThriftClient> RegisterClient(const std::string& host, int32_t port) = 0;

    virtual std::shared_ptr<IThriftClient> RegisterSslClient(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler) = 0;

    virtual bool UnRegisterClient(const std::string& host, int32_t port) = 0;

    // 可能存在多张网卡下绑定特定网卡的情况
    virtual std::shared_ptr<IThriftServer> RegisterServer(const std::string& host, int32_t port) = 0;

    virtual std::shared_ptr<IThriftServer> RegisterSslServer(const std::string& host, int32_t port,
        std::shared_ptr<certificateservice::ICertificateHandler> certificateHandler) = 0;

    virtual bool UnRegisterServer(const std::string& host, int32_t port) = 0;
};
}

#endif