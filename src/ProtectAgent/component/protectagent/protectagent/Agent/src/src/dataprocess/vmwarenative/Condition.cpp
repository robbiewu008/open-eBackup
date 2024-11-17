/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
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
