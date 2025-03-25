#ifndef IOBSERVER_H_
#define IOBSERVER_H_

#include <memory>

namespace messageservice {
class IEvent;
class IObserver {
public:
    virtual ~IObserver() = default;
    virtual void Update(std::shared_ptr<IEvent> event) = 0;
};
}

#endif