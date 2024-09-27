/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * @file RpcPublishObserver.h
 * @brief  The implemention about RpcPublishObserver.h
 * @version 1.0.0.0
 * @date 2021-11-17
 * @author jwx966562
 */
#ifndef _RPC_PUBLISH_OBSERVER_H
#define _RPC_PUBLISH_OBSERVER_H

#include "common/Types.h"
#include "servicecenter/messageservice/include/IObserver.h"
#include "servicecenter/messageservice/include/IEvent.h"
#include "servicecenter/thriftservice/include/IThriftServer.h"

namespace messageservice {

class RpcPublishEvent : public messageservice::IEvent {
public:
    RpcPublishEvent(std::shared_ptr<thriftservice::IThriftServer> thriftServer) : m_thriftServer(thriftServer)
    {}
    virtual ~RpcPublishEvent()
    {}
    const std::shared_ptr<thriftservice::IThriftServer> GetThriftServer()
    {
        return m_thriftServer;
    }

private:
    std::shared_ptr<thriftservice::IThriftServer> m_thriftServer;  // 需要发布的thrift服务端
};

class RpcPublishObserver : public messageservice::IObserver {
public:
    virtual ~RpcPublishObserver()
    {}
    void Update(std::shared_ptr<messageservice::IEvent> event) override final;

protected:
    virtual void Update(std::shared_ptr<RpcPublishEvent> event) = 0;

protected:
    mp_string m_processorName;
    std::shared_ptr<TProcessor> m_processor;
};
}  // namespace messageservice

#endif