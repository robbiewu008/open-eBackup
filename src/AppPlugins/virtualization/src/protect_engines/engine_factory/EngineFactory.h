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
#ifndef ENGINE_FACTORY_H
#define ENGINE_FACTORY_H

#include <memory>
#include <job/JobCommonInfo.h>

#include <common/Macros.h>
#include <common/Structs.h>
#include <protect_engines/ProtectEngine.h>

namespace VirtPlugin {
using PipelinePtr = std::function<std::shared_ptr<ProtectEngine>(std::shared_ptr<JobHandle>, std::string, std::string)>;
using EngineFactoryPipeline = std::unordered_map<std::string, PipelinePtr>;

class EngineFactory {
public:
    /**
     *  @brief 根据生产环境类型构造对应的保护引擎
     *
     *  @param jobType [IN]任务类型，指示用于根据类型从实际的JobCommonInfo解析出所需的任务信息
     *  @param jobInfo [IN]任务信息
     *  @return ProtectEngine 对应类型的保护引擎
     */
    static std::shared_ptr<ProtectEngine> CreateProtectEngine(const JobType &jobType,
        std::shared_ptr<JobCommonInfo> jobInfo, std::string jobId, std::string subJobId);

    static std::shared_ptr<ProtectEngine> CreateProtectEngineWithoutTask(const std::string appType);

private:
#ifndef WIN32
    static std::shared_ptr<ProtectEngine> CreateK8SEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
    static std::shared_ptr<ProtectEngine> CreateHCSEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
    static std::shared_ptr<ProtectEngine> CreateOpenStackEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
    static std::shared_ptr<ProtectEngine> CreateApsaraStackEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
    static std::shared_ptr<ProtectEngine> CreateCNwareEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
#else
    static std::shared_ptr<ProtectEngine> CreateHyperVEngine(std::shared_ptr<JobHandle> jobHandle,
        std::string jobId, std::string subJobId);
#endif
private:
    static EngineFactoryPipeline m_enginePipeline;
};
}

#endif  // _ENGINE_FACTORY_H_
