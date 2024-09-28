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
#include "message/tcp/TCPClientHandlerTest.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "message/tcp/MessageHandler.h"
#include "common/CMpThread.h"

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubTCPClientHandlerGetValueInt32Return);                                                                      \
    } while (0)

mp_int32 StubRemoveBusiClient()
{
    return MP_SUCCESS;
}

mp_int32 StubRemove()
{
    return MP_SUCCESS;
}

TEST_F(TCPClientHandlerTest, Init)
{
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CMpThread, Create), Create_fail);
    mp_int32 iRet = om.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CMpThread, Create), Create_succ);
    iRet = om.Init();
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(TCPClientHandlerTest, Connect)
{
    mp_string busiIp = "192.168";
    mp_uint16 busiPort = 1001;
    MESSAGE_ROLE role = ROLE_HOST_AGENT;
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.Connect(busiIp, busiPort, role);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(TCPClientHandlerTest, GetHBExitFlag)
{
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetHBExitFlag();
}

TEST_F(TCPClientHandlerTest, GetRecvExitFlag)
{
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetRecvExitFlag();
}

TEST_F(TCPClientHandlerTest, GetSendExitFlag)
{
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetSendExitFlag();
}

TEST_F(TCPClientHandlerTest, GetMsgHandlerExitFlag)
{
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetMsgHandlerExitFlag();
}

TEST_F(TCPClientHandlerTest, RecvMessage)
{
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CSocket, FdIsSet), StubFdIsSet);
    stub.set(ADDR(TCPClientHandler, WaitTcpEvents), StubWaitTcpEvents);
    stub.set(ADDR(TCPClientHandler, RemoveBusiClient), StubRemoveBusiClient);
    mp_int32 iRet = om.RecvMessage();
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(DppSocket, RecvMsg), StubRecvMsg);
    stub.set(ADDR(CConnection, GetLinkState), StubGetLinkState_NoLinked);
    iRet = om.RecvMessage();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(TCPClientHandlerTest, SendDppMsg)
{
    CDppMessage message;
    TCPClientHandler om;
    stub.set(ADDR(TCPClientHandler, GetConnectionByMessage), StubGetConnectionByMessage);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SendDppMsg(message);
}

TEST_F(TCPClientHandlerTest, HandleMessage)
{
    TCPClientHandler om;
    CDppMessage *c = new CDppMessage;
    c->mCmd = 1001;
    om.msgList.push_back(c);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.HandleMessage();
}

TEST_F(TCPClientHandlerTest, WaitTcpEvents)
{
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.WaitTcpEvents();
}

TEST_F(TCPClientHandlerTest, PushHBMsg)
{
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.PushHBMsg();
}

TEST_F(TCPClientHandlerTest, NewMsgPair)
{
    CDppMessage *reqMsg = new CDppMessage;
    CDppMessage *rspMsg = new CDppMessage;
    mp_uint64 seqNo = 222;
    TCPClientHandler om;

    stub.set(&CLogger::Log, StubCLoggerLog);
    om.NewMsgPair(reqMsg, rspMsg, seqNo);
}

TEST_F(TCPClientHandlerTest, GetBusiClientByIpPort)
{
    mp_string busiIp = "192.168";
    mp_uint16 busiPort = 1001;
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetBusiClientByIpPort(busiIp, busiPort);
}

TEST_F(TCPClientHandlerTest, AddBusiClient)
{
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.AddBusiClient(b);
}

TEST_F(TCPClientHandlerTest, RemoveBusiClient)
{
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(BusinessClient, Remove), StubRemove);
    om.RemoveBusiClient(b);
}

TEST_F(TCPClientHandlerTest, GetConnectionByMessage)
{
    CDppMessage message;
    BusinessClient* b = new BusinessClient;
    TCPClientHandler om;
    om.busiClients.push_back(b);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.GetConnectionByMessage(message);
}

// TEST_F(TCPClientHandlerTest, HandleRecvDppMessage)
// {
//     CDppMessage message;
//     message.mCmd = 1001;
//     message.dppMessage.body = new char[5];
//     memcpy (message.dppMessage.body, "test", 5);
//     TCPClientHandler om;
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     stub.set(&CLogger::Log, StubCLoggerLog);
//     stub.set(WipeSensitiveForJsonData, StubWipeSensitiveForJsonData_fail);
//     om.HandleRecvDppMessage(message);
// }

TEST_F(TCPClientHandlerTest, InitBusinessClients)
{
    CDppMessage message;
    message.mCmd = 1001;
    TCPClientHandler om;

    stub.set(&CLogger::Log, StubCLoggerLog);
    om.InitBusinessClients();
}

TEST_F(TCPClientHandlerTest, HandleHBAck)
{
    CDppMessage message;
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.HandleHBAck(message);
}

TEST_F(TCPClientHandlerTest, HandleTaskCompleted)
{
    CDppMessage message;
    TCPClientHandler om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.HandleTaskCompleted(message);
    EXPECT_EQ(MP_SUCCESS, iRet);
}
