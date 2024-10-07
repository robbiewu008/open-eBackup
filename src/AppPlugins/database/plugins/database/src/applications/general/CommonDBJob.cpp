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
#include "CommonDBJob.h"
#include "CTime.h"
#include "trjsontostruct.h"
#include "ReportJobDetailHandler.h"
#include "Utils.h"

using namespace Module;
using namespace GeneralDB;

namespace {
    const mp_string MOUDLE_DB = "CommonDBJob";
    const mp_string CLUSTER_HANDLE = "ClusterHandle";
    const mp_int32 CAN_SWITCH = 1;
    const mp_uint32 SEND_ADDNEWJOB_DELAY_TIME = 10 * 1000;  // 10s delay
    const mp_uint32 ADDNEWJOB_TIMEOUT = 5 * 60;  // 5min
    using defer = std::shared_ptr<void>;
}

CommonDBJob::CommonDBJob()
{
    // 在任意节点执行固定数量任务
    m_genJobMap[static_cast<uint32_t>(RunType::GEN_ANYNODE_RUN_TASK)] = GenFunc(std::bind(&CommonDBJob::GenAnyNodeTask,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
        std::placeholders::_5));
    // 在执行主任务的节点执行子任务
    m_genJobMap[static_cast<uint32_t>(RunType::GEN_LOCAL_RUN_TASK)] = GenFunc(std::bind(&CommonDBJob::GenLocalTask,
        this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4,
        std::placeholders::_5));
    // 每个节点执行一个任务
    m_genJobMap[static_cast<uint32_t>(RunType::GEN_FIXEDNODE_RUN_TASK)] = GenFunc(
        std::bind(&CommonDBJob::GenFixNodeTask, this, std::placeholders::_1, std::placeholders::_2,
        std::placeholders::_3, std::placeholders::_4, std::placeholders::_5));
}

int CommonDBJob::GenSubJob(mp_string typeHandle, const mp_string &appType, Json::Value& extendInfo,
    const std::vector<ApplicationEnvironment>& agentList)
{
    defer _(nullptr, [&](...) {
        SetJobToFinish();
    });
    HCP_Log(INFO, MOUDLE_DB) << "Begin to generate sub job.jobId = " << m_jobId << HCPENDLOG;
    Json::Value conf;
    mp_string subJobId;
    if (ParseConfigFile::GetInstance()->GetGenJobConfHandle(conf) != MP_SUCCESS) {
        HCP_Log(ERR, MOUDLE_DB) << "GetGenJobConfHandle Failed!jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    if (!conf.isMember(CLUSTER_HANDLE) || !conf.isMember(typeHandle)) {
        HCP_Log(ERR, MOUDLE_DB) << "Configuration file have no " << CLUSTER_HANDLE <<
            " or " << typeHandle << " jobId = "<< m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    Json::Value clusterHandle = conf[CLUSTER_HANDLE];
    Json::Value handleMap = conf[typeHandle];
    if (!clusterHandle.isObject() || !handleMap.isObject()) {
        ERRLOG("ClusterHandle and handleMap is not json object, jobId=%s.", m_jobId.c_str());
        return MP_FAILED;
    }

    if (GetSplitLogic(appType, extendInfo, clusterHandle, handleMap) == MP_FAILED) {
        HCP_Log(ERR, MOUDLE_DB) << "GenerateSubJob failed for get splitLogic failed.jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    if (GenerateSubJobInner(extendInfo, agentList) != MP_SUCCESS) {
        HCP_Log(ERR, MOUDLE_DB) << "GenerateSubJobInner failed.jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    HCP_Log(INFO, MOUDLE_DB) << "Finish to generate sub job.jobId = " << m_jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonDBJob::GenerateSubJobManually()
{
    return MP_SUCCESS;
}

int CommonDBJob::GetSplitLogic(const mp_string& appType, Json::Value& extendInfo, Json::Value& clusterHandle,
    const Json::Value& handleMap)
{
    mp_string deployType = extendInfo["deployType"].asString();
    if (m_clusterTypeConversion.find(deployType) == m_clusterTypeConversion.end()) {
        HCP_Log(ERR, MOUDLE_DB) << "No such deployType in map." << DBG(deployType) << " jobId = "
            << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    mp_string clusterType = m_clusterTypeConversion[deployType];
    mp_string splitLogic;
    if (handleMap.isMember(appType) && handleMap[appType].isMember(clusterType) &&
        !handleMap[appType][clusterType].isNull() && handleMap[appType][clusterType].isString()) {
        splitLogic = handleMap[appType][clusterType].asString();
    } else {
        HCP_Log(ERR, MOUDLE_DB) << "Get splitLogic failed!jobId = " << m_jobId << ", " << DBG(appType)
            << ", " << DBG(deployType) << HCPENDLOG;
        return MP_FAILED;
    }
    HCP_Log(DEBUG, MOUDLE_DB) << DBG(splitLogic) << HCPENDLOG;
    if (splitLogic.find("manual_handle") != std::string::npos) {
        m_isManual = true;
        return GenerateSubJobManually();
    }
    if (clusterHandle.isMember(splitLogic) && !clusterHandle[splitLogic].isNull() &&
        clusterHandle[splitLogic].isArray()) {
        for (auto &mem : clusterHandle[splitLogic]) {
            SplitLogic temp;
            if (!Module::JsonHelper::JsonValueToStruct(mem, temp)) {
                HCP_Log(ERR, MOUDLE_DB) << "Parse clusterHandle splitLogic from conf failed!jobId = " <<
                    m_jobId << HCPENDLOG;
                return MP_FAILED;
            }
            HCP_Log(DEBUG, MOUDLE_DB) << "Cluster mem.name = " << temp.name << " mem.runType = " << temp.runType
                << " mem.policy = " << temp.policy << " mem.priority = " << temp.priority << HCPENDLOG;
            m_splitLogics.push_back(temp);
        }
    } else {
        HCP_Log(ERR, MOUDLE_DB) << "GenerateSubJob failed for get m_splitLogics failed.jobId = "
            << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

int CommonDBJob::SetPriority(const SplitLogic& mem, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob> &subVec, bool isMasterPrior)
{
    auto sub = subVec.begin();
    auto agent = agentList.begin();
    for (; sub != subVec.end() && agent != agentList.end(); sub++, agent++) {
        Json::Value extendInfo;
        Module::JsonHelper::JsonStringToJsonValue(agent->extendInfo, extendInfo);
        if (!extendInfo.isMember("role")) {
            HCP_Log(ERR, MOUDLE_DB) << "Extendinfo has not role member." << HCPENDLOG;
            return MP_FAILED;
        }
        std::string roleStr = extendInfo["role"].asString();
        HCP_Log(DEBUG, MOUDLE_DB) << "Role str." << roleStr << DBG(sub->execNodeId) << HCPENDLOG;
        int role = SafeStoi(roleStr);
        int activePrior = isMasterPrior ? mem.priority : mem.priority + 1;
        int standbyPrior = isMasterPrior ? mem.priority + 1 : mem.priority;
        if (role == static_cast<int>(NodeRole::ACTIVE)) {
            sub->__set_jobPriority(activePrior);
        } else if (role == static_cast<int>(NodeRole::STANDBY)) {
            sub->__set_jobPriority(standbyPrior);
        }
    }
    return MP_SUCCESS;
}

int CommonDBJob::SetPriorityByPolicy(const SplitLogic& mem, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob> &subVec)
{
    if (mem.policy > static_cast<int>(Policy::PRIOR_SLAVER) || mem.policy < 0) {
        HCP_Log(ERR, MOUDLE_DB) << "Policy is out of range.jobId = " << m_jobId
            << " mem.policy = " << mem.policy << HCPENDLOG;
        return MP_FAILED;
    }
    HCP_Log(DEBUG, MOUDLE_DB) << "Set priority = " << mem.priority << " mem.policy = " << mem.policy << HCPENDLOG;
    switch (mem.policy) {
        case static_cast<int>(Policy::PRIOR_NOUSE): {
            for (auto &sub : subVec) {
                sub.__set_jobPriority(mem.priority);
            }
            break;
        }
        case static_cast<int>(Policy::PRIOR_MASTER): {
            SetPriority(mem, agentList, subVec);
            break;
        }
        case static_cast<int>(Policy::PRIOR_SLAVER): {
            SetPriority(mem, agentList, subVec, false);
            break;
        }
        default: break;
    }

    return MP_SUCCESS;
}

int CommonDBJob::SetPolicyByRunType(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub)
{
    if (m_genJobMap.find(mem.runType) == m_genJobMap.end()) {
        HCP_Log(ERR, MOUDLE_DB) << DBG(mem.runType) << ", no such split operation in map, split sub job failed!"
            << HCPENDLOG;
        return MP_FAILED;
    }
    auto genFunc = m_genJobMap[mem.runType];
    if (genFunc(extendInfo, agentList, tasks, mem, sub) != MP_SUCCESS) {
        HCP_Log(ERR, MOUDLE_DB) << "create job failed" << HCPENDLOG;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_string CommonDBJob::SetSubJobInfo(SubJob& sub, uint32_t policy)
{
    SubJobInfo subJobInfo;
    subJobInfo.policy = policy;
    mp_string tempStr;
    if (!Module::JsonHelper::StructToJsonString(subJobInfo, tempStr)) {
        HCP_Log(ERR, MOUDLE_DB) << "Struct to json string failed.jobId =" << m_jobId << HCPENDLOG;
        return "";
    }
    sub.__set_jobInfo(tempStr);
    return tempStr;
}

int CommonDBJob::GenAnyNodeTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub)
{
    if (mem.num == 0) {
        HCP_Log(ERR, MOUDLE_DB) << "Gen sub num can not be zero!jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    uint32_t policy = 0;
    if (mem.limit == CAN_SWITCH && extendInfo.isMember("slave_node_first") && agentList.size() > 1) {
        sub.__set_policy(ExecutePolicy::RETRY_OTHER_NODE_WHEN_FAILED);
        policy = static_cast<int>(ExecutePolicy::RETRY_OTHER_NODE_WHEN_FAILED);
    } else {
        sub.__set_policy(ExecutePolicy::ANY_NODE);
        policy = static_cast<int>(ExecutePolicy::ANY_NODE);
    }
    SetSubJobInfo(sub, policy);
    if (mem.num > agentList.size()) {
        HCP_Log(ERR, MOUDLE_DB) << "Split task num bigger than nodes size.jobId = "
            << m_jobId << ", " << DBG(agentList.size()) << HCPENDLOG;
        return MP_FAILED;
    }
    std::vector<SubJob> subVec(mem.num, sub);
    if (SetPriorityByPolicy(mem, agentList, subVec) != MP_SUCCESS) {
        return MP_FAILED;
    }
    HCP_Log(DEBUG, MOUDLE_DB) << "Add sub." << DBG(mem.runType) << ", jobId = " << m_jobId
        << ", " << DBG(subVec.size()) << HCPENDLOG;
    tasks.insert(tasks.end(), subVec.begin(), subVec.end());
    return MP_SUCCESS;
}

int CommonDBJob::GenLocalTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub)
{
    if (mem.num == 0) {
        HCP_Log(ERR, MOUDLE_DB) << "Gen sub num can not be zero! jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    sub.__set_policy(ExecutePolicy::LOCAL_NODE);
    SetSubJobInfo(sub, static_cast<int>(ExecutePolicy::LOCAL_NODE));
    if (mem.num > agentList.size()) {
        HCP_Log(ERR, MOUDLE_DB) << "Split task num bigger than nodes size.jobId = "
            << m_jobId << ", " << DBG(agentList.size()) << HCPENDLOG;
        return MP_FAILED;
    }
    std::vector<SubJob> subVec(mem.num, sub);
    HCP_Log(DEBUG, MOUDLE_DB) << "Add sub." << DBG(mem.runType) << ", jobId = " << m_jobId
        << ", " << DBG(subVec.size())<< HCPENDLOG;
    tasks.insert(tasks.end(), subVec.begin(), subVec.end());
    return MP_SUCCESS;
}

int CommonDBJob::GenFixNodeTask(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob>& tasks, const SplitLogic& mem, SubJob& sub)
{
    sub.__set_policy(ExecutePolicy::FIXED_NODE);
    SetSubJobInfo(sub, static_cast<int>(ExecutePolicy::FIXED_NODE));
    std::vector<SubJob> subVec(agentList.size(), sub);
    auto it = subVec.begin();
    auto agent = agentList.begin();
    for (; it != subVec.end() && agent != agentList.end(); it++, agent++) {
        HCP_Log(DEBUG, MOUDLE_DB) << DBG(agent->id) << HCPENDLOG;
        it->__set_execNodeId(agent->id);
    }
    if (SetPriorityByPolicy(mem, agentList, subVec) != MP_SUCCESS) {
        return MP_FAILED;
    }
    HCP_Log(DEBUG, MOUDLE_DB) << "Add sub." << DBG(mem.runType) << ", jobId = " << m_jobId
        << ", " << DBG(subVec.size()) << HCPENDLOG;
    tasks.insert(tasks.end(), subVec.begin(), subVec.end());
    return MP_SUCCESS;
}

int CommonDBJob::GetJobFromResultFile(std::vector<SubJob>& tasks)
{
    if (m_manualResult.isNull() || !m_manualResult.isArray()) {
        HCP_Log(ERR, MOUDLE_DB) << "Result is empty or not array!jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    for (auto &result : m_manualResult) {
        SubJob sub;
        JsonToStruct(result, sub);
        tasks.emplace_back(sub);
    }
    HCP_Log(INFO, MOUDLE_DB) << "Get job from result file success!jobId = " << m_jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonDBJob::GetJobFromConfigureFile(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList,
    std::vector<SubJob>& tasks)
{
    for (auto &mem : m_splitLogics) {
        SubJob sub;
        sub.__set_jobId(m_jobId);
        sub.__set_jobType(SubJobType::BUSINESS_SUB_JOB);
        sub.__set_jobName(mem.name); // 任务类型放在jobname，透传conf name，由应用自行分析执行
        sub.__set_ignoreFailed(mem.ignoreFailed);
        sub.__set_jobPriority(mem.priority);
        HCP_Log(DEBUG, MOUDLE_DB) << DBG(sub.ignoreFailed) << HCPENDLOG;
        if (SetPolicyByRunType(extendInfo, agentList, tasks, mem, sub) != MP_SUCCESS) {
            HCP_Log(ERR, MOUDLE_DB) << "SetPolicyByRunType Failed!jobId = " << m_jobId << HCPENDLOG;
            return MP_FAILED;
        }
    }
    HCP_Log(INFO, MOUDLE_DB) << "Get job from configure file success!jobId = " << m_jobId << HCPENDLOG;
    return MP_SUCCESS;
}

int CommonDBJob::GenerateSubJobInner(Json::Value& extendInfo, const std::vector<ApplicationEnvironment>& agentList)
{
    std::vector<SubJob> tasks;
    if (!m_isManual) {
        if (GetJobFromConfigureFile(extendInfo, agentList, tasks) != MP_SUCCESS) {
            return MP_FAILED;
        }
    } else {
        if (GetJobFromResultFile(tasks) != MP_SUCCESS) {
            return MP_FAILED;
        }
    }
    if (tasks.size() == 0) {
        HCP_Log(ERR, MOUDLE_DB) << "Get tasks from conf emptly.jobId = " << m_jobId << HCPENDLOG;
        return MP_FAILED;
    }
    for (auto &task : tasks) {
        HCP_Log(DEBUG, MOUDLE_DB) << DBG(task.jobId) << DBG(task.subJobId) << DBG(task.execNodeId) << HCPENDLOG;
    }
    time_t startTimeStamp;
    CTime::Now(startTimeStamp);
    while (true) {
        ActionResult result;
        JobService::AddNewJob(result, tasks);
        if (result.code == MP_SUCCESS) {
            INFOLOG("AddNewJob success, jobId=%s.", m_jobId.c_str());
            break;
        }
        time_t nowTimeStamp;
        CTime::Now(nowTimeStamp);
        if (nowTimeStamp - startTimeStamp > ADDNEWJOB_TIMEOUT) {
            ERRLOG("AddNewJob timeout 5 min, jobId=%s.", m_jobId.c_str());
            return MP_FAILED;
        }
        // sleep 10s
        CTime::DoSleep(SEND_ADDNEWJOB_DELAY_TIME);
        // 重试阶段上报任务状态为Running
        JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::RUNNING, LOG_LABEL_TYPE::UNDEFIND_LABEL};
        ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
        WARNLOG("AddNewJob failed, will try again, jobId=%s.", m_jobId.c_str());
    }
    JobLogDetail jobLogDetail = {m_jobId, "", SubJobStatus::COMPLETED, LOG_LABEL_TYPE::UNDEFIND_LABEL};
    ReportJobDetailHandler::GetInstance()->ReportJobDetailToFrame(jobLogDetail);
    return MP_SUCCESS;
}
