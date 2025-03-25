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
#ifndef __VMFS5_IO_ENGINE_H__
#define __VMFS5_IO_ENGINE_H__

#include <cstring>
#include <vector>

#include "VmfsIO.h"
#include "IOEngine.h"
#include "apps/vmwarenative/VMwareDef.h"
#include "dataprocess/ioscheduler/libvmfs/Vmfs.h"

namespace NSVmfsIO {

/* support read only */
class Vmfs5IOEngine : public IOEngine {
public:
    Vmfs5IOEngine(Vmfs5IO::VmfsFsT *vmfsptr, const vmware_volume_info& vol, mp_int32 protectType)
        : m_vmfs(vmfsptr), m_volInfo(vol), m_protectType(protectType) {}
    virtual ~Vmfs5IOEngine() = default;

    virtual mp_int32 Open() override;
    virtual mp_int32 Read(const uint64_t& offsetInBytes,
        uint64_t& bufferSizeInBytes, unsigned char* buffer) override;
    virtual mp_int32 Write(const uint64_t& offsetInBytes,
        uint64_t& bufferSizeInBytes, unsigned char* buffer) override
    {
        return MP_FAILED;
    }
    virtual mp_int32 Close() override;
    virtual mp_int32 Exist(const std::string &file);

    mp_string GetFileName() override
    {
        return m_filename;
    }

private:
    mp_int32 SetDiskPath();
    mp_int32 VmfsIOLookup(const char *name, bool checkOnly = false);
    void Split(const std::string &str, const char split, std::vector<std::string> &res);

private:
    mp_int32 m_protectType;
    mp_string m_filename;
    Vmfs5IO::VmfsFsT *m_vmfs = nullptr;
    Vmfs5IO::VmfsFileT *m_file = nullptr;
    Vmfs5IO::VmfsInoT m_ino = VMFS5_ROOT_INO;
    const vmware_volume_info& m_volInfo;  // vmware_volume_info中vecDirtyRange数据量大，需以引用方式使用
};

}; // namespace VmfsIO

#endif