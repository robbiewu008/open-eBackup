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

#include "JobExecution.h"
#include "log/Log.h"
#include "ApplicationProtectBaseDataType_types.h"
#include "JobMgr.h"

using namespace common::jobmanager;

namespace {
    constexpr auto MODULE = "JobExecution";
}

JobExecution::JobExecution()
{
    m_funcMap[OperType::PRE] = CallFunc(
        std::bind(&JobExecution::ExecPreJob, this, std::placeholders::_1, std::placeholders::_2));
    m_funcMap[OperType::GENERATE] = CallFunc(
        std::bind(&JobExecution::ExecGenSubJob, this, std::placeholders::_1, std::placeholders::_2));
    m_funcMap[OperType::EXECUTE] = CallFunc(
        std::bind(&JobExecution::ExecSubJob, this, std::placeholders::_1, std::placeholders::_2));
    m_funcMap[OperType::POST] = CallFunc(
        std::bind(&JobExecution::ExecPostJob, this, std::placeholders::_1, std::placeholders::_2));
    m_funcMap[OperType::RESOURCE] = CallFunc(
        std::bind(&JobExecution::ExecuteAsyncJob, this, std::placeholders::_1, std::placeholders::_2));
}

JobExecution::~JobExecution() {}

int JobExecution::ExecuteJob(AppProtect::ActionResult& result, const std::shared_ptr<BasicJob>& job,
    const std::string& jobId, OperType type)
{
    auto func = m_funcMap[type];
    int ret = func(job, jobId);
    return ret;
}

int JobExecution::ExecPreJob(std::shared_ptr<BasicJob> job, const std::string& jobId)
{
    if (JobMgr::GetInstance().CheckJobIdExist(jobId)) {
        HCP_Log(WARN, MODULE) << "job id already exist in job map jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(&BasicJob::PrerequisiteJob, job);
    return InitAsyncThread(job, thread, jobId);
}

int JobExecution::ExecGenSubJob(std::shared_ptr<BasicJob> job, const std::string& jobId)
{
    if (JobMgr::GetInstance().CheckJobIdExist(jobId)) {
        HCP_Log(WARN, MODULE) << "job id already exist in job map jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(&BasicJob::GenerateSubJob, job);
    return InitAsyncThread(job, thread, jobId);
}

int JobExecution::ExecSubJob(std::shared_ptr<BasicJob> job, const std::string& jobId)
{
    if (JobMgr::GetInstance().CheckJobIdExist(jobId)) {
        HCP_Log(WARN, MODULE) << "job id already exist in job map jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(&BasicJob::ExecuteSubJob, job);
    return InitAsyncThread(job, thread, jobId);
}

int JobExecution::ExecPostJob(std::shared_ptr<BasicJob> job, const std::string& jobId)
{
    if (JobMgr::GetInstance().CheckJobIdExist(jobId)) {
        HCP_Log(WARN, MODULE) << "job id already exist in job map jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(&BasicJob::PostJob, job);
    return InitAsyncThread(job, thread, jobId);
}

int JobExecution::ExecuteAsyncJob(std::shared_ptr<BasicJob> job, const std::string& jobId)
{
    HCP_Log(INFO, MODULE) << "ExecuteAsyncJob jobId: " << jobId << HCPENDLOG;
    JobMgr::GetInstance().EraseFinishJob();
    HCP_Log(INFO, MODULE) << "Erase finish job" << HCPENDLOG;
    if (JobMgr::GetInstance().CheckJobIdExist(jobId)) {
        HCP_Log(WARN, MODULE) << "job id already exist in job map jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }
    if (job == nullptr) {
        HCP_Log(ERR, MODULE) << "Job nullptr! jobId: " << jobId << HCPENDLOG;
        return Module::FAILED;
    }
    std::shared_ptr<std::thread> thread = std::make_shared<std::thread>(
        &BasicJob::ExecuteAsyncJob, job);
    return InitReentrantAsyncThread(job, thread, jobId);
}

int JobExecution::InitReentrantAsyncThread(std::shared_ptr<BasicJob> job,
    std::shared_ptr<std::thread> thread, const std::string& jobId)
{
    if (thread == nullptr || job == nullptr) {
        HCP_Log(ERR, MODULE) << "job thread is null" << HCPENDLOG;
        return Module::FAILED;
    }
    job->SetJobThread(thread);
    job->DetachJobThread();
    bool jobExists = JobMgr::GetInstance().CheckJobIdExist(jobId);
    if (!jobExists && (JobMgr::GetInstance().InsertJob(jobId, job) != Module::SUCCESS)) {
        HCP_Log(ERR, MODULE) << "insert job into job mgr map fail" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

int JobExecution::InitAsyncThread(std::shared_ptr<BasicJob> job, std::shared_ptr<std::thread> thread,
    const std::string& jobId)
{
    if (thread == nullptr) {
        HCP_Log(ERR, MODULE) << "job thread is null" << HCPENDLOG;
        return Module::FAILED;
    }
    job->SetJobThread(thread);
    job->DetachJobThread();
    if (JobMgr::GetInstance().InsertJob(jobId, job) != Module::SUCCESS) {
        HCP_Log(ERR, MODULE) << "insert job into job mgr map fail" << HCPENDLOG;
        return Module::FAILED;
    }
    return Module::SUCCESS;
}
