#ifndef __DATA_CONTEXT_H__
#define __DATA_CONTEXT_H__

#include <map>
#include "common/Log.h"
#include "common/Types.h"
#include "common/Defines.h"
#include "common/Types.h"
#include "message/tcp/CSocket.h"
class DataContext {
public:
    mp_void SetSockFd(mp_socket sFd);
    mp_socket GetSockFd();
    mp_int32 GetDiskFdByName(const mp_string &diskName);
    mp_int32 SetDiskFdByName(const mp_string &diskName, mp_int32 diskFd);

private:
    mp_socket sockFd;
    std::map<mp_string, mp_int32> mapOfDisks;
};
#endif