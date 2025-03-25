/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Condition.h
 * @brief  The implemention about Condition
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_VMWARENATIVECONDITION_H__
#define __AGENT_VMWARENATIVECONDITION_H__

#include <semaphore.h>
#include <sys/time.h>
#include "MutexLock.h"

namespace AGENT_VMWARENATIVE_CONDITION {
class Condition {
public:
    Condition(const Condition &) = delete;
    Condition &operator=(const Condition &) = delete;

public:
    Condition(AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock &mutex);
    ~Condition();
    void wait();
    void notify();
    void notifyAll();

private:
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock &m_mutexlock;
    // condition var used for multi-threads sync
    pthread_cond_t m_pcond;
    struct timespec m_time;
};
}  // namespace AGENT_VMWARENATIVE_CONDITION
#endif