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
#include "LinuxVolumeFileGranularRestore.h"
#include <vector>
#include <string>
#include <map>
#include <algorithm>
#include "BasicJob.h"
// Module includes
#include "define/Types.h"
#include "system/System.hpp"
#include "log/Log.h"
// Backup includes
#include "VolumeCopyMountProvider.h"
// plugin includes
#include "PluginConstants.h"
#include "HostCommonStruct.h"
#include "VolumeCommonService.h"
#include "VolumeUtils.h"
#include "PluginUtilities.h"
#include "VolumeFileGranularRestore.h"

using namespace std;
using namespace PluginUtils;
using namespace FilePlugin;
using namespace AppProtect;
using namespace volumeprotect;

namespace {
    const auto MODULE = "LinuxVolumeFileGranularRestore";

    constexpr uint64_t NUMBER5 = 5L;
    constexpr uint64_t NUMBER10 = 10L;
    constexpr uint64_t NUMBER100 = 100L;
    constexpr uint64_t NUMBER1024 = 1024L;
    constexpr uint64_t NUMBER4000 = 4000L;
    constexpr uint64_t NUMBER10000 = 10000L;
    const char SLASH_CH = '/';
    const std::string SYS_BOOT_VOLUME = "boot";
}

bool LinuxVolumeFileGranularRestore::InitSubJobInfo(
    SubJob &subJob, const std::string& ctrlPath, const std::string& volumeName)
{
    DBGLOG("Enter InitSubJobInfo, ctrlPath %s, volume %s", ctrlPath.c_str(), volumeName.c_str());
    std::string subJobName;
    int subJobPrio = 0;
    std::string restoreSubJobInfoStr;
    VolumeFileGranularRestoreInfo restoreSubJobInfo {};

    restoreSubJobInfo.volumeName = volumeName;
    restoreSubJobInfo.ctrlFilePath = ctrlPath;
    restoreSubJobInfo.metaDirPath = GetScanMetaDirPath(volumeName);
    restoreSubJobInfo.dstRootPath = GetRestoreTargetPath();
    restoreSubJobInfo.srcRootPath = PluginUtils::GetPathName(GetRestoreSrcRootPath(volumeName));
    restoreSubJobInfo.trimDstRootPathPrefix = "";

    std::string uniqueId = std::to_string(m_idGenerator->GenerateId());
    std::string fileName = PluginUtils::GetFileName(ctrlPath);
    if (fileName.find(HARDLINK_CTRL_PREFIX) != std::string::npos) {
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK;
        subJobName = SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_HARDLINK + uniqueId;
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK;
        subJobPrio = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK_PRIO;
    } else if (fileName.find(MTIME_CTRL_PREFIX) != std::string::npos) {
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME;
        subJobName = SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_MTIME + uniqueId;
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME;
        subJobPrio = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME_PRIO;
    } else if (fileName.find(CONTROL_CTRL_PREFIX) != std::string::npos) {
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY;
        subJobName = SUBJOB_NAME_VOLUME_GRANULAR_RESTORE_COPY + uniqueId;
        restoreSubJobInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY;
        subJobPrio = SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY_PRIO;
    } else {
        ERRLOG("Get SubJob Type By FileName failed");
        return false;
    }
    Module::JsonHelper::StructToJsonString(restoreSubJobInfo, restoreSubJobInfoStr);
    INFOLOG("ctrl %s, subJobInfo %s, volume %s", ctrlPath.c_str(), restoreSubJobInfoStr.c_str(), volumeName.c_str());
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobName(subJobName);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreSubJobInfoStr);
    subJob.__set_jobPriority(subJobPrio);
    subJob.__set_ignoreFailed(false);
    return true;
}

static void GeneratedCopyCtrlFileCb(void* /* usrData */, std::string ctrlFile)
{
    INFOLOG("Generated copy ctrl file for volume granular restore: %s", ctrlFile.c_str());
}

static void GeneratedHardLinkCtrlFileCb(void* /* usrData */, std::string ctrlFile)
{
    INFOLOG("Generated hard link ctrl file for volume granular restore: %s", ctrlFile.c_str());
}

void LinuxVolumeFileGranularRestore::FillGranularRestoreScanConfig(
    ScanConfig& scanConfig, const std::string& metaPath, const std::string& outputControlDirPath)
{
    // 1. fill basic config
    INFOLOG("FillGranularRestoreScanConfig, using metaPath %s, outputControlDirPath %s",
        metaPath.c_str(), outputControlDirPath.c_str());
    scanConfig.reqID  = PluginUtils::GenerateHash(m_jobId);
    scanConfig.jobId = m_jobId;
    scanConfig.curDcachePath  = metaPath;
    scanConfig.metaPathForCtrlFiles = outputControlDirPath;
    scanConfig.scanType = ScanJobType::CONTROL_GEN;
    scanConfig.scanIO = IOEngine::DEFAULT;
    scanConfig.generatorIsFull = true;
    scanConfig.scanCheckPointEnable = false;
    scanConfig.maxOpendirReqCount = NUMBER4000;
    scanConfig.maxCommonServiceInstance = 1;
    scanConfig.maxWriteQueueSize = NUMBER10000;
    scanConfig.triggerTime = PluginUtils::GetCurrentTimeInSeconds();
    scanConfig.usrData = (void*)this;
    scanConfig.scanResultCb = GeneratedCopyCtrlFileCb;
    scanConfig.scanHardlinkResultCb = GeneratedHardLinkCtrlFileCb;

    /* Inhert host file scanner configuration */
    scanConfig.scanCopyCtrlFileSize = Module::ConfigReader::getInt("FilePluginConfig", "PosixCopyCtrlFileSize");
    scanConfig.scanCtrlMaxDataSize = Module::ConfigReader::getString("FilePluginConfig", "PosixMaxCopyCtrlDataSize");
    scanConfig.scanCtrlMinDataSize = Module::ConfigReader::getString("FilePluginConfig", "PosixMinCopyCtrlDataSize");
    scanConfig.scanCtrlFileTimeSec = NUMBER5;
    scanConfig.scanCtrlMaxEntriesFullBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMaxCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMaxEntriesIncBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMaxCopyCtrlEntriesIncBackup");
    scanConfig.scanCtrlMinEntriesFullBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMinCopyCtrlEntriesFullBackup");
    scanConfig.scanCtrlMinEntriesIncBkup =
        Module::ConfigReader::getInt("FilePluginConfig", "PosixMinCopyCtrlEntriesIncBackup");
    scanConfig.scanMetaFileSize = NUMBER1024 * NUMBER1024 * NUMBER1024; // one GB

    // 2. add filter rule
    for (const auto& restoreSubObject : m_jobInfoPtr->restoreSubObjects) {
        std::string path = restoreSubObject.name; // check if last charactor is '/' to determine if is dir or not
        if (path.back() != SLASH_CH) { // file
            INFOLOG("volume granular restore, push file filter rule: %s", path.c_str());
            scanConfig.fCtrlFltr.push_back(path);
        } else { // dir
            INFOLOG("volume granular restore, push dir filter rule: %s", path.c_str());
            scanConfig.dCtrlFltr.push_back(path);
        }
    }
}

bool LinuxVolumeFileGranularRestore::GenerateRestoreExecuteSubJob(
    const std::vector<std::string>& controlFileList, const std::string& volumeName)
{
    DBGLOG("GenerateRestoreExecuteSubJob volumeName %s", volumeName.c_str());
    if (controlFileList.empty()) {
        return true;
    }
    std::vector<SubJob> subJobList;
    std::vector<std::string> tempContrlFileList;
    std::string outputCtrlDirPath = GetScanOutputCtrlDirPath(volumeName);
    std::string restoreCtrlDirPath = GetRestoreCtrlDirPath(volumeName);
    DBGLOG("move control files from %s to %s", outputCtrlDirPath.c_str(), restoreCtrlDirPath.c_str());
    for (const std::string& controlFilePath : controlFileList) {
        std::string fileName = PluginUtils::GetFileName(controlFilePath);
        std::string restoreControlfilePath = PluginUtils::PathJoin(restoreCtrlDirPath, fileName);
        DBGLOG("perform copy scanctrl %s => restorectrl %s", controlFilePath.c_str(), restoreControlfilePath.c_str());
        if (!PluginUtils::CopyFile(controlFilePath, restoreControlfilePath)) {
            return false;
        }
        SubJob subJob {};
        if (!InitSubJobInfo(subJob, restoreControlfilePath, volumeName)) {
            ERRLOG("Init subjob struct failed, %s (%s)", restoreControlfilePath.c_str(), volumeName.c_str());
            return false;
        }
        subJobList.push_back(subJob);
        tempContrlFileList.push_back(controlFilePath);
        // We create 10 Jobs at a time. If 10 is not accumulated, continue
        if (subJobList.size() % NUMBER10 != 0) {
            continue;
        }
        if (!ReportSubJobToAgent(subJobList, tempContrlFileList)) {
            ERRLOG("Exit GenerateRestoreExecuteSubJob, Create subtask failed");
            return false;
        }
        subJobList.clear();
        tempContrlFileList.clear();
    }
    if (!ReportSubJobToAgent(subJobList, tempContrlFileList)) {
        ERRLOG("Exit GenerateRestoreExecuteSubJob, Create subtask failed");
        return false;
    }
    return true;
}

bool LinuxVolumeFileGranularRestore::GenerateSubTaskFromDCacheFCache(const std::string& volumeName)
{
    // 1. init scanner
    ScanConfig scanConfig {};
    std::string scanMetaDirPath = GetScanMetaDirPath(volumeName);
    std::string scanOutputCtrlDirPath = GetScanOutputCtrlDirPath(volumeName);
    std::string restoreOutputCtrlDirPath = GetRestoreCtrlDirPath(volumeName);
    PluginUtils::CreateDirectory(scanOutputCtrlDirPath);
    PluginUtils::CreateDirectory(restoreOutputCtrlDirPath);
    INFOLOG("generate subtask from dcache/fcache using volume %s, metaPath : %s, ctrlPath : %s",
        volumeName.c_str(), scanMetaDirPath.c_str(), scanOutputCtrlDirPath.c_str());
    FillGranularRestoreScanConfig(scanConfig, scanMetaDirPath, scanOutputCtrlDirPath);
    m_scanner = ScanMgr::CreateScanInst(scanConfig);
    if (m_scanner == nullptr || m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
        ERRLOG("failed to start scanner instance");
        return false;
    }
    // 2. scan and generate subtask
    if (!WaitScannerTerminate(volumeName)) {
        WARNLOG("scanner terminte abnormaly with status %d", m_scanner->GetStatus());
    }
    // 3. accumulate total scan statistics
    ScanStatistics currentVolumeScanStatistic = m_scanner->GetStatistics();
    m_scannerStatistic.mScanDuration += currentVolumeScanStatistic.mScanDuration;
    m_scannerStatistic.mTotDirs += currentVolumeScanStatistic.mTotDirs;
    m_scannerStatistic.mTotFiles += currentVolumeScanStatistic.mTotFiles;
    m_scannerStatistic.mTotalSize += currentVolumeScanStatistic.mTotalSize;
    m_scannerStatistic.mTotDirsToBackup += currentVolumeScanStatistic.mTotDirsToBackup;
    m_scannerStatistic.mTotFilesToBackup += currentVolumeScanStatistic.mTotFilesToBackup;
    m_scannerStatistic.mTotFilesDeleted += currentVolumeScanStatistic.mTotFilesDeleted;
    m_scannerStatistic.mTotDirsDeleted += currentVolumeScanStatistic.mTotDirsDeleted;
    m_scannerStatistic.mTotalSizeToBackup += currentVolumeScanStatistic.mTotalSizeToBackup;
    m_scannerStatistic.mTotalControlFiles += currentVolumeScanStatistic.mTotalControlFiles;
    m_scannerStatistic.mTotFailedDirs += currentVolumeScanStatistic.mTotFailedDirs;
    m_scannerStatistic.mTotFailedFiles += currentVolumeScanStatistic.mTotFailedFiles;
    // 4. clean scanner
    m_scanner->Destroy();
    m_scanner.reset();
    return true;
}

bool LinuxVolumeFileGranularRestore::SetupMounts()
{
    std::string driveInfo;
    if (!MountNasShare(m_dataFs, m_dataFsPersistMountTarget, m_jobInfoPtr->extendInfo, driveInfo)) {
        ERRLOG("mount persist dataFs failed, jobId %s", m_jobId.c_str());
        return false;
    }
    std::vector<std::string> volumeNameList = GetAllvolumeNameList(m_dataFsPersistMountTarget);
    std::string volumeMountRecordPath;
    for (const std::string& volumeName : volumeNameList) {
        if (volumeName == SYS_BOOT_VOLUME) {
            INFOLOG("skip mount system boot volume in %s", m_dataFsPersistMountTarget.c_str());
            continue;
        }
        if (!MountSingleVolumeReadOnly(
            volumeName,
            PluginUtils::PathJoin(m_dataFsPersistMountTarget, volumeName),
            PluginUtils::PathJoin(m_volumesMountTargetRoot, volumeName),
            PluginUtils::PathJoin(m_volumesMountRecordRoot, volumeName),
            volumeMountRecordPath)) {
            return false;
        }
    }
    return true;
}

int LinuxVolumeFileGranularRestore::ExecuteVolumeGranularTearDownSubJob()
{
    INFOLOG("Enter ExecuteVolumeGranularTearDownSubJob JobId %s subJobId %s",
        m_jobId.c_str(), m_subJobId.c_str());
    BackupStatistic mainBackupStats {};
    if (!CalcuMainBackupStats(mainBackupStats)) {
        ERRLOG("UpdateMainBackupStats failed");
        return Module::FAILED;
    }
    m_dataSize =  CalculateSizeInKB(mainBackupStats.noOfBytesCopied);
    INFOLOG("noOfDirsFailed: %llu, noOfFilesFailed: %llu, m_jobId: %s",
        mainBackupStats.noOfDirFailed, mainBackupStats.noOfFilesFailed, m_jobId.c_str());
    if (mainBackupStats.noOfDirFailed != 0 || mainBackupStats.noOfFilesFailed != 0) {
        ReportJobLabel(JobLogLevel::TASK_LOG_WARNING,
            "file_plugin_host_restore_data_completed_with_warn_label",
            to_string(mainBackupStats.noOfDirCopied),
            to_string(mainBackupStats.noOfFilesCopied),
            PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied),
            to_string(mainBackupStats.noOfDirFailed),
            to_string(mainBackupStats.noOfFilesFailed));
    } else {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO,
            "file_plugin_host_restore_data_completed_label",
            to_string(mainBackupStats.noOfDirCopied),
            to_string(mainBackupStats.noOfFilesCopied),
            PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied));
    }
    return Module::SUCCESS;
}