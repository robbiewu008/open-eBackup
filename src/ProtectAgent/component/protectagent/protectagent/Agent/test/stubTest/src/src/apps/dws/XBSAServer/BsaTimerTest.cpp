#include "apps/dws/XBSAServer/BsaTimerTest.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include <event2/event.h>
namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}
#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)
mp_int32 StubReturnResultSetFailed(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_void ReadLevelAndCountTest()
{
}
mp_int32 CreateTest(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_void CTimerThreadLoopAA(struct event_base &base)
{
    return;
}
}

TEST_F(CTimerTest, InitTest)
{
    // stub.set((XMLElement* (XMLElement::*)(const char* name))ADDR(XMLElement, FirstChildElement), StubFirstChildElement);
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubReturnResultSetFailed);
    stub.set(ADDR(CLogger, ReadLevelAndCount), ReadLevelAndCountTest);
    stub.set(ADDR(CMpThread, Create), CreateTest);
    DoGetJsonStringTest();
    CTimer::GetInstance().Init();
}

TEST_F(CTimerTest, TimerOutCbTest)
{
    DoGetJsonStringTest();
    mp_uint32 handle = 1;
    CTimer::GetInstance().TimerOutCb(handle);
    handle = 1;
    CTimer::GetInstance().TimerOutCb(handle);
    Timer timer;
    CTimer::GetInstance().m_timerMap[1] = timer;
    CTimer::GetInstance().TimerOutCb(handle);
}

TEST_F(CTimerTest, StartTimerTest)
{
    DoGetJsonStringTest();
    mp_uint32 timeout;
    void (*timeoutCb)(mp_uint64 , void *);
    mp_uint64 param1;
    void *param2;
    CTimer::GetInstance().StartTimer(timeout, timeoutCb, param1, param2);
}

TEST_F(CTimerTest, StopTimerTest)
{

    DoGetJsonStringTest();
    mp_uint32 handle;
    CTimer::GetInstance().StopTimer(handle);
    Timer timer;
    CTimer::GetInstance().m_timerMap[1] = timer;
    CTimer::GetInstance().StopTimer(handle);
}

TEST_F(CTimerTest, ResetTimerTest)
{
    DoGetJsonStringTest();
    mp_uint32 handle;
    CTimer::GetInstance().ResetTimer(handle);
    Timer timer;
    CTimer::GetInstance().m_timerMap[1] = timer;
    CTimer::GetInstance().ResetTimer(handle);
}

TEST_F(CTimerTest, AdjustTimerTest)
{
    DoGetJsonStringTest();
    mp_uint32 handle;
    mp_uint32 newTimeout;
    CTimer::GetInstance().AdjustTimer(handle, newTimeout);
    Timer timer;
    CTimer::GetInstance().m_timerMap[1] = timer;
    CTimer::GetInstance().AdjustTimer(handle, newTimeout);
}

TEST_F(CTimerTest, CTimerThreadLoopTest)
{
    DoGetJsonStringTest();
    // struct event_base baset;
    // CTimer::GetInstance().CTimerThreadLoop(baset);
}

TEST_F(CTimerTest, CTimerThreadTest)
{
    DoGetJsonStringTest();
    CTimer::GetInstance().CTimerThread(nullptr);
    stub.set(ADDR(CTimer, CTimerThreadLoop), CTimerThreadLoopAA);
    CTimer::GetInstance().CTimerThread(&CTimer::m_instance);
    stub.reset(ADDR(CTimer, CTimerThreadLoop));
    //CTimer::m_instance.m_base = NULL;
    //CTimer::GetInstance().CTimerThread(&CTimer::m_instance);
}