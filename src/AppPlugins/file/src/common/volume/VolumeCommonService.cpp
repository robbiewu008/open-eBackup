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
#ifdef _WIN32
#include <filesystem>
#include "win32/BCD.h"
#include "FileSystemUtil.h"
#else
#include <boost/filesystem.hpp>
#endif

#ifdef __linux__
#include <sys/mount.h>
#include <sys/stat.h>
#include <mntent.h>
#endif

#include "log/BackupFailureRecorder.h"
#include "common/EnvVarManager.h"
#include "PluginUtilities.h"
#include "VolumeCommonStruct.h"
#include "VolumeCopyMountProvider.h"
#include "VolumeCommonService.h"

using namespace std;
using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace volumeprotect::mount;
using namespace Module;

#ifdef _WIN32
namespace fs = std::filesystem;
using namespace PluginUtils::Win32;
#else
namespace fs = boost::filesystem;
#endif

namespace FilePlugin {
namespace {
    const string MODULE = "VolumeCommonService";
    const uint64_t DEFAULT_MAX_FAILURE_RECORDS_NUM = 1000000; /* take about 200MB disk space */
    const uint64_t NUM1 = 1;
    constexpr uint64_t NUMBER1024 = 1024;
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
    const std::string DEFAULT_FAILURE_DEFAULT_ROOT =
        R"(\DataBackup\ProtectClient\ProtectClient-E\log\Plugins\FilePlugin)";
    const std::string SYS_MOUNTS_ENTRY_PATH = "/proc/mounts";
    const int MNTENT_BUFFER_MAX = 4096;
    const std::string XFS_FS_TYPE_STR = "xfs";
    const std::string FILE_SYSTEM_EXT2 = "ext2";
    const std::string FILE_SYSTEM_EXT3 = "ext3";
    const std::string FILE_SYSTEM_EXT4 = "ext4";
};
VolumeCommonService::VolumeCommonService()
{
    /* use logger root as the failure records output root */
    m_failureRecordRoot = Module::CLogger::GetInstance().GetLogRootPath();
    INFOLOG("Using logger root path %s as default root path for failure recorder", m_failureRecordRoot.c_str());
    if (m_failureRecordRoot.empty()) {
        WARNLOG("Got empty failure record root from log root, set to default");
        m_failureRecordRoot = Module::EnvVarManager::GetInstance()->GetAgentHomePath() + DEFAULT_FAILURE_DEFAULT_ROOT;
    }
    m_maxFailureRecordsNum = DEFAULT_MAX_FAILURE_RECORDS_NUM;
}

void VolumeCommonService::SetMainJobId(const std::string& jobId)
{
    m_jobId = jobId;
}

void VolumeCommonService::SetSubJobId()
{
    m_subJobId = GetSubJobId();
}

void VolumeCommonService::SetJobCtrlPhase(const std::string& jobCtrlPhase)
{
    m_jobCtrlPhase = jobCtrlPhase;
}

bool VolumeCommonService::InitIdGenerator()
{
    m_idGenerator = std::make_shared<Module::Snowflake>();
    if (m_idGenerator == nullptr) {
        ERRLOG("Init idGenerator failed, iter is nullptr. jobid: %s", m_jobId.c_str());
        return false;
    }
    size_t machineId = Module::GetMachineId();
    m_idGenerator->SetMachine(machineId);
    return true;
}

bool VolumeCommonService::CreateSubTask(const SubJob &subJob)
{
    ActionResult ret;
    INFOLOG("Create subtask, jobId: %s , jobName: %s, jobType: %d, jobPrio: %d, jobInfo: %s, time: %llu",
        subJob.jobId.c_str(), subJob.jobName.c_str(), subJob.jobType, subJob.jobPriority,
        subJob.jobInfo.c_str(), PluginUtils::GetCurrentTimeInSeconds());

    vector<SubJob> subJobList { subJob };
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, subJobList);
        if (ret.code == Module::SUCCESS) {
            return true;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
    return false;
}

void VolumeCommonService::SerializeVolumeStats(const volumeprotect::task::TaskStatistics& backupStats,
    uint64_t volumeCount, VolumeReportStatistic& backupStatistic)
{
    INFOLOG("volumeCount:%llu, backupStats.bytesWritten:%llu, backupStats.bytesTowrite:%llu", volumeCount,
        backupStats.bytesWritten, backupStats.bytesToWrite);
        backupStatistic.bytesWritten = backupStats.bytesWritten;
        backupStatistic.bytesToWrite = backupStats.bytesToWrite;
        backupStatistic.volumeCount = volumeCount;
}

bool VolumeCommonService::UpdateSubRestoreStats()
{
    if (m_subJobId.empty()) {
        DBGLOG("UpdateRestoreTaskStats - subJobId is empty, main jobId: %s", m_jobId.c_str());
        return true;
    }
    TaskStatistics currStats = m_backup->GetStatistics();
    uint64_t volumeCount = m_backup->GetStatus() == TaskStatus::SUCCEED ? 1 : 0;
    SerializeVolumeStats(currStats, volumeCount, m_subBackupStats);
    ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().UpdateResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, m_subJobId);
    if (!ret) {
        ERRLOG("Update sub job stats failed, jobId: %s, sub jobId: %s", m_jobId.c_str(), m_subJobId.c_str());
        return false;
    }
    return true;
}

bool VolumeCommonService::IsRestoreStatusInProgress(SubJobStatus::type& jobStatus)
{
    if (m_backupStatus == TaskStatus::SUCCEED) {
        INFOLOG("Monitor Restore - retorePhaseStatus::SUCCESS");
        jobStatus = SubJobStatus::COMPLETED;
        return false;
    } else if (m_backupStatus == TaskStatus::FAILED) {
        ERRLOG("Monitor Restore - restorePhaseStatus::FAILED");
        jobStatus = SubJobStatus::FAILED;
        return false;
    } else if (m_backupStatus == TaskStatus::ABORTED) {
        ERRLOG("Monitor Restore - restorePhaseStatus::ABORTED");
        jobStatus = SubJobStatus::ABORTED;
        return false;
    } else if (m_backupStatus == TaskStatus::ABORTING) {
        ERRLOG("Monitor Restore - restorePhaseStatus::ABORTING");
        jobStatus = SubJobStatus::ABORTING;
        return true;
    } else if (m_backupStatus == TaskStatus::RUNNING) {
        INFOLOG("Monitor Restore - restorePhaseStatus::RUNNING");
        jobStatus = SubJobStatus::RUNNING;
        return true;
    }
    return true;
}

bool VolumeCommonService::InitGeneralResource(const std::string& path)
{
    ShareResourceManager::GetInstance().SetResourcePath(path, m_jobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::GENERAL, m_jobId, m_generalInfo);
    if (!ret) {
        ERRLOG("Init general shared resourace failed, jobId: %s", m_jobId.c_str());
    }
    return ret;
}

uint64_t VolumeCommonService::CalculateSizeInKB(const uint64_t bytes) const
{
    uint64_t kiloBytes = bytes / NUM1024;
    if (kiloBytes == 0) { // consider less than 1KB as 1KB
        return NUM1;
    }
    return kiloBytes;
}

void VolumeCommonService::CalcuSumStructBackupStatistic(VolumeReportStatistic& mainStats,
    VolumeReportStatistic& subStats)
{
    DBGLOG("start to calcu the sum of A_backupStatistic and B_backupStatistic");
    mainStats.bytesToWrite += subStats.bytesToWrite;
    mainStats.bytesWritten += subStats.bytesWritten;
    mainStats.volumeCount += subStats.volumeCount;
}

bool VolumeCommonService::ReportBackupCompletionStatus()
{
    VolumeReportStatistic mainBackupStats {};
    if (!UpdateMainBackupStats(mainBackupStats)) {
        ERRLOG("UpdateMainBackupStats failed");
        return false;
    }
    std::string label;
    if (m_jobType == JobType::BACKUP) {
        label = "file_plugin_volume_backup_data_completed_label";
    } else {
        label = "file_plugin_volume_restore_data_completed_label";
    }
    m_dataSize = mainBackupStats.bytesWritten / NUMBER1024;
    INFOLOG("bytesWritten:%llu dataSize: %llu", mainBackupStats.bytesWritten, m_dataSize);
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, label,
        to_string(mainBackupStats.volumeCount),
        FormatCapacity(mainBackupStats.bytesWritten));
    return true;
}

int VolumeCommonService::ReportJobProgress(SubJobStatus::type &jobStatus)
{
    if (jobStatus == SubJobStatus::FAILED) {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::FAILED, PROGRESS100});
        return Module::FAILED;
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS100});
        return Module::SUCCESS;
    }
}

bool VolumeCommonService::UpdateCopyPhaseStartTime()
{
    if (m_generalInfo.backupPhaseStartTime != 0) {
        DBGLOG("Not first generate backup job, don't need report!");
        return true;
    }
    m_generalInfo.backupPhaseStartTime = PluginUtils::GetCurrentTimeInSeconds();
    if (!InitGeneralResource(m_statisticsPath)) {
        return false;
    }
    std::string label;
    if (m_jobType == JobType::BACKUP) {
        label = "file_plugin_volume_backup_data_start_label";
    } else {
        label = "file_plugin_volume_restore_data_start_label";
    }
    ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_backup");
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, label);
    return true;
}

bool VolumeCommonService::ReportBackupRunningStats(VolumeReportStatistic mainStats, bool forceReport)
{
    if (!forceReport && !ShareResourceManager::GetInstance().CanReportStatToPM(m_jobId + "_backup")) {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS50});
        return true;
    }
    DBGLOG("Data copied: %llu Byte, jobId: %s, subJobId: %s",
        mainStats.bytesWritten, m_jobId.c_str(), m_subJobId.c_str());
    if (mainStats.bytesWritten != 0) {
        std::string label;
        if (m_jobType == JobType::BACKUP) {
            label = "file_plugin_volume_backup_data_inprogress_label";
        } else {
            label = "file_plugin_volume_restore_data_inprogress_label";
        }
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, label,
            std::to_string(mainStats.volumeCount),
            FormatCapacity(mainStats.bytesWritten));
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
    }
    return true;
}

bool VolumeCommonService::MonitorRestore(SubJobStatus::type& jobStatus)
{
    DBGLOG("Enter Monitor Restore");
    do {
        VolumeReportStatistic mainStats;
        m_backupStatus = m_backup->GetStatus();
        m_volumeBackupErrorCode = m_backup->GetErrorCode();
        INFOLOG("m_backupStatus:%d", static_cast<int>(m_backupStatus));
        if (!IsRestoreStatusInProgress(jobStatus)) {
            m_volumeBackupErrorCode = m_backup->GetErrorCode();
            DBGLOG("m_backupStatus:%d", static_cast<int>(m_backupStatus));
            break;
        }
        UpdateSubRestoreStats();
        UpdateMainBackupStats(mainStats);
        if (IsAbortJob()) {
            INFOLOG("Backup - Abort is invoked for taskid: %s, subjobId: %s", m_jobId.c_str(), m_subJobId.c_str());
            if (m_backup->GetStatus() != TaskStatus::ABORTED) {
                m_backup->Abort();
            }
            while (m_backup->GetStatus() != TaskStatus::ABORTED) {
                INFOLOG("Abort not successful. Retrying... Current status: %d", m_backup->GetStatus());
                Module::SleepFor(std::chrono::seconds(ABORT_TIME_IN_SEC));
            }
            if (m_backup->GetStatus() == TaskStatus::ABORTED) {
                INFOLOG("Aborted successfully!");
                break;
            }
        }
        Module::SleepFor(std::chrono::seconds(EXECUTE_SUBTASK_MONITOR_DUR_IN_SEC));
    } while (true);
    UpdateSubRestoreStats();
    VolumeReportStatistic mainStats;
    UpdateMainBackupStats(mainStats, true);
    return true;
}

bool VolumeCommonService::UpdateMainBackupStats(VolumeReportStatistic& mainStats, bool forceReport)
{
    vector<string> statsList;
    if (!GetFileListInDirectory(m_statisticsPath, statsList)) {
        ERRLOG("Get backup stats list failed, jobId: %s", m_jobId.c_str());
        return false;
    }
    bool ret = ShareResourceManager::GetInstance().QueryResource(ShareResourceType::GENERAL, m_jobId, m_generalInfo);
    time_t currTime = PluginUtils::GetCurrentTimeInSeconds();
    time_t backDuration = currTime - m_generalInfo.backupPhaseStartTime;
    DBGLOG("UpdateMainBackupStats, currTime: %d, startTime: %d, duration: %d",
        currTime, m_generalInfo.backupPhaseStartTime, backDuration);
    VolumeReportStatistic subStats;
    for (const string& path : statsList) {
        string subJobId = path.substr(m_statisticsPath.length() + NUMBER1, m_subJobId.length());
        DBGLOG("UpdateMainBackupStats, path: %s, jobId: %s, subJobId: %s",
            path.c_str(), m_jobId.c_str(), subJobId.c_str());
        ShareResourceManager::GetInstance().Wait(ShareResourceType::BACKUP, subJobId);
        bool ret = ShareResourceManager::GetInstance().QueryResource(path, subStats);
        ShareResourceManager::GetInstance().Signal(ShareResourceType::BACKUP, subJobId);
        DBGLOG("UpdateMainBackupStats, sub: %llu, main: %llu", subStats.bytesWritten, mainStats.bytesWritten);
        if (!ret) {
            ERRLOG("Query failed, jobId: %s, subJobId: %s, path: %s", m_jobId.c_str(), path.c_str(), subJobId.c_str());
            return false;
        }
        CalcuSumStructBackupStatistic(mainStats, subStats);
    }
    uint64_t dataSize = 0LLU;
    if (m_subTaskType != SUBJOB_TYPE_TEARDOWN_VOLUME) {
        dataSize = mainStats.bytesWritten / NUMBER1024;
    }
    if (backDuration == 0) {
        backDuration = 1;
    }
    m_jobSpeed = dataSize / backDuration;
    ReportBackupRunningStats(mainStats, forceReport);
    DBGLOG("UpdateMainBackupStats, data: %lluKB, duration: %llus, m_jobSpeed: %lluKB/s",
        dataSize, backDuration, m_jobSpeed);
    return true;
}

bool VolumeCommonService::InitSubBackupJobResource()
{
    DBGLOG("Enter InitSubBackupJobResource");
    if (m_subJobId.empty()) {
        ERRLOG("InitSubBackupJobResources failed, m_subJobId empty.");
        return false;
    }
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_subJobId);
    bool ret = ShareResourceManager::GetInstance().InitResource(
        ShareResourceType::BACKUP, m_subJobId, m_subBackupStats);
    if (!ret) {
        ERRLOG("Init sub backup shared resource failed");
    }
    return ret;
}

size_t VolumeCommonService::VolumeCommonService::SetRequestId(const string& jobId, const string& subJobId)
{
    // set requestid for this thread
    size_t subJobRequestId = GenerateHash(m_jobId + m_subJobId);
    INFOLOG("mainJobID: %s, subJobID: %s, subJobRequestId: 0x%x", jobId.c_str(), subJobId.c_str(), subJobRequestId);
    HCPTSP::getInstance().reset(subJobRequestId);
    return subJobRequestId;
}

void VolumeCommonService::MergeBackupFailureRecords() const
{
    try {
        INFOLOG("start to merge failure records, output directory root: path %s, jobID: %s",
            m_failureRecordRoot.c_str(), m_jobId.c_str());
        if (!fs::exists(m_failureRecordRoot) || !fs::is_directory(m_failureRecordRoot)) {
            ERRLOG("failed to merge failure records, dir %s not exists", m_failureRecordRoot.c_str());
            return;
        }
        if (!Module::BackupFailureRecorder::Merge(m_failureRecordRoot, m_jobId, m_failureRecordRoot)) {
            ERRLOG("merge backup failure records failed");
        }
    } catch (const std::exception& e) {
        ERRLOG("merge backup failure records failed with exception: %s", e.what());
    }
}

std::string VolumeCommonService::GetMountDevicetype(const std::string& mountType, const std::string& volumePath, const std::string moduleType) const
{
    DBGLOG("mountType: %s, volumePath: %s", mountType.c_str(), volumePath.c_str());
    std::string result;
    std::string getMountTypeVersioneCmd;
    std::set<string> mountTypeExtSet = {FILE_SYSTEM_EXT2, FILE_SYSTEM_EXT3, FILE_SYSTEM_EXT4};
    if (mountTypeExtSet.count(mountType) > 0) {
        getMountTypeVersioneCmd = "tune2fs -l \"" + volumePath + "\" | head -n 1 | awk  '{print $2}'";
    } else if (mountType == XFS_FS_TYPE_STR) {
        getMountTypeVersioneCmd = "xfs_info -l \"" + volumePath + "\" | grep -o 'version=[0-9]*' | awk -F'=' '{print $2}";
    } else {
        WARNLOG(" This filesystem type is not supported, mountType: %s", mountType.c_str());
        return result;
    }
    DBGLOG("getMountTypeVersioneCmd:%s", getMountTypeVersioneCmd.c_str());
    std::vector<std::string> output;
    std::vector<std::string> errput;
    int ret = Module::runShellCmdWithOutput(INFO, moduleType, 0, getMountTypeVersioneCmd, {}, output, errput);
    if (ret != 0) {
        ERRLOG("getMountTypeVersioneCmd failed, getMountTypeVersioneCmd:%s", getMountTypeVersioneCmd.c_str());
        return result;
    }
    if (output.size() > 0) {
        DBGLOG("mountTypeVersion: %s", output[0].c_str());
        result = output[0];
    }
    return result;
}

bool VolumeCommonService::MountNasShare(
    const AppProtect::StorageRepository& repository,
    const std::string& targetPath,
    const std::string& extendInfo,
    std::string& driveInfo) const
{
    INFOLOG("execute MountNasShared, shareName: %s, target %s", repository.remoteName.c_str(), targetPath.c_str());
    JobPermission permission {};
    permission.__set_user("0");
    permission.__set_group("0");

    AppProtect::StorageRepository dataRepo = repository;
    dataRepo.__set_path(std::vector<std::string> { targetPath });

    PrepareRepositoryByPlugin repos {};
    repos.__set_repository(std::vector<AppProtect::StorageRepository> { dataRepo });
    repos.__set_permission(permission);
    repos.__set_extendInfo(extendInfo);

    ActionResult actionResult {};
    JobService::MountRepositoryByPlugin(actionResult, repos);
    if (actionResult.code != 0) {
        WARNLOG("Call MountRepositoryByPlugin failed! action code %u, message %s",
            actionResult.code, actionResult.message.c_str());
        if (actionResult.code != ERROR_POINT_MOUNTED) {
            JobService::UnMountRepositoryByPlugin(actionResult, repos);
        }
        return false;
    }
#ifdef WIN32
    VolumeSetMountDrive drive;
    if (!Module::JsonHelper::JsonStringToStruct(actionResult.message, drive)) {
        ERRLOG("JsonStringToStruct failed, FileSetMountDrive");
        return false;
    }
    driveInfo = drive.mountDirve;
    INFOLOG("return mountDrive: %s", driveInfo.c_str());
#endif
    return true;
}

bool VolumeCommonService::UmountNasShare(
    const AppProtect::StorageRepository& repository,
    const std::string& targetPath,
    const std::string& extendInfo) const
{
    INFOLOG("execute UmountNasShare, target %s", targetPath.c_str());
    JobPermission permission {};
    permission.__set_user("0");
    permission.__set_group("0");

    AppProtect::StorageRepository dataRepo = repository;
    dataRepo.__set_path(std::vector<std::string> { targetPath });

    PrepareRepositoryByPlugin repos {};
    repos.__set_repository(std::vector<AppProtect::StorageRepository> { dataRepo });
    repos.__set_permission(permission);
    repos.__set_extendInfo(extendInfo);

    ActionResult actionResult {};
    JobService::UnMountRepositoryByPlugin(actionResult, repos);
    if (actionResult.code != 0) {
        ERRLOG("Call UnMountRepositoryByPlugin failed! action code %u, message %s",
            actionResult.code, actionResult.message.c_str());
        return false;
    }
    return true;
}

void VolumeCommonService::ForceUmountNasShare(const std::string& nasShareMountTarget) const
{
    INFOLOG("umount nas share mounted at %s", nasShareMountTarget.c_str());
#ifdef __linux__
    if (::umount2(nasShareMountTarget.c_str(), MNT_FORCE | MNT_DETACH) != 0) {
        WARNLOG("umount %s failed with errno %d", nasShareMountTarget.c_str(), errno);
    }
#endif
}

inline std::string GetReadOnlyMountOptions(const std::string& mountFsType)
{
    if (mountFsType == "vfat") {
        return "noatime";
    } else if (mountFsType == "ext2") {
        return "ro";
    }
    return "ro,norecovery";
}

bool VolumeCommonService::MountSingleVolumeReadWrite(
    const std::string& volumeName,
    const std::string& volumeDirPath,
    const std::string& volumeMountTarget,
    const std::string& outputRecordDirPath,
    std::string& mountRecordPath) const
{
    std::string volumeMountArgsEntriesJsonPath = PluginUtils::PathJoin(volumeDirPath, VOLUME_MOUNT_ENTRIES_JSON_NAME);
    INFOLOG("mount volume (%s) from data dir %s, using target dir %s and record dir %s",
        volumeName.c_str(), volumeDirPath.c_str(), volumeMountTarget.c_str(), outputRecordDirPath.c_str());
    if (!PluginUtils::CreateDirectory(volumeMountTarget) || !PluginUtils::CreateDirectory(outputRecordDirPath)) {
        ERRLOG("failed to create volume mount target or output record dir");
        return false;
    }
    VolumeMountEntries volumeMountEntries {};
    if (!JsonFileTool::ReadFromFile(volumeMountArgsEntriesJsonPath, volumeMountEntries)) {
        ERRLOG("failed to read volume (%s) mount args entries json from %s",
            volumeName.c_str(), volumeMountArgsEntriesJsonPath.c_str());
        return false;
    }
    if (volumeMountEntries.mountEntries.empty()) {
        WARNLOG("volume (%s) mount args entries empty, won't mount, skip", volumeName.c_str());
        return true;
    }
    // Perform mount by invoking FS_Backup mount toolkit
    VolumeCopyMountConfig mountConf {};
    mountConf.copyName = DEFAULT_COPY_NAME;
    mountConf.copyMetaDirPath = volumeDirPath;
    mountConf.copyDataDirPath = volumeDirPath;
    mountConf.outputDirPath = outputRecordDirPath;
    mountConf.mountTargetPath = volumeMountTarget;
    mountConf.readOnly = false;
    mountConf.mountFsType = volumeMountEntries.mountEntries[0].mountType;
    mountConf.mountOptions = volumeMountEntries.mountEntries[0].mountOptions;
    INFOLOG("mount target: %s, mountFsType: %s, mountOptions: %s",
        mountConf.mountTargetPath.c_str(), mountConf.mountFsType.c_str(), mountConf.mountOptions.c_str());
    std::unique_ptr<VolumeCopyMountProvider> mountProvider = VolumeCopyMountProvider::Build(mountConf);
    if (mountProvider == nullptr) {
        ERRLOG("failed to build volume mount provider");
        return false;
    }
    if (!mountProvider->Mount()) {
        ERRLOG("volume mount failed, message : %s", mountProvider->GetError().c_str());
        return false;
    }
    INFOLOG("volume copy mount successfully, json record path %s", mountProvider->GetMountRecordPath().c_str());
    mountRecordPath = mountProvider->GetMountRecordPath();
    return true;
}

bool VolumeCommonService::MountSingleVolumeReadOnly(
    const std::string& volumeName,
    const std::string& volumeDirPath,
    const std::string& volumeMountTarget,
    const std::string& outputRecordDirPath,
    std::string& mountRecordPath) const
{
    std::string volumeMountArgsEntriesJsonPath = PluginUtils::PathJoin(volumeDirPath, VOLUME_MOUNT_ENTRIES_JSON_NAME);
    INFOLOG("mount volume (%s) from data dir %s, using target dir %s and record dir %s",
        volumeName.c_str(), volumeDirPath.c_str(), volumeMountTarget.c_str(), outputRecordDirPath.c_str());
    if (!PluginUtils::CreateDirectory(volumeMountTarget) || !PluginUtils::CreateDirectory(outputRecordDirPath)) {
        ERRLOG("failed to create volume mount target or output record dir");
        return false;
    }
    VolumeMountEntries volumeMountEntries {};
    if (!JsonFileTool::ReadFromFile(volumeMountArgsEntriesJsonPath, volumeMountEntries)) {
        ERRLOG("failed to read volume (%s) mount args entries json from %s",
            volumeName.c_str(), volumeMountArgsEntriesJsonPath.c_str());
        return false;
    }
    if (volumeMountEntries.mountEntries.empty()) {
        WARNLOG("volume (%s) mount args entries empty, won't mount, skip", volumeName.c_str());
        return true;
    }
    // Perform mount by invoking FS_Backup mount toolkit
    VolumeCopyMountConfig mountConf {};
    mountConf.copyName = DEFAULT_COPY_NAME;
    mountConf.copyMetaDirPath = volumeDirPath;
    mountConf.copyDataDirPath = volumeDirPath;
    mountConf.outputDirPath = outputRecordDirPath;
    mountConf.mountTargetPath = volumeMountTarget;
    mountConf.readOnly = false;
    mountConf.mountFsType = volumeMountEntries.mountEntries[0].mountType;
    mountConf.mountOptions = GetReadOnlyMountOptions(volumeMountEntries.mountEntries[0].mountType);

    INFOLOG("mount target: %s, mountFsType: %s, mountOptions: %s",
        mountConf.mountTargetPath.c_str(), mountConf.mountFsType.c_str(), mountConf.mountOptions.c_str());
    std::unique_ptr<VolumeCopyMountProvider> mountProvider = VolumeCopyMountProvider::Build(mountConf);
    if (mountProvider == nullptr) {
        ERRLOG("failed to build volume mount provider");
        return false;
    }
    if (!mountProvider->Mount()) {
        ERRLOG("volume mount failed, message : %s", mountProvider->GetError().c_str());
        return false;
    }
    INFOLOG("volume copy mount successfully, json record path %s", mountProvider->GetMountRecordPath().c_str());
    mountRecordPath = mountProvider->GetMountRecordPath();
    return true;
}

bool VolumeCommonService::UmountSingleVolume(
    const std::string& mountRecordJsonPath) const
{
    VolumeMountRecordJsonCommon recordJsonCommon {};
    if (!JsonFileTool::ReadFromFile(mountRecordJsonPath, recordJsonCommon)) {
        ERRLOG("read record json common struct from file: %s failed!", mountRecordJsonPath.c_str());
        return false;
    }
    std::unique_ptr<VolumeCopyUmountProvider> umountProvider
        = VolumeCopyUmountProvider::Build(mountRecordJsonPath);
    if (umountProvider == nullptr) {
        ERRLOG("failed to build umount provider, record path %s", mountRecordJsonPath.c_str());
        return false;
    }
    if (!umountProvider->Umount()) {
        ERRLOG("volume umount failed with error %s, record path %s",
            umountProvider->GetError().c_str(), mountRecordJsonPath.c_str());
        return false;
    }
    return true;
}

std::vector<std::string> VolumeCommonService::GetAllvolumeNameList(const std::string& dataFsPath) const
{
    bool skipSnapshot = true;
    INFOLOG("check volume copies in data in %s", dataFsPath.c_str());
    std::vector<std::string> volumeDirPaths;
    if (!PluginUtils::GetDirListInDirectory(dataFsPath, volumeDirPaths, skipSnapshot)) {
        ERRLOG("failed to get volume copies in %s", dataFsPath.c_str());
    }
    return volumeDirPaths;
}
}