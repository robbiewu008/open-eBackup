#include "message/tcp/DppSocket.h"
#include "common/Log.h"
#include "common/TimeOut.h"
#include "common/Utils.h"
#include "message/tcp/CSocket.h"

#ifdef SUPPORT_SSL
#include <openssl/err.h>
#endif
namespace {
    const mp_uint32 CMD_FILTER_COUNT = 4;
    const mp_uint32 RECEIVE_INTERVAL = 10; // 连续接受报文时的间隔 单位ms
    const mp_uint32 CMD_FILTER_ARRAY[CMD_FILTER_COUNT] = {
        1032, 1044, 1080, 1092
    };
}


DppSocket::DppSocket()
{
    CSocket::FdInit(fdsRead);
    CSocket::FdInit(fdsWrite);
    CMpThread::InitLock(&resetLock);
}

DppSocket::~DppSocket()
{
    CMpThread::DestroyLock(&resetLock);
}

mp_bool DppSocket::CmdFilter(mp_uint32 cmd)
{
    // vmware backup/restore cmd filter
    static std::vector<mp_uint32> CMD_FILTER(CMD_FILTER_ARRAY, CMD_FILTER_ARRAY  + CMD_FILTER_COUNT);
    mp_bool isCmdFound = false;
    for (int i = 0; i < CMD_FILTER.size(); i++) {
        if (CMD_FILTER[i] == cmd) {
            isCmdFound = true;
            break;
        }
    }
    return isCmdFound;
}

mp_int32 DppSocket::SendMsgImpl(CDppMessage &message, CConnection &conn)
{
    mp_socket sock = conn.GetClientSocket();
    mp_string clientIp = conn.GetClientIpAddrStr();
    mp_uint16 clientPort = conn.GetClientPort();
    mp_uint64 seqNo = message.GetOrgSeqNo();
    control_stat_t *ctlStat = conn.GetControlStat();
    mp_uint32 manageCmd = message.GetManageCmd();

    DBGLOG("SendMsg conn socket %d, server [%s:%u], seq=%llu.", sock, clientIp.c_str(), clientPort, seqNo);
    conn.SetSendingState(MP_TRUE);

    // send header
    ctlStat->uiBytesSent = 0;
    ctlStat->uiBytesToSend = message.GetSize1();
#if defined(AIX) || defined(SOLARIS)
    message.ItemEendianSwap();
#endif
    mp_int32 iRet = SendBuffer(message.GetStart(), conn);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Send %s:%u(%d) header failed, iRet %d, seq=%llu.", clientIp.c_str(), clientPort, sock, iRet, seqNo);
        return iRet;
    }
    DBGLOG("Send %s(%d) header succ, len %u, seq=%llu.", clientIp.c_str(), sock, ctlStat->uiBytesSent, seqNo);

    // send body
#if defined(AIX) || defined(SOLARIS)
    message.ItemEendianSwap();
#endif
    if (message.GetSize2() != 0) {
        ctlStat->uiBytesSent = 0;
        ctlStat->uiBytesToSend = message.GetSize2();
        DBGLOG("Send body to server[%s:%u], cmd is 0x%.8x, seq=%llu",
            clientIp.c_str(), clientPort, manageCmd, seqNo);
        iRet = SendBuffer(message.GetBuffer(), conn);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Send [%s:%u](%d) body fail, iRet=%d, seq=%llu.", clientIp.c_str(), clientPort, sock, iRet, seqNo);
            return iRet;
        }
        DBGLOG("Send [%s:%u](%d) body succ, len %u, seq=%llu.",
            clientIp.c_str(), clientPort, sock, ctlStat->uiBytesSent, seqNo);
    }

    return MP_SUCCESS;
}

#ifdef SUPPORT_SSL
    mp_int32 DppSocket::CheckMsgLength(mp_int32 len, mp_socket &sock, CConnection &conn, const mp_string &ip,
        mp_uint16 port, SSL *pSsl)
#else
    mp_int32 DppSocket::CheckMsgLength(mp_int32 len, mp_socket &sock, CConnection &conn, const mp_string &ip,
        mp_uint16 port)
#endif
{
    if (len < 0) {
        mp_int32 iErr = 0;
#ifdef SUPPORT_SSL
        mp_int32 connErr = GetConnectErr(iErr, pSsl);
#else
        mp_int32 connErr = GetConnectErr(iErr);
#endif
        if (connErr == MP_ERROR) {
            // reconnet current connection
            mp_char szErr[256] = {0};
            ERRLOG("Recv message[%s:%u](%d) fail, err[%d]:%s.", ip.c_str(), port, sock, iErr, szErr);
            return MP_FAILED;
        }

        if (connErr == MP_EAGAIN) {
            DBGLOG("Recv message[%s:%u](%d) again, error connErr:%d iErr:%d.", ip.c_str(), port, sock, connErr, iErr);
        } else {
            WARNLOG("Recv message[%s:%u](%d) failed, error connErr:%d iErr:%d.", ip.c_str(), port, sock, connErr, iErr);
        }
        return connErr;
    }

    // socket closed by peer
    if (len == 0) {
#ifdef SUPPORT_SSL
        GetSSLError(pSsl);
#endif
        ERRLOG("Connection about [%s:%u](%d) have closed by peer.", ip.c_str(), port, sock);
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 DppSocket::ReceiveAllMsg(CConnection &conn, CDppMessage *message, mp_int32 len, control_stat_t *stat,
    mp_socket &socket, mp_string &ip, mp_int32 port)
{
    mp_uint32 uLen = static_cast<mp_uint32>(len);
    if (uLen > stat->uiBytesToRecv) {
        ERRLOG("Connection [%s:%u](%d), uLen %u, toRecv %u.", ip.c_str(), port, socket, uLen, stat->uiBytesToRecv);
        ResetConnection(conn);
        return MP_FAILED;
    }

    stat->uiBytesToRecv -= uLen;
    stat->uiBytesRecv += uLen;
    // recv part of the msg
    if (stat->uiBytesToRecv > 0) {
        DBGLOG("Connection[%s:%u](%d) Message have %d to recv.", ip.c_str(), port, socket, stat->uiBytesToRecv);
        CMpTime::DoSleep(RECEIVE_INTERVAL);
        return MP_SUCCESS;
    }
    if (conn.InReceivingState(STATE_RECV_PART1)) {
        return ProcessMsgPart1(conn, message);
    } else if (conn.InReceivingState(STATE_RECV_PART2)) {
        return ProcessMsgPart2(conn, message);
    }
    return MP_SUCCESS;
}

mp_int32 DppSocket::RecvMsgImpl(CConnection &connection)
{
    control_stat_t *ctlStat = connection.GetControlStat();
    mp_socket sock = connection.GetClientSocket();
    mp_string adminIp = connection.GetClientIpAddrStr();
    mp_uint16 port = connection.GetClientPort();

    if (connection.InReceivingState(STATE_RECV_PART1) != MP_TRUE &&
        connection.InReceivingState(STATE_RECV_PART2) != MP_TRUE) {
        ERRLOG("Close connection about [%s:%u], socket %d.", adminIp.c_str(), port, sock);
        return MP_FAILED;
    }

    DBGLOG("--------Begin recv msg--------.");
    CDppMessage *message = connection.GetDppMessage();
    if (message == NULL) {
        ERRLOG("Get NULL message about [%s:%u], socket %d.", adminIp.c_str(), port, sock);
        return MP_FAILED;
    }

    // recv whole dpp message
    connection.SetRecvFlag(MP_TRUE);

    while (connection.InRecvFlag() == MP_TRUE) {
        mp_char *recvBuf = NULL;
        mp_int32 iRet = GetBuffer(connection, *message, recvBuf);
        if (iRet != MP_SUCCESS) {
            ERRLOG("GetBuffer from message about [%s:%u], socket %d.", adminIp.c_str(), port, sock);
            return MP_FAILED;
        }

        mp_int32 iLen = connection.Recv(recvBuf + ctlStat->uiBytesRecv, ctlStat->uiBytesToRecv);
#ifdef SUPPORT_SSL
        iRet = CheckMsgLength(iLen, sock, connection, adminIp, port, connection.GetSSL());
#else
        iRet = CheckMsgLength(iLen, sock, connection, adminIp, port);
#endif
        if (iRet != MP_SUCCESS) {
            if (iRet == MP_EAGAIN) {
                DBGLOG("check message MP_EAGAIN, len=%d, socket=%d, errorcode:%d.", iLen, sock, iRet);
            } else {
                ERRLOG("check message valid failed, len=%d, socket=%d, errorcode:%d.", iLen, sock, iRet);
            }
            return iRet;
        }

        DBGLOG("connection [%s:%u](%d), iLen:%d.", adminIp.c_str(), port, sock, iLen);
        iRet = ReceiveAllMsg(connection, message, iLen, ctlStat, sock, adminIp, port);
        if (iRet != MP_SUCCESS) {
            return iRet;
        }
    }

    return MP_SUCCESS;
}

mp_int32 DppSocket::ProcessMsgPart1(CConnection &connection, CDppMessage *message)
{
    COMMLOG(OS_LOG_DEBUG, "--------Begin process msg part1--------.");
    connection.SetReceivingState(STATE_RECV_PART1, MP_FALSE);
    if (message->IsValidPrefix() == MP_FALSE) {
        return MP_FAILED;
    }

    mp_int32 iRet = message->InitMsgBody();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init message body failed %d", iRet);
        return MP_FAILED;
    }

    connection.SetReceivingState(STATE_RECV_PART2, MP_TRUE);
    connection.GetControlStat()->uiBytesRecv = 0;
    connection.GetControlStat()->uiBytesToRecv = message->GetSize2();
    COMMLOG(OS_LOG_DEBUG, "Process msg part1 succ, part2 length %d.", message->GetSize2());
    return MP_SUCCESS;
}

mp_int32 DppSocket::ProcessMsgPart2(CConnection &connection, CDppMessage *message)
{
    mp_int32 iRet = MP_SUCCESS;
    COMMLOG(OS_LOG_DEBUG, "--------Begin process msg part2--------.");
    connection.SetReceivingState(STATE_RECV_PART2, MP_FALSE);

    // config message source
    message->SetLinkInfo(connection.GetClientIpAddrStr(), connection.GetClientPort());
    iRet = HandleRecvDppMessage(*message);
    if (iRet != MP_SUCCESS) {
        // faild and retry to handle message
        ERRLOG("handle recv message failed, continue receive message.");
    }

    iRet = connection.InitMsgHead();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init message head failed %d", iRet);
        return iRet;
    }
    connection.SetReceivingState(STATE_RECV_PART1, MP_TRUE);
    connection.GetControlStat()->uiBytesRecv = 0;
    connection.GetControlStat()->uiBytesToRecv = connection.GetDppMessage()->GetSize1();
    // have received whole message
    connection.SetRecvFlag(MP_FALSE);
    mp_uint32 msgBodySize = message->GetSize2();
    DBGLOG("Process msg part2 succ, length %u.", msgBodySize);
    return MP_SUCCESS;
}

mp_int32 DppSocket::SendBuffer(mp_char *buf, CConnection &connection)
{
    if (buf == NULL) {
        ERRLOG("SendBuffer, buffer is null.");
        return MP_FAILED;
    }

    for (;;) {
        mp_int32 iLen = connection.Send(buf, connection.GetControlStat()->uiBytesToSend);
        if (iLen < 0) {
            if (SendBufferLessThan0(connection) != MP_EAGAIN) {
                return MP_FAILED;
            }
        }

        if (iLen == 0) {
            ERRLOG(" connection about [%s:%u](%d) close by peer",
                connection.GetClientIpAddrStr().c_str(),
                connection.GetClientPort(),
                connection.GetClientSocket());
            return MP_FAILED;
        }
        if (iLen > 0) {
            connection.GetControlStat()->uiBytesSent += iLen;
            connection.GetControlStat()->uiBytesToSend -= iLen;
        }

        // send part of the msg
        if (connection.GetControlStat()->uiBytesToSend != 0) {
            INFOLOG("Connection [%s:%u](%d) have send %u, left %u.",
                connection.GetClientIpAddrStr().c_str(),
                connection.GetClientPort(),
                connection.GetClientSocket(),
                connection.GetControlStat()->uiBytesSent,
                connection.GetControlStat()->uiBytesToSend);
            continue;
        }
        break;
    }
    return MP_SUCCESS;
}

mp_int32 DppSocket::SendBufferLessThan0(CConnection &connection)
{
    mp_string ipStr = connection.GetClientIpAddrStr();
    mp_uint16 uPort = connection.GetClientPort();
    mp_socket sock = connection.GetClientSocket();

    mp_int32 iErr = 0;
#ifdef SUPPORT_SSL
    mp_int32 iRet = GetConnectErr(iErr, connection.GetSSL());
#else
    mp_int32 iRet = GetConnectErr(iErr);
#endif

    ERRLOG("Send [%s:%u](%d) failed, error %d.", ipStr.c_str(), uPort, sock, iErr);
    return iRet;
}
mp_void DppSocket::CloseConnect(CConnection& connection)
{
    if (connection.GetLinkState() != LINK_STATE_NO_LINKED) {
        DBGLOG("Close connect %d.", connection.GetClientSocket());
        CSocket::FdClr(connection.GetClientSocket(), fdsRead);
        connection.DisConnect();
    } else {
        DBGLOG("connect is already disconnected.");
    }
}

mp_int32 DppSocket::ResetConnection(CConnection& connection)
{
    DBGLOG("Reset connection %d.", connection.GetClientSocket());
    CSocket::FdClr(connection.GetClientSocket(), fdsRead);
    return connection.ResetConnection();
}

mp_int32 DppSocket::GetBuffer(CConnection &connection, CDppMessage &message, mp_char *&recvBuf)
{
    if (connection.InReceivingState(STATE_RECV_PART1)) {
        recvBuf = message.GetStart();
    } else if (connection.InReceivingState(STATE_RECV_PART2)) {
        recvBuf = message.GetBuffer();
    } else {
        // close current connection
        ERRLOG("connection status is invalid, close connection.");
        ResetConnection(connection);
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

#ifdef SUPPORT_SSL
mp_int32 DppSocket::GetSSLError(SSL* pSsl)
{
    if (pSsl == NULL) {
        return SSL_ERROR_NONE;
    }
    mp_int32 iRet = 0;
    mp_int32 sslError = SSL_get_error(pSsl, iRet);
    if (iRet < 0 || sslError != 0) {
        if (SSL_ERROR_SSL == sslError) {
            const char *err_str = ERR_reason_error_string(ERR_get_error());
            ERRLOG("SSL error, ssl_status=%d, ssl_err=%d, err_msg=%s.", iRet, sslError, err_str);
        } else {
            ERRLOG("SSL error, ssl_status=%d, ssl_err=%d.", iRet, sslError);
        }
    }
    return sslError;
}
#endif

mp_int32 DppSocket::CheckOSErr(mp_int32& iErr)
{
    iErr = GetOSError();
    return !mp_would_block(iErr) && !mp_retry(iErr);
}

#ifdef SUPPORT_SSL
mp_int32 DppSocket::GetConnectErr(mp_int32 &iErr, SSL *pSsl)
#else
mp_int32 DppSocket::GetConnectErr(mp_int32& iErr)
#endif
{
#ifdef SUPPORT_SSL
    mp_int32 sslErr = GetSSLError(pSsl);
    if (SSL_ERROR_WANT_READ == sslErr || SSL_ERROR_WANT_WRITE == sslErr) {
        DBGLOG("the same TLS/SSL I/O function should be called again later (sslErr:%d)", sslErr);
        return MP_EAGAIN;
    }
#endif
    if (CheckOSErr(iErr)) {
        return MP_ERROR;
    }

    if (iErr == EINTR || iErr == EAGAIN) {
        return MP_EAGAIN;
    }
    return MP_FAILED;
}

mp_int32 DppSocket::RecvMsg(CConnection &connection)
{
    const mp_int32 maxRetryNum = 3;
    mp_int32 retryNum(0);
    while (retryNum < maxRetryNum) {
        mp_int32 iRet = RecvMsgImpl(connection);
        if (iRet != MP_SUCCESS && iRet != MP_EAGAIN) {
            if (HandleRecvSendFail(connection) == MP_TRUE) {
                return MP_FAILED;
            }
        } else {
            return iRet;
        }
        if (connection.GetRecvExitFlag() == MP_TRUE) {
            INFOLOG("Receive Exiting Flag is set, stoping receive retry.");
            return MP_ABORTED;
        }
        connection.InitMsgHead();
        ++retryNum;
        DBGLOG("try to Recv Msg %d:th times", retryNum);
    }
    return MP_FAILED;
}

mp_int32 DppSocket::SendMsg(CDppMessage &message, CConnection &connection)
{
    const mp_int32 maxRetryNum = 3;
    mp_int32 retryNum(0);
    while (retryNum < maxRetryNum) {
        mp_int32 iRet = SendMsgImpl(message, connection);
        if (iRet != MP_SUCCESS && iRet != MP_EAGAIN) {
            if (HandleRecvSendFail(connection) == MP_TRUE) {
                return MP_FAILED;
            }
        } else {
            return iRet;
        }
        ++retryNum;
        DBGLOG("try to Send Msg %d:th times", retryNum);
    }
    return MP_FAILED;
}

mp_bool DppSocket::HandleRecvSendFail(CConnection &connection)
{
    if (!connection.GetSendMsgFlag()) {
        return MP_TRUE;
    }
    if (connection.GetConnectRole() == SERVER) {
        CloseConnect(connection);
        return MP_TRUE;
    } else {
        CThreadAutoLock lock(&resetLock);
        mp_time now;
        CMpTime::Now(now);
        // 防止读写线程同时去reset connect，一秒内reset过连接不再去重连
        if (CMpTime::Difftime(now, connection.GetResetConnectTime()) > 1) {
            connection.UpdateResetConnectTime();
            return (ResetConnection(connection) == MP_SUCCESS) ? (MP_FALSE) : (MP_TRUE);
        }
        WARNLOG("Connect reset in one secs, return direct this time");
        return MP_TRUE;
    }
}