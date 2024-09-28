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
#include "HostLivemount.h"
#include "log/Log.h"
#include "ClientInvoke.h"
#include "PluginUtilities.h"
#include "constant/PluginConstants.h"
#include "constant/ErrorCode.h"
#include "common/Thread.h"

using namespace std;
using namespace AppProtect;

namespace FilePlugin {
namespace {
    constexpr auto MODULE = "HostLivemount";
    constexpr uint8_t INDEX2 = 2;
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
}

std::shared_ptr<LivemountJob> HostLivemount::GetJobInfoBody()
{
    std::shared_ptr<LivemountJob> jobPtr = dynamic_pointer_cast<LivemountJob>(m_jobCommonInfo->GetJobInfo());
    return jobPtr;
}

EXTER_ATTACK int HostLivemount::PrerequisiteJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("HostLivemount PrerequisiteJob failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter Hostlivemount PrerequisiteJob, %s", m_livemountPara->jobId.c_str());
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemount PrerequisiteJob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostLivemount::GenerateSubJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("HostLivemount generate sub job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter Hostlivemount GenerateSubjob, %s", m_livemountPara->jobId.c_str());
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemount GenerateSubjob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostLivemount::ExecuteSubJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr ||
        m_subJobInfo == nullptr) {
        ERRLOG("HostLivemount execute sub job failed for livemountPara is nullptr or subjobInfo is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostLivemount ExecuteSubjob, %s", m_livemountPara->jobId.c_str());
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint ExcuteSubjob, %s", m_livemountPara->jobId.c_str());
    return ret;
}

EXTER_ATTACK int HostLivemount::PostJob()
{
    m_livemountPara = GetJobInfoBody();
    if (m_livemountPara == nullptr) {
        ERRLOG("HostLivemount post job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostLivemount Postjob, %s", m_livemountPara->jobId);
    int ret = PostJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint Postjob, %s", m_livemountPara->jobId);
    return ret;
}

int HostLivemount::PrerequisiteJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

int HostLivemount::GenerateSubJobInner()
{
    SubJob subJob {};
    subJob.__set_jobId(m_livemountPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("livemount");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    vector<SubJob> vec {};
    vec.push_back(subJob);

    ActionResult result {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(result, vec);
        if (result.code == Module::SUCCESS) {
            break;
        }
        Module::SleepFor(std::chrono::seconds(SEND_ADDNEWJOB_RETRY_INTERVAL));
        // 重试阶段上报任务状态为Running
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
    string description = "Generate sub task for live mount task successfully";
    LogDetail logDetail {};
    vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(result, subJobDetails);
    return Module::SUCCESS;
}

int HostLivemount::AddDriveNameToExtendInfo(const std::string& driveName)
{
    HostLivemountCopyExtendAddDrive copyExtendInfo;
    // step1: 获取m_livemountPara->copy.extendInfo数据，转换成结构体
    string jsonString = m_livemountPara->copy.extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
    if (!Module::JsonHelper::JsonStringToStruct(jsonString, copyExtendInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToStruct failed, m_filesetExt" << HCPENDLOG;
        return Module::FAILED;
    }

    // step2: 盘符信息传入copyExtendInfo的driveInfo
    copyExtendInfo.driveInfo = driveName;

    // step3: 结构体转换成json string
    string jsonStringNew;
    if (!Module::JsonHelper::StructToJsonString(copyExtendInfo, jsonStringNew)) {
        HCP_Log(ERR, MODULE) << "StructToJsonString failed, m_filesetExt" << HCPENDLOG;
        return Module::FAILED;
    }

    // step4: 更新的json string添加到m_livemountPara->copy.extendInfo
    m_livemountPara->copy.__set_extendInfo(jsonStringNew);

    // step5: 保存extendInfo信息到数据库
    ActionResult result;
    m_livemountPara->copy.repositories = {};
    JobService::ReportCopyAdditionalInfo(result, m_parentJobId, m_livemountPara->copy);

    return Module::SUCCESS;
}

int HostLivemount::GetFileSetMountDriveInfo(ActionResult &actionResult)
{
    FileSetMountDrive driveInfo;
    if (!Module::JsonHelper::JsonStringToStruct(actionResult.message, driveInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToStruct failed, FileSetMountDrive" << HCPENDLOG;
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }

    if (AddDriveNameToExtendInfo(driveInfo.mountDirve) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "AddDriveNameToExtendInfo failed, FileSetMountDrive" << HCPENDLOG;
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    // agent通过&拼接了挂载点和网络共享
    // 例如：C:\mnt\databackup\general_type\Windows_CIFS_MOUNT\\129.115.135.13&\\129.115.135.13\mount_1697871948854
    size_t pos = driveInfo.mountDirve.find('&');
    if (pos == std::string::npos) {
        ERRLOG("The mounting information does not meet the specifications. %s.", driveInfo.mountDirve.c_str());
        return Module::FAILED;
    }
    std::string mountPath = driveInfo.mountDirve.substr(0, pos);
#ifdef WIN32
    AddLogDetail(logDetail, "file_plugin_host_livemount_cifs_mountinfo_label", JobLogLevel::TASK_LOG_INFO, mountPath);
#else
    AddLogDetail(logDetail, "file_plugin_livemount_nfs_mountinfo_label", JobLogLevel::TASK_LOG_INFO, mountPath);
#endif
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, SubJobStatus::RUNNING);
    return Module::SUCCESS;
}

int HostLivemount::ExecuteSubJobInner()
{
    // 取出extendInfo的targetPath, 传给data repo的path
    HostLivemountExtend extendInfo;
    string jsonString = m_livemountPara->extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
    DBGLOG("After Strip: %s", jsonString.c_str());
    if (!Module::JsonHelper::JsonStringToStruct(jsonString, extendInfo)) {
        HCP_Log(ERR, MODULE) << "JsonStringToStruct failed, m_filesetExt" << HCPENDLOG;
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    int ret = IdentifyDataRepo();
    if (ret != Module::SUCCESS || !m_dataRepo) {
        ERRLOG("data repo not found");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    DBGLOG("dst path is %s", extendInfo.dstPath.c_str());
    vector<StorageRepository> dataRepoVec{ *m_dataRepo.get() };
    ActionResult ar;
    PrepareRepositoryByPlugin repos;
    JobPermission permission;
    permission.__set_user("0");
    permission.__set_group("0");
    vector<string> path{};
    // clear
    dataRepoVec[0].__set_path(path);
    path.emplace_back(extendInfo.dstPath);
    // set
    dataRepoVec[0].__set_path(path);
    repos.__set_repository(dataRepoVec);
    repos.__set_permission(permission);
    repos.__set_extendInfo(m_livemountPara->extendInfo);
    JobService::MountRepositoryByPlugin(ar, repos);
    if (ar.code != 0) {
        ERRLOG("Call MountRepositoryByPlugin failed!");
        if (ar.code != ERROR_POINT_MOUNTED) {
            JobService::UnMountRepositoryByPlugin(ar, repos);
        }
        ReportJobDetailsWithLabelAndErrcode(make_tuple(JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, PROGRESS100),
            "file_plugin_host_livemount_execute_script_failed", ar.bodyErr, ar.message);
        return Module::FAILED;
    }

#ifdef WIN32
    // 取出FileSetMountDrive的target Drive Name
    if (GetFileSetMountDriveInfo(ar) != Module::SUCCESS) {
        return Module::FAILED;
    }
#endif

    ReportJob(SubJobStatus::COMPLETED);
    INFOLOG("Finish to execute subjob , main job id: %s, subjob id: %s",
        m_livemountPara->jobId.c_str(), m_subJobInfo->subJobId.c_str());
    return Module::SUCCESS;
}

int HostLivemount::PostJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}

void HostLivemount::ReportJobDetailsWithLabelAndErrcode(
    const std::tuple<JobLogLevel::type, SubJobStatus::type, const int>& reportInfo,
    const std::string& logLabel, const int64_t errCode, const std::string& message)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail {};
    logDetail.__set_additionalDesc(vector<string>{message});
    std::vector<LogDetail> logDetailList;
    ActionResult result;
    SubJobStatus::type jobStatus = std::get<1>(reportInfo);
    AddLogDetail(logDetail, logLabel, std::get<0>(reportInfo));
    AddErrCode(logDetail, errCode);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail,
        std::get<INDEX2>(reportInfo), 0, jobStatus);
}

void HostLivemount::ReportJob(SubJobStatus::type status)
{
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}

int HostLivemount::IdentifyDataRepo()
{
    for (unsigned int i = 0; i < m_livemountPara->copy.repositories.size(); i++) {
        if (m_livemountPara->copy.repositories[i].repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
            DBGLOG("set data repo");
            m_dataRepo = std::make_shared<StorageRepository>(m_livemountPara->copy.repositories[i]);
            return Module::SUCCESS;
        }
    }
    return Module::FAILED;
}
}
