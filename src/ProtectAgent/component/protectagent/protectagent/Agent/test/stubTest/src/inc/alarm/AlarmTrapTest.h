#ifndef __AGENT_ALARM_TRAP_TEST_H__
#define __AGENT_ALARM_TRAP_TEST_H__

#define private public
#define protected public

#include "gtest/gtest.h"
#include "stub.h"

class CAlarmTrapTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub m_stub;
};

void CAlarmTrapTest::SetUp() {}

void CAlarmTrapTest::TearDown() {}

void CAlarmTrapTest::SetUpTestCase() {}

void CAlarmTrapTest::TearDownTestCase() {}

#endif
