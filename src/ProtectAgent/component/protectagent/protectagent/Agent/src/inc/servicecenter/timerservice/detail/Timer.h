#ifndef TIMER_H_
#define TIMER_H_
#include <atomic>
#include <condition_variable>
#include <memory>
#include <vector>
#include <mutex>
#include <thread>
#include "servicecenter/timerservice/include/ITimer.h"
#include "servicecenter/timerservice/detail/TimerInfo.h"

namespace timerservice {
namespace detail {
class Timer : public ITimer {
public:
    virtual ~Timer();
    virtual bool Start();
    virtual bool Stop();
    virtual int32_t AddTimeoutExecutor(const TimeoutExecuter& func, int32_t ms);
    virtual bool RemoveTimeoutExecutor(int32_t timeId);
    void SystemTimeChange();

private:
    void TimeLoop();
    void WaitForever();
    bool StopThread();
    void HandleTimeout();
    void Sort();
    void WaitTimeout();

private:
    std::atomic<int32_t> m_timeId {0};
    bool m_stop {false};
    std::mutex m_lock;
    std::condition_variable m_cond;
    std::shared_ptr<std::thread> m_thread;
    std::vector<std::shared_ptr<TimerInfo>> m_infos;
};
}  // namespace detail
}  // namespace timerservice
#endif