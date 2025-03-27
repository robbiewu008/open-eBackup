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
#include "VirtualizationJobFactory.h"
#include "client/ClientInvoke.h"
#include "job_controller/jobs/restore/RestoreJob.h"
#include "job_controller/jobs/restore/ArchiveRestoreJob.h"
#ifndef WIN32
#include "job_controller/jobs/livemount/LivemountJob.h"
#include "job_controller/jobs/livemount/CancelLivemountJob.h"
#include "job_controller/jobs/livemount/InstantRestoreJob.h"
#include "job_controller/jobs/delcopy/DelCopyJob.h"
#endif

namespace {
constexpr auto MODULE_NAME = "VirtualizationJobFactory";
const int32_t REPORT_JOB_TIMEOUT = 60;  // 单位:s
constexpr uint32_t KB_SIZE = 1024;
const int32_t NUM_2 = 2;
const int32_t NUM_3 = 3;
const int32_t NUM_100 = 100;
}

static std::once_flag g_jobReportThread;

namespace VirtPlugin {
VirtualizationJobFactory* VirtualizationJobFactory::GetInstance()
{
    static VirtualizationJobFactory instance;
    std::call_once(g_jobReportThread, [&]() {
        instance.InitReportThread();
        std::thread thread(&VirtualizationJobFactory::ReportJobStatus, &instance);
        thread.detach();
    });

    return &instance;
}

VirtualizationJobFactory::VirtualizationJobFactory()
{
    m_commonJobMap[static_cast<int>(JobType::BACKUP)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::BackupJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::RESTORE)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::RestoreJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::ARCHIVE_RESTORE)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::ArchiveRestoreJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::CHECK_COPY)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::VerifyJob>,
            this, std::placeholders::_1));
#ifndef WIN32
    m_commonJobMap[static_cast<int>(JobType::LIVEMOUNT)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::LivemountJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::CANCELLIVEMOUNT)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::CancelLivemountJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::INSTANT_RESTORE)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::InstantRestoreJob>,
            this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::DELCOPY)] =
        JobFunc(std::bind(&VirtualizationJobFactory::CreateFactoryJob<VirtPlugin::DelCopyJob>,
            this, std::placeholders::_1));
#endif
}

VirtualizationJobFactory::~VirtualizationJobFactory()
{
    INFOLOG("VirtualizationJ job factory destructor.");
    m_exitFlag = true;
    for (int i = 0; i < m_workerNum; ++i) {
        JobReportRequest request;
        AddJobReportRequest(request);   // wakeup all worker thread to exit.
    }

    for (std::shared_ptr<std::thread> worker : m_workers) {
        if (worker) {
            worker->join();
        }
    }
    INFOLOG("VirtualizationJ job factory destructor end.");
}

std::shared_ptr<BasicJob> VirtualizationJobFactory::CreateJob(
    const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType)
{
    if (jobInfo == nullptr) {
        ERRLOG("No jobInfo provided, null pointer.");
        return nullptr;
    }
    uint64_t mapKey = static_cast<int>(jobType);
    DBGLOG("Virtualization plugin create job, jobType: %llu", mapKey);
    if (m_commonJobMap.find(mapKey) == m_commonJobMap.end()) {
        ERRLOG("No such operation in map, create job failed");
        return nullptr;
    }
    if (jobType == JobType::RESTORE) {
        auto restorePara = std::dynamic_pointer_cast<AppProtect::RestoreJob>(jobInfo->GetJobInfo());
        // 从repo中拿到protocol的值 s3恢复:2
        for (const auto& copy : restorePara->copies) {
            for (const auto& repo : copy.repositories) {
                mapKey = (repo.protocol == RepositoryProtocolType::type::S3) ?
                    static_cast<int>(JobType::ARCHIVE_RESTORE) : static_cast<int>(jobType);
            }
        }
    }
    auto func = m_commonJobMap[mapKey];
    auto jobPtr = func(jobInfo);
    if (jobPtr == nullptr) {
        ERRLOG("create job failed");
        return nullptr;
    }
    // 将正在运行任务对象添加到vec中，用于定时上报状态
    std::lock_guard<std::mutex> lock(m_mutex);
    m_vecJobPtr.push_back(jobPtr);

    return jobPtr;
}

template<typename T>
std::shared_ptr<BasicJob> VirtualizationJobFactory::CreateFactoryJob(std::shared_ptr<JobCommonInfo> jobInfo)
{
    auto job = std::make_shared<T>();
    job->SetJobInfo(jobInfo);
    return job;
}

void VirtualizationJobFactory::InitReportThread()
{
    m_workerNum = Module::ConfigReader::getUint("General", "ReportThreadNum");
    m_queueSize = Module::ConfigReader::getUint("General", "ReportQueueSize");
    for (uint32_t i = 0; i < m_workerNum; ++i) {
        std::shared_ptr<std::thread> woker = std::make_shared<std::thread>(
            std::bind(&VirtualizationJobFactory::CheckThreadReport, this));
        if (woker == nullptr) {
            ERRLOG("Create CheckThreadReport failed.");
            return;
        }
        m_workers.push_back(woker);
    }
}

void VirtualizationJobFactory::AddJobReportRequest(const JobReportRequest& request)
{
    // producer
    std::unique_lock<std::mutex> lock(m_requestMtx);
    m_producerCond.wait(lock, [this] { return (m_requestQueue.size() < m_queueSize); });
    m_requestQueue.push_back(request);

    m_consumerCond.notify_one();
}

JobReportRequest VirtualizationJobFactory::GetJobReportRequest()
{
    // consumer
    std::unique_lock<std::mutex> lock(m_requestMtx);

    m_consumerCond.wait(lock, [this] { return !m_requestQueue.empty(); });

    JobReportRequest request = m_requestQueue.front();
    m_requestQueue.pop_front();

    m_producerCond.notify_one();
    return request;
}

void VirtualizationJobFactory::CheckThreadReport()
{
    INFOLOG("Enter CheckThreadReport.");
    while (!m_exitFlag) {
        JobReportRequest request = GetJobReportRequest();
        ReportLog2Agent(request.parentId, request.subJobId, request.completedDataSize, request.totalSize);
    }
    INFOLOG("Exit CheckThreadReport.");
}

void VirtualizationJobFactory::ReportJobStatus()
{
    while (true) {
        std::vector<std::tuple<std::string, std::string, uint64_t, uint64_t>> jobVec;
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            for (const auto &jobPtr : m_vecJobPtr) {
                std::shared_ptr<VirtualizationBasicJob> virJobPtr =
                    dynamic_pointer_cast<VirtualizationBasicJob>(jobPtr);
                jobVec.push_back(std::make_tuple(virJobPtr->GetParentJobId(), virJobPtr->GetSubJobId(),
                    virJobPtr->m_completedDataSize, virJobPtr->m_totalVolumeSize));
            }
        }
        std::string parentId;
        std::string subJobId;
        uint64_t completedDataSize;
        uint64_t totalVolumeSize;
        for (const auto &job : jobVec) {
            parentId = std::get<0>(job);
            subJobId = std::get<1>(job);
            completedDataSize = std::get<NUM_2>(job);
            totalVolumeSize = std::get<NUM_3>(job);
            INFOLOG("Begin report job status. parentId:%s, subjobId:%s, completed data size %llu, total size %llu",
                parentId.c_str(), subJobId.c_str(), completedDataSize / KB_SIZE, totalVolumeSize);
            if (subJobId.empty() || parentId.empty()) {
                continue;
            }
            JobReportRequest request = {parentId, subJobId, completedDataSize / KB_SIZE, totalVolumeSize};
            AddJobReportRequest(request);
        }

        jobVec.clear();

        std::this_thread::sleep_for(std::chrono::seconds(REPORT_JOB_TIMEOUT));
    }
}

void VirtualizationJobFactory::ReportLog2Agent(const std::string &parentId, const std::string &subJobId,
    const uint64_t &completedDataSize, const uint64_t &totalSize)
{
    SubJobDetails subJobDetails;
    std::vector<LogDetail> logDetailList;
    ActionResult result;
    LogDetail logDetail {};
    uint32_t progress = 0;
    if (totalSize != 0 && completedDataSize * KB_SIZE <= totalSize) {
        progress = completedDataSize * KB_SIZE * NUM_100 / totalSize;
    }
    VirtPlugin::ReportLog2AgentParam param = {
        subJobDetails, result, logDetailList, logDetail, progress, completedDataSize, SubJobStatus::RUNNING
    };

    param.subJobDetails.__set_jobId(parentId);
    if (subJobId != "") {
        param.subJobDetails.__set_subJobId(subJobId);
    }
    param.subJobDetails.__set_jobStatus(param.curJobstatus);
    param.subJobDetails.__set_logDetail(param.logDetailList);
    param.subJobDetails.__set_dataSize(completedDataSize);
    
    DBGLOG("report job=[%s], subJobId=[%s], jobStatus=[%d].", param.subJobDetails.jobId.c_str(),
        param.subJobDetails.subJobId.c_str(), param.curJobstatus);

    JobService::ReportJobDetails(param.returnValue, param.subJobDetails);
    param.logDetailList.clear();
    param.logDetail.__set_description("");
}

void VirtualizationJobFactory::RemoveFinishJob(const std::string &jobId)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    auto jobPtr = m_vecJobPtr.begin();
    for (; jobPtr != m_vecJobPtr.end();) {
        if ((*jobPtr)->GetJobId() == jobId || (*jobPtr)->GetParentJobId() == jobId) {
            INFOLOG("Begin remove job vec. jobId: %s, subJobId: %s.",
                (*jobPtr)->GetParentJobId().c_str(), (*jobPtr)->GetJobId().c_str());
            jobPtr = m_vecJobPtr.erase(jobPtr);
            continue;
        }
        jobPtr++;
    }
}
}