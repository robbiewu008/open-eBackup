#ifndef BSA_TIMER_H
#define BSA_TIMER_H
#include <map>
#include <mutex>
#include <event2/event.h>
#include "common/Types.h"
#include "common/CMpThread.h"

typedef void (*timeoutCb)(mp_uint64, void *);

typedef struct tagTimer {
    mp_uint32 *phandle;
    mp_uint32 timeout;
    struct event *pevtimer;
    timeoutCb pfunc;
    mp_uint64 param1;
    void *param2;
}Timer;

class CTimer {
public:
    static CTimer& GetInstance()
    {
        return m_instance;
    };
    ~CTimer()
    {
        if (threadId.os_id != 0) {
            CMpThread::WaitForEnd(&threadId, NULL);
        }
        m_timerMap.clear();
    };

    mp_uint32 Init();
    mp_void TimerOutCb(mp_uint32 handle);

    mp_uint32 StartTimer(mp_uint32 timeout, timeoutCb func, mp_uint64 param1, void *param2);
    mp_int32 StopTimer(mp_uint32 handle);
    mp_int32 ResetTimer(mp_uint32 handle);
    mp_int32 AdjustTimer(mp_uint32 handle, mp_uint32 newTimeout);

private:
    CTimer() : m_base(NULL) {};
    mp_uint32 NewHandle();
    static mp_void *CTimerThread(void *arg);
    static mp_void CTimerThreadLoop(struct event_base &base);

    static CTimer m_instance;
    thread_id_t threadId;
    struct event_base *m_base;
    std::mutex m_mutex;
    std::map<mp_uint32, Timer> m_timerMap;
};


#endif // _BSA_TIMER_H_