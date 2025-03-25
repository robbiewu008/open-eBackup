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
#include "VolumeLivemount.h"
#include "define/Types.h"
#include "VolumeCopyMountProvider.h"
#include "log/Log.h"

using namespace FilePlugin;
using namespace AppProtect;
using namespace volumeprotect;
using namespace volumeprotect::mount;

namespace {
    constexpr auto MODULE = "VolumeLivemount";
    const std::string VOLUME_LIVEMOUNT_CACHE_DIR_NAME = "volumelivemount";
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
    const std::string SYS_BOOT_VOLUME = "boot";
}

// implement livemount public methods ...

EXTER_ATTACK int VolumeLivemount::PrerequisiteJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("VolumeLivemount PrerequisiteJob failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeLivemount PrerequisiteJob, %s", m_livemountPara->jobId.c_str());
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    INFOLOG("Leave VolumeLivemount PrerequisiteJob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int VolumeLivemount::GenerateSubJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("VolumeLivemount generate sub job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeLivemount GenerateSubjob, %s", m_livemountPara->jobId.c_str());
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave VolumeLivemount GenerateSubjob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int VolumeLivemount::ExecuteSubJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("VolumeLivemount execute sub job failed for livemountPara is nullptr or subjobInfo is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeLivemount ExecuteSubjob, %s", m_livemountPara->jobId.c_str());
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint ExcuteSubjob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int VolumeLivemount::PostJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("VolumeLivemount post job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeLivemount Postjob, %s", m_livemountPara->jobId.c_str());
    int ret = PostJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint Postjob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

// implement livemount private methods ...

std::shared_ptr<LivemountJob> VolumeLivemount::GetJobInfoBody() const
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("m_jobCommonInfo is null!");
    }
    return (m_jobCommonInfo == nullptr) ? nullptr
        : std::dynamic_pointer_cast<LivemountJob>(m_jobCommonInfo->GetJobInfo());
}

int VolumeLivemount::PrerequisiteJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

int VolumeLivemount::GenerateSubJobInner()
{
    SubJob subJob {};
    subJob.__set_jobId(m_livemountPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("livemount");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    std::vector<SubJob> vec {};
    vec.push_back(subJob);

    ActionResult result {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(result, vec);
        if (result.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // keep live
        ReportJob(SubJobStatus::RUNNING);
        if (result.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, will try again, jobId:%s, result.code:%d.", m_jobId.c_str(), result.code);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (result.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return Module::FAILED;
    }
    SubJobDetails subJobDetails {};
    subJobDetails.__set_jobId(m_livemountPara->jobId);
    subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    std::string description = "Generate sub task for volume live mount task successfully";
    LogDetail logDetail {};
    std::vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(result, subJobDetails);
    return Module::SUCCESS;
}

int VolumeLivemount::ExecuteSubJobInner()
{
    // 1. setup meta/data repo ptr
    if (!SetupCopyRepo()) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    // 2. extract mount config from json
    std::string jsonString = m_livemountPara->extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
#ifndef WIN32
    PluginUtils::StripEscapeChar(jsonString);
#endif
    INFOLOG("volume livemount parameter extend info %s, jobId: %s, subJobId: %s",
        jsonString.c_str(), m_livemountPara->jobId.c_str(), m_subJobInfo->subJobId.c_str());

    VolumeLivemountExtend volumeLivemountExtend {};
    if (!Module::JsonHelper::JsonStringToStruct(jsonString, volumeLivemountExtend)) {
        ERRLOG("JsonStringToStruct failed: %s, jobId: %s", jsonString.c_str(), m_livemountPara->jobId.c_str());
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    if (!PrepareBasicDirectory(volumeLivemountExtend)) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    // 3. mount nas share
    if (!MountShare()) {
        ERRLOG("mount nas shared failed, jobId: %s", m_livemountPara->jobId.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_host_livemount_execute_script_failed");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    // 4. mount volume
    if (!MountVolumes()) {
        WARNLOG("failed to mount all volumes, jobId %s", m_livemountPara->jobId.c_str());
        // allow partial success
    }
    if (m_mountedRecords.empty()) {
        ERRLOG("none of volume mounted, task fail! jobId %s", m_livemountPara->jobId.c_str());
        UmountNasShare(*m_dataRepo.get(), m_nasShareMountTarget, m_livemountPara->extendInfo);
        ReportJob(SubJobStatus::FAILED);
    }
    // success
    ReportJob(SubJobStatus::COMPLETED);
    INFOLOG("Finish to execute volume livemount subjob, jobId: %s", m_livemountPara->jobId.c_str());
    return Module::SUCCESS;
}

int VolumeLivemount::PostJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

bool VolumeLivemount::MountShare()
{
    return true;
}

void VolumeLivemount::ReportJob(SubJobStatus::type status)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}

bool VolumeLivemount::SetupCopyRepo()
{
    for (const auto& repo : m_livemountPara->copy.repositories) {
        if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
            m_dataRepo = std::make_shared<StorageRepository>(repo);
            m_cloneCopyId = m_livemountPara->copy.id;
            INFOLOG("setup data repo, found copy id %s", m_cloneCopyId.c_str());
        } else if (repo.repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
            m_cacheRepo = std::make_shared<StorageRepository>(repo);
            INFOLOG("setup cache repo, found");
        } else if (repo.repositoryType == RepositoryDataType::type::META_REPOSITORY) {
            m_metaRepo = std::make_shared<StorageRepository>(repo);
            INFOLOG("set up meta repo, found");
        }
    }
    if (m_dataRepo == nullptr || m_cacheRepo == nullptr || m_cacheRepo->path.empty()) {
        ERRLOG("data repo or cache repo invalid, jobId: %s", m_livemountPara->jobId.c_str());
        return false;
    }
    return true;
}

bool VolumeLivemount::PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo)
{
    return true;
}

bool VolumeLivemount::MountVolumes()
{
    return true;
}