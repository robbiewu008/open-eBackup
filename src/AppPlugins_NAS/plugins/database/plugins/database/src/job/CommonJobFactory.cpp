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
#include "CommonJobFactory.h"

using namespace GeneralDB;

namespace {
    constexpr auto MODULE = "CommonJobFactory";
    const int NUM_INT_1 = 1;
    const int NUM_INT_100 = 100;
} // namespace

CommonJobFactory::CommonJobFactory()
{
    m_commonJobMap[static_cast<int>(JobType::BACKUP)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonBackupJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonRestoreJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::LIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonLivemountJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::DELCOPY)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonDelCopyJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::CANCELLIVEMOUNT)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonCancelLivemountJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::CHECK_COPY)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonCheckCopyJob>, this, std::placeholders::_1));
    m_commonJobMap[static_cast<int>(JobType::INSTANT_RESTORE)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonInstantRestoreJob>, this, std::placeholders::_1));
#ifndef WIN32
    m_commonJobMap[static_cast<int>(JobType::INDEX)] =
        JobFunc(std::bind(&CommonJobFactory::CreateFactoryJob<CommonCreateIndexJob>, this, std::placeholders::_1));
#endif
}

CommonJobFactory::~CommonJobFactory() {}

std::shared_ptr<BasicJob> CommonJobFactory::CreateJob(const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType)
{
    uint64_t mapKey = static_cast<int>(jobType);
    if (m_commonJobMap.find(mapKey) == m_commonJobMap.end()) {
        HCP_Log(ERR, MODULE) << "no such operation in map, create job failed" << HCPENDLOG;
        return nullptr;
    }
    auto func = m_commonJobMap[mapKey];
    auto jobPtr = func(jobInfo);
    if (jobPtr == nullptr) {
        HCP_Log(ERR, MODULE) << "create job failed" << HCPENDLOG;
        return nullptr;
    }
    return jobPtr;
}

template<typename T>
std::shared_ptr<BasicJob> CommonJobFactory::CreateFactoryJob(std::shared_ptr<JobCommonInfo> jobInfo)
{
    auto job = std::make_shared<T>();
    job->SetJobInfo(jobInfo);
    return job;
}