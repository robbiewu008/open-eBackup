#ifndef __BUSINIESS_CONNECTION_TEST_H__
#define __BUSINIESS_CONNECTION_TEST_H__

#define private public

#include "message/tcp/BusinessClient.h"
#include <vector>
#include <algorithm>
#include "stub.h"
#include "gtest/gtest.h"
#include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "common/Log.h"

class BusinessClientTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
    Stub stub;
};

void BusinessClientTest::SetUp() {}

void BusinessClientTest::TearDown() {}

void BusinessClientTest::SetUpTestCase() {}

void BusinessClientTest::TearDownTestCase() {}

mp_int32 StubBusinessClientGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif