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
#ifndef COMMON_JOB_FACTORY_H
#define COMMON_JOB_FACTORY_H

#include <functional>
#include <map>
#include "JobFactoryBase.h"
#include "CommonBackupJob.h"
#include "CommonRestoreJob.h"
#include "CommonLivemountJob.h"
#include "CommonDelCopyJob.h"
#include "CommonCheckCopyJob.h"
#include "CommonCancelLivemountJob.h"
#include "CommonInstantRestoreJob.h"
#include "CommonCreateIndexJob.h"
#include "common/PluginTypes.h"

namespace GeneralDB {
class CommonJobFactory : public JobFactoryBase {
public:
    CommonJobFactory(const CommonJobFactory&) = delete;
    CommonJobFactory& operator=(const CommonJobFactory&) = delete;
    static CommonJobFactory* GetInstance()
    {
        static CommonJobFactory instance;
        return &instance;
    }

    std::shared_ptr<BasicJob> CreateJob(const std::shared_ptr<JobCommonInfo>& jobInfo, JobType jobType) override;

private:
    CommonJobFactory();
    ~CommonJobFactory();
    template<typename T>
    std::shared_ptr<BasicJob> CreateFactoryJob(std::shared_ptr<JobCommonInfo> jobInfo);

    using JobFunc = std::function<std::shared_ptr<BasicJob>(std::shared_ptr<JobCommonInfo>)>;
    std::map<uint64_t, JobFunc> m_commonJobMap {};
};
}
#endif // COMMON_JOB_FACTORY_H