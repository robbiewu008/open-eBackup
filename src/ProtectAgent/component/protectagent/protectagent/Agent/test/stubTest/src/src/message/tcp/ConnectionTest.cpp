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
#include "message/tcp/ConnectionTest.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "message/tcp/CSocket.h"

namespace{
mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_void StubVoid(void *pobj)
{
    return;
}

mp_int32 StubIPV6UIntToStr(mp_void* pIpAddr, mp_string& strIpAddr)
{
    strIpAddr = "";
    return MP_FAILED;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubCConnectionGetValueInt32Return);                                                                      \
            stub.set(&CLogger::Log, StubCLoggerLog);                                                                   \
    } while (0)

TEST_F(CConnectionTest, InitMsgBody)
{
    CConnection om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.InitMsgBody();
    NEW_CATCH(om.dppMsg, CDppMessage);
    om.InitMsgBody();
    om.dppMsg = NULL;
}

TEST_F(CConnectionTest, SetIndex)
{
    mp_uint32 uiIndex = 1;
    CConnection om;
    om.SetIndex(uiIndex);
}

TEST_F(CConnectionTest, InRecvFlag)
{
    CConnection om;
    om.InRecvFlag();
}

TEST_F(CConnectionTest, InSendingState)
{
    CConnection om;
    om.InSendingState();

    om.conn->uiState = 1;
    om.InSendingState();
}

TEST_F(CConnectionTest, InReceivingState)
{
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_uint32 uiSubState = 1;
    CConnection om;
    om.InReceivingState(uiSubState);
    om.conn->uiState = 1;
    om.InReceivingState(uiSubState);
    om.conn->uiState = 0;

    uiSubState = 2;
    om.InReceivingState(uiSubState);

    uiSubState = 3;
    om.InReceivingState(uiSubState);

    om.conn->uiState = 1;
    om.InReceivingState(uiSubState);
}

TEST_F(CConnectionTest, Connect)
{
    StubClogToVoidLogNullPointReference();
    mp_int32 iRet;
    CConnection om;
    iRet = om.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    om.conn->uiClientIPaddr = 1;
    om.conn->uiClientPort = 0;
    iRet = om.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    om.conn->uiClientPort = 2000;
    om.linkState = LINK_STATE_LINKED;
    iRet = om.Connect();
    EXPECT_EQ(MP_SUCCESS, iRet);

    om.linkState = LINK_STATE_NO_LINKED;
    stub.set(ADDR(CSocket, CreateTcpSocket), StubFailed);
    iRet = om.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CSocket, CreateTcpSocket), StubSuccess);
    stub.set(ADDR(CSocket, Bind), StubSuccess);
    stub.set(ADDR(CSocket, Connect), StubFailed);
    stub.set(ADDR(CSocket, Close), StubVoid);
    om.m_listenIp = "192.168.0.1";
    om.m_isIpv4 = true;
    iRet = om.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CSocket, Bind), StubSuccess);
    stub.set(ADDR(CSocket, ConnectIpv6), StubSuccess);
    stub.set(ADDR(CSocket, SetNonBlocking), StubVoid);
    stub.set(ADDR(CConnection, SetSocket), StubVoid);
    om.m_listenIp = "";
    om.m_isIpv4 = false;
    iRet = om.Connect();
    EXPECT_EQ(MP_SUCCESS, iRet);
}
TEST_F(CConnectionTest, ResetConnection)
{
    StubClogToVoidLogNullPointReference();
    CConnection om;
    om.ResetConnection();

    om.conn->clientSock = MP_INVALID_SOCKET;
    om.m_role = SERVER;
    mp_int32 iRet = om.ResetConnection();
    EXPECT_EQ(iRet, MP_SUCCESS);
}
TEST_F(CConnectionTest, SetSocket)
{
    StubClogToVoidLogNullPointReference();
    mp_socket clientSock;
    CConnection om;
    om.SetSocket(clientSock);
}

TEST_F(CConnectionTest, SetRecvFlag)
{
    mp_bool bRecv = MP_FALSE;
    CConnection om;
    om.SetRecvFlag(bRecv);
}

TEST_F(CConnectionTest, SetSendingState)
{
    mp_bool isSending = MP_FALSE;
    CConnection om;
    om.SetSendingState(isSending);
}

TEST_F(CConnectionTest, SetReceivingState)
{
    mp_uint32 uiSubState;
    mp_bool bIsReceiving = MP_FALSE;
    CConnection om;

    uiSubState = STATE_RECV;
    om.SetReceivingState(uiSubState, bIsReceiving);
    uiSubState = STATE_RECV_PART2;
    om.SetReceivingState(uiSubState, bIsReceiving);
}

TEST_F(CConnectionTest, GetIndex)
{
    CConnection om;
    om.conn->uiIndex = 1;
    mp_uint32 uiRet = om.GetIndex();
    EXPECT_EQ(uiRet, 1);
}

TEST_F(CConnectionTest, GetClientIpAddrStr)
{
    CConnection om;
    om.m_isIpv4 = MP_FALSE;
    stub.set(ADDR(CIP, IPV6UIntToStr), StubIPV6UIntToStr);
    mp_string sRet = om.GetClientIpAddrStr();
    EXPECT_EQ(sRet, "");
}

TEST_F(CConnectionTest, GetState)
{
    CConnection om;
    om.conn->uiState = 1;
    mp_uint16 uiRet = om.GetState();
    EXPECT_EQ(uiRet, 1);
}

TEST_F(CConnectionTest, ClearAll)
{
    CConnection om;
    mp_bool bSavePendingMesgs;
    mp_int32 iRet = om.ClearAll(bSavePendingMesgs);
    EXPECT_EQ(iRet, MP_SUCCESS);
}

TEST_F(CConnectionTest, StartRecvMsg)
{
    CConnection om;
    stub.set(ADDR(CConnection, InitMsgHead), StubFailed);
    mp_int32 iRet = om.StartRecvMsg();
    EXPECT_EQ(iRet, MP_FAILED);
}

TEST_F(CConnectionTest, GetLastHBTime)
{
    CConnection om;
    om.GetLastHBTime();
}

TEST_F(CConnectionTest, Destroy)
{
    CConnection om;
    om.conn = NULL;
    om.Destroy();
}

TEST_F(CConnectionTest, SetClientIpv6Addr)
{
    CConnection om;
    mp_uint32 uiClientIpv6Addr[4];
    uint32_t uLen;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_bool bRet = om.SetClientIpv6Addr(uiClientIpv6Addr, uLen);
    EXPECT_EQ(MP_FALSE, bRet);

    uLen = 4;
    bRet = om.SetClientIpv6Addr(uiClientIpv6Addr, uLen);
    EXPECT_EQ(MP_TRUE, bRet);

    uLen = 4;
    mp_uint32* uiClientIpv6Addrs = NULL;
    bRet = om.SetClientIpv6Addr(uiClientIpv6Addrs, uLen);
    EXPECT_EQ(MP_FALSE, bRet);
}

TEST_F(CConnectionTest, Recv)
{
    CConnection om;
    mp_char *buff;
    mp_uint32 iBuffLen;
    om.m_sslLink = false;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSocket::Recv, StubFailed);
    mp_int32 iRet = om.Recv(buff, iBuffLen);
    EXPECT_EQ(MP_FAILED, iRet);

    om.m_sslLink = true;
    #ifndef SUPPORT_SSL
        iRet = om.Recv(buff, iBuffLen);
        EXPECT_EQ(MP_FAILED, iRet);
    #else
        stub.set(&CSocket::SslRecv, StubFailed);
        iRet = om.Recv(buff, iBuffLen);
    #endif
}

TEST_F(CConnectionTest, Send)
{
    CConnection om;
    mp_char *buff;
    mp_uint32 iBuffLen;
    om.m_sslLink = false;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CSocket::Send, StubFailed);
    mp_int32 iRet = om.Send(buff, iBuffLen);
    EXPECT_EQ(MP_FAILED, iRet);

    om.m_sslLink = true;
    #ifndef SUPPORT_SSL
        iRet = om.Send(buff, iBuffLen);
        EXPECT_EQ(MP_FAILED, iRet);
    #else
        stub.set(&CSocket::SslSend, StubFailed);
        iRet = om.Send(buff, iBuffLen);
    #endif
}