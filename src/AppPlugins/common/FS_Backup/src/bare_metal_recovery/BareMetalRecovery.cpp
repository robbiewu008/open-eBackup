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
#include "BareMetalRecovery.h"
#include "common/EnvVarManager.h"


using namespace std;
using namespace Module;

namespace {
    constexpr auto MODULE = "BMR";

    const uint64_t ONE_KB = 1024LLU;
    const uint64_t ONE_MB = 1024LLU * ONE_KB;
    const uint64_t ONE_GB = 1024LLU * ONE_MB;
    const uint64_t ONE_TB = 1024LLU * ONE_GB;
    const std::string PARTITIONS_INFO_PREFIX = "partition__dev_";
    const std::string PARTITIONS_INFO_PREFIX_DOS = "dos_partition__dev_";
    const std::string PARTITIONS_INFO_PREFIX_GPT = "gpt_partition__dev_";
    const std::string FSTAB = "fstab";
    const std::string MOUNTED_INFO = "mounts";
    const std::string FSTAB_PATH = "/etc/fstab";
    const std::string VGLIST = "vglist";
    const std::string PVLIST = "pvlist";
    const std::string LV_FSTYPE = "lv_fsType";
    const std::string SWAP = "swap";
    const std::string EFI_SYSTEM_DIR = "/sys/firmware/efi/efivars";
    const std::string GRUB_CONFIG = "/boot/grub2/grub.cfg";
    const uint8_t BLK_DISK_FILE_COLUMN_LENGTH = 4;
    const uint8_t FSTAB_FILE_COLUMN_LENGTH = 6;
    const uint64_t SUPPER_BLOCK_SIZE = 2048;
    const std::string REPLACE_VG_DIR = "/DataBackup/ProtectClient/ProtectClient-E/slog/Plugins/FilePlugin/VG";

    const int NUM0 = 0;
    const int NUM1 = 1;
    const int NUM2 = 2;
    const int NUM3 = 3;
    const int NUM4 = 4;
    const int NUM5 = 5;
    const int NUM9 = 9;
}

BareMetalRecovery::BareMetalRecovery(const BareMetalRecoveryConfig& config)
{
    m_config = config;
    m_lvmInfoPath = FSBackupUtils::JoinPath(m_config.sysInfoPath, "lvm_info/");
    m_replaceVgDir = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + REPLACE_VG_DIR;
    return;
}

bool BareMetalRecovery::SetDiskMapInfo(std::string &diskMapInfoSet)
{
    Json::Value value;
    if (!Module::JsonHelper::JsonStringToJsonValue(diskMapInfoSet, value)) {
        ERRLOG("diskMapJsonString change failed");
        return false;
    }
    for (auto val : value) {
        DiskMapInfo diskMapInfo;
        if (!Module::JsonHelper::JsonValueToStruct(val, diskMapInfo)) {
            ERRLOG("Json change failed");
            continue;
        }
        INFOLOG("Diskmap info : sourceDiskName %s, sourceDiskSize %llu, targetDiskName %s, targetDiskSize %llu",
            diskMapInfo.sourceDiskName.c_str(),
            diskMapInfo.sourceDiskSize,
            diskMapInfo.targetDiskName.c_str(),
            diskMapInfo.targetDiskSize);
        DiskInfo sourceDiskInfo{diskMapInfo.sourceDiskName, diskMapInfo.sourceDiskSize};
        DiskInfo targetDiskInfo{diskMapInfo.targetDiskName, diskMapInfo.targetDiskSize};
        // map顺序需要改
        m_diskMapping[sourceDiskInfo] = targetDiskInfo;
    }
    return true;
}

BackupRetCode BareMetalRecovery::Start()
{
    INFOLOG("Start BareMetalRecovery!");
    if (!GetTargetDiskInfo()) {
        return BackupRetCode::FAILED;
    }
    if (!GetSourceDiskInfo()) {
        return BackupRetCode::FAILED;
    }
    if (!GetPvInfo()) {
        ERRLOG("get source physical volumes failed");
        return BackupRetCode::FAILED;
    }
    for (const DiskInfo& sourceDiskInfo : m_sourceDiskInfoSet) {
        if (m_diskMapping.count(sourceDiskInfo) == 0) {
            continue;
        }
        DiskInfo targetDiskInfo = m_diskMapping[sourceDiskInfo];
        if (!DiskPartition(sourceDiskInfo, targetDiskInfo)) {
            ERRLOG("disk partition failed, targetDisk: %s, sourceDisk: %s",
                targetDiskInfo.diskName.c_str(), sourceDiskInfo.diskName.c_str());
            return BackupRetCode::FAILED;
        }
    }
    if (!LvmRestoration()) {
        ERRLOG("restore lvm info failed");
        return BackupRetCode::FAILED;
    }
    return BackupRetCode::SUCCESS;
}

BackupRetCode BareMetalRecovery::Abort()
{
    return BackupRetCode::FAILED;
}

bool BareMetalRecovery::DiskMapping()
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
    // 按名字匹配失败
    diskMapping.clear();
    return true;
}

bool BareMetalRecovery::DiskMappingByName(std::unordered_map<DiskInfo, DiskInfo, DiskInfoHash> &diskMapping)
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

bool BareMetalRecovery::GetSourceDiskInfo()
{
    m_sourceDiskInfoSet.clear();
    std::string blockDiskPath = FSBackupUtils::JoinPath(m_config.sysInfoPath, "blk_disk");
    std::string cmd = "cat " + blockDiskPath;
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("GetSourceDiskInfo failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    for (auto& disk : output) {
        vector<string> diskAndSize;
        INFOLOG("volumeInfo information:%s", disk.c_str());
        boost::split(diskAndSize, disk, boost::is_any_of(" "), boost::token_compress_on);
        if (diskAndSize.size() != NUM2) {
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

bool BareMetalRecovery::GetTargetDiskInfo()
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
            ERRLOG("errmsg : %s", msg.c_str());
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

uint64_t BareMetalRecovery::GetDiskSize(const std::string& diskSizeStr)
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


bool BareMetalRecovery::DiskPartition(const DiskInfo& sourceDisk, const DiskInfo& targetDisk)
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

    std::string sourcePartitionsFile = FSBackupUtils::JoinPath(m_config.sysInfoPath,
        PARTITIONS_INFO_PREFIX + sourceDiskName);
    std::string sourcePartitionsFileForDos = FSBackupUtils::JoinPath(m_config.sysInfoPath,
        PARTITIONS_INFO_PREFIX_DOS + sourceDiskName);
    std::string sourcePartitionsFileForGPT = FSBackupUtils::JoinPath(m_config.sysInfoPath,
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

bool BareMetalRecovery::LvmRestoration()
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
        if (!CreateVolumeGroup(vgName) || !ActivateVolumeGroup(vgName)) { // 如果存在就激活卷组？
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

bool BareMetalRecovery::MakeVgBeforeGenerate()
{
    if (!FSBackupUtils::RemoveDir(m_replaceVgDir)) {
        ERRLOG("Clean m_replaceVgDir: %s failed", m_replaceVgDir.c_str());
        return false;
    }
    FSBackupUtils::RecurseCreateDirectory(m_replaceVgDir);
    return true;
}

bool BareMetalRecovery::ReplaceVgFile(const std::string &vgName)
{
    std::string sourceVgFile = FSBackupUtils::JoinPath(m_lvmInfoPath, vgName);
    std::string targetVgFile = FSBackupUtils::JoinPath(m_replaceVgDir, vgName);
    if (!FSBackupUtils::Exists(sourceVgFile)) {
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

bool BareMetalRecovery::ReplaceAndCreatePv()
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

void BareMetalRecovery::ReplaceNonLvName(std::string &volumeName)
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

bool BareMetalRecovery::GetVgNames()
{
    std::string vgListFile = FSBackupUtils::JoinPath(m_lvmInfoPath, VGLIST);
    if (!FSBackupUtils::Exists(vgListFile)) {
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

bool BareMetalRecovery::GetPvInfo()
{
    // 文件里的每一行分别是：pv_name, vg_name, pv_uuid
    std::string pvListFile = FSBackupUtils::JoinPath(m_lvmInfoPath, PVLIST); // lvm_info/pvlist -> pvlist
    if (!FSBackupUtils::Exists(pvListFile)) {
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
        if (vec.size() < NUM2) {
            ERRLOG("dismiss invalid lvm_info/pvlist entry %s, requre at least 2 colume!", line.c_str());
            continue;
        }
        if (vec.size() == NUM2) {
            m_pvInfo[vec[NUM0]] = {"", vec[NUM1]};
            INFOLOG("get pv info, pv: %s vg: (unbinded) uuid: %s", vec[NUM0].c_str(), vec[NUM1].c_str());
        } else {
            m_pvInfo[vec[NUM0]] = {vec[NUM1], vec[NUM2]};
            INFOLOG("get pv info, pv: %s vg: %s uuid: %s", vec[NUM0].c_str(), vec[NUM1].c_str(), vec[NUM2].c_str());
        }
    }
    file.close();
    INFOLOG("get pv info success, num of pvs: %d", m_pvInfo.size());
    return true;
}

bool BareMetalRecovery::GetLvInfo()
{
    std::string lvFsTypeFile = FSBackupUtils::JoinPath(m_config.sysInfoPath, LV_FSTYPE);
    if (!FSBackupUtils::Exists(lvFsTypeFile)) {
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
        if (vec.size() < NUM2) {
            WARNLOG("dismiss invalid lvm_info/lv_fstype entry %s, requre 2 colume!", line.c_str());
            continue;
        }
        m_lvFsType[vec[NUM0]] = vec[NUM1];
        INFOLOG("get lv info, lv: %s fsType: %s", vec[NUM0].c_str(), vec[NUM1].c_str());
    }
    file.close();
    INFOLOG("get lv info success, num of lvs: %d", m_lvFsType.size());
    return true;
}

bool BareMetalRecovery::CreatePhysicalVolume(const std::string& sourcePvName, const std::string& targetPvName,
                                             const std::string& pvUuid)
{
    std::string vgName = m_pvInfo[sourcePvName][0];
    std::string lvmBackupFile = FSBackupUtils::JoinPath(m_replaceVgDir, vgName);
    if (!vgName.empty() && !FSBackupUtils::Exists(lvmBackupFile)) {
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

bool BareMetalRecovery::CreateVolumeGroup(const std::string& vgName)
{
    INFOLOG("Enter CreateVolumeGroup, vgName: %s", vgName.c_str());
    std::string lvmBackupFile = FSBackupUtils::JoinPath(m_replaceVgDir, vgName);
    std::string cmd = "vgcfgrestore -f " + lvmBackupFile + " " + vgName;
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("vgcreate cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("vgcreate failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool BareMetalRecovery::ActivateVolumeGroup(const std::string& vgName)
{
    INFOLOG("Enter ActivateVolumeGroup, vgName: %s", vgName.c_str());
    std::string cmd = "vgchange -a y " + vgName;
    vector<std::string> output;
    vector<std::string> errOutput;
    INFOLOG("VG activate cmd: %s", cmd.c_str());
    if (runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput) != 0) {
        ERRLOG("VG activate failed!");
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool BareMetalRecovery::FormatFileSystemForLvs()
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
                ERRLOG("errmsg : %s", msg.c_str());
            }
        }
    }
    INFOLOG("Exit FormatFileSystemForLvs");
    return true;
}

bool BareMetalRecovery::FormatFileSystemForBootPartition()
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

std::string BareMetalRecovery::FormatUuidToDosSerial(const std::string& vfatUuid) const
{
    if (vfatUuid.length() == NUM9 && vfatUuid[NUM4] == '-') {
        std::string dosSerial = vfatUuid.substr(NUM0, NUM4) + vfatUuid.substr(NUM5, NUM4);
        INFOLOG("change vfat UUID to DOS serial number %s => %s", vfatUuid.c_str(), dosSerial.c_str());
        return dosSerial;
    }
    return vfatUuid;
}

bool BareMetalRecovery::MkfsAndMountBoot(const VolumeInfo& volumeInfo)
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

bool BareMetalRecovery::GetBootVolumeInfoFromFile()
{
    bool ret = true;
    std::string bootInfoFile = FSBackupUtils::JoinPath(m_config.sysInfoPath, "boot_part");
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
        ReplaceNonLvName(volume[NUM0]);
        if (volume[NUM2] == "/boot") {
            m_bootVolumeInfo.volumeName = volume[NUM0];
            m_bootVolumeInfo.fsType= volume[NUM1];
            m_bootVolumeInfo.mountPoint = volume[NUM2];
            m_bootVolumeInfo.volumeUUID = volume[NUM3];
        }
        if (volume[NUM2] == "/boot/efi") {
            m_bootEfiVolumeInfo.volumeName = volume[NUM0];
            m_bootEfiVolumeInfo.fsType= volume[NUM1];
            m_bootEfiVolumeInfo.mountPoint = volume[NUM2];
            m_bootEfiVolumeInfo.volumeUUID = volume[NUM3];
        }
        INFOLOG("Get volume info, name: %s, fsType: %s, mountPoint: %s, uuid: %s", volume[NUM0].c_str(),
            volume[NUM1].c_str(), volume[NUM2].c_str(), volume[NUM3].c_str());
    }
    file.close();
    return ret;
}

bool BareMetalRecovery::RestoreFsAndMountInfo()
{
    INFOLOG("Enter RestoreFsAndMountInfo!");
    std::string fstabFile = FSBackupUtils::JoinPath(m_config.sysInfoPath, FSTAB);
    std::string mountedInfoFile = FSBackupUtils::JoinPath(m_config.sysInfoPath, MOUNTED_INFO);
    if (!FSBackupUtils::Exists(fstabFile) || !FSBackupUtils::Exists(mountedInfoFile)) {
        ERRLOG("fastb or mounted_info file not exist %s %s", fstabFile.c_str(), mountedInfoFile.c_str());
        return false;
    }
    std::string initDirCmd = "cat " + fstabFile +
                      "| grep -E \"xfs|ext2|ext3|ext4|btrfs|vfat|fat32\" | awk '{print $1,\"/bmr_" + m_config.jobId +
                      "\"$2,$3,$4,$5,$6}' | awk '{print $2}' | xargs -i mkdir -p {}";
    INFOLOG("ready to init fstab mount point %s", initDirCmd.c_str());
    RunCommand(initDirCmd);
    // 把系统和boot分区临时挂载到/bmr_jobid目录下
    std::string cmd = "cat " + fstabFile +
                      "| grep -E \"xfs|ext2|ext3|ext4|btrfs|vfat|fat32\" | awk '{if($4~\"bind\")"
                      "{print \"/bmr_"+ m_config.jobId +"\"$1,\"/bmr_" +
                      m_config.jobId + "\"$2,$3,$4\",X-mount.mkdir\",$5,$6}"
                      "else {print $1,\"/bmr_" + m_config.jobId +
                      "\"$2,$3,$4\",X-mount.mkdir\",$5,$6}}' >> " + FSTAB_PATH;
    INFOLOG("ready to mount all fstab mount point: %s", cmd.c_str());
    if (RunCommand(cmd) != SUCCESS) {
        ERRLOG("mount all fstab mount point failed!");
        return false;
    }
    std::string mountCmd = "mount -a";
    RunCommand(mountCmd);  // 不用关心mount -a的执行结果，fstab可能存在没有恢复的分区，只要系统卷挂载了就可以
    return true;
}
  
bool BareMetalRecovery::MountBindSysProcDev()
{
    INFOLOG("Enter MountBindSysProcDev");
    std::string cmd = "mkdir -p /bmr_" + m_config.jobId +"/sys && mount --bind /sys /bmr_" + m_config.jobId +"/sys" +
        " && mkdir -p /bmr_" + m_config.jobId +"/proc && mount --bind /proc /bmr_" + m_config.jobId +"/proc" +
        " && mkdir -p /bmr_" + m_config.jobId +"/dev && mount --bind /dev /bmr_" + m_config.jobId +"/dev";
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    INFOLOG("MountBindSysProcDev cmd: %s", cmd.c_str());
    if (ret != 0) {
        ERRLOG("MountBindSysProcDev failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
}

bool BareMetalRecovery::RebuildInitramfs()
{
    INFOLOG("Enter RebuildInitramfs");
    // For now, rebuild initramfs, trying to solve the problem
    std::string cmd = "chroot /bmr_" + m_config.jobId + " dracut -a resume --regenerate-all --force";
    if (RunCommand(cmd) != SUCCESS) {
        WARNLOG("RebuildInitramfs failed!");
        return false;
    }
    return true;
}

bool BareMetalRecovery::GenerateNewGrubCfg()
{
    INFOLOG("Enter GenerateNewGrubCfg");
    // If EFI_SYSTEM_DIR is exist(it's efi system), we don't need to generate grub.cfg.
    // Install legacy GRUB (core.img) as ESP do not need recovery
    if (FSBackupUtils::Exists(EFI_SYSTEM_DIR)) {
        INFOLOG("UEFI detected, ESP not need recovery! ESP volume %s", m_bootEfiVolumeInfo.volumeName.c_str());
        return true;
    }
    INFOLOG("Generate Legacy GRUB config for %s", m_bootVolumeInfo.volumeName.c_str());
    std::string grubInstallDevice = "/dev/sda";
    // 找不到boot所在的设备，也尝试往下走，/boot所在的设备有可能已经安装过grub2了
    SearchBootDisk(grubInstallDevice);
    // 检查原操作系统grub修复命令(grub2-install, grub-install, grub-install.unsupported)
    if (UseGrub2Install()) {
        std::string cmdInstallGrub = "grub2-install --skip-fs-probe --root-directory=/bmr_" + m_config.jobId +
            " --no-floppy " + grubInstallDevice;
        std::string cmdChangeRootMakeConfig = "chroot /bmr_" + m_config.jobId + " grub2-mkconfig -o " + GRUB_CONFIG;
        std::string cmdAddSelinuxToGrubConfig = "sed -i \"/vmlinuz/s/$/ selinux=0/\" /bmr_" +
            m_config.jobId + GRUB_CONFIG;
        if (RunCommand(cmdInstallGrub) != SUCCESS || RunCommand(cmdChangeRootMakeConfig) != SUCCESS
            || RunCommand(cmdAddSelinuxToGrubConfig) != SUCCESS) {
            return false;
        }
        return true;
    } else if (UseGrubInstall()) {
        std::string grubCmd = "chroot /bmr_" + m_config.jobId + " /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install " +
            grubInstallDevice + " --no-floppy'";
        return RunCommand(grubCmd) == SUCCESS;
    } else {
        std::string grubCmd = "chroot /bmr_" + m_config.jobId +
            " /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install.unsupported " + grubInstallDevice + " --no-floppy'";
        return RunCommand(grubCmd) == SUCCESS;
    }
}

bool BareMetalRecovery::UseGrub2Install()
{
    std::string checkCommand = "chroot /bmr_" + m_config.jobId +
        " /bin/bash -c 'PATH=$PATH:/bin:/sbin grub2-install --version'";
    return RunCommand(checkCommand) == SUCCESS;
}

bool BareMetalRecovery::UseGrubInstall()
{
    std::string checkCommand = "chroot /bmr_" + m_config.jobId +
        " /bin/bash -c 'PATH=$PATH:/bin:/sbin grub-install --version'";
    return RunCommand(checkCommand) == SUCCESS;
}

void BareMetalRecovery::SearchBootDisk(std::string& grubInstallDevice) const
{
    std::string bootPath = "/bmr_" + m_config.jobId + "/boot";
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
        cmdSearchBootDisk = "lsblk -p -o NAME,MOUNTPOINT,PKNAME | awk '$2==\"/bmr_" + m_config.jobId + "\"{print $3}'";
        if (runShellCmdWithOutput(INFO, MODULE, 0, cmdSearchBootDisk, {}, output, errOutput) != 0 ||
            output.size() == 0) {
            WARNLOG("can not find /boot path from lsblk, use default device(%s)!", grubInstallDevice.c_str());
            return;
        }
    }
    grubInstallDevice = output[0];
    // lsblk获取到的设备名可能包含数字，例如/dev/sda1，需要获取父设备名字/dev/sda
    while (!grubInstallDevice.empty() && isdigit(grubInstallDevice.back())) {
        grubInstallDevice = grubInstallDevice.substr(0, grubInstallDevice.size() - 1);
    }
}

bool BareMetalRecovery::CopyBoot(const std::string& path)
{
    std::string targetPath = "/bmr_" + m_config.jobId;
    std::string cmdCopyBoot = "cp -ir " + path + " " + targetPath;
    if (RunCommand(cmdCopyBoot) != SUCCESS) {
        return false;
    }
    return true;
}

int BareMetalRecovery::RunCommand(const std::string& cmd)
{
    vector<std::string> output;
    vector<std::string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("cmd failed! %d", ret);
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
        }
    }
    return ret;
}

bool BareMetalRecovery::WipeDisk(const std::string& diskName)
{
    int fd = ::open(diskName.c_str(), O_RDWR | O_EXCL, S_IRUSR | S_IWUSR);
    if (fd < 0) {
        ERRLOG("open disk(%s) failed!", diskName.c_str());
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