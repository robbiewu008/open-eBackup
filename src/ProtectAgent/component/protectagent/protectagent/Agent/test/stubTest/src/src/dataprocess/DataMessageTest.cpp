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
#include "dataprocess/DataMessageTest.h"
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32Return); \
} while (0)

TEST_F(DataMessageTest, Init) {
    mp_socket sock = 1234;
    mp_string ip = "192.168.1.1";
    mp_uint16 port = 38294;
    DataMessage om;

    mp_int32 iRet = om.Init(sock, ip, port);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(DataMessageTest, SetSockTimeOut) {
    DataMessage om;
    mp_int32 iRet = 0;

    stub.set(ADDR(CConnection, GetClientSocket), StubGetClientSocketFail);
    iRet = om.SetSockTimeOut(2);
    EXPECT_EQ(MP_SUCCESS, iRet);
    stub.reset(ADDR(CConnection, GetClientSocket));


    iRet = om.SetSockTimeOut(-1);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(DataMessageTest, CreateClient) {
    DataMessage om;
    mp_int32 iRet = 0;
    mp_socket clientSock;
    StubClogToVoidLogNullPointReference();
    // socket connect fail
    {
        stub.set(ADDR(CSocket, CreateClientSocket), StubCreateClientSocket);
        stub.set((XMLElement* (XMLElement::*)(const char* name))ADDR(XMLElement, FirstChildElement), StubFirstChildElement);
        iRet = om.CreateClient(clientSock);
        EXPECT_EQ(9, iRet);
    }
    // socket connect succ
    {
        stub.set(ADDR(CSocket, CreateClientSocket), StubCreateClientSocket);
        stub.set((XMLElement* (XMLElement::*)(const char* name))ADDR(XMLElement, FirstChildElement), StubFirstChildElement);
        stub.set(ADDR(CSocket, Connect), StubConnect);
        iRet = om.CreateClient(clientSock);
        EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
    }

    stub.reset(ADDR(CSocket, Connect));
    stub.reset(ADDR(CSocket, CreateClientSocket));
    stub.reset((XMLElement* (XMLElement::*)(const char* name))ADDR(XMLElement, FirstChildElement));
}

TEST_F(DataMessageTest, GetRandomPort) {
    DataMessage om;
    mp_int32 iRet = 0;
    StubClogToVoidLogNullPointReference();
    // get port fail
    {
        stub.set(ADDR(CSocket, Bind), BindFail);
        iRet = om.GetRandomPort(2, "192.168.1.1");
        EXPECT_EQ(0, iRet);
    }
    // get port succ
    {
        stub.set(ADDR(CSocket, Bind), BindSucc);
        iRet = om.GetRandomPort(2, "192.168.1.1");
        EXPECT_NE(0, iRet);
    }

    stub.reset(ADDR(CSocket, Bind));
}

TEST_F(DataMessageTest, CreateRandomPortServer) {
    DataMessage om;
    mp_int32 iRet = 0;
    mp_socket sockket;
    mp_string dpparam;
    StubClogToVoidLogNullPointReference();

    stub.set(ADDR(CSocket, CreateTcpSocket), StubCreateTcpSocket);
    stub.set(ADDR(CSocket, SetReuseAddr), StubSetReuseAddr);
    stub.set(ADDR(CSocket, StartListening), StubStartListening);
    stub.set(ADDR(DataMessage, GetRandomPort), StubGetRandomPort);
    stub.set(ADDR(DataMessage, WritePort), StubWritePort);

    iRet = om.CreateRandomPortServer(sockket, "192,168.1.1", 1, dpparam);
    EXPECT_EQ(MP_SUCCESS, iRet);
    
    stub.reset(ADDR(CSocket, CreateTcpSocket));
    stub.reset(ADDR(CSocket, SetReuseAddr));
    stub.reset(ADDR(CSocket, StartListening));
    stub.reset(ADDR(DataMessage, GetRandomPort));
    stub.reset(ADDR(DataMessage, WritePort));
}

TEST_F(DataMessageTest, CloseDppConnection) {
    DataMessage om;
    mp_int32 iRet = 0;
    StubClogToVoidLogNullPointReference();

    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    om.CloseDppConnection();

    stub.reset(ADDR(DppSocket, CloseConnect));
}

TEST_F(DataMessageTest, SendDpp) {
    DataMessage om;
    mp_int32 iRet = 0;
    CDppMessage message;
    StubClogToVoidLogNullPointReference();

    stub.set(ADDR(CDppMessage, ReinitMsgBody), StubReinitMsgBody);
    stub.set(ADDR(DppSocket, SendMsg), StubSendMsg);
    iRet = om.SendDpp(message);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.reset(ADDR(CDppMessage, ReinitMsgBody));
    stub.reset(ADDR(DppSocket, SendMsg));
}

TEST_F(DataMessageTest, StartReceiveDpp) {
    DataMessage om;
    mp_int32 iRet = 0;
    CDppMessage message;
    StubClogToVoidLogNullPointReference();

    om.recvdMsg = new CDppMessage;
    iRet = om.StartReceiveDpp();
    EXPECT_EQ(MP_SUCCESS, iRet);

}

TEST_F(DataMessageTest, ConvertDppBody2Json) {
    DataMessage om;
    mp_int32 iRet = 0;
    CDppMessage message;
    Json::Value bodyMsg;
    StubClogToVoidLogNullPointReference();
    //have no "body"
    {
        stub.set(ADDR(CDppMessage, GetBuffer), StubGetBufferNoKey);
        iRet = om.ConvertDppBody2Json(message, bodyMsg);
        EXPECT_EQ(ERROR_COMMON_INVALID_PARAM, iRet);
    }
    //have "body"
    {
        stub.set(ADDR(CDppMessage, GetBuffer), StubGetBuffer);
        iRet = om.ConvertDppBody2Json(message, bodyMsg);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }

    stub.reset(ADDR(CDppMessage, GetBuffer));
}

TEST_F(DataMessageTest, HandleRecvDppMessage) {
    DataMessage om;
    mp_int32 iRet = 0;
    CDppMessage message;
    StubClogToVoidLogNullPointReference();

    iRet = om.HandleRecvDppMessage(message);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(DataMessageTest, WritePort) {
    DataMessage om;
    mp_int32 iRet = 0;
    mp_string file;
    mp_uint16 uiPort = 12345;
    StubClogToVoidLogNullPointReference();
    //no file
    {
        iRet = om.WritePort(file, uiPort);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //have file
    {
        file = "test";
        iRet = om.WritePort(file, uiPort);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
}

TEST_F(DataMessageTest, ReadPort) {
    DataMessage om;
    mp_uint16 iRet = 0;
    mp_string file;
    StubClogToVoidLogNullPointReference();
    //no file:有符号数字的-1为无符号数的最大值65535
    {
        iRet = om.ReadPort(file);
        EXPECT_EQ(65535, iRet);
    }
    //have file
    {
        file = "test";
        iRet = om.ReadPort(file);
        EXPECT_EQ(12345, iRet);
    }
}