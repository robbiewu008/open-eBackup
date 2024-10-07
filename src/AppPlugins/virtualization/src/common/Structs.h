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
#ifndef STRUCTS_H
#define STRUCTS_H

#include <string>
#include <memory>
#include <list>
#include <vector>
#include <json/json.h>
#include "Macros.h"
#include "ApplicationProtectFramework_types.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "common/Constants.h"
#include "common/JsonHelper.h"
#include "common/PluginTypes.h"
#include "job/JobCommonInfo.h"
#include "common/CleanMemPwd.h"
#include <job/BasicJob.h>

using AppProtect::SubJobStatus;
using AppProtect::JobLogLevel;

namespace VirtPlugin {
enum class RestoreLevel {
    RESTORE_TYPE_VM = 0,
    RESTORE_TYPE_DISK,
    RESTORE_TYPE_UNKNOW
};

enum class LivemountType {
    MOUNT = 0,
    MOUNT_CANCEL,
    MIGRATION,
    RESTORE,
    UNKNOWN
};

enum class JobStage {
    PRE_PREREQUISITE,
    GENERATE_SUB_JOB,
    EXECUTE_SUB_JOB,
    EXECUTE_POST_SUB_JOB,
    POST_JOB,
};

enum class HookType {
    PRE_HOOK,
    POST_HOOK
};

typedef struct ExecHookParam {
    HookType hookType{ HookType::PRE_HOOK };
    JobStage stage{ JobStage::PRE_PREREQUISITE };
    int nextState{0};
    int postHookState{0};
    int jobExecRet{Module::SUCCESS};
} ExecHookParam;

/**
 *  任务句柄，用于保存、获取任务中相关的信息，如生产环境信息、虚拟机信息等
 */
class JobHandle {
public:
    JobHandle(const JobType &jobType, std::shared_ptr<JobCommonInfo> jobInfo) : m_jobType(jobType), m_jobInfo(jobInfo)
    {
    }

    AppProtect::ApplicationEnvironment GetAppEnv() const;
    AppProtect::Application GetApp() const;
    std::vector<AppProtect::ApplicationResource> GetVolumes() const;
    JobType GetJobType() const
    {
        return m_jobType;
    }
    std::shared_ptr<JobCommonInfo> GetJobCommonInfo()
    {
        return m_jobInfo;
    }
    std::vector<AppProtect::StorageRepository> GetStorageRepos() const;
    AppProtect::BackupJobType GetBackupType() const;
    std::string GetTaskId() const;

protected:
    JobType m_jobType = JobType::UNDEFINED_JOB_TYPE;
    std::shared_ptr<JobCommonInfo> m_jobInfo;
};

/**
 * 卷所在底层存储拓展信息。
 * 如果应用有其他字段需要添加，可以继承此类。
 */
class VolumeDSExtendInfo {
public:
    std::string m_volId;     /* 卷底层生产存储的卷 ID, e.g. lun id */
    std::string m_volWwn;    /* 卷底层生产存储的卷 wwn e.g. lun wwn */
    std::string m_volName;   /* 卷底层生产存储的卷 name */

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volId, volId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volWwn, volWwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volName, volName)
    END_SERIAL_MEMEBER
};

/**
 *  卷所在的Datastore信息
 */
struct DatastoreInfo {
    std::string m_name;     // 名称
    std::string m_type;     // 存储类型
    std::string m_moRef;    // 对象的MoRef （存储的SN，备份必传）
    std::string m_dcMoRef;  // 所属DC的MoRef
    std::string m_ip;       // IP地址
    std::string m_port;     // 端口
    std::string m_poolId;   // 存储池ID
    std::string m_volumeName; // 底层存储卷名称 fusionstorage使用
    std::string m_ipList;
    /**
     * Application defined struct - VolumeDSExtendInfo:
     * Flexvolume/HCS:
     * {
     *    "volId", "",
     *    "volWwn", "",
	 *    "volName", ""
     * }
     */
    std::string m_extendInfo;
    std::string m_details;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, moRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_dcMoRef, dcMoRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_port, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_poolId, poolId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeName, volumeName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, ipList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_details, details)
    END_SERIAL_MEMEBER
};

class VolSnapInfo {
public:
    std::string m_volUuid;       // 卷唯一标识
    std::string m_snapshotName;  // 快照名称(每个卷的快照名称需要不同)
    std::string m_snapshotId;    // 快照ID
    std::string m_storageSnapId; // 存储底层快照ID
    std::string m_snapshotWwn;   // 快照wwn
    std::string m_createTime;    // 快照时间
    uint64_t m_size;             // 快照大小
    bool m_deleted{false};       // 快照是否已删除
    DatastoreInfo m_datastore;   // 快照所在卷所在的生产存储信息
    std::string m_metadata;      // 快照元数据
    std::string m_status;        // 快照状态
    std::string m_extendInfo;
    std::string m_bitmapName;
    std::string m_snapshotDescription;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volUuid, volUuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotName, snapshotName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotId, snapshotId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageSnapId, storageSnapId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotWwn, snapshotWwn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_createTime, createTime)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_deleted, deleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_datastore, datastore)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bitmapName, bitmapName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapshotDescription, snapshotDescription)
    END_SERIAL_MEMEBER
};

/**
 *  整机快照信息
 */
class SnapshotInfo {
public:
    std::string m_moRef;    // 快照标识，如VMware的moref，FusionCompute的uri
    std::string m_vmName;   // 产生快照的虚拟机名称
    std::string m_vmMoRef;  // 产生快照的虚拟机UUID
    std::vector<VolSnapInfo> m_volSnapList;
    bool m_deleted;         // 快照是否已删除(当所有卷的快照删除后，此标记置为 true )
                            // 应用在查询快照是否存在的接口需要填入正确的值
    std::string m_consistengroupId;  // 一致性卷组ID
    bool m_isconsistentSnapshot {false};   // 是否为一致性卷组快照
    std::string m_snapFlag;         // 快照标志位，供差异化区分快照使用
    std::string m_extendInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, moRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmName, vmName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmMoRef, vmMoRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volSnapList, volSnapList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_deleted, deleted)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_consistengroupId, consistentgroupId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_isconsistentSnapshot, isconsistentSnapshot)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_snapFlag, snapFlag)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

/**
 *  生产存储信息
 */
struct StorageInfo {
    std::string m_sn;
    std::string m_ip;
    int32_t m_port;
    std::string m_userName;
    std::string m_passWd;
    std::string m_enableCert = "0";
    std::string m_certification;
    std::string m_revocationList;
    std::string m_storageType;
    std::string m_ipList;
    std::string m_vbsUserName;
    std::string m_vbsPassWord;
    std::string m_vbsIp;
    int32_t m_vbsPort;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_sn, sn)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_port, port)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_userName, username)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_passWd, password)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_enableCert, enableCert)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_certification, certification)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_revocationList, revocationlist)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storageType, storageType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ipList, ipList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vbsUserName, vbsNodeUserName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vbsIp, vbsNodeIp)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vbsPort, vbsNodePort)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vbsPassWord, vbsNodePassword)
    END_SERIAL_MEMEBER
    StorageInfo()
    {
    }
    ~StorageInfo()
    {
        Module::CleanMemoryPwd(m_passWd);
        Module::CleanMemoryPwd(m_certification);
    }
};

struct Storages {
    std::string m_storages;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storages, storages)
    END_SERIAL_MEMEBER
};

struct JobTypeParam {
    AppProtect::BackupJob m_job;
    SnapshotInfo m_snapshotInfo;
};

// protected environment object type(vCenter,ESX,VRM and so on)
enum class EnvObjType {
    INVALID = 1000
};

struct StorageResource {
    std::string m_name;
    std::string m_moref;
    std::string m_extendInfo;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, storageName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moref, storageMoref)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER

    uint64_t size;
    StorageResource() : size(0)
    {
    }
};

struct NetCardInfo {
    std::string adapterName;
    std::string networkName;
    std::string networkMoRef;
    std::string m_extendInfo;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(adapterName, adapterName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(networkName, networkName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(networkMoRef, networkMoRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

enum class RestorePlace {
    ORIGINAL_PLACE,
    NEW_PLACE,
    SPECIFY_PLACE
};

struct VolAttachMents {
    VolAttachMents()
    {
    }
    explicit VolAttachMents(std::string device) : m_device(device)
    {
    }
    std::string m_device;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_device, device)
    END_SERIAL_MEMEBER
};

/**
 *  卷信息
 */
class VolInfo {
public:
    std::string m_moRef;        // 卷标识，如VMware的moref，FusionCompute的uri（备份必传）
    std::string m_uuid;         // 卷的UUID （卷ID，备份必传）
    std::string m_name;         // 卷名称 （备份必传）
    std::string m_type;         // 卷类型 （存储设备型号，备份必传）
    std::string m_vmMoRef;      // 卷所属的虚拟机标识符 （保护对象唯一标识，备份必传）
    DatastoreInfo m_datastore;  // 卷所在的底层存储信息 （卷所在存储设备信息，备份必传）
    uint64_t m_volSizeInBytes;  // 卷大小 （备份必传）
    std::string m_slotId;       // 槽位号，源卷和目标卷根据槽位号以及卷底层存储对应
    std::string m_metadata;     // 卷元数据
    bool m_newCreate {false};           // 是否是新创卷
    std::vector<VolAttachMents> m_attachPoints;
    std::string m_extendInfo;
    std::string m_bootable;     // 是否为启动盘
    std::string m_volumeType;   // volume type
    std::string m_location;    // 卷所属位置，如项目、区域、可用区
    std::string m_provisionType;    // 置备类型
    bool m_supportBackup {true};
    bool operator==(const VolInfo &vol)
    {
        return m_volSizeInBytes == vol.m_volSizeInBytes && m_uuid == vol.m_uuid;
    }

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, moRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_type, type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_vmMoRef, vmMoRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_datastore, datastore)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volSizeInBytes, volSizeInBytes)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_slotId, slotId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metadata)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_newCreate, ifNewVolume)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_attachPoints, attachments)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_bootable, bootable)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volumeType, volume_type)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_location, location)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_provisionType, provisionType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_supportBackup, supportBackup)
    END_SERIAL_MEMEBER
};

class BridgeInterfaceInfo {
public:
    std::string m_moRef;             // 网卡虚拟机标识，如VMware的moref，FusionCompute的uri
    std::string m_uuid;              // 网卡的UUID
    std::string m_name;              // 网卡端口组名称
    std::string m_parentName;        // 网卡交换机名称
    std::string m_mac;
    std::string m_ip;
    std::string m_metadata;          // 网卡元数据
    /* extend info */
    std::string m_extendInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, moRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, uuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_parentName, parentName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_mac, mac)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_ip, ip)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metaData)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

/**
 *  虚拟机信息
 */
class VMInfo {
public:
    std::string m_moRef;             // 虚拟机标识，如VMware的moref，FusionCompute的uri
    std::string m_uuid;              // 虚拟机的UUID
    std::string m_name;              // 虚拟机名称
    std::string m_location;          // 虚拟机所属，可以是集群或主机的moref
    std::string m_locationName;      // 虚拟机所属，可以是集群或主机的名称
    std::string m_metadata;          // 虚拟机元数据
    std::vector<VolInfo> m_volList;  // 卷列表
    std::vector<BridgeInterfaceInfo> m_interfaceList;  // 网卡列表
    /* extend info */
    std::string m_extendInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_moRef, moRef)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_uuid, volUuid)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, volName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_location, targetFolderLocation)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_locationName, targetFolderLocationName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_metadata, metaData)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volList, volList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_interfaceList, interfaceList)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

class ReportJobDetailsParam {
public:
    std::string label;
    JobLogLevel::type level { JobLogLevel::TASK_LOG_INFO };
    SubJobStatus::type status { SubJobStatus::INITIALIZING };
    uint32_t progress { 0 };
    uint64_t dataSize { 0 };
    int64_t errCode { 0 };
    std::string extendInfo = "";
};

struct ApplicationLabelType {
    std::string label;
    JobLogLevel::type level{ JobLogLevel::TASK_LOG_INFO };
    std::vector<std::string> params; /* log description(label) parameters */
    int64_t errCode = 0;
    std::vector<std::string> errorParams; /* error code parameters */
    std::vector<std::string> additionalDesc; /* log additional information for detail description */
};

class ReportJobResultPara {
public:
    ReportJobDetailsParam m_jobDetailsParam;
    std::vector<std::string> m_args;
};

struct VolPair {
    VolInfo m_originVol;  // 源卷信息
    VolInfo m_targetVol;  // 目标卷信息
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_originVol, originVol)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_targetVol, targetVol)
    END_SERIAL_MEMEBER
};

struct VolMatchPairInfo {
    std::vector<VolPair> m_volPairList;
    void AddVolPair(const VolInfo &originVol, const VolInfo &targetVol)
    {
        VolPair volPair;
        volPair.m_originVol = originVol;
        volPair.m_targetVol = targetVol;
        m_volPairList.push_back(volPair);
    }
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volPairList, volPairList)
    END_SERIAL_MEMEBER
};

enum class VolAccessMode {
    REPLACE = 0,  // 恢复到原卷
    REBUILD       // 新建卷
};

struct ProductEnv {
    ProductEnv()
    {
    }
    ~ProductEnv()
    {
    }
    std::string ip;
    int port;
    std::string userName;
    std::string password;
    std::string protocol;
    std::string cert;
};

// 恢复任务高级参数
struct RestoreExtendInfo {
    DatastoreInfo m_datastoreInfo{};  // 恢复目的datastore信息
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_datastoreInfo, datastoreInfo)
    END_SERIAL_MEMEBER
};

// 恢复任务子任务高级参数
struct SubJobExtendInfo {
    uint32_t m_subTaskType {};
    std::string m_targetVolumeInfo {};
    std::string m_originVolumeInfo {};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_subTaskType, SubTaskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_targetVolumeInfo, TargetVolumeInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_originVolumeInfo, OriginVolumeInfo)
    END_SERIAL_MEMEBER
};

struct ArchiveS3Info {
    struct IpPort {
        std::string ip;
        int port { -1 };
        BEGIN_SERIAL_MEMEBER
        SERIAL_MEMBER_TO_SPECIFIED_NAME(ip, ip)
        SERIAL_MEMBER_TO_SPECIFIED_NAME(port, port)
        END_SERIAL_MEMEBER
    };

    std::vector<IpPort> serviceInfo;
    bool enableSSL {false};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(enableSSL, enable_ssl)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(serviceInfo, service_info)
    END_SERIAL_MEMEBER
};

struct AuthObj {
    std::string name;
    std::string passwd;
    std::string cert;      // 证书
    std::string revocationList; // 吊销列表
    bool certVerifyEnable {false}; // 是否进行证书认证

    ~AuthObj()
    {
        Module::CleanMemoryPwd(passwd);
        Module::CleanMemoryPwd(cert);
    }
};

struct RequestInfo {
    std::string m_method;   // GET,POST,PUT,HEAD,DELETE,......
    std::string m_resourcePath; // url
    std::map<std::string, std::string> m_pathParams;    // 路径中需要补充的参数
    std::map<std::string, std::string> m_queryParams;   // 查询条件
    std::map<std::string, std::string> m_headerParams;
    std::string m_body;
    AuthObj m_auth;

    ~RequestInfo()
    {
        if (m_headerParams.find("X-Auth-Token") != m_headerParams.end()) {
            Module::CleanMemoryPwd(m_headerParams["X-Auth-Token"]);
        }
    }
};

struct BackupSubJobInfo {
    VolInfo m_volInfo;
    VolSnapInfo m_preSnapshotInfo;
    VolSnapInfo m_curSnapshotInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_volInfo, volInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_preSnapshotInfo, preSnapshotInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_curSnapshotInfo, curSnapshotInfo)
    END_SERIAL_MEMEBER
};

struct RestoreSubJobInfo {
    uint32_t m_subTaskType {};
    VolInfo m_targetVolumeInfo;
    VolInfo m_originVolumeInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_subTaskType, SubTaskType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_targetVolumeInfo, TargetVolumeInfo)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_originVolumeInfo, OriginVolumeInfo)
    END_SERIAL_MEMEBER
};

struct ReportLog2AgentParam {
    SubJobDetails subJobDetails;
    ActionResult returnValue;
    std::vector<LogDetail> logDetailList;
    LogDetail logDetail;
    uint32_t progress { 0 };
    uint64_t dataSize { 0 };
    SubJobStatus::type curJobstatus;
};

struct Cpu {
    bool m_useOriginal {true};
    std::string m_core;
    std::string m_socket;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_useOriginal, use_original)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_core, num_virtual_sockets)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_socket, num_cores_per_virtual)
    END_SERIAL_MEMEBER
};

struct Memory {
    bool m_useOriginal {true};
    std::string m_size;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_useOriginal, use_original)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_size, memory_size)
    END_SERIAL_MEMEBER
};


struct MachineConfig {
    Cpu m_cpu;
    Memory m_memory;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_cpu, cpu)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_memory, memory)
    END_SERIAL_MEMEBER
};

struct ConfigLivemount {
    MachineConfig m_config;
    bool m_powerOn;
    std::string m_name;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_config, config)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_powerOn, power_on)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    END_SERIAL_MEMEBER
};

struct Livemount {
    std::string m_name;
    std::string m_poolId;
    std::string m_storeId;
    std::string m_extendInfo;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_poolId, poolId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_storeId, storeId)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(m_extendInfo, extendInfo)
    END_SERIAL_MEMEBER
};

/* Virtual plugin repo layout:
/*
 * Virtual plugin repo layout:
    ├── cache_repo
    │   └── VIRTUAL_PLUGIN_CACHEDATA
    │       └── restore_job
    │           ├── new_vm.info         // 用于虚拟机恢复时前置任务保存新建虚拟机信息,以便后置任务使用
    │           ├── new_vols.info       // 用于虚拟机恢复时前置任务保存新建的卷列表信息，以便后置任务使用
    │           └── vol_match.info
    │       └── checkpoint
    │           └── main_job_(id)
    │               └── sub_job_(id1)
    │               └── sub_job_(id2)
    │       └── main_task_status.info  // 当执行完前置、分解任务后，生成此文件
    │                                  // 场景：多代理执行恢复任务时，当分解完子任务，插件已经向Agent上报分解完成信息，但是Agent还未向UBC上报。此时stop掉其中某个Agent进程，
    │                                  // UBC由于没收到主任务完成的信息，再次下发主任务以及子任务至另一个Agent上，主任务和子任务同时执行，恢复会报错。
    │                                  // 方案：主任务执行完成后，生成一个标记文件至cache仓。当再次执行主任务时，发现此文件，主任务直接跳过。
    ├── data_repo
    └── meta_repo
        └── VIRTUAL_PLUGIN_METADATA
            ├── pre_snapshot.info
            ├── snapshot.info
            ├── vm.info
            ├── snapshot.tobedeleted       // 将删除的快照，多次删除失败后，发送告警，并保存到残存的快照信息里
            ├── snapshot_residual.info     // 残留的快照信息，会再次尝试删除，若成功，则清除告警
            └── volumes
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E1.ovf
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E2.ovf
                └── {VOL_UUID}.ovf
            └── volumes_block_bitmap      // 卷4M块数据位图信息, 每个block用一位表示是否分配数据 1-已分配数据，0-未分配数据
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E1_block_bitmap.info
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E2_block_bitmap.info
                └── {VOL_UUID}_block_bitmap.info
            └── volumes_hash_dir
                └── {VOL_UUID}_hash.info
        └── sha256file
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E1_sha256.info
                ├── CD042510-B090-4E2B-9AA0-FE43C0F991E2_sha256.info
                └── {VOL_UUID}_sha256.info
*/
#ifndef WIN32
const std::string VIRT_PLUGIN_META_ROOT = "/VIRTUAL_PLUGIN_METADATA/";
const std::string VIRT_PLUGIN_CACHE_ROOT = "/VIRTUAL_PLUGIN_CACHEDATA/";
const std::string VIRT_PLUGIN_SHA_FILE_ROOT = "/sha256file/";
const std::string VIRT_PLUGIN_META_BACKUPJOB_ROOT = VIRT_PLUGIN_META_ROOT + "/backup_job/";
const std::string VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "/restore_job/";
const std::string VIRT_PLUGIN_CACHE_LIVEMOUNTJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "/livemount_job/";
const std::string VIRT_PLUGIN_CACHE_CANCELMOUNTJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "/cancel_mount_job/";
const std::string VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT = VIRT_PLUGIN_CACHE_ROOT + "/checkpoint/";

/* job context saved in cache repository */
const std::string VIRT_PLUGIN_SNAP_TOBEDELETED = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "/snapshot.tobedeleted";
const std::string VIRT_PLUGIN_SNAP_RESIDUAL = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "/snapshot_residual.info";
const std::string VIRT_PLUGIN_NEW_VM_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "/new_vm.info";
const std::string VIRT_PLUGIN_VOL_MATCH_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "/vol_match.info";
const std::string VIRT_PLUGIN_RESTORE_TARGET_VM_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "/target_vm.info";
const std::string VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO = VIRT_PLUGIN_CACHE_ROOT + "/main_task_status.info";
const std::string VIRT_PLUGIN_PRE_VOLUMES_INFO = VIRT_PLUGIN_CACHE_ROOT + "/volumes/";
const std::string VIRT_PLUGIN_LIVE_MOUNT_INFO = VIRT_PLUGIN_CACHE_ROOT + "/live.tobedeleted";

/* metadata saved in meta repository */
const std::string VIRT_PLUGIN_VM_INFO = VIRT_PLUGIN_META_ROOT + "vm.info";
const std::string VIRT_PLUGIN_LIVE_VM_INFO = VIRT_PLUGIN_META_ROOT + "livevm.info";
const std::string VIRT_PLUGIN_SNAPSHOT_INFO = VIRT_PLUGIN_META_ROOT + "snapshot.info";
const std::string VIRT_PLUGIN_PRE_SNAPSHOT_INFO = VIRT_PLUGIN_META_ROOT + "pre_snapshot.info";
const std::string VIRT_PLUGIN_VOLUMES_META_DIR = VIRT_PLUGIN_META_ROOT + "/volumes/";
const std::string VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP = VIRT_PLUGIN_META_ROOT + "volumes_block_bitmap/";
/* volumes hash file repository */
const std::string VIRT_PLUGIN_VOLUMES_HASH_DIR = VIRT_PLUGIN_META_ROOT + "volumes_hash_dir/";
const std::string VIRT_PLUGIN_SNAPHOT_CREATE_VOLUME_INFO = VIRT_PLUGIN_META_ROOT + "snapshot_create_volume.info";
const std::string VIRT_PLUGIN_GROUP_INFO = VIRT_PLUGIN_META_ROOT + "groupsnapshot.info";
const std::string VIRT_PLUGIN_VOLUME_TOBEDELETED = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "/volumes.tobedeleted";
const std::string VIRT_PLUGIN_VOLUME_RESIDUAL = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "/volumes_residual.info";
const std::string VIRT_VM_BACKUP_FILE_PATH = VIRT_PLUGIN_META_ROOT + "/backup_vm_file_dir";
#else
const std::string VIRT_PLUGIN_META_ROOT = "\\VIRTUAL_PLUGIN_METADATA\\";
const std::string VIRT_PLUGIN_CACHE_ROOT = "\\VIRTUAL_PLUGIN_CACHEDATA\\";
const std::string VIRT_PLUGIN_SHA_FILE_ROOT = "\\sha256file\\";
const std::string VIRT_PLUGIN_META_BACKUPJOB_ROOT = VIRT_PLUGIN_META_ROOT + "backup_job\\";
const std::string VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "restore_job\\";
const std::string VIRT_PLUGIN_CACHE_LIVEMOUNTJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "livemount_job\\";
const std::string VIRT_PLUGIN_CACHE_CANCELMOUNTJOB_ROOT = VIRT_PLUGIN_CACHE_ROOT + "cancel_mount_job\\";
const std::string VIRT_PLUGIN_CACHE_CHECKPOINT_ROOT = VIRT_PLUGIN_CACHE_ROOT + "checkpoint\\";

/* job context saved in cache repository */
const std::string VIRT_PLUGIN_SNAP_TOBEDELETED = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "snapshot.tobedeleted";
const std::string VIRT_PLUGIN_SNAP_RESIDUAL = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "snapshot_residual.info";
const std::string VIRT_PLUGIN_NEW_VM_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "new_vm.info";
const std::string VIRT_PLUGIN_VOL_MATCH_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "vol_match.info";
const std::string VIRT_PLUGIN_RESTORE_TARGET_VM_INFO = VIRT_PLUGIN_CACHE_RESTOREJOB_ROOT + "target_vm.info";
const std::string VIRT_PLUGIN_GEN_MAIN_TASK_STATUS_INFO = VIRT_PLUGIN_CACHE_ROOT + "main_task_status.info";
const std::string VIRT_PLUGIN_PRE_VOLUMES_INFO = VIRT_PLUGIN_CACHE_ROOT + "volumes\\";
const std::string VIRT_PLUGIN_LIVE_MOUNT_INFO = VIRT_PLUGIN_CACHE_ROOT + "live.tobedeleted";

/* metadata saved in meta repository */
const std::string VIRT_PLUGIN_VM_INFO = VIRT_PLUGIN_META_ROOT + "vm.info";
const std::string VIRT_PLUGIN_LIVE_VM_INFO = VIRT_PLUGIN_META_ROOT + "livevm.info";
const std::string VIRT_PLUGIN_SNAPSHOT_INFO = VIRT_PLUGIN_META_ROOT + "snapshot.info";
const std::string VIRT_PLUGIN_PRE_SNAPSHOT_INFO = VIRT_PLUGIN_META_ROOT + "pre_snapshot.info";
const std::string VIRT_PLUGIN_VOLUMES_META_DIR = VIRT_PLUGIN_META_ROOT + "volumes\\";
const std::string VIRT_PLUGIN_VOLUMES_BLOCK_BITMAP = VIRT_PLUGIN_META_ROOT + "volumes_block_bitmap\\";
/* volumes hash file repository */
const std::string VIRT_PLUGIN_VOLUMES_HASH_DIR = VIRT_PLUGIN_META_ROOT + "volumes_hash_dir\\";
const std::string VIRT_PLUGIN_SNAPHOT_CREATE_VOLUME_INFO = VIRT_PLUGIN_META_ROOT + "snapshot_create_volume.info";
const std::string VIRT_PLUGIN_GROUP_INFO = VIRT_PLUGIN_META_ROOT + "groupsnapshot.info";
const std::string VIRT_PLUGIN_VOLUME_TOBEDELETED = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "volumes.tobedeleted";
const std::string VIRT_PLUGIN_VOLUME_RESIDUAL = VIRT_PLUGIN_META_BACKUPJOB_ROOT + "volumes_residual.info";
const std::string VIRT_VM_BACKUP_FILE_PATH = VIRT_PLUGIN_META_ROOT + "backup_vm_file_dir";
#endif

/* sha data saved in meta repository */
const std::string VIRT_PLUGIN_SHA_INTERRUPT_INFO = VIRT_PLUGIN_SHA_FILE_ROOT + "sha256_interrupt.info";
const std::string VIRT_PLUGIN_SHA_FAILED_INFO = VIRT_PLUGIN_SHA_FILE_ROOT + "sha256_fail.info";
const std::string VIRT_PLUGIN_CEPH_SNAPSHOT_INFO = VIRT_PLUGIN_META_ROOT + "cephSnapshot.info";
}
#endif  // STRUCTS_H
