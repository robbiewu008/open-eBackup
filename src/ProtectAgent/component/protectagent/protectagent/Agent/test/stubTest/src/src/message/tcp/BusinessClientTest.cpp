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
#include "message/tcp/BusinessClientTest.h"
#include "message/tcp/MessageHandler.h"
#include "common/DB.h"
#include "common/Ip.h"
#include "common/Utils.h"

#ifdef SUPPORT_SSL
#include "openssl/ssl.h"
#include "openssl/asn1.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#endif

#include "openssl/ssl.h"
#include "openssl/asn1.h"
#include "openssl/bio.h"
#include "openssl/err.h"

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubBusinessClientGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

mp_uint32 GetClientIpAddrTest()
{
    return true;
}

mp_uint32 GetClientPortTest()
{
    return 8888;
}

mp_int32 CheckHostLinkStatusTest(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}

mp_int32 CreateTcpSocketTest(mp_socket& sock, mp_bool keepSocketInherit, mp_bool isIPV4)
{
    return MP_SUCCESS;
}

mp_int32 ConnectTest(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort)
{
    return MP_SUCCESS;
}

mp_int32 SslConnectTest(const mp_socket& sock)
{
    return MP_SUCCESS;
}

mp_int32 StartRecvMsgTest()
{
    return MP_SUCCESS;
}

mp_bool CheckHBTimeOutTest()
{
    return true;
}

mp_bool CheckHBTimeOutFalseTest()
{
    return false;
}

mp_bool StubTrue()
{
    return true;
}

mp_bool StubFalse()
{
    return false;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_uint32 StubGetClientIpAddr()
{
    return 1;
}

mp_uint32 StubGetClientPort()
{
    return 2000;
}

bool StubGetNginxListenIPFalse(mp_string& strIP)
{
    return MP_FALSE;
}

bool StubGetNginxListenIPTrue(mp_string& strIP, mp_int32& nPort)
{
    strIP = "";
    return true;
}

mp_int32 StubCheckHostLinkStatusFailed(const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_FAILED;
}

mp_int32 StubCheckHostLinkStatusSuccess(const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}

mp_string StubGetLisetenIPEmpty()
{
    return "";
}

mp_string StubGetLisetenIPNotEmpty()
{
    return "2000";
}

LinkState GetLinkStateTest()
{
    return LINK_STATE_LINKED;
}

LinkState GetLinkStateNoTest()
{
    return LINK_STATE_NO_LINKED;
}
mp_bool StubGetSendMsgFlagFalse()
{
    return MP_FALSE;
}

mp_bool StubGetSendMsgFlagTrue()
{
    return MP_TRUE;
}

mp_void StubVoid(void *pobj)
{
    return;
}

TEST_F(BusinessClientTest, InitTest)
{
    BusinessClient client;
    mp_string serverIp;
    mp_uint16 serverPort = 0;
    MESSAGE_ROLE role;
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
#ifdef SUPPORT_SSL
    SSL_CTX* psslCtx;
    client.Init(serverIp, serverPort, role, psslCtx);
    serverPort = 2000;
    client.Init(serverIp, serverPort, role, psslCtx);
    stub.set(ADDR(CIP, IsIPV4), StubFalse);
    stub.set(ADDR(CIP, IsIPv6), StubTrue);
    stub.set(ADDR(CIP, IPV6StrToUInt), StubSuccess);
    client.Init(serverIp, serverPort, role, psslCtx);
    stub.set(ADDR(CIP, IsIPv6), StubFalse);
    client.Init(serverIp, serverPort, role, psslCtx);
#else
    client.Init(serverIp, serverPort, role);
    serverPort = 2000;
    serverIp = "127.0.0.1";
    client.Init(serverIp, serverPort, role);
    stub.set(ADDR(CIP, IsIPV4), StubFalse);
    stub.set(ADDR(CIP, IsIPv6), StubTrue);
    stub.set(ADDR(CIP, IPV6StrToUInt), StubSuccess);
    client.Init(serverIp, serverPort, role);
    stub.set(ADDR(CIP, IsIPv6), StubFalse);
    client.Init(serverIp, serverPort, role);
#endif
}

TEST_F(BusinessClientTest, GetConnectionTest)
{
    BusinessClient client;
    client.GetConnection();
}

TEST_F(BusinessClientTest, GetRoleTest)
{
    BusinessClient client;
    client.GetRole();
}

TEST_F(BusinessClientTest, GetSeqNoTest)
{
    BusinessClient client;
    client.GetSeqNo();
}

TEST_F(BusinessClientTest, GetClientSocketTest)
{
    BusinessClient client;
    client.GetClientSocket();
}

TEST_F(BusinessClientTest, ConnectTestaa)
{
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    BusinessClient client;
    client.Connect();
    stub.set(ADDR(CConnection, GetClientIpAddr), GetClientIpAddrTest);
    stub.set(ADDR(CConnection, GetClientPort), GetClientPortTest);
    stub.set(ADDR(CSocket, CheckHostLinkStatus), CheckHostLinkStatusTest);
    stub.set(ADDR(CSocket, CreateTcpSocket), CreateTcpSocketTest);
    client.Connect();
    stub.set(ADDR(CSocket, Connect), ConnectTest);
    client.Connect();
#ifdef SUPPORT_SSL
    stub.set(ADDR(BusinessClient, SslConnect), SslConnectTest);
#endif
    client.Connect();
    stub.set(ADDR(CConnection, StartRecvMsg), StartRecvMsgTest);
    client.Connect();
}

TEST_F(BusinessClientTest, ConnectTest)
{
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    BusinessClient client;
    stub.set(ADDR(BusinessClient, GetLisetenIP), StubGetLisetenIPEmpty);
    mp_int32 iRet = client.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(BusinessClient, GetLisetenIP), StubGetLisetenIPNotEmpty);
    stub.set(ADDR(CConnection, Connect), StubFailed);
    iRet = client.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CConnection, Connect), StubSuccess);
    stub.set(ADDR(CConnection, StartRecvMsg), StubFailed);
    iRet = client.Connect();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CConnection, StartRecvMsg), StubSuccess);
    iRet = client.Connect();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

// TEST_F(BusinessClientTest, SslConnect)
// {
//     BusinessClient client;
//     mp_socket sock;
//     client.SslConnect(sock);
// }

// TEST_F(BusinessClientTest, CreateSslConnect)
// {
//     BusinessClient client;
//     mp_socket sock;
//     client.CreateSslConnect(sock);
// }

mp_void DisConnectTest()
{
    return;
}

mp_void DelBusiClientFromDBTest()
{
    return;
}

mp_string GetClientIpAddrStrTest()
{
    return "192.168.1.1";
}

mp_int32 QueryTableTest(
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    return MP_SUCCESS;
}

mp_int32 QueryTableTest2(
    const mp_string& strSql, DbParamStream& dps, DBReader& readBuff, mp_int32& iRowCount, mp_int32& iColCount)
{
    mp_string text = "a";
    iRowCount = 1;
    readBuff << text;
    return MP_SUCCESS;
}

mp_int32 ExecSqlTest(const mp_string& strSql, DbParamStream& dps)
{
    return MP_SUCCESS;
}

mp_int32 ExecSqlFailedTest(const mp_string& strSql, DbParamStream& dps)
{
    return MP_FAILED;
}

TEST_F(BusinessClientTest, DisConnectTest)
{
    BusinessClient client;
    stub.set(ADDR(CConnection, DisConnect), DisConnectTest);
    client.DisConnect();
}

TEST_F(BusinessClientTest, RemoveTest)
{
    BusinessClient client;
    stub.set(ADDR(CConnection, DisConnect), DisConnectTest);
    stub.set(ADDR(BusinessClient, DelBusiClientFromDB), DelBusiClientFromDBTest);
    client.Remove();
}

TEST_F(BusinessClientTest, HeartBeatTest)
{
    BusinessClient client;
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CConnection, CheckHBTimeOut), CheckHBTimeOutTest);
    stub.set(ADDR(CConnection, GetLinkState), GetLinkStateTest);
    stub.set(ADDR(CConnection, GetClientIpAddrStr), GetClientIpAddrStrTest);
    stub.set(ADDR(CConnection, GetClientPort), GetClientPortTest);
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    client.HeartBeat();

    stub.set(ADDR(CConnection, CheckHBTimeOut), CheckHBTimeOutFalseTest);
    stub.set(ADDR(CConnection, GetLinkState), GetLinkStateTest);
    stub.set(ADDR(CConnection, GetSendMsgFlag), StubGetSendMsgFlagFalse);
    stub.set(ADDR(BusinessClient, Connect), StubSuccess);
    client.HeartBeat();

    stub.set(ADDR(CConnection, GetSendMsgFlag), StubGetSendMsgFlagTrue);
    stub.set(ADDR(BusinessClient, NewMsgPair), StubFailed);
    client.HeartBeat();

    stub.set(ADDR(CDppMessage, InitMsgHead), StubVoid);
    stub.set(ADDR(CDppMessage, SetLinkInfo), StubVoid);
    stub.set(ADDR(CDppMessage, SetMsgSrc), StubVoid);
    stub.set(ADDR(CDppMessage, SetMsgTgt), StubVoid);
    stub.set(ADDR(CDppMessage, SetMsgBody), StubVoid);
    stub.set(ADDR(BusinessClient, NewMsgPair), StubSuccess);
    stub.set(ADDR(MessageHandler, PushRspMsg), StubSuccess);
    client.HeartBeat();
}

TEST_F(BusinessClientTest, InsertBusiClientToDB)
{
    BusinessClient client;
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CConnection, GetClientIpAddrStr), GetClientIpAddrStrTest);
    stub.set(ADDR(CConnection, GetClientPort), GetClientPortTest);
    stub.set(ADDR(CDB, QueryTable), QueryTableTest);
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    client.InsertBusiClientToDB();

    stub.set(ADDR(CDB, ExecSql), ExecSqlFailedTest);
    client.InsertBusiClientToDB();

    // stub.set(ADDR(CDB, QueryTable), QueryTableTest2);
    // client.InsertBusiClientToDB();
}

TEST_F(BusinessClientTest, GetLisetenIPTest)
{
    BusinessClient client;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CConnection, GetClientIpAddr), StubGetClientIpAddr);
    stub.set(ADDR(CConnection, GetClientPort), StubGetClientPort);
    stub.set(GetNginxListenIP, StubGetNginxListenIPTrue);
    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatusFailed);
    mp_string sRet = client.GetLisetenIP();
    EXPECT_EQ(sRet, "");

    stub.set(ADDR(CSocket, CheckHostLinkStatus), StubCheckHostLinkStatusSuccess);
    sRet = client.GetLisetenIP();
    EXPECT_EQ(sRet, "");
}

TEST_F(BusinessClientTest, DelBusiClientFromDBTest)
{
    BusinessClient client;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(CConnection, GetClientIpAddrStr), GetClientIpAddrStrTest);
    stub.set(ADDR(CConnection, GetClientPort), GetClientPortTest);
    stub.set(ADDR(CDB, ExecSql), ExecSqlTest);
    client.DelBusiClientFromDB();

    stub.set(ADDR(CDB, ExecSql), ExecSqlFailedTest);
    client.DelBusiClientFromDB();
}

TEST_F(BusinessClientTest, NewMsgPairTest)
{
    BusinessClient client;
    CDppMessage* reqMsg = NULL;
    CDppMessage* rspMsg = NULL;
    mp_uint64 seqNo;

    client.NewMsgPair(reqMsg, rspMsg, seqNo);
}
