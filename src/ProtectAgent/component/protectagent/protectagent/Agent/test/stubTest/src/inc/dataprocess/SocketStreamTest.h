/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
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