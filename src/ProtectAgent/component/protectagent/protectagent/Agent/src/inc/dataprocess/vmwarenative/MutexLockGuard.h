/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file MutexLockGuard.h
 * @brief  Contains function declarations MutexLockGuard Operations
 * @version 1.0.0
 * @date 2020-04-02
 * @author wangguitao 00510599
 */
#ifndef __AGENT_VMWARENATIVE_MUTEXLOCKGUARD_H__
#define __AGENT_VMWARENATIVE_MUTEXLOCKGUARD_H__

#include "MutexLock.h"

namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD {
class MutexLockGuard {
public:
    MutexLockGuard(const MutexLockGuard &) = delete;
    MutexLockGuard &operator=(const MutexLockGuard &) = delete;

public:
    MutexLockGuard(AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock &mutex);
    ~MutexLockGuard();

private:
    AGENT_VMWARENATIVE_MUTEXLOCK::MutexLock &m_mutexlock;
};
}  // namespace AGENT_VMWARENATIVE_MUTEXLOCKGUARD
#endif
