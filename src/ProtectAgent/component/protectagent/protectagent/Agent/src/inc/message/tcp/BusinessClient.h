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
#include "common/Types.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/CDppMessage.h"

#ifdef SUPPORT_SSL
#include "openssl/ssl.h"
#include "openssl/asn1.h"
#include "openssl/bio.h"
#include "openssl/err.h"
#endif

class BusinessClient {
public:
    BusinessClient();
    virtual ~BusinessClient();

#ifdef SUPPORT_SSL
    mp_int32 Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role, SSL_CTX* psslCtx = NULL);
#else
    mp_int32 Init(mp_string &serverIp, mp_uint16 serverPort, MESSAGE_ROLE role);
#endif
    CConnection &GetConnection();
    MESSAGE_ROLE GetRole();
    mp_uint64 GetSeqNo();
    mp_socket GetClientSocket();

    mp_int32 Connect();
    mp_void DisConnect();
    mp_void Remove();
    mp_void HeartBeat();
    mp_int32 SslConnect(const mp_socket& sock);

private:
    mp_string GetLisetenIP();

private:
    CConnection conn;
    MESSAGE_ROLE role;
    volatile mp_uint64 seqNum;
    mp_int32 InsertBusiClientToDB();
    mp_void DelBusiClientFromDB();
    mp_int32 NewMsgPair(CDppMessage *&reqMsg, CDppMessage *&rspMsg, mp_uint64 seqNo);
};
#endif  // !defined(__BUSINIESS_CONNECTION_H__)
