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
#include "Thread.h"
#include "CTime.h"

namespace Module {

int CMpThread::Create(thread_id_t* id, thread_proc_t proc, void* arg, uint32_t uiStackSize)
{
#ifdef WIN32
    id->handle = CreateThread(NULL, uiStackSize, proc, arg, 0, &id->os_id);
    if (NULL == id->handle)
    {
        return FAILED;
    }
#else
    int iRet = SUCCESS;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    iRet = pthread_attr_setstacksize(&attr, (int)uiStackSize);
    if (0 != iRet)
    {
        (void)pthread_attr_destroy(&attr);
        return FAILED;
    }

    iRet = pthread_create(&id->os_id, &attr, proc, arg);
    if (0 != iRet)
    {
        (void)pthread_attr_destroy(&attr);
        return FAILED;
    }

    (void)pthread_attr_destroy(&attr);
#endif
    
    return SUCCESS;
}

int CMpThread::WaitForEnd(thread_id_t* id, void** retValue)
{
#ifdef WIN32
    if (WaitForSingleObject(id->handle, INFINITE) == WAIT_FAILED) {
        return FAILED;
    }

    if (NULL != retValue) {
        (void)GetExitCodeThread(id->handle, (LPDWORD)retValue);
    }
    (void)CloseHandle(id->handle);
    id->handle = NULL;
#else
    int iRet;

    iRet = pthread_join(id->os_id, retValue);
    if (iRet != 0)
    {
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CMpThread::InitLock(thread_lock_t* plock)
{
    if (NULL == plock) {
        return FAILED;
    }

#ifdef WIN32
    InitializeCriticalSection(plock);
#else
    if (0 != pthread_mutex_init(plock, NULL))
    {
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CMpThread::DestroyLock(thread_lock_t* plock)
{
    if (NULL == plock) {
        return FAILED;
    }

#ifdef WIN32
    DeleteCriticalSection(plock);
#else
    if (0 != pthread_mutex_destroy(plock)) {
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CMpThread::InitCond(thread_cond_t* pcond)
{
    if (NULL == pcond) {
        return FAILED;
    }

#ifdef WIN32
    pcond->sem = CreateSemaphore(NULL, 0, 2048, NULL);
    pcond->count = 0;
#else
    (void)pthread_mutex_init(&pcond->lock, NULL);
    (void)pthread_cond_init(&pcond->cond, NULL);
#endif

    return SUCCESS;
}

int CMpThread::DestroyCond(thread_cond_t* pcond)
{
    if (NULL == pcond) {
        return FAILED;
    }
    
#ifdef WIN32
    (void)CloseHandle(pcond->sem);
    pcond->sem = NULL;
    pcond->count = 0;
#else
    (void)pthread_mutex_destroy(&pcond->lock);
    (void)pthread_cond_destroy(&pcond->cond);
#endif

    return SUCCESS;
}

int CMpThread::Lock(thread_lock_t* plock)
{
    if (NULL == plock) {
        return FAILED;
    }

#ifdef WIN32
    InitLock(plock);
    EnterCriticalSection(plock);
#else
    if (0 != pthread_mutex_lock(plock)) {
        return FAILED;
    }
#endif

    return SUCCESS;
}

int CMpThread::TryLock(thread_lock_t* plock)
{
#ifdef WIN32
    if (0 == TryEnterCriticalSection(plock)) {
        return THREAD_LOCK_BUSY;
    }

    return SUCCESS;
#else
    int iRet = 0;

    iRet = pthread_mutex_trylock(plock);
    if (0 == iRet) {
        return SUCCESS;
    } else if (EBUSY == iRet) {
        return THREAD_LOCK_BUSY;
    } else {
        return FAILED;
    }
#endif
}

uint32_t CMpThread::CalcRetryInterval(uint32_t uiTimeoutInMillisecs)
{
    uint32_t result = uiTimeoutInMillisecs / 100;
    return result < 1 ? 1 : result;
}

bool CMpThread::IsTimeout(uint64_t ulStartTime, uint32_t uiTimeoutInMillisecs)
{
    if ((CTime::GetTimeUsec() - ulStartTime) >= (uint64_t)uiTimeoutInMillisecs * 1000)
    {
        return true;
    }

    return false;
}

int CMpThread::TimedLock(thread_lock_t* plock, uint32_t uiTimeoutInMillisecs)
{
    uint32_t uiRetryInterval = CalcRetryInterval(uiTimeoutInMillisecs);
    uint64_t start_time = CTime::GetTimeUsec();

    int result = TryLock(plock);
    while((result != SUCCESS) && !IsTimeout(start_time, uiTimeoutInMillisecs))
    {
        CTime::DoSleep(uiRetryInterval);
        result = TryLock(plock);
    }

    return result;
}

int CMpThread::Unlock(thread_lock_t* plock)
{
    if (NULL == plock) {
        return FAILED;
    }

#ifdef WIN32
    LeaveCriticalSection(plock);
    DestroyLock(plock);
#else
    if (pthread_mutex_unlock(plock) != 0) {
        return FAILED;
    }
#endif

    return SUCCESS;
}

thread_os_id_t CMpThread::GetThreadId()
{
    thread_os_id_t tid;

#ifdef WIN32
    tid = GetCurrentThreadId();
#else
    tid = pthread_self();
#endif

    return tid;
}

CThreadAutoLock::CThreadAutoLock(thread_lock_t* pLock)
{
    m_plock = pLock;
    if(m_plock)
    {
        CMpThread::Lock(m_plock);
    }
}

CThreadAutoLock::~CThreadAutoLock()
{
    if(m_plock)
    {
        CMpThread::Unlock(m_plock);
        m_plock = NULL;
    }
}

} // namespace Module