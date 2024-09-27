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
#ifndef _AGENT_TSF_SOCKET_CLIENT_TEST_H__
#define _AGENT_TSF_SOCKET_CLIENT_TEST_H__

#define private public
#define protected public
#include "message/tcp/DppSocket.h"
#include "common/ConfigXmlParse.h"
#include "gtest/gtest.h"
#include "stub.h"
class DppSocketTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void DppSocketTest::SetUp() {}

void DppSocketTest::TearDown() {}

void DppSocketTest::SetUpTestCase() {}

void DppSocketTest::TearDownTestCase() {}

mp_int32 StubDppSocketGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 StubWipeSensitiveForJsonData(const mp_string& rawBuffer, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubSendBuffer(mp_char *buf, CConnection &connection)
{
    return MP_SUCCESS;
}

mp_int32 StubHandleRecvDppMessage(CDppMessage &message)
{
    return MP_SUCCESS;
}

mp_int32 StubGetOSError_1()
{
    return 1;
}

mp_int32 StubGetOSError_4()
{
    return 4;
}

mp_char* StubGetOSStrErr(mp_int32 err, mp_char buf[], std::size_t buf_len)
{
    return "test";
}

mp_void StubCloseConnect(CConnection &connection)
{
    return;
}

mp_bool StubInReceivingState(mp_uint32 uiSubState)
{
    return true;
}

mp_bool StubInReceivingStateFalse(mp_uint32 uiSubState)
{
    return false;
}

mp_bool StubIsValidPrefix()
{
    return true;
}

mp_int32 StubInitMsgBody()
{
    return MP_SUCCESS;
}

mp_int32 StubInitMsgHead(mp_uint16 cmdNo, mp_uint16 flag, mp_uint64 seqNo)
{
    return MP_SUCCESS;
}

mp_int32 StubSend(mp_char *buff, mp_uint32 iBuffLen)
{
    return 0;
}

mp_int32 StubSendGt0(mp_char *buff, mp_uint32 iBuffLen)
{
    mp_int32 numFive = 5;
    return numFive;
}

LinkState StubGetLinkState()
{
    return LINK_STATE_LINKED;
}

#endif