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
#ifndef __ADD_DISK_REQUEST__
#define __ADD_DISK_REQUEST__

#include <string>
#include "protect_engines/cnware/common/Structs.h"
#include "../CNwareRequest.h"

namespace CNwarePlugin {

struct AddDomainDiskDevicesReq {
    int32_t bus = 0;
    std::string oldPool;
    std::string oldVol;
    std::string preallocation = "off";
    int32_t source = 0;
    int32_t cache = 1; // 1.直接读写(directsync) 2.一级物理缓存(writethrough) 3.二级虚拟缓存(writeback) 4.一级虚拟缓存
    int64_t ioHangTimeout = 1800000; // 毫秒
    bool shareable = false;
    int32_t type = 1; // 1.智能：qcow2 (默认),2.高速 raw

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bus)
    SERIAL_MEMEBER(oldPool)
    SERIAL_MEMEBER(oldVol)
    SERIAL_MEMEBER(preallocation)
    SERIAL_MEMEBER(source)
    SERIAL_MEMEBER(cache)
    SERIAL_MEMEBER(ioHangTimeout)
    SERIAL_MEMEBER(shareable)
    SERIAL_MEMEBER(type)
    END_SERIAL_MEMEBER
};

class AddDiskRequest : public CNwareRequest {
public:
    AddDiskRequest() {}
    ~AddDiskRequest() {}

    void SetDomainDiskDevices(const AddDomainDiskDevicesReq &disk)
    {
        m_disk = disk;
    }

    AddDomainDiskDevicesReq GetDomainDiskDevices()
    {
        return m_disk;
    }

    int32_t BuildRequestBodyString()
    {
        if (!Module::JsonHelper::StructToJsonString(m_disk, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        DBGLOG("Convert to string: %s", WIPE_SENSITIVE(m_body).c_str());
        return VirtPlugin::SUCCESS;
    }

private:
    AddDomainDiskDevicesReq m_disk;
};
};

# endif