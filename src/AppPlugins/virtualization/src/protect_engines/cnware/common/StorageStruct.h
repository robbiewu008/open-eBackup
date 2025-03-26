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
#ifndef CNWARE_STORAGE_STRUCTS_H
#define CNWARE_STORAGE_STRUCTS_H
#include <vector>
#include <string>
#include <common/JsonHelper.h>
#include <common/Structs.h>
#include "protect_engines/cnware/common/Structs.h"

namespace CNwarePlugin {

struct HostRelationRspList {
std::string m_id;
std::string m_hostId;
std::string m_clusterId;
std::string m_poolId;
bool m_active;

BEGIN_SERIAL_MEMEBER;
SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostId, hostId)
SERIAL_MEMBER_TO_SPECIFIED_NAME(m_clusterId, clusterId)
SERIAL_MEMBER_TO_SPECIFIED_NAME(m_poolId, poolId)
SERIAL_MEMBER_TO_SPECIFIED_NAME(m_active, active)
END_SERIAL_MEMEBER;
};

struct StoragePool {
    uint64_t m_capacity = 0;
    uint64_t m_available = 0;
    std::string m_id;
    std::string m_name;
    std::string m_storageSanId;
    std::string m_storageSanName;
    int32_t m_storageType = 0;
    std::string m_storageResourceName;
    std::string m_storageResourceId;
    int32_t m_useType = 0;
    int32_t m_type = 0;
    int32_t m_status = 0;
    std::string m_remark;
    std::vector<HostRelationRspList> m_spHostRelationRspList;
    uint64_t m_allocation = 0;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_available, available)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageSanId, storageSanId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageSanName, storageSanName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageType, storageType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageResourceName, storageResourceName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageResourceId, storageResourceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_useType, useType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remark, remark)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_spHostRelationRspList, spHostRelationRspList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_allocation, allocation)
    END_SERIAL_MEMEBER;
};

struct StoragePoolInfo {
    std::vector<StoragePool> m_data;
    int32_t m_total = 0;
    int32_t m_start = 0;
    int32_t m_size = 0;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_total, total)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_start, start)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    END_SERIAL_MEMEBER;
};

struct StoragePoolExInfo {
    StoragePool m_data;

    BEGIN_SERIAL_MEMEBER;
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    END_SERIAL_MEMEBER;
};

struct AddNfsReq {
    std::string m_name;
    std::vector<std::string> m_hostIdList;
    std::string m_ip;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostIdList, hostIdList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    END_SERIAL_MEMEBER
};

struct AddNfsStorageReq {
    std::string m_storagePoolName;
    std::string m_name;
    std::vector<std::string> m_hostIdList;
    std::string m_resourceId;
    std::string m_useType = "1";

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storagePoolName, storagePoolName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostIdList, hostIdList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceId, resourceId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_useType, useType)
    END_SERIAL_MEMEBER
};

struct StoreResourceReq {
    uint32_t m_size = 100;
    uint32_t m_start = 1;
    std::string m_nameType = "1";
    std::string m_name;
    bool m_needStoragePoolName {true};
    std::string m_storeId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_start, start)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_nameType, nameType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, nameLike)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_needStoragePoolName, needStoragePoolName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storeId, storeId)
    END_SERIAL_MEMEBER
};

struct StoreResource {
    std::string m_id;
    std::string m_resourceName;
    uint32_t m_status;
    uint32_t m_storeType;
    uint32_t m_available;
    uint64_t m_capacity;
    uint64_t m_usedCapacity;
    std::string m_storageSanId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_resourceName, resourceName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storeType, storeType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_available, available)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_usedCapacity, usedCapacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageSanId, storageSanId)
    END_SERIAL_MEMEBER
};

struct StoreResourceRes {
    std::vector<StoreResource> m_data;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    END_SERIAL_MEMEBER
};

struct NfsReq {
    uint32_t m_size = 50;
    uint32_t m_start = 1;
    std::string m_name;
    uint32_t m_storeType = 3;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_start, start)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, nameLike)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storeType, storeType)
    END_SERIAL_MEMEBER
};

struct NfsStore {
    std::string m_id;
    uint32_t m_storeType = 3;
    std::string m_name;
    std::string m_ip;
    std::string m_remark;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storeType, storeType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, storageName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remark, remark)
    END_SERIAL_MEMEBER
};

struct NfsStoreRes {
    std::vector<NfsStore> m_data;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    END_SERIAL_MEMEBER
};

struct StorageVolume {
    std::string m_id;
    std::string m_name;
    uint32_t m_type {0};
    uint32_t m_joinType {0};
    std::string m_path;
    bool m_encrypt {false};
    uint32_t m_status {0};
    uint64_t m_capacity {0};
    bool m_shareable {false};
    uint32_t m_preallocation {0};
    std::string m_remark;
    std::vector<CNwareDiskUserList> m_userList;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_joinType, joinType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_path, path)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encrypt, encrypt)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_capacity, capacity)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preallocation, preallocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_remark, remark)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userList, userList)
    END_SERIAL_MEMEBER
};

struct StorageVolumeRes {
    std::vector<StorageVolume> m_data;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data)
    END_SERIAL_MEMEBER
};

struct MigrationVol {
    std::string m_bus;
    std::string m_dev;
    std::string m_preallocation;
    std::string m_destStoragePoolId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bus, bus)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dev, dev)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preallocation, preallocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_destStoragePoolId, destStoragePoolId)
    END_SERIAL_MEMEBER
};

struct MigrationDownTime {
    int32_t m_max { 10000 };
    int32_t m_min { 300 };

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_max, max)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_min, min)
    END_SERIAL_MEMEBER
};

struct MigrationVolReq {
    std::vector<MigrationVol> m_migrateVols;
    std::string m_destHostId;
    bool m_keepNuma { false };
    MigrationDownTime m_downtime;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_migrateVols, migrateVols)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_destHostId, destHostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_keepNuma, keepNuma)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_downtime, downtime)
    END_SERIAL_MEMEBER
};

struct V2MigrationVolReq {
    std::vector<MigrationVol> m_migrateVols;
    std::string m_destHostId;
    bool m_keepNuma { false };
    MigrationDownTime m_downtime;
    bool m_diskSynchronousWrites { false };

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_migrateVols, migrateVols)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_destHostId, destHostId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_keepNuma, keepNuma)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_downtime, downtime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_diskSynchronousWrites, diskSynchronousWrites)
    END_SERIAL_MEMEBER
};

struct DeleteStoragePoolReq {
    bool m_forceDel = false;

    BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_forceDel, forceDel)
    END_SERIAL_MEMEBER
};

struct DeleteStoragePoolRespTaskId {
    std::string mTaskId;

    BEGIN_SERIAL_MEMEBER;
        SERIAL_MEMBER_TO_SPECIFIED_NAME(mTaskId, taskId);
    END_SERIAL_MEMEBER;
};

struct DeleteStoragePoolResponseBody {
    DeleteStoragePoolRespTaskId m_data;

    BEGIN_SERIAL_MEMEBER;
        SERIAL_MEMBER_TO_SPECIFIED_NAME(m_data, data);
    END_SERIAL_MEMEBER;
};
}
#endif