/* *
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @Description a timer implementation based on libevent.
 * @Create 2021-06-09
 * @Author wuchuan wwx563312
 */
#include "apps/dws/XBSAServer/CTimer.h"

#include <event2/event.h>
#include <event2/thread.h>
#include "common/Log.h"

CTimer CTimer::m_instance;

mp_uint32 CTimer::NewHandle()
{
    static mp_uint32 handle = 1;
    if (++handle == 0) {
        handle = 1;
    }
    return handle;
}

static inline mp_void Ms2Timeval(uint32_t timeInMs, struct timeval &tval)
{
    const uint32_t ratio = 1000;
    tval.tv_sec  = timeInMs / ratio;
    tval.tv_usec = (timeInMs % ratio) * ratio;
}

static void TimeOutCb(int fd, short what, void *arg)
{
    (void)fd;
    if ((what & EV_TIMEOUT)) {
        mp_uint32 *pHandle = (mp_uint32 *)arg;
        CTimer::GetInstance().TimerOutCb(*pHandle);
    }
}

static void LoopTimeOutCb(int fd, short what, void *arg)
{
    (void)fd;
    const mp_uint32 period = 600;
    // do nothing.
    if ((what & EV_TIMEOUT)) {
        static mp_uint32 count = 1;
        if (count++ % period == 0) {
            INFOLOG("CTimer loopTimer timeout");
        }
    }
}

mp_void CTimer::TimerOutCb(mp_uint32 handle)
{
    mp_uint64 param1;
    void *param2 = NULL;
    timeoutCb func = NULL;

    INFOLOG("timer timeout,handle=%lu", handle);

    if (handle != 0) {
        std::lock_guard<std::mutex> lock(m_mutex);
        auto iter = m_timerMap.find(handle);
        if (iter == m_timerMap.end()) {
            ERRLOG("timer not found!handle=%lu", handle);
            return;
        }
        param1 = iter->second.param1;
        param2 = iter->second.param2;
        func = iter->second.pfunc;
        INFOLOG("timer timeout,func=%p, param1=%llu,param2=%p", func, param1, param2);
    }

    if (func != NULL) { // m_mutex锁的范围不能覆盖回调，否则可能导致死锁
        func(param1, param2);
    }
}

mp_void CTimer::CTimerThreadLoop(struct event_base &base)
{
    // 启动一个空任务循环定时器，否则无法dispatch
    // 空循环定时器的超时时间不能设置太长，否则其他定时器可能偏差较大
    struct event *loopTimer = event_new(&base, -1, EV_TIMEOUT | EV_PERSIST, LoopTimeOutCb, NULL);
    if (loopTimer == NULL) {
        ERRLOG("event_new loopTimer fail.");
        return;
    }

    struct timeval tv = {10, 0};
    int ret = event_add(loopTimer, &tv);
    if (ret != 0) {
        ERRLOG("event_add loopTimer fail,ret=%d.", ret);
        event_free(loopTimer);
        return;
    }

    INFOLOG("CTimer thread start dispatch!");

    ret = event_base_dispatch(&base);
    if (ret != 0) {
        ERRLOG("event_base_dispatch fail,ret=%d.", ret);
    }
}

mp_void *CTimer::CTimerThread(void *arg)
{
    CTimer *pThis = (CTimer *)arg;
    if (pThis == NULL) {
        ERRLOG("pThis is NULL.");
        return NULL;
    }
#ifndef AIX
    INFOLOG("Into CTimer libevent thread!");
    auto ret = evthread_use_pthreads();
    if (ret != MP_SUCCESS) {
        ERRLOG("Evthread_use_pthreads init fail code is %d.", ret);
        return NULL;
    }
#endif
    pThis->m_base = event_base_new();
    if (pThis->m_base == NULL) {
        ERRLOG("event_base_new fail.");
        return NULL;
    }

    CTimerThreadLoop(*(pThis->m_base));
    return NULL;
}

mp_uint32 CTimer::Init()
{
    (mp_void) memset_s(&threadId, sizeof(threadId), 0, sizeof(threadId));
    mp_int32 ret = CMpThread::Create(&threadId, CTimerThread, this);
    if (ret != MP_SUCCESS) {
        ERRLOG("Creat Ctimer thread failed, ret=%d.", ret);
        return MP_FAILED;
    }

    INFOLOG("Init CTimer thread success!");
    return MP_SUCCESS;
}

mp_uint32 CTimer::StartTimer(mp_uint32 timeout, timeoutCb func, mp_uint64 param1, void *param2)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    mp_uint32 *pHandle = (mp_uint32 *)malloc(sizeof(mp_uint32));
    if (pHandle == NULL) {
        ERRLOG("start timer MALLOC fail!");
        return 0;
    }
    *pHandle = NewHandle();

    Timer timer;
    timer.phandle = pHandle;
    timer.timeout = timeout;
    timer.pfunc = func;
    timer.param1 = param1;
    timer.param2 = param2;
    timer.pevtimer = evtimer_new(m_base, TimeOutCb, (void *)pHandle);
    if (timer.pevtimer == NULL) {
        ERRLOG("start timer new timer fail!");
        free(pHandle);
        return 0;
    }

    struct timeval delay;
    Ms2Timeval(timeout, delay);
    if (evtimer_add(timer.pevtimer, &delay) != 0) {
        ERRLOG("start timer add timer fail!");
        free(pHandle);
        return 0;
    }

    m_timerMap.emplace(std::pair<mp_uint32, Timer>(*pHandle, timer));

    INFOLOG("start timer success!handle=%lu", *pHandle);
    return *pHandle;
}

mp_int32 CTimer::StopTimer(mp_uint32 handle)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto iter = m_timerMap.find(handle);
    if (iter == m_timerMap.end()) {
        ERRLOG("timer not found!handle=%lu", handle);
        return MP_FAILED;
    }

    event_del(iter->second.pevtimer);
    event_free(iter->second.pevtimer);
    iter->second.pevtimer = NULL;

    free(iter->second.phandle);
    iter->second.phandle = NULL;

    m_timerMap.erase(iter);

    INFOLOG("stop  timer success!handle=%lu", handle);
    return MP_SUCCESS;
}

mp_int32 CTimer::ResetTimer(mp_uint32 handle)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto iter = m_timerMap.find(handle);
    if (iter == m_timerMap.end()) {
        ERRLOG("timer not found!handle=%lu", handle);
        return MP_FAILED;
    }

    struct timeval delay;
    Ms2Timeval(iter->second.timeout, delay);
    if (evtimer_add(iter->second.pevtimer, &delay) != 0) {
        ERRLOG("reset timer add timer fail!handle=%lu", handle);
        return MP_FAILED;
    }

    DBGLOG("reset timer success!handle=%lu", handle);
    return MP_SUCCESS;
}

mp_int32 CTimer::AdjustTimer(mp_uint32 handle, mp_uint32 newTimeout)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto iter = m_timerMap.find(handle);
    if (iter == m_timerMap.end()) {
        ERRLOG("timer not found!handle=%lu", handle);
        return MP_FAILED;
    }

    struct timeval delay;
    Ms2Timeval(newTimeout, delay);
    if (evtimer_add(iter->second.pevtimer, &delay) != 0) {
        ERRLOG("adjust timer add timer fail!handle=%lu", handle);
        return MP_FAILED;
    }

    INFOLOG("adjust timer success!handle=%lu", handle);
    return MP_SUCCESS;
}