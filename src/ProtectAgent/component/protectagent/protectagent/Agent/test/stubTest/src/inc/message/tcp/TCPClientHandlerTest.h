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
#include "message/tcp/TCPClientHandler.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"
class TCPClientHandlerTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void TCPClientHandlerTest::SetUp() {}

void TCPClientHandlerTest::TearDown() {}

void TCPClientHandlerTest::SetUpTestCase() {}

void TCPClientHandlerTest::TearDownTestCase() {}

mp_int32 StubTCPClientHandlerGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_int32 Create_succ(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}

mp_int32 Create_fail(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_FAILED;
}


mp_bool StubGetMsgHandlerExitFlag()
{
    return false;
}

mp_bool StubGetSendExitFlag()
{
    return false;
}

mp_int32 StubPopRspMsg(message_pair_t& msgPair)
{
    return MP_SUCCESS;
}

mp_int32 StubWaitTcpEvents(mp_void* pThis)
{
    return MP_SUCCESS;
}

mp_bool StubFdIsSet(mp_socket sock, fd_set& fdset)
{
    return MP_TRUE;
}

mp_int32 StubRecvMsg(CConnection &connection)
{
    return MP_SUCCESS;
}

LinkState StubGetLinkState_NoLinked()
{
    return LINK_STATE_NO_LINKED;
}

CConnection *StubGetConnectionByMessage(CDppMessage &message)
{
    CConnection* c = new CConnection;
    return c;
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_int32 StubWipeSensitiveForJsonData_fail(const mp_string& rawBuffer, mp_string& strValue)
{
    return MP_FAILED;
}

#endif