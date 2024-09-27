#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "dataprocess/datareadwrite/DataContext.h"

mp_socket DataContext::GetSockFd()
{
    return sockFd;
}

mp_void DataContext::SetSockFd(mp_socket sFd)
{
    sockFd = sFd;
}

mp_int32 DataContext::GetDiskFdByName(const mp_string &diskName)
{
    std::map<std::string, int>::iterator it = mapOfDisks.begin();
    while (it != mapOfDisks.end()) {
        if (it->first == diskName)
            return it->second;
        ++it;
    }
    return MP_FAILED;
}

mp_int32 DataContext::SetDiskFdByName(const mp_string &diskName, mp_int32 diskFd)
{
    mp_int32 ret = MP_SUCCESS;
    mapOfDisks.insert(std::make_pair(diskName, diskFd));
    return ret;
}