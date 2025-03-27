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
#include "DelCopyJob.h"
#include <common/utils/Utils.h>
#include <protect_engines/engine_factory/EngineFactory.h>
#include "job_controller/factory/VirtualizationJobFactory.h"

VIRT_PLUGIN_NAMESPACE_BEGIN
EXTER_ATTACK int32_t DelCopyJob::GenerateSubJob()
{
    GenerateSubJobInner();
    int32_t ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int32_t DelCopyJob::GenerateSubJobInner()
{
    m_stateHandles[static_cast<int32_t>(VirtPlugin::DelCopyJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB)] =
        std::bind(&DelCopyJob::CreateSubTask, this);
    m_nextState = static_cast<int32_t>(VirtPlugin::DelCopyJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB);
    return SUCCESS;
}

int32_t DelCopyJob::CreateSubTask()
{
    std::vector<AppProtect::SubJob> execSubs;
    AppProtect::SubJob subJob {};
    subJob.__set_jobId(m_jobId);
    subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
    subJob.__set_jobName("plugin-virtual-copy-delete");
    subJob.__set_policy(ExecutePolicy::ANY_NODE);
    int32_t priority = 1;
    subJob.__set_jobPriority(priority);
    execSubs.push_back(subJob);
    ActionResult result{};
    JobService::AddNewJob(result, execSubs);
    if (result.code != SUCCESS) {
        ERRLOG("Add new sub job failed.");
        return FAILED;
    }
    m_nextState = static_cast<int32_t>(DelCopyJobSteps::STATE_NONE);
    return SUCCESS;
}

EXTER_ATTACK int32_t DelCopyJob::ExecuteSubJob()
{
    DBGLOG("Enter ExecuteSubJob.");
    int32_t ret = ExecuteSubJobInner();
    ReportJobResultPara reportJobResultPara;
    if (ret != SUCCESS) {
        std::vector<std::string> args = { m_subJobInfo->subJobId };
        ReportJobDetailsParam param = {
            "plugin_task_subjob_fail_label",
            JobLogLevel::TASK_LOG_ERROR,
            SubJobStatus::FAILED, 0 };
        reportJobResultPara.m_jobDetailsParam = param;
        reportJobResultPara.m_args = args;
    }
    ReportJobResult(ret, "ExecuteSubJob finish.", 0, reportJobResultPara);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int32_t DelCopyJob::ExecuteSubJobInner()
{
    InitExecStateMachine();
    return RunStateMachine();
}

void DelCopyJob::InitExecStateMachine()
{
    m_stateHandles[static_cast<int32_t>(DelCopyJobSteps::STEP_EXEC_SUB_JOB_INIT)] =
        std::bind(&DelCopyJob::InitExecInfo, this);
    m_stateHandles[static_cast<int>(DelCopyJobSteps::STATE_EXECJOB_PREHOOK)] =
        std::bind(&DelCopyJob::ExecuteSubJobPreHook, this);
    m_stateHandles[static_cast<int32_t>(DelCopyJobSteps::STEP_EXEC_DELETE_SNAPSHOT)] =
        std::bind(&DelCopyJob::DeleteSnapshot, this);
    m_stateHandles[static_cast<int>(DelCopyJobSteps::STATE_EXECJOB_POSTHOOK)] =
        std::bind(&DelCopyJob::ExecuteSubJobPostHook, this);
    m_nextState = static_cast<int32_t>(DelCopyJobSteps::STEP_EXEC_SUB_JOB_INIT);
}

int32_t DelCopyJob::ExecuteSubJobPreHook()
{
    ExecHookParam para;
    para.hookType = HookType::PRE_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int32_t>(DelCopyJobSteps::STEP_EXEC_DELETE_SNAPSHOT);
    para.postHookState = static_cast<int32_t>(DelCopyJobSteps::STATE_EXECJOB_POSTHOOK);
    return ExecHook(para);
}

int32_t DelCopyJob::ExecuteSubJobPostHook()
{
    ExecHookParam para;
    para.hookType = HookType::POST_HOOK;
    para.stage = JobStage::EXECUTE_SUB_JOB;
    para.nextState = static_cast<int32_t>(DelCopyJobSteps::STATE_NONE);
    return ExecHook(para);
}

int32_t DelCopyJob::InitExecInfo()
{
    if (ParseJobInfo() != SUCCESS) {
        return FAILED;
    }
    std::string snapshotInfoMetaFile = m_metaRepoPath + VIRT_PLUGIN_SNAPSHOT_INFO;
    if (Utils::LoadFileToStructWithRetry(m_metaRepoHandler, snapshotInfoMetaFile, m_snapshotInfo) != SUCCESS) {
        WARNLOG("No snapshot loaded from file(%s) to be deleted, %s", snapshotInfoMetaFile.c_str(), m_taskInfo.c_str());
        return FAILED;
    }
    m_nextState = static_cast<int32_t>(DelCopyJobSteps::STATE_EXECJOB_PREHOOK);
    return SUCCESS;
}

int32_t DelCopyJob::DeleteSnapshot()
{
    m_nextState = static_cast<int>(DelCopyJobSteps::STATE_EXECJOB_POSTHOOK);
    /* delete snapshot */
    if (m_protectEngine->DeleteSnapshot(m_snapshotInfo) != SUCCESS) {
        WARNLOG("Delete snapshot failed, vm: %s, %s", m_snapshotInfo.m_vmMoRef.c_str(), m_taskInfo.c_str());
    }
    /* query snapshot to confirm and change vol snap status */
    if (m_protectEngine->QuerySnapshotExists(m_snapshotInfo) != SUCCESS) {
        WARNLOG("Query snapshot exists fail, vm: %s, %s.", m_snapshotInfo.m_vmMoRef.c_str(), m_taskInfo.c_str());
    }
    /* confirm whether snapshot deleted */
    if (!m_snapshotInfo.m_deleted) {
        WARNLOG("Snapshot not deleted. vm: %s, %s.", m_snapshotInfo.m_vmMoRef.c_str(), m_taskInfo.c_str());
        ReportJobDetailsParam param = {
            "virtual_plugin_backup_job_delete_snapshot_failed_label",
            JobLogLevel::TASK_LOG_WARNING,
            SubJobStatus::RUNNING, 0, 0 };
        std::string residualSnapshotStr = GetSnapshotsLogDetails(m_snapshotInfo.m_volSnapList);
        ReportJobDetailsWithLabel(param, residualSnapshotStr);
    }
    return SUCCESS;
}

std::string DelCopyJob::GetSnapshotsLogDetails(const std::vector<VolSnapInfo>& volSnapshots)
{
    std::string msg;
    for (const auto &volSnap : volSnapshots) {
        msg += ("[vol id: " + volSnap.m_volUuid +
            ", snapshot name: " + volSnap.m_snapshotName + "]");
    }
    return std::move(msg);
}

int32_t DelCopyJob::ParseJobInfo()
{
    // 从任务信息中解析出Repository并进行初始化
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("Job common info is null.");
        return FAILED;
    }
    m_delCopyPara = std::dynamic_pointer_cast<AppProtect::DelCopyJob>(m_jobCommonInfo->GetJobInfo());
    if (m_delCopyPara == nullptr) {
        ERRLOG("Verify job body is empty.");
        return FAILED;
    }
    if (m_delCopyPara->copies.empty()) {
        ERRLOG("No copy data.");
        return FAILED;
    }
    if (InitProtectEngineHandler(JobType::DELCOPY) != SUCCESS) {
        ERRLOG("Initialize protect engine handler failed, %s", m_taskInfo.c_str());
        return FAILED;
    }
    const std::vector<StorageRepository>& vectRepo = m_delCopyPara->copies[0].repositories;
    for (const auto &repo : vectRepo) {
        if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            if (!DoInitHandlers(repo, m_metaRepoHandler, m_metaRepoPath)) {
                ERRLOG("Init meta repo handler failed.");
                return FAILED;
            }
        }
    }
    if (m_metaRepoHandler == nullptr) {
        ERRLOG("Init repo handler failed.meta repo: %ld, taskId: %s.",
            m_metaRepoHandler.get(), m_jobId.c_str());
        return FAILED;
    }
    return SUCCESS;
}

VIRT_PLUGIN_NAMESPACE_END
