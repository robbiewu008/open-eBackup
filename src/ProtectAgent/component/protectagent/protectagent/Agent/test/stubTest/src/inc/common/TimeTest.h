#ifndef _TIMETEST_H_
#define _TIMETEST_H_

#include "common/CMpTime.h"
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCMpTimeLogVoid(mp_void* pthis);

class CMpTimeTest: public testing::Test{
public:
    Stub stub;
};

mp_tm* StubCMpTimeLocalTimeR(mp_time& pTime, mp_tm& pTm){
    return NULL;
}

mp_void StubCMpTimeLogVoid(mp_void* pthis){
    return;
}
#endif
