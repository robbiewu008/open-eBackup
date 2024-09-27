#ifndef __AGENT_DISK_OM_H__
#define __AGENT_DISK_OM_H__

#include <vector>
#include <map>

#include "array/storage_defines.h"

// oecan mobility used
class DiskOM {
public:
    static mp_int32 IsSSD(media_type_t& mediaType, mp_string& strDeviceName, mp_string& strDeviceNum);
    static mp_int32 GetDiskSize(mp_string& strDevieName, mp_uint64& iDiskSize);
private:
    static mp_int32 GetDiskHWType(std::vector<mp_string>& vecResult, media_type_t& mediaType,
    mp_string& strDeviceName, mp_string& strDeviceNum);
};

#endif

