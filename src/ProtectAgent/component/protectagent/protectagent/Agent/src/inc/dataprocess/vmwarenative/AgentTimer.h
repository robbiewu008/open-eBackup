#ifndef AGENT_VMWARENATIVE_TIMER_H
#define AGENT_VMWARENATIVE_TIMER_H

#include <chrono>
#include "common/Types.h"

namespace AGENT_VMWARENATIVE_TIMER {
struct TimePoint {
    TimePoint();
    std::chrono::high_resolution_clock::time_point value;
};

mp_int64 operator-(const TimePoint& lhs, const TimePoint& rhs);
mp_bool operator>(const TimePoint& lhs, const TimePoint& rhs);
mp_bool operator<(const TimePoint& lhs, const TimePoint& rhs);

class Timer {
    TimePoint m_t0;

public:
    Timer();
    virtual ~Timer()
    {}
    inline mp_int64 Duration()
    {
        return TimePoint() - m_t0;
    }
};

class FixedTimer {
    TimePoint m_t0;
    TimePoint m_t1;
    bool m_measure;

public:
    FixedTimer();
    virtual ~FixedTimer()
    {}
    inline mp_int64 Duration()
    {
        if (m_measure) {
            m_measure = false;
            m_t1 = TimePoint();
        }
        return m_t1 - m_t0;
    }
};
}  // namespace AGENT_VMWARENATIVE_TIMER
#endif
