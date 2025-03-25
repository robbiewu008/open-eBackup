/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Condition.cpp
 * @brief  The implemention about Condition
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/Condition.h"

using namespace AGENT_VMWARENATIVE_CONDITION;
using namespace AGENT_VMWARENATIVE_MUTEXLOCK;

Condition::Condition(MutexLock &mutex) : m_mutexlock(mutex)
{
    pthread_cond_init(&m_pcond, NULL);
    m_time = {0, 0};
}

Condition::~Condition()
{
    pthread_cond_destroy(&m_pcond);
}
void Condition::wait()
{
    struct timeval now;
    gettimeofday(&now, NULL);
    int iNumNsec = 1000;
    int iNumSec = 2;
    m_time.tv_sec = now.tv_sec;
    m_time.tv_nsec = now.tv_usec * iNumNsec;
    m_time.tv_sec += iNumSec;
    pthread_cond_wait(&m_pcond, m_mutexlock.getPthreadMutex());
}

void Condition::notify()
{
    pthread_cond_signal(&m_pcond);
}
void Condition::notifyAll()
{
    pthread_cond_broadcast(&m_pcond);
}
