#include "message/tcp/SocketTest.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "message/tcp/TCPClientHandler.h"

namespace {
mp_int32 flag = 0;

mp_int32 StubFailed()
{
    return MP_FAILED;
}

mp_int32 StubSuccess()
{
    return MP_SUCCESS;
}

mp_int32 StubSelectTimeOut()
{
    return 0;
}

mp_int32 StubSelect()
{
    return 1;
}

mp_bool StubTrue()
{
    return MP_TRUE;
}

mp_int32 StubReturnO_NONBLOCK()
{
    return O_NONBLOCK;
}

mp_bool StubFalse()
{
    return MP_FALSE;
}

mp_int32 StubFailedOnTwo()
{
    if (flag++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 Stubmemset_s()
{
    return 1;
}

mp_int32 Stubmemset_sEOK()
{
    return 0;
}

mp_int32 StubWaitRecvEventZero(mp_socket sock, mp_uint32 uiSecondes)
{
    return 0;
}

mp_int32 StubWaitSendEventZero(mp_socket sock, mp_uint32 uiSecondes)
{
    return 0;
}

mp_int32 StubGetOSErrorTwo()
{
    mp_int32 numFour = 4;
    if (flag++ < 1) {
        return numFour;
    }
    return MP_FAILED;
}

mp_int32 StubFcntlFailedOnTwo()
{
    if (flag++ < 1) {
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 Stubrecv()
{
    return 1;
}
mp_int32 Stubsend()
{
    return 1;
}
}  // namespace

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubCSocketGetValueInt32Return);                                                                           \
    } while (0)

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

/*
 * 测试用例：初始化CSocket
 * 前置条件：无
 * CHECK点：注册信号成功初始化成功，注册信号失败初始化失败
 */
TEST_F(CSocketTest, Init)
{
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(SignalRegister, StubSignalRegisterFailed);
    mp_int32 iRet = om.Init();
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(SignalRegister, StubSignalRegister);
    iRet = om.Init();
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 测试用例：创建TcpSocket
 * 前置条件：无
 * CHECK点：1.插件套接字无效 2.fcntl失败则创建失败
 */
TEST_F(CSocketTest, CreateTcpSocket)
{
    mp_socket sock = 15;
    mp_bool keepSocketInherit = false;
    mp_bool isIPV4 = true;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(SignalRegister, StubSignalRegisterFailed);
    mp_int32 iRet = om.CreateTcpSocket(sock, keepSocketInherit, isIPV4);
    EXPECT_EQ(MP_SUCCESS, iRet);

    isIPV4 = false;
    stub.set(socket, StubSocketInvalid);
    iRet = om.CreateTcpSocket(sock, keepSocketInherit, isIPV4);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset(socket);
    stub.set(fcntl, StubFailed);
    iRet = om.CreateTcpSocket(sock, keepSocketInherit, isIPV4);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    flag = 0;
    stub.set(fcntl, StubFailedOnTwo);
    iRet = om.CreateTcpSocket(sock, keepSocketInherit, isIPV4);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
}

TEST_F(CSocketTest, SetReuseAddr)
{
    mp_socket sock = 15;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.SetReuseAddr(sock);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 测试用例：开始监听socket
 * 前置条件：socket创建成功
 * CHECK点：监听socket
 */
TEST_F(CSocketTest, StartListening)
{
    mp_socket sock = 15;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.StartListening(sock);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(listen, StubSuccess);
    iRet = om.StartListening(sock);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 测试用例：socket绑定ip和port
 * 前置条件：socket创建成功
 * CHECK点：1.ip为Ipv6，使用Ipv6绑定 2.ip为Ipv4串绑定成功
 */
TEST_F(CSocketTest, Bind)
{
    mp_socket sock = 15;
    mp_string strIp = "192.168";
    mp_uint16 uiPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.Bind(sock, strIp, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    strIp = "2001:0db8:86a3:08d3:1319:8a2e:0370:7344";
    stub.set(ADDR(CSocket, BindIpv6), StubSuccess);
    iRet = om.Bind(sock, strIp, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    strIp = "";
    stub.set(bind, StubSuccess);
    iRet = om.Bind(sock, strIp, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 测试用例：socket绑定ip和port
 * 前置条件：socket创建成功
 * CHECK点：1.ip和port符合条件绑定成功 2.转化ip地址为二进制是否成功
 */
TEST_F(CSocketTest, BindIpv6)
{
    mp_socket sock = 15;
    mp_string strIp = "2001:0db8:86a3:08d3:1319:8a2e:0370:7344";
    mp_uint16 uiPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(inet_pton, StubTrue);
    stub.set(bind, StubSuccess);
    mp_int32 iRet = om.BindIpv6(sock, strIp, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    strIp = "[2]";
    uiPort = 0;
    stub.set(inet_pton, StubFalse);
    iRet = om.BindIpv6(sock, strIp, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(inet_pton, StubTrue);
    stub.set(bind, StubFailed);
    iRet = om.BindIpv6(sock, strIp, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);
}

/*
 * 测试用例：创建连接
 * 前置条件：socket创建成功
 * CHECK点：连接失败返回系统系统错误码
 */
TEST_F(CSocketTest, Connect)
{
    mp_socket clientSock = 15;
    mp_uint32 uiServerAddr = 192168;
    mp_uint16 uiPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.Connect(clientSock, uiServerAddr, uiPort);
    EXPECT_NE(MP_SUCCESS, iRet);

    stub.set(connect, StubSuccess);
    stub.set(memset_s, Stubmemset_s);
    iRet = om.Connect(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(memset_s, Stubmemset_sEOK);
    stub.set(getsockname, StubFailed);
    iRet = om.Connect(clientSock, uiServerAddr, uiPort);
    stub.set(getsockname, StubSuccess);
    iRet = om.Connect(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 测试用例：接收连接
 * 前置条件：socket创建成功
 * CHECK点：socket无效，接收失败，否则接收连接成功
 */
TEST_F(CSocketTest, AcceptClient)
{
    mp_socket serverSock = 10;
    mp_socket clientSock = 15;
    mp_uint32 uiServerAddr = 192168;
    mp_uint32 uiClientAddr = 168192;
    mp_uint16 uiClientPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.AcceptClient(serverSock, uiClientAddr, uiClientPort, clientSock);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(accept, StubSuccess);
    iRet = om.AcceptClient(serverSock, uiClientAddr, uiClientPort, clientSock);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CSocketTest, Close)
{
    mp_socket sock = 10;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.Close(sock);
}

TEST_F(CSocketTest, Recv)
{
    mp_socket sock = 10;
    mp_char* buff = "test";
    mp_uint32 iBuffLen = 10;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.Recv(sock, buff, iBuffLen);
}

/*
 * 测试用例：接收socket中的字符串
 * 前置条件：无
 * CHECK点：1.接收是否超时，超时则失败 2.未超时接收字符串到缓存区
 */
TEST_F(CSocketTest, RecvBuffer)
{
    mp_socket sock = 10;
    mp_char* buff = "test";
    mp_uint32 iBuffLen = 5;
    mp_uint32 uiTimeOut = 2;
    mp_uint32 iRecvLen = 0;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(ADDR(CSocket, WaitRecvEvent), StubWaitRecvEventZero);
    iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(SOCKET_TIMEOUT, iRet);

    stub.set(ADDR(CSocket, WaitRecvEvent), StubWaitRecvEvent);
    stub.set(recv, StubSuccess);
    iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(MP_SUCCESS, iRet);

    flag = 0;
    stub.set(recv, StubFailed);
    stub.set(GetOSError, StubGetOSErrorTwo);
    iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(recv, Stubrecv);
    iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iBuffLen = 0;
    iRet = om.RecvBuffer(sock, buff, iBuffLen, uiTimeOut, iRecvLen);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

TEST_F(CSocketTest, Send)
{
    mp_socket sock = 10;
    mp_char* buff = "test";
    mp_uint32 iBuffLen = 10;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.Send(sock, buff, iBuffLen);
}

/*
 * 测试用例：发送字符串
 * 前置条件：无
 * CHECK点：1.等待发送是否超时，超时则失败 2.未超时发动字符串成功
 */
TEST_F(CSocketTest, SendBuffer)
{
    mp_socket sock = 10;
    mp_char* buff = "test";
    mp_uint32 iBuffLen = 5;
    mp_uint32 uiTimeOut = 2;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.SendBuffer(sock, buff, iBuffLen, uiTimeOut);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSocket::WaitSendEvent, StubWaitSendEventZero);
    iRet = om.SendBuffer(sock, buff, iBuffLen, uiTimeOut);
    EXPECT_EQ(SOCKET_TIMEOUT, iRet);

    stub.set(&CSocket::WaitSendEvent, StubWaitSendEvent);
    stub.set(GetOSError, StubGetOSErrorTwo);
    iRet = om.SendBuffer(sock, buff, iBuffLen, uiTimeOut);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(send, Stubsend);
    iRet = om.SendBuffer(sock, buff, iBuffLen, uiTimeOut);
    mp_int32 numFive = 5;
    EXPECT_EQ(numFive, iRet);
    stub.reset(GetOSError);
    stub.reset(CSocket::WaitSendEvent);
}

/*
 * 测试用例：设置文件描述符的FD_CLOEXEC标志
 * 前置条件：无
 * CHECK点：检查返回值
 */
TEST_F(CSocketTest, SetFdCloexec)
{
    mp_uint32 fd = 10;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.SetFdCloexec(fd);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);

    stub.set(fcntl, StubSuccess);
    iRet = om.SetFdCloexec(fd);
    EXPECT_EQ(MP_SUCCESS, iRet);

    flag = 0;
    stub.set(fcntl, StubFcntlFailedOnTwo);
    iRet = om.SetFdCloexec(fd);
    EXPECT_EQ(ERROR_COMMON_OPER_FAILED, iRet);
}

/*
 * 测试用例：设置socket为非阻塞式
 * 前置条件：无
 * CHECK点：检查返回值
 */
TEST_F(CSocketTest, SetNonBlocking)
{
    mp_socket sock = 10;
    mp_bool bNonBlock = true;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.SetNonBlocking(sock, bNonBlock);

    bNonBlock = false;
    om.SetNonBlocking(sock, bNonBlock);
}

/*
 * 测试用例：获得socket非阻塞式标志
 * 前置条件：无
 * CHECK点：检查返回值
 */
TEST_F(CSocketTest, GetNonBlocking)
{
    mp_socket sock = 10;
    mp_bool bNonBlock = true;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(fcntl, StubReturnO_NONBLOCK);
    om.GetNonBlocking(sock, bNonBlock);
}

/*
 * 测试用例：创建Client端socket
 * 前置条件：无
 * CHECK点：创建TcpSocket是否成功
 */
TEST_F(CSocketTest, CreateClientSocket)
{
    mp_socket clientSock = 10;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.CreateClientSocket(clientSock);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(&CSocket::CreateTcpSocket, StubFailed);
    iRet = om.CreateClientSocket(clientSock);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(&CSocket::CreateTcpSocket);
}

TEST_F(CSocketTest, FdInit)
{
    fd_set fdSet;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.FdInit(fdSet);
}

TEST_F(CSocketTest, FdSet)
{
    mp_socket sock = 10;
    fd_set fdSet;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.FdSet(sock, fdSet);
}

TEST_F(CSocketTest, FdClr)
{
    mp_socket sock = 10;
    fd_set fdSet;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.FdClr(sock, fdSet);
}

TEST_F(CSocketTest, FdIsSet)
{
    mp_socket sock = 10;
    fd_set fdSet;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.FdIsSet(sock, fdSet);
}

/*
 * 测试用例：创建Server端socket
 * 前置条件：无
 * CHECK点：1.创建TcpSocket是否成功 2.绑定ip和端口是否成功 3.是否开始监听
 */
TEST_F(CSocketTest, CreateServerSocket)
{
    mp_socket servSock = 10;
    mp_char* ip = "192.168";
    mp_uint16 uiPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.CreateServerSocket(servSock, ip, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSocket::CreateTcpSocket, StubFailed);
    iRet = om.CreateServerSocket(servSock, ip, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSocket::CreateTcpSocket, StubSuccess);
    stub.set(&CSocket::SetReuseAddr, StubFailed);
    iRet = om.CreateServerSocket(servSock, ip, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSocket::SetReuseAddr, StubSuccess);
    stub.set(&CSocket::Bind, StubSuccess);
    stub.set(&CSocket::StartListening, StubFailed);
    iRet = om.CreateServerSocket(servSock, ip, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CSocket::StartListening, StubSuccess);
    iRet = om.CreateServerSocket(servSock, ip, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.reset(&CSocket::CreateTcpSocket);
    stub.reset(&CSocket::SetReuseAddr);
    stub.reset(&CSocket::Bind);
    stub.reset(&CSocket::StartListening);
}

TEST_F(CSocketTest, WaitEvents)
{
    fd_set fdRead;
    fd_set fdWrite;
    mp_int32 iMaxFd = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(GetOSError, StubGetOSErrorTwo);
    mp_int32 iRet = om.WaitEvents(fdRead, fdWrite, iMaxFd);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(select, StubSelectTimeOut);
    iRet = om.WaitEvents(fdRead, fdWrite, iMaxFd);
    EXPECT_EQ(0, iRet);

    stub.set(select, StubSelect);
    iRet = om.WaitEvents(fdRead, fdWrite, iMaxFd);
    EXPECT_EQ(1, iRet);
}

TEST_F(CSocketTest, WaitRecvEvent)
{
    mp_socket sock = 10;
    mp_int32 uiSecondes = 2;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.WaitRecvEvent(sock, uiSecondes);
    EXPECT_EQ(-1, iRet);
}

TEST_F(CSocketTest, WaitSendEvent)
{
    mp_socket sock = 10;
    mp_int32 uiSecondes = 2;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.WaitSendEvent(sock, uiSecondes);
    EXPECT_EQ(-1, iRet);
}

/*
 * 测试用例：使用Ipv6连接
 * 前置条件：无
 * CHECK点：1.能否将IP地址从字符串格式转换成网络地址格式 2.是否创建连接成功
 */
TEST_F(CSocketTest, ConnectIpv6)
{
    mp_socket clientSock = 10;
    mp_string uiServerAddr = "[1921]";
    mp_uint16 uiPort = 1001;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(-2, iRet);

    stub.set(inet_pton, StubFailed);
    iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(inet_pton, StubTrue);
    stub.set(connect, StubFailed);
    stub.set(GetOSError, StubFailed);
    iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(GetOSError);

    stub.set(connect, StubSuccess);
    stub.set(memset_s, Stubmemset_s);
    iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(memset_s, Stubmemset_sEOK);
    stub.set(getsockname, StubFailed);
    iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(getsockname, StubSuccess);
    iRet = om.ConnectIpv6(clientSock, uiServerAddr, uiPort);
    EXPECT_EQ(MP_SUCCESS, iRet);
}

/*
 * 测试用例：检查socket连接状态
 * 前置条件：socket创建成功
 * CHECK点：1.检测套接字状态 2.套接口的选项当前值
 */
TEST_F(CSocketTest, CheckSockLinkStatus)
{
    mp_socket sock = 10;
    mp_int32 timeout = 2;
    CSocket om;
    stub.reset(select);
    stub.set(select, StubSelectTimeOut);
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.CheckSockLinkStatus(sock, timeout);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(select, StubSelect);
    stub.set(getsockopt, StubFailed);
    iRet = om.CheckSockLinkStatus(sock, timeout);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(getsockopt, StubSuccess);
    iRet = om.CheckSockLinkStatus(sock, timeout);
    EXPECT_EQ(MP_FAILED, iRet);
    stub.reset(select);
    stub.reset(getsockopt);
}

/*
 * 测试用例：检查Host连接状态
 * 前置条件：无
 * CHECK点：检查返回值
 */
TEST_F(CSocketTest, CheckHostLinkStatus)
{
    mp_string strSrcIp = "192.168";
    mp_string strHostIp = "192.167";
    mp_uint16 uiPort = 100;
    mp_int32 timeout = 2;
    CSocket om;
    stub.set(&CLogger::Log, StubCLoggerLog);
    stub.set(&CIP::IsIPV4, StubIsIPV4);
    mp_int32 iRet = om.CheckHostLinkStatus(strSrcIp, strHostIp, uiPort, timeout);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CIP::IsIPV4, StubIsIPV4False);
    stub.set(&CIP::IsIPv6, StubIsIPV6False);
    iRet = om.CheckHostLinkStatus(strSrcIp, strHostIp, uiPort, timeout);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.set(&CIP::IsIPv6, StubIsIPV6);
    stub.set(&CSocket::CreateTcpSocket, StubFailed);
    iRet = om.CheckHostLinkStatus(strSrcIp, strHostIp, uiPort, timeout);
    EXPECT_EQ(MP_FAILED, iRet);

    strSrcIp = "";
    stub.set(&CIP::IsIPv6, StubIsIPV6);
    stub.set(&CSocket::CreateTcpSocket, StubSuccess);
    stub.set(&CSocket::ConnectIpv6, StubSuccess);
    iRet = om.CheckHostLinkStatus(strSrcIp, strHostIp, uiPort, timeout);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set(&CSocket::ConnectIpv6, StubFailed);
    stub.set(&CSocket::CheckSockLinkStatus, StubSuccess);
    iRet = om.CheckHostLinkStatus(strSrcIp, strHostIp, uiPort, timeout);
    EXPECT_EQ(MP_FAILED, iRet);
}