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
#ifndef AGENT_THREAD_H
#define AGENT_THREAD_H

#include <stdlib.h>
#include <chrono>
#include <thread>
#ifdef WIN32
#else
#include <pthread.h>
#endif
#include "common/Defines.h"

#ifdef WIN32
typedef CRITICAL_SECTION thread_lock_t;
typedef mp_uint32 thread_local_var_t;
typedef DWORD thread_os_id_t;
#else
typedef pthread_mutex_t thread_lock_t;
typedef pthread_key_t thread_local_var_t;
typedef pthread_t thread_os_id_t;
#endif

#ifdef WIN32
#define CM_INFINITE INFINITE
#define CMPTHREAD_RETURN return 0
#else
static const mp_uint32 CM_INFINITE = 0xFFFFFFFF;
#define CMPTHREAD_RETURN return NULL
#endif

typedef struct tag_thread_cond {
#ifdef WIN32
    HANDLE sem;
    mp_uint32 count;
#else
    pthread_mutex_t lock;
    pthread_cond_t cond;
#endif
} thread_cond_t;

typedef struct tag_thread_id {
    thread_os_id_t os_id;
#ifdef WIN32
    HANDLE handle;
#endif
} thread_id_t;

#ifdef WIN32
typedef LPTHREAD_START_ROUTINE thread_proc_t;
#else
typedef mp_void* (*thread_proc_t)(mp_void* arg);
#endif

#define DEFAULE_THREAD_STACK_SIZE (512 * 1024)
static const mp_uchar THREAD_LOCK_BUSY = -2;

enum THREAD_STATUS_E { THREAD_STATUS_IDLE = 0, THREAD_STATUS_RUNNING, THREAD_STATUS_EXITED };

class AGENT_API CMpThread {
public:
    static mp_int32 Create(
        thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize = DEFAULE_THREAD_STACK_SIZE);
    static mp_int32 WaitForEnd(thread_id_t* id, mp_void** retValue);
    static mp_int32 InitLock(thread_lock_t* plock);
    static mp_int32 DestroyLock(thread_lock_t* plock);
    static mp_int32 InitCond(thread_cond_t& pcond);
    static mp_int32 DestroyCond(thread_cond_t& pcond);
    static mp_int32 Lock(thread_lock_t* plock);
    static mp_int32 TryLock(thread_lock_t* plock);
    static mp_int32 TimedLock(thread_lock_t* plock, mp_uint32 uiTimeoutInMillisecs);
    static mp_int32 Unlock(thread_lock_t* plock);
    static thread_os_id_t GetThreadId();

private:
    static mp_uint32 CalcRetryInterval(mp_uint32 uiTimeoutInMillisecs);
    static mp_bool IsTimeout(mp_uint64 ulStartTime, mp_uint32 uiTimeoutInMillisecs);
};

class AGENT_API CThreadAutoLock {
public:
    CThreadAutoLock(thread_lock_t* pLock);
    ~CThreadAutoLock();

private:
    thread_lock_t* m_plock;
};

/* provide some sleep function implementation here */

/*
 * used to fix the bugs of some compiler that sleep call depend on system clock
 * using Spinlock to block thread, may cause frequent thread context switching and introduce performance issue
 */
template<typename _Rep, typename _Period>
inline void SteadySleep(const std::chrono::duration<_Rep, _Period>& duration)
{
    auto start = std::chrono::steady_clock::now();
    /*
     * although steady_clock with sleep_util is recommended in C++ standard library doc,
     * a few STL still used system_clock in implementation.
     */
    while (std::chrono::steady_clock::now() < start + duration) {
        std::this_thread::yield();
    }
}

template<typename _Rep, typename _Period>
inline void SleepFor(const std::chrono::duration<_Rep, _Period>& duration)
{
#ifdef WIN32
    SteadySleep(duration);
#else
    std::this_thread::sleep_for(duration);
#endif
}

inline void SleepForMS(int64_t ms)
{
#ifdef WIN32
    Sleep(ms);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
#endif
}

#endif  // __AGENT_THREAD_H__
