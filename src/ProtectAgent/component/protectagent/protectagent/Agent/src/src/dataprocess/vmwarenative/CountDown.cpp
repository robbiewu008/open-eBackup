/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file CountDown.cpp
 * @brief  Implementation of the Class CountDown
 * @version 1.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/CountDown.h"

using namespace std;
using namespace AGENT_VMWARENATIVE_COUNTDOWN;
using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;

CountDown::CountDown(int n) : m_condition(m_mutexlock), m_num(n)
{}

CountDown::~CountDown()
{}

void CountDown::wait()
{
    MutexLockGuard guardLock(m_mutexlock);
    while (m_num > 0) {
        m_condition.wait();
    }
}

void CountDown::Done()
{
    MutexLockGuard guardLock(m_mutexlock);
    --m_num;
    if (m_num == 0) {
        m_condition.notify();
    }
}