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
#include "LinuxVolumeRestore.h"
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "PluginConstants.h"
#include "define/Types.h"
#include "system/System.hpp"
#include "VolumeCommonService.h"
#include "VolumeUtils.h"
#include "PluginUtilities.h"
#include "log/Log.h"
#include "BareMetalRecovery.h"

using namespace std;
using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace AppProtect;

namespace FilePlugin {

namespace {
const auto MODULE = "LinuxVolumeRestore";
const std::string BMR_SUCCEED_FLAG_SUFFIX = "_bmr_succeed";
const std::string BMR_LIVE_OS_FLAG_PATH = "/etc/databackup-bmr-livecd";
const std::string FILE_SYSTEM_EXT2 = "ext2";
const std::string FILE_SYSTEM_EXT3 = "ext3";
const std::string FILE_SYSTEM_EXT4 = "ext4";
const std::string FILE_SYSTEM_XFS = "xfs";
}  // namespace

int LinuxVolumeRestore::PrerequisiteJobInner()
{
    INFOLOG("Enter PrerequisiteJobInner");
    if (!InitInfo()) {
        return Module::FAILED;
    }
    if (m_enableBareMetalRestore && !CheckBMRCompatible()) {
        return Module::FAILED;
    }
    if (m_enableBareMetalRestore) {
        ReportBMRConfigurationLabel();
    }
    if (!GetUmountVolumeInfo()) {
        ERRLOG("umount volume failed");
        return Module::FAILED;
    }
    if (!WriteMountInfoToFile(m_mountInfoFilePath)) {
        ERRLOG("Write mount info to file failed");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool LinuxVolumeRestore::CheckBMRCompatible()
{
    INFOLOG("Enter CheckBMRCompatible");
    if (!PluginUtils::IsPathExists(BMR_LIVE_OS_FLAG_PATH)) {
        ERRLOG("not suppport to perform BMR on non live os, %s not exists", BMR_LIVE_OS_FLAG_PATH.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_bmr_not_a_live_linux_label");
        return false;
    }
    // check platform compatible
    std::vector<std::string> output;
    std::vector<std::string> errput;
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, "uname -m", {}, output, errput) != 0 || output.empty()) {
        ERRLOG("uknown platform, uname -m exec failed");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_bmr_incompatible_arch");
        return false;
    }
    std::string currentOsPlatform = output.front();
    std::string copyOsPlatform = PluginUtils::ReadFileContent(PluginUtils::PathJoin(m_sysInfoPath, "cpu_arch"));
    const char *trimStr = " \t\n\r\f\v";
    currentOsPlatform.erase(currentOsPlatform.find_last_not_of(trimStr) + NUM1);
    currentOsPlatform.erase(NUM0, currentOsPlatform.find_first_not_of(trimStr));
    copyOsPlatform.erase(copyOsPlatform.find_last_not_of(trimStr) + NUM1);
    copyOsPlatform.erase(NUM0, copyOsPlatform.find_first_not_of(trimStr));
    if (currentOsPlatform != copyOsPlatform) {
        ERRLOG("platform missmatch (%s) (%s)", currentOsPlatform.c_str(), copyOsPlatform.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_bmr_incompatible_arch");
        return false;
    }
    return true;
}

bool LinuxVolumeRestore::BareMetalRestore()
{
    if (!m_enableBareMetalRestore) {
        return true;
    }
    std::string diskMapInfoSet;
    if (!InitDiskMapInfo(diskMapInfoSet)) {
        ERRLOG("InitDiskMapInfo failed");
        return false;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_cleaning_residual_lvm_pv_vg_lv_warning_label");
    INFOLOG("Enter cleaning residual lvm pv/vg/lv");
    std::vector<std::string> output;
    std::vector<std::string> errput;
    std::string cmd = R"(lvs --noheadings | awk '{print "/dev/"$2"/"$1}' | xargs -i lvremove -f {}; )"
                      R"(vgs --noheadings | awk '{print $1}' | xargs -i vgremove -f {}; )"
                      R"(pvs --noheadings | awk '{print $1}' | xargs -i pvremove -f {})";
    if (Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput) != 0) {
        WARNLOG("cleaning residual lvm pv/vg/lv failed, cmd:%s", cmd.c_str());
    }
    INFOLOG("Exit cleaning residual lvm pv/vg/lv");

    BareMetalRecoveryConfig config{};
    config.sysInfoPath = m_sysInfoPath;
    INFOLOG("+++ meta path:%s", m_metaFsPath.c_str());
    config.jobId = m_jobId;
    BareMetalRecovery bmrTask(config);
    if (!bmrTask.SetDiskMapInfo(diskMapInfoSet)) {
        ERRLOG("BareMetalRecovery setDiskMapInfo Failed!");
        return false;
    }
    if (bmrTask.Start() != BackupRetCode::SUCCESS) {
        ERRLOG("BareMetalRecovery Start Failed!");
        return false;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_disk_partition_and_lvm_config_recovered_label");
    return true;
}

bool LinuxVolumeRestore::InitDiskMapInfo(std::string &diskMapInfoSet)
{
    std::string extJsonString = m_jobInfoPtr->extendInfo;
    INFOLOG("Extend info json string: %s", extJsonString.c_str());
    Json::Value value;
    if (!Module::JsonHelper::JsonStringToJsonValue(extJsonString, value)) {
        ERRLOG("Convert to extJsonValue failed.");
        return false;
    }
    if (!(value.isObject() && value.isMember("diskMap") && value["diskMap"].isString())) {
        ERRLOG("json change failed");
        return false;
    }
    diskMapInfoSet = value["diskMap"].asString();
    INFOLOG("diskMapInfoSet json string: %s", diskMapInfoSet.c_str());
    return true;
}

bool LinuxVolumeRestore::BareMetalRestoreRepairGrub()
{
    if (!m_enableBareMetalRestore) {
        return true;
    }
    BareMetalRecoveryConfig config{};
    config.sysInfoPath = m_sysInfoPath;
    INFOLOG("+++ meta path:%s", m_metaFsPath.c_str());
    config.jobId = m_jobId;
    BareMetalRecovery bmrTask(config);
    std::string diskMapInfoSet;
    if (!InitDiskMapInfo(diskMapInfoSet)) {
        ERRLOG("InitDiskMapInfo failed");
        return false;
    }
    if (!bmrTask.SetDiskMapInfo(diskMapInfoSet)) {
        ERRLOG("BareMetalRecovery setDiskMapInfo Failed!");
        return false;
    }
    if (!bmrTask.FormatFileSystemForBootPartition()) {
        ERRLOG("BareMetalRecovery FormatFileSystemForBootPartition Failed!");
        return false;
    }
    if (!bmrTask.RestoreFsAndMountInfo()) {
        ERRLOG("BareMetalRecovery RestoreFsAndMountInfo Failed!");
        return false;
    }
    if (!bmrTask.CopyBoot(m_dataFsPath + "/boot")) {
        ERRLOG("BareMetalRecovery RestoreFsAndMountInfo Failed!");
        return false;
    }
    if (!bmrTask.MountBindSysProcDev()) {
        ERRLOG("BareMetalRecovery MountBindSysProcDev Failed!");
        return false;
    }
    if (!bmrTask.RebuildInitramfs()) {
        WARNLOG("BareMetalRecovery RebuildInitramfs failed! Booting may fail!");
    }
    if (!bmrTask.GenerateNewGrubCfg()) {
        ERRLOG("BareMetalRecovery GenerateNewGrubCfg Failed!");
        return false;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_startups_recovered_label");
    PluginUtils::CreateDirectory("/tmp");
    std::string bmrSucceedFlagFilePath = PluginUtils::PathJoin("/tmp", m_jobId + BMR_SUCCEED_FLAG_SUFFIX);
    std::ofstream bmrSucceedFlagFile(bmrSucceedFlagFilePath);
    bmrSucceedFlagFile.close();
    return true;
}

bool LinuxVolumeRestore::IsSystemVolume(const std::string &volumeName) const
{
    std::string volumeMountArgsEntriesJsonPath =
        PluginUtils::PathJoin(m_dataFsPath, volumeName, VOLUME_MOUNT_ENTRIES_JSON_NAME);
    VolumeMountEntries volumeMountEntries{};
    if (!JsonFileTool::ReadFromFile(volumeMountArgsEntriesJsonPath, volumeMountEntries)) {
        ERRLOG("failed to read volume (%s) mount args entries json from %s",
            volumeName.c_str(),
            volumeMountArgsEntriesJsonPath.c_str());
        return false;
    }
    if (volumeMountEntries.mountEntries.empty()) {
        INFOLOG("volume (%s) mount args entries empty, not system volume", volumeName.c_str());
        return false;
    }
    return (std::find_if(volumeMountEntries.mountEntries.begin(),
                volumeMountEntries.mountEntries.end(),
                [&](const VolumeMountEntry &mountEntry) {
                    return IsSystemVolumeMountPath(mountEntry.mountTargetPath);
                }) != volumeMountEntries.mountEntries.end());
}

bool LinuxVolumeRestore::IsSystemVolumeMountPath(const std::string &mountPath) const
{
    std::vector<std::string> sysVolMountPoints{
        "/bin", "/etc", "/lib", "/lib64", "/opt", "/root", "/sbin", "/usr", "/var", "/home"};
    auto prefixChecker = [&mountPath](const std::string &systemVolMountPoint) {
        return mountPath.find(systemVolMountPoint + "/") == 0;
    };
    return (mountPath == "/" ||
            std::find(sysVolMountPoints.begin(), sysVolMountPoints.end(), mountPath) != sysVolMountPoints.end() ||
            std::find_if(sysVolMountPoints.begin(), sysVolMountPoints.end(), prefixChecker) != sysVolMountPoints.end());
}

int LinuxVolumeRestore::ExecuteTearDownVolume()
{
    INFOLOG("Enter Linux ExecuteTearDownVolume");
    if (!BareMetalRestoreRepairGrub()) {
        return Module::FAILED;
    }
    if (!ReportBackupCompletionStatus()) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int LinuxVolumeRestore::PostJobInner()
{
    if (!InitInfo()) {
        return Module::FAILED;
    }
    if (!MountInitalVolume()) {
        ERRLOG("mount volume failed");
        return Module::FAILED;
    }
    if (m_enableBareMetalRestore) {
        std::string bmrSucceedFlagFilePath = PluginUtils::PathJoin("/tmp", m_jobId + BMR_SUCCEED_FLAG_SUFFIX);
        if (PluginUtils::IsPathExists(bmrSucceedFlagFilePath)) {
            ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_completed_label");
            RebootSystem();
        } else {
            WARNLOG("succeed flag %s not detected, BMR fail, not reboot system.", bmrSucceedFlagFilePath.c_str());
        }
    }
    INFOLOG("Exit PostJobInner");
    return Module::SUCCESS;
}

bool LinuxVolumeRestore::RebootSystem()
{
    if (!m_rebootSystemAfterRestore) {
        return false;
    }
    INFOLOG("Reboot System");
    std::string cmd = "shutdown -r +1";
    // to do pm上添加挂载失败卷信息
    std::vector<std::string> output;
    std::vector<std::string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("reboot failed, cmd:%s", cmd.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_bmr_reboot_failed");
        return false;
    }
    INFOLOG("reboot success, cmd:%s", cmd.c_str());
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_ready_to_reboot_in_one_minute_label");
    return MountVolume();
}

bool LinuxVolumeRestore::MountInitalVolume()
{
    INFOLOG("Enter MountInitalVolume");
    ReadMountInfoFromFile(m_mountInfoFilePath);
    return MountVolume();
}

bool LinuxVolumeRestore::MountVolume()
{
    for (const MountInfo &mountInfo : m_mountInfoSet) {
        if (mountInfo.mountOper.find("rw") != string::npos) {
            if (!ModifyVolumeInfo(mountInfo.deviceName, "rw")) {
                WARNLOG("modify volume info failed, or volume is already rw");
            }
        }
        std::string mountCmd =
            "mount -o " + mountInfo.mountOper + " " + mountInfo.deviceName + " " + mountInfo.mountPoint;
        // to do pm上添加挂载失败卷信息
        std::vector<std::string> output;
        std::vector<std::string> errput;
        int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, mountCmd, {}, output, errput);
        if (ret != 0) {
            ERRLOG("mount failed, mountCmd:%s", mountCmd.c_str());
            ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
                "file_plugin_volume_livemount_volume_mount_failed",
                mountInfo.deviceName,
                mountInfo.mountPoint);
            return false;
        }
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO,
            "file_plugin_volume_livemount_volume_mount_success",
            mountInfo.deviceName,
            mountInfo.mountPoint);
        INFOLOG("mount success, mountCmd:%s", mountCmd.c_str());
    }
    return true;
}

bool LinuxVolumeRestore::ModifyVolumeInfo(const std::string &deviceName, const ::std::string &attribute)
{
    std::string lvWithVg = VolumeNameTransform(deviceName);
    std::string modifyCmd = "lvchange --permission " + attribute + " " + lvWithVg;
    std::vector<std::string> output;
    std::vector<std::string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, modifyCmd, {}, output, errput);
    if (ret != 0) {
        WARNLOG("logical volume attribute modify failed, modifyCmd:%s", modifyCmd.c_str());
        for_each(errput.begin(), errput.end(), [&](const string &v) { ERRLOG("errput: %s", v.c_str()); });
        return false;
    }
    return true;
}

bool LinuxVolumeRestore::CompareVersion(std::string& version1, std::string& version2)
{
    std::istringstream stream1(version1);
    std::istringstream stream2(version2);
    std::string part1;
    std::string part2;
    while (std::getline(stream1, part1, '.') && std::getline(stream2, part2, '.')) {
        int num1 = std::stoi(part1);
        int num2 = std::stoi(part2);
        if (num1 < num2) {
            return true;
        }
        if (num1 > num2) {
            return false;
        }
    }
    return true;
}

bool LinuxVolumeRestore::CompareFilesystemVersion(const std::string &infoFilePath, const std::string &dataDstPath,
    std::string &oriVolumeFilesystemType)
{
    DBGLOG("Enter CompareFilesystemVersion");
    std::string originalVolumeFilesystemVersion;
    std::string targetVolumeFilesystemVersion;
    if (!GetOriVolumeFilesystemInfo(infoFilePath, originalVolumeFilesystemVersion, oriVolumeFilesystemType)) {
        ERRLOG("Failed to retrieve source volume information.");
        return false;
    }
    if (!GetTarVolumeFilesystemInfo(dataDstPath, targetVolumeFilesystemVersion, oriVolumeFilesystemType)) {
        ERRLOG("Failed to retrieve source volume information.");
        return false;
    }
    if (originalVolumeFilesystemVersion == targetVolumeFilesystemVersion) {
        INFOLOG("The versions of the two volumes are same");
        return true;
    }
    if (CompareVersion(originalVolumeFilesystemVersion, targetVolumeFilesystemVersion)) {
        return true;
    }
    DBGLOG("Exit CompareFilesystemVersion");
    return false;
}

bool LinuxVolumeRestore::GetOriVolumeFilesystemInfo(const std::string &infoFilePath,
    std::string &oriVolumeFilesystemVersion,
    std::string &oriVolumeFilesystemType)
{
    DBGLOG("Enter GetOriVolumeFilesystemInfo");
    if (!IsPathExists(infoFilePath)) {
        WARNLOG("infoFile:%s don't exist", infoFilePath.c_str());
        return false;
    }
    ifstream file(infoFilePath);
    string line;
    Json::Value js;
    getline(file, line);
    DBGLOG("read mount info json:%s", line.c_str());//删除
    if (line.empty() || !Module::JsonHelper::JsonStringToJsonValue(line, js)) {
        ERRLOG("empty line or json struct failed");
        file.close();
        return false;
    }
    if (!js.isMember("mountEntries")) {
        ERRLOG("The mountEntries of file is not exist");
        file.close();
        return false;
    }
    try {
        oriVolumeFilesystemType = js["mountEntries"][0]["mountType"].asString();
        oriVolumeFilesystemVersion = js["mountEntries"][0]["mountTypeVersion"].asString();
    } catch(exception &e) {
        ERRLOG("oriVolumeFilesystemVersion error: %s", e.what());
        file.close();
        return false;
    }
    DBGLOG("oriVolumeFilesystemVersion: %s", oriVolumeFilesystemVersion.c_str());
    DBGLOG("oriVolumeFilesystemType: %s", oriVolumeFilesystemType.c_str());
    file.close();
    DBGLOG("Exit GetOriVolumeFilesystemInfo");
    return true;
}

bool LinuxVolumeRestore::GetTarVolumeFilesystemInfo(const std::string &dataDstPath,
    std::string &targetVolumeFilesystemVersion, std::string &oriVolumeFilesystemType)
{
    DBGLOG("Enter GetTarVolumeFilesystemInfo");
    targetVolumeFilesystemVersion = GetMountDevicetype(oriVolumeFilesystemType, dataDstPath, MODULE);
    DBGLOG("Exit GetTarVolumeFilesystemInfo");
    return true;
}

bool LinuxVolumeRestore::GetUmountVolumeInfo()
{
    DBGLOG("Enter VolumeRestore::UmountVolume");
    for (auto restoreInfo : m_restoreInfoSet) {
        std::string  oriVolumeFilesystemType;
        if (!CompareFilesystemVersion(PluginUtils::PathJoin(m_dataFsPath, restoreInfo.volumeName, "volume_mount_entries.json"), restoreInfo.dataDstPath, oriVolumeFilesystemType)) {
            INFOLOG("The versions of the two volumes are inconsistent");
            ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_restore_volume_filesystem_not_consistent");
        }
        std::string mountInfoCmd =
            "mount | awk '$1==\"" + restoreInfo.dataDstPath + "\"' | awk '{print $1,$3,substr($6, 2, length($6)-2) }'";
        DBGLOG("mountInfoCmd:%s", mountInfoCmd.c_str());
        std::vector<std::string> output;
        std::vector<std::string> errput;
        int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, mountInfoCmd, {}, output, errput);
        for (auto out : output) {
            DBGLOG("%s,", out.c_str());
        }
        for (auto err : errput) {
            DBGLOG("%s, ", err.c_str());
        }
        if (ret != 0) {
            continue;
        }
        for (auto out : output) {
            std::vector<std::string> mountInfoStr;
            DBGLOG("mountInfoStr:%s", out.c_str());
            boost::split(mountInfoStr, out, boost::is_any_of(" "), boost::token_compress_on);
            MountInfo mountInfo;
            if (mountInfoStr.size() < NUM3) {
                INFOLOG("out:%s is not the effctive mount information", out.c_str());
                continue;
            }
            mountInfo.deviceName = mountInfoStr[NUM0];
            mountInfo.mountPoint = mountInfoStr[NUM1];
            mountInfo.mountOper = mountInfoStr[NUM2];
            INFOLOG("mountInfo describe, deviceName:%s, mountPoint:%s, mountOper:%s",
                mountInfo.deviceName.c_str(),
                mountInfo.mountPoint.c_str(),
                mountInfo.mountOper.c_str());

            if (!UmountVolume(mountInfo.mountPoint)) {
                continue;
            }
            m_mountInfoSet.push_back(mountInfo);
        }
    }
    return true;
}

bool LinuxVolumeRestore::UmountVolume(const std::string &mountPoint)
{
    std::string umountCmd = "umount " + mountPoint;
    std::vector<std::string> output;
    std::vector<std::string> errput;
    INFOLOG(
        "Start UmountVolume, mountPoint %s , jobId %s, cmd %s", mountPoint.c_str(), m_jobId.c_str(), umountCmd.c_str());
    int ret = Module::runShellCmdWithOutput(INFO, MODULE, 0, umountCmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("umountCmd:%s exeuate failed", umountCmd.c_str());
        for_each(errput.begin(), errput.end(), [&](const string &v) { ERRLOG("errput: %s", v.c_str()); });
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING, "file_plugin_volume_livemount_volume_umount_failed", mountPoint);
        return false;
    }
    INFOLOG("Exit UmountVolume, mountPoint %s, jobId %s", mountPoint.c_str(), m_jobId.c_str());
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_umount_success", mountPoint);
    return true;
}

bool LinuxVolumeRestore::WriteMountInfoToFile(const std::string &infoFilePath)
{
    DBGLOG("Enter writeMountInfoToFile");
    if (m_mountInfoSet.empty()) {
        INFOLOG("there has no device to umount");
        return true;
    }
    if (infoFilePath.empty()) {
        ERRLOG("infoFilePath is empty");
        return false;
    }

    std::vector<string> mountInfoStrSet;
    for (auto mountInfo : m_mountInfoSet) {
        std::string mountInfoStr;
        if (!Module::JsonHelper::StructToJsonString(mountInfo, mountInfoStr)) {
            ERRLOG("cannot change mountInfo to jsonstr");
            continue;
        }
        INFOLOG("mountInfoStr:%s", mountInfoStr.c_str());
        mountInfoStrSet.push_back(mountInfoStr);
    }
    if (!CreateDirectory(GetPathName(infoFilePath))) {
        ERRLOG("creata dir %s failed", infoFilePath.c_str());
        return false;
    }
    ofstream file(infoFilePath, ios::out | ios::trunc);
    ostream_iterator<string> iter(file, "\n");
    copy(mountInfoStrSet.begin(), mountInfoStrSet.end(), iter);
    file.close();
    return true;
}

void LinuxVolumeRestore::ReadMountInfoFromFile(const string &infoFilePath)
{
    DBGLOG("Enter ReadMountInfoFromFile");
    if (!IsPathExists(infoFilePath)) {
        WARNLOG("infoFile:%s don't exist", infoFilePath.c_str());
        return;
    }
    ifstream file(infoFilePath);
    while (file) {
        string line;
        MountInfo mountInfo;
        getline(file, line);
        DBGLOG("read mount info json:%s", line.c_str());
        if (line.empty() || !Module::JsonHelper::JsonStringToStruct(line, mountInfo)) {
            ERRLOG("empty line or json struct failed");
            continue;
        }
        m_mountInfoSet.push_back(mountInfo);
    }
    file.close();
}

void LinuxVolumeRestore::HandleRestoreErrorCode()
{
    if (m_volumeBackupErrorCode == VOLUMEPROTECT_ERR_VOLUME_ACCESS_DENIED) {
        ERRLOG("volume %s not accessible", m_restoreInfo.dataDstPath.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
            "file_plugin_volume_backup_selected_path_not_accessible_label",
            m_restoreInfo.dataDstPath);
    }
    if (m_volumeBackupErrorCode == EBUSY) {
        ERRLOG("volume %s is busy", m_restoreInfo.dataDstPath.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
            "file_plugin_volume_backup_selected_volume_busy_label", m_restoreInfo.dataDstPath);
    }
}

bool LinuxVolumeRestore::CheckBMR(const BareMetalRestoreParam& param)
{
    if (param.enableBareMetalRestore == "true") {
        INFOLOG("=====This is BARE_METAL_RESTORE=====");
        return true;
    }
    return false;
}

}  // namespace FilePlugin