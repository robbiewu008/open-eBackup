/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataProcess.h
 * @brief  The implemention Class of DataProcess
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef DP_H
#define DP_H

#include "common/Types.h"
#include "common/CMpThread.h"
#include "message/tcp/DppSocket.h"
#include "message/tcp/CSocket.h"
#include "message/tcp/MessageHandler.h"
#include "message/tcp/CDppMessage.h"
#include "message/tcp/CConnection.h"

struct ext_cmd_info {
    mp_string vol_id;
    mp_string vol_name;
};

class DataProcess : public DppSocket {
public:
    DataProcess();
    ~DataProcess();

    mp_int32 Init(mp_socket sock, mp_string ip, mp_uint16 port);
    CConnection &GetConnection();
    mp_bool GetConnectionFlag();
    mp_bool GetSendExitFlag();

    mp_int32 CreateClient(mp_socket &clientfd);
    mp_int32 CreateServer(mp_socket &serverfd, mp_string ip, mp_uint16 port, mp_socket &cliSock);
    mp_int32 CreateRandomPortServer(mp_socket &servSock, mp_string ip);
    mp_int32 CloseDppConnection();

    CConnection *GetConnByMessage(CDppMessage &message);
    mp_int32 CreateDppMsg(mp_uint32 Cmd, CDppMessage &msg);
    mp_int32 SendDpp(CDppMessage &msg);
    mp_int32 StartReceiveDpp(CConnection &connection);
    mp_int32 HandleRecvDppMessage(CDppMessage &message);

    mp_int32 ConvertDppBody2Json(CDppMessage &message, Json::Value &bodyMsg);
    std::queue<ext_cmd_info> vol_queue;
    mp_int32 PushVolinfo(ext_cmd_info &volInfo);
    mp_int32 PopVolinfo(ext_cmd_info &volInfo);
    thread_lock_t vol_queue_lock;
    mp_int32 ext_cmd_protect_vol(ext_cmd_info &ext_cmd);

    mp_int32 WritePort(const mp_string &file, mp_int16 port);
    mp_int16 ReadPort(const mp_string &file);
private:
    mp_socket clientfd;
    mp_socket serverfd;
    CConnection conn;
    ext_cmd_info *ext_cmd;
    thread_id_t sender_tid;  // send thread
    volatile mp_bool sendExitFlag;
#ifdef WIN32
    static DWORD WINAPI senderthreadfunc(mp_void *client);
#else
    static void *senderthreadfunc(mp_void *client);
#endif
};

#endif
