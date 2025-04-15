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
#ifndef TCP_DPP_CONNECTION_H
#define TCP_DPP_CONNECTION_H

#include <atomic>
#include "common/Defines.h"
#include "common/Types.h"
#include "common/CMpTime.h"
#include "message/tcp/CDppMessage.h"
#include "common/CMpThread.h"

#ifdef SUPPORT_SSL
#include "openssl/ssl.h"
#endif

static const mp_int32 INVALID_PORT = 1024;

enum ReceptionState {
    STATE_RECV = 1,    // 开始接收
    STATE_RECV_PART1,  // 正在接收dpp的part1
    STATE_RECV_PART2   // 正在接收dpp的part2
};

typedef enum {
    CLINET = 0x0,   // client connection，support reconnect
    SERVER = 0x1,   // server connection，not support reconnect
} ConnectRole;

// record socket send and receive bytes number
typedef struct tag_control_stat {
    mp_uint32 uiBytesRecv;
    mp_uint32 uiBytesToRecv;
    mp_uint32 uiBytesSent;
    mp_uint32 uiBytesToSend;
} control_stat_t;

struct Connection {
    mp_socket clientSock;
    mp_uint16 uiState;
    mp_uint16 uiClientPort;
    mp_uint32 uiClientIPaddr;       // uiClientIPaddr for ipv4(4bytes)
    mp_uint32 uiClientIpAddrV6[3];  // (uiClientIPaddr + uiClientIpAddrV6) for ipv6(16bytes)
    mp_uint64 uiConnSeqNo;
    mp_uint32 uiIndex;
    mp_uint64 uiLastRecvMsgSeqNo;
    // begin --control_stat_t--
    mp_uint32 uiBytesRecv;
    mp_uint32 uiBytesToRecv;
    mp_uint32 uiBytesSent;
    mp_uint32 uiBytesToSend;
    // end --control_stat_t--
};

typedef enum {
    TSF_CONN_RECV_FLAG = 0x0001,        // receive finished
    TSF_CONN_STATE_RECEIVING = 0x0002,  // connection is receiving
    TSF_CONN_STATE_SENDING = 0x0004    // connection is sending
                                        // CONN_STATE_TRACE       = 0x0008 , // connection is being traced
} TSF_CONN_STATE;

typedef enum {
    TSF_RECEPT_STATE_RECEIVING_PART1 = 0x8000,  // receiving first part of a message
    TSF_RECEPT_STATE_RECEIVING_PART2 = 0x4000  // receiving second part of a message
} TSF_RECEPT_STATE;

// client state to server
typedef enum {
    LINK_STATE_NO_LINKED = 0x0000,  // no connectted
    LINK_STATE_LINKED = 0x0001     // connected, certification, no login action
} LinkState;

class CConnection {
public:
    CConnection();
    ~CConnection();

    mp_int32 InitMsgHead(mp_uint16 cmdNo = MSG_DATA_TYPE_MANAGE, mp_uint16 flag = 0, mp_uint64 seqNo = 0);
    mp_int32 InitMsgBody();
    CDppMessage* GetDppMessage();
    mp_void ReleaseMsg();
    mp_int32 StartRecvMsg();

    mp_void SetClientIpAddr(mp_uint32 uiClientIpAddr);
    mp_bool SetClientIpv6Addr(mp_uint32* uiClientIpv6Addr, uint32_t uLen);
    mp_void SetClientPort(mp_uint16 uiClientPort);
    mp_void SetClientSocket(mp_socket clientSock);
    mp_void SetIndex(mp_uint32 uiIndex);
    mp_bool InRecvFlag();
    mp_bool InSendingState();
    mp_bool InReceivingState(mp_uint32 uiSubState);
    mp_void SetRecvFlag(mp_bool bRecv);
    mp_void SetSendingState(mp_bool isSending);
    mp_void SetReceivingState(mp_uint32 uiSubState, mp_bool bIsReceiving);
    mp_uint32 GetIndex();
    Connection* GetConn();
    mp_uint32 GetClientIpAddr();
    mp_string GetClientIpAddrStr();
    mp_uint64 GetLastMsgSeqNo();
    mp_uint16 GetClientPort();
    mp_uint64 GetSeqNo();
    mp_socket GetClientSocket();
    mp_uint16 GetState();
    control_stat_t* GetControlStat();
    mp_int32 ClearAll(mp_bool bSavePendingMesgs = MP_FALSE);
    mp_void DisConnect();
    mp_int32 ResetConnection();
    mp_void SetSocket(mp_socket clientSock);
    mp_int32 Connect();
    void SetSockNoBlock(mp_bool flag);

    void SetConnectionRole(ConnectRole role)
    {
        m_role = role;
    }

    ConnectRole GetConnectRole()
    {
        return m_role;
    }
    mp_void SetListenIp(const mp_string& listenIp)
    {
        m_listenIp = listenIp;
    }

    void UpdateResetConnectTime()
    {
        CMpTime::Now(lastReconnectTime);
    }
    mp_time GetResetConnectTime()
    {
        return lastReconnectTime;
    }

    mp_bool GetSendMsgFlag()
    {
        CThreadAutoLock lk(&m_sendMsgLock);
        return m_sendMesFlag;
    }

    mp_void SetSendMsgFlag(const mp_bool &flag)
    {
        CThreadAutoLock lk(&m_sendMsgLock);
        m_sendMesFlag = flag;
    }

    mp_void SetRecvExitFlag(const mp_bool& flag)
    {
        CThreadAutoLock lk(&m_recvExitFlagLock);
        m_recvExitFlag = flag;
    }
 
    mp_bool GetRecvExitFlag()
    {
        CThreadAutoLock lk(&m_recvExitFlagLock);
        return m_recvExitFlag;
    }

    mp_void UpdateHBTime();
    mp_time& GetLastHBTime();
    // 检查心跳是否超时
    mp_bool CheckHBTimeOut();
    LinkState GetLinkState();
    mp_bool GetIsIpv4();

#ifdef SUPPORT_SSL
    void SetSSL(SSL* pssl)
    {
        pSsl = pssl;
        if (pssl != NULL) {
            m_sslLink = MP_TRUE;
        } else {
            m_sslLink = MP_FALSE;
        }
    }

    SSL* GetSSL() const
    {
        return pSsl;
    }
    mp_void SetSslCtx(SSL_CTX* psslCtx);

#endif

    mp_int32 Recv(mp_char* buff, mp_uint32 iBuffLen);
    mp_int32 Send(mp_char* buff, mp_uint32 iBuffLen);

    mp_bool IsSSLLink()
    {
        return m_sslLink;
    }
private:
#ifdef SUPPORT_SSL
    mp_int32 CreateSslConnect(const mp_socket& sock);
#endif
    Connection* Create();
    mp_void Destroy();

private:
    Connection* conn;
    CDppMessage* dppMsg;
    // laste heartbeat time
    mp_time lastHbTime;
    // 链路连接状态
    std::atomic<LinkState> linkState;
    mp_bool m_isIpv4;
    mp_string m_listenIp;
    // 是否是安全通道
    mp_bool m_sslLink;
    ConnectRole m_role;
    mp_bool m_isNoblock;
    // last reconnect time
    mp_time lastReconnectTime;
    thread_lock_t m_stateLock;
    thread_lock_t m_sendMsgLock;
    thread_lock_t m_connectLock;
    mp_bool m_sendMesFlag = MP_TRUE;
    thread_lock_t m_recvExitFlagLock;
    mp_bool m_recvExitFlag = MP_FALSE;
#ifdef SUPPORT_SSL
    SSL* pSsl;
    SSL_CTX* m_SslCtx;
#endif
};

#endif
