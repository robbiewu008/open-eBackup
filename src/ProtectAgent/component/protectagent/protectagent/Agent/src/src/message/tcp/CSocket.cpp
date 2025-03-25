#include "message/tcp/CSocket.h"
#include "common/Log.h"
#include "common/TimeOut.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "common/Ip.h"

mp_int32 CSocket::Init()
{
    COMMLOG(OS_LOG_DEBUG, "Begin init socket.");
#ifdef WIN32
    struct WSAData wd;
    mp_uint16 wVersionRequest = MAKEWORD(1, 1);
    mp_int32 iRet = MP_SUCCESS;
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;

    iRet = WSAStartup(wVersionRequest, &wd);
    if (iRet != 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "WSAStartup failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#else
    mp_int32 iRet = MP_SUCCESS;

    iRet = SignalRegister(SIGPIPE, SIG_IGN);
    if (MP_SUCCESS != iRet) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "Invoke signal register failed, errno[%d]:%s.",
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
#endif

    COMMLOG(OS_LOG_DEBUG, "Init socket succ.");
    return MP_SUCCESS;
}

mp_int32 CSocket::CreateTcpSocket(mp_socket& sock, mp_bool keepSocketInherit, mp_bool isIPV4)
{
#ifdef WIN32
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        mp_int32 err = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("WSAStartup failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        ERRLOG("WSADATA wVersion failed, LOBYTE=%d, HIBYTE=%d.", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        WSACleanup();
        return MP_FAILED;
    }
#endif
    if (isIPV4) {
        sock = socket(AF_INET, SOCK_STREAM, 0);
    } else {
        sock = socket(AF_INET6, SOCK_STREAM, 0);
    }

    if (sock == MP_INVALID_SOCKET) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        COMMLOG(OS_LOG_ERROR, "Invoke socket failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
#ifdef WIN32
        WSACleanup();
#endif
        return MP_FAILED;
    }

    if (keepSocketInherit == MP_FALSE) {
        // 设置文件描述符的FD_CLOEXEC标志，保证fork出来的子进程不继承父进程的资源
#ifndef WIN32
        mp_int32 iFlag = fcntl(sock, F_GETFD);
        if (iFlag == MP_FAILED) {
            COMMLOG(OS_LOG_ERROR, "fcntl failed! sock = %d\n.", sock);
            CSocket::Close(sock);
            return ERROR_COMMON_OPER_FAILED;
        }
        iFlag = iFlag | FD_CLOEXEC;
        mp_int32 iRet = fcntl(sock, F_SETFD, iFlag);
        if (iRet == MP_FAILED) {
            COMMLOG(OS_LOG_ERROR, "fcntl failed! sock = %d\n.", sock);
            CSocket::Close(sock);
            return ERROR_COMMON_OPER_FAILED;
        }
#endif
    }
    INFOLOG("Create socket succ, socket %d.", sock);
    return MP_SUCCESS;
}

mp_int32 CSocket::SetReuseAddr(mp_socket sock)
{
    mp_int32 iRet = MP_SUCCESS;
    sock_option_t option = 1;

    COMMLOG(OS_LOG_DEBUG, "Begin set reuse addr, sock %d.", sock);
    iRet = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, (mp_char*)&option, sizeof(mp_int32));
    if (-1 == iRet) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "Invoke setsockopt failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Set reuse addr succ.");
    return MP_SUCCESS;
}

mp_int32 CSocket::StartListening(mp_socket sock)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "Begin start listening, sock %d.", sock);
    mp_int32 listenLen = 5;
    iRet = listen(sock, listenLen);
    if (iRet != 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR,
            "Start listening sock %d failed, errno[%d]:%s.",
            sock,
            iErr,
            GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 CSocket::Bind(mp_socket sock, const mp_string& strIp, mp_uint16 uiPort)
{
    // ipv6 bind
    if (strIp.find(":") != std::string::npos) {
        return BindIpv6(sock, strIp, uiPort);
    }

    struct sockaddr_in localAddr;
    COMMLOG(OS_LOG_DEBUG, "Bind info, sock %d, ip %s, port %d.", sock, strIp.c_str(), uiPort);
    (mp_void) memset_s(&localAddr, sizeof(localAddr), 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    if (uiPort != 0) {
        localAddr.sin_port = htons(uiPort);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Set socket %d bind port to any.", sock);
    }

    if (strIp.length() != 0) {
        localAddr.sin_addr.s_addr = inet_addr(strIp.c_str());
    } else {
        COMMLOG(OS_LOG_DEBUG, "Set socket %d bind ip to any.", sock);
        localAddr.sin_addr.s_addr = INADDR_ANY;
    }

    if (bind(sock, (struct sockaddr*)(mp_void*)&localAddr, sizeof(sockaddr)) != 0) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "Invoke bind failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "Bind succ.");
    return MP_SUCCESS;
}

mp_int32 CSocket::BindIpv6(mp_socket sock, const mp_string& strIp, mp_uint16 uiPort)
{
    static const mp_int32 ipv6number = 2;
    std::string localIP = strIp;
    if (localIP.find("[") != std::string::npos) {
        localIP = localIP.substr(1, localIP.length() - ipv6number);
    }
    struct sockaddr_in6 localAddr;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};

    COMMLOG(OS_LOG_DEBUG, "BindIpv6 info, sock %d, ip %s, port %d.", sock, localIP.c_str(), uiPort);
    (mp_void) memset_s(&localAddr, sizeof(localAddr), 0, sizeof(localAddr));
    localAddr.sin6_family = AF_INET6;
    if (uiPort != 0) {
        localAddr.sin6_port = htons(uiPort);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Set socket %d bind port to any.", sock);
    }

    if (localIP.length() == 0) {
        localIP = "::";
        COMMLOG(OS_LOG_DEBUG, "Set socket %d bind ip to any.", sock);
    }

    mp_int32 iRet = inet_pton(AF_INET6, localIP.c_str(), &localAddr.sin6_addr);
    if (iRet != 1) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Invoke inet_pton failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    if (bind(sock, (struct sockaddr*)&localAddr, sizeof(sockaddr_in6)) != 0) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Invoke bind failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_DEBUG, "BindIpv6 succ.");
    return MP_SUCCESS;
}

mp_int32 CSocket::Connect(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort)
{
    mp_int32 iRet = MP_SUCCESS;
    sockaddr_in peer;

    INFOLOG("Begin connect [%u:%u].", uiServerAddr, uiPort);
    peer.sin_family = AF_INET;
    peer.sin_addr.s_addr = uiServerAddr;
    peer.sin_port = htons(uiPort);

    iRet = connect(clientSock, (struct sockaddr*)&peer, sizeof(sockaddr_in));
    if (MP_SUCCESS != iRet) {
        mp_int32 iErr = GetOSError(); // Do not use other means to replace
        mp_char szErr[256] = {0};
        ERRLOG("Invoke connect failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return iErr; // 外层调用可能会判断connect返回的系统错误码，需返回iErr
    }

    // get client socket port
    sockaddr_in localAddr;
    socklen_t len = sizeof(localAddr);
    iRet = memset_s(static_cast<void*>(&localAddr), len, 0, len);
    if (iRet != EOK) {
        WARNLOG("Connect %u:%u succ, but get local socket info failed, errno[%d].", uiServerAddr, uiPort, iRet);
    } else {
        DBGLOG("Begin get sock %d name.", clientSock);
        if (MP_FAILED == getsockname(clientSock, (struct sockaddr*)&localAddr, &len)) {
            INFOLOG("Connect %u:%u succ, but get local socket info failed.", uiServerAddr, uiPort);
        } else {
            INFOLOG("Connect %u:%u succ, local port:%u.", uiServerAddr, uiPort, ntohs(localAddr.sin_port));
        }
    }
    return MP_SUCCESS;
}

mp_int32 CSocket::AcceptClient(
    mp_socket serverSock, mp_uint32& uiClientAddr, mp_uint16& uiClientPort, mp_socket& clientSock)
{
    struct sockaddr_in sa;
    socklen_t uiLen = sizeof(sockaddr);

    COMMLOG(OS_LOG_DEBUG, "Begin accept client.");
    clientSock = accept(serverSock, (struct sockaddr*)&sa, &uiLen);
    if (MP_INVALID_SOCKET == clientSock) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        COMMLOG(OS_LOG_ERROR, "Invoke accept failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    uiClientAddr = sa.sin_addr.s_addr;
    uiClientPort = htons(sa.sin_port);
    COMMLOG(OS_LOG_DEBUG, "Accept client succ.");

    return MP_SUCCESS;
}

mp_void CSocket::Close(mp_socket sock)
{
#ifndef WIN32
    shutdown(sock, SHUT_RDWR);
#endif
    mp_close_socket(sock);
#ifdef WIN32
    WSACleanup();
#endif
    INFOLOG("Close sock, sock %d.", sock);
}

mp_int32 CSocket::Recv(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen)
{
    return recv(sock, buff, iBuffLen, 0);
}

#ifdef SUPPORT_SSL
mp_int32 CSocket::SslRecv(SSL* ssl, mp_char* buff, mp_uint32 ibufLen)
{
    return SSL_read(ssl, buff, ibufLen);
}

mp_int32 CSocket::SslSend(SSL* ssl, mp_char* buff, mp_uint32 iBuffLen)
{
    return SSL_write(ssl, buff, iBuffLen);
}
#endif

mp_int32 CSocket::RecvBuffer(
    mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut, mp_uint32& iRecvLen)
{
    mp_int32 iRet = MP_SUCCESS;
    mp_uint32 iRemainDataLen = iBuffLen;
    mp_int32 iWaitRet = 0;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    CTimeOut cTimtout(uiTimeOut);

    COMMLOG(OS_LOG_DEBUG, "Begin recv buff, sock %d, bufflen %d.", sock, iBuffLen);
    iRecvLen = 0;
    SetNonBlocking(sock, MP_TRUE);

    while (iRemainDataLen > 0) {
        iWaitRet = WaitRecvEvent(sock, cTimtout.Remaining());
        if (-1 == iWaitRet) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR,
                "Wait recv event failed, waitRet %d, errno[%d]:%s.",
                iWaitRet,
                iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }

        if (iWaitRet == 0) {
            COMMLOG(OS_LOG_ERROR, "Wait recv events timeout.");
            return SOCKET_TIMEOUT;
        }

        iRet = recv(sock, buff, iRemainDataLen, 0);
        if (iRet == 0) {
            COMMLOG(OS_LOG_INFO, "Connection is closed by peer.");
            return MP_SUCCESS;  // if socket was closed by peer, return 0
        }

        if (iRet < 0) {
            iErr = GetOSError();
            // recv was interrupted by singal, continue to recv
            if (EINTR == iErr) {  // EWOULDBLOCK ??
                continue;
            }

            COMMLOG(OS_LOG_ERROR, "Invoke recv failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }

        iRemainDataLen -= iRet;
        iRecvLen += iRet;
        buff += iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "End recv buff.");
    return MP_SUCCESS;
}

mp_int32 CSocket::Send(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen)
{
    return send(sock, buff, iBuffLen, 0);
}

mp_int32 CSocket::SendBuffer(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut)
{
    CTimeOut cTimtout(uiTimeOut);
    mp_uint32 iRemainDataLen = iBuffLen;
    mp_uint32 iSendLen = 0;  // return value
    mp_int32 iWaitRet = 0;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};

    COMMLOG(OS_LOG_DEBUG, "Begin send buff, sock %d, bufflen %d.", sock, iBuffLen);
    SetNonBlocking(sock, MP_TRUE);
    while (iRemainDataLen > 0) {
        iWaitRet = WaitSendEvent(sock, cTimtout.Remaining());
        if (-1 == iWaitRet) {
            iErr = GetOSError();
            COMMLOG(OS_LOG_ERROR,
                "Wait send event failed, waitRet %d, errno[%d]:%s.",
                iWaitRet,
                iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
            iSendLen = -1;
            break;
        }

        if (iWaitRet == 0) {
            iSendLen = SOCKET_TIMEOUT;  // select time out
            COMMLOG(OS_LOG_ERROR, "Wait send events timeout.");
            break;
        }

        iRet = send(sock, buff, iRemainDataLen, 0);
        if (iRet < 0) {
            iErr = GetOSError();
            // send was interrupted by singal, continue to send
            if (EINTR == iErr) {  // EWOULDBLOCK ??
                continue;
            }

            iSendLen = MP_FAILED;
            COMMLOG(OS_LOG_ERROR, "Invoke send failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            break;
        }

        iRemainDataLen -= iRet;
        iSendLen += iRet;
        buff += iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "End send buff, ret %d.", iSendLen);
    return iSendLen;
}

mp_int32 CSocket::SetFdCloexec(mp_int32 fd)
{
    mp_char szErr[256] = {0};
    mp_int32 iErr = 0;
    // 设置文件描述符的FD_CLOEXEC标志，保证fork出来的子进程不继承父进程的资源
#ifndef WIN32
    mp_int32 iFlag = fcntl(fd, F_GETFD);
    if (iFlag == MP_FAILED) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "fcntl failed! errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        Close(fd);
        return ERROR_COMMON_OPER_FAILED;
    }
    iFlag = iFlag | FD_CLOEXEC;
    mp_int32 iRet = fcntl(fd, F_SETFD, iFlag);
    if (iRet == MP_FAILED) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "fcntl failed! errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        Close(fd);
        return ERROR_COMMON_OPER_FAILED;
    }
#endif
    return MP_SUCCESS;
}

mp_void CSocket::SetNonBlocking(mp_socket sock, mp_bool bNonBlock)
{
#ifdef WIN32
    mp_ulong mode = bNonBlock ? 1 : 0;
    ioctlsocket(sock, FIONBIO, &mode);
#else
    mp_int32 flags = fcntl(sock, F_GETFL, 0);
    if (bNonBlock)
        flags |= O_NONBLOCK;
    else
        flags &= ~O_NONBLOCK;
    fcntl(sock, F_SETFL, flags);
#endif
    COMMLOG(OS_LOG_DEBUG, "Set non blocking, sock %d, bNonBlock %d.", sock, bNonBlock);
}

mp_void CSocket::GetNonBlocking(mp_socket sock, mp_bool& bNonBlock)
{
#ifndef WIN32
    mp_int32 flags = fcntl(sock, F_GETFL, 0);
    if ((flags & O_NONBLOCK) == O_NONBLOCK) {
        bNonBlock = MP_TRUE;
    }
    bNonBlock = MP_FALSE;
#endif
}

mp_int32 CSocket::CreateClientSocket(mp_socket& clientSock)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, "Begin create client socket.");
    iRet = CreateTcpSocket(clientSock);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create tcp socket failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create client socket succ, client sock %d.", clientSock);
    return MP_SUCCESS;
}

mp_int32 CSocket::CreateServerSocket(mp_socket& servSock, const mp_string& ip, mp_uint16 uiPort)
{
    mp_int32 iRet = MP_SUCCESS;

    COMMLOG(OS_LOG_DEBUG, "Begin create server socket, ip %s, port %d.", ip.c_str(), uiPort);
    iRet = CreateTcpSocket(servSock);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create tcp socket failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = SetReuseAddr(servSock);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Set resue addr failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = Bind(servSock, ip, uiPort);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Bind failed, iRet %d.", iRet);
        return iRet;
    }

    iRet = StartListening(servSock);
    if (MP_SUCCESS != iRet) {
        COMMLOG(OS_LOG_ERROR, "Start listening failed, iRet %d.", iRet);
        return iRet;
    }

    COMMLOG(OS_LOG_DEBUG, "Create server socket succ, server sock %d.", servSock);
    return MP_SUCCESS;
}

mp_void CSocket::FdInit(fd_set& fdSet)
{
    FD_ZERO(&fdSet);
}

mp_void CSocket::FdSet(mp_socket sock, fd_set& fdSet)
{
    FD_SET(sock, &fdSet);
}

mp_void CSocket::FdClr(mp_socket sock, fd_set& fdSet)
{
    FD_CLR(sock, &fdSet);
}

mp_bool CSocket::FdIsSet(mp_socket sock, fd_set& fdset)
{
    if (FD_ISSET(sock, &fdset)) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_int32 CSocket::WaitEvents(fd_set& fdRead, fd_set& fdWrite, mp_int32 iMaxFd)
{
    static const mp_uint32 SELECT_TIMEOUT = 1;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    timeval tv;
    tv.tv_sec = SELECT_TIMEOUT;
    tv.tv_usec = 0;

    for (;;) {
        iRet = select(iMaxFd + 1, &fdRead, NULL, NULL, &tv);
        if (iRet < 0) {
            iErr = GetOSError();
            if (EINTR == iErr) {
                continue;
            }

            COMMLOG(OS_LOG_ERROR, "Invoke select failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }

        // select timeout
        if (iRet == 0) {
            return iRet;
        }

        break;  // select succ
    }

    COMMLOG(OS_LOG_DEBUG, "End wait events, iRet %d.", iRet);
    return iRet;
}

#ifdef WIN32
// private
mp_int32 CSocket::WaitRecvEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    fd_set readfds;
    struct timeval timeout = {uiSecondes, 0};
    struct timeval* pTimeout = (uiSecondes == TIMEOUT_INFINITE) ? NULL : &timeout;

    FD_ZERO(&readfds);
    FD_SET(sock, &readfds);

    return select(sock + 1, &readfds, NULL, NULL, pTimeout);
}

mp_int32 CSocket::WaitSendEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    fd_set writefds;
    struct timeval timeout = {uiSecondes, 0};
    struct timeval* pTimeout = (uiSecondes == TIMEOUT_INFINITE) ? NULL : &timeout;

    FD_ZERO(&writefds);
    FD_SET(sock, &writefds);

    return select(sock + 1, NULL, &writefds, NULL, pTimeout);
}

#else
mp_int32 CSocket::WaitEvents(mp_socket sock)
{
    static const mp_uint32 POLL_TIMEOUT = 1;
    mp_int32 iRet = MP_SUCCESS;
    mp_int32 iErr = 0;
    mp_char szErr[256] = {0};
    std::vector<struct pollfd> fds;

    struct pollfd pfd;
    pfd.fd = sock;
    pfd.events = POLLIN; // 监听读事件
    pfd.revents = 0;
    fds.push_back(pfd);

    for (;;) {
        iRet = poll(fds.data(), fds.size(), POLL_TIMEOUT * MILLI_SECOND);
        if (iRet < 0) {
            iErr = GetOSError();
            if (EINTR == iErr) {
                continue;
            }
            COMMLOG(OS_LOG_ERROR, "Fd %d Invoke poll failed, errno[%d]:%s.",
                pfd.fd, iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
            return MP_FAILED;
        }

        COMMLOG(OS_LOG_DEBUG, "Fd %d iRet is %d.", pfd.fd, iRet);
        for (int i = 0; i < fds.size(); i++) {
            if (fds[i].revents & POLLIN) {
                return MP_DATA_VALID;
            }
        }

        // poll timeout
        if (iRet == 0) {
            return iRet;
        }

        break;  // poll succ
    }

    COMMLOG(OS_LOG_DEBUG, "End wait events, iRet %d.", iRet);
    return iRet;
}

// private
mp_int32 CSocket::WaitRecvEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLIN; // 监听读事件

    int timeout = (uiSecondes == TIMEOUT_INFINITE) ? -1 : uiSecondes * MILLI_SECOND; // uiSecondes单位为秒

    return poll(fds, 1, timeout);
}

mp_int32 CSocket::WaitSendEvent(mp_socket sock, mp_uint32 uiSecondes)
{
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLOUT; // 监听读事件

    int timeout = (uiSecondes == TIMEOUT_INFINITE) ? -1 : uiSecondes * MILLI_SECOND; // uiSecondes单位为秒

    return poll(fds, 1, timeout);
}
#endif

mp_int32 CSocket::ConnectIpv6(mp_socket clientSock, const mp_string& uiServerAddr, mp_uint16 uiPort)
{
    mp_int32 iRet = MP_SUCCESS;
    sockaddr_in6 peer;

    INFOLOG("Begin connect [%s:%u].", uiServerAddr.c_str(), uiPort);
    memset_s(&peer, sizeof(sockaddr_in6), 0, sizeof(sockaddr_in6));
    peer.sin6_family = AF_INET6;
    iRet = inet_pton(AF_INET6, uiServerAddr.c_str(), &peer.sin6_addr);
    if (iRet != 1) {
        if (iRet == 0) {
            const mp_int32 notValidIp = -2;
            return notValidIp;
        } else {  // return -1
            ERRLOG("Convert ipv6 string to numeric failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
    }
    peer.sin6_port = htons(uiPort);

    iRet = connect(clientSock, (struct sockaddr*)&peer, sizeof(sockaddr_in6));
    if (MP_SUCCESS != iRet) {
        mp_int32 iErr = GetOSError();
        mp_char szErr[256] = {0};
        ERRLOG("Invoke connect failed, errno[%d]:%s.", iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return iErr; // 外层调用可能会判断connect返回的系统错误码，需返回iErr
    }

    // get client socket port
    sockaddr_in6 localAddr;
    socklen_t len = sizeof(localAddr);
    iRet = memset_s(static_cast<void*>(&localAddr), len, 0, len);
    if (iRet != EOK) {
        WARNLOG("Connect %u:%u succ, but get local socket info failed, errno[%d].", uiServerAddr, uiPort, iRet);
    } else {
        if (MP_FAILED == getsockname(clientSock, (struct sockaddr*)&localAddr, &len)) {
            ERRLOG("Connect %u:%u succ, but get local socket info failed.", uiServerAddr, uiPort);
        }
    }
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 CSocket::CheckSockLinkStatus(mp_socket sock, mp_int32 timeout)
{
    const mp_int32 unit = 1000;
    struct timeval tm;
    tm.tv_sec = timeout / unit;
    tm.tv_usec = (timeout % unit) * unit;

    fd_set testSet;
    CSocket::FdInit(testSet);
    CSocket::FdSet(sock, testSet);
    if (select(sock + 1, NULL, &testSet, NULL, &tm) <= 0) {
        ERRLOG("select socket %d failed, error:%d.", sock, GetOSError());
        return MP_FAILED;
    } else {
        mp_int32 error = -1;
        socklen_t optLen = sizeof(mp_int32);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (mp_char*)&error, &optLen) < 0) {
            ERRLOG("socket %d getsockopt failed, err:%d.", sock, GetOSError());
            return MP_FAILED;
        } else {
            if (error != 0) {
                // like error code #define ECONNREFUSED    111     /* Connection refused */
                ERRLOG("socket %d getsockopt failed %d, err:%d.", sock, error, GetOSError());
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}

#else
mp_int32 CSocket::CheckSockLinkStatus(mp_socket sock, mp_int32 timeout)
{
    struct pollfd fds[1];
    fds[0].fd = sock;
    fds[0].events = POLLOUT;

    if (poll(fds, 1, timeout) <= 0) {
        ERRLOG("poll socket %d failed, error:%d.", sock, GetOSError());
        return MP_FAILED;
    } else {
        mp_int32 error = -1;
        socklen_t optLen = sizeof(mp_int32);
        if (getsockopt(sock, SOL_SOCKET, SO_ERROR, (mp_char*)&error, &optLen) < 0) {
            ERRLOG("socket %d getsockopt failed, err:%d.", sock, GetOSError());
            return MP_FAILED;
        } else {
            if (error != 0) {
                // like error code #define ECONNREFUSED    111     /* Connection refused */
                ERRLOG("socket %d getsockopt failed %d, err:%d.", sock, error, GetOSError());
                return MP_FAILED;
            }
        }
    }

    return MP_SUCCESS;
}
#endif

mp_int32 CSocket::CheckHostLinkStatus(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    COMMLOG(OS_LOG_DEBUG, "Will test src[%s] link dst[%s] status.", strSrcIp.c_str(), strHostIp.c_str());
    mp_uint32 ipv4Addr;
    mp_string hostIp = strHostIp;

    mp_bool isIPV4 = CIP::IsIPV4(hostIp);
    mp_int32 iRet = 0;
    if (isIPV4) {
        iRet = CIP::IPV4StrToUInt(hostIp, ipv4Addr);
    } else if (CIP::IsIPv6(hostIp)) {
        mp_uint32 ipv6Addr[4];
        iRet = CIP::IPV6StrToUInt(hostIp, ipv6Addr, sizeof(ipv6Addr));
    } else {
        ERRLOG("ip address %s is invalid.", hostIp.c_str());
        return MP_FAILED;
    }

    mp_socket sock = MP_INVALID_SOCKET;
    iRet = CSocket::CreateTcpSocket(sock, MP_TRUE, isIPV4);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Create %s(%u) tcp socket failed, iRet %d.", hostIp.c_str(), uiPort, iRet);
        return iRet;
    }

    if (strSrcIp.length() != 0) {
        CSocket::Bind(sock, strSrcIp, 0);
    }

    CSocket::SetNonBlocking(sock, MP_TRUE);
    if (isIPV4) {
        iRet = CSocket::Connect(sock, ipv4Addr, uiPort);
    } else {
        iRet = CSocket::ConnectIpv6(sock, hostIp, uiPort);
    }

    if (iRet != MP_SUCCESS) {
        if (mp_nonblocking_connect_is_inprogress(iRet)) {
            if (CheckSockLinkStatus(sock, timeout) != MP_SUCCESS) {
                CSocket::Close(sock);
                ERRLOG("Check host addr %s(%u), socket %d failed.", hostIp.c_str(), uiPort, sock);
                sock = MP_INVALID_SOCKET;
                return MP_FAILED;
            }
        } else {
            CSocket::Close(sock);
            ERRLOG("Check host addr %s(%u), socket %d failed,error=%d.", hostIp.c_str(), uiPort, sock, iRet);
            sock = MP_INVALID_SOCKET;
            return MP_FAILED;
        }
    }

    CSocket::Close(sock);
    INFOLOG("Check host addr %s(%u), socket %u succ.", hostIp.c_str(), uiPort, sock);
    sock = MP_INVALID_SOCKET;
    return MP_SUCCESS;
}
