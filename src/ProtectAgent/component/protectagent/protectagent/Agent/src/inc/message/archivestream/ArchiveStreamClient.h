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
#ifndef GET_FILE_CLIENT_H
#define GET_FILE_CLIENT_H

#include "common/Types.h"
#include "common/CMpThread.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/DppSocket.h"

#ifdef SUPPORT_SSL
#include "openssl/ssl.h"
#include "openssl/asn1.h"
#include "openssl/bio.h"
#endif

class ArchiveStreamClient : public DppSocket {
public:
    ArchiveStreamClient();
    virtual ~ArchiveStreamClient();

#ifdef SUPPORT_SSL
    mp_int32 Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role, SSL_CTX* psslCtx);
#else
    mp_int32 Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role);
#endif
    CConnection &GetConnection();
    MESSAGE_ROLE GetRole();
    mp_uint64 GetSeqNo();
    mp_socket GetClientSocket();

    mp_int32 Connect();
    mp_void DisConnect();

    mp_int32 SendDppMsg(CDppMessage &message);
    CDppMessage *GetRecvMsg();
    mp_void setRecvMsg(CDppMessage *message);
    mp_bool RecvEventsReady();
    mp_int32 RecvMessage();

private:
    mp_int32 HandleRecvDppMessage(CDppMessage &message);

private:
    CConnection m_conn;
    MESSAGE_ROLE m_role;
    volatile mp_uint64 m_seqNum;
    thread_lock_t m_seqNoMutext;
    CDppMessage *m_recvMsg;
};
#endif  // !defined(__BSA_CLIENT_H__)
