#ifndef __IO_TASK_TEST_H__
#define __IO_TASK_TEST_H__

#include "dataprocess/ioscheduler/IOTask.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"
class IOTaskTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void IOTaskTest::SetUp() {}

void IOTaskTest::TearDown() {}

void IOTaskTest::SetUpTestCase() {}

void IOTaskTest::TearDownTestCase() {}

mp_int32 StubIOTaskGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif