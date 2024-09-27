#define SUPPORT_SSL
#include "message/archivestream/ArchiveStreamClientTest.h"

namespace {
mp_void CLogger_Log_Stub(mp_void* pthis)
{
    return;
}

}

/*
* 用例名称：连接archive失败
* 前置条件：无
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, Connect_Fail1)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    EXPECT_EQ(client.Connect(), MP_FAILED);
}

/*
* 用例名称：连接archive失败
* 前置条件：底层连接成功，但是开启接收进程失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, Connect_Fail2)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConnection, Connect), Connect_SUCCESS2);
    stub.set(ADDR(CConnection, StartRecvMsg), StartRecvMsg_FAILED);
    EXPECT_EQ(client.Connect(), MP_FAILED);
}

/*
* 用例名称：连接archive成功
* 前置条件：底层连接成功，开启接收进程成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClient, Connect_Succ)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CConnection, Connect), Connect_SUCCESS2);
    stub.set(ADDR(CConnection, StartRecvMsg), StartRecvMsg_SUCCESS);
    EXPECT_EQ(client.Connect(), MP_SUCCESS);
}

/*
* 用例名称：接收事件失败
* 前置条件：等待事件触发失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, RecvEventsReady_False1)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CSocket, WaitEvents), WaitEvents_Fail1);
    EXPECT_EQ(client.RecvEventsReady(), MP_FALSE);
}

/*
* 用例名称：接收事件失败
* 前置条件：等待事件触发失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, RecvEventsReady_False2)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CSocket, WaitEvents), WaitEvents_Fail2);
    EXPECT_EQ(client.RecvEventsReady(), MP_FALSE);
}

/*
* 用例名称：接收事件成功
* 前置条件：等待事件触发成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClient, RecvEventsReady_True)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CSocket, WaitEvents), WaitEvents_Sucess);
    EXPECT_EQ(client.RecvEventsReady(), MP_TRUE);
}

/*
* 用例名称：获取archive连接成功
* 前置条件：底层连接成功，开启接收进程成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClient, GetRole)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    EXPECT_EQ(MESSAGE_ROLE::ROLE_HOST_AGENT, client.GetRole());
}

/*
* 用例名称：获取信号序号成功
* 前置条件：底层连接成功，开启接收进程成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClient, GetSeqNo)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    EXPECT_EQ(client.GetSeqNo(), MP_SUCCESS);
}

/*
* 用例名称：发送报文失败
* 前置条件：底层连接失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, SendDppMsg_Fail)
{
    ArchiveStreamClient client;
    CDppMessage message;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CDppMessage, ReinitMsgBody), ReinitMsgBody_FAILED);
    EXPECT_EQ(client.SendDppMsg(message), MP_FAILED);
}

/*
* 用例名称：发送报文成功
* 前置条件：底层连接成功
* check点：接口返回成功
*/
TEST_F(TestArchiveStreamClient, SendDppMsg_OK)
{
    ArchiveStreamClient client;
    CDppMessage message;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(CDppMessage, ReinitMsgBody), ReinitMsgBody_SUCCESS);
    stub.set(ADDR(DppSocket, SendMsg), SendMsg_SUCCESS);
    EXPECT_EQ(client.SendDppMsg(message), MP_SUCCESS);
}

/*
* 用例名称：接收报文失败
* 前置条件：底层连接失败
* check点：接口返回失败
*/
TEST_F(TestArchiveStreamClient, RecvMessage_Failed)
{
    ArchiveStreamClient client;
    Stub stub;
    stub.set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))
        ADDR(CLogger, Log), CLogger_Log_Stub);
    stub.set(ADDR(DppSocket, RecvMsg), RecvMsg_FAILED);
    EXPECT_EQ(client.RecvMessage(), MP_FAILED);

    stub.set(ADDR(DppSocket, RecvMsg), RecvMsg_EAGAIN);
    EXPECT_EQ(client.RecvMessage(), MP_FAILED);
}


/*
* 用例名称：接收消息
* 前置条件：无
* check点：接收成功
*/
TEST_F(TestArchiveStreamClient, HandleRecvDppMessage_Success)
{
    ArchiveStreamClient client;
    Stub stub;
    CDppMessage *message;
    NEW_CATCH(message, CDppMessage);

    stub.set(&CDppMessage::GetCmd, StubGetCmd);
    stub.set(&CDppMessage::GetSize2, StubGetSize2);
    stub.set(&CDppMessage::GetOrgSeqNo, StubGetOrgSeqNo);
    EXPECT_EQ(client.HandleRecvDppMessage(*message), MP_SUCCESS);
}
