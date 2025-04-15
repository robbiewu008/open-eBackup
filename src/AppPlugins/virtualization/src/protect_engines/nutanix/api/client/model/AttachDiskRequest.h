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
#ifndef NUTANIX_ATTACH_DISK_REQ_H
#define NUTANIX_ATTACH_DISK_REQ_H
 
#include <string>
#include <vector>
#include <common/JsonHelper.h>
#include "common/model/ResponseModel.h"
#include "common/Structs.h"
#include "NutanixRequest.h"
#include "protect_engines/nutanix/common/NutanixStructs.h"

namespace NutanixPlugin {

struct AttachDiskAddressWithDNFSFilePath {
    std::string ndfsFilepath; // 现有虚拟磁盘的NDFS路径。

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ndfsFilepath, ndfs_filepath)
    END_SERIAL_MEMEBER
};

struct AttachDiskClone {
    AttachDiskAddressWithDNFSFilePath attachDiskAddress;
    std::string snapshotGroupId; // 快照vmDisk的快照一致性组的UUID
    std::string storageContainerId; // 存储指定映像或vmDisk的存储容器的Uuid，源为快照必填

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachDiskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapshotGroupId, snapshot_group_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerId, storage_container_uuid)
    END_SERIAL_MEMEBER
};

struct AttachSnapshotCloneVmDisk {
    AttachDiskClone attachDiskClone;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachDiskClone, vm_disk_clone)
    END_SERIAL_MEMEBER
};

struct AttachDiskAddress {
    std::string deviceBus; // 虚拟磁盘设备的设备总线， ['SCSI', 'IDE', 'PCI', 'SATA', 'SPAPR', 'NVME'],
    int32_t deviceIndex; // 适配器类型上的设备索引。如果未指定，系统将分配默认总线上的下一个可用插槽
    std::string ndfsFilepath; // 现有虚拟磁盘的NDFS路径。
    std::string vmDiskId; // 虚拟磁盘标识符
    std::string volumeGroupUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceBus, device_bus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceIndex, device_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ndfsFilepath, ndfs_filepath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskId, vmdisk_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeGroupUuid, volume_group_uuid)
    END_SERIAL_MEMEBER
};

struct AttachDiskCreate {
    uint64_t size = 0;
    std::string storageContainerUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerUuid, storage_container_uuid)
    END_SERIAL_MEMEBER
};

struct AttachCreateNewVmDisk {
    AttachDiskCreate vmDiskCreate;
    AttachDiskAddress attachDiskAddress;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskCreate, vm_disk_create)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(attachDiskAddress, disk_address)
    END_SERIAL_MEMEBER
};

struct DiskAddressCloneById {
    std::string vmDiskUuid;;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskUuid, vmdisk_uuid)
    END_SERIAL_MEMEBER
};

struct DiskCloneById {
    DiskAddressCloneById diskAddressCloneById;;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddressCloneById, disk_address)
    END_SERIAL_MEMEBER
};

struct AttachCloneByIdVmDisk {
    DiskCloneById diskCloneById;
    AttachDiskAddress diskAddress;
    std::string storageContainerUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerUuid, storage_container_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskCloneById, vm_disk_clone)
    END_SERIAL_MEMEBER
};

struct AttachDiskInfoCloneBySnapshot {
    std::vector<AttachSnapshotCloneVmDisk> vmDisks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDisks, vm_disks)
    END_SERIAL_MEMEBER
};

struct AttachDiskInfoCreateNew {
    std::vector<AttachCreateNewVmDisk> vmDisks;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDisks, vm_disks)
    END_SERIAL_MEMEBER
};

struct AttachDiskInfoCloneById {
    std::vector<AttachCloneByIdVmDisk> vmDisks;
    std::string uuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDisks, vm_disks)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(uuid, uuid)
    END_SERIAL_MEMEBER
};

using namespace VirtPlugin;

class AttachDiskRequest : public NutanixRequest {
public:
    explicit AttachDiskRequest(std::string vmId) : m_vmId(vmId)
    {
        url = "PrismGateway/services/rest/v2.0/vms/{vmId}/disks/attach";
    }
    ~AttachDiskRequest() = default;

    std::string GetVmId()
    {
        return m_vmId;
    }

    void SetSnapshotCloneVmDisk(const VmDisk &vmDisk)
    {
        AttachSnapshotCloneVmDisk tmp;
        tmp.attachDiskClone.attachDiskAddress.ndfsFilepath = vmDisk.vmDiskClone.diskAddress.ndfsFilepath;
        tmp.attachDiskClone.snapshotGroupId = vmDisk.vmDiskClone.snapshotGroupId;
        tmp.attachDiskClone.storageContainerId = vmDisk.vmDiskClone.storageContainerId;
        m_attachSnapShotCloneVmDisks.vmDisks.emplace_back(tmp);
    }

    void SetNewCreateVmDisk(const AttachCreateNewVmDisk &newDisk, const std::string &vmId)
    {
        m_attachNewCreateVmDisks.vmDisks.emplace_back(newDisk);
    }

    void SetCloneByIdVmDisk(const AttachCloneByIdVmDisk &disk, const std::string &vmId)
    {
        m_attachCloneByIdVmDisks.vmDisks.emplace_back(disk);
        m_attachCloneByIdVmDisks.uuid = vmId;
    }

    void FillRequest(RequestInfo &requestInfo)
    {
        requestInfo.m_method = "POST";
        requestInfo.m_pathParams["vmId"]= m_vmId;
        std::string jsonStr;
        if (!m_attachSnapShotCloneVmDisks.vmDisks.empty()) {
            if (! Module::JsonHelper::StructToJsonString(m_attachSnapShotCloneVmDisks, requestInfo.m_body)) {
                ERRLOG("Convert AttachDiskInfo failed.");
                return;
            }
        } else if (!m_attachNewCreateVmDisks.vmDisks.empty()) {
            if (! Module::JsonHelper::StructToJsonString(m_attachNewCreateVmDisks, requestInfo.m_body)) {
                ERRLOG("Convert AttachDiskInfo failed.");
                return;
            }
        } else if (!m_attachCloneByIdVmDisks.vmDisks.empty()) {
            if (! Module::JsonHelper::StructToJsonString(m_attachCloneByIdVmDisks, requestInfo.m_body)) {
                ERRLOG("Convert AttachDiskInfo failed.");
                return;
            }
        }
    }

private:
    AttachDiskInfoCloneBySnapshot m_attachSnapShotCloneVmDisks;
    AttachDiskInfoCreateNew m_attachNewCreateVmDisks;
    AttachDiskInfoCloneById m_attachCloneByIdVmDisks;
    std::string m_vmId;
    std::string m_sourceVmId;
};

};
 
#endif