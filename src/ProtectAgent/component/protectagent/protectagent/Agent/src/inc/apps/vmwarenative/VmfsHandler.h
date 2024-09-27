#ifndef __VMFSHANDLER_H__
#define __VMFSHANDLER_H__

#include <vector>
#include "common/Types.h"

class VmfsHandler {
public:
    VmfsHandler();
    ~VmfsHandler();

    mp_int32 CheckTool();
    mp_int32 Mount(const std::vector<mp_string> &wwn, mp_string &mountpoint);
    mp_int32 Umount(const mp_string& mountpoint);
};
#endif