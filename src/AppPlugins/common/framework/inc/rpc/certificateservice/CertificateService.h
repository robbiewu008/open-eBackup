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
#ifndef CRETIFICATE_SERVICE_H
#define CRETIFICATE_SERVICE_H

#include "ICertificateService.h"

namespace certificateservice {
namespace detail {
class CertificateService : public ICertificateService {
public:
    CertificateService();
    virtual ~CertificateService();
    virtual bool Initailize();
    virtual bool Uninitailize();
    std::shared_ptr<ICertificateHandler> GetCertificateHandler() override;
    // 通过代理的方式获取自定义的证书路径和证书名
    std::shared_ptr<ICertificateHandler> GetCertificateHandler(
        const std::shared_ptr<ICertificatePathProxy>& pathProxy) override;

private:
    std::shared_ptr<ICertificatePathProxy> m_pathProxy;
};
}  // namespace detail
}  // namespace certificateservice
#endif