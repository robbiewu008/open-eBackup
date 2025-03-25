/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataStream.h
 * @brief  Contains function declarations about DataStream
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef DATA_INTERFACE_H
#define DATA_INTERFACE_H

#include "common/Log.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
#include "dataprocess/datareadwrite/DataContext.h"

class DataStream {
public:
    virtual mp_int32 StreamWrite(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen) = 0;

    virtual mp_int32 StreamRead(mp_void *ctx, mp_char *buff, mp_int32 iBuffLen, mp_uint32 &iRecvLen) = 0;

private:
    DataContext ctxdata;
};

#endif