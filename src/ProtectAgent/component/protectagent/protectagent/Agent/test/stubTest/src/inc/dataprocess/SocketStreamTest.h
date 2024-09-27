#ifndef __SOCKET_DATA_INTERFACE_TEST_H__
#define __SOCKET_DATA_INTERFACE_TEST_H__

#include "dataprocess/datareadwrite/SocketStream.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"

class SocketStreamTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void SocketStreamTest::SetUp() {}

void SocketStreamTest::TearDown() {}

void SocketStreamTest::SetUpTestCase() {}

void SocketStreamTest::TearDownTestCase() {}

mp_int32 StubSocketStreamGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_socket StubGetSockFd()
{
    return 1;
}

mp_int32 StubSendBuffer(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut) {
    return 3;
}

mp_int32 StubRecvBuffer(
    mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut, mp_uint32& iRecvLen)
{
    return MP_SUCCESS;
}

mp_int32 StubRecvBufferFail(
    mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut, mp_uint32& iRecvLen)
{
    return MP_FAILED;
}

#endif