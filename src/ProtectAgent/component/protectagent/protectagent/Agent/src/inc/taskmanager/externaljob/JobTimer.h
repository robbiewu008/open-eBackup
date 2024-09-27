/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file JobTimer.h
 * @brief Job timer
 * @version 1.1.0
 * @date 2022-2-23
 */

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