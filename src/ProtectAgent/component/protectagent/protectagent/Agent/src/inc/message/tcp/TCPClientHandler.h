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
#ifndef _TCPCLIENT_HANDLER_H
#define _TCPCLIENT_HANDLER_H

#include <vector>
#include "common/Types.h"
#include "common/Log.h"
#include "common/CMpThread.h"
#include "message/tcp/DppSocket.h"
#include "message/tcp/BusinessClient.h"

static const mp_uint32 THREAD_SLEEP_TIME = 2000;  // unit millisecond
static const mp_uint32 DEFULAT_SEND_TIMEOUT = 3;  // send time out, default 3 seconds
static const mp_uint32 PERIOD_HEARTBEAT = 60000;  // heartbeat period, unit millisecond

class TCPClientHandler : public DppSocket {
public:
    static TCPClientHandler &GetInstance();

    mp_int32 Init();
    mp_int32 Connect(mp_string &busiIp, mp_uint16 busiPort, MESSAGE_ROLE role);
    mp_bool GetHBExitFlag();
    mp_bool GetRecvExitFlag();
    mp_bool GetSendExitFlag();
    mp_bool GetMsgHandlerExitFlag();

#ifdef SUPPORT_SSL
    EXTER_ATTACK mp_void InitSsl();
    mp_bool InitVerityCert();
#endif

#ifdef WIN32
    static DWORD WINAPI HeartBeat(mp_void *client);
    static DWORD WINAPI RecvThread(mp_void *client);
    static DWORD WINAPI SendThread(mp_void *client);
    static DWORD WINAPI HandleMsgThread(mp_void *client);
#else
    static void *HeartBeat(mp_void *client);
    static void *RecvThread(mp_void *client);
    static void *SendThread(mp_void *client);
    static void *HandleMsgThread(mp_void *client);
#endif

    mp_int32 RecvMessage();
    mp_int32 SendDppMsg(CDppMessage &message);
    mp_void HandleMessage();

private:
    TCPClientHandler();
    ~TCPClientHandler();

    mp_int32 WaitTcpEvents();
    mp_int32 PushHBMsg();
    mp_int32 NewMsgPair(CDppMessage *&reqMsg, CDppMessage *&rspMsg, mp_uint64 seqNo);
    BusinessClient *GetBusiClientByIpPort(const mp_string &busiIp, mp_uint16 busiPort);
    CConnection *GetConnectionByMessage(CDppMessage &message);
    mp_void AddBusiClient(BusinessClient *busiClient);
    mp_void RemoveBusiClient(BusinessClient *&busiClient);
    mp_int32 HandleRecvDppMessage(CDppMessage &message);
    mp_int32 InitBusinessClients();

    // message handle functions
    mp_int32 HandleHBAck(CDppMessage &message);
    mp_int32 HandleTaskCompleted(CDppMessage &message);

private:
    static TCPClientHandler tcpHandler;
    thread_id_t hbTid;          // heartbeat thread
    thread_id_t recvTid;        // recv thread
    thread_id_t sendTid;        // send thread
    thread_id_t msgHandlerTid;  // message handler thread
    volatile mp_bool hbExitFlag;
    volatile mp_bool recvExitFlag;
    volatile mp_bool sendExitFlag;
    volatile mp_bool msgHandlerExitFlag;

    thread_lock_t msgListMutex;
    thread_lock_t busiCliensMutex;
    thread_lock_t busiConnectMutex;
    thread_lock_t rwConnLock;
    std::vector<CDppMessage *> msgList;
    std::vector<BusinessClient *> busiClients;
    typedef mp_int32 (TCPClientHandler::*DppAction)(CDppMessage &);
    std::map<mp_uint32, DppAction> dppHandlers;
    
#ifdef SUPPORT_SSL
    SSL_CTX *pSslCtx;
#endif
    mp_bool m_secureCh;
};

#endif
