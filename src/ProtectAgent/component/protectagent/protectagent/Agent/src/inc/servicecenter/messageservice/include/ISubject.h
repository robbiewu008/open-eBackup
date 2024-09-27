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