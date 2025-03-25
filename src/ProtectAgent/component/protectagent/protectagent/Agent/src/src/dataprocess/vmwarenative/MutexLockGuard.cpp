/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file MutexLockGuard.cpp
 * @brief  Contains function declarations MutexLockGuard Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/MutexLockGuard.h"

using namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD;
using namespace AGENT_VMWARENATIVE_MUTEXLOCK;
MutexLockGuard::MutexLockGuard(MutexLock &mutex) : m_mutexlock(mutex)
{
    m_mutexlock.lock();
}
MutexLockGuard::~MutexLockGuard()
{
    m_mutexlock.unlock();
}