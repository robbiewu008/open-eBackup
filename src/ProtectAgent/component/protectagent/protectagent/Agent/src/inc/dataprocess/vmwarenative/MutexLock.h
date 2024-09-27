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