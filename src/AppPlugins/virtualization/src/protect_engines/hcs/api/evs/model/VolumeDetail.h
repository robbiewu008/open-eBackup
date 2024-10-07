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
#ifndef VOLUME_DETAILS_H
#define VOLUME_DETAILS_H

#include <common/JsonHelper.h>
#include "protect_engines/hcs/common/HcsMacros.h"

namespace HcsPlugin {
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

class MetaData {
public:
    std::string m_takeOverLunWWN;
    std::string m_tenancy;
    std::string m_lunWWN;
    std::string m_storageType;
    std::string m_sysIsServerVol;
    std::string m_readonly;
    std::string m_attachedMode;
    std::string m_hwPassthrough; // 默认为VBD类型 返回值中可以没有该字段
    std::string m_encrypted;
    std::string m_cmkId;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_takeOverLunWWN, take_over_lun_wwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_tenancy, tenancy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunWWN, lun_wwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageType, StorageType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sysIsServerVol, __sys_is_server_vol__)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_readonly, readonly)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachedMode, attached_mode)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hwPassthrough, hw:passthrough)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encrypted, __system__encrypted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cmkId, __system__cmkid)
    END_SERIAL_MEMEBER
};

class EncryptionInfo {
public:
    std::string m_cipher;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cipher, cipher)
    END_SERIAL_MEMEBER
};

class Attachments {
public:
    std::string m_serverId;
    std::string m_attachId;
    std::string m_hostName;
    std::string m_volumeId;
    std::string m_device;
    std::string m_id;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_serverId, server_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachId, attachment_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostName, host_name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeId, volume_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_device, device)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    END_SERIAL_MEMEBER
};

class DriverData {
public:
    std::string m_lunId;
    std::string m_sn;
    /* fusionstorage */
    std::string m_ip;   // fusionstorage 管理IP
    int32_t m_pool; // 卷所在存储池ID
    std::string m_volName; // 底层存储卷的名称
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_lunId, lun_id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sn, sn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_pool, pool)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volName, vol_name)
    END_SERIAL_MEMEBER
};

class HostVolume {
public:
    std::string m_id;           // 卷的UUID
    std::string m_displayName;  // 卷名称
    std::string m_status;
    std::vector<Attachments> m_attachPoints; // 挂载点
    std::string m_avaliableZone;
    std::string m_volumeType;         // 卷类型
    uint64_t m_size;  // 卷大小
    MetaData m_metaData;
    std::string m_bootable;
    bool m_shareable;
    VolumeImageMetadata m_volImageMetadata;
    EncryptionInfo m_encryptionInfo;
    std::string m_osVolRepDriverData;
    DriverData m_drvData;
    std::string m_metadata;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_id, id)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_displayName, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_status, status)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachPoints, attachments)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_avaliableZone, availability_zone)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, size)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metaData, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootable, bootable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_shareable, shareable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volImageMetadata, volume_image_metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_osVolRepDriverData, os-volume-replication:driver_data)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_encryptionInfo, encryption_info)
    END_SERIAL_MEMEBER
};

class HSCVolDetail {
public:
    HostVolume m_hostVolume;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_hostVolume, volume)
    END_SERIAL_MEMEBER
};

class VolumeDetailList {
public:
    std::vector<HostVolume> m_volumeInfo;     // 磁盘详细信息

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeInfo, volumes)
    END_SERIAL_MEMEBER
};
}

#endif