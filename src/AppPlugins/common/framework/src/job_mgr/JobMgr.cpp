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
#include "JobMgr.h"
#include "log/Log.h"

using namespace common::jobmanager;

namespace {
    constexpr auto MODULE = "JobMgr";
    const int JOB_MGR_MONITOR_INTERVAL = 1;
    const std::string PREREQUISITE = "PRE";
    const std::string GENERATE = "GEN";
    const std::string POST = "PST";
    const std::vector<std::string> JOBPOSTFIX{PREREQUISITE, GENERATE, POST};
}

JobMgr JobMgr::instance;
 
JobMgr& JobMgr::GetInstance()
{
    return instance;
}

JobMgr::JobMgr() : m_monitorInterval(JOB_MGR_MONITOR_INTERVAL)
{}

JobMgr::~JobMgr()
{
    EndJobMonitor();
}

int JobMgr::InsertJob(std::string jobId, std::shared_ptr<BasicJob> jobPtr)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (jobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "Insert job failed, job ptr is null" << HCPENDLOG;
        return Module::FAILED;
    }
    if (CheckJobIdExist(jobId)) {
        HCP_Log(ERR, MODULE) << "Insert job failed, job id exist jobId:" << jobId << HCPENDLOG;
        return Module::FAILED;
    }
    m_jobIdMap.emplace(jobId, jobPtr);
    HCP_Log(INFO, MODULE) << "insert job success, mgr jobId: " << jobId << HCPENDLOG;
    return Module::SUCCESS;
}

int JobMgr::AsyncAbortJob(const std::string& jobId, const std::string& subJobId)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (CheckJobIdExist(subJobId)) {
        m_jobIdMap[subJobId]->SetJobAborted();
        HCP_Log(INFO, MODULE) << "abort job success, jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }

    int count = 0;
    for (auto str : JOBPOSTFIX) {
        std::string mgrJobId = jobId + "_" + str;
        if (CheckJobIdExist(mgrJobId)) {
            m_jobIdMap[mgrJobId]->SetJobAborted();
            HCP_Log(INFO, MODULE) << "abort job success, jobId: " << jobId
                << " mgr JobId: " << mgrJobId << HCPENDLOG;
            count++;
        }
    }

    if (count == 0) {
        HCP_Log(WARN, MODULE) << "no existed job to abort jobId: " << jobId << HCPENDLOG;
        return Module::FAILED;
    }

    HCP_Log(INFO, MODULE) << "abort job success, count: " << count << HCPENDLOG;
    return Module::SUCCESS;
}

int JobMgr::PauseJob(const std::string& jobId, const std::string& subJobId)
{
    std::lock_guard<std::mutex> lock(m_mtx);
    if (CheckJobIdExist(subJobId)) {
        m_jobIdMap[subJobId]->SetJobToPause();
            HCP_Log(INFO, MODULE) << "pause job success, jobId: " << jobId << HCPENDLOG;
        return Module::SUCCESS;
    }

    int count = 0;
    for (auto str : JOBPOSTFIX) {
        std::string mgrJobId = jobId + "_" + str;
        if (CheckJobIdExist(mgrJobId)) {
            m_jobIdMap[mgrJobId]->SetJobToPause();
            HCP_Log(INFO, MODULE) << "pause job success, jobId: " << jobId
                << " mgr JobId: " << mgrJobId << HCPENDLOG;
            count++;
        }
    }

    if (count == 0) {
        HCP_Log(WARN, MODULE) << "no existed job to pause jobId: " << jobId << HCPENDLOG;
        return Module::FAILED;
    }

    HCP_Log(INFO, MODULE) << "pause job success, count: " << count << HCPENDLOG;
    return Module::SUCCESS;
}

void JobMgr::PauseAllJob()
{
    INFOLOG("Enter PauseAllJob");
    std::lock_guard<std::mutex> lock(m_mtx);
    for (auto& pair : m_jobIdMap) {
        pair.second->SetJobToPause();
    }
}

void JobMgr::EraseFinishJob()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    for (auto it = m_jobIdMap.begin(); it != m_jobIdMap.end();) {
        if (it->second->IsJobFinish()) {
            HCP_Log(INFO, MODULE) << "erase finish job, mgr jobid: " << it->first << HCPENDLOG;
            it = m_jobIdMap.erase(it);
        } else {
            it++;
        }
    }
}

bool JobMgr::CheckJobIdExist(const std::string &jobId)
{
    return m_jobIdMap.count(jobId) != 0;
}

void JobMgr::EndJobMonitor()
{
    m_isMonitoring = false;
    if (m_monitorJobMapThread && m_monitorJobMapThread->joinable()) {
        m_monitorJobMapThread->join();
        m_monitorJobMapThread.reset();
    }
}

int JobMgr::StartMonitorJob()
{
    HCP_Log(INFO, MODULE) << "start job monitor" << HCPENDLOG;
    m_monitorJobMapThread = std::make_unique<std::thread>(std::thread([&] {
        while (m_isMonitoring) {
            std::this_thread::sleep_for(std::chrono::seconds(m_monitorInterval));
            EraseFinishJob();
        }
    }));
    if (m_monitorJobMapThread == nullptr) {
        ERRLOG("Start job monitor thread failded!");
        return Module::FAILED;
    }
    return Module::SUCCESS;
}

void JobMgr::SetMonitorInterval(int i)
{
    m_monitorInterval = i;
}
