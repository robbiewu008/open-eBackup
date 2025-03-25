/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobServerRpcPublishObserver.h
 * @brief  Factory for Service Center
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */
#ifndef JOB_SERVER_RPC_PUBLISH_OBSERVER_H_
#define JOB_SERVER_RPC_PUBLISH_OBSERVER_H_
#include <servicecenter/messageservice/include/RpcPublishObserver.h>

namespace jobservice {
class JobServerRpcPublishObserver : public messageservice::RpcPublishObserver {
public:
    JobServerRpcPublishObserver(){};
    virtual ~JobServerRpcPublishObserver()
    {}
    void SetProcessor(std::shared_ptr<TProcessor> processor);

protected:
    void Update(std::shared_ptr<messageservice::RpcPublishEvent> event);
};
}  // namespace jobservice
#endif