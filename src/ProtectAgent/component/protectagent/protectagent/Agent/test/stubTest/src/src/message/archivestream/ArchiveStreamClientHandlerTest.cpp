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
#include "message/archivestream/ArchiveStreamClientHandlerTest.h"

static mp_int32 reTryFlag = 0;
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

mp_int32 Create_SUCCESS(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_SUCCESS;
}
mp_int32 Create_FAILED(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    return MP_FAILED;
}

mp_int32 Create_FAILED_TWO(thread_id_t* id, thread_proc_t proc, mp_void* arg, mp_uint32 uiStackSize)
{
    if (reTryFlag ++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 Stub_FAILED()
{
    return MP_FAILED;
}

mp_int32 Stub_SUCCESS()
{
    return MP_SUCCESS;
}

mp_bool Stub_TRUE()
{
    return MP_TRUE;
}

mp_void Stub_VOID()
{
    return;
}

mp_void StubDoSleep()
{
    reTryFlag++;
    return;
}

mp_string Stub_GetClientIpAddrStr()
{
    return "";
}

mp_int32 Connect_SUCCESS()
{
    return MP_SUCCESS;
}
mp_int32 Connect_FAILED()
{
    return MP_FAILED;
}

#ifdef SUPPORT_SSL
mp_int32 Init_SUCCESS(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role, SSL_CTX *psslCtx)
{
    return MP_SUCCESS;
}
#else
mp_int32 Init_SUCCESS(mp_string& serverIp, mp_uint16 serverPort, MESSAGE_ROLE role)
{
    return MP_SUCCESS;
}
#endif

CConnection GetConnection_stub()
{
    CConnection conn;
    return conn;
}
mp_void InitSsl_stub()
{
    return;
}

mp_string GetIpAddrStr_stub()
{
    return "127.0.0.1";
}

/*
* 用例名称：初始化失败
* 前置条件：线程创建失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Init_Fail1)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CMpThread, Create), Create_FAILED);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Init(), MP_FAILED);
}

/*
* 用例名称：初始化成功
* 前置条件：线程创建成功
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Init_Fail2)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CMpThread, Create), Create_FAILED_TWO);
    stub.set(ADDR(CMpThread, WaitForEnd), Stub_SUCCESS);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Init(), MP_FAILED);
}

/*
* 用例名称：初始化成功
* 前置条件：线程创建成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClientHandler, Init_OK)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CMpThread, Create), Create_SUCCESS);
    stub.set(ADDR(ArchiveStreamClientHandler, InitSsl), InitSsl_stub);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Init(), MP_SUCCESS);
}

/*
* 用例名称：初始化成功
* 前置条件：线程创建成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClientHandler, Init_OK2)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    m_handler.m_inited = true;
    EXPECT_EQ(m_handler.Init(), MP_SUCCESS);
}


/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Connect_Fail)
{
    mp_string busiIp = "127.0.0.1";
    mp_uint16 busiPort;
    MESSAGE_ROLE role;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, false), MP_FAILED);
}

/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Connect_Fail2)
{
    mp_string busiIp = "";
    mp_uint16 busiPort;
    MESSAGE_ROLE role;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, false), MP_FAILED);
}

/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Connect_Fail3)
{
    mp_string busiIp = "";
    mp_uint16 busiPort;
    MESSAGE_ROLE role;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, ROLE_HOST_AGENT,false), MP_FAILED);
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort,false), MP_FAILED);
}

/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Connect_Fail4)
{
    mp_string busiIp = "127.0.0.1,";
    mp_uint16 busiPort;
    MESSAGE_ROLE role;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Stub_SUCCESS);
    ArchiveStreamClientHandler m_handler;
    m_handler.m_dissconnected = MP_TRUE;
    stub.set(ADDR(ArchiveStreamClientHandler, CreateThreadIfReconnect), Stub_FAILED);
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, false), MP_FAILED);

    stub.set(ADDR(ArchiveStreamClientHandler, CreateThreadIfReconnect), Stub_SUCCESS);
    stub.set(ADDR(CMpThread, Create), Create_FAILED);
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, false), MP_SUCCESS);
}


/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, Connect_NoParam_Fail)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    m_handler.m_serverList.push_back("test");

    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Connect_FAILED);
    EXPECT_EQ(m_handler.Connect(), MP_FAILED);

    m_handler.m_dissconnected = MP_TRUE;
    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Connect_SUCCESS);
    stub.set(ADDR(ArchiveStreamClientHandler, CreateThreadIfReconnect), Stub_FAILED);
    EXPECT_EQ(m_handler.Connect(), MP_FAILED);
}

/*
* 用例名称：连接archive成功
* 前置条件：无
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClientHandler, Connect_NoParam_Success)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler m_handler;
    m_handler.m_serverList.push_back("test");

    m_handler.m_dissconnected = MP_FALSE;
    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Connect_SUCCESS);
    EXPECT_EQ(m_handler.Connect(), MP_SUCCESS);
}


/*
* 用例名称：连接archive成功
* 前置条件：初始化成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClientHandler, Connect_Success)
{
    mp_string busiIp = "127.0.0.1";
    mp_uint16 busiPort;
    MESSAGE_ROLE role;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CMpThread, Create), Create_SUCCESS);
    stub.set(ADDR(ArchiveStreamClient, Init), Init_SUCCESS);
    stub.set(ADDR(ArchiveStreamClient, Connect), Connect_SUCCESS);

    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.Connect(busiIp, busiPort, role), MP_SUCCESS);
}

/*
* 用例名称：获取连接archive状态失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, GetConnectState_Success)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.GetConnectState(), MP_FAILED);
}

/*
* 用例名称：获取发送成功信号失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, GetSendExitFlag)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
   ArchiveStreamClientHandler m_handler;
   EXPECT_EQ(m_handler.GetSendExitFlag(), false);
}

/*
* 用例名称：获取序列号信号失败
* 前置条件：无
* check点：序列号自增
*/
TEST_F(TestArchiveStreamClientHandler, GetSeqNo)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
   ArchiveStreamClientHandler m_handler;
   EXPECT_EQ(m_handler.GetSeqNo(), 0);
   EXPECT_EQ(m_handler.GetSeqNo(), 1);
}

/*
* 用例名称：获取连接的archiveip成功
* 前置条件：设置client的ip信息
* check点：成功获取对应ip
*/
TEST_F(TestArchiveStreamClientHandler, GetConnectedServerAddr)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);
    stub.set(ADDR(CConnection, GetClientIpAddrStr), GetIpAddrStr_stub);
    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.GetConnectedServerAddr(), "127.0.0.1");
}

/*
* 用例名称：获取回复报文成功
* 前置条件：设置全局的回复变量
* check点：接口返回成功
*/

TEST_F(TestArchiveStreamClientHandler, GetResponseMessage_Success)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    CDppMessage *message = NULL;
    NEW_CATCH(message, CDppMessage);
    mp_string taskId = "127.0.0.1";
    mp_uint64 seqNo = 1;
    mp_uint32 timeout = 50;
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);

    ArchiveStreamClientHandler m_handler;
    EXPECT_EQ(m_handler.GetResponseMessage(taskId, message, seqNo, timeout), MP_SUCCESS);
}

/*
* 用例名称：接收消息
* 前置条件：无
* check点：获取消息体失败
*/
TEST_F(TestArchiveStreamClientHandler, handleRecevMsg_Failed)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    CDppMessage *message = NULL;
    NEW_CATCH(message, CDppMessage);
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);

    stub.set(ADDR(CDppMessage, GetManageBody), Stub_FAILED);
    ArchiveStreamClientHandler m_handler;
    m_handler.handleRecevMsg(message);
    EXPECT_NE(message, nullptr);
}

/*
* 用例名称：接收消息
* 前置条件：无
* check点：获取消息体失败
*/
TEST_F(TestArchiveStreamClientHandler, handleRecevMsg_Failed2)
{
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    CDppMessage *message = NULL;
    NEW_CATCH(message, CDppMessage);
    message->dppMessage.uiCmd = CMD_ARCHIVE_GET_FILE_DATA_BIN_ACK;
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);

    stub.set(ADDR(CDppMessage, GetManageBody), Stub_FAILED);
    ArchiveStreamClientHandler m_handler;
    m_handler.handleRecevMsg(message);
    EXPECT_NE(message, nullptr);
}

/*
* 用例名称：连接archive成功
* 前置条件：无
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClientHandler, SwitchConnect_Success)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    m_handler.m_serverList.push_back("127.0.0.1");
    m_handler.m_serverList.push_back("127.0.0.2");
    m_handler.m_currentServerIP = "127.0.0.1";
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);
    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Connect_SUCCESS);
    EXPECT_EQ(m_handler.SwitchConnect(), MP_SUCCESS);
}

/*
* 用例名称：发起连接
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, SwitchConnect_Fail)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    m_handler.m_serverList.push_back("127.0.0.1");
    m_handler.m_serverList.push_back("127.0.0.2");
    m_handler.m_currentServerIP = "127.0.0.1";
    stub.set(ADDR(ArchiveStreamClient, GetConnection), GetConnection_stub);
    stub.set((mp_int32(ArchiveStreamClientHandler::*)(const mp_string&, mp_uint16, MESSAGE_ROLE, bool))
        ADDR(ArchiveStreamClientHandler, Connect), Connect_FAILED);
    EXPECT_EQ(m_handler.SwitchConnect(), MP_FAILED);
}

/*
* 用例名称：接收线程
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, RecvThread)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler *tcpClient;
    tcpClient = nullptr;
    EXPECT_EQ(m_handler.RecvThread(tcpClient), nullptr);
}

/*
* 用例名称：发送线程
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClientHandler, SendThread)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    ArchiveStreamClientHandler *tcpClient;
    tcpClient = nullptr;
    EXPECT_EQ(m_handler.SendThread(tcpClient), nullptr);
}

/*
* 用例名称：重连时创建线程失败
* 前置条件：无
* check点：启动接收线程失败
*/
TEST_F(TestArchiveStreamClientHandler, CreateThreadIfReconnect_Fail)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    stub.set(ADDR(CMpThread, Create), Create_FAILED);
    EXPECT_EQ(m_handler.CreateThreadIfReconnect(), MP_FAILED);
}

/*
* 用例名称：重连时创建线程失败
* 前置条件：无
* check点：启动发送线程失败
*/
TEST_F(TestArchiveStreamClientHandler, CreateThreadIfReconnect_Fail2)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    reTryFlag = 0;
    stub.set(ADDR(CMpThread, Create), Create_FAILED_TWO);
    stub.set(ADDR(CMpThread, WaitForEnd), Stub_VOID);
    EXPECT_EQ(m_handler.CreateThreadIfReconnect(), MP_FAILED);
}

/*
* 用例名称：重连时创建线程成功
* 前置条件：无
* check点：检查接口返回值
*/
TEST_F(TestArchiveStreamClientHandler, CreateThreadIfReconnect_Success)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);

    reTryFlag = 0;
    stub.set(ADDR(CMpThread, Create), Create_SUCCESS);
    EXPECT_EQ(m_handler.CreateThreadIfReconnect(), MP_SUCCESS);
}

/*
* 用例名称：发送消息
* 前置条件：无
* check点：无
*/
TEST_F(TestArchiveStreamClientHandler, SendDPMessage)
{
    Stub stub;
    ArchiveStreamClientHandler m_handler;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    mp_string taskId;
    CDppMessage *reqMsg;
    reqMsg = nullptr;
    m_handler.SendDPMessage(taskId, reqMsg);
    EXPECT_EQ(nullptr, reqMsg);
}