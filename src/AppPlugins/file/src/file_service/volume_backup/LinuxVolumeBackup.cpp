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
#include "LinuxVolumeBackup.h"
#include <thread>
#include <vector>
#include <set>
#include "common/volume/VolumeCommonService.h"
#include "define/Types.h"
#include "PluginUtilities.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Path.h"
#include "host/HostCommonStruct.h"
#include "ShareResourceManager.h"
#include "PluginConstants.h"
#include "VolumeProtector.h"
#include "native/FileSystemAPI.h"

#include <sys/mount.h>
#include <mntent.h>
#include "LvmSnapshotProvider.h"

using namespace std;
using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;

namespace FilePlugin {

namespace {
    const std::string MODULE = "LinuxVolumeBackup";
    const std::string SYS_MOUNTS_ENTRY_PATH = "/proc/mounts";
    const std::string LOOP_DEVICE_CONTROL_PATH = "/dev/loop-control";
    const int MNTENT_BUFFER_MAX = 4096;
    const std::string BTRFS_FS_TYPE_STR = "btrfs";
    const std::string XFS_FS_TYPE_STR = "xfs";
    const std::string DEFAULT_SNAPSHOT_PERCENT = "5";
}

bool LinuxVolumeBackup::VolumeExists(const std::string& volumeName)
{
    return PluginUtils::GetVolumeSize(volumeName) != 0;
}

void LinuxVolumeBackup::CheckExistVolume(const std::vector<std::string>& volumeNames,
    std::vector<std::string>& existVolumes, std::string& notExistVolumes)
{
    std::string sepChar = ", ";
    for (const std::string& volumeName : volumeNames) {
        if (!VolumeExists(volumeName)) {
            ERRLOG("Volume %s is not exist!", volumeName.c_str());
            notExistVolumes += (volumeName + sepChar);
        } else {
            existVolumes.push_back(volumeName);
        }
    }
    if (!notExistVolumes.empty()) {
        size_t pos = notExistVolumes.size() - sepChar.size(); // 上报的path不带最后一个逗号
        notExistVolumes = notExistVolumes.substr(0, pos);
    }
}

void LinuxVolumeBackup::GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes)
{
    for (auto &iter : m_sysVolumeList) {
        auto fcmp = [&](VolumeInfo &elem) { return elem.volumePath == iter.volumePath; };
        if (std::find_if(sourceVolumes.begin(), sourceVolumes.end(), fcmp) != sourceVolumes.end()) {
            continue;
        }
        sourceVolumes.push_back(iter);
    }

    for (auto &iter : m_protectVolumeVec) {
        auto fcmp = [&](VolumeInfo &elem) { return elem.volumePath == iter; };
        if (std::find_if(sourceVolumes.begin(), sourceVolumes.end(), fcmp) != sourceVolumes.end()) {
            continue;
        }
        VolumeInfo vol {};
        vol.volumePath = iter;
        vol.type = VolumeType::LVM;
        vol.size = PluginUtils::GetVolumeSize(iter);
        sourceVolumes.push_back(vol);
    }
}

bool LinuxVolumeBackup::CopyBoot()
{
    std::string cmdCopyBoot = "/bin/cp -rf /boot " + m_dataFsPath;
    INFOLOG("Enter CopyBoot command: %s", cmdCopyBoot.c_str());
    // to do pm上添加挂载失败卷信息
    std::vector<std::string> output;
    std::vector<std::string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmdCopyBoot, {}, output, errput);
    if (ret != 0) {
        ERRLOG("copy failed, cmdCopyBoot:%s", cmdCopyBoot.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_backup_backup_boot_warning_label");
        return false;
    }
    return true;
}

bool LinuxVolumeBackup::GenerateSubJobHook()
{
    // 获取系统卷
    if (m_advParms.systemBackupFlag == TRUE_STR) {
        if (InitSystemVolume() != Module::SUCCESS) {
            return false;
        }
    }
    return true;
}

bool LinuxVolumeBackup::TearDownVolumeHook()
{
    if (m_advParms.systemBackupFlag == TRUE_STR && !CopyBoot()) {
        ERRLOG("Failed to copy boot folder!");
        return false;
    }
    return true;
}

volumeprotect::CopyFormat LinuxVolumeBackup::CopyFormat()
{
    return volumeprotect::CopyFormat::IMAGE;
}

bool LinuxVolumeBackup::IsLimitedKernel()
{
    if (!PluginUtils::IsPathExists(LOOP_DEVICE_CONTROL_PATH)) {
        // /dev/loop-control doesn't exist
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_backup_limited_kernel_warning_label");
        return true;
    }
    return false;
}

bool LinuxVolumeBackup::SaveVolumeMountEntriesJson()
{
    std::string volumePath = m_volumeInfomation.oriVolumePath;
    CreateDirectory(m_dataFsPath + SEP + m_volumeInfomation.volumeName);
    std::string volumeMountEntriesJson =
        m_dataFsPath + SEP + m_volumeInfomation.volumeName + SEP + VOLUME_MOUNT_ENTRIES_JSON_NAME;
    VolumeMountEntries volumeMountEntries {};
    volumeMountEntries.volumePath = volumePath;
    INFOLOG("save volume mount args json of %s to %s", volumePath.c_str(), volumeMountEntriesJson.c_str());
    FILE* mountsFile = ::setmntent(SYS_MOUNTS_ENTRY_PATH.c_str(), "r");
    if (mountsFile == nullptr) {
        ERRLOG("failed to open /proc/mounts, errno %u", errno);
        return false;
    }
    struct mntent entry {};
    char mntentBuffer[MNTENT_BUFFER_MAX] = { 0 };
    while (::getmntent_r(mountsFile, &entry, mntentBuffer, MNTENT_BUFFER_MAX) != nullptr) {
        if (std::string(entry.mnt_fsname) == volumePath) {
            VolumeMountEntry volumeMountEntry {};
            volumeMountEntry.mountTargetPath = entry.mnt_dir;
            volumeMountEntry.mountType = entry.mnt_type;
            volumeMountEntry.mountOptions = entry.mnt_opts;
            volumeMountEntry.mountTypeVersion = GetMountTypeVersion(volumeMountEntry.mountType, volumePath);
            INFOLOG("volume mount entry: %s ===> %s, type %s, options %s, version %s",
                volumePath.c_str(), volumeMountEntry.mountTargetPath.c_str(),
                volumeMountEntry.mountType.c_str(), volumeMountEntry.mountOptions.c_str(),
                volumeMountEntry.mountTypeVersion.c_str());
            volumeMountEntries.mountEntries.push_back(volumeMountEntry);
        }
    }
    ::endmntent(mountsFile);
    if (volumeMountEntries.mountEntries.empty()) {
        WARNLOG("failed to get mntent of %s, not found", volumePath.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_backup_volume_not_mounted_warning_label",
            volumePath);
    }
    return JsonFileTool::WriteToFile(volumeMountEntries, volumeMountEntriesJson);
}

bool LinuxVolumeBackup::InitProtectVolume()
{
    if (m_backupJobPtr->protectSubObject.empty()) {
        if (m_advParms.systemBackupFlag == FALSE_STR) {
            ERRLOG("Invalid volume set, set is empty");
            return false;
        }
        return true;
    }
    std::vector<std::string> volumeNames;
    for (AppProtect::ApplicationResource resource : m_backupJobPtr->protectSubObject) {
        volumeNames.push_back(resource.name);
        INFOLOG("protected path push %s", resource.name.c_str());
    }
    // 检查受保护卷是否存在
    std::string notExistVolumes;
    CheckExistVolume(volumeNames, m_protectVolumeVec, notExistVolumes);
    if (!notExistVolumes.empty()) {
        /* path not exists or have no sufficient privelege will both use this label */
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_backup_selected_path_not_accessible_label",
                       notExistVolumes);
    }
    if (m_protectVolumeVec.empty()  && (m_advParms.systemBackupFlag == FALSE_STR)) {
        ERRLOG("Not exist volume can be protected!");
        return false;
    }
    bool btrfsMountFound = false;
    for (const std::string& volumePath : m_protectVolumeVec) {
        if (GetMountedVolumeFsType(volumePath) == BTRFS_FS_TYPE_STR) {
            ERRLOG("volume %s is mounted btrfs, inform user to umount first", volumePath.c_str());
            btrfsMountFound = true;
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_mounted_btrfs_warning_label", volumePath);
        }
    }
    if (btrfsMountFound) {
        return false;
    }
    PushVolumeToFile();
    return true;
}
 
int LinuxVolumeBackup::GetSysVolumeList()
{
    // Get LVM volume, output of lvs command may include volumes without file systems
    std::vector<std::string> lvmDevList;
    std::vector<std::string> errOutput;
    std::string cmd = "lvs --noheadings -o lv_name,vg_name | awk '$3 !~ /^s/ {print \"/dev/mapper/\" $2 \"-\" $1}'";
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, { }, lvmDevList, errOutput);
    if (ret != Module::SUCCESS) {
        PluginUtils::LogCmdExecuteError(ret, lvmDevList, errOutput);
        return ret;
    }
 
    std::string sysVolFile = PluginUtils::PathJoin(m_sysInfoPath, "sysvol.txt");
    std::ifstream filep(sysVolFile, std::ios::in);
    if (!filep.is_open()) {
        ERRLOG("Failed to open file %s", sysVolFile.c_str());
        return Module::FAILED;
    }
 
    while (true) {
        std::string sysVolumeDevPath {};
        if (!std::getline(filep, sysVolumeDevPath)) {
            break;
        }
        if (sysVolumeDevPath.empty()) {
            break;
        }
        INFOLOG("System volume device is %s", sysVolumeDevPath.c_str());
        VolumeInfo volDev {};
        volDev.volumePath = sysVolumeDevPath;
        volDev.size = PluginUtils::GetVolumeSize(sysVolumeDevPath);
        if (std::find(lvmDevList.begin(), lvmDevList.end(), sysVolumeDevPath) != lvmDevList.end()) {
            volDev.type = VolumeType::LVM;
        } else {
            volDev.type = VolumeType::PART;
        }
        m_sysVolumeList.push_back(volDev);
    }
 
    filep.close();
    return Module::SUCCESS;
}
 
int LinuxVolumeBackup::GetSystemInfo()
{
    if (!PluginUtils::CreateDirectory(m_sysInfoPath)) {
        ERRLOG("Failed create directory %s", m_sysInfoPath.c_str());
        return Module::FAILED;
    }
 
    std::vector<std::string> output;
    std::vector<std::string> errOutput;
    std::vector<std::string> paramList;
    paramList.push_back(Module::CPath::GetInstance().GetRootPath());
    paramList.push_back(m_sysInfoPath);
    std::string cmd = "sh ?/bin/GetSystemInfo.sh ?";
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, paramList, output, errOutput);
    if (ret != Module::SUCCESS) {
        PluginUtils::LogCmdExecuteError(ret, output, errOutput);
        return ret;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_system_state_and_boot_partition_backuped_label");
    return Module::SUCCESS;
}
 
int LinuxVolumeBackup::CheckSystemVolume()
{
    for (auto &iter : m_sysVolumeList) {
        if ((iter.type == VolumeType::PART) && (iter.size > volumeprotect::ONE_GB)) {
            WARNLOG("Volume %s size is %llu, over 1GB", iter.volumePath.c_str(), iter.size);
        }
    }
    return Module::SUCCESS;
}
 
int LinuxVolumeBackup::InitSystemVolume()
{
    int ret = GetSystemInfo();
    if (ret != Module::SUCCESS) {
        return ret;
    }
 
    return Module::SUCCESS;
}

std::string LinuxVolumeBackup::GetMountTypeVersion(const std::string& mountType, const std::string& volumePath) const
{
    DBGLOG("Enter GetTarVolumeFilesystemInfo");
    std::string result = GetMountDevicetype(mountType, volumePath, MODULE);
    DBGLOG("Exit GetTarVolumeFilesystemInfo");
    return result;
}

std::string LinuxVolumeBackup::LoadSnapshotParentPath() const
{
    std::string snapshotParentPath = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, SNAPSHOT_PARENT_PATH_KEY);
    INFOLOG("using SnapshotParentPath key: %s, value = %s",
        SNAPSHOT_PARENT_PATH_KEY.c_str(), snapshotParentPath.c_str());
    return snapshotParentPath;
}

void LinuxVolumeBackup::ClearResidualSnapshotsAndAlarm()
{
    string snapResidualFile = PluginUtils::PathJoin(m_cacheFsParentPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    if (!PluginUtils::IsFileExist(snapResidualFile)) {
        INFOLOG("No residual snapshot exists, return directly! jobId: %s", m_jobId.c_str());
        return;
    }
    HostSnapResidualInfoList snapResidualInfos;
    if (!JsonFileTool::ReadFromFile(snapResidualFile, snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return;
    }
    if (snapResidualInfos.infos.empty()) {
        INFOLOG("snapResidualInfos is empty, return directly! jobId: %s", m_jobId.c_str());
        return;
    }
    HostSnapResidualInfoList newResidualInfos;
    for (const auto& info :  snapResidualInfos.infos) {
        std::vector<std::string> snapshots;
        for (const string& snapshotName : info.snapshotInfos) {
            if (!LvmSnapshotProvider::DeleteSnapshotByVolume(snapshotName)) {
                snapshots.push_back(snapshotName);
            }
        }
        if (snapshots.empty()) {
            ActionResult result;
            AppProtect::AlarmDetails alarm;
            alarm.alarmId = ALARM_CODE_FAILED_DELETE_SNAPSHOT;
            alarm.parameter = m_backupJobPtr->protectObject.type + "," + info.jobId;
            JobService::ClearAlarm(result, alarm);
        } else {
            // 清理失败, 更新失败快照信息
            HostSnapResidualInfo snapResidualInfo {};
            snapResidualInfo.jobId = info.jobId;
            snapResidualInfo.snapshotInfos = snapshots;
            newResidualInfos.infos.push_back(snapResidualInfo);
        }
    }
    if (newResidualInfos.infos.empty()) {
        // 残留快照都被删除，删除残留快照信息文件
        PluginUtils::Remove(snapResidualFile);
    } else if (!JsonFileTool::WriteToFile(newResidualInfos, snapResidualFile)) {
        // 残留快照未被完全删除，更新残留快照文件
        WARNLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    }
}

bool LinuxVolumeBackup::CreateSnapshot(const std::string& volumeName, std::string& snapDevVol)
{
    std::string logLabel  = "file_plugin_volume_backup_prepare_create_snap_failed_label";
    std::string oriVolumeName = LvmSnapshotProvider::GetLvmPath(volumeName);
    if (oriVolumeName.empty()) {
        ERRLOG("Get lvm path for: %s failed!", volumeName.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        return false;
    }
    std::string snapshotParentPath = LoadSnapshotParentPath();
    std::string snapshotPercent = m_advParms.snapshotPercent;
    snapshotPercent = snapshotPercent.empty() ? DEFAULT_SNAPSHOT_PERCENT : snapshotPercent;

    int errorCode = 0;
    auto future = std::async(std::launch::async, LvmSnapshotProvider::CreateSnapshotByVolumeName,
                             snapshotParentPath, m_jobId, oriVolumeName, snapshotPercent, std::ref(errorCode));
    if (future.wait_for(90s) != std::future_status::ready) {
        ERRLOG("Create snapshot for volume %s timeout.", volumeName.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        return false;
    }

    if (errorCode == SNAP_ERRNO_SPACELESS) {
        ERRLOG("lvm vg of %s doesn't have enough space!", volumeName.c_str());
        PluginReportInfo reportInfo;
        reportInfo.logLevel = JobLogLevel::TASK_LOG_WARNING;
        reportInfo.logLabel = "file_plugin_volume_backup_prepare_create_snap_lvm_vg_space_insufficent_label";
        size_t pos = oriVolumeName.find('/');
        std::string vgName = oriVolumeName.substr(0, pos);
        ReportJobDetails(reportInfo, vgName, volumeName, snapshotPercent + "%");
        return false;
    }

    std::shared_ptr<LvmSnapshot> snapShot = future.get();
    if (snapShot == nullptr) {
        size_t pos = oriVolumeName.find('/');
        std::string vgName = oriVolumeName.substr(0, pos);
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
            "file_plugin_volume_backup_prepare_create_snap_lvm_vg_space_insufficent_label",
            vgName, volumeName, snapshotPercent + "%");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        ERRLOG("Create snapshot for volume %s failed, error=%d", volumeName.c_str(), errorCode);
        return false;
    }

    INFOLOG("lvm snapshot created, [%s] [%s] [%s] [%s] [%s]",
        snapShot->m_oriDeviceVolume.c_str(), snapShot->m_oriDeviceMountPath.c_str(),
        snapShot->m_snapDeviceVolume.c_str(), snapShot->m_snapDeviceMountPath.c_str(), snapShot->m_snapshotId.c_str());
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_prepare_create_snap_succeed_label",
        snapShot->m_snapDeviceVolume);

    snapDevVol = snapShot->m_snapDeviceVolume;
    RecordResidualSnapshots({ snapShot->m_snapDeviceVolume });
    INFOLOG("Create snapshot for volume %s success, snapshot is %s", volumeName.c_str(), snapDevVol.c_str());
    return true;
}

void LinuxVolumeBackup::DeleteSnapshot()
{
    std::vector<std::string> snapshotNames;
    if (!GetSnapshots(snapshotNames)) {
        ERRLOG("Get snapshot info failed!");
        return;
    }
    std::vector<std::string> deleteFailedSnapshot;
    std::vector<std::string> deleteSucceedSnapshot;
    for (const std::string& snapshotName : snapshotNames) {
        if (!LvmSnapshotProvider::DeleteSnapshotByVolume(snapshotName)) {
            ERRLOG("Delete snapshot %s failed", snapshotName.c_str());
            deleteFailedSnapshot.push_back(snapshotName);
            continue;
        }
        deleteSucceedSnapshot.push_back(snapshotName);
    }
    ClearSucceedDeletedSnapshotRecord(deleteSucceedSnapshot);
    if (!deleteFailedSnapshot.empty()) {
        // 上报残留快照并告警
        SendAlarmForResidualSnapshots(deleteFailedSnapshot);
        // 记录残留快照
        (void)RecordResidualSnapshots(deleteFailedSnapshot);
    }
}

bool LinuxVolumeBackup::PreProcess(std::string& snapShotDev)
{
    if (m_volumeInfomation.type == VolumeType::LVM) {
        if (!CreateSnapshot(m_volumeInfomation.volumePath, snapShotDev)) {
            ERRLOG("Create snapshots failed");
            return false;
        }
        m_volumeInfomation.oriVolumePath = m_volumeInfomation.volumePath;
        m_volumeInfomation.volumePath = snapShotDev;
    }

    m_volumeInfomation.volumeName = PluginUtils::GetVolumeName(m_volumeInfomation.oriVolumePath);
    return true;
}

bool LinuxVolumeBackup::ScanVolumesToGenerateTask()
{
    INFOLOG("Enter, scan volumes to generate subtask.");
    std::vector<VolumeInfo> sourceVolumes;
    GetOriVolumes(sourceVolumes);

    VolumeBackupSubJob backupSubJob;
    uint64_t volumeCount = 0;
    uint64_t totalSize = 0;
    for (const VolumeInfo& volInfo : sourceVolumes) {
        backupSubJob.volumePath = volInfo.volumePath;
        backupSubJob.type = static_cast<int>(volInfo.type);
        backupSubJob.volumeName = volInfo.volumeName;
        DBGLOG("ScanVolumes, volumePath: %s", volInfo.volumePath.c_str());
        if (!CreateBackupSubTask(backupSubJob, SUBJOB_TYPE_DATACOPY_VOLUME)) { // 生成卷任务
            return false;
        }

        ++volumeCount;
        totalSize += GetVolumeSize(volInfo.volumePath);
    }

    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_scan_completed_label",
        to_string(volumeCount), FormatCapacity(totalSize), to_string(volumeCount), FormatCapacity(totalSize));
    if (!UpdateCopyPhaseStartTime()) {
        ERRLOG("Updated restore start time failed");
        return false;
    }
    return true;
}

void LinuxVolumeBackup::FillVolumeInfo(const VolumeBackupSubJob& backupSubJob)
{
    m_volumeInfomation.volumePath = backupSubJob.volumePath;
    m_volumeInfomation.type = static_cast<VolumeType>(backupSubJob.type);
    m_volumeInfomation.size = GetVolumeSize(m_volumeInfomation.volumePath);
}

void LinuxVolumeBackup::FillBackupConfig(VolumeBackupConfig &backupParams)
{
    backupParams.backupType = IsFullBackup() ? volumeprotect::BackupType::FULL : volumeprotect::BackupType::FOREVER_INC;
    backupParams.copyFormat = CopyFormat();
    backupParams.volumePath = m_volumeInfomation.volumePath;
    backupParams.prevCopyMetaDirPath = m_prevMetaPath + SEP + m_volumeInfomation.volumeName;
    backupParams.outputCopyMetaDirPath = m_metaFilePath + SEP + m_volumeInfomation.volumeName;
    backupParams.outputCopyDataDirPath = m_dataFsPath + SEP + m_volumeInfomation.volumeName;

    backupParams.blockSize = m_blockSize;
    backupParams.sessionSize = m_sessionSize;
    backupParams.hasherNum = volumeprotect::DEFAULT_HASHER_NUM;
    backupParams.hasherEnabled = true;
    backupParams.enableCheckpoint = false;
    INFOLOG("backup config: volumePath: %s, DataDirPath: %s, MetaDirPath: %s", backupParams.volumePath.c_str(),
            backupParams.outputCopyDataDirPath.c_str(), backupParams.outputCopyMetaDirPath.c_str());
    return;
}

uint32_t LinuxVolumeBackup::GetSysVolumeSize()
{
    return m_sysVolumeList.size();
}

std::string LinuxVolumeBackup::GetMountedVolumeFsType(const std::string& volumePath) const
{
    FILE* mountsFile = ::setmntent(SYS_MOUNTS_ENTRY_PATH.c_str(), "r");
    if (mountsFile == nullptr) {
        ERRLOG("failed to open /proc/mounts, errno %u", errno);
        return "";
    }
    struct mntent entry {};
    char mntentBuffer[MNTENT_BUFFER_MAX] = { 0 };
    std::string fsType;
    while (::getmntent_r(mountsFile, &entry, mntentBuffer, MNTENT_BUFFER_MAX) != nullptr) {
        if (std::string(entry.mnt_fsname) == volumePath) {
            fsType = entry.mnt_type;
            break;
        }
    }
    ::endmntent(mountsFile);
    return fsType;
}

} // namespace FilePlugin
