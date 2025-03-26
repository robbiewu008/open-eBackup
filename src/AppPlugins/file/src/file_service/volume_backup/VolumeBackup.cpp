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
#include "VolumeBackup.h"
#include <thread>
#include "define/Types.h"
#include "PluginUtilities.h"
#include "config_reader/ConfigIniReader.h"
#include "common/Path.h"
#include "host/HostCommonStruct.h"
#include "ShareResourceManager.h"
#include "PluginConstants.h"
#include "VolumeProtector.h"
#include "native/FileSystemAPI.h"
#include "job/ChannelManager.h"

using namespace std;
using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;

namespace {
    const int NUMBER1200 = 1200;
    const int NUMBER100 = 100;
    const int NUMBER0 = 0;
    const int NUMBER2 = 2;
}

namespace FilePlugin {

const std::string MODULE = "VolumeBackup";
uint32_t VolumeBackup::g_numberOfSubTask = 0;

bool VolumeBackup::IsFullBackup() const
{
    if (m_backupJobPtr->jobParam.backupType == AppProtect::BackupJobType::FULL_BACKUP) {
        INFOLOG("backupJobType: FULL_BACKUP");
        return true;
    }
    INFOLOG("backupJobType: INCREMENT_BACKUP");
    return false;
}

VolumeBackup::VolumeBackup()
{
    INFOLOG("Construct VolumeBackup %llu", this_thread::get_id());
}

VolumeBackup::~VolumeBackup()
{
    INFOLOG("Destruct VolumeBackup %llu", this_thread::get_id());
}

/*
 * 检查备份任务类型为同步接口，通过函数返回值来判断是否需要增量转全量
 * return SUCCESS 1、本来就是全量备份；2、不需要进行类型转换
 * return FAILED  需要增量转全量
 */
EXTER_ATTACK int VolumeBackup::CheckBackupJobType()
{
    if (!GetBackupJobInfo()) {
        return Module::FAILED;
    }

    SetBackupJobInfo(JOB_CTRL_PHASE_CHECKBACKUPJOBTYPE);

    ENTER;
    int ret = CheckBackupJobTypeInner();
    EXIT;
    return ret;
}

EXTER_ATTACK int VolumeBackup::PrerequisiteJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetBackupJobInfo(JOB_CTRL_PHASE_PREJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeBackup::KeepJobAlive, this);
    int ret = PrerequisiteJobInner();
    EXIT;
    m_JobComplete = true;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0});
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    }
    SetJobToFinish();
    return ret;
}

int VolumeBackup::PrerequisiteJobInner()
{
    if (!InitJobInfo()) {
        return Module::FAILED;
    }
    // 清理残留的快照
    ClearResidualSnapshotsAndAlarm();
    // 检查卷是否存在
    if (!InitProtectVolume()) {
        return Module::FAILED;
    }
    // 初始化ShareResource，为执行子任务提供共享文件读写
    if (!InitShareResouce()) {
        return Module::FAILED;
    }
    // 检查操作系统内核（Win/Linux）是否被完全支持
    if (IsLimitedKernel()) {
        WARNLOG("Kernel version too low! Some features are not supported!");
    }
    return Module::SUCCESS;
}

EXTER_ATTACK int VolumeBackup::GenerateSubJob()
{
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetBackupJobInfo(JOB_CTRL_PHASE_GENSUBJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeBackup::KeepJobAlive, this);
    int ret = GenerateSubJobInner();
    EXIT;
    m_JobComplete = true;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0});
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    }

    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VolumeBackup::ExecuteSubJob()
{
    string jobId = GetParentJobId();
    string subJobId = GetSubJobId();
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ChannelManager::getInstance().removeSubJob(jobId, subJobId);
        INFOLOG("remove subJob from map, jobId: %s, subJobId: %s", jobId.c_str(), subJobId.c_str());
    });
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetBackupJobInfo(JOB_CTRL_PHASE_EXECSUBJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeBackup::KeepJobAlive, this);
    int ret = ExecuteSubJobInner();
    EXIT;
    m_JobComplete = true;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0});
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    }

    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VolumeBackup::PostJob()
{
    INFOLOG("Enter VolumeBackup PostJob, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    if (!GetBackupJobInfo()) {
        SetJobToFinish();
        return Module::FAILED;
    }
    SetBackupJobInfo(JOB_CTRL_PHASE_POSTJOB);

    ENTER;
    m_JobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeBackup::KeepJobAlive, this);
    int ret = PostJobInner();
    EXIT;
    m_JobComplete = true;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0});
    } else {
        ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    }
    SetJobToFinish();
    return ret;
}

void VolumeBackup::GetOriVolumes(std::vector<VolumeInfo>& sourceVolumes)
{
    return;
}

int VolumeBackup::PostJobInner()
{
    if (!InitJobInfo()) {
        return Module::FAILED;
    }
    PostClean();
    return Module::SUCCESS;
}

bool VolumeBackup::MergeVolumeInfo()
{
    return true;
}

bool VolumeBackup::GetBackupJobInfo()
{
    m_backupJobPtr = dynamic_pointer_cast<AppProtect::BackupJob>(GetJobInfo()->GetJobInfo());
    if (m_backupJobPtr == nullptr) {
        ERRLOG("Failed to get backupJobPtr.");
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS0});
        return false;
    }
    return true;
}

void VolumeBackup::SetBackupJobInfo(const std::string jonPhase)
{
    SetMainJobId(m_backupJobPtr->jobId);
    SetSubJobId();
    SetJobCtrlPhase(jonPhase);
}

int VolumeBackup::CheckBackupJobTypeInner()
{
    INFOLOG("backupJobType: %s", (IsFullBackup() ? "FULL" : "INC"));
    if (!InitRepoPaths()) {
        ERRLOG("InitRepoPaths failed");
        return Module::FAILED;
    }
    // Get systemBackupFlag value, systemBackupFlag default value is false.
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->extendInfo, m_advParms)) {
        ERRLOG("JsonStringToStruct failed for systemBackupFlag");
        return Module::FAILED;
    }
    // 判断当前备份类型是否为全量
    if (IsFullBackup()) {
        return Module::SUCCESS;
    }
    // 判断是否执行过全量，判断当前配置与上一次全量是否一致，检查卷大小是否改变
    if (IsFirstBackup() ||
        !IsSameBackupConfig() ||
        !IsSameVolumeSize()) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool VolumeBackup::InitDataLayoutInfo()
{
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->extendInfo, m_dataLayoutExt)) {
        ERRLOG("Failed to parse protctEnv extendInfo json to struct");
        return false;
    }
    m_jobType = JobType::BACKUP;
    return true;
}

void VolumeBackup::PrintJobInfo() const
{
    return;
}

bool VolumeBackup::ScanVolumesToGenerateTask()
{
    return true;
}

bool VolumeBackup::CreateBackupSubTask(VolumeBackupSubJob &backupSubJob, uint32_t stage)
{
    SubJob subJob {};
    
    if (m_idGenerator == nullptr) {
        InitIdGenerator();
    }
    string uniqueId = to_string(m_idGenerator->GenerateId());
    if (stage == SUBJOB_TYPE_DATACOPY_VOLUME)  {
        backupSubJob.subTaskType = SUBJOB_TYPE_DATACOPY_VOLUME;
        std::string subTaskName = SUBJOB_TYPE_VOLUME_JOBNAME + uniqueId;
        subJob.__set_jobName(subTaskName);
        subJob.__set_jobPriority(SUBJOB_TYPE_DATACOPY_VOLUME_PRIO);
    } else if (stage == SUBJOB_TYPE_TEARDOWN_VOLUME) {
        INFOLOG("Generate TearDown subJob");
        backupSubJob.subTaskType = SUBJOB_TYPE_TEARDOWN_VOLUME;
        subJob.__set_jobName(SUBJOB_TYPE_VOLUME_TEARDOWN);
        subJob.__set_jobPriority(SUBJOB_TYPE_TEARDOWN_VOLUME_PRIO);
    }
    string backupSubJobStr;
    if (!Module::JsonHelper::StructToJsonString(backupSubJob, backupSubJobStr)) {
        ERRLOG("Exit CreateBackupJobTask failed");
        return false;
    }

    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_ignoreFailed(true);
    subJob.__set_jobInfo(backupSubJobStr);

    if (!CreateSubTask(subJob)) {
        ERRLOG("Exit CreateBackupSubTask failed.");
        return false;
    }
    return true;
}

int VolumeBackup::GenerateSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);

    if (!InitJobInfo()) {
        return Module::FAILED;
    }

    if (!InitIdGenerator()) {
        return Module::FAILED;
    }

    // 获取用户保护的卷
    ReadVolumeFromFile();

    if (!GenerateSubJobHook()) {
        return Module::FAILED;
    }

    if (m_protectVolumeVec.empty() && (GetSysVolumeSize() == 0)) {
        WARNLOG("Protected volume is empty");
        return Module::SUCCESS;
    }

    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::SCAN, m_jobId, m_scanStats);
    if (!ret) {
        ERRLOG("Init scan resource failed");
        return Module::FAILED;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_scan_start_label");

    if (!ScanVolumesToGenerateTask()) {
        return Module::FAILED;
    }
    if (!GenerateTearDownTask()) {
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

uint32_t VolumeBackup::GetSysVolumeSize()
{
    return 0;
}

bool VolumeBackup::GenerateSubJobHook()
{
    return true;
}

bool VolumeBackup::GenerateTearDownTask()
{
    VolumeBackupSubJob backupSubJob;
    if (!CreateBackupSubTask(backupSubJob, SUBJOB_TYPE_TEARDOWN_VOLUME)) { // 生成卷任务
        return false;
    }
    return true;
}

bool VolumeBackup::IsFirstBackup()
{
    if (!IsFileExist(m_metaFsPath + VOLUME_COPY_METAFILE)) {
        return false;
    }
    return true;
}

bool VolumeBackup::IsSameBackupConfig()
{
    if (!JsonFileTool::ReadFromFile(m_backupCopyInfoFilePath, m_prevBackupCopyInfo)) {
        ERRLOG("Read backup copy meta info from file failed");
        return false;
    }
    VolumeBackupCopy volumeBackupCopy {};
    volumeBackupCopy.systemBackupFlag = m_advParms.systemBackupFlag;
    volumeBackupCopy.blockSize = volumeprotect::DEFAULT_BLOCK_SIZE;
    volumeBackupCopy.sessionSize = volumeprotect::DEFAULT_SESSION_SIZE;
    for (const AppProtect::ApplicationResource& resource : m_backupJobPtr->protectSubObject) {
        volumeBackupCopy.protectedVolumePaths.push_back(resource.name);
    }
    std::sort(volumeBackupCopy.protectedVolumePaths.begin(), volumeBackupCopy.protectedVolumePaths.end());
    std::sort(m_prevBackupCopyInfo.protectedVolumePaths.begin(), m_prevBackupCopyInfo.protectedVolumePaths.end());
    if (m_prevBackupCopyInfo.systemBackupFlag != volumeBackupCopy.systemBackupFlag ||
        m_prevBackupCopyInfo.blockSize != volumeBackupCopy.blockSize ||
        m_prevBackupCopyInfo.sessionSize != volumeBackupCopy.sessionSize ||
        m_prevBackupCopyInfo.protectedVolumePaths != volumeBackupCopy.protectedVolumePaths) {
        WARNLOG("NeedIncToFull main_jobId:%s, preBackupSystemFlag:%s, newBackSystemFlag:%s, " \
            "preblockSize:%llu, newBlockSize:%llu, preSessionSize:%llu, newSessionSize:%llu",
            m_jobId.c_str(), m_prevBackupCopyInfo.systemBackupFlag.c_str(), volumeBackupCopy.systemBackupFlag.c_str(),
            m_prevBackupCopyInfo.blockSize, volumeBackupCopy.blockSize, m_prevBackupCopyInfo.sessionSize,
            volumeBackupCopy.sessionSize);
        return false;
    }
    return true;
}

bool VolumeBackup::IsSameVolumeSize()
{
    if (!JsonFileTool::ReadFromFile(m_backupCopyInfoFilePath, m_prevBackupCopyInfo)) {
        ERRLOG("Read backup copy meta info from file failed");
        return false;
    }
    std::vector<std::pair<std::string, uint64_t>> prevVolumes {};
    std::vector<std::pair<std::string, uint64_t>> currentVolumes {};
    for (const VolumeInfomation& volumeInfo : m_prevBackupCopyInfo.volumeInfoSet.volumeInfoSet) {
        INFOLOG("previous volume %s size %llu", volumeInfo.volumePath.c_str(), volumeInfo.size);
        prevVolumes.push_back(std::pair<std::string, uint64_t>(volumeInfo.volumePath, volumeInfo.size));
    }
    for (const AppProtect::ApplicationResource& resource : m_backupJobPtr->protectSubObject) {
        std::string volumePath = resource.name;
        uint64_t volumeSize = PluginUtils::GetVolumeSize(volumePath);
        INFOLOG("current volume %s size %llu", volumePath.c_str(), volumeSize);
        currentVolumes.push_back(std::pair<std::string, uint64_t>(volumePath, volumeSize));
    }
    std::sort(prevVolumes.begin(), prevVolumes.end());
    std::sort(currentVolumes.begin(), currentVolumes.end());
    return currentVolumes == prevVolumes;
}

bool VolumeBackup::InitJobInfo()
{
    if (!InitRepoPaths()) {
        return false;
    }
    /* Data layout details */
    if (!InitDataLayoutInfo()) {
        return false;
    }
    if (!InitVolumeInfo()) {
        return false;
    }
    // Get systemBackupFlag value, systemBackupFlag default value is false.
    if (!Module::JsonHelper::JsonStringToStruct(m_backupJobPtr->extendInfo, m_advParms)) {
        ERRLOG("JsonStringToStruct failed for systemBackupFlag");
        return false;
    }
    m_lastBackupTime = PluginUtils::GetCurrentTimeInSeconds();
    INFOLOG("InitJobInfo, m_lastBackupTime: %s, m_backupJobPtr->extendInfo systemBackupFlag is %s",
            ConvertToReadableTime(m_lastBackupTime).c_str(), m_advParms.systemBackupFlag.c_str());

    // 每个阶段都设置下ShareResource路径，防止插件重启后路径找不到
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_jobId);
    PrintJobInfo();
    return true;
}

bool VolumeBackup::InitVolumeInfo()
{
    if (m_backupJobPtr->protectSubObject.empty()) {
        ERRLOG("Invalid volume, volume is empty");
        return false;
    }
    for (const AppProtect::ApplicationResource& resource : m_backupJobPtr->protectSubObject) {
        if (!volumeprotect::fsapi::IsVolumeExists(resource.name)) {
            ERRLOG("The selected volume: %s is not exit", resource.name.c_str());
            continue;
        }
        m_protectedVolumes.emplace(resource.name);
        INFOLOG("protected volume push %s", resource.name.c_str());
    }
    m_blockSize = volumeprotect::DEFAULT_BLOCK_SIZE;
    m_sessionSize = volumeprotect::DEFAULT_SESSION_SIZE;
    return true;
}

bool VolumeBackup::InitRepoPaths()
{
    for (auto repository : m_backupJobPtr->repositories) {
        if (repository.repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFs = repository;
        } else if (repository.repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            m_dataFs = repository;
        } else if (repository.repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaFs = repository;
        }
    }

    if (m_metaFs.path.empty() || m_dataFs.path.empty() || m_cacheFs.path.empty()) {
        ERRLOG("Received info is wrong, m_dataFs size: %zu, m_cacheFs size: %zu, m_metaFs size: %zu",
            m_dataFs.path.size(), m_cacheFs.path.size(), m_metaFs.path.size());
        return false;
    }
    m_cacheFsPath = m_cacheFs.path[0];
    m_metaFsPath = m_metaFs.path[0];
    m_dataFsPath = m_dataFs.path[(g_numberOfSubTask++) % m_dataFs.path.size()];
#ifdef __linux__
    m_metaFilePath = m_dataFsPath;
#else
    m_metaFilePath = m_metaFsPath;
#endif
    m_cacheFsParentPath= GetPathName(m_cacheFsPath);
    m_prevMetaPath = PluginUtils::PathJoin(m_cacheFsParentPath, SEP, PREVIOUS);
    m_currMetaPath = PluginUtils::PathJoin(m_metaFsPath, SEP, LATEST);
    m_sysInfoPath = PluginUtils::PathJoin(m_metaFsPath, "sys_info");
    m_statisticsPath = PluginUtils::PathJoin(m_cacheFsPath, "statistics", m_jobId);
    m_backupCopyInfoFilePath = PluginUtils::PathJoin(m_cacheFsParentPath, BACKUP_COPY_METAFILE);
    m_protectVolumeFile = PluginUtils::PathJoin(m_cacheFsPath, "volumeInfo", m_jobId + "_volumeInfo.json");
    DBGLOG("m_cacheFsPath:        %s", m_cacheFsPath.c_str());
    DBGLOG("m_metaFsPath:         %s", m_metaFsPath.c_str());
    DBGLOG("m_sysInfoPath:        %s", m_sysInfoPath.c_str());
    DBGLOG("m_dataFsPath:         %s", m_dataFsPath.c_str());
    DBGLOG("m_statisticsPath:     %s", m_statisticsPath.c_str());
    DBGLOG("m_protectVolumeFile:  %s", m_protectVolumeFile.c_str());
    DBGLOG("prevMetaPath:  %s", m_prevMetaPath.c_str());
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_jobId);
    return true;
}

bool VolumeBackup::InitShareResouce()
{
    HostSnapResidualInfo info;
    info.jobId = m_jobId;
    bool ret = ShareResourceManager::GetInstance().InitResource(ShareResourceType::SNAPSHOT, m_jobId, info);
    if (!ret) {
        ERRLOG("Init SNAPSHOT share resource failed!");
    }
    return ret;
}

bool VolumeBackup::InitJobInfoExecuteSubJob()
{
    if (!InitJobInfo()) {
        return false;
    }

    // set m_subTaskType from VolumeBackupSubJob
    VolumeBackupSubJob backupSubJob {};
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, backupSubJob)) {
        ERRLOG("Get backup subjob info failed!");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
        return Module::FAILED;
    }
    m_subTaskType = backupSubJob.subTaskType;
    FillVolumeInfo(backupSubJob);
    return true;
}

void VolumeBackup::FillVolumeInfo(const VolumeBackupSubJob& backupSubJob)
{
    return;
}

bool VolumeBackup::PreProcess(std::string& snapShotDev)
{
    return true;
}

int VolumeBackup::ExecuteSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});

    if (!InitJobInfoExecuteSubJob()) {
        ERRLOG("Init Job Info failed!");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
        return Module::FAILED;
    }
    m_subJobRequestId = SetRequestId(m_jobId, m_subJobId);

    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_jobId);
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ShareResourceManager::GetInstance().DecreaseRunningSubTasks(m_jobId);
    });

    int ret = Module::SUCCESS;
    if (m_subTaskType == SUBJOB_TYPE_DATACOPY_VOLUME) {
        std::string snapShotDev;
        if (!PreProcess(snapShotDev)) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
            return Module::FAILED;
        }
        ret = ExecuteDataCopyVolume();
        if (ret == Module::FAILED) {
            ReportJobDetails({JobLogLevel::TASK_LOG_WARNING, SubJobStatus::FAILED, PROGRESS100,
                "file_plugin_volume_backup_volume_snapshot_failed_label"}, snapShotDev);
        }
        if (!RecordSnap(snapShotDev)) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
                "file_plugin_volume_backup_prepare_create_snap_failed_label", snapShotDev);
            return Module::FAILED;
        }
        if (!RecordVolume()) {
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
            return Module::FAILED;
        }
    } else if (m_subTaskType == SUBJOB_TYPE_TEARDOWN_VOLUME) {
        ret = ExecuteTearDownVolume();
    }
    return ret;
}

bool VolumeBackup::RecordVolume()
{
    return true;
}

bool VolumeBackup::RecordSnap(std::string& snapShotDev)
{
    if (!snapShotDev.empty()) {
        ShareResourceManager::GetInstance().Wait(ShareResourceType::SNAPSHOT, m_jobId);
        HostSnapResidualInfo hostScanStatistics;
        if (!ShareResourceManager::GetInstance().QueryResource(ShareResourceType::SNAPSHOT, m_jobId,
            hostScanStatistics)) {
            ERRLOG("Query snapshot share resource failed, m_jobId is %s!", m_jobId.c_str());
            return false;
        }
        hostScanStatistics.snapshotInfos.push_back(snapShotDev);
        if (!ShareResourceManager::GetInstance().UpdateResource(ShareResourceType::SNAPSHOT, m_jobId,
            hostScanStatistics)) {
            ERRLOG("Update snapshot share resource faield, m_jobId is %s!", m_jobId.c_str());
            return false;
        }
        ShareResourceManager::GetInstance().Signal(ShareResourceType::SNAPSHOT, m_jobId);
    }
    return true;
}

bool VolumeBackup::SaveVolumeMountEntriesJson()
{
    return true;
}

int VolumeBackup::ExecuteDataCopyVolume()
{
    INFOLOG("Enter ExecuteDataCopyVolume, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    if (!InitSubBackupJobResource()) {
        ERRLOG("Init Sub restore backup job failed");
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
        return Module::FAILED;
    }
    if (!SaveVolumeMountEntriesJson()) {
        WARNLOG("failed to save mount args json");
    }
    if (!StartBackup()) {
        ERRLOG("Start volume backup failed.");
        ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::BACKUP, m_subJobId);
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_volume_backup_data_fail_label");
        return Module::FAILED;
    }
    
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    MonitorRestore(jobStatus);
    INFOLOG("Exit ExecuteDataCopyVolume, jobId: %s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    if (jobStatus == SubJobStatus::FAILED) {
        return Module::FAILED;
    } else {
        return Module::SUCCESS;
    }
}

void VolumeBackup::FillBackupCopyInfo(VolumeBackupCopy& volumeBackupCopy)
{
    volumeBackupCopy.systemBackupFlag = m_advParms.systemBackupFlag;
    volumeBackupCopy.blockSize = m_blockSize;
    volumeBackupCopy.sessionSize = m_sessionSize;
    volumeBackupCopy.protectedVolumePaths.clear();
    volumeBackupCopy.volumeInfoSet.volumeInfoSet.clear();
    for (const AppProtect::ApplicationResource& resource : m_backupJobPtr->protectSubObject) {
        std::string volumePath = resource.name;
        volumeBackupCopy.protectedVolumePaths.push_back(volumePath);
        VolumeInfomation volumeInfomation {};
        volumeInfomation.volumePath = volumePath;
        volumeInfomation.volumeName = PluginUtils::GetVolumeName(volumePath);
        volumeInfomation.size = PluginUtils::GetVolumeSize(volumePath);
        DBGLOG("volumeInfomation, volumePath:%s, volumeName:%s, size:%llu", volumeInfomation.volumePath.c_str(),
            volumeInfomation.volumeName.c_str(), volumeInfomation.size);
        volumeBackupCopy.volumeInfoSet.volumeInfoSet.push_back(volumeInfomation);
    }
}

bool VolumeBackup::SaveMeta()
{
    std::vector<string> subVolMetaDirName;
    if (!GetDirListInDirectory(m_metaFilePath, subVolMetaDirName)) {
        ERRLOG("Get dir list of data repo failed");
        return false;
    }
    for (const string& name : subVolMetaDirName) {
        INFOLOG("volumeName:%s", name.c_str());
        if (name == ".snapshot") {
            continue;
        }
        std::string preMetaFile = m_prevMetaPath + SEP + name;
        std::string curMetaFile = m_metaFilePath + SEP + name;
        if (IsDirExist(preMetaFile)) {
            Remove(preMetaFile);
        }
        if (!CreateDirectory(preMetaFile)) {
            ERRLOG("create meta dir %s failed", name.c_str());
            return false;
        }
        std::vector<string> metaFiles;
        if (!GetFileListInDirectory(curMetaFile, metaFiles)) {
            ERRLOG("Get metafiles failed from dir");
            return false;
        }
        for (const string& metaFile : metaFiles) {
            if (!SaveMetaFile(metaFile, name)) {
                ERRLOG("save meta file failed for metafile:%s, volumeName:%s", metaFile.c_str(), name.c_str());
                return false;
            }
        }
    }
    return true;
}

bool VolumeBackup::SaveMetaFile(const std::string& metaFile, const std::string& volumeName)
{
    std::string metaFileName = PluginUtils::GetFileName(metaFile);
    DBGLOG("metaFileName: %s", metaFileName.c_str());
    if (metaFileName.find(VOLUME_COPY_METAFILE) != string::npos || metaFileName.find(SHA_META) != string::npos) {
        std::string srcMetaFile = PluginUtils::PathJoin(m_metaFilePath, volumeName, metaFileName);
        std::string dstMetaFile = PluginUtils::PathJoin(m_prevMetaPath, volumeName, metaFileName);
        if (!CopyFile(srcMetaFile,  dstMetaFile)) {
            ERRLOG("Copy meta file from data %s to meta %s failed", srcMetaFile.c_str(), dstMetaFile.c_str());
            return false;
        }
    }
    return true;
}

bool VolumeBackup::TearDownVolumeHook()
{
    return true;
}

int VolumeBackup::ExecuteTearDownVolume()
{
    INFOLOG("Enter ExecuteTeardownSubJob");
    if (!ReportBackupCompletionStatus()) {
        return Module::FAILED;
    }
    VolumeBackupCopy volumeBackupCopy {};
    FillBackupCopyInfo(volumeBackupCopy);
    if (!JsonFileTool::WriteToFile(volumeBackupCopy, m_backupCopyInfoFilePath)) {
        ERRLOG("WriteBackupCpyToFile failed");
        return Module::FAILED;
    }
    if (!SaveMeta()) {
        ERRLOG("SaveScannerMeta failed");
        return Module::FAILED;
    }

    if (!TearDownVolumeHook()) {
        ERRLOG("Failed to process teardown hook");
        return Module::FAILED;
    }

    if (!MergeVolumeInfo()) {
        return Module::FAILED;
    }

    if (!PostReportCopyAdditionalInfo()) {
        ERRLOG("PostReportCopyAdditionalInfo failed");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool VolumeBackup::PostReportCopyAdditionalInfo()
{
    Copy image;
    image.__set_id(m_backupJobPtr->copy.id);
    image.__set_formatType(m_backupJobPtr->copy.formatType);
    string extendInfo;
    if (!FillCopyInfo(extendInfo)) {
        return false;
    }
    image.__set_extendInfo(extendInfo);
    AppProtect::ActionResult returnValue;
    JobService::ReportCopyAdditionalInfo(returnValue, m_backupJobPtr->jobId, image);
    if (returnValue.code != Module::SUCCESS) {
        ERRLOG("Exit ReportCopyAdditionalInfo Failed,returnCode: %d,jobId: %s",
            returnValue.code, m_backupJobPtr->jobId.c_str());
        return false;
    }
    INFOLOG("ReportCopyAdditionalInfo Success,jobId: %s", m_backupJobPtr->jobId.c_str());
    return true;
}

void VolumeBackup::GetMountPointFromFile(const std::string& volumeMountEntriesJson, std::string& mountPoints)
{
    VolumeMountEntries volumeMountEntries {};
    const std::string separator = ",";
    JsonFileTool::ReadFromFile(volumeMountEntriesJson, volumeMountEntries);
    for (auto entry : volumeMountEntries.mountEntries) {
        DBGLOG("GetMountPointFromFile: %s", entry.mountTargetPath.c_str());
        mountPoints += (entry.mountTargetPath + separator);
    }
    if (!mountPoints.empty() && mountPoints.back() == separator.front()) {
        mountPoints.pop_back();
    }
}

bool VolumeBackup::FillCopyInfo(std::string& extendInfo)
{
    VolumeInfoSet volumeInfoSet;
    for (auto volumePath : m_protectedVolumes) {
        std::string mountPoints;
        std::string volumeName = PluginUtils::GetVolumeName(volumePath);

        GetMountPointFromFile(m_dataFsPath + SEP + volumeName + SEP + VOLUME_MOUNT_ENTRIES_JSON_NAME, mountPoints);
        INFOLOG("mountPoints is: %s", mountPoints.c_str());
        VolumeInfomation volumeInfomation;
        volumeInfomation.volumePath = volumePath;
        volumeInfomation.mountPoint = mountPoints;
        volumeInfomation.volumeName = volumeName;
        volumeInfomation.uuid = volumeInfomation.volumeName;
        volumeInfomation.size = PluginUtils::GetVolumeSize(volumePath);
        DBGLOG("volumeInfomation, volumePath:%s, volumeName:%s, size:%llu", volumeInfomation.volumePath.c_str(),
            volumeInfomation.volumeName.c_str(), volumeInfomation.size);
        volumeInfoSet.volumeInfoSet.push_back(volumeInfomation);
    }
    // add disk info to extendInfo
    if (m_advParms.systemBackupFlag == TRUE_STR && !AddDiskInfoToExtend(volumeInfoSet)) {
        ERRLOG("Get disk info, write to extendInfo failed!");
        return false;
    }
    
    if (!Module::JsonHelper::StructToJsonString(volumeInfoSet, extendInfo)) {
        ERRLOG("volumeInfoSet change to json failed");
        return false;
    }
    INFOLOG("VolumeInfoSet:%s", extendInfo.c_str());
    return true;
}

bool VolumeBackup::AddDiskInfoToExtend(VolumeInfoSet &volumeInfoSet)
{
    std::string diskInfoPath = PluginUtils::PathJoin(m_sysInfoPath, "blk_disk");
    if (!PluginUtils::IsPathExists(diskInfoPath)) {
        ERRLOG("diskInfoPath not exist, path: %s", diskInfoPath.c_str());
        return false;
    }
    ifstream file(diskInfoPath);
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
        if (vec.size() == NUMBER2) {
            size_t pos = vec[NUMBER0].rfind("/");
            std::string diskName = vec[NUMBER0].substr(pos + 1);
            uint64_t diskSize = Module::SafeStoUll(vec[NUMBER1], UINT64_MAX);
            INFOLOG("diskName:%s, diskSize:%s, diskSizeInt:%llu",
                diskName.c_str(), vec[NUMBER1].c_str(), diskSize);
            DiskInfomation diskInfo;
            diskInfo.diskName = diskName;
            diskInfo.diskSize = diskSize;
            volumeInfoSet.diskInfoSet.push_back(diskInfo);
        } else {
            ERRLOG("dismiss invalid disk info: %s, require 2 colume!", line.c_str());
            continue;
        }
    }
    file.close();
    return true;
}

bool VolumeBackup::StartBackup()
{
    VolumeBackupConfig backupParams {};
    FillBackupConfig(backupParams);

    INFOLOG("Start Backup, backup phase: %u", m_subTaskType);

    m_backup = VolumeProtectTask::BuildBackupTask(backupParams);
    if (m_backup == nullptr) {
        ERRLOG("Create backup instance failed!");
        return false;
    }

    if (!m_backup->Start()) {
        ERRLOG("Start backup instance failed!");
        return false;
    }

    return true;
}

bool VolumeBackup::IsBackupTerminated()
{
    return m_backup->IsTerminated();
}

TaskStatus VolumeBackup::GetBackupGetStatus()
{
    return m_backup->GetStatus();
}

void VolumeBackup::AbortBackup()
{
    m_backup->Abort();
}

TaskStatistics VolumeBackup::GetBackupStatistics()
{
    return m_backup->GetStatistics();
}

void VolumeBackup::FillBackupConfig(VolumeBackupConfig &backupParams)
{
    return;
}

volumeprotect::CopyFormat VolumeBackup::CopyFormat()
{
    return volumeprotect::CopyFormat::IMAGE;
}

bool VolumeBackup::InitProtectVolume()
{
    return true;
}
 
bool VolumeBackup::PushVolumeToFile()
{
    PluginUtils::CreateDirectory(PluginUtils::GetPathName(m_protectVolumeFile));
    ofstream file(m_protectVolumeFile, ios::out | ios::trunc);
    if (!file.is_open()) {
        ERRLOG("PushProtectVolumeToFile open file failed, path: %s", m_protectVolumeFile.c_str());
        return false;
    }
    std::ostream_iterator<std::string> iter(file, "\n");
    copy(m_protectVolumeVec.begin(), m_protectVolumeVec.end(), iter);
    for (std::string volume : m_protectVolumeVec) {
        DBGLOG("Volume info: %s has been writen to file", volume.c_str());
    }
    file.close();
    return true;
}
 
bool VolumeBackup::ReadVolumeFromFile()
{
    if (!PluginUtils::IsFileExist(m_protectVolumeFile)) {
        return false;
    }
    ifstream file(m_protectVolumeFile);
    while (file) {
        std::string line;
        getline(file, line);
        if (line.empty()) {
            continue;
        }
        m_protectVolumeVec.push_back(line);
        INFOLOG("Get protect volume: %s", line.c_str());
    }
    file.close();
    return true;
}

void VolumeBackup::ClearResidualSnapshotsAndAlarm()
{
    return;
}

bool VolumeBackup::CreateSnapshot(const std::string& volumeName, std::string& snapDevVol)
{
    return true;
}

void VolumeBackup::DeleteSnapshot()
{
    return;
}

void VolumeBackup::ClearSucceedDeletedSnapshotRecord(const std::vector<std::string>& snapshotNames)
{
    string snapResidualFile = PluginUtils::PathJoin(m_cacheFsParentPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    INFOLOG("ClearSucceedDeletedSnapshotRecord, snapResidualFile: %s, jobId: %s",
        snapResidualFile.c_str(), m_jobId.c_str());
    HostSnapResidualInfoList snapResidualInfos;
    HostSnapResidualInfoList newSnapResidualInfos;
    if (PluginUtils::IsFileExist(snapResidualFile) &&
        !JsonFileTool::ReadFromFile(snapResidualFile, snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return;
    }
    for (const auto& info : snapResidualInfos.infos) {
        std::vector<std::string> newSnapshotInfos {};
        for (const std::string& snapshotName : info.snapshotInfos) {
            if (std::find(snapshotNames.begin(), snapshotNames.end(), snapshotName) == snapshotNames.end()) {
                newSnapshotInfos.push_back(snapshotName);
            }
        }
        if (newSnapshotInfos.empty()) {
            continue;
        }
        HostSnapResidualInfo newInfo {};
        newInfo.jobId = info.jobId;
        newInfo.snapshotInfos = newSnapshotInfos;
        newSnapResidualInfos.infos.push_back(newInfo);
    }
 
    if (!JsonFileTool::WriteToFile(newSnapResidualInfos, snapResidualFile)) {
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return;
    }
    return;
}

void VolumeBackup::SendAlarmForResidualSnapshots(const std::vector<std::string>& snapshotNames)
{
    // report residule snapshot infos to pm
    std::string msg;
    for (const std::string& snapshotName : snapshotNames) {
        msg += "[snapshot: " + snapshotName + "]";
    }
    INFOLOG("Report residule snapshot infos: %s, jobId: %s", msg.c_str(), m_jobId.c_str());
    PluginReportInfo reportInfo {
        JobLogLevel::TASK_LOG_ERROR,
        SubJobStatus::RUNNING,
        PROGRESS0,
        "file_plugin_volume_backup_post_delete_snap_failed_label",
        INITIAL_ERROR_CODE
    };
    ReportJobDetails(reportInfo, msg);
    // send alarm for residule snapshot infos
    ActionResult result;
    AppProtect::AlarmDetails alarm;
    alarm.alarmId = ALARM_CODE_FAILED_DELETE_SNAPSHOT;
    alarm.parameter = m_backupJobPtr->protectObject.type + "," + m_jobId;
    JobService::SendAlarm(result, alarm);
}

bool VolumeBackup::GetSnapshots(std::vector<std::string>& snapshotNames)
{
    // 从shareresource中获取当前任务打的快照卷，后置读取不需要加锁
    HostSnapResidualInfo snapResidualInfo {};
    if (!ShareResourceManager::GetInstance().QueryResource(ShareResourceType::SNAPSHOT, m_jobId, snapResidualInfo)) {
        ERRLOG("Query snapshot share resource failed, m_jobId is %s!", m_jobId.c_str());
        return false;
    }
    snapshotNames = snapResidualInfo.snapshotInfos;
    return true;
}

bool VolumeBackup::RecordResidualSnapshots(const std::vector<std::string>& snapshotNames)
{
    // 删除快照失败后调用该函数，将残留快照落盘，下次备份任务前置步骤删除
    string snapResidualFile = PluginUtils::PathJoin(m_cacheFsParentPath, RESIDUAL_SNAPSHORTS_INFO_FILE);
    INFOLOG("RecordResidualSnapshots, snapResidualFile: %s, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
    HostSnapResidualInfoList snapResidualInfos;
    if (PluginUtils::IsFileExist(snapResidualFile) &&
        !JsonFileTool::ReadFromFile(snapResidualFile, snapResidualInfos)) {
        ERRLOG("Read snap residual infos from file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    HostSnapResidualInfo info;
    info.jobId = m_jobId;
    info.snapshotInfos = snapshotNames;
    INFOLOG("SNAPSHOT ID: %s", snapshotNames[0].c_str());
    snapResidualInfos.infos.push_back(info);
    if (!JsonFileTool::WriteToFile(snapResidualInfos, snapResidualFile)) {
        ERRLOG("Write snap residual infos to file: %s failed, jobId: %s", snapResidualFile.c_str(), m_jobId.c_str());
        return false;
    }
    return true;
}

void VolumeBackup::PostClean()
{
    // 删除快照
    DeleteSnapshot();
    // 删除临时文件
    DeleteTmpFile();
}

void VolumeBackup::DeleteTmpFile()
{
    // 删除ShareResource
    ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::SNAPSHOT, m_jobId);
    // 删除统计信息目录
    PluginUtils::Remove(m_statisticsPath);
    // 删除保护卷信息
    PluginUtils::Remove(m_protectVolumeFile);
}

// 检查该 kernel 是否具有实现某些选项的必要文件
// 若缺失文件，则返回 true, kernel 功能受限
// 若不缺失，则返回 false, 该 kernel 被支持
bool VolumeBackup::IsLimitedKernel()
{
    return false;
}

void VolumeBackup::KeepJobAlive()
{
    INFOLOG("Start keep job alive for job(%s).", m_jobId.c_str());
    int count = 0;
    while (true && !IsAbortJob()) {
        if (m_JobComplete) {
            INFOLOG("jobKeepAlive thread shut down.");
            break;
        }
        ++count;
        // 使用count加sleep记时，120s上报一次
        if (count == NUMBER1200) {
            ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
            count = 0;
        }
        Module::SleepFor(std::chrono::milliseconds(NUMBER100)); // sleep 0.1s, 线程能更快被join退出
    }
}

} // namespace FilePlugin
