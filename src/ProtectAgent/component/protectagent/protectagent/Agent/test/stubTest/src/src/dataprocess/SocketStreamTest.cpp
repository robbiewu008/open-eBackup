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
#include "dataprocess/SocketStreamTest.h"
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubSocketStreamGetValueInt32Return); \
} while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(SocketStreamTest, StreamWrite) {
    mp_void *ctx;
    mp_char buff[] = "abc";
    mp_int32 iBuffLen = 1;
    SocketStream om;
    mp_int32 iRet = 0;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //����ʧ��
    {
        stub.set(ADDR(DataContext, GetSockFd), StubGetSockFd);
        stub.set(ADDR(CSocket, SendBuffer), StubSendBuffer);
        iRet = om.StreamWrite(ctx, buff, iBuffLen);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //���ͳɹ�
    {
        iBuffLen = 3;
        stub.set(ADDR(DataContext, GetSockFd), StubGetSockFd);
        stub.set(ADDR(CSocket, SendBuffer), StubSendBuffer);
        iRet = om.StreamWrite(ctx, buff, iBuffLen);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
       
    stub.reset(ADDR(CSocket, SendBuffer));
    stub.reset(ADDR(DataContext, GetSockFd));
}

TEST_F(SocketStreamTest, StreamRead) {
    mp_void *ctx;
    mp_char buff[] = "abc";
    mp_int32 iBuffLen = 1;
    mp_uint32 uiRecvLen = 0;
    SocketStream om;
    mp_int32 iRet = 0;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //����ʧ��
    {
        stub.set(ADDR(DataContext, GetSockFd), StubGetSockFd);
        stub.set(ADDR(CSocket, RecvBuffer), StubRecvBufferFail);
        iRet = om.StreamRead(ctx, buff, iBuffLen, uiRecvLen);
        EXPECT_EQ(MP_FAILED, iRet);
    }
    //���ճɹ�
    {
        stub.set(ADDR(DataContext, GetSockFd), StubGetSockFd);
        stub.set(ADDR(CSocket, RecvBuffer), StubRecvBuffer);
        iRet = om.StreamRead(ctx, buff, iBuffLen, uiRecvLen);
        EXPECT_EQ(MP_SUCCESS, iRet);
    }
}

// mp_int32 SocketStream::StreamWrite(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen)
// {
//     mp_int32 uiSendLen = 0;
//     mp_socket sock = 0;

//     DataContext *ctxdata = static_cast<DataContext *>(ctx);
//     sock = ctxdata->GetSockFd();

//     uiSendLen = CSocket::SendBuffer(sock, buff, iBuffLen, TIMEOUT_INFINITE);
//     if (iBuffLen == uiSendLen) {
//         COMMLOG(OS_LOG_DEBUG, "Send messge to sock %d, len=%d.", sock, uiSendLen);
//         return MP_SUCCESS;
//     }
//     COMMLOG(OS_LOG_ERROR, "Send msg failed, need send %u, send %u.", iBuffLen, uiSendLen);

//     return MP_FAILED;
// }

// mp_int32 SocketStream::StreamRead(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen, mp_uint32 &iRecvLen)
// {
//     // /1. check the data availability, if not available, return immediately
//     // /2. receive a complete cmd not a partical cmd

//     mp_int32 iRet = MP_SUCCESS;
//     mp_uint32 uiRecvLen = 0;
//     mp_socket sock = 0;

//     DataContext *ctxdata = static_cast<DataContext *>(ctx);
//     sock = ctxdata->GetSockFd();

//     iRet = CSocket::RecvBuffer(sock, buff, iBuffLen, TIMEOUT_INFINITE, uiRecvLen);
//     if (iRet == MP_FAILED) {
//         COMMLOG(OS_LOG_ERROR, "Receive message failed, iRet %d", iRet);
//         return (iRet);
//     }

//     return (iRet);
// }
