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
#include "VolumeCancelLivemount.h"
#include "define/Types.h"
#include "PluginUtilities.h"
#include "VolumeCopyMountProvider.h"
#include "log/Log.h"
#include "FileSystemUtil.h"

using namespace FilePlugin;
using namespace AppProtect;
using namespace volumeprotect;
using namespace volumeprotect::mount;
using namespace Module::FileSystemUtil;

// implement CancelLivemount public methods ...
EXTER_ATTACK int VolumeCancelLivemount::PrerequisiteJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("VolumeCancelLivemount PrerequisiteJob failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeCancelLivemount PrerequisiteJob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    INFOLOG("Leave VolumeCancelLivemount PrerequisiteJob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int VolumeCancelLivemount::GenerateSubJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("VolumeCancelLivemount generate sub job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeCancelLivemount GenerateSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave VolumeCancelLivemount GenerateSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int VolumeCancelLivemount::ExecuteSubJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr || m_subJobInfo == nullptr) {
        ERRLOG("VolumeCancelLivemount execute sub job failed for livemountPara is nullptr or subjobInfo is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeCancelLivemount ExecuteSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave Volumelivemouint ExcuteSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int VolumeCancelLivemount::PostJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("VolumeCancelLivemount post job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter VolumeCancelLivemount Postjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = PostJobInner();
    SetJobToFinish();
    INFOLOG("Leave Volumelivemouint Postjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}

// implement CancelLivemount private methods ...
std::shared_ptr<CancelLivemountJob> VolumeCancelLivemount::GetJobInfoBody() const
{
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("m_jobCommonInfo is null!");
    }
    return (m_jobCommonInfo == nullptr) ? nullptr
        : std::dynamic_pointer_cast<CancelLivemountJob>(m_jobCommonInfo->GetJobInfo());
}

bool VolumeCancelLivemount::SetupCopyRepo()
{
    for (const auto& repo : m_cancelLivemountPara->copy.repositories) {
        if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
            m_dataRepo = std::make_shared<StorageRepository>(repo);
            m_cloneCopyId = m_cancelLivemountPara->copy.id;
            INFOLOG("setup data repo, found copy id %s", m_cloneCopyId.c_str());
        } else if (repo.repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
            m_cacheRepo = std::make_shared<StorageRepository>(repo);
            INFOLOG("setup cache repo, found");
        }
    }
    if (m_dataRepo == nullptr || m_cacheRepo == nullptr || m_cacheRepo->path.empty()) {
        ERRLOG("data repo or cache repo invalid, jobId: %s", m_cancelLivemountPara->jobId.c_str());
        return false;
    }
    VolumeCancelLivemountDataRepoExtendInfo extendInfo;
    if (!Module::JsonHelper::JsonStringToStruct(m_dataRepo->extendInfo, extendInfo)) {
        WARNLOG("convert json failed: %s", m_dataRepo->extendInfo.c_str());
    }
    m_shareName = extendInfo.fileSystemShareInfo.advanceParams.shareName;
    INFOLOG("parse shareName: %s", m_shareName.c_str());
    return true;
}

int VolumeCancelLivemount::PrerequisiteJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}
 
int VolumeCancelLivemount::GenerateSubJobInner()
{
    SubJob subJob {};
    subJob.__set_jobId(m_cancelLivemountPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("livemount");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);

    ActionResult ret {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, std::vector<SubJob>{ subJob });
        if (ret.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
        ReportJob(SubJobStatus::RUNNING);
        if (ret.bodyErr != E_JOB_SERVICE_SUB_JOB_CNT_MAX) {
            WARNLOG("AddNewJob failed, jobId: %s, code: %d, bodyErr: %d", m_jobId.c_str(), ret.code, ret.bodyErr);
            --retryTimes;
            continue;
        }
        WARNLOG("AddNewJob failed, Sub job count of main task: %s has reached max, will try again", m_jobId.c_str());
    }
    if (ret.code != Module::SUCCESS) {
        ERRLOG("AddNewJob timeout 5 min, jobId: %s", m_jobId.c_str());
        return Module::FAILED;
    }
    SubJobDetails subJobDetails {};
    subJobDetails.__set_jobId(m_cancelLivemountPara->jobId);
    subJobDetails.__set_jobStatus(SubJobStatus::COMPLETED);
    std::string description = "Generate sub task for volume cancel livemount task successfully";
    LogDetail logDetail {};
    std::vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(ret, subJobDetails);
    return Module::SUCCESS;
}

int VolumeCancelLivemount::ExecuteSubJobInner()
{
    if (!SetupCopyRepo()) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    std::string jsonString = m_cancelLivemountPara->extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
    INFOLOG("volume cancel livemount parameter extend info: %s", jsonString.c_str());
    if (!LoadBasicDirectory()) {
        ReportJob(SubJobStatus::FAILED);
        ERRLOG("failed to load basic directory created by livemount!");
        return Module::FAILED;
    }
    LoadUmountRecords();
    UmountVolumesFromRecords();
    ForceUmountNasShare(m_nasShareMountTarget);
    std::string livemountTaskDirectory = PluginUtils::PathJoin(VOLUME_LIVEMOUNT_PATH_ROOT, m_cloneCopyId);
    INFOLOG("remove volume livemount task directory %s", livemountTaskDirectory.c_str());
    PluginUtils::Remove(livemountTaskDirectory);
    ReportJob(SubJobStatus::COMPLETED);
    INFOLOG("Finish to execute subjob, job id: %s", m_cancelLivemountPara->jobId.c_str());
    return Module::SUCCESS;
}

void VolumeCancelLivemount::LoadUmountRecords()
{
    INFOLOG("load volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    std::vector<std::string> volumeMountRecordDirNameList;
    if (!PluginUtils::GetDirListInDirectory(m_volumesMountRecordRoot, volumeMountRecordDirNameList, true)) {
        WARNLOG("failed to get volume mount record directories in %s", m_volumesMountRecordRoot.c_str());
    }
    for (const std::string& volumeMountRecordDirName : volumeMountRecordDirNameList) {
        std::vector<std::string> jsonfileList;
        std::string volumeMountRecordDirPath = PluginUtils::PathJoin(
            m_volumesMountRecordRoot, volumeMountRecordDirName);
        INFOLOG("check json file in %s", volumeMountRecordDirPath.c_str());
        PluginUtils::GetFileListInDirectory(volumeMountRecordDirPath, jsonfileList);
        for (const std::string& jsonfile : jsonfileList) {
            if (jsonfile.find(MOUNT_RECORD_JSON_EXTENSION) != std::string::npos) {
                INFOLOG("detected umount record json file %s", jsonfile.c_str());
                m_volumeMountRecordJsonList.push_back(jsonfile);
            }
        }
    }
}

void VolumeCancelLivemount::UmountVolumesFromRecords()
{
    for (const std::string& mountRecordJsonPath : m_volumeMountRecordJsonList) {
        INFOLOG("using %s to umount", mountRecordJsonPath.c_str());
        VolumeMountRecordJsonCommon recordJsonCommon {};
        if (!JsonFileTool::ReadFromFile(mountRecordJsonPath, recordJsonCommon)) {
            ERRLOG("read record json common struct from file: %s failed!", mountRecordJsonPath.c_str());
            continue;
        }
        INFOLOG("check1 : %s", recordJsonCommon.mountTargetPath.c_str());
        UmountVolumeFromRecord(mountRecordJsonPath);
        ReportJobLabel(
            JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_umount_success",
            recordJsonCommon.mountTargetPath);
#ifdef WIN32
        INFOLOG("remove dir: %s", recordJsonCommon.mountTargetPath.c_str());
        if (!::RemoveDirectory(Utf8ToUtf16(recordJsonCommon.mountTargetPath).c_str())) {
            WARNLOG("Remove fail! %d", ::GetLastError());
        }
#endif
    }
}

void VolumeCancelLivemount::UmountVolumeFromRecord(const std::string& mountRecordJsonPath)
{
    INFOLOG("Enter UmountVolumeFromRecord: %s, remoteName: %s", mountRecordJsonPath.c_str(), m_shareName.c_str());
    std::unique_ptr<VolumeCopyUmountProvider> umountProvider
        = VolumeCopyUmountProvider::Build(mountRecordJsonPath, m_shareName);
    if (umountProvider == nullptr) {
        ERRLOG("failed to build umount provider, record path %s", mountRecordJsonPath.c_str());
        return;
    }
    if (!umountProvider->Umount()) {
        WARNLOG("volume umount failed with error %s, record path %s",
            umountProvider->GetError().c_str(), mountRecordJsonPath.c_str());
    }
}

int VolumeCancelLivemount::PostJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}
 
void VolumeCancelLivemount::ReportJob(SubJobStatus::type status)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}

bool VolumeCancelLivemount::LoadBasicDirectory()
{
    return true;
}