/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.cpp
 * @brief  implement for ThreadFactoryProxy
 * @version 1.1.0
 * @date 2021-12-14
 * @author caomin 00511255
 */

#include "servicecenter/thriftservice/detail/ThreadFactoryProxy.h"
#include "servicecenter/thriftservice/detail/ThreadProxy.h"

using namespace apache::thrift::concurrency;
namespace thriftservice {
namespace detail {
std::shared_ptr<Thread> ThreadFactoryProxy::newThread(std::shared_ptr<Runnable> runnable) const
{
    std::shared_ptr<ThreadProxy> result = std::make_shared<ThreadProxy>(isDetached(), runnable);
    runnable->thread(result);
    return result;
}
}
}

