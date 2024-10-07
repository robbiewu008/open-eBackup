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
#include "VirtualizationBasicJob.h"
#include <client/ClientInvoke.h>
#include <protect_engines/engine_factory/EngineFactory.h>
#include "common/CTime.h"
#include "repository_handlers/factory/RepositoryFactory.h"

#ifdef WIN32
#include <repository_handlers/win32filesystem/Win32FileSystemHandler.h>
#endif
namespace {
constexpr uint32_t KB_NUM = 1024;
const std::string VOLUME_DATA_PROCESS = "VOLUME_DATA_PROCESS";
const std::string VOLUME_SEGMENT_THRESHOLD = "VOLUME_SEGMENT_THRESHOLD";
constexpr uint32_t UNIT_CONVERT = 1024;
}

VIRT_PLUGIN_NAMESPACE_BEGIN

int VirtualizationBasicJob::ExecHook(const ExecHookParam &para)
{
    int ret = FAILED;
    try {
        if (m_protectEngine == nullptr) {
            ERRLOG("Exec hook failed, protect engine handler is null.");
            return FAILED;
        }
        auto preHook = std::bind(&ProtectEngine::PreHook, m_protectEngine, std::placeholders::_1);
        auto postHook = std::bind(&ProtectEngine::PostHook, m_protectEngine, std::placeholders::_1);
        auto hook = (para.hookType == HookType::PRE_HOOK) ? preHook : postHook;
        ret = hook(para);
        if (ret == FAILED) {
            ERRLOG("Exec hook failed, stage: %d.", static_cast<int>(para.stage));
            return ret;
        }
        if (ret == DIFFERENT_FLOW) { // 只有前置才可能返回DIFFERENT_FLOW
            INFOLOG("Hook has special workflow, skip preset steps.");
            m_nextState = para.postHookState;
            ret = SUCCESS;
        } else {
            DBGLOG("Exec hook success, stage: %d.", static_cast<int>(para.stage));
            m_nextState = para.nextState;
        }
    } catch (const std::exception &e) {
        ERRLOG("Run hook(stage: %d), exception: %s.", static_cast<int>(para.stage), WIPE_SENSITIVE(e.what()).c_str());
        ret = FAILED;
    }
    return ret;
}

int VirtualizationBasicJob::InitProtectEngineHandler(JobType jobType)
{
    if (m_protectEngine != nullptr) {
        return SUCCESS;
    }
    m_protectEngine = EngineFactory::CreateProtectEngine(jobType, m_jobCommonInfo, m_parentJobId,
        m_subJobInfo ? m_subJobInfo->subJobId : "");
    if (m_protectEngine == nullptr) {
        ERRLOG("Create ProtectEngine failed.");
        return FAILED;
    }
    auto handler = std::bind(&VirtualizationBasicJob::ReportApplicationLabels, this, std::placeholders::_1);
    m_protectEngine->SetReportJobDetailHandler(handler);
    return SUCCESS;
}

void VirtualizationBasicJob::ReportLog2Agent(ReportLog2AgentParam &param)
{
    param.subJobDetails.__set_jobId(m_parentJobId);
    if (m_subJobInfo != nullptr && m_subJobInfo->subJobId != "") {
        param.subJobDetails.__set_subJobId(m_subJobInfo->subJobId);
    }
    if (param.logDetail.description != "") {
        DBGLOG("Current label is : %s.", WIPE_SENSITIVE(param.logDetail.description).c_str());
        param.logDetailList.push_back(param.logDetail);
    }
    param.subJobDetails.__set_jobStatus(param.curJobstatus);
    if (param.progress != 0) {
        param.subJobDetails.__set_progress(param.progress);
    }
    // 增量备份时，备份的数据可能为0，若不set datasize的话，Agent会判断datasize的isset为false，
    // 而上报给UBC datasize为-1，UBC会采取其他方式计算副本大小
    if (param.dataSize >= 0) {
        param.subJobDetails.__set_dataSize(param.dataSize);
    }
    param.subJobDetails.__set_logDetail(param.logDetailList);

    param.subJobDetails.__set_extendInfo(param.subJobDetails.extendInfo);

    DBGLOG("report job=[%s], subJobId=[%s], description=[%s], jobStatus=[%d].",
        param.subJobDetails.jobId.c_str(), param.subJobDetails.subJobId.c_str(),
        WIPE_SENSITIVE(param.logDetail.description).c_str(), param.curJobstatus);
    uint16_t retry = 0;
    do {
        JobService::ReportJobDetails(param.returnValue, param.subJobDetails);
    } while (param.curJobstatus == SubJobStatus::COMPLETED
        && param.returnValue.code != SUCCESS && retry++ < MAX_RETRY_CNT);
    param.logDetailList.clear();
    param.logDetail.__set_description("");
}

template<typename... Args>
void VirtualizationBasicJob::ReportJobDetailsWithLabel(const ReportJobDetailsParam &jobParam, Args... logArgs)
{
    SubJobDetails subJobDetails;
    subJobDetails.extendInfo = jobParam.extendInfo;

    std::vector<LogDetail> logDetailList;
    ActionResult result;
    LogDetail logDetail {};

    INFOLOG("Report job details label: %s level: %d jobstatus: %d totalSize: %ld extendInfo: %s, %s",
        jobParam.label.c_str(), jobParam.level, jobParam.status, jobParam.dataSize,
        WIPE_SENSITIVE(subJobDetails.extendInfo).c_str(), m_taskInfo.c_str());

    AddLogDetail(logDetail, jobParam.label, jobParam.level, logArgs...);
    AddErrCode(logDetail, jobParam.errCode, logArgs...);
    ReportLog2AgentParam param = {subJobDetails, result, logDetailList, logDetail,
        jobParam.progress, jobParam.dataSize, jobParam.status };
    ReportLog2Agent(param);
}

void VirtualizationBasicJob::ReportJobDetailWithErrorParams()
{
    ReportJobDetailsParam reportParam;
    m_protectEngine->GetReportParam(reportParam);
    std::vector<std::string> reportArgs;
    m_protectEngine->GetReportArgs(reportArgs);
    if (reportParam.label.empty() || reportArgs.empty()) {
        return;
    }
    ReportJobDetailsWithLabel(reportParam, reportArgs[0]);
}

void VirtualizationBasicJob::ReportJobDetailWithLabel(const ReportJobDetailsParam &jobParam,
                                                      const std::vector<std::string> &args)
{
    SubJobDetails subJobDetails;
    std::vector<LogDetail> logDetailList;
    ActionResult result;
    LogDetail logDetail{};
    INFOLOG("Report job detail label: %s jobstatus: %d totalSize: %ld, %s",
        jobParam.label.c_str(), jobParam.status, jobParam.dataSize, m_taskInfo.c_str());
    logDetail.__set_description(jobParam.label);
    logDetail.__set_level(jobParam.level);
    logDetail.__set_params(args);
    logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    if (jobParam.errCode != 0) {
        logDetail.__set_errorCode(jobParam.errCode);
        logDetail.__set_errorParams(args);
        if (!jobParam.extendInfo.empty()) {
            logDetail.__set_additionalDesc(std::vector<std::string>({jobParam.extendInfo}));
        }
    }
    ReportLog2AgentParam param = {subJobDetails, result, logDetailList, logDetail,
        jobParam.progress, jobParam.dataSize, jobParam.status};
    ReportLog2Agent(param);
}

void VirtualizationBasicJob::ReportJobResult(int ret, const std::string &msg, uint64_t dataSizeInByte,
    const ReportJobResultPara &reportPara)
{
    ReportJobDetailsParam param;
    if (ret == SUCCESS) {
        // 任务成功后，需要将备份数据上报
        param = {"", JobLogLevel::TASK_LOG_INFO, SubJobStatus::COMPLETED, 100, dataSizeInByte / KB_NUM};
    } else {
        if (!reportPara.m_jobDetailsParam.label.empty()) {
            ReportJobDetailWithLabel(reportPara.m_jobDetailsParam, reportPara.m_args);
            return;
        }
        param = {"", JobLogLevel::TASK_LOG_ERROR, SubJobStatus::FAILED, 100, 0};
    }
    ReportJobDetailsWithLabel(param, msg);
}

void VirtualizationBasicJob::ReportApplicationLabels(const ApplicationLabelType &label)
{
    ReportJobDetailsParam param;
    param.label = label.label;
    param.level = label.level;
    /* Job status should be decided by virtual plugin framework.
     * So we dont provide status to applications.
     */
    param.status = SubJobStatus::RUNNING;
    ReportJobDetailWithLabel(param, label.params);
}

void VirtualizationBasicJob::HandleAbortPoint()
{
    INFOLOG("Receive abort req, End Task.");
    // 释放资源
    for (const auto &handle : m_cleanHandlesForStop) {
        int32_t retry = 0;
        while (retry < MAX_RETRY_CNT) {
            if (handle() == SUCCESS) {
                break;
            }
            INFOLOG("Handle failed, will retry. times:%d", (retry + 1));
            retry++;
        }
    }
    INFOLOG("Clean all resource done.");
    SubJobDetails subJobDetails;
    std::vector<LogDetail> logDetailList;
    ActionResult result;
    LogDetail logDetail {};
    // 所有步骤上报speed都为0
    ReportLog2AgentParam param = {subJobDetails, result, logDetailList, logDetail, 0, SubJobStatus::ABORTED };
    ReportLog2Agent(param);
}

int VirtualizationBasicJob::AbortJob()
{
    if (IsAbortJob()) {
        HandleAbortPoint();
        m_nextState = 0;
        return FAILED;
    }
    return SUCCESS;
}

/**
 *  @brief 执行子任务_上报备份速度
 *
 *  @return
 */
void VirtualizationBasicJob::ReportJobSpeed(const uint64_t &dataSizeInByte)
{
    std::string description = "Virtual plugin report sub job speed.";
    ReportJobDetailsParam param = {
        "",
        JobLogLevel::TASK_LOG_INFO,
        SubJobStatus::RUNNING, 0, dataSizeInByte / KB_NUM};
    ReportJobDetailsWithLabel(param, description);
}

void VirtualizationBasicJob::ReportTaskLabel(const std::vector<std::string> &reportArgs)
{
    ReportJobDetailsParam reportParam;
    m_protectEngine->GetReportParam(reportParam);
    if (reportParam.label.empty()) {
        return;
    }
    std::vector<std::string> Args;
    if (reportArgs.empty()) {
        m_protectEngine->GetReportArgs(Args);
    } else {
        Args = reportArgs;
    }
    ReportJobDetailWithLabel(reportParam, Args);
}

bool VirtualizationBasicJob::DoInitHandlers(const AppProtect::StorageRepository &storageRepo,
    std::shared_ptr<RepositoryHandler> &repoHandler, std::string &repoPath)
{
    repoHandler = RepositoryFactory::CreateRepositoryHandler(storageRepo);
    if (repoHandler == nullptr) {
        ERRLOG("Create repository failed, %s", m_taskInfo.c_str());
        return false;
    }

    if (!GetRepoPath(repoHandler, storageRepo, repoPath)) {
        return false;
    }
    return true;
}

bool VirtualizationBasicJob::GetRepoPath(
    const std::shared_ptr<RepositoryHandler> &repoHandler, const StorageRepository &repo, std::string &repoPath)
{
    DBGLOG("In GetRepoPath(), %s", m_taskInfo.c_str());
    bool isBusinessSubJob = false;
    int subJobIndex = 0;
    if (m_subJobInfo != nullptr && m_subJobInfo->jobName != "") {
        DBGLOG("Sub job name: %s", m_subJobInfo->jobName.c_str());
        int nPos = m_subJobInfo->jobName.find(BUSINESS_SUB_JOB_NAME_PREFIX);
        if (nPos != std::string::npos) {
            std::string indexStr = m_subJobInfo->jobName.substr(nPos + BUSINESS_SUB_JOB_NAME_PREFIX.length(),
                m_subJobInfo->jobName.length() - BUSINESS_SUB_JOB_NAME_PREFIX.length());
            try {
                subJobIndex = std::stoi(indexStr);
                isBusinessSubJob = true;
                DBGLOG("Is business sub job, job index:%d, %s", subJobIndex, m_taskInfo.c_str());
            } catch (std::exception &e) {
                WARNLOG("Business sub job name exception: %s, %s", WIPE_SENSITIVE(e.what()).c_str(),
                    m_taskInfo.c_str());
            }
        }
    }
    int pathCount = repo.path.size();
    int curPathIndex = 0;
    std::string nodeBalance = Module::ConfigReader::getString(GENERAL_CONF, "DataRepoPathBalance");
    for (const auto &path : repo.path) {
        // business sub job get data repo path from index
        if (repo.repositoryType == RepositoryDataType::DATA_REPOSITORY && isBusinessSubJob) {
            if (nodeBalance == "true") {
                return LoadBalancer::GetInstance()->GetNodePath(repo.path, repoPath);
            } else if (pathCount > 0 && (subJobIndex % pathCount) == curPathIndex && repoHandler->Exists(path)) {
                repoPath = path;
                DBGLOG("Repo path:%s, %s", repoPath.c_str(), m_taskInfo.c_str());
                return true;
            }
            ++curPathIndex;
        } else {
            if (repoHandler->Exists(path)) {
                repoPath = path;
                return true;
            }
        }
    }
    ERRLOG("Failed to get repo path, %s", m_taskInfo.c_str());
    return false;
}

uint64_t VirtualizationBasicJob::GetSegementSizeFromConf()
{
    uint64_t segmentSize = Module::ConfigReader::getUint(
        VOLUME_DATA_PROCESS, VOLUME_SEGMENT_THRESHOLD) * UNIT_CONVERT * UNIT_CONVERT;
    segmentSize = (segmentSize == 0 ? DEFAULT_SEGMENT_THRESHOLD : segmentSize);

    return segmentSize;
}

bool VirtualizationBasicJob::AddNewJobWithRetry(const std::vector<SubJob> &subJobs)
{
    ActionResult result;
    ReportJobDetailsParam param = {"", JobLogLevel::TASK_LOG_INFO,
                                   SubJobStatus::RUNNING, 0, 0};
    std::string desc = "Add new Job";
    time_t startTime;
    Module::CTime::Now(startTime);
    time_t addJobTimeout = 300;
    int retryInterval = 10;
    while (true) {
        JobService::AddNewJob(result, subJobs);
        if (result.code == SUCCESS) {
            DBGLOG("Add new job success, %s", m_taskInfo.c_str());
            return true;
        }
        time_t curTime;
        Module::CTime::Now(curTime);
        if (curTime - startTime > addJobTimeout) {
            break;
        }
        WARNLOG("Add new job failed, will retry, %s.", m_taskInfo.c_str());
        Utils::SleepSeconds(retryInterval);
        ReportJobDetailsWithLabel(param, desc);
    }
    return false;
}

void VirtualizationBasicJob::VolHandlerReportTaskLabel(const std::shared_ptr<VolumeHandler> &volHandler,
    const std::vector<std::string> &reportArgs)
{
    ReportJobDetailsParam reportPara;
    volHandler->GetReportPara(reportPara);
    if (reportPara.label.empty()) {
        return;
    }
    std::vector<std::string> Args;
    if (reportArgs.empty()) {
        volHandler->GetReportArgs(Args);
    } else {
        Args = reportArgs;
    }
    ReportJobDetailWithLabel(reportPara, Args);
}

void VirtualizationBasicJob::SendAlarm(const std::string &alarmId, const std::string &alarmPara,
    const std::string &resourceId)
{
    ActionResult result;
    AppProtect::AlarmDetails alarm;
    alarm.alarmId = alarmId;
    alarm.parameter = alarmPara;
    alarm.resourceId = resourceId;
    JobService::SendAlarm(result, alarm);
    INFOLOG("Job(%s) send alrm(id: %s, param: %s). result: %d.",
        GetJobId().c_str(), alarmId.c_str(), alarmPara.c_str(), result.code);
}

bool VirtualizationBasicJob::AddWhiteList(const std::string &ipListStr)
{
    ActionResult result;
    JobService::AddIpWhiteList(result, GetParentJobId(), ipListStr);
    INFOLOG("Job(%s) add whiteList(param: %s). result: %d. message:%s",
        GetParentJobId().c_str(), ipListStr.c_str(), result.code, result.message.c_str());
    return result.code == SUCCESS; // TO DO result.code == SUCCESS;
}

LoadBalancer::LoadBalancer()
{}

LoadBalancer* LoadBalancer::GetInstance()
{
    static LoadBalancer instance;
    return &instance;
}

LoadBalancer::~LoadBalancer()
{
    std::unordered_map<std::string, int32_t>::iterator it = m_totalDataNode.begin();
    std::unordered_map<std::string, int32_t>::iterator itErase;
    while (it != m_totalDataNode.end()) {
        itErase = it;
        it++;
        m_totalDataNode.erase(itErase);
    }
}

bool LoadBalancer::GetNodePath(const std::vector<std::string> &repoPaths, std::string &dataPath)
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    std::vector<std::pair<std::string, int32_t>> dataNode;
    std::string prePath;
    for (const auto &path : repoPaths) {
        int32_t pos = path.rfind("/");
        std::string node = path.substr(pos);
        prePath = path.substr(0, pos);
        if (m_totalDataNode.find(node) != m_totalDataNode.end()) {
            dataNode.push_back(make_pair(node, m_totalDataNode[node]));
        } else {
            m_totalDataNode[node] = 0;
            dataNode.push_back(make_pair(node, 0));
        }
    }
    std::sort(dataNode.begin(), dataNode.end(), [=] (std::pair<std::string, int32_t> a,
        std::pair<std::string, int32_t> b) {
        return a.second < b.second;
    });
    if (dataNode.size() == 0) {
        ERRLOG("Node path is null.");
        return false;
    }
#ifndef WIN32
    std::shared_ptr<RepositoryHandler> repoHandler = std::make_shared<FileSystemHandler>();
#else
    std::shared_ptr<RepositoryHandler> repoHandler = std::make_shared<Win32FileSystemHandler>();
#endif
    std::string nodePath;
    for (const auto &node : dataNode) {
        nodePath = prePath + node.first;
        if (repoHandler->Exists(nodePath)) {
            m_totalDataNode[node.first]++;
            dataPath = nodePath;
            INFOLOG("BusinessSubJob use datarepo path: %s, node path size:[%d].", dataPath.c_str(),
                m_totalDataNode[node.first]);
            return true;
        }
    }
    ERRLOG("No valid datapath.");
    return false;
}

void LoadBalancer::RemoveNodePath(const std::string &dataPath)
{
    std::lock_guard<std::mutex> lock(m_mapMutex);
    int32_t pos = dataPath.rfind("/");
    std::string node = dataPath.substr(pos);
    if (m_totalDataNode.find(node) != m_totalDataNode.end()) {
        m_totalDataNode[node]--;
        INFOLOG("BusinessSubJob remove datarepo path: %s, node path size:[%d].", dataPath.c_str(),
            m_totalDataNode[node]);
    }
}


VIRT_PLUGIN_NAMESPACE_END
