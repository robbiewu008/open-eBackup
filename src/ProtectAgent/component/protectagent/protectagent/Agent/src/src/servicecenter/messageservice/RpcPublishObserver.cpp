/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
* @file RpcPublishObserver.cpp
* @brief  The implemention about RpcPublishObserver.cpp
* @version 1.0.0.0
* @date 2021-11-17
* @author jwx966562
*/
#include "servicecenter/messageservice/include/RpcPublishObserver.h"
#include "common/Log.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"

namespace messageservice {
void RpcPublishObserver::Update(std::shared_ptr<messageservice::IEvent> event)
{
    std::shared_ptr<RpcPublishEvent> rpcPubEvent(std::dynamic_pointer_cast<RpcPublishEvent>(event));
    if (rpcPubEvent.get() == nullptr) {
        ERRLOG("RpcPublishObserver received a null event.");
        return;
    }
    Update(rpcPubEvent);
}
}