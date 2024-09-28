#include "taskmanager/externaljob/JobTimer.h"

void JobTimer::StopTimer()
{
    m_startFlag = false;
}

void JobTimer::StartTimer()
{
    m_startFlag = true;
    m_startTime = std::chrono::steady_clock::now();
}

bool JobTimer::IsOverInterval()
{
    if (!m_startFlag) {
        StartTimer();
        return false;
    }
    std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - m_startTime);
    if (duration.count() > m_interval) {
        m_startTime = std::chrono::steady_clock::now();
        return true;
    }
    return false;
}
