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
#include "message/archivestream/ArchiveStreamClient.h"
#include <sstream>
#include "common/Log.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/DB.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"

#ifdef SUPPORT_SSL
#include "openssl/err.h"
#endif
namespace {
    const mp_int32 DPP_MSG_RETRY_TIMES = 5;
}
using std::ostringstream;
ArchiveStreamClient::ArchiveStreamClient()
{
    m_seqNum = 0;
    m_conn.UpdateHBTime();
    m_recvMsg = NULL;
    (mp_void)CMpThread::InitLock(&m_seqNoMutext);
    m_role = MESSAGE_ROLE::ROLE_HOST_AGENT;
}

ArchiveStreamClient::~ArchiveStreamClient()
{
    INFOLOG("Enter destruct");
    if (m_recvMsg != NULL) {
        delete m_recvMsg;
        m_recvMsg = NULL;
    }
    (mp_void)CMpThread::DestroyLock(&m_seqNoMutext);
}

#ifdef SUPPORT_SSL
mp_int32 ArchiveStreamClient::Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role, SSL_CTX *psslCtx)
#else
mp_int32 ArchiveStreamClient::Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role)
#endif
{
    INFOLOG("ArchiveStreamClient ip=%s, port=%d.", serverIp.c_str(), serverPort);

    if (serverPort <= INVALID_PORT) {
        ERRLOG("tcp port %u is invalid.", serverPort);
        return MP_FAILED;
    }

    mp_uint32 ipv4Addr;
    mp_uint32 ipv6Addr[4];
    mp_int32 iRet = MP_FAILED;
    if (CIP::IsIPV4(serverIp)) {
        iRet = CIP::IPV4StrToUInt(serverIp, ipv4Addr);
    } else if (CIP::IsIPv6(serverIp)) {
        iRet = CIP::IPV6StrToUInt(serverIp, ipv6Addr, sizeof(ipv6Addr));
    }

    if (iRet != MP_SUCCESS) {
        ERRLOG("Convert ip string to numeric failed, errno[%d], ip:%s.", iRet, serverIp.c_str());
        return MP_FAILED;
    }

    m_conn.SetSockNoBlock(MP_FALSE);
    m_conn.SetClientPort(serverPort);
    if (CIP::IsIPV4(serverIp)) {
        m_conn.SetClientIpAddr(ipv4Addr);
    } else {
        mp_uint32 ipVecLen = 4;
        m_conn.SetClientIpv6Addr(ipv6Addr, ipVecLen);
    }

    m_role = role;

#ifdef SUPPORT_SSL
    if (psslCtx == NULL) {
        m_conn.SetSSL(NULL);
    }
    m_conn.SetSslCtx(psslCtx);
#endif

    return MP_SUCCESS;
}

mp_int32 ArchiveStreamClient::Connect()
{
    mp_int32 ret = m_conn.Connect();
    if (ret != MP_SUCCESS) {
        ERRLOG("connect server(%s:%d) failed.", m_conn.GetClientIpAddrStr().c_str(), m_conn.GetClientPort());
        return ret;
    }

    ret = conn.StartRecvMsg();
    if (ret != MP_SUCCESS) {
        conn.DisConnect();
        ERRLOG("ArchiveStreamClient start recv message failed.");
        return MP_FAILED;
    }

    DBGLOG("ArchiveStreamClient Connected.");
    return MP_SUCCESS;
}

mp_void ArchiveStreamClient::DisConnect()
{
    DBGLOG("ArchiveStreamClient Disconnect.");
    m_conn.DisConnect();
}

CConnection &ArchiveStreamClient::GetConnection()
{
    return m_conn;
}

MESSAGE_ROLE ArchiveStreamClient::GetRole()
{
    return m_role;
}

mp_uint64 ArchiveStreamClient::GetSeqNo()
{
    CThreadAutoLock lock(&m_seqNoMutext);
    return m_seqNum++;
}

mp_socket ArchiveStreamClient::GetClientSocket()
{
    return m_conn.GetClientSocket();
}

mp_int32 ArchiveStreamClient::SendDppMsg(CDppMessage &message)
{
    mp_int32 iRet = message.ReinitMsgBody();
    if (iRet != MP_SUCCESS) {
        ERRLOG("reintial message body failed.");
        return iRet;
    }

    DBGLOG("Dest addr:%s, port:%u, cmd=0x%x, seq=%llu.", m_conn.GetClientIpAddrStr().c_str(), m_conn.GetClientPort(),
        message.GetCmd(), message.GetOrgSeqNo());

    return SendMsg(message, m_conn);
}

CDppMessage *ArchiveStreamClient::GetRecvMsg()
{
    return m_recvMsg;
}

void ArchiveStreamClient::setRecvMsg(CDppMessage *message)
{
    m_recvMsg = message;
}

mp_int32 ArchiveStreamClient::HandleRecvDppMessage(CDppMessage &message)
{
    m_recvMsg = &message;
    DBGLOG("Received message, cmd=0x%x, len=%u, seq=%llu.", m_recvMsg->GetCmd(), m_recvMsg->GetSize2(),
        m_recvMsg->GetOrgSeqNo());

    return MP_SUCCESS;
}

mp_bool ArchiveStreamClient::RecvEventsReady()
{
#ifdef WIN32
    fd_set fdsRead;
    fd_set fdsWrite;
    CSocket::FdInit(fdsRead);
    CSocket::FdInit(fdsWrite);

    mp_uint16 sock = GetClientSocket();
    CSocket::FdSet(sock, fdsRead);

    // use select
    mp_int32 ret = CSocket::WaitEvents(fdsRead, fdsWrite, sock);
    if (ret < 0) {
        ERRLOG("Wait events failed, ret=%d.", ret);
        return MP_FALSE;
    } else if (ret == 0) { // select timeout
        return MP_FALSE;
    }

    return CSocket::FdIsSet(sock, fdsRead);
#else
    mp_uint16 sock = GetClientSocket();
    // use poll
    mp_int32 ret = CSocket::WaitEvents(sock);
    if (ret < 0) {
        ERRLOG("Wait events failed, ret=%d.", ret);
        return MP_FALSE;
    } else if (ret == 0) { // select timeout
        return MP_FALSE;
    }
    return MP_TRUE;
#endif
}

mp_int32 ArchiveStreamClient::RecvMessage()
{
    m_conn.InitMsgHead();
    m_conn.SetReceivingState(STATE_RECV, MP_TRUE);
    m_conn.SetReceivingState(STATE_RECV_PART1, MP_TRUE);
    m_conn.GetControlStat()->uiBytesRecv = 0;
    m_conn.GetControlStat()->uiBytesToRecv = m_conn.GetDppMessage()->GetSize1();

    mp_int32 againTimes = 0;
    while (m_recvMsg == NULL) {
        mp_int32 iRet = RecvMsg(m_conn);
        if (iRet == MP_EAGAIN) {
            DBGLOG("RecvMsg return MP_EAGAIN");
            againTimes++;
            if (againTimes > DPP_MSG_RETRY_TIMES) {
                DBGLOG("RecvMsg return MP_EAGAIN %d Times, recvMsg  failed", againTimes);
                return MP_FAILED;
            }
            continue;
        }

        if ((m_conn.GetRecvExitFlag() == MP_TRUE) || (iRet == MP_ABORTED)) {
            INFOLOG("Receive message from client node aborted.");
            return MP_ABORTED;
        }

        againTimes = 0;
        if (iRet != MP_SUCCESS) {
            ERRLOG("Receive message from client node failed.");
            return iRet;
        }
    }

    if (m_recvMsg != NULL) {
        m_recvMsg->SetLinkInfo(m_conn.GetClientIpAddrStr(), m_conn.GetClientPort());
    }

    return MP_SUCCESS;
}
