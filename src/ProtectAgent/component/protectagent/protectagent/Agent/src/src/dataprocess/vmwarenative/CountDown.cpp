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