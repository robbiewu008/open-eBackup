/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VMwareDef.h
 * @brief  Contains function declarations VMwareDef
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef AGENT_VMWARENATIVE_DEF_H
#define AGENT_VMWARENATIVE_DEF_H

#include <vector>
#include "common/Types.h"

namespace VMWAREDEF {
// const string defination
const mp_string PARAM_DATAPROCESS_SERVICETYPE = "DppServiceType";
const mp_string PARAM_VDDKLIB_INIT_STATUS = "VddkInitState";
const mp_string PARAM_VDDKLIB_VERSION = "VddkLibVersion";
const mp_string PARAM_VDDKLIB_PATH = "VddkLibPath";
const mp_string PARAM_NASMEDIUM_MOUNT_STATUS = "NasMediumStatus";

// protocol of backend storage used
const mp_int32 VMWARE_STORAGE_PROTOCOL_ISCSI = 1;
const mp_int32 VMWARE_STORAGE_PROTOCOL_NAS = 2;
const mp_int32 VMWARE_VDDK_MINJOR_VERSION = 0;
const mp_string PARAM_DISKTASK_STR = "taskId";
const mp_string PARAM_PARENTTASK_STR = "parentTaskId";
const mp_string PARAM_STORAGE_STR = "storage";
const mp_string PARAM_NAS_OVER_FC_STR = "accessNASOverFC";
const mp_string PARAM_STORAGE_TYPE_STR = "storType";
const mp_string PARAM_STORAGE_PROTOCOL_STR = "storProtocol";
const mp_string PARAM_STORAGE_IP_STR = "storageIps";
const mp_string PARAM_LOCAL_STORAGE_IP_STR = "localStorageIps";
const mp_string PARAM_OTHER_STORAGE_IP_STR = "otherStorageIps";
const mp_string PARAM_VOLUMES_STR = "Volumes";
const mp_string PARAM_MEDIUMID_STR = "mediumID";

// disktype
const mp_string PARAM_DISKTYPE_STR = "diskType";
const mp_string VMWARE_NORMAL_DISK = "normal";
const mp_string VMWARE_RDM_DISK = "rdm";

// mount params' key
const mp_string KEY_PARENTTASK_ID = "parentTaskID";
const mp_string KEY_BACKUP_ID = "backupID";
const mp_string KEY_STORAGE_IP = "storageIp";
const mp_string KEY_BACKUP_STORAGE_IP = "backupStorageIp";
const mp_string KEY_NAS_FILESYSTEM_NAME = "nasFileSystemName";
const mp_string STR_VIXDISKLIB_PATH = "vmware-vix-disklib-distrib";
const mp_string KEY_DATATURBOSTORAGENAME = "storageName";
const mp_string KEY_LINKENCRYPTION = "linkEncryption";

// specific value
const mp_int32 DEFAULT_PROGRESS_VALUE = 0;
const mp_int32 PROGRESS_QUERY_FAILURE_VALUE = 3;

// expected transport mode
const mp_uint32 TRANSPORT_MODE_NONE = 0;
const mp_uint32 TRANSPORT_MODE_SAN = 1;
const mp_uint32 TRANSPORT_MODE_HOT_ADD = 2;
const mp_uint32 TRANSPORT_MODE_NBDSSL = 3;
}  // namespace VMWAREDEF

typedef struct tag_vmware_vm_info {
    tag_vmware_vm_info() : strVmID(""), strVmName(""), strVmRef(""), strSnapshotRef("")
    {}
    tag_vmware_vm_info(const mp_string& id, const mp_string& name, const mp_string& vmref, const mp_string& snapref)
        : strVmID(id), strVmName(name), strVmRef(vmref), strSnapshotRef(snapref)
    {}

    mp_string strVmID;
    mp_string strVmName;
    mp_string strVmRef;
    mp_string strSnapshotRef;
} vmware_vm_info;

typedef struct tag_vmware_product_manager_info {
    tag_vmware_product_manager_info()
        : ulProtocol(0),
          ulPort(443),
          ulStatus(0),
          strIP(""),
          strUserName(""),
          strPassword(""),
          strVersion(""),
          strCerts(0)
    {}
    tag_vmware_product_manager_info(mp_uint64 protocol, mp_uint64 port, mp_uint64 status, const mp_string& ip,
        const mp_string& user, const mp_string& pwd, const mp_string& version, const std::vector<mp_string>& vec)
        : ulProtocol(protocol),
          ulPort(port),
          ulStatus(status),
          strIP(ip),
          strUserName(user),
          strPassword(pwd),
          strVersion(version),
          strCerts(vec.begin(), vec.end())
    {}
    mp_uint64 ulProtocol;
    mp_uint64 ulPort;
    mp_uint64 ulStatus;
    mp_string strIP;
    mp_string strUserName;
    mp_string strPassword;
    mp_string strVersion;
    mp_string strThumbPrint;
    std::vector<mp_string> strCerts;
} vmware_pm_info;

typedef struct tag_vmware_protect_env_info {
    mp_string strVmRef;
    mp_string strHostAgentIP;
    vmware_pm_info productManager;
} vmware_pe_info;

typedef struct tag_dirty_range_info {
    tag_dirty_range_info() : start(0), length(0)
    {}
    tag_dirty_range_info(mp_uint64 start, mp_uint64 size) : start(start), length(size)
    {}

    mp_uint64 start;
    mp_uint64 length;
} dirty_range;

typedef struct tag_desc_file_info {
    tag_desc_file_info()
        : iVersion(1),
          strEncode("UTF-8"),
          strCID("fffffffe"),
          strParentCID("ffffffff"),
          strCreateType("vmfs"),
          strAdapterType("lsilogic"),
          strCylinders("261"),
          strHeads("255"),
          strSectors("63"),
          strLogContentID("f268ebb7bf131ab1961938aefffffffe"),
          strUUID(""),
          strVirtualHWVersion("14")
    {}

    void SetCylinders(const mp_string& val)
    {
        this->strCylinders = val;
    }

    void SetHeads(const mp_string& val)
    {
        this->strHeads = val;
    }
    void SetSectors(const mp_string& val)
    {
        this->strSectors = val;
    }

    void SetContentID(const mp_string& contentId)
    {
        this->strLogContentID = contentId;
    }

    void SetUUID(const mp_string& id)
    {
        this->strUUID = id;
    }

    mp_uint32 iVersion;
    mp_string strEncode;
    mp_string strCID;
    mp_string strParentCID;
    mp_string strCreateType;
    mp_string strAdapterType;
    mp_string strCylinders;
    mp_string strHeads;
    mp_string strSectors;
    mp_string strLogContentID;
    mp_string strUUID;
    mp_string strVirtualHWVersion;
} desc_file_info;

struct FilterDIiskInfo {
    FilterDIiskInfo() : m_diskSizeInBytes(0)
    {}
    FilterDIiskInfo(const std::string &taskID, const std::string &diskID, const std::string &diskType,
        const std::string &strWwn, const std::vector<mp_string> &dsWWns, const std::string &diskPath,
        mp_uint64 diskSizeInBytes, const std::string &diskRelativePath, const std::string &vmfsIOFlg,
        const std::string &nasIOFlg)
        : m_taskID(taskID), m_diskID(diskID), m_diskType(diskType), m_strWwn(strWwn), datastoreWwn(dsWWns),
          m_strDiskPath(diskPath), m_diskSizeInBytes(diskSizeInBytes), m_strDiskRelativePath(diskRelativePath),
          m_vmfsIOFlag(vmfsIOFlg), m_nasIOFlag(nasIOFlg)

    {}
    std::string m_taskID;
    std::string m_diskID;
    std::string m_diskType;
    std::string m_strWwn;
    std::vector<mp_string> datastoreWwn;
    std::string m_strDiskPath;
    mp_uint64 m_diskSizeInBytes;

    mp_string m_strDiskRelativePath;
    mp_string m_vmfsIOFlag;
    mp_string m_nasIOFlag;
};

struct DataExclusion {
    DataExclusion() : m_deletedFiles(false), m_tmpFiles(false), m_specified(false)
    {}
    DataExclusion(mp_bool isExclDeleted, mp_bool isExclTmpFiles, mp_bool isExclSpecified,
        const std::vector<std::string>& excludeList)
        : m_deletedFiles(isExclDeleted), m_tmpFiles(isExclTmpFiles), m_specified(isExclSpecified)
    {
        m_specifiedList.assign(excludeList.begin(), excludeList.end());
    }

    bool IsEnable()
    {
        if (m_deletedFiles || m_tmpFiles || (m_specified && m_specifiedList.size() > 0)) {
            return true;
        }
        return false;
    }

    bool IsEnableDeleted()
    {
        if (!m_deletedFiles) {
            return false;
        }
        return true;
    }

    mp_bool m_deletedFiles;                    // Exclude Deleted Files
    mp_bool m_tmpFiles;                        // Tmp files include: pagefile.sys and hiberfil.sys
    mp_bool m_specified;                       // Exclude specified files or directories.
    std::vector<std::string> m_specifiedList;  // enable bExclSpecifiedFiles and exclude files or directorys
};

typedef struct tag_vmware_volume_info {
    tag_vmware_volume_info()
        : strTaskID(""),
          strParentTaskID(""),
          strMediumID(""),
          strBackupedDiskID(""),
          strDiskID(""),
          strDiskType(""),
          strWwn(""),
          strDiskPath(""),
          strDiskRelativePath(""),
          vmfsIOFlag(""),
          nasIOFlag(""),
          strTargetLunPath(""),
          bIsSystem(false),
          ulDiskSize(0),
          uLimitSpeed(0),
          strEagerlyCrub("false"),
          uBackupLevel(2),
          bSupportSAN(false)
    {}

    tag_vmware_volume_info(const mp_string& taskId, const mp_string& pTaskId, const mp_string& mediumId,
        const mp_string& backupedDiskId, const mp_string& id, const mp_string& diskType, const mp_string& wwn,
        const mp_string& path, const mp_string& lunPath,
        mp_bool issystem, mp_uint64 size, mp_uint32 speed, const mp_string& eagerlyCrub, mp_uint16 backupLevel,
        mp_bool supportSAN)
        : strTaskID(taskId),
          strParentTaskID(pTaskId),
          strMediumID(mediumId),
          strBackupedDiskID(backupedDiskId),
          strDiskID(id),
          strDiskType(diskType),
          strWwn(wwn),
          strDiskPath(path),
          strTargetLunPath(lunPath),
          bIsSystem(issystem),
          ulDiskSize(size),
          uLimitSpeed(speed),
          strEagerlyCrub(eagerlyCrub),
          uBackupLevel(backupLevel),
          bSupportSAN(supportSAN)
    {}

    mp_string strTaskID;
    mp_string strParentTaskID;    // parent task id
    mp_string strMediumID;        // wwn or medium id
    mp_string strBackupedDiskID;  // disk backuped
    mp_string strDiskID;
    mp_string strDiskType;
    mp_string strWwn;
    mp_string strDiskPath;
    /* storage snapshot backup */
    std::vector<mp_string> datastoreWwn; /* wwn of the datastores lun snap where disk located */
    mp_string strDiskRelativePath;
    mp_string vmfsIOFlag;
    mp_string nasIOFlag;
    /* storage snapshot backup - END */
    mp_string strTargetLunPath;
    mp_bool bIsSystem;
    mp_uint64 ulDiskSize;
    mp_uint32 uLimitSpeed;
    mp_string strEagerlyCrub;
    mp_uint32 uBackupLevel;  // 1-incr, 2-full
    mp_bool bSupportSAN;
    std::vector<tag_dirty_range_info> vecDirtyRange;
    desc_file_info descFileInfo;
    mp_uint32 transportModeExpected = VMWAREDEF::TRANSPORT_MODE_NONE;

    DataExclusion dataExclude;
} vmware_volume_info;

typedef struct tag_scsi_target_info {
    tag_scsi_target_info()
        : strTargetIP(""), strChapName(""), strChapPwd(""), iTargetPort(0), iNormalType(0), iDiscoveryType(0)
    {}
    tag_scsi_target_info(
        const mp_string& ip, const mp_string& name, const mp_string& pwd,
            mp_int32 port, mp_int32 normaltype, mp_int32 discoverytype)
        : strTargetIP(ip),
          strChapName(name),
          strChapPwd(pwd),
          iTargetPort(port),
          iNormalType(normaltype),
          iDiscoveryType(discoverytype)
    {}

    mp_string strTargetIP;
    mp_string strChapName;
    mp_string strChapPwd;
    mp_uint32 iTargetPort;
    mp_uint32 iNormalType;
    mp_uint32 iDiscoveryType;
} scsi_target_info;

// shared by backup and recovery
typedef struct tag_vmware_protection_params {
    tag_vmware_protection_params()
        : strTaskID(""),
          strParentTaskID(""),
          ulBackupDataLayout(10),
          ulBackupStorageType(10),
          ulSnapType(2),
          bDoCrc(false)
    {}
    tag_vmware_protection_params(
        const mp_string& id, const mp_string& pid, mp_uint64 layout, mp_uint64 type, mp_bool crc, mp_uint64 snaptype)
        : strTaskID(id),
          strParentTaskID(pid),
          ulBackupDataLayout(layout),
          ulBackupStorageType(type),
          ulSnapType(snaptype),
          bDoCrc(crc)
    {}

    mp_string strTaskID;
    mp_string strParentTaskID;
    mp_uint64 ulBackupDataLayout;
    mp_uint64 ulBackupStorageType;
    mp_uint64 ulSnapType;
    mp_bool bDoCrc;
    vmware_vm_info vmInfo;
    vmware_pm_info pmInfo;
    std::vector<vmware_volume_info> volumeInfos;
    std::vector<scsi_target_info> scsiTargets;
} vmware_protection_params;

#endif
