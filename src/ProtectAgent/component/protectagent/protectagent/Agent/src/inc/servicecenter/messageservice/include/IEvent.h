/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file IEvent.h
 * @brief  Base for Message Service Event Data
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
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