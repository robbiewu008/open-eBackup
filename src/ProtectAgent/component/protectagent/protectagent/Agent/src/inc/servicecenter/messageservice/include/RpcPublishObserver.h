/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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