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
#ifndef BARE_METAL_RECOVERY_H
#define BARE_METAL_RECOVERY_H

#include <unordered_map>
#include <unordered_set>
#include "Backup.h"
#include "log/Log.h"
#include "JsonHelper.h"

struct DiskInfo {
    std::string diskName; // e.g. sda,sdb,hda,vda...
    uint64_t diskSize;  // GB
    bool operator==(const DiskInfo& other) const
    {
        return (this->diskName == other.diskName) && (this->diskSize == other.diskSize);
    }
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

class DiskInfoHash {
public:
    std::size_t operator()(const DiskInfo& info) const
    {
        return info.diskSize;
    }
};

// 先写成结构体，后续可能增加参数
struct BareMetalRecoveryConfig {
    std::string sysInfoPath;
    std::string jobId;
    // key: targetDiskInfo, value: sourceDiskInfo
    std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> diskMapping;
};

struct VolumeInfo {
    std::string volumeName; // e.g. sda1,sdb1,hda1,vda1...
    std::string volumeSize;  // GB
    std::string volumeUUID;
    std::string fsType; // 文件系统类型：ext3/ext4/xfs/btrfs/fat32/...
    std::string mountPoint;
};

class BareMetalRecovery {
public:
    explicit BareMetalRecovery(const BareMetalRecoveryConfig& config);

    BackupRetCode Start();
    BackupRetCode Abort();
    bool SetDiskMapInfo(std::string &diskMapInfoSet);
    bool GetTargetDiskInfo();   // 裸机恢复前，获取目标主机的磁盘信息
    bool GetSourceDiskInfo();
    bool MountBindSysProcDev();
    bool GenerateNewGrubCfg();
    bool FormatFileSystemForBootPartition();
    bool RestoreFsAndMountInfo();
    bool CopyBoot(const std::string& path);
    bool DiskMapping();
    int RunCommand(const std::string& cmd);
    bool MkfsAndMountBoot(const VolumeInfo& volumeInfo);

    uint64_t GetDiskSize(const std::string& diskSizeStr);
    bool DiskPartition(const DiskInfo& sourceDisk, const DiskInfo& targetDisk);       // 目标主机磁盘分区
    
    bool DiskMappingByName(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping);
    bool DiskMappingBySize(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping);

    bool LvmRestoration();
    bool CreatePhysicalVolume(const std::string& sourcePvName, const std::string& targetPvName,
        const std::string& pvUuid);
    bool CreateVolumeGroup(const std::string& vgName);
    bool ActivateVolumeGroup(const std::string& vgName);
    bool FormatFileSystemForLvs();

    bool GetBootVolumeInfoFromFile();
    bool RebuildInitramfs();
    bool UseGrub2Install();
    bool UseGrubInstall();

    bool GetVgNames();
    bool GetPvInfo();
    bool GetLvInfo();
    void SearchBootDisk(std::string& grubInstallDevice) const;
    std::string FormatUuidToDosSerial(const std::string& vfatUuid) const;
    bool ReplaceVgFile(const std::string &vgName);
    bool ReplaceAndCreatePv();
    void ReplaceNonLvName(std::string &volumeName);
    bool MakeVgBeforeGenerate();

    std::unordered_set<DiskInfo, DiskInfoHash> m_targetDiskInfoSet;
    std::unordered_set<DiskInfo, DiskInfoHash> m_sourceDiskInfoSet;
    std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> m_diskMapping; // key: targetDiskInfo, value: sourceDiskInfo
    std::vector<std::string> m_vgNames;
    std::unordered_map<std::string, std::string> m_lvFsType;              // key: lv_name, value: fsType
    std::unordered_map<std::string, std::vector<std::string>> m_pvInfo; // key: pv_name, value: vg_name, pv_uuid

    BareMetalRecoveryConfig m_config;
    bool m_abort { false };
    std::string m_lvmInfoPath;
    std::string m_replaceVgDir;

    VolumeInfo m_bootVolumeInfo;
    VolumeInfo m_bootEfiVolumeInfo;

private:
    bool WipeDisk(const std::string& diskName);
};

#endif