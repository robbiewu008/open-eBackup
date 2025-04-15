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
#include "dataprocess/vmwarenative/MutexLock.h"

using namespace AGENT_VMWARENATIVE_MUTEXLOCK;

MutexLock::MutexLock()
{
    // create mutex with default attr
    pthread_mutex_init(&m_mutex, NULL);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

pthread_mutex_t *MutexLock::getPthreadMutex()
{
    return &m_mutex;
}
