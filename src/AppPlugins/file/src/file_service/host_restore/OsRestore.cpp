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
#include <cctype>
#include <sstream>
#include <fstream>
#include <algorithm>
#include <sys/stat.h>
#include "define/Types.h"
#include "system/System.hpp"
#include "FSBackupUtils.h"
#include "OsRestore.h"
#include "PluginUtilities.h"
#include "common/Path.h"
#include "common/EnvVarManager.h"

using namespace std;
using namespace Module;


namespace {
    constexpr uint32_t NUMBER9 = 9;
    constexpr uint32_t NUMBER8 = 8;
    constexpr uint32_t NUMBER7 = 7;
    constexpr uint32_t NUMBER6 = 6;
    constexpr uint32_t NUMBER5 = 5;
    constexpr uint32_t NUMBER4 = 4;
    constexpr uint32_t NUMBER3 = 3;
    constexpr uint32_t NUMBER2 = 2;
    constexpr uint32_t NUMBER1 = 1;
    constexpr uint32_t NUMBER0 = 0;

    const uint64_t ONE_KB = 1024LLU;
    const uint64_t ONE_MB = 1024LLU * ONE_KB;
    const uint64_t ONE_GB = 1024LLU * ONE_MB;
    const uint64_t ONE_TB = 1024LLU * ONE_GB;

    const string MODULE = "OsRestore";
    const std::string PARTITIONS_INFO_PREFIX = "partition__dev_";
    const std::string PARTITIONS_INFO_PREFIX_DOS = "dos_partition__dev_";
    const std::string PARTITIONS_INFO_PREFIX_GPT = "gpt_partition__dev_";
    const std::string FSTAB = "fstab";
    const std::string TMPFSTAB = "tmp_fstab";
    const std::string MOUNTED_INFO = "mounts";
    const std::string FSTAB_PATH = "/etc/fstab";
    const std::string LVS_COMMAND_PATH = "/usr/sbin/lvs";
    const std::string VGLIST = "vglist";
    const std::string PVLIST = "pvlist";
    const std::string LV_FSTYPE = "lv_fsType";
    const std::string NON_LV_FSTYPE = "non_lv_fsType";
    const std::string SWAP = "swap";
    const std::string EFI_SYSTEM_DIR = "/sys/firmware/efi/efivars";
    const std::string GRUB_CONFIG = "/boot/grub2/grub.cfg";
    const std::string OLD_GRUB_CONFIG1 = "/boot/grub/grub.cfg";
    const std::string OLD_GRUB_CONFIG2 = "/boot/grub/grub.conf";
    const uint8_t BLK_DISK_FILE_COLUMN_LENGTH = 4;
    const uint8_t FSTAB_FILE_COLUMN_LENGTH = 6;
    const uint64_t SUPPER_BLOCK_SIZE = 2048;

    const std::string BMR_SUCCEED_FLAG_SUFFIX = "_bmr_succeed";
    const std::string BMR_LIVE_OS_FLAG_PATH = "/etc/databackup-bmr-livecd";
    const std::string REPLACE_VG_Dir = "/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/FilePlugin/VG";
}


OsRestore::OsRestore(const OsRestoreInfo& config)
{
    m_config = config;
    m_sysInfoPath = m_config.sysInfoPath;
    m_lvmInfoPath = FSBackupUtils::JoinPath(m_config.sysInfoPath, "lvm_info/");
    m_replaceVgDir = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + REPLACE_VG_Dir;
    DBGLOG("m_replaceVgDir PATH:   %s ", m_replaceVgDir.c_str());
    return;
}

void OsRestore::PrintConfig()
{
    INFOLOG("m_config.sysInfoPath: %s, m_config.jobId: %s", m_sysInfoPath.c_str(), m_config.jobId.c_str());
    return;
}

void OsRestore::SetSysInfoPath(const std::string &sysInfoPath)
{
    m_sysInfoPath = sysInfoPath;
    m_lvmInfoPath = FSBackupUtils::JoinPath(m_sysInfoPath, "lvm_info");
    return;
}

void OsRestore::SetDiskMap(std::vector<DiskMapInfo> &m_diskMapInfoSet)
{
    for (const DiskMapInfo &diskMapInfo : m_diskMapInfoSet) {
        DiskInfo sourceDiskInfo{diskMapInfo.sourceDiskName, diskMapInfo.sourceDiskSize};
        DiskInfo targetDiskInfo{diskMapInfo.targetDiskName, diskMapInfo.targetDiskSize};
        INFOLOG("SetDiskMap, sourceDisk: %s %llu, targetDisk: %s %llu",
            sourceDiskInfo.diskName.c_str(), sourceDiskInfo.diskSize,
            targetDiskInfo.diskName.c_str(), targetDiskInfo.diskSize);
        m_diskMapping[sourceDiskInfo] = targetDiskInfo;
    }
    return;
}

bool OsRestore::DiskMapping()
{
    // Get Current Disk List
    if (!GetTargetDiskInfo()) {
        return false;
    }
    if (!GetSourceDiskInfo()) {
        return false;
    }

    std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> diskMapping {};
    if (DiskMappingByName(diskMapping)) {
        m_diskMapping = diskMapping;
        return true;
    }
    // 按名字匹配失败, 开启按大小匹配
    diskMapping.clear();
    if (DiskMappingBySize(diskMapping)) {
        m_diskMapping = diskMapping;
        m_diskMapBySizeSuccess = true;
        return true;
    }

    // 匹配失败
    diskMapping.clear();
    return false;
}

bool OsRestore::DiskMappingBySize(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping)
{
    INFOLOG("DiskMappingByName failed, start DiskMappingBySize");
    // get source and target info and classify
    if (!ClassifyDisk(m_sourceDiskInfoSet, m_sourceDiskClassifyMap)) {
        ERRLOG("ClassifySourceDisk failed");
        return false;
    }
    if (!ClassifyDisk(m_targetDiskInfoSet, m_targetDiskClassifyMap)) {
        ERRLOG("ClassifyTargetDisk failed");
        return false;
    }
    PrintClassifyDiskInfo();

    // m_sourceDiskClassifyMap  m_targetDiskClassifyMap匹配->m_diskMapping
    for (const auto &pair : m_sourceDiskClassifyMap) {
        std::string diskType = pair.first;
        size_t typeSize = pair.second.size();
        if (m_targetDiskClassifyMap.count(diskType) == 0 || m_targetDiskClassifyMap[diskType].size() < typeSize) {
            ERRLOG("DiskMappingBySize failed: targetDiskClassify has less disk number!");
            return false;
        }
        // pair.second  ...  m_targetDiskClassifyMap[diskType]
        std::vector<DiskInfo> sourceDiskInfos= pair.second;
        std::vector<DiskInfo> targetDiskInfos= m_targetDiskClassifyMap[diskType];
        for (size_t i = 0; i < typeSize; i++) {
            if (sourceDiskInfos[i].diskSize > targetDiskInfos[i].diskSize) {
                ERRLOG("DiskMappingBySize failed: targetDiskClassify has less disk size!"
                    "source name: %s, size: %llu,  target name: %s, size: %llu",
                    sourceDiskInfos[i].diskName.c_str(), sourceDiskInfos[i].diskSize,
                    targetDiskInfos[i].diskName.c_str(), targetDiskInfos[i].diskSize);
                return false;
            }
            diskMapping[sourceDiskInfos[i]] = targetDiskInfos[i];
            INFOLOG("DiskMappingBySize find mapping: %s -> %s", sourceDiskInfos[i].diskName.c_str(),
                targetDiskInfos[i].diskName.c_str());
        }
    }
    return true;
}

bool OsRestore::ClassifyDisk(std::unordered_set<DiskInfo, DiskInfoHash> &diskInfoSet,
    std::unordered_map<std::string, std::vector<DiskInfo>> &diskClassifyMap)
{
    for (const DiskInfo& diskInfo : diskInfoSet) {
        std::string diskLable{};
        if (diskInfo.diskName.size() > NUMBER2) {
            diskLable = diskInfo.diskName.substr(0, NUMBER2);
        } else if (diskInfo.diskName.size() == NUMBER2) {
            diskLable = diskInfo.diskName.substr(0, NUMBER1);
        } else {
            ERRLOG("incorrect disk name: %s", diskInfo.diskName.c_str());
            return false;
        }
        diskClassifyMap[diskLable].push_back(diskInfo);
    }

    auto compare = [](const DiskInfo& diskA, const DiskInfo& diskB) {
        if (diskA.diskSize != diskB.diskSize) {
            return diskA.diskSize > diskB.diskSize;
        } else {
            return diskA.diskName < diskB.diskName;
        }
    };
    for (auto &pair : diskClassifyMap) {
        std::sort(pair.second.begin(), pair.second.end(), compare);
    }
    return true;
}

void OsRestore::PrintClassifyDiskInfo()
{
    INFOLOG("print m_sourceDiskClassifyMap: ");
    for (const auto &pair : m_sourceDiskClassifyMap) {
        INFOLOG("disk type: %s", pair.first.c_str());
        for (const auto &diskInfo : pair.second) {
            INFOLOG("disk name: %s, disk size: %llu", diskInfo.diskName.c_str(), diskInfo.diskSize);
        }
    }

    INFOLOG("print m_targetDiskClassifyMap: ");
    for (const auto &pair : m_targetDiskClassifyMap) {
        INFOLOG("disk type: %s", pair.first.c_str());
        for (const auto &diskInfo : pair.second) {
            INFOLOG("disk name: %s, disk size: %llu", diskInfo.diskName.c_str(), diskInfo.diskSize);
        }
    }
    return;
}

bool OsRestore::GetTargetDiskInfo()
{
    // 执行裸机恢复前，获取目标磁盘名称和大小，存放到set中，如：sda 100G
    m_targetDiskInfoSet.clear();
    std::string cmd = "lsblk -o NAME,TYPE,SIZE -b | awk '$2==\"disk\" {print $1 \":\" $3}'";
    std::vector<std::string> paramList;
    std::vector<std::string> stdOutput;
    std::vector<std::string> errOutput;

    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, stdOutput, errOutput);
    if (ret != 0) {
        ERRLOG("GetSourceDiskInfo failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }

    for (std::string& diskStr : stdOutput) {
        INFOLOG("detected disk and its size: %s", diskStr.c_str());
        std::string::size_type pos = diskStr.find(':');
        std::string diskName = diskStr.substr(0, pos);
        std::string diskSizeStr = diskStr.substr(pos + 1);
        uint64_t diskSize = GetDiskSize(diskSizeStr);
        DiskInfo diskInfo{diskName, diskSize};
        m_targetDiskInfoSet.insert(diskInfo);
    }

    return true;
}

uint64_t OsRestore::GetDiskSize(const std::string& diskSizeStr)
{
    size_t index = 0;
    for (size_t i = 0; i < diskSizeStr.size(); ++i) {
        if (std::isalpha(diskSizeStr[i])) {
            index = i;
            break;
        }
    }
    if (index == 0) {
        return std::stoull(diskSizeStr);
    }
    uint64_t diskSize = std::stoull(diskSizeStr.substr(0, index));
    switch (diskSizeStr[index]) {
        case 'T':
            diskSize *= ONE_TB;
            break;
        case 'G':
            diskSize *= ONE_GB;
            break;
        case 'M':
            diskSize *= ONE_MB;
            break;
        default:
            break;
    }
    return diskSize;
}

bool OsRestore::GetSourceDiskInfo()
{
    m_sourceDiskInfoSet.clear();
    std::string blockDiskPath = PluginUtils::PathJoin(m_sysInfoPath, "blk_disk");
    std::string cmd = "cat " + blockDiskPath;
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("GetSourceDiskInfo failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    for (auto& disk : output) {
        vector<string> diskAndSize;
        INFOLOG("volumeInfo information:%s", disk.c_str());
        boost::split(diskAndSize, disk, boost::is_any_of(" "), boost::token_compress_on);
        if (diskAndSize.size() != NUMBER2) {
            ERRLOG("GetVolumeInfoFromFile failed! this line of mount don't have enough information");
            return false;
        }
        size_t pos = diskAndSize[0].rfind("/");
        std::string diskName = diskAndSize[0].substr(pos + 1);
        uint64_t diskSize = GetDiskSize(diskAndSize[1]);
        INFOLOG("diskAndSize diskName:%s, diskSize:%s, diskSizeInt:%llu",
            diskAndSize[0].c_str(), diskAndSize[1].c_str(), diskSize);
        DiskInfo diskInfo{diskName, diskSize};
        m_sourceDiskInfoSet.insert(diskInfo);
    }
    return true;
}

bool OsRestore::DiskMappingByName(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping)
{
    INFOLOG("source set num %d, target set num %d", m_sourceDiskInfoSet.size(), m_targetDiskInfoSet.size());
    for (const auto& disk : m_sourceDiskInfoSet) {
        INFOLOG("source disk %s %llu", disk.diskName.c_str(), disk.diskSize);
    }
    for (const auto& disk : m_targetDiskInfoSet) {
        INFOLOG("target disk %s %llu", disk.diskName.c_str(), disk.diskSize);
    }

    for (auto sourceDisk : m_sourceDiskInfoSet) {
        bool isMap = false;
        for (auto targetDisk : m_targetDiskInfoSet) {
            INFOLOG("disk mapping by name, source:%s %llu, target:%s %llu",
                sourceDisk.diskName.c_str(), sourceDisk.diskSize, targetDisk.diskName.c_str(), targetDisk.diskSize);
            if (targetDisk.diskName != sourceDisk.diskName) {
                continue;
            }
            isMap = true;
            if (targetDisk.diskSize < sourceDisk.diskSize) {
                return false;
            }
            INFOLOG("disk mapping by name, found mapping, source:%s %llu, target:%s %llu",
                    sourceDisk.diskName.c_str(), sourceDisk.diskSize, targetDisk.diskName.c_str(), targetDisk.diskSize);
            diskMapping[sourceDisk] = targetDisk;
            break;
        }
        if (!isMap) {
            return false;
        }
    }
    return true;
}

bool OsRestore::Start()
{
    INFOLOG("Start OS Recovery!");
    if (!GetTargetDiskInfo()) {
        return false;
    }

    if (!GetSourceDiskInfo()) {
        return false;
    }

    if (!DiskPartitionRestore()) {
        ERRLOG("DiskPartitionRestore Failed!");
        return false;
    }

    if (!FormatFileSystemForBootPartition()) {
        ERRLOG("OSRecovery FormatFileSystemForBootPartition Failed!");
        return false;
    }

    if (!RestoreFsAndMountInfo()) {
        ERRLOG("OSRecovery RestoreFsAndMountInfo Failed!");
        return false;
    }
    
    if (!MountBindSysProcDev()) {
        ERRLOG("OSRecovery MountBindSysProcDev Failed!");
        return false;
    }
    return true;
}


bool OsRestore::DiskPartitionRestore()
{
    INFOLOG("Start DiskPartitionRestore!");
    if (!GetPvInfo()) {
        ERRLOG("get source physical volumes failed");
        return false;
    }
    for (const DiskInfo& sourceDiskInfo : m_sourceDiskInfoSet) {
        if (m_diskMapping.count(sourceDiskInfo) == 0) {
            continue;
        }
        DiskInfo targetDiskInfo = m_diskMapping[sourceDiskInfo];
        if (!DiskPartition(sourceDiskInfo, targetDiskInfo)) {
            ERRLOG("disk partition failed, targetDisk: %s, sourceDisk: %s",
                targetDiskInfo.diskName.c_str(), sourceDiskInfo.diskName.c_str());
            return false;
        }
    }

    if (m_pvInfo.size() != 0 && !LVMRestoration()) {
        ERRLOG("restore lvm info failed");
        return false;
    }
    if (!NonLVMRestoration()) {
        ERRLOG("restore Non-lvm info failed");
        return false;
    }
    return true;
}

bool OsRestore::GetPvInfo()
{
    // 文件里的每一行分别是：pv_name, vg_name, pv_uuid
    std::string pvListFile = PluginUtils::PathJoin(m_lvmInfoPath, PVLIST); // lvm_info/pvlist -> pvlist
    if (!PluginUtils::IsPathExists(pvListFile)) {
        ERRLOG("pvInfoFile not exist, path: %s", pvListFile.c_str());
        return false;
    }
    ifstream file(pvListFile);
    while (file) {
        std::string line;
        getline(file, line);
        if (line.empty()) {
            continue;
        }
        std::stringstream ss(line);
        std::string str;
        std::vector<std::string> vec;
        while (ss >> str) {
            vec.push_back(str);
        }
        if (vec.size() < NUMBER2) {
            ERRLOG("dismiss invalid lvm_info/pvlist entry %s, requre at least 2 colume!", line.c_str());
            continue;
        }
        if (vec.size() == NUMBER2) {
            m_pvInfo[vec[NUMBER0]] = {"", vec[NUMBER1]};
            INFOLOG("get pv info, pv: %s vg: (unbinded) uuid: %s", vec[NUMBER0].c_str(), vec[NUMBER1].c_str());
        } else {
            m_pvInfo[vec[NUMBER0]] = {vec[NUMBER1], vec[NUMBER2]};
            INFOLOG("get pv info, pv: %s vg: %s uuid: %s", vec[NUMBER0].c_str(), vec[NUMBER1].c_str(), vec[NUMBER2].c_str());
        }
    }
    file.close();
    INFOLOG("get pv info success, num of pvs: %d", m_pvInfo.size());
    return true;
}

bool OsRestore::DiskPartition(const DiskInfo& sourceDisk, const DiskInfo& targetDisk)
{
    // 找到副本中磁盘分区信息文件partitions_sdx,恢复到目标磁盘
    std::string sourceDiskName = sourceDisk.diskName;
    uint64_t sourceDiskSize = sourceDisk.diskSize;
    sourceDiskSize = GetDiskSize(std::to_string(sourceDiskSize));
    std::string targetDiskName = targetDisk.diskName;
    uint64_t targetDiskSize = targetDisk.diskSize;
    if (targetDiskSize < sourceDiskSize) {
        ERRLOG("Target disk size(%llu) is smaller than source disk size(%llu)", targetDiskSize, sourceDiskSize);
        return false;
    }

    std::string sourcePartitionsFile = PluginUtils::PathJoin(m_sysInfoPath,
        PARTITIONS_INFO_PREFIX + sourceDiskName);
    std::string sourcePartitionsFileForDos = PluginUtils::PathJoin(m_sysInfoPath,
        PARTITIONS_INFO_PREFIX_DOS + sourceDiskName);
    std::string sourcePartitionsFileForGPT = PluginUtils::PathJoin(m_sysInfoPath,
        PARTITIONS_INFO_PREFIX_GPT + sourceDiskName);
    std::string restorePartitionCmd;
    std::string nukeEmbeddingCmd;  // 用于清除磁盘分区信息
    struct stat st;
    if (stat(sourcePartitionsFileForDos.c_str(), &st) == 0) {
        restorePartitionCmd = "sfdisk --force /dev/" + targetDiskName + " < " + sourcePartitionsFileForDos;
        nukeEmbeddingCmd = "dd if=/dev/zero of=/dev/" + targetDiskName + " seek=0 count=2048";
    } else if (stat(sourcePartitionsFileForGPT.c_str(), &st) == 0) {
        restorePartitionCmd = "sgdisk --load-backup=" + sourcePartitionsFileForGPT + " /dev/" + targetDiskName;
        nukeEmbeddingCmd = "sgdisk -g -o /dev/" + targetDiskName;
    } else if (stat(sourcePartitionsFile.c_str(), &st) == 0) {
        // 升级兼容性，不带dos开头的同样用sfdisk恢复分区
        restorePartitionCmd = "sfdisk --force /dev/" + targetDiskName + " < " + sourcePartitionsFile;
        nukeEmbeddingCmd = "dd if=/dev/zero of=/dev/" + targetDiskName + " seek=0 count=2048";
    } else {
        // 未找到分区信息文件，说明磁盘分区类型不支持或者磁盘没有分区
        INFOLOG("skip format partition, source(%s) and target(%s)", sourceDiskName.c_str(), targetDiskName.c_str());
        return true;
    }
    // 保留以下判断，保证旧版本的兼容性
    if (st.st_size == 0) {
        INFOLOG("skip format partition, disk %s.", targetDiskName.c_str());
        return true;
    }
    if (RunCommand(nukeEmbeddingCmd) != 0 || RunCommand(restorePartitionCmd) != 0) {
        ERRLOG(
            "restore partition failed, targetDisk: %s, sourceDisk: %s", targetDiskName.c_str(), sourceDiskName.c_str());
        return false;
    }
    return true;
}

bool OsRestore::LVMRestoration()
{
    INFOLOG("Enter LvmRestoration!");
    if (m_diskMapping.empty()) {
        ERRLOG("No disk designated");
        return false;
    }

    if (!GetVgNames() || !GetLvInfo()) {
        return false;
    }

    if (!m_vgNames.empty()) {
        // 根据map匹配信息,替换vg文件中的相关信息
        if (!MakeVgBeforeGenerate()) {
            return false;
        }
        for (const std::string& vgName : m_vgNames) {
            if (!ReplaceVgFile(vgName)) {
                ERRLOG("replace vg file failed, vg: %s", vgName.c_str());
                return false;
            }
        }
    }

    if (!m_pvInfo.empty() && !ReplaceAndCreatePv()) {
        return false;
    }

    for (const std::string& vgName : m_vgNames) {
        if (!CreateVolumeGroup(vgName) || !ActivateVolumeGroup(vgName)) { // 如果存在就激活卷组
            return false;
        }
    }

    if (!FormatFileSystemForLvs()) {
        ERRLOG("format filesystem for logical volume failed");
        return false;
    }

    INFOLOG("lvm restoration success!");
    return true;
}

bool OsRestore::MakeVgBeforeGenerate()
{
    if (!PluginUtils::Remove(m_replaceVgDir)) {
        ERRLOG("Clean m_replaceVgDir: %s failed", m_replaceVgDir.c_str());
        return false;
    }
    if (!PluginUtils::CreateDirectory(m_replaceVgDir)) {
        ERRLOG("Create m_replaceVgDir: %s failed", m_replaceVgDir.c_str());
        return false;
    }
    return true;
}

bool OsRestore::ReplaceVgFile(const std::string &vgName)
{
    std::string sourceVgFile = PluginUtils::PathJoin(m_lvmInfoPath, vgName);
    std::string targetVgFile = PluginUtils::PathJoin(m_replaceVgDir, vgName);
    if (!PluginUtils::IsPathExists(sourceVgFile)) {
        ERRLOG("vg backup file: %s not exist", sourceVgFile.c_str());
        return false;
    }
    std::ifstream inFile(sourceVgFile);
    std::ofstream outFile(targetVgFile, std::ios::app);

    if (!inFile.is_open()) {
        ERRLOG("open source vg: %s file: %s failed", vgName.c_str(), sourceVgFile.c_str());
        return false;
    }
    if (!outFile.is_open()) {
        ERRLOG("open target vg: %s file: %s failed", vgName.c_str(), targetVgFile.c_str());
        return false;
    }

    INFOLOG("Start to replace vgname: %s file ", vgName.c_str());
    std::string line;
    uint64_t lineCount = 0;
    while (std::getline(inFile, line)) {
        // 对每一行进行设备名称检测替换
        DBGLOG("get sourceVgFile line: %s", line.c_str());
        ++lineCount;
        std::string modifiedLine = line;
        for (const auto& diskMap : m_diskMapping) {
            std::string sourceDiskName = diskMap.first.diskName;
            std::string targetDiskName = diskMap.second.diskName;
            size_t pos = line.find(sourceDiskName);
            if (pos != std::string::npos) {
            // 如果找到了匹配的设备名称，则进行替换
                INFOLOG("SourceVgFile line find sourceDisk: %s, line: %llu, pos: %zu ",
                    sourceDiskName.c_str(), lineCount, pos);
                modifiedLine.replace(pos, sourceDiskName.size(), targetDiskName);
                break;
            }
        }
        // 写入修改后的内容到输出文件
        outFile << modifiedLine << std::endl;
    }

    inFile.close();
    outFile.close();
    return true;
}

bool OsRestore::ReplaceAndCreatePv()
{
    INFOLOG("Start ReplaceAndCreatePv!");
    for (auto it = m_diskMapping.begin(); it != m_diskMapping.end(); ++it) {
        std::string sourceDiskName = (it->first).diskName;
        std::string targetDiskName = (it->second).diskName;
        for (auto iter = m_pvInfo.begin(); iter != m_pvInfo.end(); ++iter) {
            std::string sourcePvName = iter->first;
            size_t pos = sourcePvName.find(sourceDiskName);
            if (pos == std::string::npos) {
                continue;
            }

            std::string targetPvName{sourcePvName};
            std::string pvUuid = m_pvInfo[sourcePvName][1];
            targetPvName.replace(pos, sourceDiskName.size(), targetDiskName);
            INFOLOG("sourcePvName: %s -> targetPvName: %s", sourcePvName.c_str(), targetPvName.c_str());
            if (!CreatePhysicalVolume(sourcePvName, targetPvName, pvUuid)) {
                ERRLOG("pvcreate failed");
                return false;
            }
        }
    }
    return true;
}

bool OsRestore::CreatePhysicalVolume(const std::string& sourcePvName,
    const std::string& targetPvName, const std::string& pvUuid)
{
    std::string vgName = m_pvInfo[sourcePvName][0];
    std::string lvmBackupFile = PluginUtils::PathJoin(m_replaceVgDir, vgName);
    if (!vgName.empty() && !PluginUtils::IsPathExists(lvmBackupFile)) {
        ERRLOG("lvm backup file: %s not exist", lvmBackupFile.c_str());
        return false;
    }
    std::string cmd = "pvcreate --uuid " + pvUuid + " --restorefile " + lvmBackupFile + " " + targetPvName;
    if (vgName.empty()) {
        cmd = "pvcreate " + targetPvName;
    }
    INFOLOG("pvcreate cmd: %s", cmd.c_str());
    // 创建pv卷前，先清除磁盘分区信息，防止磁盘之前已分区过存在信息残留
    if (!WipeDisk(targetPvName) || RunCommand(cmd) != 0) {
        ERRLOG("pvcreate(%s) failed!", targetPvName.c_str());
        return false;
    }
    return true;
}

bool OsRestore::GetVgNames()
{
    std::string vgListFile = PluginUtils::PathJoin(m_lvmInfoPath, VGLIST);
    if (!PluginUtils::IsPathExists(vgListFile)) {
        ERRLOG("vgNameFile not exist, path: %s", vgListFile.c_str());
        return false;
    }
    ifstream file(vgListFile);
    while (file) {
        std::string line;
        getline(file, line);
        if (line.empty()) {
            continue;
        }
        m_vgNames.emplace_back(line);
        INFOLOG("vg name: %s", line.c_str());
    }
    file.close();
    return true;
}

bool OsRestore::GetLvInfo()
{
    std::string lvFsTypeFile = PluginUtils::PathJoin(m_sysInfoPath, LV_FSTYPE);
    if (!PluginUtils::IsPathExists(lvFsTypeFile)) {
        ERRLOG("file lv_fsType not exist, path: %s", lvFsTypeFile.c_str());
        return false;
    }
    ifstream file(lvFsTypeFile);
    while (file) {
        std::string line;
        getline(file, line);
        if (line.empty()) {
            continue;
        }
        std::stringstream ss(line);
        std::string str;
        std::vector<std::string> vec;
        while (ss >> str) {
            vec.push_back(str);
        }
        if (vec.size() < NUMBER2) {
            ERRLOG("dismiss invalid lvm_info/lv_fstype entry %s, requre 2 colume!", line.c_str());
            continue;
        }
        m_lvFsType[vec[NUMBER0]] = vec[NUMBER1];
        INFOLOG("get lv info, lv: %s fsType: %s", vec[NUMBER0].c_str(), vec[NUMBER1].c_str());
    }
    file.close();
    INFOLOG("get lv info success, num of lvs: %d", m_lvFsType.size());
    return true;
}

bool OsRestore::CreateVolumeGroup(const std::string& vgName)
{
    INFOLOG("Enter CreateVolumeGroup, vgName: %s", vgName.c_str());
    std::string lvmBackupFile = PluginUtils::PathJoin(m_replaceVgDir, vgName);

    std::string cmd = "vgcfgrestore -f " + lvmBackupFile + " " + vgName;
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("vgcreate cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("vgcreate failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool OsRestore::ActivateVolumeGroup(const std::string& vgName)
{
    INFOLOG("Enter ActivateVolumeGroup, vgName: %s", vgName.c_str());
    std::string cmd = "vgchange -a y " + vgName;
    vector<std::string> output;
    vector<std::string> errOutput;
    INFOLOG("VG activate cmd: %s", cmd.c_str());
    if (runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput) != 0) {
        ERRLOG("VG activate failed!");
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool OsRestore::FormatFileSystemForLvs()
{
    INFOLOG("Enter FormatFileSystemForLvs");
    for (const auto& pair : m_lvFsType) {
        std::string cmd;
        if (pair.second == "swap") {
            cmd = "echo y | mkswap " + pair.first;
        } else if (pair.second == "xfs") {
            // for compatible issue
            cmd = "echo y | mkfs.xfs -f -m bigtime=0,inobtcount=0,reflink=0 -i sparse=0 " + pair.first;
        } else if (pair.second == "btrfs") {
            cmd = "echo y | mkfs.btrfs -f " + pair.first;
        } else {
            cmd = "echo y | mkfs -t " + pair.second + " " + pair.first;
        }
        INFOLOG("using cmd %s", cmd.c_str());
        vector<std::string> output;
        vector<std::string> errOutput;
        int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
        if (ret != 0) {
            ERRLOG("format fileSystem for lv failed! %d", ret);
            for (auto msg : errOutput) {
                WARNLOG("errmsg : %s", msg.c_str());
            }
        }
    }
    INFOLOG("Exit FormatFileSystemForLvs");
    return true;
}

bool OsRestore::NonLVMRestoration()
{
    INFOLOG("Enter NonLVM Restoration! ");
    if (!GetNonLvInfo()) {
        ERRLOG("Get non-lvm info failed! ");
        return false;
    }

    if (!FormatFileSystemForNonLvs()) {
        ERRLOG("Format FileSystem for Non-Lvs failed! ");
        return false;
    }
    return true;
}

bool OsRestore::GetNonLvInfo()
{
    // m_nonLvFsType
    // NON_LV_FSTYPE
    INFOLOG("Enter Get Non-Lv Information! ");
    bool ret = true;
    std::string lvNonFsTypeFile = PluginUtils::PathJoin(m_sysInfoPath, NON_LV_FSTYPE);
    if (!PluginUtils::IsPathExists(lvNonFsTypeFile)) {
        ERRLOG("file non_lv_fsType not exist, path: %s", lvNonFsTypeFile.c_str());
        return false;
    }
    ifstream file(lvNonFsTypeFile);
    while (file) {
        std::string volumeInfo;
        getline(file, volumeInfo);
        if (volumeInfo.empty()) {
            continue;
        }
        vector<string> volume;
        boost::split(volume, volumeInfo, boost::is_any_of(" "), boost::token_compress_on);
        // 创文件系统但未挂载，也应该创建文件系统而不返回
        ReplaceNonLvName(volume[NUMBER0]);
        if (volume.size() == NUMBER3) {
            OSVolumeInfo nonLvmInfo;
            nonLvmInfo.volumeName = volume[NUMBER0];
            nonLvmInfo.fsType= volume[NUMBER1];
            nonLvmInfo.volumeUUID = volume[NUMBER2];
            m_nonLvmInfo.push_back(nonLvmInfo);
            INFOLOG("Get non-logical volume info, name: %s, fsType: %s, uuid: %s", volume[NUMBER0].c_str(),
                volume[NUMBER1].c_str(), volume[NUMBER2].c_str());
        }
        // m_nonLvmInfo
        if (volume.size() == NUMBER4) {
            OSVolumeInfo nonLvmInfo;
            nonLvmInfo.volumeName = volume[NUMBER0];
            nonLvmInfo.fsType= volume[NUMBER1];
            nonLvmInfo.mountPoint = volume[NUMBER2];
            nonLvmInfo.volumeUUID = volume[NUMBER3];
            m_nonLvmInfo.push_back(nonLvmInfo);
            INFOLOG("Get non-logical volume info, name: %s, fsType: %s, mountPoint: %s, uuid: %s", volume[NUMBER0].c_str(),
                volume[NUMBER1].c_str(), volume[NUMBER2].c_str(), volume[NUMBER3].c_str());
        }
    }
    file.close();
    return ret;
}
void OsRestore::ReplaceNonLvName(std::string &volumeName)
{
    std::string sourceVolumeName = volumeName;
    for (auto it = m_diskMapping.begin(); it != m_diskMapping.end(); ++it) {
            std::string sourceDiskName = (it->first).diskName;
            std::string targetDiskName = (it->second).diskName;

            size_t pos = sourceVolumeName.find(sourceDiskName);
            if (pos == std::string::npos) {
                continue;
            }
            volumeName.replace(pos, sourceDiskName.size(), targetDiskName);
            INFOLOG("Replace: sourceNonLvName: %s -> targetNonLvName: %s",
                sourceVolumeName.c_str(), volumeName.c_str());
            break;
    }
    return;
}

bool OsRestore::FormatFileSystemForNonLvs()
{
    // ${sysInfoPath}/non_lv_fsType
    // NON_LV_FSTYPE
    // m_nonLvFsType  m_nonLvmInfo
    INFOLOG("Enter FormatFileSystem for Non-Lvs ");
    for (const OSVolumeInfo& nonLvmInfo : m_nonLvmInfo) {
        if (!MkfsAndAddUidForLvs(nonLvmInfo)) {
            ERRLOG("device: %s mkfs failed! ", nonLvmInfo.volumeName.c_str());
            return false;
        }
    }
    INFOLOG("Exit FormatFileSystemForNonLvs");
    return true;
}

bool OsRestore::MkfsAndAddUidForLvs(const OSVolumeInfo& volumeInfo)
{
    std::string cmd;
    std::string path = volumeInfo.mountPoint;
    if (volumeInfo.fsType == "swap") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkswap -f -U " + volumeInfo.volumeUUID + " " +volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkswap -f " + volumeInfo.volumeName;
        }
    } else if (volumeInfo.fsType == "btrfs") {
        DealBtrfsCase(volumeInfo, cmd);
    } else if (volumeInfo.fsType == "xfs") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs.xfs -f -m uuid=" + volumeInfo.volumeUUID +
                ",bigtime=0,inobtcount=0,reflink=0 -i sparse=0 " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs.xfs -f -m bigtime=0,inobtcount=0,reflink=0 -i sparse=0 " + volumeInfo.volumeName;
        }
    } else if (volumeInfo.fsType == "ext4") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs.ext4 -O ^metadata_csum,^64bit -U "
                + volumeInfo.volumeUUID + " " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs.ext4 -O ^metadata_csum,^64bit " + volumeInfo.volumeName;
        }
    } else if (volumeInfo.fsType == "ext2" || volumeInfo.fsType == "ext3") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " -U "
                + volumeInfo.volumeUUID + " " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " " + volumeInfo.volumeName;
        }
    } else if (volumeInfo.fsType == "vfat") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " -i "
                + FormatUuidToDosSerial(volumeInfo.volumeUUID) + " " + volumeInfo.volumeName;
        } else {
            // 存在用户自己创建的vfat分区，没有uuid，这种情况下，不需要指定-i参数
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " " + volumeInfo.volumeName;
        }
    }
    if (RunCommand(cmd) != SUCCESS) {
        return false;
    }
    return true;
}

void OsRestore::DealBtrfsCase(const OSVolumeInfo& volumeInfo, std::string& cmd)
{
    // mkfs.btrfs使用相同uuid，mkfs报错，先清理该文件系统
    INFOLOG("Enter btrfs case! ");
    std::string btrWipeCmd = "wipefs -a " + volumeInfo.volumeName;
    if (RunCommand(btrWipeCmd) != SUCCESS) {
        ERRLOG("wipefs for btrfs failed");
    }
    if (!volumeInfo.volumeUUID.empty()) {
        cmd = "echo y | mkfs.btrfs -f -U " + volumeInfo.volumeUUID + " " + volumeInfo.volumeName;
    } else {
        cmd = "echo y | mkfs.btrfs -f " + volumeInfo.volumeName;
    }
    return;
}

bool OsRestore::FormatFileSystemForBootPartition()
{
    if (!GetBootVolumeInfoFromFile()) {
        ERRLOG("Get boot info failed!");
        return false;
    }
    if (m_bootVolumeInfo.mountPoint == "/boot") {
        INFOLOG("mkfs for boot volume, name: %s, uuid: %s, fsType: %s, mount: %s",
            m_bootVolumeInfo.volumeName.c_str(), m_bootVolumeInfo.volumeUUID.c_str(),
            m_bootVolumeInfo.fsType.c_str(), m_bootVolumeInfo.mountPoint.c_str());
        if (!MkfsAndMountBoot(m_bootVolumeInfo)) {
            return false;
        }
    }
    if (m_bootEfiVolumeInfo.mountPoint == "/boot/efi") {
        INFOLOG("mkfs for efi volume, name: %s, uuid: %s, fsType: %s, mount: %s",
            m_bootEfiVolumeInfo.volumeName.c_str(), m_bootEfiVolumeInfo.volumeUUID.c_str(),
            m_bootEfiVolumeInfo.fsType.c_str(), m_bootEfiVolumeInfo.mountPoint.c_str());
        if (!MkfsAndMountBoot(m_bootEfiVolumeInfo)) {
            return false;
        }
    }
    return true;
}

bool OsRestore::GetBootVolumeInfoFromFile()
{
    bool ret = true;
    std::string bootInfoFile = PluginUtils::PathJoin(m_sysInfoPath, "boot_part");
    ifstream file(bootInfoFile);
    while (file) {
        std::string volumeInfo;
        getline(file, volumeInfo);
        if (volumeInfo.empty()) {
            continue;
        }
        vector<string> volume;
        boost::split(volume, volumeInfo, boost::is_any_of(" "), boost::token_compress_on);
        if (volume.size() != BLK_DISK_FILE_COLUMN_LENGTH) {
            ERRLOG("line(%s) invalid!", volumeInfo.c_str());
            ret = false;
            break;
        }
        ReplaceNonLvName(volume[NUMBER0]);
        if (volume[NUMBER2] == "/boot") {
            m_bootVolumeInfo.volumeName = volume[NUMBER0];
            m_bootVolumeInfo.fsType= volume[NUMBER1];
            m_bootVolumeInfo.mountPoint = volume[NUMBER2];
            m_bootVolumeInfo.volumeUUID = volume[NUMBER3];
        }
        if (volume[NUMBER2] == "/boot/efi") {
            m_bootEfiVolumeInfo.volumeName = volume[NUMBER0];
            m_bootEfiVolumeInfo.fsType= volume[NUMBER1];
            m_bootEfiVolumeInfo.mountPoint = volume[NUMBER2];
            m_bootEfiVolumeInfo.volumeUUID = volume[NUMBER3];
        }
        INFOLOG("Get volume info, name: %s, fsType: %s, mountPoint: %s, uuid: %s", volume[NUMBER0].c_str(),
            volume[NUMBER1].c_str(), volume[NUMBER2].c_str(), volume[NUMBER3].c_str());
    }
    file.close();
    return ret;
}

bool OsRestore::MkfsAndMountBoot(const OSVolumeInfo& volumeInfo)
{
    std::string cmd;
    std::string path = volumeInfo.mountPoint;
    if (volumeInfo.fsType == "xfs") {
        // for compatible issue
        // bigtime 4.8+
        // inobtcount,bigtime 5.1+
        // redhat7.4+uefi not surpport sparse
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs.xfs -f -m uuid=" + volumeInfo.volumeUUID +
                ",bigtime=0,inobtcount=0,reflink=0 -i sparse=0 " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs.xfs -f -m bigtime=0,inobtcount=0,reflink=0 -i sparse=0 " + volumeInfo.volumeName;
        }
    }
    if (volumeInfo.fsType == "ext4") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs.ext4 -O ^metadata_csum,^64bit -U "
                + volumeInfo.volumeUUID + " " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs.ext4 -O ^metadata_csum,^64bit " + volumeInfo.volumeName;
        }
    }
    if (volumeInfo.fsType == "ext2" || volumeInfo.fsType == "ext3") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " -U "
                + volumeInfo.volumeUUID + " " + volumeInfo.volumeName;
        } else {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " " + volumeInfo.volumeName;
        }
    }
    if (volumeInfo.fsType == "vfat") {
        if (!volumeInfo.volumeUUID.empty()) {
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " -i "
                + FormatUuidToDosSerial(volumeInfo.volumeUUID) + " " + volumeInfo.volumeName;
        } else {
            // 存在用户自己创建的vfat分区，没有uuid，这种情况下，不需要指定-i参数
            cmd = "echo y | mkfs -t " + volumeInfo.fsType + " " + volumeInfo.volumeName;
        }
    }
    if (RunCommand(cmd) != SUCCESS) {
        return false;
    }
    return true;
}

std::string OsRestore::FormatUuidToDosSerial(const std::string& vfatUuid) const
{
    if (vfatUuid.length() == NUMBER9 && vfatUuid[NUMBER4] == '-') {
        std::string dosSerial = vfatUuid.substr(NUMBER0, NUMBER4) + vfatUuid.substr(NUMBER5, NUMBER4);
        INFOLOG("change vfat UUID to DOS serial number %s => %s", vfatUuid.c_str(), dosSerial.c_str());
        return dosSerial;
    }
    return vfatUuid;
}

bool OsRestore::RestoreFsAndMountInfo()
{
    INFOLOG("Enter RestoreFsAndMountInfo!");
    std::string fstabFile = PluginUtils::PathJoin(m_sysInfoPath, FSTAB);
    std::string tmpFstabFile = PluginUtils::PathJoin(m_sysInfoPath, TMPFSTAB);
    std::string newFstabDir = "/tmp/bmr_mount_" + m_config.jobId;
    if (!PluginUtils::CreateDirectory(newFstabDir)) {
        ERRLOG("CreateDirectory newFstabDir failed!");
        return false;
    }
    std::string newFstabFile = PluginUtils::PathJoin(newFstabDir, "new_fstab");
    if (!GetFstabMountPoint(newFstabFile)) {
        ERRLOG("GetFstabMountPoint failed ");
        return false;
    }
    GenerateNewFstabFile(newFstabFile);

    std::string mountedInfoFile = PluginUtils::PathJoin(m_sysInfoPath, MOUNTED_INFO);
    if (!PluginUtils::IsPathExists(fstabFile) || !PluginUtils::IsPathExists(mountedInfoFile)) {
        ERRLOG("fstab or mounted_info file not exist %s %s", fstabFile.c_str(), mountedInfoFile.c_str());
        return false;
    }
    std::string initDirCmd = "cat " + newFstabFile +
                      "| grep -E \"xfs|ext2|ext3|ext4|btrfs|vfat|fat32\" | awk '{print $1,\"/bmr_os_restore" +
                      "\"$2,$3,$4,$5,$6}' | awk '{print $2}' | xargs -i mkdir -p {}";
    INFOLOG("ready to init fstab mount point %s", initDirCmd.c_str());
    RunCommand(initDirCmd);

    // 把系统和boot分区临时挂载到/bmr_os_restore目录下
    std::string mountNonBindCmd = "cat " + newFstabFile +
                      " | grep -E \"xfs|ext2|ext3|ext4|btrfs|vfat|fat32\" | awk '{if($4 !~ \"bind\")" +
                      "{print $1,\"/bmr_os_restore" +
                      "\"$2,$3,$4\",X-mount.mkdir\",$5,$6}}' >> " + FSTAB_PATH + ";mount -a;";
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, mountNonBindCmd, {}, output, errOutput);
    INFOLOG("RestoreNotBindMountInfo cmd: %s", mountNonBindCmd.c_str());
    if (ret != 0) {
        ERRLOG("RestoreNotBindMountInfo failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    if (!RestoreBindMountInfo(newFstabFile)) {
        return false;
    }
    return true;
}

bool OsRestore::RestoreBindMountInfo(std::string& newFstabFile)
{
    // 创建bind源目录并重新挂载
    std::string mountBindCmd = "cat " + newFstabFile +
                      " | grep -E \"xfs|ext2|ext3|ext4|btrfs|vfat|fat32\" | awk '{if($4 ~ \"bind\")" +
                      " {system(\"mkdir -p /bmr_os_restore\" $1) ; " +
                      "print \"/bmr_os_restore" +"\"$1,\"/bmr_os_restore" +
                      "\"$2,$3,$4\",X-mount.mkdir\",$5,$6}}' " +
                      " >> " + FSTAB_PATH + ";mount -a;";
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, mountBindCmd, {}, output, errOutput);
    INFOLOG("RestoreBindMountInfo cmd: %s", mountBindCmd.c_str());
    if (ret != 0) {
        ERRLOG("RestoreBindMountInfo failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool OsRestore::GetFstabMountPoint(std::string& newFstabFile)
{
    INFOLOG("Enter GetFstabMountPoint to set!");
    INFOLOG("newFstabFile full path is : %s", newFstabFile.c_str());
    std::string fstabFile = PluginUtils::PathJoin(m_sysInfoPath, FSTAB);
    // 创建newfstab文件，将FSTAB写入
    std::ofstream newFile(newFstabFile);
    if (!newFile.is_open()) {
        ERRLOG("Failed to create file: %s", newFstabFile.c_str());
        return false;
    }
    newFile.close();
    
    if (!PluginUtils::IsPathExists(fstabFile)) {
        ERRLOG("fstab is not existed");
        return false;
    }
    ifstream file(fstabFile);
    std::ofstream destFile(newFstabFile, std::ios::app);
    while (file) {
        std::string mountInfo;
        getline(file, mountInfo);
        destFile << mountInfo << std::endl;
        INFOLOG("Get mountpoint info line: %s", mountInfo.c_str());
        if (mountInfo.empty()) {
            continue;
        }
        vector<string> mount;
        boost::split(mount, mountInfo, boost::is_any_of(" "), boost::token_compress_on);
        if (mount[NUMBER0] == "#" || mount.size() < NUMBER2) {
            DBGLOG("line(%s) invalid!", mountInfo.c_str());
            continue;
        }
        m_fstabMountPoint.emplace(mount[NUMBER1]);
        INFOLOG("Get mountpoint: %s", mount[NUMBER1].c_str());
    }
    file.close();
    destFile.close();
    return true;
}

void OsRestore::GenerateNewFstabFile(std::string& newFstabFile)
{
    INFOLOG("Enter GenerateNewFstabFile ");
    std::string tmpFstabFile = PluginUtils::PathJoin(m_sysInfoPath, TMPFSTAB);

    ifstream file(tmpFstabFile);
    std::ofstream newFile(newFstabFile, std::ios::app);
    while (file) {
        std::string mountInfo;
        getline(file, mountInfo);
        if (mountInfo.empty()) {
            continue;
        }

        vector<string> mount;
        boost::split(mount, mountInfo, boost::is_any_of(" "), boost::token_compress_on);
        if (mount[NUMBER0] == "#" || mount.size() < NUMBER2) {
            DBGLOG("line(%s) invalid!", mountInfo.c_str());
            continue;
        }
        if (m_fstabMountPoint.find(mount[NUMBER1]) == m_fstabMountPoint.end()) {
            // push this line mountinfo -> newFstabFile
            newFile << mountInfo << std::endl;
            INFOLOG("push mountinfo line %s to dir: %s", mountInfo.c_str(), newFstabFile.c_str());
        }
    }
    file.close();
    newFile.close();
    return;
}

bool OsRestore::MountBindSysProcDev()
{
    INFOLOG("Enter MountBindSysProcDev");
    std::string cmd = "mkdir -p /bmr_os_restore/sys && mount --bind /sys /bmr_os_restore/sys \
        && mkdir -p /bmr_os_restore/proc && mount --bind /proc /bmr_os_restore/proc \
        && mkdir -p /bmr_os_restore/dev && mount --bind /dev /bmr_os_restore/dev \
        && mkdir -p /bmr_os_restore/run && mount --bind /run /bmr_os_restore/run";
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("MountBindSysProcDev cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("MountBindSysProcDev failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

int OsRestore::CleanFstabAndMountAfterFailed()
{
    INFOLOG("Enter clean fstab and mountpoint after failed! ");
    std::vector<std::string> output;
    std::vector<std::string> errOutput;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    
    std::string cmd = "sh ?/bin/CleanFstabAndMount.sh";
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, errOutput);
    if (ret != Module::SUCCESS) {
        ERRLOG("excute clean fstab and mountpoint failed! ");
        PluginUtils::LogCmdExecuteError(ret, output, errOutput);
        return ret;
    }
    INFOLOG("clean fstab and mountpoint success! ");
    return Module::SUCCESS;
}

bool OsRestore::OSRestoreRepairGrub()
{
    if (!RebuildInitramfs()) {
        return false;
    }

    if (!GenerateNewGrubCfg()) {
        ERRLOG("OSRecovery GenerateNewGrubCfg Failed!");
        return false;
    }
    PluginUtils::CreateDirectory("/tmp");
    std::string bmrSucceedFlagFilePath = PluginUtils::PathJoin("/tmp", m_config.jobId + BMR_SUCCEED_FLAG_SUFFIX);
    INFOLOG("PRE-OSRestoreRepairGrub's bmrSucceedFlagFilePath: %s", bmrSucceedFlagFilePath.c_str());
    std::ofstream bmrSucceedFlagFile(bmrSucceedFlagFilePath);
    bmrSucceedFlagFile.close();
    return true;
}

bool OsRestore::RebuildInitramfs()
{
    INFOLOG("Enter RebuildInitramfs");
    // For now, rebuild initramfs, trying to solve the problem
    std::string chrootCmd{"chroot /bmr_os_restore"};
    std::string cmd = chrootCmd + " dracut -a resume --regenerate-all --force";
    if (RunCommand(cmd) != SUCCESS) {
        WARNLOG("RebuildInitramfs failed!");
        return false;
    }
    return true;
}

bool OsRestore::GenerateNewGrubCfg()
{
    INFOLOG("Enter GenerateNewGrubCfg");
    // If EFI_SYSTEM_DIR is exist(it's efi system), we don't need to generate grub.cfg.
    // Install legacy GRUB (core.img) as ESP do not need recovery
    INFOLOG("Generate Legacy GRUB config for %s", m_bootVolumeInfo.volumeName.c_str());
    std::string grubInstallDevice = "/dev/sda";
    // 找不到boot所在的设备，也尝试往下走，/boot所在的设备有可能已经安装过grub2了
    SearchBootDisk(grubInstallDevice);
    INFOLOG("grubInstallDevice: %s", grubInstallDevice.c_str());
    // 检查原操作系统grub修复命令(grub2-install, grub-install, grub-install.unsupported)
    PluginUtils::CreateDirectory("/bmr_os_restore/tmp");
    if (UseGrub2Install()) {
        if (!PluginUtils::IsPathExists(EFI_SYSTEM_DIR)) {
            std::string cmdInstallGrub = "grub2-install --skip-fs-probe --root-directory=/bmr_os_restore --no-floppy " +
                                            grubInstallDevice;
            if (RunCommand(cmdInstallGrub) != 0) {
                return false;
            }
        }
        std::string cmdChangeRootMakeConfig = "chroot /bmr_os_restore grub2-mkconfig -o " + GRUB_CONFIG;
        std::string cmdAddSelinuxToGrubConfig = "sed -i \"/vmlinuz/s/$/ selinux=0/\" /bmr_os_restore" + GRUB_CONFIG;
        if (RunCommand(cmdChangeRootMakeConfig) != 0 || RunCommand(cmdAddSelinuxToGrubConfig) != 0) {
            return false;
        }
        return true;
    } else if (UseGrubInstall()) {
        std::string grubCmd = "chroot /bmr_os_restore /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install " +
            grubInstallDevice + " --no-floppy'";
        std::string AddSelinuxToGrubConfig1 = "sed -i \"/vmlinuz/s/$/ selinux=0/\" /bmr_os_restore" + OLD_GRUB_CONFIG1;
        std::string AddSelinuxToGrubConfig2 = "sed -i \"/vmlinuz/s/$/ selinux=0/\" /bmr_os_restore" + OLD_GRUB_CONFIG2;
        if (RunCommand(grubCmd) != 0 ||
            ((RunCommand(AddSelinuxToGrubConfig1) != 0) && (RunCommand(AddSelinuxToGrubConfig2) != 0))) {
            ERRLOG("run grub-install failed!");
            return false;
        }
        return true;
    } else {
        std::string grubCmd = "chroot /bmr_os_restore /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install.unsupported " +
                                grubInstallDevice + " --no-floppy'";
        return RunCommand(grubCmd) == 0;
    }
}

void OsRestore::SearchBootDisk(std::string& grubInstallDevice) const
{
    std::string bootPath = "/bmr_os_restore/boot";
    std::string cmdSearchBootDisk = "lsblk -p -o NAME,MOUNTPOINT,PKNAME | awk '$2==\"" + bootPath + "\"{print $3}'";
    INFOLOG("SearchBootDisk cmd: %s", cmdSearchBootDisk.c_str());
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmdSearchBootDisk, {}, output, errOutput);
    if (ret != 0) {
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return;
    }
    if (output.size() == 0) {
        WARNLOG("can not find /boot path from lsblk, try to find /!");
        cmdSearchBootDisk = "lsblk -p -o NAME,MOUNTPOINT,PKNAME | awk '$2==\"/bmr_os_restore\"{print $3}'";
        if (runShellCmdWithOutput(INFO, MODULE, 0, cmdSearchBootDisk, {}, output, errOutput) != 0 ||
            output.size() == 0) {
            WARNLOG("can not find /boot path from lsblk, use default device(%s)!", grubInstallDevice.c_str());
            return;
        }
    }
    grubInstallDevice = output[0];
    // lsblk获取到的设备名/dev/nvme0n1，/dev/nvme0n1p3，需特殊处理
    if (CheckAndDealNvmeDisk(grubInstallDevice)) {
        return;
    }
    // lsblk获取到的设备名可能包含数字，例如/dev/sda1，需要获取父设备名字/dev/sda
    while (!grubInstallDevice.empty() && isdigit(grubInstallDevice.back())) {
        grubInstallDevice = grubInstallDevice.substr(0, grubInstallDevice.size() - 1);
    }
}

bool OsRestore::CheckAndDealNvmeDisk(std::string& grubInstallDevice) const
{
    if (grubInstallDevice.size() < NUMBER9) {
        return false;
    }
    std::string typeStr = grubInstallDevice.substr(NUMBER5, NUMBER4);
    if (typeStr == "nvme") {
        if (grubInstallDevice[grubInstallDevice.size() - NUMBER2] == 'p') {
            grubInstallDevice = grubInstallDevice.substr(0, grubInstallDevice.size() - NUMBER2);
        }
        INFOLOG("grubInstallDevice nvme type device: %s", grubInstallDevice.c_str());
        return true;
    }
    return false;
}

bool OsRestore::UseGrub2Install()
{
    std::string checkCommand = "chroot /bmr_os_restore /bin/bash -c 'PATH=$PATH:/bin:/sbin grub2-install --version'";
    return RunCommand(checkCommand) == SUCCESS;
}

bool OsRestore::UseGrubInstall()
{
    std::string checkCommand = "chroot /bmr_os_restore /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install --version'";
    return RunCommand(checkCommand) == SUCCESS;
}

bool OsRestore::RebootSystem()
{
    INFOLOG("Reboot System");
    std::string cmd = "shutdown -r +1";
    // to do pm上添加挂载失败卷信息
    std::vector<std::string> output;
    std::vector<std::string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("reboot failed, cmd:%s", cmd.c_str());
        return false;
    }
    INFOLOG("reboot success, cmd:%s", cmd.c_str());
    return true;
}

int OsRestore::RunCommand(const std::string& cmd)
{
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("cmd failed! %d", ret);
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
    }
    return ret;
}

bool OsRestore::WipeDisk(const std::string& diskName)
{
    int fd = ::open(diskName.c_str(), O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERRLOG("open disk(%s) failed!error: %d", diskName.c_str(), errno);
        return false;
    }
    uint8_t* buffer = new uint8_t[SUPPER_BLOCK_SIZE];
    bzero(buffer, SUPPER_BLOCK_SIZE);
    ::lseek(fd, 0, SEEK_SET);
    if (::write(fd, buffer, SUPPER_BLOCK_SIZE) < 0) {
        ERRLOG("write disk(%s) failed! errno: %d", diskName.c_str(), errno);
        ::close(fd);
        delete[] buffer;
        return false;
    }
    ::close(fd);
    delete[] buffer;
    return true;
}