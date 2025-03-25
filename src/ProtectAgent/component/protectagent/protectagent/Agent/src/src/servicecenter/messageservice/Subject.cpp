/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Subject.cpp
 * @brief  implement for ISubject
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */
#include "messageservice/detail/Subject.h"
#include <algorithm>
#include "messageservice/include/IObserver.h"
#include "messageservice/include/IEvent.h"
namespace messageservice {
namespace detail {
bool Subject::Register(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer)
{
    std::lock_guard<std::mutex> lcx(m_lock);
    if (m_observers.find(type) == m_observers.end()) {
        std::vector<std::shared_ptr<IObserver>> observers = {observer};
        m_observers.emplace(type, observers);
    } else {
        m_observers[type].emplace_back(observer);
    }
    return true;
}

bool Subject::Unregister(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer)
{
    std::lock_guard<std::mutex> lcx(m_lock);
    if (m_observers.find(type) != m_observers.end()) {
        auto it = std::find(m_observers[type].begin(), m_observers[type].end(), observer);
        if (it != m_observers[type].end()) {
            m_observers[type].erase(it);
        }
    }
    return true;
}

bool Subject::Notify(EVENT_TYPE type, const std::shared_ptr<IEvent>& event)
{
    UpdateEvent(event, type);
    std::lock_guard<std::mutex> lcx(m_lock);
    if (m_observers.find(type) != m_observers.end()) {
        std::for_each(m_observers[type].begin(), m_observers[type].end(),
            [&event](std::shared_ptr<IObserver> observer) {
                observer->Update(event);
            });
    }
    return true;
}
}
}