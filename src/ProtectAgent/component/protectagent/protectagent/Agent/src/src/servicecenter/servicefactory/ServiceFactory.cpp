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
#include "servicefactory/include/ServiceFactory.h"
#include "servicefactory/detail/ServiceCenter.h"

#include "servicecenter/timerservice/detail/TimerService.h"
#include "servicecenter/thriftservice/detail/ThriftService.h"
#include "servicecenter/services/jobservice/detail/JobServer.h"
#include "servicecenter/messageservice/detail/MessageService.h"
#include "servicecenter/certificateservice/detail/CertificateService.h"
#include "apps/appprotect/plugininterface/ShareResourceHandler.h"
#include "apps/appprotect/plugininterface/SecurityServiceHandler.h"
#include "apps/appprotect/plugininterface/PluginRegisterHandler.h"

namespace servicecenter {
using namespace detail;

ServiceFactory* ServiceFactory::GetInstance()
{
    static bool g_centerInitailize = ServiceFactory::GetServiceCenter()->Initailize();
    if (g_centerInitailize) {
        IServiceCenter::GetInstance()->Register("ITimerService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<timerservice::detail::TimerService>());
        });
        IServiceCenter::GetInstance()->Register("IThriftService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<thriftservice::detail::ThriftService>());
        });
        IServiceCenter::GetInstance()->Register("IJobServer", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<jobservice::JobServer>());
        });
        IServiceCenter::GetInstance()->Register("IMessageService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<messageservice::detail::MessageService>());
        });
        IServiceCenter::GetInstance()->Register("ICertificateService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(
                std::make_shared<certificateservice::detail::CertificateService>());
        });
        IServiceCenter::GetInstance()->Register("ShareResourceService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<AppProtect::ShareResourceHandler>());
        });
        IServiceCenter::GetInstance()->Register("SecurityService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<AppProtect::SecurityServiceHandler>());
        });
        IServiceCenter::GetInstance()->Register("PluginRegisterService", []() -> std::shared_ptr<IService> {
            return std::dynamic_pointer_cast<IService>(std::make_shared<AppProtect::PluginRegisterHandler>());
        });
    }
    static ServiceFactory g_instance;
    return &g_instance;
}

ServiceFactory::ServiceFactory()
{
}

std::shared_ptr<IServiceCenter> ServiceFactory::GetServiceCenter()
{
    return std::make_shared<ServiceCenter>();
}
}