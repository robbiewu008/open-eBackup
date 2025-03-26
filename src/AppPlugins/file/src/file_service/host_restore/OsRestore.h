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
#ifndef OS_RESTORE_H
#define OS_RESTORE_H

#include <unordered_map>
#include <unordered_set>
#include "Backup.h"
#include "log/Log.h"
#include "Module/src/common/JsonHelper.h"

struct OsRestoreInfo {
    std::string sysInfoPath;
    std::string jobId;
};

struct DiskInfo {
    std::string diskName; // e.g. sda,sdb,hda,vda...
    uint64_t diskSize;  // GB
    bool operator==(const DiskInfo& other) const
    {
        return (this->diskName == other.diskName) && (this->diskSize == other.diskSize);
    }
};

class DiskInfoHash {
public:
    std::size_t operator()(const DiskInfo& info) const
    {
        return info.diskSize;
    }
};

struct OSVolumeInfo {
    std::string volumeName; // e.g. sda1,sdb1,hda1,vda1...
    std::string volumeSize;  // GB
    std::string volumeUUID;
    std::string fsType; // 文件系统类型：ext3/ext4/xfs/btrfs/fat32/...
    std::string mountPoint;
};

struct DiskMapInfo {
    std::string sourceDiskName;
    uint64_t sourceDiskSize;
    std::string targetDiskName;
    uint64_t targetDiskSize;
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sourceDiskName, sourceDiskName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(sourceDiskSize, sourceDiskSize)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(targetDiskName, targetDiskName)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(targetDiskSize, targetDiskSize)
    END_SERIAL_MEMEBER
};


class OsRestore {
public:
// 构造等
    explicit OsRestore(const OsRestoreInfo& config);

    bool DiskMapping();
    bool GetTargetDiskInfo();
    bool GetSourceDiskInfo();
    bool DiskMappingByName(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping);

    bool Start();
    bool DiskPartitionRestore();
    bool GetPvInfo();
    bool DiskPartition(const DiskInfo& sourceDisk, const DiskInfo& targetDisk);
    bool LVMRestoration();
    bool NonLVMRestoration();
    bool GetNonLvInfo();
    bool FormatFileSystemForNonLvs();
    void DealBtrfsCase(const OSVolumeInfo& volumeInfo, std::string& cmd);
    bool CreatePhysicalVolume(const std::string& sourcePvName, const std::string& targetPvName,
                              const std::string& pvUuid);
    bool GetVgNames();
    bool GetLvInfo();
    bool CreateVolumeGroup(const std::string& vgName);
    bool ActivateVolumeGroup(const std::string& vgName);
    bool FormatFileSystemForLvs();
    bool MkfsAndAddUidForLvs(const OSVolumeInfo& volumeInfo);

    bool FormatFileSystemForBootPartition();
    bool GetBootVolumeInfoFromFile();
    bool MkfsAndMountBoot(const OSVolumeInfo& volumeInfo);
    std::string FormatUuidToDosSerial(const std::string& vfatUuid) const;

    bool RestoreFsAndMountInfo();
    bool MountBindSysProcDev();

    bool OSRestoreRepairGrub();
    bool GenerateNewGrubCfg();
    void SearchBootDisk(std::string& grubInstallDevice) const;
    bool UseGrub2Install();
    bool UseGrubInstall();

    bool RebootSystem();
    uint64_t GetDiskSize(const std::string& diskSizeStr);
    int RunCommand(const std::string& cmd);
    bool WipeDisk(const std::string& diskName);
    int CleanFstabAndMountAfterFailed();
    bool CheckAndDealNvmeDisk(std::string& grubInstallDevice) const;
    void PrintConfig();
    bool GetFstabMountPoint(std::string& newFstabFile);
    void GenerateNewFstabFile(std::string& newFstabFile);
    bool RestoreBindMountInfo(std::string& newFstabFile);
    bool DiskMappingBySize(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping);
    bool ClassifyDisk(std::unordered_set<DiskInfo, DiskInfoHash> &diskInfoSet,
        std::unordered_map<std::string, std::vector<DiskInfo>> &diskClassifyMap);
    void PrintClassifyDiskInfo();
    bool ReplaceVgFile(const std::string &vgName);
    bool ReplaceAndCreatePv();
    void ReplaceNonLvName(std::string &volumeName);
    bool MakeVgBeforeGenerate();

    bool RebuildInitramfs();
    void SetDiskMap(std::vector<DiskMapInfo> &m_diskMapInfoSet);
    void SetSysInfoPath(const std::string &sysInfoPath);
private:
OsRestoreInfo m_config; // m_sysinfopath
std::string m_sysInfoPath;
std::string m_lvmInfoPath;
std::string m_replaceVgDir;

std::unordered_set<DiskInfo, DiskInfoHash> m_targetDiskInfoSet;
std::unordered_set<DiskInfo, DiskInfoHash> m_sourceDiskInfoSet;
std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> m_diskMapping; // key: sourceDiskInfo, value: targetDiskInfo
bool m_diskMapBySizeSuccess = false;
std::unordered_map<std::string, std::vector<DiskInfo>> m_sourceDiskClassifyMap; // key: disktype, value: sourceDiskInfo
std::unordered_map<std::string, std::vector<DiskInfo>> m_targetDiskClassifyMap; // key: disktype, value: targetDiskInfo
std::vector<std::string> m_vgNames;
std::unordered_map<std::string, std::string> m_lvFsType;              // key: lv_name, value: fsType
std::unordered_map<std::string, std::string> m_nonLvFsType;              // key: non_lv_name, value: fsType
std::unordered_map<std::string, std::vector<std::string>> m_pvInfo; // key: pv_name, value: vg_name, pv_uuid
std::unordered_set<std::string> m_fstabMountPoint;

OSVolumeInfo m_bootVolumeInfo;
OSVolumeInfo m_bootEfiVolumeInfo;
std::vector<OSVolumeInfo> m_nonLvmInfo;
};
#endif