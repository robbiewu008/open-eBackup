/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Connection.cpp
 * @brief  Implementation of the Class CConnection
 * @version 1.0.0.0
 * @date 2019-11-15
 * @author wangguitao 00510599
 */

#include "message/tcp/CConnection.h"

#include "common/Ip.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "message/tcp/CSocket.h"
#ifdef SUPPORT_SSL
#include <openssl/ssl.h>
#endif
namespace {
const int SLEEP_1000_MS = 1000;
const int RETRY_CONNECT_NUM = 10;
}  // namespace

CConnection::CConnection() : m_isIpv4(MP_TRUE)
{
    m_isNoblock = MP_TRUE;
    linkState = LINK_STATE_NO_LINKED;
    m_sslLink = MP_FALSE;
    dppMsg = NULL;
#ifdef SUPPORT_SSL
    pSsl = NULL;
    m_SslCtx = NULL;
#endif
    conn = Create();
    m_role = CLINET;
    CMpTime::Now(lastReconnectTime);
    CMpThread::InitLock(&m_stateLock);
    CMpThread::InitLock(&m_sendMsgLock);
    CMpThread::InitLock(&m_recvExitFlagLock);
}

CConnection::~CConnection()
{
    Destroy();
}

mp_int32 CConnection::InitMsgHead(mp_uint16 cmdNo, mp_uint16 flag, mp_uint64 seqNo)
{
    NEW_CATCH_RETURN_FAILED(dppMsg, CDppMessage);
    dppMsg->InitMsgHead(cmdNo, flag, seqNo);
    return MP_SUCCESS;
}

mp_int32 CConnection::InitMsgBody()
{
    if (dppMsg == NULL) {
        COMMLOG(OS_LOG_ERROR, "dppMsg is NULL");
        return MP_FAILED;
    }
    return dppMsg->InitMsgBody();
}

mp_void CConnection::SetClientIpAddr(mp_uint32 uiClientIpAddr)
{
    conn->uiClientIPaddr = uiClientIpAddr;
    m_isIpv4 = MP_TRUE;
}

mp_void CConnection::SetClientPort(mp_uint16 uiClientPort)
{
    conn->uiClientPort = uiClientPort;
}

mp_void CConnection::SetClientSocket(mp_socket clientSock)
{
    conn->clientSock = clientSock;
}

mp_void CConnection::SetIndex(mp_uint32 uiIndex)
{
    conn->uiIndex = uiIndex;
}

mp_bool CConnection::InRecvFlag()
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (conn->uiState & TSF_CONN_RECV_FLAG) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool CConnection::InSendingState()
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (conn->uiState & TSF_CONN_STATE_SENDING) {
        return MP_TRUE;
    }

    return MP_FALSE;
}

mp_bool CConnection::InReceivingState(mp_uint32 uiSubState)
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (uiSubState == STATE_RECV) {
        if (conn->uiState & TSF_CONN_STATE_RECEIVING) {
            return MP_TRUE;
        }

        return MP_FALSE;
    } else if (uiSubState == STATE_RECV_PART1) {
        if (conn->uiState & TSF_RECEPT_STATE_RECEIVING_PART1) {
            return MP_TRUE;
        }

        return MP_FALSE;
    } else {  // STATE_RECV_PART2
        if (conn->uiState & TSF_RECEPT_STATE_RECEIVING_PART2) {
            return MP_TRUE;
        }

        return MP_FALSE;
    }
}

mp_void CConnection::SetRecvFlag(mp_bool bRecv)
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (bRecv == MP_TRUE) {
        conn->uiState |= TSF_CONN_RECV_FLAG;
    } else {
        conn->uiState &= ~TSF_CONN_RECV_FLAG;
    }
}

mp_void CConnection::SetSendingState(mp_bool isSending)
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (isSending == MP_TRUE) {
        conn->uiState |= TSF_CONN_STATE_SENDING;
    } else {
        conn->uiState &= ~TSF_CONN_STATE_SENDING;
    }
}

mp_void CConnection::SetReceivingState(mp_uint32 uiSubState, mp_bool bIsReceiving)
{
    CThreadAutoLock lock_guard(&m_stateLock);
    if (bIsReceiving == MP_TRUE) {
        if (uiSubState == STATE_RECV) {
            conn->uiState |= TSF_CONN_STATE_RECEIVING;
        } else if (uiSubState == STATE_RECV_PART1) {
            conn->uiState |= TSF_RECEPT_STATE_RECEIVING_PART1;
        } else {  // STATE_RECV_PART2
            conn->uiState |= TSF_RECEPT_STATE_RECEIVING_PART2;
        }
    } else {
        if (uiSubState == STATE_RECV) {
            conn->uiState &= ~TSF_CONN_STATE_RECEIVING;
        } else if (uiSubState == STATE_RECV_PART1) {
            conn->uiState &= ~TSF_RECEPT_STATE_RECEIVING_PART1;
        } else {  // STATE_RECV_PART2
            conn->uiState &= ~TSF_RECEPT_STATE_RECEIVING_PART2;
        }
    }
}

mp_uint32 CConnection::GetIndex()
{
    return conn->uiIndex;
}

mp_uint32 CConnection::GetClientIpAddr()
{
    return conn->uiClientIPaddr;
}

mp_string CConnection::GetClientIpAddrStr()
{
    mp_string strIp;
    mp_int32 iRet = MP_SUCCESS;

    if (m_isIpv4) {
        iRet = CIP::IPV4UIntToStr(GetClientIpAddr(), strIp);
    } else {
        const mp_uint32 ipv6VecSize = 4;
        mp_uint32 ipv6Addr[ipv6VecSize];
        ipv6Addr[0] = conn->uiClientIPaddr;
        for (size_t i = 1; i < ipv6VecSize; i++) {
            ipv6Addr[i] = conn->uiClientIpAddrV6[i - 1];
        }
        mp_uint32 *tempIpv6Pointer = ipv6Addr;
        iRet = CIP::IPV6UIntToStr(tempIpv6Pointer, strIp);
    }

    if (MP_SUCCESS != iRet) {
        return "";
    }

    return strIp;
}

mp_uint64 CConnection::GetLastMsgSeqNo()
{
    return conn->uiLastRecvMsgSeqNo;
}

mp_uint16 CConnection::GetClientPort()
{
    return conn->uiClientPort;
}

mp_uint64 CConnection::GetSeqNo()
{
    return conn->uiConnSeqNo;
}

mp_socket CConnection::GetClientSocket()
{
    return conn->clientSock;
}

mp_uint16 CConnection::GetState()
{
    CThreadAutoLock lock_guard(&m_stateLock);
    return conn->uiState;
}

control_stat_t *CConnection::GetControlStat()
{
    return (control_stat_t *)&conn->uiBytesRecv;
}

mp_int32 CConnection::ClearAll(mp_bool bSavePendingMesgs)
{
    COMMLOG(OS_LOG_DEBUG, "Begin clear all.");
    (mp_void) memset_s(conn, sizeof(Connection), 0, sizeof(Connection));
    COMMLOG(OS_LOG_DEBUG, "Clear all succ.");
    return MP_SUCCESS;
}

CDppMessage *CConnection::GetDppMessage()
{
    return dppMsg;
}

mp_void CConnection::ReleaseMsg()
{
    if (dppMsg) {
        delete dppMsg;
        dppMsg = NULL;
    }
}

mp_int32 CConnection::StartRecvMsg()
{
    mp_int32 iRet = InitMsgHead();
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    SetReceivingState(STATE_RECV, MP_TRUE);
    SetReceivingState(STATE_RECV_PART1, MP_TRUE);
    GetControlStat()->uiBytesRecv = 0;
    GetControlStat()->uiBytesToRecv = GetDppMessage()->GetSize1();
    return MP_SUCCESS;
}

mp_void CConnection::SetSocket(mp_socket clientSock)
{
    conn->clientSock = clientSock;
    linkState = LINK_STATE_LINKED;
}

mp_void CConnection::DisConnect()
{
    if (conn->clientSock != MP_INVALID_SOCKET) {
        INFOLOG("CConnection::DisConnect %d", conn->clientSock);
        CSocket::Close(conn->clientSock);
        conn->clientSock = MP_INVALID_SOCKET;
#ifdef SUPPORT_SSL
        if (pSsl != NULL) {
            SSL_shutdown(pSsl);
            SSL_free(pSsl);
            pSsl = NULL;
        }
#endif
    }
    linkState = LINK_STATE_NO_LINKED;
}

mp_int32 CConnection::ResetConnection()
{
    ERRLOG("Reset connection %d. %d %d", conn->clientSock, m_role, m_isNoblock);
    if (conn->clientSock != MP_INVALID_SOCKET) {
        DisConnect();
    }
    if (m_role == CLINET) {
        return Connect();
    }
    return MP_SUCCESS;
}

mp_int32 CConnection::Connect()
{
    mp_string ipStr = GetClientIpAddrStr();
    INFOLOG("Connect %s:%u(enter) ....", ipStr.c_str(), conn->uiClientPort);
    mp_socket sock;
    if (conn->uiClientIPaddr == 0 || conn->uiClientPort == 0) {
        ERRLOG("connection %d have not initail server information.", conn->clientSock);
        return MP_FAILED;
    }
    if (linkState == LINK_STATE_LINKED) {
        INFOLOG("connection %d have already connected.", conn->clientSock);
        return MP_SUCCESS;
    }
    mp_int32 iRet = CSocket::CreateTcpSocket(sock, MP_FALSE, m_isIpv4);
    if (MP_SUCCESS != iRet) {
        ERRLOG("Create tcp socket failed, iRet %d, ip 0x%X, %u.", iRet, conn->uiClientIPaddr, conn->uiClientPort);
        return iRet;
    }

    if (!m_listenIp.empty()) {
        iRet = CSocket::Bind(sock, m_listenIp, 0);
        DBGLOG("Bind Nginx IP ip:%s iRet:%d", m_listenIp.c_str(), iRet);
    }

    if (m_isIpv4) {
        iRet = CSocket::Connect(sock, conn->uiClientIPaddr, conn->uiClientPort);
    } else {
        iRet = CSocket::ConnectIpv6(sock, ipStr, conn->uiClientPort);
    }
    if (iRet != MP_SUCCESS) {
        CSocket::Close(sock);
        ERRLOG("Connect failed(%d), addr %s(%u):%d.", iRet, ipStr.c_str(), conn->uiClientPort, sock);
        return iRet;
    }
    INFOLOG("Connect %s:%u(%d) noBlock:%d succ.", ipStr.c_str(), conn->uiClientPort, sock, m_isNoblock);
    CSocket::SetNonBlocking(sock, m_isNoblock);
#ifdef SUPPORT_SSL
    if (m_SslCtx != NULL) {
        iRet = CreateSslConnect(sock);
        if (iRet != MP_SUCCESS) {
            CSocket::Close(sock);
            ERRLOG("ssl connect safe channel failed.");
            return iRet;
        }
    } else {
        SetSSL(NULL);
    }
#endif
    SetSocket(sock);
    return MP_SUCCESS;
}

#ifdef SUPPORT_SSL
mp_void CConnection::SetSslCtx(SSL_CTX* psslCtx)
{
    m_SslCtx = psslCtx;
}

mp_int32 CConnection::CreateSslConnect(const mp_socket &sock)
{
    INFOLOG("CreateSslConnect enter.");
    if (m_SslCtx == NULL) {
        ERRLOG("ssl ctx is null.");
        return MP_FAILED;
    }
    SSL *ssl = SSL_new(m_SslCtx);
    mp_int32 iRet = SSL_set_fd(ssl, sock);
    if (iRet <= 0) {
        ERRLOG("Failed to ssl set fd sock.");
        SSL_free(ssl);
        ssl = NULL;
        return MP_FAILED;
    }
    INFOLOG("SSL set sock fd=%d.", sock);

    int iReconnectCount = 0;
    while (iReconnectCount < RETRY_CONNECT_NUM) {
        iRet = SSL_connect(ssl);
        if (iRet != MP_TRUE) {
            mp_int32 errorStatus = SSL_get_error(ssl, iRet);
            if (errorStatus == SSL_ERROR_WANT_READ || errorStatus == SSL_ERROR_WANT_WRITE) {
                iReconnectCount++;
                DoSleep(SLEEP_1000_MS);
                continue;
            }
            ERRLOG("Failed to ssl connect to the server, error=%d, sockerror=%d.", errorStatus, errno);
            SSL_free(ssl);
            ssl = NULL;
            return (errorStatus == SSL_ERROR_SYSCALL && errno == 0) ? MP_ARCHIVE_TOO_MUCH_CONNECTION : MP_FAILED;
        } else {
            DBGLOG("Connected with %s encryption\n", SSL_get_cipher(ssl));
            if (SSL_get_verify_result(ssl) == X509_V_OK) {
                SetSSL(ssl);
                DBGLOG("Certificate dual-ended authentication succeeded");
                return MP_SUCCESS;
            } else {
                SSL_free(ssl);
                ssl = NULL;
                ERRLOG("Certificate dual-ended authentication failed.");
                return MP_FAILED;
            }
        }
    }
    ERRLOG("Create ssl connection failed.");
    // release ssl once retry failure
    SSL_free(ssl);
    ssl = NULL;
    return MP_FAILED;
}
#endif

mp_void CConnection::UpdateHBTime()
{
    CMpTime::Now(lastHbTime);
}

mp_time &CConnection::GetLastHBTime()
{
    return lastHbTime;
}

mp_bool CConnection::CheckHBTimeOut()
{
    mp_time nowTime;
    CMpTime::Now(nowTime);
    mp_double diff = CMpTime::Difftime(nowTime, lastHbTime);
    return diff >= MAX_DPP_HBTIMEOUT;
}

LinkState CConnection::GetLinkState()
{
    return linkState;
}

Connection *CConnection::Create()
{
    Connection *pConn = new (std::nothrow) Connection();
    if (NULL == pConn) {
        COMMLOG(OS_LOG_ERROR, "Create connection failed, calloc failed.");
        return NULL;
    }

    return pConn;
}

mp_void CConnection::Destroy()
{
    if (conn) {
        delete conn;
        conn = NULL;
    }

    ReleaseMsg();
    CMpThread::DestroyLock(&m_stateLock);
    CMpThread::DestroyLock(&m_sendMsgLock);
    CMpThread::DestroyLock(&m_recvExitFlagLock);
}
mp_bool CConnection::GetIsIpv4()
{
    return m_isIpv4;
}

mp_bool CConnection::SetClientIpv6Addr(mp_uint32 *uiClientIpv6Addr, uint32_t uLen)
{
    mp_uint32 ipv6Len = 4;
    if (uLen != ipv6Len) {
        COMMLOG(OS_LOG_ERROR, "The length of IPV6 is not valid: ");
        return MP_FALSE;
    }

    if (uiClientIpv6Addr == NULL) {
        return MP_FALSE;
    }

    conn->uiClientIPaddr = uiClientIpv6Addr[0];
    for (size_t i = 1; i < ipv6Len; i++) {
        conn->uiClientIpAddrV6[i - 1] = uiClientIpv6Addr[i];
    }

    m_isIpv4 = MP_FALSE;
    return MP_TRUE;
}

mp_int32 CConnection::Recv(mp_char *buff, mp_uint32 iBuffLen)
{
    if (m_sslLink == MP_TRUE) {
#ifndef SUPPORT_SSL
        ERRLOG("need support SSL.");
        return MP_FAILED;
#else
        if (pSsl == NULL) {
            ERRLOG("ip:%s(%u) Recv ssl is NULL.", GetClientIpAddrStr().c_str(), GetClientPort());
            // return zero, will close the connection
            return 0;
        }

        return CSocket::SslRecv(pSsl, buff, iBuffLen);
#endif
    }
    return CSocket::Recv(conn->clientSock, buff, iBuffLen);
}

mp_int32 CConnection::Send(mp_char *buff, mp_uint32 iBuffLen)
{
    if (m_sslLink == MP_TRUE) {
#ifndef SUPPORT_SSL
        ERRLOG("need support SSL.");
        return MP_FAILED;
#else
        if (pSsl == NULL) {
            ERRLOG("ip:%s(%u) Send ssl is NULL.", GetClientIpAddrStr().c_str(), GetClientPort());
            // return zero, will close the connection
            return 0;
        }
        return CSocket::SslSend(pSsl, buff, iBuffLen);
#endif
    }

    return CSocket::Send(conn->clientSock, buff, iBuffLen);
}

void CConnection::SetSockNoBlock(mp_bool flag)
{
    m_isNoblock = flag;
}
