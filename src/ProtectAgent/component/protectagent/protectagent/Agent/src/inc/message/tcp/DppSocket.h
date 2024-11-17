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
#ifndef AGENT_TSF_SOCKET_CLIENT_H
#define AGENT_TSF_SOCKET_CLIENT_H

#include <vector>
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/CConnection.h"
#include "common/CMpThread.h"

class DppSocket {
public:
    DppSocket();
    virtual ~DppSocket();

    mp_int32 SendMsg(CDppMessage &message, CConnection &conn);
    mp_int32 RecvMsg(CConnection &connection);

protected:
    CConnection conn;
    fd_set fdsRead;
    fd_set fdsWrite;

    // when receive new dpp message, need to handle it
    virtual mp_int32 HandleRecvDppMessage(CDppMessage &message) = 0;
    mp_int32 SendBuffer(mp_char *buf, CConnection &connection);
    mp_int32 ProcessMsgPart1(CConnection &connection, CDppMessage *message);
    mp_int32 ProcessMsgPart2(CConnection &connection, CDppMessage *message);
    mp_void CloseConnect(CConnection &connection);
    mp_int32 GetBuffer(CConnection &connection, CDppMessage &message, mp_char *&recvBuf);

#ifdef SUPPORT_SSL
    mp_int32 CheckMsgLength(
        mp_int32 len, mp_socket &sock, CConnection &conn, const mp_string &ip, mp_uint16 port, SSL *pSsl);
#else
    mp_int32 CheckMsgLength(mp_int32 len, mp_socket &sock, CConnection &conn, const mp_string &ip, mp_uint16 port);
#endif
    mp_int32 ReceiveAllMsg(CConnection &conn, CDppMessage *message, mp_int32 len, control_stat_t *stat,
        mp_socket &socket, mp_string &ip, mp_int32 port);

    mp_int32  SendBufferLessThan0(CConnection &connection);
    mp_bool CmdFilter(mp_uint32 cmd);

private:
#ifdef SUPPORT_SSL
    mp_int32 GetSSLError(SSL *pSsl);
    mp_int32 GetConnectErr(mp_int32& iErr, SSL *pSsl);
#else
    mp_int32 GetConnectErr(mp_int32& iErr);
#endif
    
    mp_int32 CheckOSErr(mp_int32& iErr);
    mp_int32 ResetConnection(CConnection &connection);

    mp_int32 SendMsgImpl(CDppMessage &message, CConnection &connection);
    mp_int32 RecvMsgImpl(CConnection &connection);
    mp_bool HandleRecvSendFail(CConnection &connection);
private:
    thread_lock_t resetLock;
};

#endif  // __AGENT_TSF_SOCKET_H__
