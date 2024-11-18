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
#include "BasicJob.h"
#include "log/Log.h"

namespace {
    constexpr auto MODULE = "BasicJob";
}

BasicJob::BasicJob()
{}

BasicJob::~BasicJob()
{}

int BasicJob::PrerequisiteJob()
{
    SetJobToFinish();
    return Module::SUCCESS;
}
int BasicJob::GenerateSubJob()
{
    SetJobToFinish();
    return Module::SUCCESS;
}
int BasicJob::ExecuteSubJob()
{
    SetJobToFinish();
    return Module::SUCCESS;
}
int BasicJob::PostJob()
{
    SetJobToFinish();
    return Module::SUCCESS;
}

void BasicJob::SetJobThread(std::shared_ptr<std::thread> th)
{
    m_jobThread = std::move(th);
}

std::shared_ptr<std::thread> BasicJob::GetJobThread()
{
    return m_jobThread;
}

void BasicJob::SetJobInfo(std::shared_ptr<JobCommonInfo> info)
{
    m_jobCommonInfo = info;
}

std::string BasicJob::GetJobId()
{
    return m_jobId;
}

std::string BasicJob::GetParentJobId()
{
    return m_parentJobId;
}

std::string BasicJob::GetSubJobId()
{
    return (m_subJobInfo != nullptr) ? m_subJobInfo->subJobId : "";
}

int BasicJob::DetachJobThread()
{
    if (!m_jobThread->joinable()) {
        HCP_Log(INFO, MODULE) << "thread is not joinable, not do detach" << HCPENDLOG;
        return Module::SUCCESS;
    }
    m_jobThread->detach();
    return Module::SUCCESS;
}

void BasicJob::SetJobId(const std::string &jobId)
{
    m_jobId = jobId;
}

void BasicJob::SetParentJobId(const std::string &parentJobId)
{
    m_parentJobId = parentJobId;
}

void BasicJob::SetSubJob(std::shared_ptr<SubJob> subJob)
{
    m_subJobInfo = subJob;
}

std::shared_ptr<JobCommonInfo> BasicJob::GetJobInfo()
{
    return m_jobCommonInfo;
}

bool BasicJob::IsAbortJob() const
{
    return m_isAbort;
}

void BasicJob::SetJobToFinish()
{
    HCP_Log(INFO, MODULE) << "Set job to finish" << HCPENDLOG;
    m_isFinish = true;
}

void BasicJob::SetJobToPause()
{
    m_isPause = true;
}

bool BasicJob::IsJobPause() const
{
    return m_isPause;
}

void BasicJob::SetProgress(int p)
{
    m_progress = p;
}

void BasicJob::SetJobAborted()
{
    m_isAbort = true;
}

void BasicJob::EndJob(AppProtect::SubJobStatus::type jobStatus)
{
    if (jobStatus == AppProtect::SubJobStatus::COMPLETED) {
        HCP_Log(INFO, MODULE) << "end job with completed" << HCPENDLOG;
    } else {
        HCP_Log(INFO, MODULE) << "end job with abnormal" << HCPENDLOG;
    }
    SetJobToFinish();
}

bool BasicJob::IsJobFinish()
{
    return m_isFinish;
}

void BasicJob::SetPostJobResultType(AppProtect::JobResult::type type)
{
    m_jobResult = type;
}

int BasicJob::RunStateMachine()
{
    int iRet = Module::FAILED;
    try {
        while (m_nextState > 0) {
            auto iter = m_stateHandles.find(m_nextState);
            if (m_stateHandles.end() == iter) {
                HCP_Log(ERR, MODULE) << "Unknown state, can't be processed." << DBG(m_nextState) << HCPENDLOG;
                iRet = Module::FAILED;
                break;
            }
            iRet = iter->second();
            if (iRet != Module::SUCCESS) {
                HCP_Log(ERR, MODULE) << "Failed to run state machine, state: " << m_nextState << HCPENDLOG;
                m_nextState = -1;
                return iRet;
            }
            if (AbortJob() != Module::SUCCESS) {
                m_nextState = -1;
                iRet = Module::FAILED;
            }
        }
    } catch (const std::exception& ex) {
        HCP_Log(ERR, MODULE) << "Exec state failed, state. " << m_nextState << " msg=" << ex.what() << HCPENDLOG;
    } catch (...) {
        HCP_Log(ERR, MODULE) << "Exec state failed, state: " << m_nextState << HCPENDLOG;
    }
    return iRet;
}

int BasicJob::AbortJob()
{
    return Module::SUCCESS;
}
