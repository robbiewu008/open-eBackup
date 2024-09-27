#ifndef __TCP_DPP_CONNECTION_TEST_H__
#define __TCP_DPP_CONNECTION_TEST_H__

#define private public
#include "message/tcp/CConnection.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"
class CConnectionTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void CConnectionTest::SetUp() {}

void CConnectionTest::TearDown() {}

void CConnectionTest::SetUpTestCase() {}

void CConnectionTest::TearDownTestCase() {}

mp_int32 StubCConnectionGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif