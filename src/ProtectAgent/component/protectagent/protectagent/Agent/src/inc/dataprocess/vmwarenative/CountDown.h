/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CountDown.h
 * @brief  Contains function declarations for CountDown
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_VMWARENATIVE_COUNTDOWN_H__
#define __AGENT_VMWARENATIVE_COUNTDOWN_H__

#include "Condition.h"
#include "MutexLock.h"
#include "MutexLockGuard.h"
#include "common/Types.h"

namespace AGENT_VMWARENATIVE_COUNTDOWN {
class CountDown {
public:
    CountDown(mp_int32 n);
    ~CountDown();
    void wait();
    void Done();

private:
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock m_mutexlock;
    AGENT_VMWARENATIVE_CONDITION::Condition m_condition;
    // thread number
    int m_num;
};
}  // namespace AGENT_VMWARENATIVE_COUNTDOWN
#endif