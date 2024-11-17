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
#ifndef __AGENT_VMWARENATIVE_MUTEXLOCK_H__
#define __AGENT_VMWARENATIVE_MUTEXLOCK_H__

#include <pthread.h>

namespace AGENT_VMWARENATIVE_MUTEXLOCK {
class MutexLock {
public:
    MutexLock(const MutexLock &) = delete;
    MutexLock &operator=(const MutexLock &) = delete;

public:
    MutexLock();
    ~MutexLock();

    void lock();
    void unlock();
    pthread_mutex_t *getPthreadMutex();

public:
    pthread_mutex_t m_mutex;
};
}  // namespace AGENT_VMWARENATIVE_MUTEXLOCK
#endif