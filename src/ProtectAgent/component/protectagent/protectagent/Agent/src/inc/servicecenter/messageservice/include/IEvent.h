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
#ifndef IEVENT_H_
#define IEVENT_H_

namespace messageservice {
enum class EVENT_TYPE {
    NULL_TYPE,
    RPC_PUBLISH_TYPE
};

class IEvent {
public:
    friend class ISubject;
    virtual ~IEvent() = default;
    virtual EVENT_TYPE GetEvent()
    {
        return m_type;
    }

private:
    void SetEvent(EVENT_TYPE type)
    {
        m_type = type;
    }

private:
    EVENT_TYPE m_type {EVENT_TYPE::NULL_TYPE};
};
}  // namespace messageservice

#endif