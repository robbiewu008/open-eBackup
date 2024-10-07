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
#ifndef COMMON_DB_JOB_H
#define COMMON_DB_JOB_H

#include "BasicJob.h"
#include "ParseConfigFile.h"
#include "LocalCmdExector.h"
#include "client/ClientInvoke.h"

namespace GeneralDB {
struct SplitLogic {
    mp_string name;
    uint32_t runType {0};
    uint32_t policy {0};
    uint32_t num {0};
    uint32_t limit {0};
    bool ignoreFailed {false};
    uint32_t priority {0};

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(name, name)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(runType, runType)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(policy, policy)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(num, num)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(limit, limit)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(ignoreFailed, ignoreFailed)
    SERIAL_MEMBER_TO_SPECIFIED_NAME(priority, priority)
    END_SERIAL_MEMEBER
};

struct SubJobInfo {
    uint32_t policy {0};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(policy, executePolicy)
    END_SERIAL_MEMEBER
};

enum class RunType {
    GEN_ANYNODE_RUN_TASK = 0,
    GEN_LOCAL_RUN_TASK,
    GEN_FIXEDNODE_RUN_TASK
};

enum class Policy {
    PRIOR_NOUSE = 0,
    PRIOR_MASTER,
    PRIOR_SLAVER
};

class CommonDBJob : public BasicJob {
public:
    CommonDBJob();
    ~CommonDBJob() {}
    uint64_t m_appType { 0 };
protected:
    int GenSubJob(mp_string typeHandle, const mp_string &appType, Json::Value& extendInfo,
        const std::vector<ApplicationEnvironment>& agentList);
    int GetSplitLogic(const mp_string& appType, Json::Value& extendInfo, Json::Value& clusterHandle,
        const Json::Value& handleMap);
    int SetPriorityByPolicy(const SplitLogic& mem, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob> &subVec);
    int SetPolicyByRunType(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub);
    int GetJobFromResultFile(std::vector<SubJob>& tasks);
    int GetJobFromConfigureFile(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob>& tasks);
    int GenerateSubJobInner(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList);
    void ReportJobFailed(const mp_string &jobId, const mp_string &subJobId);
    virtual int GenerateSubJobManually();
protected:
    bool m_isManual { false };
    Json::Value m_manualResult;
    std::vector<SplitLogic> m_splitLogics;
    std::map<mp_string, mp_string> m_clusterTypeConversion {{"1", "SINGLE"}, {"2", "AA"},
        {"3", "AP"}, {"4", "SHARDING"}, {"5", "DISTRIBUTED"}};

    using GenFunc = std::function<int(Json::Value&, const std::vector<ApplicationEnvironment>&, std::vector<SubJob>&,
        const SplitLogic&, SubJob&)>;
    std::map<uint32_t, GenFunc> m_genJobMap {};
private:
    int GenAnyNodeTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub);
    int GenLocalTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub);
    int GenFixNodeTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub);
    int SetPriority(const SplitLogic& mem, const std::vector<ApplicationEnvironment>& agentList,
        std::vector<SubJob> &subVec, bool isMasterPrior = true);
    mp_string SetSubJobInfo(SubJob& sub, uint32_t policy);
};
}
#endif // COMMON_DB_JOB_H