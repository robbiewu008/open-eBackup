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