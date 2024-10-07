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
#include "ServiceFactory.h"
#include "ServiceCenter.h"
#ifdef WIN32
#include "ThriftService.h"
#include "CertificateService.h"
#endif

namespace servicecenter {
using namespace detail;

ServiceFactory* ServiceFactory::GetInstance()
{
    static ServiceFactory instance;
    static bool centerInitailize = ServiceFactory::GetServiceCenter()->Initailize();
#ifdef WIN32
    if (centerInitailize) {
        IServiceCenter::GetInstance()->Register("IThriftService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<thriftservice::detail::ThriftService>());
        });
        IServiceCenter::GetInstance()->Register("ICertificateService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(
                std::make_shared<certificateservice::detail::CertificateService>());
        });
    }
#endif
    return &instance;
}

ServiceFactory::ServiceFactory()
{
}

std::shared_ptr<IServiceCenter> ServiceFactory::GetServiceCenter()
{
    return std::make_shared<ServiceCenter>();
}
}
