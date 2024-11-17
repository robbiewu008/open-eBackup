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
#ifndef __DATAMESSAGE_H__
#define __DATAMESSAGE_H__

#include "common/Types.h"
#include "common/TimeOut.h"
#include "common/CMpThread.h"
#include "message/tcp/DppSocket.h"
#include "message/tcp/CSocket.h"
#include "message/tcp/MessageHandler.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/CConnection.h"

class AGENT_API DataMessage : public DppSocket {
public:
    DataMessage();
    ~DataMessage();

    mp_int32 Init(mp_socket sock, const mp_string& ip, mp_uint16 port);
    CConnection &GetConnection();

    mp_int32 CreateClient(mp_socket &clientfd);
    mp_int32 CreateRandomPortServer(mp_socket &servSock, const mp_string& ip, mp_int32 serviceType,
        const mp_string &dpParam);

    CConnection *GetConnByMessage();
    mp_int32 SendDpp(CDppMessage &msg);
    mp_int32 StartReceiveDpp();
    mp_int32 HandleRecvDppMessage(CDppMessage &message);
    mp_int32 ConvertDppBody2Json(CDppMessage &message, Json::Value &bodyMsg);
    mp_int32 WritePort(const mp_string &file, mp_uint16 port);
    mp_uint16 ReadPort(const mp_string &file);
    mp_void CloseDppConnection();
    mp_uint16 GetRandomPort(mp_socket servSock, const mp_string& ip);
    mp_int32 GetRandom();
    mp_int32 GetPid();
    mp_int32 SetSockTimeOut(mp_int32 secondTimeout);

    CDppMessage *recvdMsg;
};

#endif
