#include "message/tcp/DppSocketTest.h"
#include "common/Utils.h"
#include "common/Log.h"
#include "message/tcp/TCPClientHandler.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "message/archivestream/ArchiveStreamClient.h"

mp_int32 flag;

namespace{
mp_void StubVoid(mp_void* pobj)
{
    return;
}

mp_bool StubTrue(mp_void* pobj)
{
    return MP_TRUE;
}

mp_bool StubFalse(mp_void* pobj)
{
    return MP_FALSE;
}

mp_bool StubCheckOSErr(mp_int32& iErr)
{
    iErr = EPERM;
    return MP_FALSE;
}

mp_double StubDifftime(mp_time end, mp_time begin)
{
    mp_double numTwo = 2;
    return numTwo;
}

ConnectRole StubGetConnectRole()
{
    return CLINET;
}
mp_string StubGetClientIpAddrStr()
{
    return "";
}
mp_uint32 StubGetSize2()
{
    return 1;
}

mp_int32 HandleRecvDppMessageTest(mp_void* pThis, CDppMessage &message)
{
    return MP_SUCCESS;
}

mp_string StubGetClientPort()
{
    return "111";
}

mp_int32 StubFailed(mp_void* pobj)
{
    return MP_FAILED;
}

mp_int32 StubFailedOnTwo(mp_void* pobj)
{
    if (flag++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 StubSuccess(mp_void* pobj)
{
    return MP_SUCCESS;
}

mp_int32 StubEagain(mp_void* pobj)
{
    return MP_EAGAIN;
}

mp_int32 StubGetConnectErr(mp_void* pobj)
{
    return MP_FAILED;
}

mp_bool StubInReceivingStateOnTwoFalse(mp_uint32 uiSubState)
{
    if (flag++ < 1) {
        return true;
    }
    return false;
}

mp_bool StubInReceivingStateOnTwoTrue(mp_uint32 uiSubState)
{
    if (flag++ < 1) {
        return false;
    }
    return true;
}

CDppMessage *StubGetDppMessage()
{
    CDppMessage* dppMsg;
    return dppMsg ;
}
}

static mp_void StubCLoggerLog(mp_void){
    return;
}

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubDppSocketGetValueInt32Return);                                                                      \
            stub.set(&CLogger::Log, StubCLoggerLog);                                                                \
    } while (0)

class TestDppSocket :  public DppSocket {
public:
    virtual mp_int32 HandleRecvDppMessage(CDppMessage &message)
    {
        return MP_SUCCESS;
    }

    ~TestDppSocket()
    {}
};

TEST_F(DppSocketTest, SendMsg)
{
    CDppMessage message;
    CConnection conn;
    message.dppMessage.uiSize = 1234;
    message.dppMessage.body = new char[5];
    memcpy (message.dppMessage.body, "test", 5);

    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(WipeSensitiveForJsonData, StubWipeSensitiveForJsonData);
    stub.set(ADDR(DppSocket, SendBuffer), StubSendBuffer);
    mp_int32 iRet = om.DppSocket::SendMsg(message, conn);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(DppSocket, SendMsgImpl), StubFailed);
    stub.set(ADDR(DppSocket, HandleRecvSendFail), StubFalse);
    iRet = om.DppSocket::SendMsg(message, conn);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset(ADDR(DppSocket, SendBuffer));
}

TEST_F(DppSocketTest, CheckMsgLength)
{
    mp_int32 len = -10;
    mp_socket sock = 5;
    CConnection conn;
    mp_string ip = "192.168";
    mp_uint16 port = 1001;

    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(GetOSError, StubGetOSError_1);
    stub.set(GetOSStrErr, StubGetOSStrErr);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    mp_int32 iRet = om.DppSocket::CheckMsgLength(len, sock, conn, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(GetOSError, StubGetOSError_4);
    iRet = om.DppSocket::CheckMsgLength(len, sock, conn, ip, port);
    EXPECT_EQ(MP_EAGAIN, iRet);

    len = 0;
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    iRet = om.DppSocket::CheckMsgLength(len, sock, conn, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset(ADDR(DppSocket, SendBuffer));

    len = -10;
    stub.set(ADDR(DppSocket, GetConnectErr), StubGetConnectErr);
    iRet = om.DppSocket::CheckMsgLength(len, sock, conn, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(ADDR(DppSocket, GetConnectErr));
}

TEST_F(DppSocketTest, ReceiveAllMsg)
{
    mp_int32 numFive = 5;
    mp_int32 numTen = 10;
    mp_int32 len = 15;
    mp_socket socket = 5;
    CConnection conn;
    mp_string ip = "192.168";
    mp_uint16 port = 1001;
    CDppMessage* message;
    control_stat_t* stat = new control_stat_t();
    stat->uiBytesToRecv = numTen;

    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    mp_int32 iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    len = numFive;
    stat->uiBytesToRecv = numTen;
    stub.set(GetOSError, StubGetOSError_4);
    iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_SUCCESS, iRet);

    len = numTen;
    stat->uiBytesToRecv = numFive;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingState);
    iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    len = numFive;
    stat->uiBytesToRecv = numFive;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingState);
    stub.set(ADDR(DppSocket, ProcessMsgPart1), StubFailed);
    stub.set(ADDR(DppSocket, ProcessMsgPart2), StubFailed);
    iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    len = numFive;
    stat->uiBytesToRecv = numFive;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingStateFalse);
    iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_SUCCESS, iRet);

    flag = 0;
    len = numFive;
    stat->uiBytesToRecv = numFive;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingStateOnTwoTrue);
    iRet = om.DppSocket::ReceiveAllMsg(conn, message, len, stat, socket, ip, port);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset(ADDR(DppSocket, CloseConnect));
    stub.reset(ADDR(DppSocket, ProcessMsgPart1));
    stub.reset(ADDR(DppSocket, ProcessMsgPart2));
    stub.reset(ADDR(CConnection, InReceivingState));
}

TEST_F(DppSocketTest, RecvMsg)
{
    CConnection connection;

    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.DppSocket::RecvMsg(connection);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(DppSocket, RecvMsgImpl), StubSuccess);
    iRet = om.DppSocket::RecvMsg(connection);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(ADDR(DppSocket, RecvMsgImpl), StubEagain);
    iRet = om.DppSocket::RecvMsg(connection);
    EXPECT_EQ(MP_EAGAIN, iRet);
}

TEST_F(DppSocketTest, ProcessMsgPart1)
{
    CConnection connection;
    CDppMessage *message;
    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    mp_int32 iRet = om.DppSocket::ProcessMsgPart1(connection, message);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CDppMessage, IsValidPrefix), StubIsValidPrefix);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    iRet = om.DppSocket::ProcessMsgPart1(connection, message);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CDppMessage, IsValidPrefix), StubIsValidPrefix);
    stub.set(ADDR(CDppMessage, InitMsgBody), StubInitMsgBody);
    iRet = om.DppSocket::ProcessMsgPart1(connection, message);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(DppSocketTest, SendBuffer)
{
    CConnection connection;
    mp_char *buf = NULL;
    TestDppSocket om;
    mp_int32 iRet = 0;
    stub.set(&CLogger::Log, StubCLoggerLog);
    iRet = om.DppSocket::SendBuffer(buf, connection);
    EXPECT_EQ(MP_FAILED, iRet);

    control_stat_t* stat = new control_stat_t();
    stat->uiBytesToSend = 10;
    mp_char *str = "test";
    stub.set(ADDR(CConnection, Send), StubSend);
    iRet = om.DppSocket::SendBuffer(str, connection);
    EXPECT_EQ(MP_FAILED, iRet);

    connection.GetControlStat()->uiBytesToSend = 10;
    stub.set(ADDR(CConnection, Send), StubSendGt0);
    iRet = om.DppSocket::SendBuffer(str, connection);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(DppSocketTest, SendBufferLessThan0)
{
    CConnection connection;
    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(GetOSError, StubGetOSError_1);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    om.DppSocket::SendBufferLessThan0(connection);
}

TEST_F(DppSocketTest, CloseConnect)
{
    CConnection connection;
    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);

    stub.set(GetOSError, StubGetOSError_1);
    stub.set(ADDR(CConnection, GetLinkState), StubGetLinkState);
    om.DppSocket::CloseConnect(connection);
}

TEST_F(DppSocketTest, GetBuffer)
{
    CConnection connection;
    CDppMessage message;
    mp_char *recvBuf;
    TestDppSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.DppSocket::GetBuffer(connection, message, recvBuf);

    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingState);
    om.DppSocket::GetBuffer(connection, message, recvBuf);

    flag = 0;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingStateOnTwoTrue);
    om.DppSocket::GetBuffer(connection, message, recvBuf);
}

static mp_int32 stubByteSend = 0;
static mp_int32 stubByteRecv = 0;
static mp_bool stubRecvReconnect = MP_FALSE;
static mp_int32 StubSendFunc(mp_char *buff, mp_uint32 iBuffLen)
{
    return stubByteSend;
}
static mp_int32 StubRecvFunc(mp_char *buff, mp_uint32 iBuffLen)
{
    return stubByteRecv;
}
static mp_void StubResetSendConnect()
{
    stubByteSend = 10;
}
static mp_void StubResetRecvConnect()
{
    stubRecvReconnect = MP_TRUE;
}
TEST_F(DppSocketTest, SendBufferByReconnect)
{
    CConnection connection;
    mp_char *buf = NULL;
    TestDppSocket om;
    mp_int32 iRet = 0;
    mp_bool reset = 0;
    StubClogToVoidLogNullPointReference();
    mp_int32 numTen = 10;
    connection.GetControlStat()->uiBytesToSend = numTen;
    mp_char *str = "test";
    stub.set(ADDR(CConnection, Send), StubSendFunc);
    stub.set(ADDR(CConnection, ResetConnection), StubResetSendConnect);
    // 构造连接关闭场景
    stubByteSend = 0;
    iRet = om.DppSocket::SendBuffer(str, connection);
    EXPECT_EQ(MP_FAILED, iRet);
    connection.GetControlStat()->uiBytesToSend = numTen;
    // 构造发送异常场景
    stubByteSend = -1;
    iRet = om.DppSocket::SendBuffer(str, connection);
    EXPECT_EQ(MP_FAILED, iRet);
}
TEST_F(DppSocketTest, RecvMsgByReconnect)
{
    CConnection connection;
    TestDppSocket om;
    StubClogToVoidLogNullPointReference();
    stub.set(ADDR(CConnection, Recv), StubRecvFunc);
    stub.set(ADDR(CConnection, ResetConnection), StubResetRecvConnect);
    // 构造连接关闭场景
    stubByteRecv = 0;
    stubRecvReconnect = MP_FALSE;
    connection.StartRecvMsg();
    mp_int32 iRet = om.DppSocket::RecvMsg(connection);
    EXPECT_EQ(iRet, MP_FAILED);
    CConnection connection1;
    // 构造发送异常场景
    stubByteRecv = -1;
    stubRecvReconnect = MP_FALSE;
    iRet = om.DppSocket::RecvMsg(connection);
    EXPECT_EQ(iRet, MP_FAILED);
}

mp_int32 ConnectStub(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort)
{
    return MP_SUCCESS;
}

TEST_F(DppSocketTest, HandleRecvSendFail)
{
    CConnection connection;
    TestDppSocket om;
    stub.set(ADDR(CSocket, Connect), ConnectStub);
    stub.set(&CLogger::Log, StubCLoggerLog);
    StubClogToVoidLogNullPointReference();
    connection.SetClientIpAddr(16777343);   // 127.0.0.1
    connection.SetClientPort(11236);
    EXPECT_EQ(om.DppSocket::HandleRecvSendFail(connection), MP_TRUE);
    connection.SetConnectionRole(SERVER);
    EXPECT_EQ(om.DppSocket::HandleRecvSendFail(connection), MP_TRUE);

    stub.set(ADDR(CConnection, GetConnectRole), StubGetConnectRole);
    stub.set(ADDR(CMpTime, Difftime), StubDifftime);
    stub.set(ADDR(CConnection, UpdateResetConnectTime), StubVoid);
    stub.set(ADDR(DppSocket, ResetConnection), StubSuccess);
    EXPECT_EQ(om.DppSocket::HandleRecvSendFail(connection), MP_FALSE);
}

mp_int32 RecvMsgImplStub(CConnection &connection)
{
    return MP_FAILED;
}

static mp_int32 g_tryTimes = 0;
static mp_bool HandleRecvSendFailStub(CConnection &connection)
{
    ++g_tryTimes;
    return MP_FALSE;
}

TEST_F(DppSocketTest, HandleRecvMsgResetConnect)
{
    CConnection connection;
    TestDppSocket om;
    connection.SetClientIpAddr(16777343);
    connection.SetClientPort(11236);
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, HandleRecvSendFail), HandleRecvSendFailStub);
    stub.set(ADDR(DppSocket, RecvMsgImpl), RecvMsgImplStub);
    StubClogToVoidLogNullPointReference();
    om.DppSocket::RecvMsg(connection);

    //最多重试三次重连
    EXPECT_EQ(g_tryTimes,3);
}

TEST_F(DppSocketTest, CmdFilter)
{
    mp_uint32 cmd;
    TestDppSocket om;

    mp_bool bRet = om.DppSocket::CmdFilter(cmd);
    EXPECT_EQ(bRet, MP_FALSE);

    cmd = 1080;
    bRet = om.DppSocket::CmdFilter(cmd);
    EXPECT_EQ(bRet, MP_TRUE);
}

TEST_F(DppSocketTest, SendMsgImpl)
{
    TestDppSocket om;
    CDppMessage message;
    CConnection connection;
    connection.SetClientIpAddr(16777343);
    connection.SetClientPort(11236);
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    
    flag = 0;
    stub.set(ADDR(DppSocket, SendBuffer), StubFailedOnTwo);
    message.dppMessage.uiSize = 1;
    stub.set(ADDR(CDppMessage, AnalyzeManageMsg), StubSuccess);
    mp_int32 iRet = om.DppSocket::SendMsgImpl(message, connection);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.reset(ADDR(DppSocket, SendBuffer));
    stub.reset(ADDR(CDppMessage, AnalyzeManageMsg));
}

TEST_F(DppSocketTest, RecvMsgImpl)
{
    TestDppSocket om;
    CDppMessage message;
    CConnection connection;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    
    connection.ReleaseMsg();
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingState);
    mp_int32 iRet = om.DppSocket::RecvMsgImpl(connection);
    EXPECT_EQ(iRet, MP_FAILED);

    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingStateOnTwoFalse);
    iRet = om.DppSocket::RecvMsgImpl(connection);
    EXPECT_EQ(iRet, MP_FAILED);

    CConnection connections;
    flag = 0;
    stub.set(ADDR(CConnection, InReceivingState), StubInReceivingState);
    stub.set(ADDR(CConnection, GetDppMessage), StubGetDppMessage);
    stub.set(ADDR(CConnection, InRecvFlag), StubInReceivingStateOnTwoFalse);
    stub.set(ADDR(DppSocket, GetBuffer), StubFailed);
    iRet = om.DppSocket::RecvMsgImpl(connections);
    EXPECT_EQ(iRet, MP_FAILED);

    flag = 0;
    stub.set(ADDR(CConnection, InRecvFlag), StubInReceivingStateOnTwoFalse);
    stub.set(ADDR(DppSocket, GetBuffer), StubSuccess);
    stub.set(ADDR(CConnection, Recv), StubSuccess);
    stub.set(ADDR(DppSocket, CheckMsgLength), StubEagain);
    iRet = om.DppSocket::RecvMsgImpl(connections);
    EXPECT_EQ(iRet, MP_EAGAIN);

    flag = 0;
    stub.set(ADDR(CConnection, InRecvFlag), StubInReceivingStateOnTwoFalse);
    stub.set(ADDR(DppSocket, CheckMsgLength), StubSuccess);
    stub.set(ADDR(DppSocket, ReceiveAllMsg), StubFailed);
    iRet = om.DppSocket::RecvMsgImpl(connections);
    EXPECT_EQ(iRet, MP_FAILED);

    flag = 0;
    stub.set(ADDR(DppSocket, ReceiveAllMsg), StubSuccess);
    iRet = om.DppSocket::RecvMsgImpl(connections);
    EXPECT_EQ(iRet, MP_SUCCESS);

    stub.reset(ADDR(CConnection, InReceivingState));
}

TEST_F(DppSocketTest, GetConnectErr)
{
    TestDppSocket om;
    mp_int32 iErr;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(ADDR(DppSocket, CloseConnect), StubCloseConnect);
    
    stub.set(ADDR(DppSocket, CheckOSErr), StubCheckOSErr);
    mp_int32 iRet = om.DppSocket::GetConnectErr(iErr);
    EXPECT_EQ(iRet, MP_FAILED);
}