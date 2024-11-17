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
#ifndef __DATA_PATH_H__
#define __DATA_PATH_H__

#include <mutex>
#include <thread>
#include <vector>
#include "common/Defines.h"
#include "common/Types.h"
#include "common/TimeOut.h"
#include "dataprocess/datamessage/DataMessage.h"
#include "dataprocess/dataconfig/DataConfig.h"
#include "dataprocess/datareadwrite/DataStream.h"

static const mp_int32 SERVICE_VMWARE = 2;
static const mp_int32 SERVICE_CDP = 1;

// Base class to handle message received from Host Agent
class DataPath {
public:
    DataPath(mp_int32 iType, const mp_string &dpParam);
    virtual ~DataPath();
    mp_bool GetSendExitFlag();
    mp_int32 DataInit(mp_socket sockFd, mp_uint16 Port, const mp_string& Ip);
    mp_int32 StartExtCmdHandler();

    thread_id_t handlerTid;
protected:
    mp_string dpParam;
    volatile mp_bool sendExitFlag;
    DataConfig dataConfig;

#ifdef WIN32
    static DWORD WINAPI ExtCmdHandler(mp_void *arg);
#else
    static void *ExtCmdHandler(mp_void *arg);
#endif

    mp_void PushMsg2Queue(std::unique_ptr<CDppMessage> &msg);
    virtual mp_int32 ExtCmdProcess(CDppMessage &message);

private:
    static std::mutex m_msgMutex;
    DataMessage dataServer;
    mp_int32 serviceType;
    std::thread sendThread;
    volatile mp_bool responseFlag;
    // vector message, waiting for sending
    std::vector<std::unique_ptr<CDppMessage>> m_msgQueue;
    // send message thread
    mp_void ResponseMsg();

    mp_int32 AcceptConnection(mp_socket serverFd);
    mp_int32 RecvMsgProcess(CConnection &conn);
    mp_int32 GetServiceType();
    mp_string GetDpParam();
};

#endif