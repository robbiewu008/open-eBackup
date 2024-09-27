/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.cpp
 * @brief  implement for ThreadFactoryProxy
 * @version 1.1.0
 * @date 2021-12-14
 * @author caomin 00511255
 */

#ifndef THREAD_FACTORY_PROXY_H_
#define THREAD_FACTORY_PROXY_H_
#include "thrift/concurrency/ThreadFactory.h"

namespace thriftservice {
namespace detail {
class ThreadFactoryProxy : public apache::thrift::concurrency::ThreadFactory {
public:
    using ThreadFactory::ThreadFactory;
    std::shared_ptr<apache::thrift::concurrency::Thread> newThread(
        std::shared_ptr<apache::thrift::concurrency::Runnable> runnable) const override;
};
}  // namespace detail
}  // namespace thriftservice
#endif