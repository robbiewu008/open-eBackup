/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServerRpcPublishObserver.cpp
 * @brief  implement for ServiceFactory
 * @version 1.1.0
 * @date 2021-11-18
 * @author caomin 00511255
 */

#include <common/Log.h>
#include "servicecenter/services/jobservice/detail/JobServerRpcPublishObserver.h"
namespace jobservice {
void JobServerRpcPublishObserver::SetProcessor(std::shared_ptr<TProcessor> processor)
{
    m_processor = processor;
}

void JobServerRpcPublishObserver::Update(std::shared_ptr<messageservice::RpcPublishEvent> event)
{
    if (!event->GetThriftServer()->RegisterProcessor("JobService", m_processor)) {
        ERRLOG("JobService register processor failed.");
    }
}
}