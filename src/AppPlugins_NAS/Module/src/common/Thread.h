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
#ifndef MODULE_THREAD_H
#define MODULE_THREAD_H

#include <stdlib.h>
#include <chrono>
#include <thread>
#ifndef WIN32
#include <pthread.h>
#else
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#endif
 
#include "define/Defines.h"

namespace Module {

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

#ifdef WIN32
/*
 * Sleep Win32 API, not depend on system clock, but may introduce compatible issue for thread created by std::thread
 * Also, Sleep cannot guarantee accuracy for windows is not a real-time operating system
 */
template<typename _Rep, typename _Period>
inline void Win32Sleep(const std::chrono::duration<_Rep, _Period>& duration)
{
	DWORD durationMills = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
	::Sleep(durationMills);
}
#endif

template<typename _Rep, typename _Period>
inline void SleepFor(const std::chrono::duration<_Rep, _Period>& duration)
{
#ifdef WIN32
    Win32Sleep(duration);
#else
    std::this_thread::sleep_for(duration);
#endif
}

#ifdef WIN32
    typedef CRITICAL_SECTION thread_lock_t;
    typedef uint32_t         thread_local_var_t;
    typedef DWORD            thread_os_id_t;   
#else
    typedef pthread_mutex_t  thread_lock_t;
    typedef pthread_key_t    thread_local_var_t;
    typedef pthread_t        thread_os_id_t;
#endif

#ifdef WIN32
#define CM_INFINITE INFINITE
#else
#define CM_INFINITE 0xFFFFFFFF
#endif

typedef struct tag_thread_cond
{
#ifdef WIN32
    HANDLE              sem;
    uint32_t            count;
#else
    pthread_mutex_t     lock;
    pthread_cond_t      cond;
#endif
} thread_cond_t;

typedef struct tag_thread_id
{
    thread_os_id_t  os_id;
#ifdef WIN32
    HANDLE          handle;
#endif
}thread_id_t;

#ifdef WIN32
typedef LPTHREAD_START_ROUTINE thread_proc_t;
#else
typedef void* (*thread_proc_t)(void* arg);
#endif

#define DEFAULE_THREAD_STACK_SIZE      (512 * 1024)
#define THREAD_LOCK_BUSY               -2

enum THREAD_STATUS_E
{
    THREAD_STATUS_IDLE = 0,
    THREAD_STATUS_RUNNING,
    THREAD_STATUS_EXITED
};

class AGENT_API CMpThread
{
public:
    static int Create(thread_id_t* id, thread_proc_t proc, void* arg, uint32_t uiStackSize = DEFAULE_THREAD_STACK_SIZE);
    static int WaitForEnd(thread_id_t* id, void** retValue);
    static int InitLock(thread_lock_t* plock);
    static int DestroyLock(thread_lock_t* plock);
    static int InitCond(thread_cond_t* pcond);
    static int DestroyCond(thread_cond_t* pcond);
    static int Lock(thread_lock_t* plock);
    static int TryLock(thread_lock_t* plock);
    static int TimedLock(thread_lock_t* plock, uint32_t uiTimeoutInMillisecs);
    static int Unlock(thread_lock_t* plock);
    static thread_os_id_t GetThreadId();

private:
    static uint32_t CalcRetryInterval(uint32_t uiTimeoutInMillisecs);
    static bool IsTimeout(uint64_t ulStartTime, uint32_t uiTimeoutInMillisecs);
};

class AGENT_API CThreadAutoLock
{
public:
    CThreadAutoLock(thread_lock_t* pLock);
    ~CThreadAutoLock();

private:
    thread_lock_t* m_plock;
};

} // namespace Module

#endif //__AGENT_THREAD_H__