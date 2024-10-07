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
#ifndef OPENSTACK_VOLUME_DETAILS_H
#define OPENSTACK_VOLUME_DETAILS_H

#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class OsVolRepDriverData {
public:
    std::string m_lunId;
    std::string m_sn;
    std::string m_ip;      // fusionstorage 管理IP
    int32_t m_pool;    // 卷所在存储池ID
    std::string m_volName;    // fusionstorage 卷名称
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunId, lun_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sn, sn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pool, pool)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volName, vol_name)
    END_SERIAL_MEMEBER
};

class VolumeImageMetadata {
public:
    std::string m_size;
    std::string m_arch;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_arch, architecture)
    END_SERIAL_MEMEBER
};

class VolumeMetaData {
public:
    std::string m_takeOverLunWWN;
    std::string m_lunWWN;
    std::string m_storageType;
    std::string m_readonly;
    std::string m_attachedMode;
    std::string m_fullClone;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_takeOverLunWWN, take_over_lun_wwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunWWN, lun_wwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageType, StorageType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_readonly, readonly)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachedMode, attached_mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fullClone, full_clone)
    END_SERIAL_MEMEBER
};

struct VolumeLink {
    std::string m_href;     // 对应快捷链接
    std::string m_rel;     // 快捷链接标记名称，枚举值：self, bookmark, alternate
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_href, href)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_rel, rel)
    END_SERIAL_MEMEBER
};

struct VolumeAttachment {
    std::string m_serverId;
    std::string m_attachmentId;
    std::string m_attachedTime;
    std::string m_hostName;
    std::string m_volumeId;
    std::string m_device;
    std::string m_id;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serverId, server_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachmentId, attachment_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachedTime, attached_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostName, host_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeId, volume_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_device, device)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

struct Volume {
    std::vector<VolumeAttachment> m_attachPoints;        // 挂载信息
    std::string m_availabilityZone;
    std::string m_bootable;
    bool m_shareable {false};
    std::string m_consistencygroupId;
    std::string m_createdTime;
    std::string m_description;
    bool m_encrypted {false};
    std::string m_id;                // 卷的UUID
    std::vector<VolumeLink> m_links;
    VolumeMetaData m_metaData;
    std::string m_migrationSstatus;
    bool m_multiattach {false};
    std::string m_name;              // 卷名称
    std::string m_replicationStatus;
    uint64_t m_size {0};                 // 卷大小
    std::string m_snapshotId;
    std::string m_sourceVolid;
    std::string m_status;
    std::string m_updatedTime;
    std::string m_userId;
    std::string m_volumeType;                // 卷类型
    VolumeImageMetadata m_volImageMetadata;
    std::string m_osVolRepDriverData;
    std::string m_groupId;                   // new in version 3.13
    std::string m_providerId;                // new in version 3.21
    bool m_sharedTargets {true};             // avalibel in version 3.68
    std::string m_clusterName;               // new in version 3.61
    std::string m_volumeTypeId;              // new in version 3.63
    bool m_consumesQuota {true};             // new in version 3.65
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachPoints, attachments)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_availabilityZone, availability_zone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootable, bootable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistencygroupId, consistencygroup_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createdTime, created_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encrypted, encrypted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_links, links)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metaData, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_migrationSstatus, migration_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_multiattach, multiattach)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_replicationStatus, replication_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotId, snapshot_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceVolid, source_volid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_updatedTime, updated_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userId, user_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volImageMetadata, volume_image_metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osVolRepDriverData, os-volume-replication:driver_data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupId, group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_providerId, provider_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sharedTargets, shared_targets)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_clusterName, cluster_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeTypeId, volume_type_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consumesQuota, consumes_quota)
    END_SERIAL_MEMEBER
};

struct VolumeConfMetaData {
    std::string m_fullClone;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_fullClone, full_clone)
    END_SERIAL_MEMEBER
};

struct NewVolumeConfInfo {
    uint64_t m_size {10};
    std::string m_availabilityZone;
    std::string m_sourceVolid;
    std::string m_description;
    bool m_multiattach {false};
    std::string m_snapshotId;
    std::string m_backupId;
    std::string m_name;
    std::string m_imageReference;
    std::string m_volumeType;
    VolumeConfMetaData m_metadata;
    std::string m_consistencygroupId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_availabilityZone, availability_zone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceVolid, source_volid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_multiattach, multiattach)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotId, snapshot_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_backupId, backup_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_imageReference, imageRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistencygroupId, consistencygroup_id)
    END_SERIAL_MEMEBER
};

struct VolumeRequestBody {
    NewVolumeConfInfo m_volume;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volume, volume)
    END_SERIAL_MEMEBER
};

struct VolumeResponseBody {
    Volume m_volume;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volume, volume)
    END_SERIAL_MEMEBER
};

class VolumeList {
public:
    std::vector<Volume> m_volumeList;     // 磁盘详细信息

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeList, volumes)
    END_SERIAL_MEMEBER
};

class VolumeDetail {
public:
    Volume m_volume;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volume, volume)
    END_SERIAL_MEMEBER
};

class VolumeExtraSpecs {
public:
    std::string m_volumeBackendName;
    std::string m_availabilityZone;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeBackendName, volume_backend_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_availabilityZone, HW:availability_zone)
    END_SERIAL_MEMEBER
};
 
class VolumeType {
public:
    std::string m_name;
    std::string m_id;
    std::string m_description;
    bool m_isPublic;
    VolumeExtraSpecs m_extraSpecs;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isPublic, is_public)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extraSpecs, extra_specs)
    END_SERIAL_MEMEBER
};
 
class VolumeTypeList {
public:
    std::vector<VolumeType> m_volumeTypes;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeTypes, volume_types)
    END_SERIAL_MEMEBER
};

class VolumeTypeRequestBodyMsg {
public:
    VolumeType m_volumeType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    END_SERIAL_MEMEBER
};
OPENSTACK_PLUGIN_NAMESPACE_END
#endif