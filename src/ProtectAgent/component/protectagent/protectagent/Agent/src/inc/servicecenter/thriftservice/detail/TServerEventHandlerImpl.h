/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ThriftClient.cpp
 * @brief  implement for ThreadProxy
 * @version 1.1.0
 * @date 2021-12-14
 * @author caomin 00511255
 */

#ifndef TSERVER_EVENT_HANDLE_IMPL_H_
#define TSERVER_EVENT_HANDLE_IMPL_H_
#include <future>
#include "thrift/server/TServer.h"

namespace thriftservice {
namespace detail {
class TServerEventHandlerImpl : public apache::thrift::server::TServerEventHandler {
public:
    using TServerEventHandler::TServerEventHandler;
    void preServe() override;
    void SetPromise(std::shared_ptr<std::promise<bool>> p);

private:
    std::shared_ptr<std::promise<bool>> m_promise;
};
}  // namespace detail
}  // namespace thriftservice
#endif