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
#ifndef SOCKET_DATA_INTERFACE_H
#define SOCKET_DATA_INTERFACE_H

#include "common/Log.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
#include "dataprocess/datareadwrite/DataContext.h"
#include "dataprocess/datareadwrite/DataStream.h"

class SocketStream : public DataStream {
public:
    SocketStream();
    ~SocketStream();
    mp_int32 StreamWrite(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen);
    mp_int32 StreamRead(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen, mp_uint32 &iRecvLen);
};

#endif