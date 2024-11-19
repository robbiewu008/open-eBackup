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
#ifndef NUTANIX_STRUCTS_H
#define NUTANIX_STRUCTS_H
#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include <common/Structs.h>
#include "protect_engines/nutanix/common/NutanixConstants.h"

namespace NutanixPlugin {

struct ClusterListMetadata {
    int32_t count;
    int32_t endIndex;
    std::string filterCriteria;
    int32_t grandTotalEntities;
    std::string nextCursor;
    int32_t page;
    std::string previousCursor;
    std::string searchString;
    std::string sortCriteria;
    int32_t startIndex;
    int32_t totalEntities;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(count, count)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(endIndex, end_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(filterCriteria, filter_criteria)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(grandTotalEntities, grand_total_entities)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(nextCursor, next_cursor)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(page, page)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(previousCursor, previous_cursor)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(searchString, search_string)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sortCriteria, sort_criteria)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(startIndex, start_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(totalEntities, total_entities)
    END_SERIAL_MEMEBER
};

struct DiskAddress {
    std::string deviceBus; // 虚拟磁盘设备的设备总线， ['SCSI', 'IDE', 'PCI', 'SATA', 'SPAPR', 'NVME'],
    int32_t deviceIndex; // 适配器类型上的设备索引。如果未指定，系统将分配默认总线上的下一个可用插槽
    std::string deviceUuid;
    std::string diskLabel; // 已连接虚拟磁盘的磁盘标签（例如，scsi0:0）。这指示虚拟磁盘在VM中的位置
    std::string ndfsFilepath; // 现有虚拟磁盘的NDFS路径。
    std::string vmdiskUuid; // 虚拟磁盘标识符
    std::string volumeGroupUuid;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceBus, device_bus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceIndex, device_index)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(deviceUuid, device_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskLabel, disk_label)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ndfsFilepath, ndfs_filepath)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmdiskUuid, vmdisk_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(volumeGroupUuid, volume_group_uuid)
    END_SERIAL_MEMEBER
};


struct VmDiskInfo {
    std::string datasourceUuid; // 支持虚拟磁盘的数据源（在AOS群集之外）的UUID
    DiskAddress diskAddress; // Nested structure
    bool flashModeEnabled; // 是否启用闪存模式
    bool isCdrom; // 是否是光驱
    bool isEmpty; // 驱动器是否为空。此字段仅适用于CD-ROM
    bool isHotRemoveEnabled; // 当虚拟机处于运行状态时，是否可以移除磁盘
    bool isScsiPassthrough; // 此SCSI磁盘是否以直通模式挂载
    bool isThinProvisioned; // 磁盘是否精简配置。注意：此字段仅适用于ESX托管的虚拟机
    bool shared; // 是否是共享磁盘。
    uint64_t size; // 磁盘的大小（以字节为单位）
    DiskAddress sourceDiskAddress;
    std::string storageContainerUuid; // 当驱动器为空时，此字段未设置

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(datasourceUuid, datasource_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(flashModeEnabled, flash_mode_enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isCdrom, is_cdrom)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isEmpty, is_empty)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isHotRemoveEnabled, is_hot_remove_enabled)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isScsiPassthrough, is_scsi_passthrough)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isThinProvisioned, is_thin_provisioned)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(shared, shared)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sourceDiskAddress, source_disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerUuid, storage_container_uuid)
    END_SERIAL_MEMEBER
};

struct VmDiskClone {
    DiskAddress diskAddress;
    std::string snapshotGroupId; // 快照vmdisk的快照一致性组的UUID
    std::string storageContainerId; // 存储指定映像或vmdisk的存储容器的Uuid，源为快照必填

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(snapshotGroupId, snapshot_group_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerId, storage_container_uuid)
    END_SERIAL_MEMEBER
};
struct VmDisk {
    DiskAddress diskAddress;
    VmDiskClone vmDiskClone;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskClone, vm_disk_clone)
    END_SERIAL_MEMEBER
};

struct VmNic {
    std::string adapterType;
    std::string ipAddress;
    std::vector<std::string> ipAddresses;
    bool isConnected;
    std::string macAddress;
    std::string model;
    std::string networkUuid;
    std::string nicUuid;
    bool requestIp;
    std::string requestedIpAddress;
    std::string vlanMode;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(adapterType, adapter_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ipAddress, ip_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ipAddresses, ip_addresses)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isConnected, is_connected)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(macAddress, mac_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(model, model)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(networkUuid, network_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(nicUuid, nic_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(requestIp, request_ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(requestedIpAddress, requested_ip_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vlanMode, vlan_mode)
    END_SERIAL_MEMEBER
};

struct BootConfig {
    std::vector<std::string> bootDeviceOrder;
    std::string bootDeviceType;
    DiskAddress diskAddress;
    bool hardwareVirtualization;
    std::string macAddr;
    bool secureBoot;
    bool uefiBoot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bootDeviceOrder, boot_device_order)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bootDeviceType, boot_device_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(diskAddress, disk_address)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hardwareVirtualization, hardware_virtualization)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(macAddr, mac_addr)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(secureBoot, secure_boot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(uefiBoot, uefi_boot)
    END_SERIAL_MEMEBER
};

struct AffinityStruct {
    std::string policy = DEFAULT_AFFINITY_POLICY;
    std::vector<std::string> hostUuids;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(policy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hostUuids, host_uuids)
    END_SERIAL_MEMEBER
};

struct NutanixVMInfo {
    bool allowLiveMigrate; // 虚拟机是否可以实时迁移。
    BootConfig boot;
    std::string guestDriverVersion; // 驱动版本
    std::string guestOs; // 仅适用于ESX虚拟机
    std::string hostId; // 如果虚拟机已关闭，则此字段为空
    std::string machineType; // = ['PC', 'PSERIES', 'Q35'],
    int32_t memoryMb; // 分配给虚拟机器的RAM(MB)
    int32_t memoryReservationMb;
    std::string name;
    //  ['UNKNOWN', 'OFF', 'POWERING_ON', 'ON', 'SHUTTING_DOWN', 'POWERING_OFF', 'PAUSING', 'PAUSED', 'SUSPENDING',
    // 'SUSPENDED', 'RESUMING', 'RESETTING', 'MIGRATING'],
    std::string powerState;
    std::string storageContainerUuid;
    std::string timezone;
    std::string id;
    int32_t numCoresPerVcpu;
    int32_t numVcpus;
    std::vector<VmDiskInfo> vmDiskInfo; // 虚拟机的磁盘信息列表
    std::vector<VmDisk> vmDisks; // 要添加到虚拟机的磁盘规格列表
    std::vector<VmNic> vmNics; // 要添加到虚拟机的网卡列表
    AffinityStruct affinity;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(allowLiveMigrate, allow_live_migrate)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(boot, boot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(guestDriverVersion, guest_driver_version)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(guestOs, guest_os)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hostId, host_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(machineType, machine_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(memoryMb, memory_mb)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(memoryReservationMb, memory_reservation_mb)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(powerState, power_state)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(storageContainerUuid, storage_container_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(timezone, timezone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(id, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDiskInfo, vm_disk_info)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmDisks, vm_disks)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmNics, vm_nics)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numCoresPerVcpu, num_cores_per_vcpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(numVcpus, num_vcpus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(affinity, affinity)
    END_SERIAL_MEMEBER
};
struct NewVMNicStruct {
    bool isConnected;
    std::string model;
    std::string networkUuid;
    std::string requestedIpAddress;
    std::string vlanMode;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(isConnected, is_connected)
    SERIAL_MEMEBER(model)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(networkUuid, network_uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vlanMode, vlan_mode)
    if (serial) {
        if (jsonValue.isMember("requested_ip_address")) {
            Module::JsonHelper::JsonValueToType(requestedIpAddress, jsonValue["requested_ip_address"]);
        }
    } else if (requestedIpAddress != "") {
        Module::JsonHelper::TypeToJsonValue(requestedIpAddress, jsonValue["requested_ip_address"]);
    }
    END_SERIAL_MEMEBER
};

struct PMInterfacePortPairList {
    std::vector<struct NewVMNicStruct> m_detail;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_detail, detail)
    END_SERIAL_MEMEBER;
};

struct PMInterfaceHostUuidList {
    std::vector<std::string> m_detail;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_detail, detail)
    END_SERIAL_MEMEBER;
};

struct RestoreNICParm {
    std::string originNicId;
    std::string targetNetworkId;
    std::string targetIp;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMEBER(originNicId)
    SERIAL_MEMEBER(targetNetworkId)
    SERIAL_MEMEBER(targetIp)
    END_SERIAL_MEMEBER;
};

struct RestoreNICList {
    std::vector<RestoreNICParm> m_detail;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_detail, detail)
    END_SERIAL_MEMEBER;
};

struct NutanixRestoreExtendInfo {
    std::string restoreLevel;
    std::string vmName;
    std::string openInterface;
    std::string bridgeInterface;
    std::string hostUuids;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(restoreLevel, restoreLevel)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(vmName, vmName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(openInterface, openInterface)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(bridgeInterface, bridgeInterface)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(hostUuids, hostUuids)
    END_SERIAL_MEMEBER
};

struct NewVolumeInfo {
    std::string agentVmUuid;
    std::string tmpVolUuid;
    std::string busType;
    int32_t busIndex = -1;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(agentVmUuid, agentVmUuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(tmpVolUuid, tmpVolUuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(busIndex, busIndex)
    END_SERIAL_MEMEBER
};

struct ContainerUserStats {
    uint64_t userFreeBytes;

    BEGIN_SERIAL_MEMEBER
    if (serial) {
        if (jsonValue.isMember("storage.user_free_bytes")) {
            std::string tmp = jsonValue["storage.user_free_bytes"].asString();
            userFreeBytes = std::stoull(tmp);
            ERRLOG("The field is storage.user_free_bytes: %s, trans as%llu", jsonValue["storage.user_free_bytes"].asString().c_str(), userFreeBytes);
        } else {
            ERRLOG("The field is not storage.user_free_bytes");
        }
        if (jsonValue.isMember("storage")) {
            ERRLOG("The fields has storage");
        } else {
            ERRLOG("The fields has no storage");
        }
    } else {
        Module::JsonHelper::TypeToJsonValue(userFreeBytes, jsonValue["storage.user_free_bytes"]);
    }
    END_SERIAL_MEMEBER
};

}
#endif