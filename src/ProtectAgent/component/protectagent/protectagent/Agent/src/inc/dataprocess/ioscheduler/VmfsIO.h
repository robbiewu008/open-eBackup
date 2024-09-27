/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * @file VmfsIO.h
 * @brief  VmfsIO5 interface defination
 * @version 1.0.0.0
 * @date 2023-11-20
 * @author hebaolong 00606494
 */

#ifndef __VMFS_IO_H__
#define __VMFS_IO_H__

#include <cstring>
#include <vector>
#include <set>

#include "IOEngine.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"
#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"

namespace NSVmfsIO {

enum class VmfsVerT {
    VMFS5 = 5,
    VMFS6
};

class VmfsIO {
public:
    VmfsIO(std::vector<std::string> &wwn) : m_wwn(wwn) {}
    virtual ~VmfsIO()
    {
        COMMLOG(OS_LOG_DEBUG, "");
        Vmfs5IO::VmfsFs::Instance()->FSClose(m_vmfs);
        Vmfs6IO::VmfsFs::Instance()->FSClose(m_vmfs6);
    }

    /* vmfsio initialization - create vmfs-fs instance */
    mp_int32 Init();

    VmfsVerT GetVmfsVersion()
    {
        return m_vmfsVersion;
    }

    Vmfs5IO::VmfsFsT *GetVmfsHandler()
    {
        return m_vmfs;
    }

    Vmfs6IO::VmfsFsT *GetVmfs6Handler()
    {
        return m_vmfs6;
    }

private:
    bool ResolveDevicePath(const std::string &wwn);
    void GetFiles(const std::string &pathName, std::vector<std::string> &files);

private:
    std::vector<std::string> m_wwn;
    std::set<mp_string> m_device;
    Vmfs5IO::VmfsFsT *m_vmfs = nullptr;
    Vmfs6IO::VmfsFsT *m_vmfs6 = nullptr;
    VmfsVerT m_vmfsVersion = VmfsVerT::VMFS5;
};

}; // namespace VmfsIO

#endif