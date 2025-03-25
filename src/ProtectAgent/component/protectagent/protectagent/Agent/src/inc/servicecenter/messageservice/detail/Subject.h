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
#ifndef SUBJECT_H_
#define SUBJECT_H_

#include <map>
#include <vector>
#include <mutex>
#include "servicecenter/messageservice/include/ISubject.h"

namespace messageservice {
namespace detail {
class Subject : public ISubject {
public:
    virtual bool Register(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer);

    virtual bool Unregister(EVENT_TYPE type, const std::shared_ptr<IObserver>& observer);

    virtual bool Notify(EVENT_TYPE type, const std::shared_ptr<IEvent>& event);
private:
    std::map<EVENT_TYPE, std::vector<std::shared_ptr<IObserver>>> m_observers;
    std::mutex m_lock;
};
}
}

#endif