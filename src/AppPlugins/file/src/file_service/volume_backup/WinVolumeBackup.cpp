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
#include "log/Log.h"
#include "FileSystemUtil.h"
#include "PluginUtilities.h"
#include "VolumeProtector.h"
#include "native/FileSystemAPI.h"
#include "VssClient.h"
#include "file_resource/Win32Handler.h"
#include "HostCommonStruct.h"
#include "WinVolumeBackup.h"
#include "win32/BCD.h"
#include "PluginUtilities.h"

using namespace std;
using namespace Module;
using namespace Module::FileSystemUtil;
using namespace FilePlugin;
using namespace PluginUtils;
using namespace PluginUtils::Win32;
using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace Win32VSSWrapper;

namespace {
    // "\\?\"
    const int VOLUME_NAME_PREFIX_LENGTH = 4;
    // NT kernel space device path starts with "\Device" while user space device path starts with "\\."
    constexpr auto WKERNEL_SPACE_DEVICE_PATH_PREFIX = LR"(\Device)";
    constexpr auto WUSER_SPACE_DEVICE_PATH_PREFIX = LR"(\\.)";
    constexpr auto WDEVICE_PHYSICAL_DRIVE_PREFIX = LR"(\\.\PhysicalDrive)";
    constexpr auto WDEVICE_HARDDISK_VOLUME_PREFIX = LR"(\\.\HarddiskVolume)";
    const std::string USER_SPACE_VOLUME_NAME_PREFIX = R"(\\?\)";
    const std::string USER_SPACE_VOLUME_NAME_SUBFIX = R"(\)";
}

uint32_t WinVolumeBackup::GetSysVolumeSize()
{
    return 0;
}

// 获取name : volume{GUID}, valid : //?/volume{GUID}/
std::string WinVolumeBackup::GetValidVolumeName(const std::string& volumeName)
{
    std::string str = USER_SPACE_VOLUME_NAME_PREFIX + volumeName + USER_SPACE_VOLUME_NAME_SUBFIX;
    INFOLOG("return valid volume name: %s", str.c_str());
    return str;
}

// prerequisite
void WinVolumeBackup::ClearResidualSnapshotsAndAlarm()
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
    VssClient client;
    SnapshotSetResult res;
    for (const auto& info : snapResidualInfos.infos) {
        std::vector<std::string> snapshots;
        for (const string& snapshotName : info.snapshotInfos) {
            INFOLOG("delete snapshot name: %s", snapshotName.c_str());
            if (!client.DeleteSnapshot(snapshotName)) {
                WARNLOG("delete snapshot failed, name: %s, %d", snapshotName.c_str(), GetLastError());
                snapshots.push_back(snapshotName);
                continue;
            }
            INFOLOG("delete snapshot succeed, name: %s", snapshotName.c_str());
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
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    }
    return;
}

bool WinVolumeBackup::InitProtectVolume()
{
    // check volume exist
    if (m_backupJobPtr->protectSubObject.empty()) {
        ERRLOG("Invalid volume set, set is empty");
        return false;
    }
    WinVolumeBackupProtectObjectExtend extendInfo;
	// Get protect information
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->protectObject.extendInfo, extendInfo)) {
        ERRLOG("JsonStringToStruct failed for extendInfo.");
        return false;
    }
    INFOLOG("extend info str : %s", extendInfo.paths.c_str());
    std::string jsonStr = "{\"paths\":" + extendInfo.paths + "}";
    WinVolumeBackupProtectObjectVolumeNameWrapper volumeNameWrapper;
    if (!Module::JsonHelper::JsonStringToStruct(jsonStr, volumeNameWrapper)) {
        ERRLOG("JsonString convert invalid!");
        return false;
    }
    std::vector<Win32VolumesDetail> volumes;
    std::string notExistVolumes;
    for (const WinVolumeBackupProtectObjectVolumeName& item : volumeNameWrapper.paths) {
        INFOLOG("Protected volume name : %s", item.name.c_str());
        m_protectVolumeVec.push_back(item.name);
    }

    PushVolumeToFile();
    return true;
}

// generate subjob
void WinVolumeBackup::GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes)
{
    for (const std::string& iter : m_protectVolumeVec) {
        auto fcmp = [&](VolumeInfo &elem) { return elem.volumeName == iter;};
        if (std::find_if(sourceVolumes.begin(), sourceVolumes.end(), fcmp) != sourceVolumes.end()) {
            continue;
        }
        VolumeInfo vol {};
        vol.volumeName = iter;
        INFOLOG("Push volume : %s", vol.volumeName.c_str());
        sourceVolumes.push_back(vol);
    }
    return;
}

bool WinVolumeBackup::ScanVolumesToGenerateTask()
{
    INFOLOG("Enter, scan volumes to generate subtask.");
    VolumeBackupSubJob backupSubJob;
    uint64_t volumeCount = 0;
    uint64_t totalSize = 0;
    for (const std::string& volumeName : m_protectVolumeVec) {
        INFOLOG("process volume: %s", volumeName.c_str());
        if (volumeName == EFI_SYSTEM_PARTITION) {
            volumeCount++;
            totalSize += EFI_SYSTEM_PARTITION_SIZE;
            backupSubJob.volumeName = volumeName;
            backupSubJob.volumePath = volumeName;
            INFOLOG("ScanVolumes, volumePath: %s", volumeName.c_str());
            if (!CreateBackupSubTask(backupSubJob, SUBJOB_TYPE_DATACOPY_VOLUME)) { // 生成卷任务
                ERRLOG("Create subtask failed!");
                return false;
            }
            continue;
        }
        if (!volumeprotect::fsapi::IsVolumeExists(volumeName)) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
                "file_plugin_backup_selected_path_not_exist_label", volumeName);
            continue;
        }
        backupSubJob.volumeName = volumeName;
        backupSubJob.volumePath = volumeName;
        INFOLOG("ScanVolumes, volumePath: %s", volumeName.c_str());
        if (!CreateBackupSubTask(backupSubJob, SUBJOB_TYPE_DATACOPY_VOLUME)) { // 生成卷任务
            return false;
        }
        volumeCount++;
        totalSize += GetVolumeSize(backupSubJob.volumePath);
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_scan_completed_label",
        to_string(volumeCount), FormatCapacity(totalSize), to_string(volumeCount), FormatCapacity(totalSize));
    if (!UpdateCopyPhaseStartTime()) {
        ERRLOG("Updated restore start time failed");
        return false;
    }
    INFOLOG("leave ScanVolumesToGenerateTask");
    return true;
}

void PrintSnapRes(const SnapshotSetResult& snapRes)
{
    INFOLOG("Print SnapRes: %s", Utf16ToUtf8(snapRes.m_wSnapshotSetID).c_str());
    for (std::wstring str : snapRes.m_wSnapshotIDList) {
        INFOLOG("Print Content: %s", Utf16ToUtf8(str).c_str());
    }
}

bool WinVolumeBackup::RecordVolume()
{
    if (!IsFullBackup()) {
        return true;
    }
    StringVolumeInfo strVolInfo;
    if (m_volumeName == EFI_SYSTEM_PARTITION) {
        strVolInfo.displayName = EFI_SYSTEM_PARTITION;
        strVolInfo.drivePath = GetEFIDrivePath();
        strVolInfo.volumeName = EFI_SYSTEM_PARTITION;
        strVolInfo.fileSystem = "FAT32";
        strVolInfo.volumeType = 0;
        strVolInfo.totalSize = EFI_SYSTEM_PARTITION_SIZE;
    } else {
        Win32Handler handler;
        std::vector<WinVolumeInfo> volumeInfos = handler.GetAllVolumes();
        WinVolumeInfo curVolume;
        for (const WinVolumeInfo& volumeInfo : volumeInfos) {
            std::string volumeNameStr = Utf16ToUtf8(volumeInfo.volumeName);
            if (volumeNameStr.find(m_volumeName) != std::string::npos) {
                INFOLOG("record volume : %s", volumeNameStr.c_str());
                curVolume = volumeInfo;
            }
        }
        
        strVolInfo = handler.ConvertVolumeInfo(curVolume);
        handler.LogStringVolumeInfo(strVolInfo);
    }

    std::string content;
    if (!Module::JsonHelper::StructToJsonString(strVolInfo, content)) {
        ERRLOG("Convert volume to json failed!");
        return false;
    }
    PluginUtils::CreateDirectory(PathJoin(m_metaFsPath, "volume_info"));
    std::string volumeMetaFile = PathJoin(m_metaFsPath, "volume_info", "volumes_" + m_subJobId + ".json");
    return WriteFile(volumeMetaFile, content);
}

bool WinVolumeBackup::IsLimitedKernel()
{
    // 检查 Windows 内核版本是否支持某些备份/恢复的功能
    // 暂时认为全部支持
    return false;
}

// execute subjob
bool WinVolumeBackup::PreProcess(std::string& snapShotDev)
{
    // 根据 m_volumeInfo里的 volumeName 打快照， 然后把快照的path 塞到m_volumeInfo的path
    INFOLOG("Enter PreProcess: %s", m_volumeName.c_str());
    if (CheckIsEFI(m_volumeName)) {
        return true;
    }
    std::string volumeName = GetValidVolumeName(m_volumeName);
    std::vector<std::string> volumeVec{ volumeName };
    VssClient client;
    SnapshotSetResult res;
    std::string snapshotPercent = m_advParms.snapshotPercent;
    std::optional<SnapshotSetResult> snapRes = client.CreateSnapshots(volumeVec, snapshotPercent);
    std::string logLabel = "file_plugin_volume_backup_prepare_create_snap_failed_label";
    if (!snapRes) {
        ERRLOG("create snapshot failed for volume!");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        if (client.isVolumeFull) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
                "file_plugin_volume_backup_prepare_create_snap_win_space_insufficent_label", volumeName);
        }
        return false;
    }
    PrintSnapRes(snapRes.value());

    Win32VolumesDetail volume = Win32VolumesDetail(Utf8ToUtf16(volumeName));
    std::optional<std::string> volumePath = volume.GetVolumeDeviceName();
    if (!volumePath) {
        ERRLOG("Failed to retrive volume path");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        return false;
    }
    m_volumePath =  volumePath.value();
    INFOLOG("set volumePath : %s", volumeName.c_str());

    auto snapshotRes = snapRes.value();
    std::wstring snapshotIDStr = snapshotRes.m_wSnapshotIDList[0];
    std::string snapshotID(snapshotIDStr.begin(), snapshotIDStr.end());
    snapShotDev = snapshotID;
    INFOLOG("snapShotId: %s", snapshotID.c_str());
    std::optional<VssSnapshotProperty> snapshotPropertyOpt = client.GetSnapshotProperty(snapshotID);
    if (!snapshotPropertyOpt) {
        ERRLOG("Failed to retrieve snapshot properties.");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, logLabel, volumeName);
        return false;
    }
    const auto& snapshotProperty = snapshotPropertyOpt.value();
    res.m_wSnapshotSetID = snapshotProperty.SnapshotDeviceObjectW();
    m_snapShotDev = Utf16ToUtf8(snapshotProperty.SnapshotDeviceObjectW());
    RecordResidualSnapshots({ snapshotID });
    INFOLOG("snapShotID:%s, snapShotDev: %s", snapshotID.c_str(), m_snapShotDev.c_str());
    return true;
}

volumeprotect::CopyFormat WinVolumeBackup::CopyFormat()
{
    return volumeprotect::CopyFormat::VHDX_DYNAMIC;
}

void WinVolumeBackup::FillVolumeInfo(const VolumeBackupSubJob& backupSubJob)
{
    m_volumeName = backupSubJob.volumeName;
    m_volumePath = backupSubJob.volumePath;
    INFOLOG("Get volume name : %s", m_volumeName.c_str());
}

void WinVolumeBackup::FillBackupConfig(VolumeBackupConfig &backupParams)
{
    backupParams.backupType = IsFullBackup() ? volumeprotect::BackupType::FULL : volumeprotect::BackupType::FOREVER_INC;
    backupParams.copyFormat = CopyFormat();
    backupParams.volumePath = m_snapShotDev;
    backupParams.shareName = m_dataFs.remoteName;
    backupParams.prevCopyMetaDirPath = m_prevMetaPath + SEP + m_volumeName;
    backupParams.outputCopyMetaDirPath = m_metaFilePath + SEP + m_volumeName;
    backupParams.outputCopyDataDirPath = m_dataFsPath + SEP + m_volumeName;

    backupParams.blockSize = m_blockSize;
    backupParams.sessionSize = m_sessionSize;
    backupParams.hasherNum = volumeprotect::DEFAULT_HASHER_NUM;
    backupParams.hasherEnabled = true;
    backupParams.enableCheckpoint = false;
    INFOLOG("backup config: volumePath: %s, DataDirPath: %s, MetaDirPath: %s, remoteName: %s", backupParams.volumePath.c_str(),
            backupParams.outputCopyDataDirPath.c_str(), backupParams.outputCopyMetaDirPath.c_str(), backupParams.shareName.c_str());
    return;
}

// post job
void WinVolumeBackup::DeleteSnapshot()
{
    std::vector<std::string> snapshotNames;
    if (!GetSnapshots(snapshotNames)) {
        ERRLOG("Get snapshot info failed!");
        return;
    }
    std::vector<std::string> deleteFailedSnapshot;
    std::vector<std::string> deleteSucceedSnapshot;
    for (const std::string& snapshotName : snapshotNames) {
        VssClient client;
        if (!client.DeleteSnapshot(snapshotName)) {
            WARNLOG("delete snapshot failed, name: %s", snapshotName.c_str());
            deleteFailedSnapshot.push_back(snapshotName);
            continue;
        }
        INFOLOG("delete snapshot succeed, name: %s", snapshotName.c_str());
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

bool WinVolumeBackup::MergeVolumeInfo()
{
    std::string sourceMetaJsonDir = PathJoin(m_metaFsPath, "volume_info");
    std::string targetMetaJson = PathJoin(m_metaFsPath, "volumes.json");
    std::vector<std::string> fileList;
    if (!GetFileListInDirectory(sourceMetaJsonDir, fileList)) {
        ERRLOG("Get file list failed! soucedir : %s", sourceMetaJsonDir.c_str());
        return false;
    }
    Json::Value volumesArray;
    for (std::string& fileName : fileList) {
        StringVolumeInfo volumeInfo;
        if (!JsonFileTool::ReadFromFile(fileName, volumeInfo)) {
            ERRLOG("Read file failed! file : %s", fileName.c_str());
            continue;
        }
        std::string content;
        m_copiedVolumes.push_back(volumeInfo);
        if (!Module::JsonHelper::StructToJsonString(volumeInfo, content)) {
            ERRLOG("Convert volume to json failed!");
            continue;
        }
        volumesArray.append(content);
    }
    Json::FastWriter jsonWriter;
    std::string volumesArrayJosnStr = jsonWriter.write(volumesArray);
    std::string volumeMetaFile = PathJoin(m_metaFsPath, "volumes.json");
    return WriteFile(volumeMetaFile, volumesArrayJosnStr);
}

bool WinVolumeBackup::FillCopyInfo(std::string& extendInfo)
{
    VolumeInfoSet volumeInfoSet;
    for (const StringVolumeInfo& volumeInfo : m_copiedVolumes) {
        VolumeInfomation volumeInfomation;
        volumeInfomation.volumePath = volumeInfo.drivePath;
        volumeInfomation.mountPoint = volumeInfo.displayName;
        volumeInfomation.mountType = volumeInfo.fileSystem;
        volumeInfomation.volumeName = volumeInfo.volumeName;
        volumeInfomation.volumeType = volumeInfo.volumeType;
        volumeInfomation.size = volumeInfo.totalSize;
        INFOLOG("volumeInfomation, volumePath:%s, volumeName:%s, size:%llu", volumeInfomation.volumePath.c_str(),
            volumeInfomation.volumeName.c_str(), volumeInfomation.size);
        volumeInfoSet.volumeInfoSet.push_back(volumeInfomation);
    }
    if (!Module::JsonHelper::StructToJsonString(volumeInfoSet, extendInfo)) {
        ERRLOG("volumeInfoSet change to json failed");
        return false;
    }
    INFOLOG("VolumeInfoSet:%s", extendInfo.c_str());
    return true;
}

bool WinVolumeBackup::CheckIsEFI(const std::string& volumeName)
{
    if (volumeName == EFI_SYSTEM_PARTITION) {
        m_snapShotDev = GetEFIDrivePath();
        return true;
    }
    std::string validVolumeName = GetValidVolumeName(volumeName);
    Win32Handler handler;
    std::vector<WinVolumeInfo> volumes = handler.GetAllVolumes();
    WinVolumeInfo tmp;
    for (const WinVolumeInfo& vol : volumes) {
        if (Utf16ToUtf8(vol.volumeName) == validVolumeName) {
            tmp = vol;
            break;
        }
    }
    if (Utf16ToUtf8(tmp.fileSystem) == "FAT32") {
        INFOLOG("set volumePath: %s", Utf16ToUtf8(tmp.drivePath).c_str());
        m_snapShotDev = Utf16ToUtf8(tmp.drivePath);
        return true;
    }
    return false;
}