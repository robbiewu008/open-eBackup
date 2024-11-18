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
#include "servicecenter/services/jobservice/detail/JobServiceFactory.h"
#include "servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h"
#include "apps/appprotect/plugininterface/JobServiceHandler.h"

namespace jobservice {
std::shared_ptr<JobServiceFactory> JobServiceFactory::GetInstance()
{
    static std::shared_ptr<JobServiceFactory> g_instance = std::make_shared<JobServiceFactory>();
    return g_instance;
}

std::shared_ptr<apache::thrift::TProcessor> JobServiceFactory::CreateProcessor()
{
    auto handler = std::make_shared<JobServiceHandler>();
    auto processor =  std::make_shared<AppProtect::JobServiceProcessor>(handler);
    return processor;
}

std::shared_ptr<messageservice::RpcPublishObserver> JobServiceFactory::CreateObserver()
{
    auto observer = std::make_shared<JobServerRpcPublishObserver>();
    observer->SetProcessor(CreateProcessor());
    return observer;
}
}
