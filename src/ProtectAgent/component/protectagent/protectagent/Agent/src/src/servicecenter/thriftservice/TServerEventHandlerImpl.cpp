/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.cpp
 * @brief  implement for TServerEventHandlerImpl
 * @version 1.1.0
 * @date 2021-12-14
 * @author caomin 00511255
 */

#include "servicecenter/thriftservice/detail/TServerEventHandlerImpl.h"

namespace thriftservice {
namespace detail {
void TServerEventHandlerImpl::preServe()
{
    m_promise->set_value(true);
}

void TServerEventHandlerImpl::SetPromise(std::shared_ptr<std::promise<bool>> p)
{
    m_promise = p;
}
}
}