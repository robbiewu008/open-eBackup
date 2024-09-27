#ifndef ITIMER_SERIVICE_H_
#define ITIMER_SERIVICE_H_
#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/timerservice/include/ITimer.h"

namespace timerservice {
class ITimerService : public servicecenter::IService {
public:
    virtual ~ITimerService() = default;
    virtual std::shared_ptr<ITimer> CreateTimer() = 0;
    virtual void DeleteTimer(std::shared_ptr<ITimer> timer) = 0;
};
}
#endif