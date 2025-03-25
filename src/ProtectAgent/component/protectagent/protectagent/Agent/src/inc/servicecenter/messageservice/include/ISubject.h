/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ISubject.h
 * @brief  Base for Subject
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

#ifndef ISUBJECT_H_
#define ISUBJECT_H_

#include <memory>
#include "servicecenter/messageservice/include/IEvent.h"

namespace messageservice {
class IObserver;
class ISubject {
public:
    virtual bool Register(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer) = 0;
    virtual bool Unregister(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer) = 0;
    virtual bool Notify(EVENT_TYPE type, const std::shared_ptr<IEvent>& event) = 0;

protected:
    void UpdateEvent(std::shared_ptr<IEvent> event, EVENT_TYPE type)
    {
        event->SetEvent(type);
    }
};
}  // namespace messageservice

#endif