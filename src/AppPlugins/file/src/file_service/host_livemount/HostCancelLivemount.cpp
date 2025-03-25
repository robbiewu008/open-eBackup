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
#include "HostCancelLivemount.h"
#include "log/Log.h"
#include "ClientInvoke.h"
#include "PluginUtilities.h"
#include "HostLivemount.h"
#include "system/System.hpp"
#include "constant/PluginConstants.h"
#include "constant/ErrorCode.h"
#include "common/Thread.h"
#include "common/EnvVarManager.h"

using namespace std;
using namespace AppProtect;
using namespace Module;
 
namespace FilePlugin {
namespace {
    constexpr auto MODULE = "HostCancelLivemount";
    const uint32_t FILE_NOT_FOUND_ERRORCODE = 2;
}
 
std::shared_ptr<CancelLivemountJob> HostCancelLivemount::GetJobInfoBody()
{
    std::shared_ptr<CancelLivemountJob> jobPtr = dynamic_pointer_cast<CancelLivemountJob>(
        m_jobCommonInfo->GetJobInfo());
    return jobPtr;
}
 
EXTER_ATTACK int HostCancelLivemount::PrerequisiteJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("HostCancelLivemount PrerequisiteJob failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostCancelLivemount PrerequisiteJob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = PrerequisiteJobInner();
    SetJobToFinish();
    INFOLOG("Leave HostCancelLivemount PrerequisiteJob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int HostCancelLivemount::GenerateSubJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("HostCancelLivemount generate sub job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostCancelLivemount GenerateSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = GenerateSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave HostCancelLivemount GenerateSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int HostCancelLivemount::ExecuteSubJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr ||
        m_subJobInfo == nullptr) {
        ERRLOG("HostCancelLivemount execute sub job failed for livemountPara is nullptr or subjobInfo is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostCancelLivemount ExecuteSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = ExecuteSubJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint ExcuteSubjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
EXTER_ATTACK int HostCancelLivemount::PostJob()
{
    m_cancelLivemountPara = GetJobInfoBody();
    if (m_cancelLivemountPara == nullptr) {
        ERRLOG("HostCancelLivemount post job failed for livemountPara is nullptr");
        ReportJob(SubJobStatus::FAILED);
        SetJobToFinish();
        return Module::FAILED;
    }
    INFOLOG("Enter HostCancelLivemount Postjob, %s", m_cancelLivemountPara->jobId.c_str());
    int ret = PostJobInner();
    SetJobToFinish();
    INFOLOG("Leave Hostlivemouint Postjob, %s", m_cancelLivemountPara->jobId.c_str());
    return ret;
}
 
int HostCancelLivemount::PrerequisiteJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}
 
int HostCancelLivemount::GenerateSubJobInner()
{
    SubJob subJob {};
    subJob.__set_jobId(m_cancelLivemountPara->jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("livemount");
    subJob.__set_policy(ExecutePolicy::LOCAL_NODE);
    vector<SubJob> vec {};
    vec.push_back(subJob);
 
    ActionResult ret {};
    int retryTimes = SEND_ADDNEWJOB_RETRY_TIMES;
    while (retryTimes > 0) {
        JobService::AddNewJob(ret, vec);
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
    string description = "Generate sub task for live mount task successfully";
    LogDetail logDetail {};
    vector<LogDetail> logDetails {};
    logDetail.__set_description(description);
    logDetails.push_back(logDetail);
    subJobDetails.__set_logDetail(logDetails);
    JobService::ReportJobDetails(ret, subJobDetails);
    return Module::SUCCESS;
}

int HostCancelLivemount::GetCmdArgForWin32(std::string& exeCmd)
{
    // get mount drive name
    HostLivemountCopyExtendAddDrive copyExtendInfo;
    string jsonString = m_cancelLivemountPara->copy.extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
    if (!Module::JsonHelper::JsonStringToStruct(jsonString, copyExtendInfo)) {
        ERRLOG("json trans failed!");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    exeCmd = copyExtendInfo.driveInfo;
    return Module::SUCCESS;
}

int HostCancelLivemount::GetCmdArgForLinux(std::string& exeCmd)
{
    // 取出extendInfo的targetPath, 传给data repo的path
    HostLivemountExtend extendInfo;
    string jsonString = m_cancelLivemountPara->extendInfo;
    PluginUtils::StripWhiteSpace(jsonString);
    if (!Module::JsonHelper::JsonStringToStruct(jsonString, extendInfo)) {
        ERRLOG("json trans failed!");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    int ret = IdentifyDataRepo();
    if (ret != Module::SUCCESS || !m_dataRepo) {
        ERRLOG("data repo not found");
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    if (!CheckBlackList(extendInfo.dstPath)) {
        ERRLOG("about to umount dir in black list! %s", extendInfo.dstPath.c_str());
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
    exeCmd = extendInfo.dstPath;

    return Module::SUCCESS;
}

int HostCancelLivemount::ExecuteSubJobInner()
{
    string mp;
#ifdef WIN32
    if (GetCmdArgForWin32(mp) != Module::SUCCESS) {
        ERRLOG("get cancel livemount command argument for windows failed!");
        return Module::FAILED;
    }
    // agent通过&拼接了挂载点和网络共享
    // 例如：C:\mnt\databackup\general_type\Windows_CIFS_MOUNT\\129.115.135.13&\\129.115.135.13\mount_1697871948854
    size_t pos = mp.find('&');
    if (pos == std::string::npos) {
        ERRLOG("The mounting information does not meet the specifications. %s.", mp.c_str());
        return Module::FAILED;
    }
    std::string mountPath = mp.substr(0, pos);
    mp = mp.substr(pos + 1);
    INFOLOG("MountPath: %s, NetShare: %s.", mountPath.c_str(), mp.c_str());
#else
    if (GetCmdArgForLinux(mp) != Module::SUCCESS) {
        ERRLOG("get cancel livemount command argument for linux failed!");
        return Module::FAILED;
    }
#endif

    bool execret = ExecuteUmountCmd(mp);
    if (!execret) {
        ReportJob(SubJobStatus::FAILED);
        return Module::FAILED;
    }
#ifdef WIN32
    // WIN下网络共享解除后需要删除挂载点
    PluginUtils::Remove(mountPath);
#endif
    ReportJob(SubJobStatus::COMPLETED);
    INFOLOG("Finish to execute subjob , main job id: %s, subjob id: %s",
        m_cancelLivemountPara->jobId.c_str(), m_subJobInfo->subJobId.c_str());
    return Module::SUCCESS;
}

bool HostCancelLivemount::ExecuteUmountCmd(const std::string& mp)
{
#ifdef WIN32
    string cmd = R"(net use )" + PluginUtils::ReverseSlash(mp) + R"( /delete /y)";
    INFOLOG("compress cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    // 挂载点失效，任务返回成功
    if (errCode == FILE_NOT_FOUND_ERRORCODE) {
        WARNLOG("The mount point is invalid, plz check the host.");
        return true;
    }
    if (ret != 0 || errCode != 0) {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return false;
    }
    return true;
#else
    if (!CheckMountPointExists(mp)) {
        WARNLOG("%s has already been umounted", mp.c_str());
        return true;
    }
#ifdef __linux__
    string cmd = "umount -l " + mp;
#else
    string cmd = "umount -f " + mp;
#endif
    INFOLOG("umount cmd : %s", cmd.c_str());
    vector<string> output;
    vector<string> errOutput;
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, {}, output, errOutput);
    if (ret != 0) {
        ERRLOG("call umount failed! %s", mp.c_str());
        for (auto msg : errOutput) {
            WARNLOG("errmsg : %s", msg.c_str());
        }
        return false;
    }
    return true;
#endif
}

bool HostCancelLivemount::CheckMountPointExists(const std::string& mp)
{
    m_deviceMountPtr = make_shared<DeviceMount>();
    if (m_deviceMountPtr == nullptr) {
        ERRLOG("m_deviceMountPtr is nullptr");
        return false;
    }
    if (!m_deviceMountPtr->LoadDevice()) {
        ERRLOG("load device failed");
        return false;
    }
    return m_deviceMountPtr->CheckWhetherMountPoint(mp);
}

int HostCancelLivemount::PostJobInner()
{
    ReportJob(SubJobStatus::COMPLETED);
    return Module::SUCCESS;
}
 
void HostCancelLivemount::ReportJob(SubJobStatus::type status)
{
    INFOLOG("Enter ReportJob: %s, %d", m_cancelLivemountPara->jobId.c_str(), static_cast<int>(status));
    SubJobDetails subJobDetails;
    LogDetail logDetail{};
    ActionResult result;
    std::vector<LogDetail> logDetailList;
    AddLogDetail(logDetail, "", JobLogLevel::TASK_LOG_INFO);
    REPORT_LOG2AGENT(subJobDetails, result, logDetailList, logDetail, 0, 0, status);
}
 
int HostCancelLivemount::IdentifyDataRepo()
{
    for (unsigned int i = 0; i < m_cancelLivemountPara->copy.repositories.size(); i++) {
        if (m_cancelLivemountPara->copy.repositories[i].repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
            DBGLOG("set data repo");
            m_dataRepo = std::make_shared<StorageRepository>(m_cancelLivemountPara->copy.repositories[i]);
            return Module::SUCCESS;
        }
    }
    return Module::FAILED;
}

bool HostCancelLivemount::CheckBlackList(const std::string& dir) const
{
    // 父目录和子目录都不能选择
    vector<string> blackList1 {"/bin", "/boot", "/dev", "/etc", "/lib", "/lost+found", "/media", "/proc",
        "/root", "/sbin", "/selinux", "/srv", "/sys", "/usr", "/usr/bin", "/usr/src", "/var", "/run"};
    for (auto path : blackList1) {
        if (path == dir.substr(0, path.length())) {
            return false;
        }
    }
    // 父目录不能选择， 子目录可以选择
    string agentHomePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath();
    vector<string> blackList2 {"/", "/mnt", agentHomePath};
    for (auto path : blackList2) {
        if (dir == path) {
            return false;
        }
    }
    return true;
}
}