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
#ifndef APSARA_STACK_STRUCTS_H
#define APSARA_STACK_STRUCTS_H

#include <vector>
#include <string>
#include <common/JsonHelper.h>

namespace  VirtPlugin {

// SDK请求公共参数
struct CommonRequest {
    std::string m_accessKeyId;
    std::string m_accessKeySecret;
    std::string m_regionId;
    std::string m_endPoint;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessKeyId, AccessKeyId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessKeySecret, AccessKeySecret)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, RegionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_endPoint, EndPoint)
    END_SERIAL_MEMEBER
};

struct APSResponse {
    int32_t m_statusCode = 200;
    std::string m_erro;
    std::string m_body;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_statusCode, StatusCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_erro, Error)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_body, Body)
    END_SERIAL_MEMEBER
};

struct CommonApsaraExtendInfo {
    std::string m_organizationId;
    std::string m_regionId;
    std::string m_accessKeyId;
    std::string m_accessKeySecret;
    std::string m_proxy;
    std::string mResourceGroupId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_organizationId, organizationId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, regionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessKeyId, AccessKeyId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_accessKeySecret, AccessKeySecret)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_proxy, Proxy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceGroupId, resourceGroupId)
    END_SERIAL_MEMEBER
};

// Region
struct Region {
    std::string m_regionId;
    std::string m_localName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, RegionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_localName, LocalName)
    END_SERIAL_MEMEBER
};

struct RegionArray {
    std::vector<Region> m_region;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_region, Region)
    END_SERIAL_MEMEBER
};

struct RegionResponse {
    RegionArray m_regions;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regions, Regions)
    END_SERIAL_MEMEBER
};

// Zone
struct Zone {
    std::string m_zoneId;
    std::string m_localName;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zoneId, ZoneId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_localName, LocalName)
    END_SERIAL_MEMEBER
};

struct ZoneArray {
    std::vector<Zone> m_zone;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zone, Zone)
    END_SERIAL_MEMEBER
};

struct ZoneResponse {
    ZoneArray m_zones;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zones, Zones)
    END_SERIAL_MEMEBER
};

struct NetworkInterface {
    std::string m_type;
    std::string m_primaryIp;
    std::string m_macAddress;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, Type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_primaryIp, PrimaryIpAddress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_macAddress, MacAddress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, NetworkInterfaceId)
    END_SERIAL_MEMEBER
};

struct NetworkInterfaces {
    std::vector<NetworkInterface> m_networkInterface;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkInterface, NetworkInterface)
    END_SERIAL_MEMEBER
};


// Instance
struct Instance {
    std::string m_name;
    std::string m_id;
    std::string m_regionId;
    std::string m_status;

    int m_memory = 0;
    int m_cpu = 0;
    std::string m_osName;
    std::string m_osType;
    std::string m_hostName;
    std::string m_zoneId;
    std::string m_resourceGroupId;
    std::string m_resourceGroupName;
    std::string m_instanceType;
    int32_t mResourceGroup;
    NetworkInterfaces m_networkInterfaces;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, InstanceName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, InstanceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, RegionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, Status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_memory, Memory)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cpu, Cpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osName, OSName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osType, OSType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostName, HostName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zoneId, ZoneId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceGroupId, ResourceGroupId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceGroupName, ResourceGroupName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceType, InstanceType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceGroup, ResourceGroup)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_networkInterfaces, NetworkInterfaces)
    END_SERIAL_MEMEBER
};

struct InstanceArray {
    std::vector<Instance> m_instance;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instance, Instance)
    END_SERIAL_MEMEBER
};

struct InstanceResponse {
    InstanceArray m_instances;
    int32_t m_total = 0;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instances, Instances)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, TotalCount)
    END_SERIAL_MEMEBER
};

struct AttachMent {
    std::string m_instanceId;
    std::string m_device;
    std::string m_attachTime;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceId, InstanceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_device, Device)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachTime, AttachedTime)
    END_SERIAL_MEMEBER
};

struct AttachMents {
    std::vector<AttachMent> m_attachment;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachment, Attachment)
    END_SERIAL_MEMEBER
};

// Disk
struct Disk {
    std::string m_name;
    std::string m_id;
    std::string m_regionId;
    std::string m_status;

    uint64_t m_size = 0;
    std::string m_sourceSnapshotId;
    std::string m_storageSetId;
    std::string m_zoneId;
    std::string m_instanceId;
    std::string m_device;
    std::string m_type;
    std::string m_multiAttach; // 是否开启了多重挂载特性
    std::string m_imageId;
    std::string m_description;
    bool m_portable = false; // true：支持。可以独立存在，且可以在可用区内自由挂载和卸载。false：不支持。不可以独立存在，且不可以在可用区内自由挂载和卸载。
    std::string m_category; // 数据盘类型
    // cloud：普通云盘
    // cloud_efficiency：高效云盘
    // cloud_ssd：SSD盘
    // cloud_essd：ESSD云盘
    // cloud_pperf：高性能云盘
    // cloud_sperf：普通性能云盘
    // 仅高性能云盘和普通性能云盘支持快照一致性组
    AttachMents m_attachMents;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, DiskName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, DiskId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_regionId, RegionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, Status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, Size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceSnapshotId, SourceSnapshotId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageSetId, StorageSetId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_zoneId, ZoneId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceId, InstanceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_device, Device)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, Type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_multiAttach, MultiAttach)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_imageId, ImageId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, Description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_category, Category)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_portable, Portable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachMents, Attachments)
    END_SERIAL_MEMEBER
};

struct DiskArray {
    std::vector<Disk> m_disk;
    
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_disk, Disk)
    END_SERIAL_MEMEBER
};

struct DiskResponse {
    DiskArray m_disks;
    int32_t m_total = 0;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_disks, Disks)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, TotalCount)
    END_SERIAL_MEMEBER
};

struct ResourceSet {
    int32_t mOrganizationId;
    std::string mCreator;
    std::string mResourceGroupName;
    std::string mOrganizationName;
    int32_t mId;
    std::string mSetId;
    int32_t mResourceGroupType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOrganizationId, organizationID)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCreator, creator)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceGroupName, resourceGroupName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOrganizationName, organizationName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSetId, rsId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceGroupType, resourceGroupType)
    END_SERIAL_MEMEBER
};

struct ResourceSetResponse {
    std::string mCode;
    int32_t mCost;
    std::vector<ResourceSet> mData;
    std::string mMessage;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCode, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCost, cost)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mData, data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMessage, message)
    END_SERIAL_MEMEBER
};

struct CreateSnapshotGroupResponse {
    std::string m_requestId;
    std::string m_snapshotGroupId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_requestId, RequestId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotGroupId, SnapshotGroupId)
    END_SERIAL_MEMEBER
};

struct APSSnapshot {
    std::string m_id;
    std::string m_name;
    std::string m_progress;
    std::string m_sourceDiskId;
    std::string m_sourceDiskType;
    std::string m_status;
    std::string m_description;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, SnapshotId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, SnapshotName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_progress, Progress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceDiskId, SourceDiskId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceDiskType, SourceDiskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, Status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, Description)
    END_SERIAL_MEMEBER
};

struct APSSnapshots {
    std::vector<APSSnapshot> m_snapshot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshot, Snapshot)
    END_SERIAL_MEMEBER
};

struct APSSnapshotGroup {
    std::string m_snapshotGroupId;
    std::string m_instanceId;
    std::string m_name;
    std::string m_status;
    APSSnapshots m_snapshots;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotGroupId, SnapshotGroupId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_instanceId, InstanceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, Status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshots, Snapshots)
    END_SERIAL_MEMEBER
};
 
struct APSSnapshotGroups {
    std::vector<APSSnapshotGroup> m_snapshotGroup;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotGroup, SnapshotGroup)
    END_SERIAL_MEMEBER
};
struct APSGetSnapshotGroupResponse {
    std::string m_requestId;
    APSSnapshotGroups m_snapshotGroups;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_requestId, RequestId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotGroups, SnapshotGroups)
    END_SERIAL_MEMEBER
};
 
struct APSCreateSnapshotResponse {
    std::string m_requestId;
    std::string m_snapshotId;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_requestId, RequestId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotId, SnapshotId)
    END_SERIAL_MEMEBER
};
 
struct SnapshotArray {
    std::vector<APSSnapshot> m_snapshot;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshot, Snapshot)
    END_SERIAL_MEMEBER
};
 
struct APSGetSnapshotResponse {
    int32_t m_pageNumber;
    int32_t m_pageSize;
    int32_t m_totalCount;
    SnapshotArray m_snapshots;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pageNumber, PageNumber)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pageSize, PageSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_totalCount, TotalCount)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshots, Snapshots)
    END_SERIAL_MEMEBER
};
 
struct APSResourceInfo {
    std::string m_name;
    std::string m_value;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, Name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_value, Value)
    END_SERIAL_MEMEBER
};
 
struct APSResourceSet {
    std::vector<APSResourceInfo> m_relatedItem;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_relatedItem, RelatedItem)
    END_SERIAL_MEMEBER
};
 
struct APSErroMsg {
    std::string m_errorMsg;
    std::string m_errorCode;
    std::string m_operationStatus;
    APSResourceSet m_relatedItemSet;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorMsg, ErrorMsg)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_errorCode, ErrorCode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_operationStatus, OperationStatus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_relatedItemSet, RelatedItemSet)
    END_SERIAL_MEMEBER
};
 
struct APSErroMsgs {
    std::vector<APSErroMsg> m_operationProgress;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_operationProgress, OperationProgress)
    END_SERIAL_MEMEBER
};
 
struct APSDeleteSnapshotGroupResponse {
    std::string m_requestId;
    APSErroMsgs m_operationProgressSet;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_requestId, RequestId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_operationProgressSet, OperationProgressSet)
    END_SERIAL_MEMEBER
};
 
struct APSCreateDiskResponse {
    std::string m_diskId;
    std::string m_requestId;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_requestId, RequestId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskId, DiskId)
    END_SERIAL_MEMEBER
};

struct ListResourcePara {
    std::string mRegionId;
    std::string mRegionName;
    std::string mResourceType;
    std::string mOrganizationId;
    std::string mResourceGroupId;
    std::string mInstanceId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mRegionId, regionId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mRegionName, regionName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceType, resourceType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mOrganizationId, organizationId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mResourceGroupId, resourceSetId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mInstanceId, uuid)
    END_SERIAL_MEMEBER
};

struct Organization {
    std::string mUuid;
    int32_t mId;
    std::string mName;
    std::string mSupportRegions;
    std::string mLevel;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mUuid, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mId, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mSupportRegions, supportRegions)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mLevel, level)
    END_SERIAL_MEMEBER
};

struct DesOrganizationResponse {
    std::string mCode;
    int32_t mCost;
    Organization mData;
    std::string mMessage;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCode, code)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mCost, cost)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mData, data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(mMessage, message)
    END_SERIAL_MEMEBER
};
 
} /* namespace  ApsaraStackPlugin */
#endif
