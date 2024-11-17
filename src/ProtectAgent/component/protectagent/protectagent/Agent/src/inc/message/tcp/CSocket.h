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
#ifndef AGENT_TSF_SOCKET_H
#define AGENT_TSF_SOCKET_H

#include "common/Defines.h"
#include "common/Types.h"

#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <arpa/inet.h>
#include <errno.h>
#include <memory.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

#ifdef SUPPORT_SSL
#include "openssl/ssl.h"
#endif

#ifdef LINUX
#include <sys/fcntl.h>
#endif

#ifdef WIN32
#define MP_INVALID_SOCKET ((mp_socket)INVALID_SOCKET)
#define mp_close_socket closesocket
#define mp_ioctl_socket ioctlsocket
#define mp_retry(err) 0
#define mp_would_block(err) (err == WSAEWOULDBLOCK)
#define mp_nonblocking_connect_is_inprogress(err) (err == WSAEWOULDBLOCK)
typedef mp_ulong sock_option_t;
#else
#define MP_INVALID_SOCKET ((mp_socket)-1)
#define mp_close_socket close
#define mp_ioctl_socket ioctl
#define mp_retry(err) (err == EINTR)
#define mp_would_block(err) (err == EWOULDBLOCK || err == EAGAIN)
#define mp_nonblocking_connect_is_inprogress(err) (err == EINPROGRESS)
typedef mp_int32 sock_option_t;
#endif

// special return value for socket
#define SOCKET_TIMEOUT -2  // select timeout
#define SOCKET_CLOSED -3   // Socket closed

typedef enum {
    CLIENT_ERRTYPE_SOCKET = 1,
    CLIENT_ERRTYPE_CLOSED,
    CLIENT_ERRTYPE_SEND_TIMEOUT,
    CLIENT_ERRTYPE_MSG_INVALID  // get partial msg
} CLIENT_ERR_TYPE;

class CSocket {
public:
    static mp_int32 Init();
    static mp_int32 SetFdCloexec(mp_int32 fd);
    static mp_int32 CreateTcpSocket(mp_socket& sock, mp_bool keepSocketInherit = MP_TRUE, mp_bool isIPV4 = MP_TRUE);
    static mp_int32 AcceptClient(
        mp_socket serverSock, mp_uint32& uiClientAddr, mp_uint16& uiClientPort, mp_socket& clientSock);
    static mp_void Close(mp_socket sock);
    static mp_int32 SetReuseAddr(mp_socket sock);
    static mp_int32 StartListening(mp_socket sock);
    static mp_int32 Bind(mp_socket sock, const mp_string& strIp, mp_uint16 uiPort);
    static mp_int32 BindIpv6(mp_socket sock, const mp_string& strIp, mp_uint16 uiPort);
    static mp_int32 Connect(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort);
    static mp_int32 Recv(mp_socket sock, mp_char* buff, mp_uint32 ibufLen);
    static mp_int32 RecvBuffer(
        mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut, mp_uint32& iRecvLen);
    static mp_int32 Send(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen);
#ifdef SUPPORT_SSL
    static mp_int32 SslRecv(SSL* ssl, mp_char* buff, mp_uint32 ibufLen);
    static mp_int32 SslSend(SSL* ssl, mp_char* buff, mp_uint32 iBuffLen);
#endif
    static mp_int32 SendBuffer(mp_socket sock, mp_char* buff, mp_uint32 iBuffLen, mp_uint32 uiTimeOut);
    static mp_void SetNonBlocking(mp_socket sock, mp_bool bNonBlock);
    static mp_void GetNonBlocking(mp_socket sock, mp_bool& bNonBlock);
    static mp_int32 CreateClientSocket(mp_socket& clientSock);
    static mp_int32 CreateServerSocket(mp_socket& servSock, const mp_string& ip, mp_uint16 uiPort);
    static mp_void FdInit(fd_set& fdSet);
    static mp_void FdSet(mp_socket sock, fd_set& fdSet);
    static mp_void FdClr(mp_socket sock, fd_set& fdSet);
    static mp_bool FdIsSet(mp_socket sock, fd_set& fdset);
    static mp_int32 WaitEvents(fd_set& fdRead, fd_set& fdWrite, mp_int32 iMaxFd);
    static mp_int32 ConnectIpv6(mp_socket clientSock, const mp_string &uiServerAddr, mp_uint16 uiPort);
    static mp_int32 CheckHostLinkStatus(
        const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort = 111, mp_int32 timeout = 200);
    
private:
    static mp_int32 WaitRecvEvent(mp_socket sock, mp_uint32 uiSecondes);
    static mp_int32 WaitSendEvent(mp_socket sock, mp_uint32 uiSecondes);
    static mp_int32 CheckSockLinkStatus(mp_socket sock, mp_int32 timeout);
};

#endif  // __AGENT_TSF_SOCKET_H__
