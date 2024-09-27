/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServer.cpp
 * @brief  implement for JobServer
 * @version 1.1.0
 * @date 2021-11-18
 * @author caomin 00511255
 */

#include "pluginfx/ExternalPluginManager.h"
#include "servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h"
#include "servicecenter/services/jobservice/detail/JobServiceFactory.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/services/jobservice/detail/JobServer.h"

namespace jobservice {
bool JobServer::Initailize()
{
    RegisterRpcObserver();
    return true;
}

bool JobServer::Uninitailize()
{
    return true;
}

void JobServer::RegisterRpcObserver()
{
    auto observer = JobServiceFactory::GetInstance()->CreateObserver();
    ExternalPluginManager::GetInstance().RegisterObserver(messageservice::EVENT_TYPE::RPC_PUBLISH_TYPE,
        std::dynamic_pointer_cast<messageservice::IObserver>(observer));
}
}