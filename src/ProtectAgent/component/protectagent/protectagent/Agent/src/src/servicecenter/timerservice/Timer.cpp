#include <algorithm>
#include "common/Log.h"
#include "servicecenter/timerservice/detail/Timer.h"
namespace timerservice {
namespace detail {
using std::chrono::steady_clock;

Timer::~Timer()
{
    Stop();
    if (m_thread) {
        m_thread->join();
        m_thread.reset();
    }
}

bool Timer::Start()
{
    std::unique_lock<std::mutex>  lock(m_lock);
    if (!m_thread) {
        m_thread = std::make_shared<std::thread>([this]() { TimeLoop(); });
        INFOLOG("Timer obj:%x, Create thread:%d", this, m_thread->get_id());
    }
    INFOLOG("Timer obj:%x, thread:%d is exist.", this, m_thread->get_id());
    return true;
}

void Timer::SystemTimeChange()
{
    std::unique_lock<std::mutex> lock(m_lock);
    m_cond.notify_one();
}

bool Timer::Stop()
{
    std::unique_lock<std::mutex> lock(m_lock);
    if (m_thread) {
        m_stop = true;
        m_cond.notify_one();
    }
    return true;
}

void Timer::HandleTimeout()
{
    std::vector<std::shared_ptr<TimerInfo>> tmp;
    {
        std::unique_lock<std::mutex> lock(m_lock);
        tmp.swap(m_infos);
    }

    while (!tmp.empty()) {
        auto now = std::chrono::steady_clock::now();
        auto info = tmp.back();
        tmp.pop_back();
        if (info->m_timePoint <= now) {
            info->m_timePoint = std::chrono::milliseconds(info->m_ms) +
                std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
            if (!info->m_func()) {
                continue;
            }
        }
        m_infos.push_back(info);
    }
    Sort();
}

void Timer::WaitTimeout()
{
    std::unique_lock<std::mutex> lock(m_lock);
    if (!m_infos.empty()) {
        auto now = std::chrono::steady_clock::now();
        auto info = m_infos.back();
        if (info->m_timePoint > now) {
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(info->m_timePoint -
                std::chrono::time_point_cast<std::chrono::milliseconds>(now));
            m_cond.wait_for(lock, ms);
        }
    }
}

int32_t Timer::AddTimeoutExecutor(const TimeoutExecuter& func, int32_t ms)
{
    std::unique_lock<std::mutex>  lock(m_lock);
    int32_t ret = ++m_timeId;
    auto expire =  std::chrono::milliseconds(ms) +
        std::chrono::time_point_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now());
    m_infos.emplace_back(std::make_shared<TimerInfo>(expire, ret, ms, func));
    Sort();
    m_cond.notify_one();
    return ret;
}

bool Timer::RemoveTimeoutExecutor(int32_t timeId)
{
    std::unique_lock<std::mutex>  lock(m_lock);
    auto it = std::find_if(m_infos.begin(), m_infos.end(), [timeId] (const std::shared_ptr<TimerInfo>& info) {
        return info->m_timeId == timeId;
    });
    if (it != m_infos.end()) {
        m_infos.erase(it);
    }
    m_cond.notify_one();
    return true;
}

void Timer::Sort()
{
    std::sort(m_infos.begin(), m_infos.end(), [] (const std::shared_ptr<TimerInfo>& t1,
        const std::shared_ptr<TimerInfo>& t2) { return t1->m_timePoint > t2->m_timePoint; });
}

void Timer::TimeLoop()
{
    for (;;) {
        WaitForever();
        if (StopThread()) {
            INFOLOG("Timer object:%x, Exit thread:%d", this, m_thread->get_id());
            return;
        }
        HandleTimeout();
        WaitTimeout();
    }
}

void Timer::WaitForever()
{
    std::unique_lock<std::mutex>  lock(m_lock);
    m_cond.wait(lock, [this]() {  return m_stop || !m_infos.empty(); });
}

bool Timer::StopThread()
{
    std::unique_lock<std::mutex> lock(m_lock);
    return m_stop;
}
}
}
