
#ifndef __JOB_QOS_MANAGER_TEST_H__
#define __JOB_QOS_MANAGER_TEST_H__
#define private public
#include "dataprocess/jobqosmanager/JobQosManager.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"
class JobQosManagerTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void JobQosManagerTest::SetUp() {}

void JobQosManagerTest::TearDown() {}

void JobQosManagerTest::SetUpTestCase() {}

void JobQosManagerTest::TearDownTestCase() {}
mp_int32 StubFileJobQosManagerGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif