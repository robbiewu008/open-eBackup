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