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