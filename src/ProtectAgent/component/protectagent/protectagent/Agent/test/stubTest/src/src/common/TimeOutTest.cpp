#include "common/TimeOutTest.h"

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(TimeOutTest, CTimeOut)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet;
    CTimeOut work(0);
    iRet = work.Remaining();
    EXPECT_EQ(iRet, 0);

    CTimeOut work1(TIMEOUT_INFINITE);
    iRet = work1.Remaining();
    EXPECT_EQ(iRet, TIMEOUT_INFINITE);

    CTimeOut work2(1);
    DoSleep(2000);
    iRet = work2.Remaining();
    EXPECT_EQ(iRet, 0);

    CTimeOut work3(48);
    iRet = work3.Remaining();
    EXPECT_EQ(iRet, 48);
}