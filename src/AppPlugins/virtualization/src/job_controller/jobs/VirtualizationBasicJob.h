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
#ifndef VIRTUALIZATION_BASIC_JOB
#define VIRTUALIZATION_BASIC_JOB

#include <job/BasicJob.h>
#include <common/Macros.h>
#include <common/Structs.h>
#include <common/utils/Utils.h>
#include <protect_engines/ProtectEngine.h>
#include <volume_handlers/VolumeHandler.h>

namespace VirtPlugin {

class VirtualizationBasicJob : public BasicJob {
public:
    VirtualizationBasicJob() = default;
    virtual ~VirtualizationBasicJob() = default;

    template<typename... Args>
    void ReportJobDetailsWithLabel(const ReportJobDetailsParam &jobParam, Args... logArgs);
    void ReportJobDetailWithLabel(const ReportJobDetailsParam &jobParam, const std::vector<std::string> &args);
    void ReportJobDetailWithErrorParams();
    void ReportJobResult(int ret, const std::string &msg, uint64_t dataSizeInByte = 0,
        const ReportJobResultPara &reportPara = ReportJobResultPara());
    void ReportApplicationLabels(const ApplicationLabelType &label);
    void ReportJobSpeed(const uint64_t &dataSizeInByte);
    void ReportTaskLabel(const std::vector<std::string> &reportArgs = std::vector<std::string>());

    bool DoInitHandlers(const AppProtect::StorageRepository &storageRepo,
                        std::shared_ptr<RepositoryHandler> &repoHandler, std::string &repoPath);
    bool GetRepoPath(const std::shared_ptr<RepositoryHandler> &repoHandler, const StorageRepository &repo,
                     std::string &repoPath);
    void SendAlarm(const std::string &alarmId, const std::string &alarmPara, const std::string &resourceId);
    bool AddWhiteList(const std::string &ipListStr);

public:
    std::string m_taskInfo;
    uint64_t m_completedDataSize = 0;
    uint64_t m_totalVolumeSize = 0;

protected:
    int ExecHook(const ExecHookParam &para);
    int InitProtectEngineHandler(JobType jobType);
    void ReportLog2Agent(ReportLog2AgentParam &param);
    void HandleAbortPoint();
    int AbortJob();
    uint64_t GetSegementSizeFromConf();
    bool AddNewJobWithRetry(const std::vector<SubJob> &subJobs);
    void VolHandlerReportTaskLabel(const std::shared_ptr<VolumeHandler> &volHandler,
        const std::vector<std::string> &reportArgs = std::vector<std::string>());
    std::string FindStartWithSourcePolicy(const std::string& fullString);
    bool CheckPathExixts(const std::shared_ptr<RepositoryHandler> &repoHandler,
        const std::string &path, const std::string &e1000RepeatedPath, std::string &repoPath);

protected:
    std::shared_ptr<ProtectEngine> m_protectEngine = nullptr;
    std::vector<std::function<int32_t(void)>> m_cleanHandlesForStop{};
};

class LoadBalancer {
public:
    static LoadBalancer* GetInstance();
    ~LoadBalancer();

    bool GetNodePath(const std::vector<std::string> &repoPaths, std::string &dataPath);
    void RemoveNodePath(const std::string &dataPath);

private:
    LoadBalancer();
    LoadBalancer(const LoadBalancer &) = delete;
    LoadBalancer& operator=(const LoadBalancer &) = delete;
    LoadBalancer(LoadBalancer &&) = delete;
    LoadBalancer& operator=(LoadBalancer &&) = delete;

    std::unordered_map<std::string, int32_t> m_totalDataNode;
    std::mutex m_mapMutex;
};

}

#endif