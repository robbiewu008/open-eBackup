#ifndef __TCP_DPP_DPPMESSAGE_TEST_H__
#define __TCP_DPP_DPPMESSAGE_TEST_H__

#define private public
#include "message/tcp/CDppMessage.h"
#include "common/ConfigXmlParse.h"
#include "common/JsonUtils.h"
#include "common/Log.h"
#include "gtest/gtest.h"
#include "stub.h"
class CDppMessageTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void CDppMessageTest::SetUp() {}

void CDppMessageTest::TearDown() {}

void CDppMessageTest::SetUpTestCase() {}

void CDppMessageTest::TearDownTestCase() {}

mp_int32 StubCDppMessageGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubAnalyzeManageMsg()
{
    return MP_SUCCESS;
}

mp_int32 StubAnalyzeManageMsgFailed()
{
    return MP_FAILED;
}

mp_int32 StubGetJsonUInt32(const Json::Value& jsValue, const mp_string& strKey, mp_uint32& iValue)
{
    return MP_FAILED;
}

mp_int32 StubGetJsonInt64(const Json::Value& jsValue, const mp_string& strKey, mp_int64& lValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetJsonString(const Json::Value& jsValue, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

#endif