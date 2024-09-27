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