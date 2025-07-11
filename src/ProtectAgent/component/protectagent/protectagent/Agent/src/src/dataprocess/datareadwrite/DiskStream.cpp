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
#include "dataprocess/datareadwrite/DiskStream.h"
#include "common/TimeOut.h"
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "dataprocess/datareadwrite/DataContext.h"
#include "dataprocess/datareadwrite/DataStream.h"

DiskStream::DiskStream() {}

DiskStream::~DiskStream() {}

mp_int32 DiskStream::StreamWrite(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen)
{
    DataContext *ctxdata = static_cast<DataContext *>(ctx);
    if (ctxdata == NULL) {
        return MP_FAILED;
    }
    mp_int32 dFd = 0;
    mp_int32 iSendLen = 0;
    mp_string diskName;
    dFd = ctxdata->GetDiskFdByName(diskName);
    COMMLOG(OS_LOG_DEBUG, "Write message called  %d, len=%d.", dFd, iSendLen);

    mp_int32 iRet = MP_SUCCESS;
    return (iRet);
}

mp_int32 DiskStream::StreamRead(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen, mp_uint32 &iRecvLen)
{
    DataContext *ctxdata = static_cast<DataContext *>(ctx);
    if (ctxdata == NULL) {
        return MP_FAILED;
    }
    mp_int32 dFd = 0;
    mp_uint32 uiRecvLen = 0;
    mp_string diskName;
    dFd = ctxdata->GetDiskFdByName(diskName);
    COMMLOG(OS_LOG_DEBUG, "Receive message called  %d, len=%d.", dFd, uiRecvLen);

    mp_int32 iRet = MP_SUCCESS;
    return (iRet);
}