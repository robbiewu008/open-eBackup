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