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
