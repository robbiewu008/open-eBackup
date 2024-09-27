#ifndef __AGENT_ALARM_TEST_H__
#define __AGENT_ALARM_TEST_H__

#define private public

#include "alarm/alarmdb.h"
#include "gtest/gtest.h"
#include "stub.h"

class CMpAlarmTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub_cmp;
};

void CMpAlarmTest::SetUp() {}

void CMpAlarmTest::TearDown() {}

void CMpAlarmTest::SetUpTestCase() {}

void CMpAlarmTest::TearDownTestCase() {}

class CAlarmDBTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void CAlarmDBTest::SetUp() {}

void CAlarmDBTest::TearDown() {}

void CAlarmDBTest::SetUpTestCase() {}

void CAlarmDBTest::TearDownTestCase() {}

mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif
