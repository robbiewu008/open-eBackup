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
#ifndef OPENSTACK_SNAPSHOT_DETAILS_H
#define OPENSTACK_SNAPSHOT_DETAILS_H

#include <common/JsonHelper.h>
#include "protect_engines/openstack/common/OpenStackMacros.h"

OPENSTACK_PLUGIN_NAMESPACE_BEGIN

const uint32_t SNAPSHOT_RETRY_PERIOD = 5;
const uint32_t SNAPSHOT_RETRY_TIMES = 120;
const uint32_t SNAPSHOT_DELETE_RETRY_TIMES = 24;
const std::string SNAPSHOT_STATUS_CREATING = "creating";
const std::string SNAPSHOT_STATUS_AVALIABLE = "available";
const std::string SNAPSHOT_STATUS_ERROR = "error";
const std::string SNAPSHOT_STATUS_DELETING = "deleting";
const std::string SNAPSHOT_STATUS_ERROR_DELETING = "error_deleting";

class SnapshotMetadata {
public:
    std::string m_enableActive;
    std::string m_snapshotWwn;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enableActive, __system__enableActive)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotWwn, snapshot_wwn)
    END_SERIAL_MEMEBER
};

class SnapshotDetails {
public:
    std::string m_id;
    std::string m_name;
    int32_t m_size;             // GiB
    std::string m_status;
    std::string m_volumeId;
    std::string m_createdAt;
    std::string m_updatedAt;
    std::string m_storageProviderAuth;    // 存储底层快照ID eg: {\"lun_id\": \"3197\", \"sn\": \"2102351NPT10J3000001\"}
    std::string m_description;
    std::string m_progress;
    std::string m_projectId;
    std::string m_groupSnapshotId;
    bool m_consumesQuota;
    SnapshotMetadata m_metadata;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeId, volume_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createdAt, created_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_updatedAt, updated_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageProviderAuth, provider_auth);
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_progress, os-extended-snapshot-attributes:progress)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, os-extended-snapshot-attributes:project_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshotId, m_groupSnapshotId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consumesQuota, consumes_quota)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metadata)
    END_SERIAL_MEMEBER
};

class SnapshotDetailsMsg {
public:
    SnapshotDetails m_snapshotDetails;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotDetails, snapshot)
    END_SERIAL_MEMEBER
};

class SnapshotDetialList {
public:
    std::vector<SnapshotDetails> m_snapshots;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshots, snapshots)
    END_SERIAL_MEMEBER
};

class CreateSnapshotRequestBody {
public:
    std::string m_name;
    std::string m_description;
    std::string m_volumeId;
    std::string m_force;
    SnapshotMetadata m_snapshotMetadata;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeId, volume_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_force, force)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotMetadata, metadata)
    END_SERIAL_MEMEBER
};

class CreateSnapshotRequestBodyMsg {
public:
    CreateSnapshotRequestBody m_snapshot;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshot, snapshot)
    END_SERIAL_MEMEBER
};

class GroupSnapshot {
public:
    std::string m_id;
    std::string m_groupId;
    std::string m_status;
    std::string m_createAt;
    std::string m_name;
    std::string m_description;
    std::string m_groupTypeId;
    std::string m_projectId;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupId, group_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createAt, created_at)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_description, description)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupTypeId, group_type_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_projectId, project_id)
    END_SERIAL_MEMEBER
};
 
class GroupSnapShotDetail {
public:
    GroupSnapshot m_groupSnapshot;
 
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_groupSnapshot, group_snapshot)
    END_SERIAL_MEMEBER
};
 

OPENSTACK_PLUGIN_NAMESPACE_END

#endif // SNAPSHOT_DETAILS_H