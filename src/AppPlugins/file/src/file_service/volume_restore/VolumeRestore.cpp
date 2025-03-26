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
#include "VolumeRestore.h"
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
#include "job/ChannelManager.h"

using namespace std;
using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace AppProtect;

namespace FilePlugin {

const auto MODULE = "VolumeRestore";
const int NUM100 = 100;
const int NUM1200 = 1200;
uint32_t VolumeRestore::m_numberOfSubTask = 0;

bool VolumeRestore::IsAbort() const
{
    return IsAbortJob() || IsJobPause();
}

EXTER_ATTACK int VolumeRestore::PrerequisiteJob()
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

EXTER_ATTACK int VolumeRestore::GenerateSubJob()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    INFOLOG("Enter VolumeRestore GenerateSubJob");
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_backup_scan_start_label");

    SetJobCtrlPhase(JOB_CTRL_PHASE_GENSUBJOB);
    ENTER;
    m_jobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeRestore::KeepJobAlive, this);
    int ret = GenerateSubJobInner();
    m_jobComplete = true;
    EXIT;
    keepAliveThread.join();

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

EXTER_ATTACK int VolumeRestore::ExecuteSubJob()
{
    INFOLOG("Enter VolumeRestore ExecuteSubJob");
    string jobId = GetParentJobId();
    string subJobId = GetSubJobId();
    std::shared_ptr<void> defer(nullptr, [&](...) {
        ChannelManager::getInstance().removeSubJob(jobId, subJobId);
        INFOLOG("remove subJob from map, jobId: %s, subJobId: %s", jobId.c_str(), subJobId.c_str());
    });

    SetJobCtrlPhase(JOB_CTRL_PHASE_EXECSUBJOB);
    ENTER;
    m_jobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeRestore::KeepJobAlive, this);
    int ret = ExecuteSubJobInner();
    m_jobComplete = true;
    EXIT;
    keepAliveThread.join();

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

EXTER_ATTACK int VolumeRestore::PostJob()
{
    INFOLOG("Enter VolumeRestore PostJob");
    m_jobCtrlPhase = JOB_CTRL_PHASE_POSTJOB;

    SetJobCtrlPhase(JOB_CTRL_PHASE_POSTJOB);
    ENTER;
    m_jobComplete = false;
    std::thread keepAliveThread = std::thread(&VolumeRestore::KeepJobAlive, this);
    int ret = PostJobInner();
    m_jobComplete = true;
    EXIT;
    keepAliveThread.join();

    if (ret != Module::SUCCESS) {
        ERRLOG("PostJob failed");
        ReportJobDetails({JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100});
        SetJobToFinish();
        return Module::FAILED;
    }
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, PROGRESS100});
    DeleteSharedResources();
    SetJobToFinish();
    return Module::SUCCESS;
}

int VolumeRestore::PrerequisiteJobInner()
{
    INFOLOG("Enter PrerequisiteJobInner");
    if (!InitInfo()) {
        return Module::FAILED;
    }

    return Module::SUCCESS;
}

int VolumeRestore::GenerateSubJobInner()
{
    INFOLOG("Enter GenerateSubJobInner");
    if (!InitInfo()) {
        return Module::FAILED;
    }
    if (!BareMetalRestore()) {
        return Module::FAILED;
    }
    if (!ScanVolumesToGenerateTask()) {
        return Module::FAILED;
    }
    if (!GenerateTearDownTask()) {
        return Module::FAILED;
    }
    INFOLOG("Exit GenerateSubJobInner");
    return Module::SUCCESS;
}

void VolumeRestore::ReportBMRConfigurationLabel()
{
    if (!m_enableBareMetalRestore) {
        return;
    }
#ifdef __linux__
    if (!m_restoreNonSystemVolume && !m_rebootSystemAfterRestore) {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_basic_info_00_label");
    } else if (!m_restoreNonSystemVolume && m_rebootSystemAfterRestore) {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_basic_info_01_label");
    } else if (m_restoreNonSystemVolume && !m_rebootSystemAfterRestore) {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_basic_info_10_label");
    } else {
        ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_basic_info_11_label");
    }
#endif
}

bool VolumeRestore::BareMetalRestore()
{
    if (!m_enableBareMetalRestore) {
        return true;
    }

    WARNLOG("Default is not implemented!");
    return true;
}

bool VolumeRestore::IsSystemVolume(const std::string &volumeName) const
{
    return false;
}

bool VolumeRestore::ScanVolumesToGenerateTask()
{
    INFOLOG("Enter ScanVolumesToGenerateTask");
    uint64_t totalSize = 0;
    uint64_t volumeCount = 0;
    for (RestoreInfo &restoreInfo : m_restoreInfoSet) {
        if (m_enableBareMetalRestore && !m_restoreNonSystemVolume && !IsSystemVolume(restoreInfo.volumeName)) {
            INFOLOG("%s is not system volume, skip restore", restoreInfo.volumeName.c_str());
            continue;
        }
        if (!CreateRestoreSubJob(restoreInfo, SUBJOB_TYPE_DATACOPY_VOLUME)) {
            ERRLOG("Create Sub Job Failed, volumeName:%s, volumeDstPath:%s",
                restoreInfo.volumeName.c_str(),
                restoreInfo.dataDstPath.c_str());
            return false;
        }
        volumeCount++;
        VolumeCopyMeta volumeCopyMeta;
        std::string currMetaPath = PluginUtils::PathJoin(m_dataFsPath, restoreInfo.volumeName);
        if (!volumeprotect::common::ReadVolumeCopyMeta(currMetaPath, DEFAULT_VOLUME_COPY_NAME, volumeCopyMeta)) {
            ERRLOG("read Volume meta file failed, metafile:%s", currMetaPath.c_str());
            return false;
        }
        totalSize += volumeCopyMeta.volumeSize;
        INFOLOG("volume size:%llu", volumeCopyMeta.volumeSize);
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO,
        "file_plugin_volume_restore_scan_data_completed_label",
        to_string(volumeCount),
        FormatCapacity(totalSize),
        to_string(volumeCount),
        FormatCapacity(totalSize));
    if (!UpdateCopyPhaseStartTime()) {
        ERRLOG("Updated restore start time failed");
        return false;
    }
    return true;
}

int VolumeRestore::ExecuteSubJobInner()
{
    ABORT_ENDTASK(m_logSubJobDetails, m_logResult, m_logDetailList, m_logDetail, 0, 0);
    ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
    if (!InitInfo()) {
        return Module::FAILED;
    }
    if (!Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_restoreInfo)) {
        ERRLOG("m_subJobInfo exchange failed");
        return Module::FAILED;
    }
    m_subTaskType = m_restoreInfo.subTaskType;
    int ret = Module::SUCCESS;
    if (m_subTaskType == SUBJOB_TYPE_DATACOPY_VOLUME) {
        ret = ExecuteDataCopyVolume();
    } else if (m_subTaskType == SUBJOB_TYPE_TEARDOWN_VOLUME) {
        ret = ExecuteTearDownVolume();
    }
    return ret;
}

int VolumeRestore::ExecuteDataCopyVolume()
{
    VolumeRestoreConfig restoreConfig;
    FillRestoreConfig(restoreConfig);
    if (!InitSubBackupJobResource()) {
        ERRLOG("Init Sub restore backup job failed");
        return Module::FAILED;
    }
    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_jobId);

    if (!StartRestore(restoreConfig)) {
        ERRLOG("Start restore failed.");
        ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::BACKUP, m_subJobId);
        return Module::FAILED;
    }
    SubJobStatus::type jobStatus = SubJobStatus::RUNNING;
    MonitorRestore(jobStatus);
    ShareResourceManager::GetInstance().IncreaseRunningSubTasks(m_jobId);
    INFOLOG("volumeBackupErrorCode is %u", m_volumeBackupErrorCode);
    HandleRestoreErrorCode();
    PostProcess();
    int ret = ReportJobProgress(jobStatus);
    INFOLOG("Exit ExecuteSubJobInner, jobId:%s, subJobId: %s", m_jobId.c_str(), m_subJobId.c_str());
    return ret;
}

void VolumeRestore::PostProcess()
{
    return;
}

int VolumeRestore::ExecuteTearDownVolume()
{
    INFOLOG("Enter default ExecuteTearDownVolume");
    if (!ReportBackupCompletionStatus()) {
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

bool VolumeRestore::StartRestore(const VolumeRestoreConfig &restoreConfig)
{
    m_backup = VolumeProtectTask::BuildRestoreTask(restoreConfig);
    if (m_backup == nullptr) {
        ERRLOG("restore task build failed, volumePath:%s", restoreConfig.volumePath.c_str());
        return false;
    }
    if (!m_backup->Start()) {
        ERRLOG("Start restore instance failed");
        return false;
    }
    return true;
}

bool VolumeRestore::CreateRestoreSubJob(RestoreInfo &restoreInfo, uint32_t stage)
{
    SubJob subJob{};
    string subTaskName{};
    string uniqueId = to_string(m_idGenerator->GenerateId());
    if (stage == SUBJOB_TYPE_DATACOPY_VOLUME) {
        restoreInfo.subTaskType = SUBJOB_TYPE_DATACOPY_VOLUME;
        subTaskName = SUBJOB_TYPE_VOLUME_JOBNAME + "_" + uniqueId;
        subJob.__set_jobPriority(SUBJOB_TYPE_DATACOPY_VOLUME_PRIO);
    } else if (stage == SUBJOB_TYPE_TEARDOWN_VOLUME) {
        INFOLOG("Generate TearDown subJob");
        restoreInfo.subTaskType = SUBJOB_TYPE_TEARDOWN_VOLUME;
        subTaskName = SUBJOB_TYPE_VOLUME_TEARDOWN + "_" + uniqueId;
        subJob.__set_jobPriority(SUBJOB_TYPE_TEARDOWN_VOLUME_PRIO);
    }
    std::string restoreInfoStr;
    if (!Module::JsonHelper::StructToJsonString(restoreInfo, restoreInfoStr)) {
        ERRLOG("restore info tranfer failed");
        return false;
    }
    DBGLOG("restoreInfo json:%s", restoreInfoStr.c_str());
    subJob.__set_jobName(subTaskName);
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    subJob.__set_jobInfo(restoreInfoStr);
    bool ignoreFailed = false;
    subJob.__set_ignoreFailed(ignoreFailed);
    if (!CreateSubTask(subJob)) {
        ERRLOG("CreateBackupSubTask failed");
        return false;
    }
    return true;
}

bool VolumeRestore::GenerateTearDownTask()
{
    INFOLOG("Enter GenerateTearDownTask");
    RestoreInfo restoreInfo;
    if (!CreateRestoreSubJob(restoreInfo, SUBJOB_TYPE_TEARDOWN_VOLUME)) {  // 生成卷任务
        return false;
    }
    return true;
}

void VolumeRestore::FillRestoreConfig(VolumeRestoreConfig &restoreConfig)
{
    restoreConfig.volumePath = m_restoreInfo.dataDstPath;
    restoreConfig.copyDataDirPath = PluginUtils::PathJoin(m_dataFsPath, m_restoreInfo.volumeName);
#ifdef __linux__
    restoreConfig.copyMetaDirPath = PluginUtils::PathJoin(m_dataFsPath, m_restoreInfo.volumeName);
#else
    restoreConfig.copyMetaDirPath = PluginUtils::PathJoin(m_metaFsPath, m_restoreInfo.volumeName);
#endif
    restoreConfig.enableCheckpoint = false;
    restoreConfig.shareName = m_dataFs.remoteName;
    INFOLOG("volumePath:%s, dataPath:%s, metaPath:%s, shareName: %s",
        restoreConfig.volumePath.c_str(),
        restoreConfig.copyDataDirPath.c_str(),
        restoreConfig.copyMetaDirPath.c_str(),
        restoreConfig.shareName.c_str());
}

int VolumeRestore::PostJobInner()
{
    if (!InitInfo()) {
        return Module::FAILED;
    }

    INFOLOG("Exit PostJobInner");
    return Module::SUCCESS;
}

bool VolumeRestore::InitInfo()
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
    if (!InitBareMetalRestoreInfo()) {
        ERRLOG("Init BareMetalRestore info failed");
        return false;
    }

    SetSubJobId();
    return true;
}

bool VolumeRestore::InitBareMetalRestoreInfo()
{
    BareMetalRestoreParam bareMetalRestoreParam;
    if (!Module::JsonHelper::JsonStringToStruct(m_jobInfoPtr->extendInfo, bareMetalRestoreParam)) {
        ERRLOG("Convert to FileSetInfo failed.");
        return Module::FAILED;
    }
    if (CheckBMR(bareMetalRestoreParam)) {
        m_enableBareMetalRestore = true;
    }

    if (!m_enableBareMetalRestore) {
        return true;
    }
    if (bareMetalRestoreParam.restoreNonSystemVolume == "true") {
        m_restoreNonSystemVolume = true;
        INFOLOG("Restore Non System Volume switch is enable");
    }
    if (bareMetalRestoreParam.rebootSystemAfterRestore == "true") {
        m_rebootSystemAfterRestore = true;
        INFOLOG("Reboot System After Restore is enable");
    }
    return true;
}

bool VolumeRestore::CheckBMR(const BareMetalRestoreParam& param)
{
    return true;
}

bool VolumeRestore::InitJobInfo()
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
    std::string copyId = m_jobInfoPtr->copies[0].id;  // 副本ID
    std::string resourceId = m_jobInfoPtr->copies[0].protectObject.id;
    INFOLOG("Copy id: %s", copyId.c_str());
    m_jobType = JobType::RESTORE;
    return true;
}

bool VolumeRestore::InitRepoInfo()
{
    INFOLOG("Enter InitRepoInfo");
    for (unsigned int i = 0; i < m_jobInfoPtr->copies[0].repositories.size(); i++) {
        if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::CACHE_REPOSITORY) {
            m_cacheFs = m_jobInfoPtr->copies[0].repositories[i];
        } else if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            m_dataFs = m_jobInfoPtr->copies[0].repositories[i];
        } else if (m_jobInfoPtr->copies[0].repositories[i].repositoryType == RepositoryDataType::META_REPOSITORY) {
            m_metaFs = m_jobInfoPtr->copies[0].repositories[i];
        }
    }
    if (m_cacheFs.path.empty() || m_metaFs.path.empty() || m_dataFs.path.empty()) {
        ERRLOG("Repository info is wrong, cacheFs.path.size():%d, dataFs.path.size():%d, metaFs.path.size():%d",
            m_cacheFs.path.size(),
            m_dataFs.path.size(),
            m_metaFs.path.size());
        return false;
    }
    m_cacheFsPath = m_cacheFs.path[0];
    m_dataFsPath = m_dataFs.path[(m_numberOfSubTask++) % m_dataFs.path.size()];
    m_metaFsPath = m_metaFs.path[0];
    m_sysInfoPath = PluginUtils::PathJoin(m_metaFsPath, "sys_info");
    m_statisticsPath = m_cacheFsPath + dir_sep + "statistics";
    INFOLOG("cachePath: %s, dataPath: %s, metaPath: %s, statisticsPath: %s, sysInfoPath: %s",
        m_cacheFsPath.c_str(),
        m_dataFsPath.c_str(),
        m_metaFsPath.c_str(),
        m_statisticsPath.c_str(),
        m_sysInfoPath.c_str());
    if (!PluginUtils::IsDirExist(m_cacheFsPath)) {
        ERRLOG("m_cacheFsPath path no exist: %s", m_cacheFsPath.c_str());
        return false;
    }
    if (!PluginUtils::IsDirExist(m_dataFsPath)) {
        ERRLOG("m_dataPath path no exist: %s", m_dataFsPath.c_str());
        return false;
    }
    if (!PluginUtils::IsDirExist(m_metaFsPath)) {
        ERRLOG("m_metaPath path no exist: %s", m_metaFsPath.c_str());
        return false;
    }
    std::string backupCopyInfo = m_jobInfoPtr->copies[0].extendInfo;
    INFOLOG("Backup copy info: %s", Module::WipeSensitiveDataForLog(backupCopyInfo.c_str()).c_str());
    m_mountInfoFilePath = PluginUtils::PathJoin(m_cacheFsPath, "mountInfo", m_jobId, "volumeMountInfo.json");
    INFOLOG("m_mountInfoFilePath:%s", m_mountInfoFilePath.c_str());
    ShareResourceManager::GetInstance().SetResourcePath(m_statisticsPath, m_jobId);
    return true;
}

bool VolumeRestore::InitRestoreInfo()
{
    std::string extJsonString = m_jobInfoPtr->extendInfo;
    INFOLOG("Extend info json string: %s", extJsonString.c_str());
    Json::Value value;
    if (!Module::JsonHelper::JsonStringToJsonValue(extJsonString, value)) {
        ERRLOG("Convert to FileSetInfo failed.");
        return false;
    }
    if (!(value.isObject() && value.isMember("restoreInfoSet") && value["restoreInfoSet"].isString())) {
        ERRLOG("json change failed");
        return false;
    }
    std::string restoreInfoSet = value["restoreInfoSet"].asString();
    if (!Module::JsonHelper::JsonStringToJsonValue(restoreInfoSet, value)) {
        ERRLOG("Json change failed");
        return false;
    }
    for (auto val : value) {
        RestoreInfo restoreInfo;
        if (!Module::JsonHelper::JsonValueToStruct(val, restoreInfo)) {
            ERRLOG("Json change failed");
            continue;
        }
#ifdef __linux__
        restoreInfo.volumeName = restoreInfo.volumeId;
#else
        restoreInfo.volumeName = restoreInfo.volumeName;
#endif
        INFOLOG("restore volume info : volumeName %s, volumeId %s, dataDstPath %s",
            restoreInfo.volumeName.c_str(),
            restoreInfo.volumeId.c_str(),
            restoreInfo.dataDstPath.c_str());
        m_restoreInfoSet.push_back(restoreInfo);
    }
    InitIdGenerator();
    return true;
}

void VolumeRestore::DeleteSubBackupResources() const
{
    INFOLOG("Enter DeleteSubBackupResources");
    ShareResourceManager::GetInstance().DeleteResource(ShareResourceType::BACKUP, m_subJobId);
}

void VolumeRestore::DeleteSharedResources() const
{
    INFOLOG("Enter DeleteSharedResources");
    Remove(m_statisticsPath);
}

void VolumeRestore::HandleRestoreErrorCode()
{
    return;
}

void VolumeRestore::KeepJobAlive()
{
    INFOLOG("Start keep job alive for job(%s).", m_jobId.c_str());
    int count = 0;
    while (true && !IsAbortJob()) {
        if (m_jobComplete) {
            INFOLOG("jobKeepAlive thread shut down.");
            break;
        }
        ++count;
        // 使用count加sleep记时，120s上报一次
        if (count == NUM1200) {
            ReportJobDetails({JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, PROGRESS0});
            count = 0;
        }
        Module::SleepFor(std::chrono::milliseconds(NUM100)); // sleep 0.1s, 线程能更快被join退出
    }
}
}  // namespace FilePlugin