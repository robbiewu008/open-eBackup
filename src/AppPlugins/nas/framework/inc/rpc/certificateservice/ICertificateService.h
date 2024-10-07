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
#ifndef ICERTIFICATE_SERVICE_H
#define ICERTIFICATE_SERVICE_H

#include <memory>
#include "IService.h"
#include "ICertificateComm.h"
#include "ICertificateHandler.h"
#include "ICertificatePathProxy.h"

namespace certificateservice {
class ICertificateService : public servicecenter::IService {
public:
    ICertificateService() = default;
    ~ICertificateService() = default;
    // 默认证书接口，使用nignx证书,使用CPATH和CConfigXmlParser，需提前初始化好
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler() = 0;
    // 通过代理的方式获取自定义的证书路径和证书名
    virtual std::shared_ptr<ICertificateHandler> GetCertificateHandler(
        const std::shared_ptr<ICertificatePathProxy>& pathProxy) = 0;
};
}  // namespace certificateservice
#endif