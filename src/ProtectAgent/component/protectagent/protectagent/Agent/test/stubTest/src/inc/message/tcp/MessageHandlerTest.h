#ifndef _AGENT_MESSAGE_HANDLER_TEST_H__
#define _AGENT_MESSAGE_HANDLER_TEST_H__

#define private public
#define protected public
#include "message/tcp/MessageHandler.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"
class MessageHandlerTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void MessageHandlerTest::SetUp() {}

void MessageHandlerTest::TearDown() {}

void MessageHandlerTest::SetUpTestCase() {}

void MessageHandlerTest::TearDownTestCase() {}

mp_int32 StubMessageHandlerGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

#endif