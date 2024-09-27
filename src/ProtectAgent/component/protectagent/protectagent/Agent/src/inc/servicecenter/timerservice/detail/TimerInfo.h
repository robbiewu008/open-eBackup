#ifndef TIMER_INFO_H_
#define TIMER_INFO_H_
namespace timerservice {
namespace detail {
struct TimerInfo {
public:
    TimerInfo(
        std::chrono::time_point<std::chrono::steady_clock> timepoint, int32_t timeId, int32_t ms,
            const TimeoutExecuter& func)
        : m_timePoint(timepoint), m_timeId(timeId), m_ms(ms), m_func(func)
    {}
    std::chrono::time_point<std::chrono::steady_clock> m_timePoint;
    int32_t m_timeId {0};
    int32_t m_ms {0};
    TimeoutExecuter m_func;
};
}  // namespace detail
}  // namespace timerservice
#endif