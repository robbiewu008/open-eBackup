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
#ifndef __AGENT_TSF_SOCKET_TEST_H__
#define __AGENT_TSF_SOCKET_TEST_H__

#define private public
#define protected public
#include "message/tcp/CSocket.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"
class CSocketTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void CSocketTest::SetUp() {}

void CSocketTest::TearDown() {}

void CSocketTest::SetUpTestCase() {}

void CSocketTest::TearDownTestCase() {}

mp_int32 StubCSocketGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSignalRegisterFailed(mp_int32 signo, signal_proc func)
{
    return MP_FAILED;
}

mp_int32 StubSocketInvalid()
{
    return MP_INVALID_SOCKET;
}

mp_int32 StubSignalRegister(mp_int32 signo, signal_proc func)
{
    return MP_SUCCESS;
}

mp_int32 StubWaitSendEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    mp_int32 numTwo = 2;
    return numTwo;
}

mp_int32 StubWaitRecvEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    mp_int32 numTwo = 2;
    return numTwo;
}

mp_bool StubIsIPV4(const mp_string& strIpAddr)
{
    return true;
}

mp_bool StubIsIPV4False(const mp_string& strIpAddr)
{
    return false;
}

mp_bool StubIsIPV6(const std::string& ip)
{
    return true;
}

mp_bool StubIsIPV6False(const std::string& ip)
{
    return false;
}

#endif