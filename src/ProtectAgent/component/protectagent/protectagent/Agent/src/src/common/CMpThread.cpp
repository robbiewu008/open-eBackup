#include "common/CMpThread.h"
#include "common/CMpTime.h"
#include "common/Utils.h"
#include "common/Log.h"

namespace {
#ifdef WIN32
const mp_int32 THREAD_NUM_2048 = 2048;
#endif
const mp_int32 THREAD_NUM_1000 = 1000;
const mp_uchar THREAD_NUM_100  = 100;
}

/* ------------------------------------------------------------
Description  : 创建线程
Input        : id -- 新线程id
               proc -- 线程回调函数
               arg -- 线程参数
               uiStackSize -- 线程栈大小
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::Create(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
#ifdef WIN32
    id->handle = CreateThread(NULL, uiStackSize, proc, arg, 0, &id->os_id);
    if (id->handle == NULL) {
        return MP_FAILED;
    }
#else
    mp_int32 iRet;
    pthread_attr_t attr;

    (mp_void)pthread_attr_init(&attr);
    iRet = pthread_attr_setstacksize(&attr, (mp_int32)uiStackSize);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_attr_setstacksize failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        (mp_void) pthread_attr_destroy(&attr);
        return iRet;
    }

    iRet = pthread_create(&id->os_id, &attr, proc, arg);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_create failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        (mp_void) pthread_attr_destroy(&attr);
        return iRet;
    }

    (mp_void) pthread_attr_destroy(&attr);
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 等待线程
Input        : id -- 新线程id
               retValue -- 线程等待结果
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::WaitForEnd(thread_id_t* id, mp_void** retValue)
{
#ifdef WIN32
    if (WaitForSingleObject(id->handle, INFINITE) == WAIT_FAILED) {
        COMMLOG(OS_LOG_ERROR, "WaitForSingleObject failed");
        return MP_FAILED;
    }

    if (retValue != NULL) {
        (mp_void) GetExitCodeThread(id->handle, (LPDWORD)retValue);
    }
    (mp_void) CloseHandle(id->handle);
    id->handle = NULL;
#else
    mp_int32 iRet = pthread_join(id->os_id, retValue);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_join failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
    id->os_id = 0;
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 初始化线程锁
Input        : plock -- 线程锁
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::InitLock(thread_lock_t* plock)
{
    if (plock == NULL) {
        return MP_FAILED;
    }

#ifdef WIN32
    InitializeCriticalSection(plock);
#else
    mp_int32 iRet = pthread_mutex_init(plock, NULL);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_mutex_init failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 销毁锁
Input        : plock -- 线程锁
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::DestroyLock(thread_lock_t* plock)
{
    if (plock == NULL) {
        return MP_FAILED;
    }

#ifdef WIN32
    DeleteCriticalSection(plock);
#else
    mp_int32 iRet = pthread_mutex_destroy(plock);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_mutex_destroy failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 初始化条件变量
Input        : pcond -- 条件变量
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::InitCond(thread_cond_t& pcond)
{
#ifdef WIN32
    pcond.sem = CreateSemaphore(NULL, 0, THREAD_NUM_2048, NULL);
    pcond.count = 0;
#else
    (mp_void) pthread_mutex_init(&pcond.lock, NULL);
    (mp_void) pthread_cond_init(&pcond.cond, NULL);
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 销毁条件变量
Input        : pcond -- 条件变量
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::DestroyCond(thread_cond_t& pcond)
{
#ifdef WIN32
    (mp_void) CloseHandle(pcond.sem);
    pcond.sem = NULL;
    pcond.count = 0;
#else
    (mp_void) pthread_mutex_destroy(&pcond.lock);
    (mp_void) pthread_cond_destroy(&pcond.cond);
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取线程锁，获取不到线程锁时会阻塞
Input        : plock -- 线程锁
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::Lock(thread_lock_t* plock)
{
    if (plock == NULL) {
        return MP_FAILED;
    }

#ifdef WIN32
    EnterCriticalSection(plock);
#else
    mp_int32 iRet = pthread_mutex_lock(plock);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_mutex_lock failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 尝试获取线程锁，获取不到线程锁则立即退出
Input        : plock -- 线程锁
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
               (THREAD_LOCK_BUSY -- 线程锁被其他线程占用)
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::TryLock(thread_lock_t* plock)
{
#ifdef WIN32
    if (TryEnterCriticalSection(plock) == 0) {
        return THREAD_LOCK_BUSY;
    }

    return MP_SUCCESS;
#else
    mp_int32 iRet;

    iRet = pthread_mutex_trylock(plock);
    if (iRet == 0) {
        return MP_SUCCESS;
    } else if (iRet == EBUSY) {
        return THREAD_LOCK_BUSY;
    } else {
        COMMLOG(OS_LOG_ERROR, "pthread_mutex_trylock failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
#endif
}

/* ------------------------------------------------------------
Description  : 计算重试间隔时间
Input        : uiTimeoutInMillisecs -- 超时时间
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_uint32 CMpThread::CalcRetryInterval(mp_uint32 uiTimeoutInMillisecs)
{
    mp_uint32 result = uiTimeoutInMillisecs / THREAD_NUM_100;
    return (result < 1) ? 1 : result;
}

/* ------------------------------------------------------------
Description  : 判断是否超时
Input        : ulStartTime -- 计时的起始时间
               uiTimeoutInMillisecs -- 超时时间(单位:毫秒)
Return       : 当前线程的线程id
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_bool CMpThread::IsTimeout(mp_uint64 ulStartTime, mp_uint32 uiTimeoutInMillisecs)
{
    if ((CMpTime::GetTimeUsec() - ulStartTime) >= (mp_uint64)uiTimeoutInMillisecs * THREAD_NUM_1000) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

/* ------------------------------------------------------------
Description  : 在指定超时时间内获取线程锁
Input        : plock -- 线程锁
               uiTimeoutInMillisecs -- 超时时间(单位:毫秒)
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
               (THREAD_LOCK_BUSY -- 线程锁被其他线程占用)
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::TimedLock(thread_lock_t* plock, mp_uint32 uiTimeoutInMillisecs)
{
    mp_uint32 uiRetryInterval = CalcRetryInterval(uiTimeoutInMillisecs);
    mp_uint64 start_time = CMpTime::GetTimeUsec();

    mp_int32 result = TryLock(plock);
    while ((result != MP_SUCCESS) && !IsTimeout(start_time, uiTimeoutInMillisecs)) {
        DoSleep(uiRetryInterval);
        result = TryLock(plock);
    }

    return result;
}

/* ------------------------------------------------------------
Description  : 释放线程锁
Input        : plock -- 线程锁
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
mp_int32 CMpThread::Unlock(thread_lock_t* plock)
{
    if (plock == NULL) {
        return MP_FAILED;
    }

#ifdef WIN32
    LeaveCriticalSection(plock);
#else
    mp_int32 iRet = pthread_mutex_unlock(plock);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "pthread_mutex_lock failed, ret %d, errno[%d]:%s.", iRet, errno, strerror(errno));
        return iRet;
    }
#endif

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 获取当前线程的线程id
Return       : 当前线程的线程id
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
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

/* ------------------------------------------------------------
Description  : 线程自动锁，变量生命周期结束时由析构函数自动释放
               所持有的线程锁
Input        : pLock -- 线程锁
Create By    : yangwenjun 00275736
------------------------------------------------------------- */
CThreadAutoLock::CThreadAutoLock(thread_lock_t* pLock)
{
    m_plock = pLock;
    if (m_plock) {
        (mp_void)CMpThread::Lock(m_plock);
    }
}

CThreadAutoLock::~CThreadAutoLock()
{
    if (m_plock) {
        (mp_void)CMpThread::Unlock(m_plock);
        m_plock = NULL;
    }
}
