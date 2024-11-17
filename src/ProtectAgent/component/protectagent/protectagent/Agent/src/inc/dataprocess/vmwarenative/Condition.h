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