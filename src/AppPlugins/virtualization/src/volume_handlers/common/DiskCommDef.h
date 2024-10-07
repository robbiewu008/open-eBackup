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
#ifndef DISK_COMM_DEF_H
#define DISK_COMM_DEF_H
#include <set>
#include <list>
#include <utility>
#include <memory>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <common/CleanMemPwd.h>
#include "volume_handlers/common/StorageDataType.h"
#include "common/Macros.h"

using namespace std;
namespace VirtPlugin {
#ifndef HCP_FULL_SNAPSHOT
#define HCP_FULL_SNAPSHOT "HCP_FULL_SNAPSHOT"
#endif


// disk message IDs
#define DISK_MESSAGE_ID_PREFIX         std::string("DiskDescriptor.Message.")
#define DISK_MESSAGE_UPDATE_CACHE_DATA DISK_MESSAGE_ID_PREFIX + "Update.CacehData"
#define DISK_MESSAGE_SEND_EVENT        DISK_MESSAGE_ID_PREFIX + "SendEvent"
#define DISK_MESSAGE_UPDATE_V3_MAPINFO DISK_MESSAGE_ID_PREFIX + "Update.V3MapInfo"

// data keys
#define DISK_EVENT_KEY_CODE       "Code"
#define DISK_EVENT_KEY_TYPE       "Type"
#define DISK_EVENT_KEY_PARAMETERS "Parameters"

// cachedata keys
#define CACHE_DATA_KEY_BUCKETS_INFO  "BucketsInfo"
#define CACHE_DATA_KEY_VOLUME_INFO   "LunVolumeParams"
#define CACHE_DATA_KEY_CONCUR_BACKUP "ConcurBackup"
// if -1 use the default volume offset size
#define INVALID_OFFSET_SIZE -1

#define DEVICE_SET_OPERATOR "DeviceSetOperator"
#define HTTPS_PORT          8088

const std::string ISCSIINITIATORNAME = "ISCSIINITIATORNAME";
const std::string FCINITIATORNAME = "FCINITIATORNAME";
const std::string LUNGROUPID = "LUNGROUPID";
const std::string LUNGROUPNAME = "LUNGROUPNAME";
const std::string HOSTID = "HOSTID";
const std::string HOSTNAME = "HOSTNAME";
const std::string HOSTGROUPID = "HOSTGROUPID";
const std::string HOSTGROUPNAME = "HOSTGROUPNAME";
const std::string MAPVIEWID = "MAPVIEWID";
const std::string MAPVIEWNAME = "MAPVIEWNAME";
const std::string LOCAL_SESSION_USERNAME = "LOCALSESSIONUSERNAME";
const std::string LOCAL_SESSION_PWD = "LOCALSESSIONPWD";
const std::string IQN = "IQN";
const std::string PROTOCOLTYPE = "PROTOCOLTYPE";
const std::string DONOTDELETEHOST = "DONOTDELETEHOST";
const std::string STR_FLAG_DELETE_HOST = "DELETE";
const std::string STR_FLAG_NOT_DELETE_HOST = "NOT";
const std::string STR_PROTO_TYPE_ISCSI = "iscsi";
const std::string STR_PROTO_TYPE_FC = "fc";

typedef struct DeviceMessage {
    std::string m_hostGroupID;
    std::string m_hostGroupName;
    std::string m_hostID;
    std::string m_hostName;
    std::string m_iqn;
    std::string m_iscsiInitiatorName;
    std::string m_fcInitiatorName;
    std::string m_lunGroupID;
    std::string m_lunGroupName;
    std::string m_mapViewID;
    std::string m_mapViewName;
    std::string m_protocolType;
    std::string m_doNotDeleteHost;
} DeviceMessage;

typedef struct DevChapInfo {
    bool m_useChap;
    std::string m_chapUserName;
    std::string m_chapPassword;
} DevChapInfo;

struct IscsiInitiatorInfo {
    IscsiInitiatorInfo() : m_bCreateHost(false)
    {}
    std::string m_deviceID;
    std::string m_hostID;
    std::string m_hostName;
    std::set<std::string> m_existHostIds;
    bool m_bCreateHost;
};

struct LunBitmapInfo {
    LunBitmapInfo() : m_lunID(""), m_parentLunID(""), m_offset(0), m_size(0), m_chunkSize(0), m_bitmap("")
    {}
    std::string m_lunID;
    std::string m_parentLunID;
    uint64_t m_offset;
    uint64_t m_size;
    uint64_t m_chunkSize;
    std::string m_bitmap;
};

struct LunGroupInfo {
    LunGroupInfo() : m_groupID(""), m_groupName("")
    {}
    std::string m_groupID;
    std::string m_groupName;
};

struct DirtyRangesCalculateInfo {
    DirtyRangesCalculateInfo() : currentOffset(0), endOffset(0)
    {}
    std::string preChangeID; // snap shot id
    std::string changeID; // target lun id
    uint64_t currentOffset;
    uint64_t endOffset;
};
/***************************************************
 * @class LunMO
 * @brief LUN数据类型
 * @details 表述LUN内容的参数
***************************************************/
class LunMO {
public:
    LunMO() : m_id(""), m_name(""), m_parentType(MO_UNKNOWN), m_parentID(""), m_parentName(""),
        m_healthStatus(HSE_UNKNOWN), m_runningStatus(RUN_UNKNOWN), m_description(""), m_allocType(SAN_UNKNOWN),
        m_capacity(ULLONG_MAX), m_initialAllocCapacity(ULLONG_MAX), m_allocCapacity(0), m_thinCapacityUsage(0),
        m_exposedToInitiator(false), m_wwn(""), m_owningController(""), m_workingController(""), m_isAddToLunGroup(-1)
    {}
    LunMO(const LunMO& mo)
    {
        m_id                        = mo.m_id;
        m_name                      = mo.m_name;
        m_parentType                = mo.m_parentType;
        m_parentID                  = mo.m_parentID;
        m_parentName                = mo.m_parentName;
        m_healthStatus              = mo.m_healthStatus;
        m_runningStatus             = mo.m_runningStatus;
        m_description               = mo.m_description;
        m_allocType                 = mo.m_allocType;
        m_capacity                  = mo.m_capacity;
        m_initialAllocCapacity      = mo.m_initialAllocCapacity;
        m_allocCapacity             = mo.m_allocCapacity;
        m_thinCapacityUsage         = mo.m_thinCapacityUsage;
        m_exposedToInitiator        = mo.m_exposedToInitiator;
        m_wwn                       = mo.m_wwn;
        m_owningController          = mo.m_owningController;
        m_workingController         = mo.m_workingController;
        m_isAddToLunGroup           = mo.m_isAddToLunGroup;
    }
    virtual ~LunMO() {}
public:
    std::string m_id;
    std::string m_name;
    MO_TYPE m_parentType;
    std::string m_parentID;
    std::string m_parentName;
    HEALTH_STATUS_E m_healthStatus;
    RUNNING_STATUS_E m_runningStatus;
    std::string m_description;
    LUN_ALLOC_TYPE_E m_allocType;
    uint64_t m_capacity;                // 配置容量，单位 sectors
    uint64_t m_initialAllocCapacity;    // 实际占用容量，单位 byte
    uint64_t m_allocCapacity;           // 实际占用容量，单位 sectors
    uint64_t m_thinCapacityUsage;
    bool m_exposedToInitiator;
    std::string m_wwn;
    std::string m_owningController;
    std::string m_workingController;
    bool m_isAddToLunGroup;
};

class SnapshotMO {
public:
    SnapshotMO() : m_id(""), m_name(""), m_parentType(MO_UNKNOWN), m_parentID(""), m_parentName(""),
        m_healthStatus(HSE_UNKNOWN), m_runningStatus(RUN_UNKNOWN), m_description(""), m_userCapacity(ULLONG_MAX),
        m_consumedCapacity(ULLONG_MAX), m_wwn("")
    {}

    SnapshotMO(const SnapshotMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_parentType = mo.m_parentType;
        m_parentID = mo.m_parentID;
        m_parentName = mo.m_parentName;
        m_healthStatus = mo.m_healthStatus;
        m_runningStatus = mo.m_runningStatus;
        m_description = mo.m_description;
        m_userCapacity = mo.m_userCapacity;
        m_consumedCapacity = mo.m_consumedCapacity;
        m_wwn = mo.m_wwn;
    }
    virtual ~SnapshotMO() {}

public:
    std::string m_id;                   // 快照ID
    std::string m_name;                 // 快照名称
    MO_TYPE m_parentType;               // 父类对象类型
    std::string m_parentID;             // 父类对象ID
    std::string m_parentName;           // 父类对象名称
    HEALTH_STATUS_E m_healthStatus;     // 健康状况
    RUNNING_STATUS_E m_runningStatus;   // 运行状态
    std::string m_description;          // 描述
    uint64_t m_userCapacity;            // 快照的用户容量
    uint64_t m_consumedCapacity;        // 快照实际消耗的容量
    std::string m_wwn;                  // WWN
};

class FCInitiatorMO {
public:
    FCInitiatorMO() : m_id(""), m_name(""), m_parentID(""), m_parentName(""), m_healthStatus(HSE_UNKNOWN),
        m_runningStatus(RUN_UNKNOWN), m_isFree(false)
    {}
    FCInitiatorMO(const FCInitiatorMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_parentID = mo.m_parentID;
        m_parentName = mo.m_parentName;
        m_healthStatus = mo.m_healthStatus;
        m_runningStatus = mo.m_runningStatus;
        m_isFree = mo.m_isFree;
    }
    virtual ~FCInitiatorMO() {
        Module::CleanMemoryPwd(m_id);
        Module::CleanMemoryPwd(m_name);
    }

public:
    std::string m_id;                   // ISCSI启动器ID
    std::string m_name;                 // ISCSI启动器名称
    std::string m_parentID;             // 父对象ID
    std::string m_parentName;           // 父类对象名称
    HEALTH_STATUS_E m_healthStatus;     // 健康状况
    RUNNING_STATUS_E m_runningStatus;   // 运行状态
    bool m_isFree;                      // 是否空闲 true：空闲  false：不空闲
};

/***************************************************
 * @class ISCSIInitiatorMO
 * @brief ISCSI启动器数据类型
 * @details 表述ISCSI内容的参数
***************************************************/
class IscsiInitiatorMO {
public:
    IscsiInitiatorMO() : m_id(""), m_name(""), m_parentID(""), m_parentName(""), m_parentType(MO_UNKNOWN),
        m_healthStatus(HSE_UNKNOWN), m_runningStatus(RUN_UNKNOWN), m_isFree(false), m_useChap(false),
        m_normalVerMode("0"), m_discoveryVerMode("0"), m_chapName(""), m_chapPassword(""), m_chapOldPassword(""),
        m_chapNameTarget(""), m_chapPasswordTarget(""), m_chapOldPasswordTarget("")
    {}
    IscsiInitiatorMO(const IscsiInitiatorMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_parentID = mo.m_parentID;
        m_parentName = mo.m_parentName;
        m_parentType = mo.m_parentType;
        m_healthStatus = mo.m_healthStatus;
        m_runningStatus = mo.m_runningStatus;
        m_isFree = mo.m_isFree;
        m_useChap = mo.m_useChap;
        m_normalVerMode = mo.m_normalVerMode;
        m_discoveryVerMode = mo.m_discoveryVerMode;
        m_chapName = mo.m_chapName;
        m_chapPassword = mo.m_chapPassword;
        m_chapOldPassword = mo.m_chapOldPassword;
        m_chapNameTarget = mo.m_chapNameTarget;
        m_chapPasswordTarget = mo.m_chapPasswordTarget;
        m_chapOldPasswordTarget = mo.m_chapOldPasswordTarget;
    }
    ~IscsiInitiatorMO() {
        Module::CleanMemoryPwd(m_id);
        Module::CleanMemoryPwd(m_name);
        Module::CleanMemoryPwd(m_chapPassword);
        Module::CleanMemoryPwd(m_chapOldPassword);
        Module::CleanMemoryPwd(m_chapPasswordTarget);
        Module::CleanMemoryPwd(m_chapOldPasswordTarget);
    }

public:
    std::string m_id;                       // ISCSI启动器ID
    std::string m_name;                     // ISCSI启动器名称
    std::string m_parentID;                 // 父对象ID
    std::string m_parentName;               // 父类对象名称
    MO_TYPE m_parentType;                   // ISCSI启动器的父类型
    HEALTH_STATUS_E m_healthStatus;         // 健康状况
    RUNNING_STATUS_E m_runningStatus;       // 运行状态
    bool m_isFree;                          // 是否空闲 true：空闲  false：不空闲
    bool m_useChap;                         // 是否启用CHAP true：启用  false：不启用
    std::string m_normalVerMode;            // normal认证方式       0:不认证 1:单向认证 2:双向认证
    std::string m_discoveryVerMode;         // discovery认证方式    0:不认证 1:单向认证 2:双向认证
    std::string m_chapName;                 // CHAP名称
    std::string m_chapPassword;             // CHAP密码
    std::string m_chapOldPassword;          // CHAP旧密码
    std::string m_chapNameTarget;           // ChapTarget名称
    std::string m_chapPasswordTarget;       // ChapTarget密码
    std::string m_chapOldPasswordTarget;    // ChapTarget旧密码
};

/***************************************************
 * @class HostMO
 * @brief 主机数据类型
 * @details 表述主机内容的参数
***************************************************/
class HostMO {
public:
    HostMO() : m_id(""), m_name(""), m_parentType(MO_UNKNOWN), m_parentId(""), m_location(""),
        m_healthStatus(HSE_UNKNOWN), m_runningStatus(RUN_UNKNOWN), m_description(""), m_operationSystem(OS_UNKNOWN),
        m_ip(""), m_isAddToHostGroup(-1)
    {}
    HostMO(const HostMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_parentType = mo.m_parentType;
        m_parentId = mo.m_parentId;
        m_location = mo.m_location;
        m_healthStatus = mo.m_healthStatus;
        m_runningStatus = mo.m_runningStatus;
        m_description = mo.m_description;
        m_operationSystem = mo.m_operationSystem;
        m_ip = mo.m_ip;
        m_isAddToHostGroup = mo.m_isAddToHostGroup;
    }
    ~HostMO() {}

public:
    std::string m_id;                   // 主机的ID
    std::string m_name;                 // 主机的名称
    MO_TYPE m_parentType;               // 主机的parent类
    std::string m_parentId;             // 父对象ID
    std::string m_location;             // 所处地理位置
    HEALTH_STATUS_E m_healthStatus;     // 健康状态
    RUNNING_STATUS_E m_runningStatus;   // 运行状态
    std::string m_description;          // 描述信息
    OS_TYPE_E m_operationSystem;        // 操作系统
    std::string m_ip;                   // IP地址
    bool m_isAddToHostGroup;            // 是否加至主机组   true:是 false:否
};

/***************************************************
 * @class LunGroupMO
 * @brief LUN组数据类型
 * @details 表述LUN组内容的参数
***************************************************/
class LunGroupMO {
public:
    LunGroupMO() : m_id(""), m_name(""),
        m_description(""),
        m_isAddToMappingView(-1),
        m_appType(APP_UNKNOWN),
        m_groupType((uint32_t)GRP_LUNGRP)
    {}

    LunGroupMO(const LunGroupMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_description = mo.m_description;
        m_isAddToMappingView = mo.m_isAddToMappingView;
        m_appType = mo.m_appType;
        m_groupType = mo.m_groupType;
    }
    ~LunGroupMO() {}

public:
    std::string m_id;               // LUN组ID
    std::string m_name;             // LUN组名称
    std::string m_description;      // 描述信息
    bool m_isAddToMappingView;      // 判断是否已添加映射视图
    APP_TYPE_E m_appType;           // 应用类型
    uint32_t m_groupType;           // 组类型
};

/***************************************************
 * @class HostGroupMO
 * @brief 主机组数据类型
 * @details 表述主机组内容的参数
***************************************************/
class HostGroupMO {
public:
    HostGroupMO() : m_id(""), m_name(""), m_description(""), m_isAddToMappingView(-1)
    {}
    HostGroupMO(const HostGroupMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_description = mo.m_description;
        m_isAddToMappingView = mo.m_isAddToMappingView;
    }
    ~HostGroupMO() {}

public:
    std::string m_id;               // 主机组ID
    std::string m_name;             // 主机组名称
    std::string m_description;      // 描述信息
    bool m_isAddToMappingView;      // 是否已添加映射视图   true：已添加    false：未添加
};

/***************************************************
 * @class HostGroupMO
 * @brief 主机组数据类型
 * @details 表述主机组内容的参数
 ***************************************************/
class MappingViewMO {
public:
    MappingViewMO() : m_id(""), m_name(""), m_description(""), m_enableInbandCommand(false)
    {}
    MappingViewMO(const MappingViewMO& mo)
    {
        m_id = mo.m_id;
        m_name = mo.m_name;
        m_description = mo.m_description;
        m_lunGroupName = mo.m_lunGroupName;
        m_lunGroupID = mo.m_lunGroupID;
        m_hostGroupName = mo.m_hostGroupName;
        m_hostGroupID = mo.m_hostGroupID;
        m_enableInbandCommand = mo.m_enableInbandCommand;
    }
    virtual ~MappingViewMO() {}

public:
    std::string m_id;               // 映射视图ID
    std::string m_name;             // 映射视图名称
    std::string m_description;      // 描述信息
    std::string m_lunGroupName;     // LUN组名称
    std::string m_lunGroupID;       // LUN组ID
    std::string m_hostGroupName;    // 主机组名称
    std::string m_hostGroupID;      // 主机组ID
    bool m_enableInbandCommand;     // 是否启用命令设备
};
}
#endif
