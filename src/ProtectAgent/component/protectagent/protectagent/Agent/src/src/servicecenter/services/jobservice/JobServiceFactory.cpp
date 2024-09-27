/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServiceHandler.cpp
 * @brief  implement for ServiceFactory
 * @version 1.1.0
 * @date 2021-11-18
 * @author caomin 00511255
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
