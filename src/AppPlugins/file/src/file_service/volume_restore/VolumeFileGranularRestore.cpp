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
#include "VolumeFileGranularRestore.h"
// stl includes
#include <vector>
#include <string>
#include <map>
#include <algorithm>
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
#include "file_service/volume_index/VolumeIndex.h"
#include "job/ChannelManager.h"

using namespace std;
using namespace PluginUtils;
using namespace AppProtect;
using namespace volumeprotect;
using namespace volumeprotect::mount;

namespace FilePlugin {

#define ENTER                                                                                                         \
    do {                                                                                                              \
        m_mainJobRequestId = GenerateHash(m_jobId);                                                                   \
        INFOLOG("Enter %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str());    \
    } while (0)

#define EXIT                                                                                                          \
    do {                                                                                                              \
        INFOLOG("Exit %s, jobId: %s, subJobId: %s", m_jobCtrlPhase.c_str(), m_jobId.c_str(), m_subJobId.c_str());     \
    } while (0)

namespace {
    const auto MODULE = "VolumeFileGranularRestore";
    const auto CURR_META = "latest";
    constexpr uint64_t NUM0 = 0;
    constexpr uint64_t NUM1 = 1;
    constexpr uint64_t NUM2 = 2;
    constexpr uint64_t NUM3 = 3;
    constexpr uint64_t NUMBER5 = 5L;
    constexpr uint64_t NUMBER10 = 10L;
    constexpr uint64_t NUMBER100 = 100L;
    constexpr uint64_t NUMBER1024 = 1024L;
    constexpr uint64_t NUMBER4000 = 4000L;
    constexpr uint64_t NUMBER10000 = 10000L;
    const std::string DEFAULT_VOLUME_COPY_NAME = "volumeprotect";
    const std::string BACKUP_KEY_SUFFIX = "_backup_stats";

    const std::string RESTORE_OPTION_OVERWRITE = "OVERWRITING";
    const std::string RESTORE_OPTION_SKIP = "SKIP";
    const std::string RESTORE_OPTION_REPLACE = "REPLACE";
    const std::string SYS_BOOT_VOLUME = "boot";
}

uint32_t VolumeFileGranularRestore::m_numberOfSubTask = 0;

EXTER_ATTACK int VolumeFileGranularRestore::PrerequisiteJob()
{
    SetJobCtrlPhase(JOB_CTRL_PHASE_PREJOB);

    ENTER;
    int ret = PrerequisiteJobInner();
    EXIT;

    if (ret != Module::SUCCESS) {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::FAILED, PROGRESS100});
        SetJobToFinish();
        return Module::FAILED;
    }
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    SetJobToFinish();
    return Module::SUCCESS;
}

EXTER_ATTACK int VolumeFileGranularRestore::GenerateSubJob()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    INFOLOG("Enter VolumeFileGranularRestore GenerateSubJob");
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_scan_start_label");
    int ret = GenerateSubJobInner();
    if (ret != Module::SUCCESS) {
        ERRLOG("GenerateSubJobInner failed");
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::FAILED, PROGRESS100});
        SetJobToFinish();
        return Module::FAILED;
    }
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    SetJobToFinish();
    return Module::SUCCESS;
}

EXTER_ATTACK int VolumeFileGranularRestore::ExecuteSubJob()
{
    INFOLOG("Enter VolumeFileGranularRestore ExecuteSubJob");
    string jobId = GetParentJobId();
    string subJobId = GetSubJobId();
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ChannelManager::getInstance().removeSubJob(jobId, subJobId);
        INFOLOG("remove subJob from map, jobId: %s, subJobId: %s", jobId.c_str(), subJobId.c_str());
    });
    
    int ret = ExecuteSubJobInner();

    INFOLOG("Report ExecuteSubJobInner State");
    if (ret != Module::SUCCESS) {
        ERRLOG("ExecuteSubJobInner failed");
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100});
        SetJobToFinish();
        return Module::FAILED;
    }
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    SetJobToFinish();
    return Module::SUCCESS;
}

EXTER_ATTACK int VolumeFileGranularRestore::PostJob()
{
    INFOLOG("Enter VolumeFileGranularRestore PostJob");
    SetJobCtrlPhase(JOB_CTRL_PHASE_POSTJOB);
    int ret = PostJobInner();
    if (ret != Module::SUCCESS) {
        ERRLOG("PostJob failed");
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100});
        SetJobToFinish();
        return Module::FAILED;
    }
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    SetJobToFinish();
    return Module::SUCCESS;
}

int VolumeFileGranularRestore::PrerequisiteJobInner()
{
    INFOLOG("Enter PrerequisiteJobInner");
    if (!InitInfo()) {
        return Module::FAILED;
    }
    if (!SetupMounts()) {
        ERRLOG("setup mounts failed, jobId %s", m_jobId.c_str());
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int VolumeFileGranularRestore::GenerateSubJobInner()
{
    INFOLOG("Enter GenerateSubJobInner");
    if (!InitInfo() || !InitIdGenerator() || !GenerateAllSubTaskFromDCacheFCache() ||
        !GenerateTearDownTask() || !GenerateGenerateInfo()) {
        return Module::FAILED;
    }
    INFOLOG("Exit GenerateSubJobInner");
    return Module::SUCCESS;
}

int VolumeFileGranularRestore::ExecuteSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
    if (!InitInfo() || !InitRestoreSubjobInfo()) {
        return Module::FAILED;
    }

    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_jobId);
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_jobId);
    });
    m_subTaskType = m_restoreInfo.subTaskType;
    if (m_subTaskType == SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN) {
        return ExecuteVolumeGranularTearDownSubJob();
    } else if (m_subTaskType == SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY ||
        m_subTaskType == SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK ||
        m_subTaskType == SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME) {
        return ExecuteVolumeGranularFilePhraseSubJob();
    }
    ERRLOG("invalid subTask Type %u", m_subTaskType);
    return Module::FAILED;
}

bool VolumeFileGranularRestore::InitRestoreSubjobInfo()
{
    std::map<std::string, RestoreReplacePolicy> restoreOptionMap {
        { RESTORE_OPTION_OVERWRITE, RestoreReplacePolicy::OVERWRITE },
        { RESTORE_OPTION_SKIP,      RestoreReplacePolicy::IGNORE_EXIST },
        { RESTORE_OPTION_REPLACE,   RestoreReplacePolicy::OVERWRITE_OLDER }
    };
    std::map<uint32_t, BackupPhase> backupPhraseMap {
        { SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_COPY,     BackupPhase::COPY_STAGE },
        { SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_HARDLINK, BackupPhase::HARDLINK_STAGE },
        { SUBJOB_TYPE_VOLUME_GRANULAR_RESTORE_MTIME,    BackupPhase::DIR_STAGE }
    };
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_restoreInfo)) {
        ERRLOG("subJobInfo exchange failed");
        return false;
    }
    // init cover policy and backup phrase
    RestoreAdvancedParamters restoreAdvancedParamters {};
    if (!Module::JsonHelper::JsonStringToStruct(m_jobInfoPtr->extendInfo, restoreAdvancedParamters)) {
        ERRLOG("convert to RestoreAdvancedParamters json failed");
        return false;
    }
    if (m_restoreInfo.subTaskType == SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN) {
        INFOLOG("subtask is teardown task");
        return true;
    }
    if (restoreOptionMap.find(restoreAdvancedParamters.restoreOption) == restoreOptionMap.end() ||
        backupPhraseMap.find(m_restoreInfo.subTaskType) == backupPhraseMap.end()) {
        ERRLOG("init restore sub job failed, %s, %s",
            m_jobInfoPtr->extendInfo.c_str(), m_subJobInfo->jobInfo.c_str());
        return false;
    }
    m_coveragePolicy = restoreOptionMap[restoreAdvancedParamters.restoreOption];
    m_backupPhase = backupPhraseMap[m_restoreInfo.subTaskType];
    return true;
}

int VolumeFileGranularRestore::ExecuteVolumeGranularTearDownSubJob()
{
    return Module::SUCCESS;
}

int VolumeFileGranularRestore::ExecuteVolumeGranularFilePhraseSubJob()
{
    INFOLOG("Enter ExecuteVolumeGranularFilePhraseSubJob JobId %s subJobId %s info %s",
        m_jobId.c_str(), m_subJobId.c_str(), m_subJobInfo->jobInfo.c_str());
    // init subjob statistics
    BackupStatistic backupStatistic {};
    INFOLOG("set statistics resource path %s , subJobId %s", m_statisticsPath.c_str(), m_subJobId.c_str());
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_subJobId);
    if (!ShareResourceManager::GetInstance().InitResource(ShareResourceType::BACKUP, m_subJobId, backupStatistic)) {
        ERRLOG("init subtask statistics failed, subJob %s", m_subJobId.c_str());
        return Module::FAILED;
    }
    BackupParams backupParams {};
    FillRestoreConfig(backupParams);
    m_backup = FS_Backup::BackupMgr::CreateBackupInst(backupParams);
    if (m_backup == nullptr) {
        ERRLOG("Create backup instance failed");
        return Module::FAILED;
    }
    if (m_backup->Enqueue(m_restoreInfo.ctrlFilePath) != BackupRetCode::SUCCESS) {
        ERRLOG("enqueue backup instance failed");
        return Module::FAILED;
    }
    if (m_backup->Start() != BackupRetCode::SUCCESS) {
        ERRLOG("Start backup task failed");
        return Module::FAILED;
    }
    // monitor
    if (!WaitBackupComplete()) {
        ERRLOG("backup monitor failed");
        return Module::FAILED;
    }
    // clean
    if (m_backup != nullptr) {
        m_backup->Destroy();
        m_backup.reset();
    }
    return Module::SUCCESS;
}

bool VolumeFileGranularRestore::GenerateAllSubTaskFromDCacheFCache()
{
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_host_backup_scan_start_label");
    std::vector<std::string> volumeNameList = GetAllvolumeNameList(m_dataFsPath);
    int failedCnt = 0;
    for (const std::string& volumeName : volumeNameList) {
        if (!GenerateSubTaskFromDCacheFCache(volumeName)) {
            return false;
        }
    }
    ReportJobLabel(
        JobLogLevel::TASK_LOG_INFO,
        "file_plugin_host_restore_scan_data_completed_label",
        to_string(m_scannerStatistic.mTotDirsToBackup),
        to_string(m_scannerStatistic.mTotFilesToBackup),
        PluginUtils::FormatCapacity(m_scannerStatistic.mTotalSizeToBackup));
    return true;
}

bool VolumeFileGranularRestore::GenerateSubTaskFromDCacheFCache(const std::string& volumeName)
{
    return true;
}

bool VolumeFileGranularRestore::GenerateTearDownTask()
{
    VolumeFileGranularRestoreInfo restoreSubtaskInfo {};
    restoreSubtaskInfo.subTaskType = SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN;
    std::string uniqueId = std::to_string(m_idGenerator->GenerateId());
    std::string subTaskName = SUBJOB_NAME_VOLUME_GRANULAR_TEARDOWN + "_" + uniqueId;
    std::string restoreInfoStr;
    if (!Module::JsonHelper::StructToJsonString(restoreSubtaskInfo, restoreInfoStr)) {
        ERRLOG("restore info tranfer failed");
        return false;
    }
    DBGLOG("restoreInfo json:%s", restoreInfoStr.c_str());
    SubJob subJob {};
    subJob.__set_jobName(subTaskName);
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreInfoStr);
    subJob.__set_ignoreFailed(false);
    subJob.__set_jobPriority(SUBJOB_TYPE_VOLUME_GRANULAR_TEARDOWN_PRIO);
    if (!CreateSubTask(subJob)) {
        ERRLOG("CreateBackupSubTask failed");
        return false;
    }
    return true;
}

bool VolumeFileGranularRestore::GenerateGenerateInfo()
{
    if (m_generalInfo.backupPhaseStartTime != 0) {
        DBGLOG("Not first generate backup job, don't need report!");
        return true;
    }
    m_generalInfo.backupPhaseStartTime = PluginUtils::GetCurrentTimeInSeconds();
    return InitGeneralResource(m_statisticsPath);
}

int VolumeFileGranularRestore::PostJobInner()
{
    INFOLOG("Enter PostJobInner, jobId %s", m_jobId.c_str());
    if (!InitInfo()) {
        return Module::FAILED;
    }
    ClearMounts();
    MergeBackupFailureRecords();
    Remove(m_statisticsPath);
    // hint:: remove job cache root later
    INFOLOG("Exit PostJobInner, jobId %s", m_jobId.c_str());
    return Module::SUCCESS;
}

bool VolumeFileGranularRestore::InitInfo()
{
    INFOLOG("Enter InitInfo");

    if (!InitJobInfo()) {
        ERRLOG("Init restore info failed.");
        return false;
    }
    if (!InitRepoInfo()) {
        ERRLOG("Init repo info failed.");
        return false;
    }
    if (!InitRestoreInfo()) {
        ERRLOG("Init restore info failed");
        return false;
    }
    SetSubJobId();
    return true;
}

bool VolumeFileGranularRestore::InitJobInfo()
{
    m_jobInfoPtr = std::dynamic_pointer_cast<AppProtect::RestoreJob>(m_jobCommonInfo->GetJobInfo());
    if (m_jobInfoPtr == nullptr) {
        ERRLOG("m_jobInfoPtr is nullptr");
        return false;
    }
    m_jobId = m_jobInfoPtr->jobId;
    INFOLOG("Init job info, job id: %s", m_jobId.c_str());
    if (m_jobInfoPtr->copies.empty()) {
        ERRLOG("there has no copies");
        return false;
    }
    std::string copyId = m_jobInfoPtr->copies[0].id; // 副本ID
    std::string resourceId = m_jobInfoPtr->copies[0].protectObject.id;
    INFOLOG("Copy id: %s", copyId.c_str());
    m_jobType = JobType::RESTORE;
    return true;
}

bool VolumeFileGranularRestore::InitBasicRepoInfo()
{
    DBGLOG("Enter InitBasicRepoInfo");
    for (unsigned int i = 0; i < m_jobInfoPtr->copies[0].repositories.size(); i++) {
        if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFs = m_jobInfoPtr->copies[0].repositories[i];
        } else if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            m_copyId = m_jobInfoPtr->copies[0].id;
            m_dataFs = m_jobInfoPtr->copies[0].repositories[i];
        } else if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaFs = m_jobInfoPtr->copies[0].repositories[i];
        }
    }
    if (m_cacheFs.path.empty() || m_metaFs.path.empty() || m_dataFs.path.empty()) {
        ERRLOG("Repository info is wrong, cacheFs.path.size:%d, dataFs.path.size:%d, metaFs.path.size:%d",
            m_cacheFs.path.size(), m_dataFs.path.size(), m_metaFs.path.size());
        return false;
    }
    m_cacheFsPath = m_cacheFs.path[0];
    m_dataFsPath = m_dataFs.path[(m_numberOfSubTask++) % m_dataFs.path.size()];
    m_metaFsPath = m_metaFs.path[0];
    INFOLOG("cachePath: %s, dataPath: %s, metaPath: %s, copyId: %s",
        m_cacheFsPath.c_str(), m_dataFsPath.c_str(), m_metaFsPath.c_str(), m_copyId.c_str());
    return true;
}

/**
 * cache folder structure:
 * ${cache}
 *    |---${resourceId}
 *         |
 *        ${copyId}/ -----------------------------------------> m_scanMetaPathRoot
 *         |----${volumeName}/
 *         |         |---- meta,xmeta,fcache,dcache....
 *         |
 *        ${jobId}  ------------------------------------------> m_cacheFsPath
 *             |----statistics/
 *             |----ctrl/
 *                    |----scan/ -----------------------------> m_scanCtrlPathRoot
 *                    |      |----${volumeName}/
 *                    |                 |----control_xx.txt
 *                    |----restore/ --------------------------> m_restoreCtrlPathRoot
 *                           |----${volumeName}/
 *                                      |----control_xx.txt
 *
 * volume mount structure:
 * /mnt/databackup
 *          |----volumegranular/
 *                    |----${jobId}/
 *                             |----share/   -----------------> m_dataFsPersistMountTarget
 *                             |----volumes/ -----------------> m_volumesMountTargetRoot
 *                             |       |----${volumeName}
 *                             |----records/ -----------------> m_volumesMountRecordRoot
 *                                     |----${volumeName}
 */
bool VolumeFileGranularRestore::InitRepoInfo()
{
    DBGLOG("Enter InitRepoInfo");
    if (!InitBasicRepoInfo()) {
        return false;
    }
    m_scanMetaPathRoot = PluginUtils::PathJoin(
        PluginUtils::GetPathName(m_cacheFsPath), m_copyId, "filemeta", "meta", "latest");
    m_scanCtrlPathRoot = PluginUtils::PathJoin(m_cacheFsPath, "ctrl", "scan");
    m_restoreCtrlPathRoot = PluginUtils::PathJoin(m_cacheFsPath, "ctrl", "restore");
    m_dataFsPersistMountTarget = PluginUtils::PathJoin(VOLUME_GRANULAR_RESTORE_PATH_ROOT, m_jobId, "share");
    m_volumesMountTargetRoot = PluginUtils::PathJoin(VOLUME_GRANULAR_RESTORE_PATH_ROOT, m_jobId, "volumes");
    m_volumesMountRecordRoot = PluginUtils::PathJoin(VOLUME_GRANULAR_RESTORE_PATH_ROOT, m_jobId, "records");
    m_statisticsPath = PluginUtils::PathJoin(m_cacheFsPath, "statistics");
    INFOLOG("dataFsPersistMountTarget %s volumesMountTargetRoot %s volumesMountRecordRoot %s",
        m_dataFsPersistMountTarget.c_str(), m_volumesMountTargetRoot.c_str(), m_volumesMountRecordRoot.c_str());
    std::vector<std::pair<std::string, std::string>> dirExistCheckList {
        { "cacheFsPath",        m_cacheFsPath },
        { "dataFsPath",         m_dataFsPath },
        { "metaFsPath",         m_metaFsPath },
        { "scanMetaPathRoot",   m_scanMetaPathRoot }
    };
    for (const auto& dirExistCheckEntry : dirExistCheckList) {
        if (!PluginUtils::IsDirExist(dirExistCheckEntry.second)) {
            ERRLOG("%s path no exist", dirExistCheckEntry.first.c_str());
            return false;
        }
    }
    std::vector<std::pair<std::string, std::string>> dirCreateList {
        { "scanCtrlPathRoot",         m_scanCtrlPathRoot },
        { "restoreCtrlPathRoot",      m_restoreCtrlPathRoot },
        { "dataFsPersistMountTarget", m_dataFsPersistMountTarget },
        { "volumesMountTargetRoot",   m_volumesMountTargetRoot },
        { "volumesMountRecordRoot",   m_volumesMountRecordRoot },
        { "statisticsPath",           m_statisticsPath }
    };
    for (const auto& dirCreateEntry : dirCreateList) {
        if (!PluginUtils::IsDirExist(dirCreateEntry.second) && !PluginUtils::CreateDirectory(dirCreateEntry.second)) {
            ERRLOG("path prepare failed: %s", dirCreateEntry.first.c_str());
            return false;
        }
    }
    return true;
}

bool VolumeFileGranularRestore::InitRestoreInfo()
{
    std::string extJsonString = m_jobInfoPtr->extendInfo;
    INFOLOG("Extend info json string: %s", extJsonString.c_str());
    return true;
}

/*
 * Not Guarantee Transactional, `ClearMounts` Method Will Do The Clean!!!
 * Any Mount Failure Will Return `false`!!!
 */
bool VolumeFileGranularRestore::SetupMounts()
{
    return true;
}

void VolumeFileGranularRestore::ClearMounts()
{
    INFOLOG("load volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    std::vector<std::string> volumeMountRecordDirNameList {};
    if (!PluginUtils::GetDirListInDirectory(m_volumesMountRecordRoot, volumeMountRecordDirNameList, true)) {
        ERRLOG("failed to get volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    }
    std::vector<std::string> volumeMountRecordJsonList {};
    for (const std::string& volumeMountRecordDirName : volumeMountRecordDirNameList) {
        std::vector<std::string> jsonfileList;
        std::string volumeMountRecordDirPath = PluginUtils::PathJoin(
            m_volumesMountRecordRoot, volumeMountRecordDirName);
        INFOLOG("check json file in %s", volumeMountRecordDirPath.c_str());
        PluginUtils::GetFileListInDirectory(volumeMountRecordDirPath, jsonfileList);
        for (const std::string& jsonfile : jsonfileList) {
            if (jsonfile.find(MOUNT_RECORD_JSON_EXTENSION) != std::string::npos) {
                INFOLOG("detected umount record json file %s", jsonfile.c_str());
                volumeMountRecordJsonList.push_back(jsonfile);
            }
        }
    }
    for (const std::string& mountRecordJsonPath : volumeMountRecordJsonList) {
        INFOLOG("using mount record json %s to umount volume", mountRecordJsonPath.c_str());
        std::unique_ptr<VolumeCopyUmountProvider> umountProvider
            = VolumeCopyUmountProvider::Build(mountRecordJsonPath);
        if (umountProvider == nullptr) {
            ERRLOG("failed to build umount provider, record path %s", mountRecordJsonPath.c_str());
            continue;
        }
        if (!umountProvider->Umount()) {
            ERRLOG("volume umount failed with error %s, record path %s",
                umountProvider->GetError().c_str(), mountRecordJsonPath.c_str());
        }
    }
    ForceUmountNasShare(m_dataFsPersistMountTarget);
    return;
}

// For Scanner

static void GeneratedCopyCtrlFileCb(void* /* usrData */, std::string ctrlFile)
{
    INFOLOG("Generated copy ctrl file for volume granular restore: %s", ctrlFile.c_str());
}

static void GeneratedHardLinkCtrlFileCb(void* /* usrData */, std::string ctrlFile)
{
    INFOLOG("Generated hard link ctrl file for volume granular restore: %s", ctrlFile.c_str());
}

std::string VolumeFileGranularRestore::GetScanMetaDirPath(const std::string& volumeName) const
{
    return PluginUtils::PathJoin(m_scanMetaPathRoot, volumeName, LATEST);
}

std::string VolumeFileGranularRestore::GetScanOutputCtrlDirPath(const std::string& volumeName) const
{
    return PluginUtils::PathJoin(m_scanCtrlPathRoot, volumeName);
}

std::string VolumeFileGranularRestore::GetRestoreCtrlDirPath(const std::string& volumeName) const
{
    return PluginUtils::PathJoin(m_restoreCtrlPathRoot, volumeName);
}

std::string VolumeFileGranularRestore::GetRestoreSrcRootPath(const std::string& volumeName) const
{
    return PluginUtils::PathJoin(m_volumesMountTargetRoot, volumeName);
}

bool VolumeFileGranularRestore::IsValidControlFile(const std::string& path)
{
    if (path.find("txt.tmp") != string::npos) {
        return false;
    }
    if ((path.find(dir_sep + "hardlink_control_") != string::npos) ||
        (path.find(dir_sep + "mtime_") != string::npos) ||
        (path.find(dir_sep + "control_") != string::npos)) {
        return true;
    }
    return false;
}

void VolumeFileGranularRestore::FillGranularRestoreScanConfig(
    ScanConfig& scanConfig, const std::string& metaPath, const std::string& outputControlDirPath)
{
    return;
}

bool VolumeFileGranularRestore::WaitScannerTerminate(const std::string& volumeName)
{
    INFOLOG("monitor scanner for volume %s", volumeName.c_str());
    SCANNER_STATUS scanStatus = SCANNER_STATUS::INIT;
    std::string outputControlDirPath = GetScanOutputCtrlDirPath(volumeName);
    std::vector<std::string> controlFileList;
    for (; !(scanStatus == SCANNER_STATUS::COMPLETED && controlFileList.empty()) ;) {
        DBGLOG("watching scanner..., volumeName %s", volumeName.c_str());
        scanStatus = m_scanner->GetStatus();
        DBGLOG("scanner status %d", scanStatus);
        if (static_cast<int>(scanStatus) < 0) {
            ERRLOG("scanner failed with status %d, volumeName %s", scanStatus, volumeName.c_str());
            return false;
        }
        std::vector<std::string> fileList;
        PluginUtils::GetFileListInDirectory(outputControlDirPath, fileList);
        controlFileList.clear();
        std::copy_if(
            fileList.begin(),
            fileList.end(),
            std::back_inserter(controlFileList),
            VolumeFileGranularRestore::IsValidControlFile);
        if (!GenerateRestoreExecuteSubJob(controlFileList, volumeName)) {
            ERRLOG("failed to generate restore subjob for volume %s", volumeName.c_str());
            return false;
        }
        ReportJobDetails({JobLogLevel::type::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        Module::SleepFor(std::chrono::seconds(SLEEP_TEN_SECONDS));
    }
    INFOLOG("generate restore subjob success for volume %s, job: %s", volumeName.c_str(), m_jobId.c_str());
    return true;
}

bool VolumeFileGranularRestore::GenerateRestoreExecuteSubJob(
    const std::vector<std::string>& controlFileList, const std::string& volumeName)
{
    return true;
}

bool VolumeFileGranularRestore::InitSubJobInfo(
    SubJob &subJob, const std::string& ctrlPath, const std::string& volumeName)
{
    return true;
}

bool VolumeFileGranularRestore::ReportSubJobToAgent(
    const std::vector<SubJob> &subJobList, std::vector<std::string> &controlFileList)
{
    DBGLOG("Enter ReportSubJobToAgent");
    if (controlFileList.empty()) {
        return false;
    }
    ActionResult ret;
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return false;
    }
    // report succeed, remove control file from scan ctrl dir
    std::for_each(controlFileList.begin(), controlFileList.end(), PluginUtils::RemoveFile);
    return true;
}

// For Backup

bool VolumeFileGranularRestore::WaitBackupComplete()
{
    INFOLOG("Enter WaitBackupComplete");
    BackupPhaseStatus backupStatus { BackupPhaseStatus::INPROGRESS };
    while (backupStatus == BackupPhaseStatus::INPROGRESS ||
        backupStatus == BackupPhaseStatus::ABORT_INPROGRESS) {
        backupStatus = m_backup->GetStatus();
        DBGLOG("backup status %u", static_cast<int>(backupStatus));
        if (IsAbortJob()) {
            INFOLOG("Backup - Abort is invocked for jobId: %s ", m_jobId.c_str());
            if (m_backup->Abort() != BackupRetCode::SUCCESS) {
                ERRLOG("backup Abort is failed");
            }
        }
        UpdateSubBackupStats();
        UpdateSpeedAndReport();
        Module::SleepFor(std::chrono::seconds(EXECUTE_SUBTASK_MONITOR_DUR_IN_SEC));
    }
    UpdateSubBackupStats();
    INFOLOG("Exit WaitBackupComplete, backupStatus %d", static_cast<int>(backupStatus));
    return backupStatus == BackupPhaseStatus::COMPLETED;
}

void VolumeFileGranularRestore::UpdateSubBackupStats()
{
    if (m_subJobId.empty()) {
        WARNLOG("UpdateBackupTaskStats - subJobId is empty, main jobId: %s", m_jobId.c_str());
    }
    BackupStats subBackupStats = m_backup->GetStats();
    SerializeBackupStats(subBackupStats, m_subBackupStats);
    ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().UpdateResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, m_subJobId);
    if (!ret) {
        ERRLOG("Update sub job stats failed, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    }
    return;
}

void VolumeFileGranularRestore::UpdateSpeedAndReport()
{
    BackupStatistic mainBackupStats {};
    if (!CalcuMainBackupStats(mainBackupStats)) {
        ERRLOG("CalcuMainBackupStats failed");
        return;
    }
    if (!ShareResourceManager::GetInstance().QueryResource(ShareResourceType::GENERAL, m_jobId, m_generalInfo)) {
        ERRLOG("failed to query generalInfo %s", m_jobId.c_str());
        return;
    }
    time_t backDuration = PluginUtils::GetCurrentTimeInSeconds() - m_generalInfo.backupPhaseStartTime;
    uint64_t dataInKB = mainBackupStats.noOfBytesCopied / NUMBER1024;
    if (backDuration != 0) {
        m_jobSpeed = dataInKB / backDuration;
    }
    if (!ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId)) {
        // no need to report main backup stats, keepalive is required.
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        return;
    }
    if (mainBackupStats.noOfBytesCopied == 0) {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        return;
    }
    DBGLOG("ReportBackupRunningStatus, noOfBytesCopied: %d", mainBackupStats.noOfBytesCopied);
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO,
        "file_plugin_host_restore_data_inprogress_label",
        to_string(mainBackupStats.noOfDirCopied),
        to_string(mainBackupStats.noOfFilesCopied),
        PluginUtils::FormatCapacity(mainBackupStats.noOfBytesCopied));
}

void VolumeFileGranularRestore::SerializeBackupStats(
    const BackupStats& backupStats, BackupStatistic& backupStatistic) const
{
    backupStatistic.noOfDirToBackup    = backupStats.noOfDirToBackup;
    backupStatistic.noOfFilesToBackup  = backupStats.noOfFilesToBackup;
    backupStatistic.noOfBytesToBackup  = backupStats.noOfBytesToBackup;
    backupStatistic.noOfDirToDelete    = backupStats.noOfDirToDelete;
    backupStatistic.noOfFilesToDelete  = backupStats.noOfFilesToDelete;
    backupStatistic.noOfDirCopied      = backupStats.noOfDirCopied;
    backupStatistic.noOfFilesCopied    = backupStats.noOfFilesCopied;
    backupStatistic.noOfBytesCopied    = backupStats.noOfBytesCopied;
    backupStatistic.noOfDirDeleted     = backupStats.noOfDirDeleted;
    backupStatistic.noOfFilesDeleted   = backupStats.noOfFilesDeleted;
    backupStatistic.noOfDirFailed      = backupStats.noOfDirFailed;
    backupStatistic.noOfFilesFailed    = backupStats.noOfFilesFailed;
    backupStatistic.backupspeed        = backupStats.backupspeed;
    backupStatistic.startTime          = backupStats.startTime;
    backupStatistic.noOfSrcRetryCount  = backupStats.noOfSrcRetryCount;
    backupStatistic.noOfDstRetryCount  = backupStats.noOfDstRetryCount;
    backupStatistic.noOfFailureRecordsWritten  = backupStats.noOfFailureRecordsWritten;
}


void VolumeFileGranularRestore::FillRestoreConfig(BackupParams& backupParams)
{
    DBGLOG("Enter FillRestoreConfig.");
    size_t subJobRequestId = PluginUtils::GenerateHash(m_jobId + m_subJobInfo->subJobId);
    backupParams.backupType = ::BackupType::RESTORE;
    backupParams.phase = m_backupPhase;
    backupParams.srcAdvParams = make_shared<HostBackupAdvanceParams>();
    backupParams.dstAdvParams = make_shared<HostBackupAdvanceParams>();
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->dataPath = m_restoreInfo.srcRootPath;
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->dataPath = m_restoreInfo.dstRootPath;
    backupParams.scanAdvParams.metaFilePath = m_restoreInfo.metaDirPath;
#ifdef _WIN32
    backupParams.commonParams.writeExtendAttribute = false;
    backupParams.srcEngine = BackupIOEngine::WIN32_IO;
    backupParams.dstEngine = BackupIOEngine::WIN32_IO;
#else
    backupParams.commonParams.writeExtendAttribute = true;
    backupParams.srcEngine = BackupIOEngine::POSIX;
    backupParams.dstEngine = BackupIOEngine::POSIX;
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->threadNum =
        Module::ConfigReader::getInt("FilePluginConfig", "HostReaderThreadNum");
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.srcAdvParams)->maxMemory =
        Module::ConfigReader::getInt("FilePluginConfig", "HostMaxMemory");
    std::dynamic_pointer_cast<HostBackupAdvanceParams>(backupParams.dstAdvParams)->threadNum =
        Module::ConfigReader::getInt("FilePluginConfig", "HostWriterThreadNum");
#endif
    backupParams.commonParams.backupDataFormat = BackupDataFormat::NATIVE;
    backupParams.commonParams.maxBufferCnt = NUMBER10;
    backupParams.commonParams.maxBufferSize = NUMBER10 * NUMBER1024; // 10kb
    backupParams.commonParams.maxErrorFiles = NUMBER100;
    backupParams.commonParams.metaPath = m_restoreInfo.metaDirPath;
    backupParams.commonParams.jobId = m_jobId;
    backupParams.commonParams.subJobId = m_subJobInfo->subJobId;
    backupParams.commonParams.reqID = subJobRequestId;
    backupParams.commonParams.failureRecordRootPath = m_failureRecordRoot;
    backupParams.commonParams.restoreReplacePolicy = m_coveragePolicy;
    backupParams.commonParams.skipFailure = true;
    backupParams.commonParams.writeSparseFile = true;
    backupParams.commonParams.writeMeta = true;
    backupParams.commonParams.writeAcl = true;
    INFOLOG("srcRootPath %s, dstRootPath %s, metaFilePath %s",
        m_restoreInfo.srcRootPath.c_str(), m_restoreInfo.dstRootPath.c_str(), m_restoreInfo.metaDirPath.c_str());
}

bool VolumeFileGranularRestore::CalcuMainBackupStats(BackupStatistic& mainBackupStats) const
{
    // accumulate each subtask statistic for total statistics
    std::vector<std::string> statsList;
    if (!PluginUtils::GetFileListInDirectory(m_statisticsPath, statsList)) {
        ERRLOG("Get backup stats list failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    auto isSubTaskStats = [](const std::string& path) {
        return path.find(BACKUP_KEY_SUFFIX) != std::string::npos;
    };
    std::vector<std::string>::iterator it = std::find_if(statsList.begin(), statsList.end(), isSubTaskStats);
    for (; it != statsList.end(); it = std::find_if(it + 1, statsList.end(), isSubTaskStats)) {
        std::string path = *it;
        BackupStatistic subStats;
        std::string subJobId = path.substr(m_statisticsPath.length() + NUMBER1, m_jobId.length());
        DBGLOG("CalcuMainBackupStats, path: %s, jobId: %s, subJobId: %s",
            path.c_str(), m_jobId.c_str(), subJobId.c_str());
        ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, m_subJobId);
        bool ret = ShareResourceManager::GetInstance().QueryResource(path, subStats);
        ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, m_subJobId);
        DBGLOG("CalcuMainBackupStats, noOfBytesCopied: %llu, main: %llu, speed: %llu",
            subStats.noOfBytesCopied, mainBackupStats.noOfBytesCopied, mainBackupStats.backupspeed);
        if (!ret) {
            ERRLOG("Query failed, jobId: %s, subJobId: %s, path: %s",
                m_jobId.c_str(), path.c_str(), m_subJobId.c_str());
            return false;
        }
        mainBackupStats = mainBackupStats + subStats;
    }
    return true;
}

std::string VolumeFileGranularRestore::GetRestoreTargetPath() const
{
    // restore target path
    std::string restoreDstPath = m_jobInfoPtr->targetObject.name;
#ifdef _WIN32
    if (restoreDstPath == "/") {
        restoreDstPath = "";
    }
#endif
    INFOLOG("restore target path %s", restoreDstPath.c_str());
    return restoreDstPath;
}

}