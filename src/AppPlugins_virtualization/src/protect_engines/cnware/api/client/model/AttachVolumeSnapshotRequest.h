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
#ifndef ATTACH_VOLUME_SNAPSHOT_DISK_REQ_H
#define ATTACH_VOLUME_SNAPSHOT_DISK_REQ_H
 
#include <string>
#include "CNwareRequest.h"

namespace CNwarePlugin {
enum class CNWARE_MOUNT_BUS_TYPE {
    VIRT_IO = 1,
    IDE = 2,
    SCSI = 3,
    SATA = 4,
    USB = 5
};

struct MountDiskReq {
    uint32_t m_bus = static_cast<int32_t>(CNWARE_MOUNT_BUS_TYPE::VIRT_IO);
    std::string m_oldPool;
    std::string m_oldVol;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bus, bus);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oldPool, oldPool);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_oldVol, oldVol);
    END_SERIAL_MEMEBER;
};

class AttachVolumeSnapshotRequest : public CNwareRequest {
public:
    AttachVolumeSnapshotRequest() {}
    ~AttachVolumeSnapshotRequest() {}
 
    void SetDomainId(const std::string &domainId)
    {
        m_domainId = domainId;
    }

    void SetMountDiskReq(const MountDiskReq &req)
    {
        m_mountDiskReq = req;
    }

    int32_t MountDiskReqToJson()
    {
        if (!Module::JsonHelper::StructToJsonString(m_mountDiskReq, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        return VirtPlugin::SUCCESS;
    }

    std::string GetDomainId()
    {
        return m_domainId;
    }
 
private:
    std::string m_domainId;
    MountDiskReq m_mountDiskReq;
};
};
 
#endif