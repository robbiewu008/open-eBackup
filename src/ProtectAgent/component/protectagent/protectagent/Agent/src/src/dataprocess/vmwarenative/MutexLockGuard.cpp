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