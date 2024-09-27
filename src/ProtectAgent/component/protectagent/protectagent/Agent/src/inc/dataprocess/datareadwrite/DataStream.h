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