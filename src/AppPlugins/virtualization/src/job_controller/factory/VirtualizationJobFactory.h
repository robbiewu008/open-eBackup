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
#ifndef __VIRTUALIZATION_JOB_FACTORY_H__
#define __VIRTUALIZATION_JOB_FACTORY_H__

#include <functional>
#include <map>
#include <vector>
#include <thread>
#include <mutex>
#include "job/JobFactoryBase.h"
#include "job_controller/jobs/backup/BackupJob.h"
#include "job_controller/jobs/verify/VerifyJob.h"

namespace VirtPlugin {
class VirtualizationJobFactory : public JobFactoryBase {
public:
    VirtualizationJobFactory(const VirtualizationJobFactory&) = delete;
    VirtualizationJobFactory& operator=(const VirtualizationJobFactory&) = delete;
    static VirtualizationJobFactory* GetInstance();

    std::shared_ptr<BasicJob> CreateJob(
        const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType) override;

    void RemoveFinishJob(const std::string &jobId);

private:
    VirtualizationJobFactory();
    ~VirtualizationJobFactory();

    template<typename T>
    std::shared_ptr<BasicJob> CreateFactoryJob(std::shared_ptr<JobCommonInfo> jobInfo);
    void ReportJobStatus();
    void ReportLog2Agent(const std::string &parentId, const std::string &subJobId,
        const uint64_t &completedDataSize = 0);

    using JobFunc = std::function<std::shared_ptr<BasicJob>(std::shared_ptr<JobCommonInfo>)>;
    std::map<uint64_t, JobFunc> m_commonJobMap {};
    std::vector<std::shared_ptr<BasicJob>> m_vecJobPtr;
    std::mutex m_mutex; // 用于m_vecJobPtr互斥
};
}
#endif // __VIRTUALIZATION_JOB_FACTORY_H__