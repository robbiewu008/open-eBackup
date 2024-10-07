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
#ifndef OPENSTACK_GROUP_DETAILS_H
#define OPENSTACK_GROUP_DETAILS_H

#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

class GroupSpecs {
public:
    std::string m_consistentGroupSnapshotEnable;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistentGroupSnapshotEnable, consistent_group_snapshot_enabled)
    END_SERIAL_MEMEBER
};

class GroupTypeRequestBody {
public:
    std::string m_name;
    std::string m_description;
    bool m_isPublic;
    GroupSpecs m_groupspecs;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isPublic, is_public)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupspecs, group_specs)
    END_SERIAL_MEMEBER
};

class GroupTypeRequestBodyMsg {
public:
    GroupTypeRequestBody m_groupType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupType, group_type)
    END_SERIAL_MEMEBER
};

class GroupRequest {
public:
    std::string m_name;
    std::string m_description;
    std::string m_groupType;
    std::vector<std::string> m_volumeType;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_types)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupType, group_type)
    END_SERIAL_MEMEBER
};

class VolumeGroupRequestBodyMsg {
public:
    GroupRequest m_group;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_group, group)
    END_SERIAL_MEMEBER
};

class GroupResponse {
public:
    std::string m_id;
    std::string m_name;
    std::string m_status;
    std::string m_availabilityZone;
    std::string m_createAt;
    std::string m_description;
    std::string m_groupType;
    std::vector<std::string> m_volumeTypes;
    std::vector<std::string> m_volumes;
    std::string m_groupSnapshotId;
    std::string m_sourceGroupId;
    std::string m_projectId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_availabilityZone, m_availabilityZone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createAt, created_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupType, group_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeTypes, volume_types)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumes, volumes)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshotId, group_snapshot_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sourceGroupId, source_group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, project_id)
    END_SERIAL_MEMEBER
};

class VolumeGroupResponseBodyMsg {
public:
    GroupResponse m_group;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_group, group)
    END_SERIAL_MEMEBER
};

class GroupTypeResponseBody {
public:
    std::string m_id;
    std::string m_name;
    std::string m_description;
    bool m_isPublic {false};
    GroupSpecs m_groupspecs;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isPublic, is_public)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupspecs, group_specs)
    END_SERIAL_MEMEBER
};
 
class GroupTypeResponseBodyMsg {
public:
    GroupTypeResponseBody m_groupType;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupType, group_type)
    END_SERIAL_MEMEBER
};

class UpdateGroupRequestBody {
public:
    std::string m_name;
    std::string m_description;
    std::string m_addVolumes;
    std::string m_removeVolumes;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_addVolumes, add_volumes)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_removeVolumes, remove_volumes)
    END_SERIAL_MEMEBER
};
class UpdateGroupRequestBodyMsg {
public:
    UpdateGroupRequestBody m_group;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_group, group)
    END_SERIAL_MEMEBER
};

class GroupSnapShotRequestBody {
public:
    std::string m_groupId;
    std::string m_name;
    std::string m_description;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupId, group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    END_SERIAL_MEMEBER
};

class GroupSnapShotRequestBodyMsg {
public:
    GroupSnapShotRequestBody m_groupSnapshot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshot, group_snapshot)
    END_SERIAL_MEMEBER
};

class GroupSnapShotResponseBody {
public:
    std::string m_id;
    std::string m_name;
    std::string m_groupTypeId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupTypeId, group_type_id)
    END_SERIAL_MEMEBER
};

class GroupSnapShotResponseBodyMsg {
public:
    GroupSnapShotResponseBody m_groupSnapshot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshot, group_snapshot)
    END_SERIAL_MEMEBER
};

class DeleteVolumes {
public:
    bool m_deleteVolumes;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_deleteVolumes, delete-volumes)
    END_SERIAL_MEMEBER
};
 
class DeleteVolumeGroupRequestBodyMsg {
public:
    DeleteVolumes m_delete;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_delete, delete)
    END_SERIAL_MEMEBER
};

class ConsistentSnapshotGroupInfo {
public:
    std::string m_groupName;                // 创建的卷组名称
    std::string m_groupId;                  // 卷组ID
    std::string m_groupTypeId;              // group_type ID
    std::string m_groupSnapshotName;        // 卷组快照名称
    std::string m_groupTypeName;            // group_type 名称
    std::string m_groupStatus;              // group status;
    std::vector<std::string> m_volumeType;  // volume_type
    bool m_isGroupTypeCreated {false};      // group_type已经创建标志
    bool m_isGroupCreated {false};          // 卷组创建标志
    bool m_isVolumeAddGroup {false};         // 卷添加到卷组中的标志
    bool m_isGroupSnapshotCreated {false};   // 一致性快照组创建标志
    std::string m_volumelist;               // 卷组中添加的卷
    std::string m_groupSnapshotId;          // 卷组快照ID
    std::string m_groupsnapshotStatus;      // 卷组快照状态
    std::string m_uuid;                     // 卷组的uuid
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupName, group_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupId, group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupTypeId, grouptype_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshotName, group_snapshot_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupStatus, group_status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isGroupTypeCreated, grouptype_is_created)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isGroupCreated, group_is_created)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isVolumeAddGroup, volume_is_add_group)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isGroupSnapshotCreated, groupsnapshot_is_created)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumelist, volume_list)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshotId, groupsnapshotid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupsnapshotStatus, groupsnapshotstatus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, uuid)
    END_SERIAL_MEMEBER
};
 
class ConsistentSnapshotGroupInfoList {
public:
    std::vector<ConsistentSnapshotGroupInfo> m_consistentGroupInfos;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistentGroupInfos, consistentgroups)
    END_SERIAL_MEMEBER
};

OPENSTACK_PLUGIN_NAMESPACE_END

#endif // OPENSTACK_GROUP_DETAILS_H