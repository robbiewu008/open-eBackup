/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DiskStream.h
 * @brief  Contains function declarations DiskStream
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef DISK_DATA_INTERFACE_H
#define DISK_DATA_INTERFACE_H

#include "common/Log.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
#include "dataprocess/datareadwrite/DataContext.h"
#include "dataprocess/datareadwrite/DataStream.h"

class DiskStream : public DataStream {
public:
    DiskStream();
    ~DiskStream();
    mp_int32 StreamWrite(mp_void* ctx, mp_char* buff, mp_int32 iBuffLen);
    mp_int32 StreamRead(mp_void* ctx, mp_char* buff, mp_int32 iBuffLen, mp_uint32& iRecvLen);
};

#endif