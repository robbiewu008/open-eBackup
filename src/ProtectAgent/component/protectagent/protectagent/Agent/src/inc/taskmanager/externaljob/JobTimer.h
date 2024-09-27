#ifndef INTERVAL_TIMER_H
#define INTERVAL_TIMER_H
#include <chrono>

class JobTimer {
public:
    JobTimer(int interval): m_interval(interval) {}
    void StopTimer();
    void StartTimer();
    bool IsOverInterval();

private:
    bool m_startFlag = false;
    int m_interval;
    std::chrono::steady_clock::time_point m_startTime;
};
#endif