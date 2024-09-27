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
