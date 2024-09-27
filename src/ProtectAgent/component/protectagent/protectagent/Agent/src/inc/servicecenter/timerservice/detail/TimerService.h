#ifndef TIMER_SERVICE_H_
#define TIMER_SERVICE_H_
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>
#include "servicecenter/timerservice/include/ITimerService.h"
#include "servicecenter/timerservice/detail/Timer.h"

namespace timerservice {
namespace detail {
class TimerService : public ITimerService {
public:
    virtual ~TimerService()
    {
        Uninitailize();
    }
    virtual bool Initailize();
    virtual bool Uninitailize();
    virtual std::shared_ptr<ITimer> CreateTimer();
    virtual void DeleteTimer(std::shared_ptr<ITimer> timer);
private:
    void MonitorSystemTime();
    bool IsSystemTimeChange(std::chrono::time_point<std::chrono::system_clock> start,
        std::chrono::time_point<std::chrono::system_clock> end);
    void NotifyTimers();
private:
    bool m_stop{false};
    std::mutex m_lock;
    std::shared_ptr<std::thread> m_systimeWatch;
    std::vector<std::shared_ptr<Timer>> m_timers;
};
}
}
#endif