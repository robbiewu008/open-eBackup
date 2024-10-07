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
#include "VerifyJob.h"
#include <iostream>
#include <functional>
#include <memory>
#include <string>
#include <openssl/sha.h>
#include "common/Constants.h"
#include "ClientInvoke.h"
#include "common/uuid/Uuid.h"
#include "common/utils/Utils.h"
#include "common/Macros.h"
#include "config_reader/ConfigIniReader.h"
#include "repository_handlers/factory/RepositoryFactory.h"
#include "protect_engines/engine_factory/EngineFactory.h"
#include "job_controller/factory/VirtualizationJobFactory.h"
#include "job_controller/jobs/backup/BackupJob.h"
#include "common/sha256/Sha256.h"
#include "BlockCheckTask.h"

namespace {
// 并发执行线程数
const uint32_t MAX_VERIFY_THREADS = 4;
// 每处理完256个块则上报一次
const uint32_t REPORT_VERIFY_BLOCK_NUM = 256;
}  // namespace

VIRT_PLUGIN_NAMESPACE_BEGIN

VerifyJob::VerifyJob()
{
    InitExecDetails();
}

EXTER_ATTACK int VerifyJob::PrerequisiteJob()
{
    ReportJobResult(SUCCESS, "PrerequisiteJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return SUCCESS;
}

EXTER_ATTACK int VerifyJob::GenerateSubJob()
{
    GenerateSubJobInner();
    m_nextState = static_cast<int>(VirtPlugin::VerifyJobSteps::STEP_GENERATE_SUBJOB_INIT);
    int ret = RunStateMachine();
    ReportJobResult(ret, "GenerateSubJob finish.");
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

int32_t VerifyJob::GenerateSubJobInner()
{
    m_stateHandles[static_cast<int>(VirtPlugin::VerifyJobSteps::STEP_GENERATE_SUBJOB_INIT)] =
        std::bind(&VerifyJob::GenerateJobInit, this);
    m_stateHandles[static_cast<int>(VirtPlugin::VerifyJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB)] =
        std::bind(&VerifyJob::CreateSubTaskByVolumeInfo, this);
    return SUCCESS;
}

int32_t VerifyJob::GenerateJobInit()
{
    if (ParseJobInfo(false) != SUCCESS) {
        ERRLOG("Init common info failed.");
        return FAILED;
    }

    m_nextState = static_cast<int>(VirtPlugin::VerifyJobSteps::STEP_GENERATE_DO_GENERATE_SUBJOB);
    return SUCCESS;
}

int32_t VerifyJob::CreateSubTaskByVolumeInfo()
{
    std::shared_ptr<AppProtect::CheckCopyJob> checkCopy =
        std::dynamic_pointer_cast<AppProtect::CheckCopyJob>(m_jobCommonInfo->GetJobInfo());
    if (checkCopy == nullptr) {
        return FAILED;
    }
    if (checkCopy->copies.empty() || checkCopy->copies[0].extendInfo.empty()) {
        ERRLOG("No copy data.");
        return FAILED;
    }
    CopyExtendInfo copyExtendInfo;
    if (!Module::JsonHelper::JsonStringToStruct(checkCopy->copies[0].extendInfo, copyExtendInfo)) {
        ERRLOG("Copy extend info to struct failed.");
        return FAILED;
    }

    if (copyExtendInfo.m_volList.empty()) {
        ERRLOG("Volume list is empty.");
        return FAILED;
    }

    std::vector<AppProtect::SubJob> execSubs;
    for (const auto& volItem : copyExtendInfo.m_volList) {
        AppProtect::SubJob subJob {};
        subJob.__set_jobId(m_jobId);
        subJob.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
        subJob.__set_jobName("plugin-virtual-copy-verify");
        subJob.__set_policy(ExecutePolicy::ANY_NODE);

        VerifySubJobInfo verifyInfo;
        verifyInfo.m_volUuid = volItem.m_uuid;

        std::string jobInfo = "";
        if (!Module::JsonHelper::StructToJsonString(verifyInfo, jobInfo)) {
            ERRLOG("SubJob extend info fomat error.");
            return FAILED;
        }
        subJob.__set_jobInfo(jobInfo);
        execSubs.push_back(subJob);
    }

    ActionResult result{};
    JobService::AddNewJob(result, execSubs);
    if (result.code != SUCCESS) {
        ERRLOG("Add new sub job failed.");
        return FAILED;
    }

    m_nextState = static_cast<int>(VirtPlugin::VerifyJobSteps::STATE_NONE);
    return SUCCESS;
}

EXTER_ATTACK int VerifyJob::ExecuteSubJob()
{
    DBGLOG("Enter ExecuteSubJob.");
    int ret = ExecuteSubJobInner();
    ReportVerifyState(false, ret);
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    SetJobToFinish();
    return ret;
}

EXTER_ATTACK int VerifyJob::PostJob()
{
    VirtualizationJobFactory::GetInstance()->RemoveFinishJob(GetJobId());
    return SUCCESS;
}

int VerifyJob::ExecuteSubJobInner()
{
    InitExecStateMachine();
    return RunStateMachine();
}

void VerifyJob::InitExecStateMachine()
{
    m_stateHandles[static_cast<int>(VerifyJobSteps::STEP_EXEC_SUB_JOB_INIT)] =
        std::bind(&VerifyJob::InitExecInfo, this);
    m_stateHandles[static_cast<int>(VerifyJobSteps::STEP_EXEC_LOAD_CHECKSUM_FILE)] =
        std::bind(&VerifyJob::LoadCheckSumFile, this);
    m_stateHandles[static_cast<int>(VerifyJobSteps::STEP_EXEC_CHECK_BLOCKS)] = std::bind(&VerifyJob::CheckBlocks, this);
    m_nextState = static_cast<int>(VerifyJobSteps::STEP_EXEC_SUB_JOB_INIT);
}

int VerifyJob::InitExecInfo()
{
    // 解析任务信息
    if (ParseJobInfo() != SUCCESS) {
        return FAILED;
    }

    // 从存储仓中解析SHA256校验文件信息
    int parseRet = ParseCheckSumFileInfo();
    if (parseRet != SUCCESS) {
        return parseRet;
    }
    std::vector<std::string> reportArgs;
    reportArgs.push_back(m_verifySubJob.m_volUuid);
    ReportJobDetailsParam param;
    param = {"virtual_plugin_verify_volume_copy_sub_job_start_label",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, 0};
    ReportJobDetailWithLabel(param, reportArgs);
    m_nextState = static_cast<int>(VerifyJobSteps::STEP_EXEC_LOAD_CHECKSUM_FILE);
    return SUCCESS;
}

int VerifyJob::ParseJobInfo(const bool parseSubJob)
{
    // 从任务信息中解析出Repository并进行初始化
    if (m_jobCommonInfo == nullptr) {
        ERRLOG("Job common info is null.");
        return FAILED;
    }
    std::shared_ptr<AppProtect::CheckCopyJob> jobInfo =
        std::dynamic_pointer_cast<AppProtect::CheckCopyJob>(m_jobCommonInfo->GetJobInfo());
    if (jobInfo == nullptr) {
        ERRLOG("Verify job body is empty.");
        return FAILED;
    }

    if (jobInfo->copies.empty()) {
        ERRLOG("No copy data.");
        return FAILED;
    }

    const std::vector<StorageRepository>& vectRepo = jobInfo->copies[0].repositories;
    for (const auto &repo : vectRepo) {
        if (repo.repositoryType == RepositoryDataType::DATA_REPOSITORY) {
            if (!DoInitHandlers(repo, m_dataRepoHandler, m_dataRepoPath)) {
                ERRLOG("Init data repo handler failed.");
                return FAILED;
            }
        } else if (repo.repositoryType == RepositoryDataType::META_REPOSITORY) {
            if (!DoInitHandlers(repo, m_metaRepoHandler, m_metaRepoPath)) {
                ERRLOG("Init meta repo handler failed.");
                return FAILED;
            }
        }
    }
    if (m_dataRepoHandler == nullptr || m_metaRepoHandler == nullptr) {
        ERRLOG("Init repo handler failed. data repo: %ld, meta repo: %ld, taskId: %s.",
            m_dataRepoHandler.get(), m_metaRepoHandler.get(), m_jobId.c_str());
        return FAILED;
    }

    // 解析校验子任务信息
    if (parseSubJob &&
        (m_subJobInfo == nullptr || !Module::JsonHelper::JsonStringToStruct(m_subJobInfo->jobInfo, m_verifySubJob))) {
        ERRLOG("Get verify subjob info failed.");
        return FAILED;
    }
    return SUCCESS;
}

int VerifyJob::ParseCheckSumFileInfo()
{
    m_checkSumFile = m_metaRepoPath + VIRT_PLUGIN_SHA_FILE_ROOT + m_verifySubJob.m_volUuid + "_sha256.info";
    m_dataImgFile = m_dataRepoPath + Module::PATH_SEPARATOR + m_verifySubJob.m_volUuid + ".raw";
    if (!m_metaRepoHandler->Exists(m_checkSumFile)) {
        ERRLOG("File: %s not exists.", m_checkSumFile.c_str());
        return DAMAGED;
    }
#ifdef WIN32
    if (m_metaRepoHandler->Open(m_checkSumFile, "r") != SUCCESS) {
        ERRLOG("File: %s open failed.", m_checkSumFile.c_str());
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) { m_metaRepoHandler->Close(); });
#endif

    m_totalSize = m_metaRepoHandler->FileSize(m_checkSumFile);
    if (m_totalSize < 0) {
        ERRLOG("Get file size failed, file: %s", m_checkSumFile.c_str());
        return FAILED;
    }
    if (m_totalSize % SHA256_DIGEST_LENGTH != 0) {
        ERRLOG("m_totalSize: %llu is invalid.", m_totalSize);
        return DAMAGED;
    }

    m_offset = 0;
    return SUCCESS;
}

int VerifyJob::LoadCheckSumFile()
{
    DBGLOG("Enter LoadCheckSumFile");
    if (m_metaRepoHandler->Open(m_checkSumFile, "r") != SUCCESS) {
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) { m_metaRepoHandler->Close(); });

    if (m_metaRepoHandler->Seek(m_offset) != 0) {
        ERRLOG("Seek file %s to offset %llu failed", m_checkSumFile.c_str(), m_offset);
        return FAILED;
    }

    // 分段加载SHA256文件，最大加载BLOCKS_SHA256_BUF_SIZE大小的数据
    uint64_t remainSize = m_totalSize - m_offset;
    m_ckSumBufSize =
#ifdef WIN32
        BLOCKS_SHA256_BUF_SIZE > remainSize ? remainSize :
            BLOCKS_SHA256_BUF_SIZE;
#else
        std::min(BLOCKS_SHA256_BUF_SIZE, remainSize);
#endif
    m_ckSumBuf.reset(new uint8_t[m_ckSumBufSize], std::default_delete<uint8_t[]>());
    size_t readSize = m_metaRepoHandler->Read(m_ckSumBuf, m_ckSumBufSize);
    if (readSize != m_ckSumBufSize) {
        ERRLOG("Failed to read sha256 file %s for task %s.", m_checkSumFile.c_str(), m_jobId.c_str());
        return FAILED;
    }
    m_offset += m_ckSumBufSize;  // 作为下一次加载的起始偏移
    DBGLOG("Check sum file total size: %llu, next offset: %llu.", m_totalSize, m_offset);

    // 下一个状态：执行块校验
    m_nextState = static_cast<int>(VerifyJobSteps::STEP_EXEC_CHECK_BLOCKS);
    return SUCCESS;
}

int VerifyJob::CheckBlocks()
{
    if (m_dataRepoHandler->Open(m_dataImgFile, "r") != SUCCESS) {
        return FAILED;
    }
    Utils::Defer _(nullptr, [&](...) { m_dataRepoHandler->Close(); });
    
    TaskPool *tp = TaskPool::GetInstance();
    TaskScheduler ts(*tp);
    int checkValue = CheckBlocksInner(ts);
    if (checkValue != SUCCESS) {
        return checkValue;
    }

    if (m_totalSize > m_offset) {
        m_nextState = static_cast<int>(VerifyJobSteps::STEP_EXEC_LOAD_CHECKSUM_FILE);  // 继续处理下一个SHA256文件分段
        DBGLOG("total_size=%llu, offset=%llu, go on to check.", m_totalSize, m_offset);
    } else {
        m_nextState = static_cast<int>(VerifyJobSteps::STATE_NONE);  // 最后一个分段处理完成
        INFOLOG("All blocks of %s are checked with sha256 checksum file: %s.",
            m_verifySubJob.m_volUuid.c_str(),
            m_checkSumFile.c_str());
    }

    return SUCCESS;
}

int VerifyJob::CheckBlocksInner(TaskScheduler &ts)
{
    size_t offset = 0;
    int taskResult = SUCCESS;
    int32_t nTaskRunning = 0;             // 当前尚在执行的任务数
    uint64_t blockID = (m_offset - m_ckSumBufSize) / SHA256_DIGEST_LENGTH;
    DBGLOG("Start block id: %llu, numbers: %d.", blockID, m_ckSumBufSize / SHA256_DIGEST_LENGTH);
    uint64_t dataFileSize = m_dataRepoHandler->FileSize(m_dataImgFile);
    uint8_t allZero[SHA256_DIGEST_LENGTH] = {0};
    while (offset < m_ckSumBufSize) {
        if (IsAbortJob()) {
            ERRLOG("Receive abort req, break job.");
            break;
        }
        DBGLOG("block id: %llu, offset: %zu.", blockID, offset);
        std::shared_ptr<uint8_t[]> checkSum = std::shared_ptr<uint8_t[]>(m_ckSumBuf, m_ckSumBuf.get() + offset);
        if (memcmp(checkSum.get(), allZero, SHA256_DIGEST_LENGTH) == 0) {
            DBGLOG("%llu is hole block, skip it.", blockID);
            ++blockID;
            offset += SHA256_DIGEST_LENGTH;
            continue;
        }
        if (nTaskRunning < MAX_VERIFY_THREADS) {
            std::shared_ptr<BlockCheckTask> task =
                std::make_shared<BlockCheckTask>(blockID, dataFileSize, checkSum, m_dataRepoHandler);
            if (!ts.Put(task)) {
                ERRLOG("Put block task to pool failed, blockID: %llu", blockID);
                continue;
            }
            ++nTaskRunning;
            ++blockID;
            offset += SHA256_DIGEST_LENGTH;
        } else {
            taskResult = TaskExecResult(ts, nTaskRunning);
            if (taskResult != SUCCESS) {
                break;
            }
        }
        if ((blockID + 1) % REPORT_VERIFY_BLOCK_NUM == 0) {
            // 每处理完256个块就上报一次任务状态
            ReportVerifyState(true);
        }
    }

    int endResult = TaskExecEnd(ts, nTaskRunning);
    if (IsAbortJob()) {
        ERRLOG("Receive abort req, task threads already end, stop job.");
        return FAILED;
    }

    return GetTaskResult(taskResult, endResult);
}

int VerifyJob::GetTaskResult(const int taskExecRet, const int endExecRet)
{
    std::vector<int> vctResult;
    vctResult.push_back(taskExecRet);
    vctResult.push_back(endExecRet);

    // 只有taskExecRet和endExecRet两者都返回true，任务才算成功
    if (std::all_of(vctResult.begin(), vctResult.end(), [](int ret) { return ret == SUCCESS; })) {
        ReportVerifyState(true);
        return SUCCESS;
    } else if (std::any_of(vctResult.begin(), vctResult.end(), [](int ret) { return ret == DAMAGED; })) {
        ERRLOG("Check block DAMAGED, vol_id: %s, task_id: %s", m_verifySubJob.m_volUuid.c_str(), m_jobId.c_str());
        return DAMAGED;
    } else {
        ERRLOG("Check block failed, vol_id: %s, task_id: %s", m_verifySubJob.m_volUuid.c_str(), m_jobId.c_str());
        return FAILED;
    }
}

void VerifyJob::ReportVerifyState(const bool jobRunning, const int execRet /* = SUCCESS */)
{
    // 上报任务状态
    ReportJobDetailsParam param;
    if (jobRunning) {
        param = { "", JobLogLevel::TASK_LOG_INFO, SubJobStatus::RUNNING, 0 };
    } else {
        std::map<int, ReportJobDetailsParam>::iterator iter = m_mapExecRetDetails.find(execRet);
        if (iter != m_mapExecRetDetails.end()) {
            param = m_mapExecRetDetails[execRet];
        } else {
            param = { "" };
        }
    }

    ReportJobDetailsWithLabel(param, std::string("Virtual plugin report verify state"));
}

int VerifyJob::TaskExecResult(TaskScheduler &ts, int &nTaskRunning)
{
    int damagedRet = SUCCESS;
    int failedRet = SUCCESS;
    std::shared_ptr<BlockTask> execTask;
    uint32_t timeout = 5;  // ms
    while (ts.Get(execTask, timeout)) {
        int taskRet = execTask->Result();
        if (taskRet != SUCCESS) {
            ERRLOG("Task exec failed. result: %d ", taskRet);
            CheckTaskExecResult(taskRet, damagedRet, failedRet);
        }
        --nTaskRunning;
    }
    return (damagedRet == DAMAGED ? damagedRet : failedRet);
}

void VerifyJob::CheckTaskExecResult(const int taskRet, int& damagedRet, int& failedRet)
{
    damagedRet = taskRet == DAMAGED ? DAMAGED : damagedRet;
    failedRet = taskRet == FAILED ? FAILED : failedRet;
}

int VerifyJob::TaskExecEnd(TaskScheduler &ts, int &nTaskRunning)
{
    int damagedRet = SUCCESS;
    int failedRet = SUCCESS;
    std::shared_ptr<BlockTask> execTask;
    while (nTaskRunning > 0) {
        if (!ts.Get(execTask)) {
            failedRet = FAILED;
            break;
        }
        int taskRet = execTask->Result();
        if (taskRet != SUCCESS) {
            ERRLOG("Task exec failed. result: %d ", taskRet);
            CheckTaskExecResult(taskRet, damagedRet, failedRet);
        }
        --nTaskRunning;
    }
    return (damagedRet == DAMAGED ? damagedRet : failedRet);
}

void VerifyJob::InitExecDetails()
{
    ReportJobDetailsParam param = { "", JobLogLevel::TASK_LOG_ERROR, SubJobStatus::COMPLETED, 0 };
    VerifyFileInfo verifyInfo;
    verifyInfo.m_verifyFileDmage = "true";

    // 校验为文件损坏
    param.status = SubJobStatus::FAILED;
    Module::JsonHelper::StructToJsonString(verifyInfo, param.extendInfo);
    m_mapExecRetDetails[DAMAGED] = param;

    // 执行失败 文件未损坏
    verifyInfo.m_verifyFileDmage = "false";
    param.status = SubJobStatus::FAILED;
    Module::JsonHelper::StructToJsonString(verifyInfo, param.extendInfo);
    m_mapExecRetDetails[FAILED] = param;

    // 执行成功 文件未损坏
    verifyInfo.m_verifyFileDmage = "false";
    param.level = JobLogLevel::TASK_LOG_INFO;
    param.status = SubJobStatus::COMPLETED;
    Module::JsonHelper::StructToJsonString(verifyInfo, param.extendInfo);
    m_mapExecRetDetails[SUCCESS] = param;
}

VIRT_PLUGIN_NAMESPACE_END
