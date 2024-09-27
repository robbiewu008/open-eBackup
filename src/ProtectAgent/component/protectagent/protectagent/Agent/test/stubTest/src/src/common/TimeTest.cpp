#include "common/TimeTest.h"

static mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)


TEST_F(CMpTimeTest,Now){
    mp_time pTime;
    CMpTime::Now(pTime);
}

TEST_F(CMpTimeTest,LocalTimeR){
    mp_time pTime;
    mp_tm pTm;
    
    CMpTime::LocalTimeR(pTime,pTm);
}

TEST_F(CMpTimeTest,GetTimeOfDay){
    timeval tp;
    
    CMpTime::GetTimeOfDay(tp);
}

TEST_F(CMpTimeTest,GetTimeUsec){
    
    CMpTime::GetTimeUsec();
}

TEST_F(CMpTimeTest,GetTimeSec){
    
    CMpTime::GetTimeSec();
}

TEST_F(CMpTimeTest,GetTimeString){

    mp_time pTime;
    StubClogToVoidLogNullPointReference();
    stub.set(&CMpTime::LocalTimeR, StubCMpTimeLocalTimeR);
    CMpTime::GetTimeString(pTime);
}

TEST_F(CMpTimeTest,GenSeconds){
    
    CMpTime::GenSeconds();
}
