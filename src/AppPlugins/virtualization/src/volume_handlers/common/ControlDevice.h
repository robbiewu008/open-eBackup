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
#ifndef CONTROL_DEVICE_H
#define CONTROL_DEVICE_H
#include <vector>
#include <memory>
#include <unordered_map>
#include <algorithm>
#include <boost/uuid/uuid_generators.hpp>
#include "common/CleanMemPwd.h"
#include "common/Macros.h"
#include "common/Constants.h"

VIRT_PLUGIN_NAMESPACE_BEGIN

const int32_t NUM_32K = 32768;
const int64_t MIN_CURL_TIME_OUT = 90;
const int32_t ERRORCODE_NOTEXIST = 404;
const int32_t MAXNAMELENGTH = 20;
const int32_t MAX_LENGTH = 31;
const int32_t KB = 1024;
const int32_t TWO = 2;
const int64_t MAX_BITMAP_LENGTH_ONCE = 256;
const int64_t ONE_MB = 1024 * 1024;
const int32_t SINGLE_WAY = 1;
const int32_t DO_NOT_USE = 0;
const uint64_t CAPACITY_COEFFICIENT = 512;
const int32_t LOGIC_PORT_IS_SERVICE = 2;
const int32_t LOGIC_PORT_IS_MANAGE_AND_SERVICE = 3;
const int32_t FUSION_STORAGE_SNAPSHOP_STATUS_NORMAL = 0;
const int32_t FUSION_STORAGE_SNAPSHOP_STATUS_REVERTING = 16;
const int32_t DORADO_ERROR_CODE_OK = 0; // no any error happen when calling Dorado rest interface.
const std::string CONTROLDEVICEMODULE = "CONTROLDEVICE";
const std::string COMPRESS = "Compress";
const std::string DEDUP = "Dedup";
const std::string RESOURCE_ID = "ResourceID";
const std::string TARGET_RESOURCE_ID = "TargetResourceID";
const std::string HOST = "Host";
const std::string ISCSICONNECTOR = "IscsiConnector";
const std::string IP = "IP";
const std::string STORAGE = "BackupStorage";
const std::string STORAGE_TYPE = "Type";
const std::string STORAGE_IP = "IP";
const std::string STORAGE_PORT = "Port";
const std::string STORAGE_POOL = "PoolId";
const std::string STORAGE_USERNAME = "Username";
const std::string STORAGE_PASSWORD = "Password";
const std::string BANDWIDTHMIN = "BandwidthMin";
const std::string BANDWIDTHMAX = "BandwidthMax";
const std::string BANDWIDTHBURST = "BandwidthBurst";
const std::string IOPSMIN = "IOPSMin";
const std::string IOPSMAX = "IOPSMax";
const std::string IOPSBURST = "IOPSBurst";
const std::string LATENCY = "Latency";
const std::string BURST_TIME = "BurstTime";
const std::string NAME = "NAME";
const std::string ID = "ID";
const std::string OPERATIONSYSTEM = "OPERATIONSYSTEM";
const std::string ALLOCTYPE = "ALLOCTYPE";
const std::string CAPACITY = "CAPACITY";
const std::string PARENTID = "PARENTID";
const std::string PARENTTYPE = "PARENTTYPE";
const std::string USECHAP = "USECHAP";
const std::string CHAPNAME = "CHAPNAME";
const std::string CHAPPASSWORD = "CHAPPASSWORD";
const std::string DISCOVERYVERMODE = "DISCOVERYVERMODE";
const std::string NORMALVERMODE = "NORMALVERMODE";
const std::string SUBTYPE = "SUBTYPE";
const std::string WRITEPOLICY = "WRITEPOLICY";
const std::string WORKLOADTYPEID = "WORKLOADTYPEID";
const std::string ENABLECOMPRESSION = "ENABLECOMPRESSION";
const std::string ENABLESMARTDEDUP = "ENABLESMARTDEDUP";
const std::string DESCRIPTION = "DESCRIPTION";
const std::string SUPPORTPROTOCOL_NFS = "1";
const std::string SUPPORTPROTOCOL_CIFS = "2";
const std::string SUPPORTPROTOCOL_NFS_CIFS = "3";
const std::string SUPPORTPROTOCOL_ISCSI = "4";
const std::string ETHERNET_PORT = "1";
const std::string PORT_ROLE_SERVICE = "2";
const std::string PORT_ROLE_MANAGE_SERVICE = "3";
const std::string PORT_ROLE_REPLICATION = "4";
const std::string RUNNINGSTATUS_LINKUP = "10";
const std::string illegalChar = ".-:/@?+()<>$!~#%^*&|,;";

enum STORAGE_ENUM { FUSIONSTORAGE = 0, DORADO = 1, OCEANSTOR = 2, NETAPP = 3, OTHERS = 4, DEFAULT = -1 };
enum PROTOCOL {SAN = 1, NAS = 2, NFS = 3, CIFS = 4, PROTOCOL_DEFAULT = -1};
enum INITIATOR_TYPE { FC = 0, ISCSI = 1 };
enum IP_TYPE { IP_V6 = 1, IP_V4 = 0 };
enum SecurityStyle { NATIVE = 1, NTFS = 2, UNIX = 3 };

struct DiffBitmap {
    DiffBitmap() : m_offset(0), m_size(0), m_chunkSize(0)
    {}
    DiffBitmap(uint64_t uOffset, uint64_t uSize, uint64_t uChunkSize)
    {
        m_offset = uOffset;
        m_size = uSize;
        m_chunkSize = uChunkSize;
    }
    uint64_t m_offset;     // 单位为字节
    uint64_t m_size;       // 单位为字节
    uint64_t m_chunkSize;  // 单位为字节
    std::string m_bitmap;
};

struct StorageSysInfo {
    StorageSysInfo() {}
    std::string m_id;     // 系统ID
    std::string m_wwn;    // 系统wwn
    std::string m_productMode; // 产品模式
};

struct UserRoleLevel {
    UserRoleLevel() {}
    std::string m_roleID;
    std::string m_level;
};

struct ControlDeviceInfo {
    ControlDeviceInfo() : m_poolId(0), m_enableCert(false) {}
    ~ControlDeviceInfo()
    {
        Module::CleanMemoryPwd(m_password);
        Module::CleanMemoryPwd(m_cert);
        Module::CleanMemoryPwd(m_revocationList);
    }
    std::string m_ip;       // login device ip
    std::string m_port;      // login device port
    std::string m_userName;  // login device user name
    std::string m_password;  // login device password
    std::string m_cert;
    std::string m_vstoreId;
    std::string m_storageType;
    std::string m_revocationList;
    std::vector<std::string> m_ipList;
    int32_t m_poolId;
    bool m_enableCert;
};

enum NativeObjectType {
    NATIVE_OBJ_TYPE_LUN,
    NATIVE_OBJ_TYPE_LUNCG,
    NATIVE_OBJ_TYPE_CDP,
    NATIVE_OBJ_TYPE_CDPCG,
    NATIVE_OBJ_TYPE_SNAPSHOT,
    NATIVE_OBJ_TYPE_BUTT
};

struct NativeObjectInfo {
    NativeObjectInfo() : m_id(""), m_type(NATIVE_OBJ_TYPE_BUTT), m_isFullBackup(false)
    {}
    std::string m_id;
    std::string m_name;
    std::string m_wwn;
    NativeObjectType m_type;
    bool m_isFullBackup; // True when FULL_BACKUP
};

struct DirtyRangesRequest {
    DirtyRangesRequest()
        : m_startOffset(0),
          m_endOffset(0),
          m_changeCapacity(0),
          m_preChangeCapacity(0),
          m_overlapSize(0),
          m_standaloneOffset(0),
          m_standaloneSize(0),
          m_lunCapacity(0)
    {}
    NativeObjectInfo m_preChangeObj;
    NativeObjectInfo m_curChangeObj;
    uint64_t m_startOffset;
    uint64_t m_endOffset;
    uint64_t m_changeCapacity;
    uint64_t m_preChangeCapacity;
    uint64_t m_overlapSize;
    uint64_t m_standaloneOffset;
    uint64_t m_standaloneSize;
    uint64_t m_lunCapacity;
    std::string m_preChange;
    std::string m_change;
};

VIRT_PLUGIN_NAMESPACE_END

#endif  // CONTROL_DEVICE_H
