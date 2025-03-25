/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Job.h
 * @brief Implement for external sub job
 * @version 1.1.0
 * @date 2021-10-29
 * @author wangguitao 00510599
 */
#include <thrift/transport/TTransportException.h>
#include <map>
#include <algorithm>
#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include "common/Log.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "message/curlclient/DmeRestClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "servicecenter/timerservice/include/ITimerService.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "taskmanager/externaljob/PluginSubJob.h"

namespace AppProtect {
const int PLUGIN_UPDATE_DETAIL_TIMEOUT = 600 * 1000;
std::map<AppProtect::SubJobStatus::type, SubJobState> StatusTransferMap = {
    {AppProtect::SubJobStatus::COMPLETED, SubJobState::SubJobComplete},
    {AppProtect::SubJobStatus::PARTIAL_COMPLETED, SubJobState::SubJobComplete},
    {AppProtect::SubJobStatus::ABORTED, SubJobState::SubJobComplete},
    {AppProtect::SubJobStatus::ABORTED_FAILED, SubJobState::SubJobComplete},
    {AppProtect::SubJobStatus::ABORTING, SubJobState::Aborting},
    {AppProtect::SubJobStatus::FAILED_NOTRY, SubJobState::SubJobFailed},
    {AppProtect::SubJobStatus::FAILED, SubJobState::SubJobFailed},
    {AppProtect::SubJobStatus::RUNNING, SubJobState::Running}
};

PluginSubJob::PluginSubJob(const PluginJobData& data) : Job(data)
{
    m_mapFunc.insert(std::make_pair(MainJobType::BACKUP_JOB,
        std::bind(&PluginSubJob::ExecBackupJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::RESTORE_JOB,
        std::bind(&PluginSubJob::ExecRestoreJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::DELETE_COPY_JOB,
        std::bind(&PluginSubJob::ExecDelCopyJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::LIVEMOUNT_JOB,
        std::bind(&PluginSubJob::ExecLivemountJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::CANCEL_LIVEMOUNT_JOB,
        std::bind(&PluginSubJob::ExecCancelLivemountJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::BUILD_INDEX_JOB,
        std::bind(&PluginSubJob::ExecBuildIndexJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::INSTANT_RESTORE_JOB,
        std::bind(&PluginSubJob::ExecInrestoreJob, this, std::placeholders::_1)));
    m_mapFunc.insert(std::make_pair(MainJobType::CHECK_COPY_JOB,
        std::bind(&PluginSubJob::ExecCheckCopyJob, this, std::placeholders::_1)));

    m_pluginUdTimeoutHandler.insert(std::make_pair(SubJobState::Aborting,
        std::bind(&PluginSubJob::AbortTimeoutHandler, this)));

    m_abortHandleMap.insert(std::make_pair(SubJobState::UNDEFINE,
        std::bind(&PluginSubJob::AbortBeforePluginRun, this)));
    m_abortHandleMap.insert(std::make_pair(SubJobState::PrepareComplete,
        std::bind(&PluginSubJob::AbortBeforePluginRun, this)));
    m_abortHandleMap.insert(std::make_pair(SubJobState::PrepareFailed,
        std::bind(&PluginSubJob::AbortBeforePluginRun, this)));
    m_abortHandleMap.insert(std::make_pair(SubJobState::Running,
        std::bind(&PluginSubJob::NotifyPluginAbort, this)));
}

PluginSubJob::~PluginSubJob()
{
    RemovePluginTimer();
}

mp_int32 PluginSubJob::Exec()
{
    INFOLOG("Start to exec sub job, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    if (m_executor) {
        return m_executor(MP_SUCCESS);
    }
    return MP_SUCCESS;
}

Executor PluginSubJob::GetJobSuccess()
{
    return [this](int32_t) {
        StartReportTiming();
        if (m_data.status != mp_uint32(SubJobState::SubJobComplete)) {
            ChangeState(SubJobState::Running);
        }
        return MP_SUCCESS;
    };
}

Executor PluginSubJob::GetJobFailed()
{
    return [this](mp_int32) {
        ERRLOG("Sub job failed, ret=%d, jobId=%s, subJobId=%s.", m_iRet, m_data.mainID.c_str(), m_data.subID.c_str());
        ChangeState(SubJobState::SubJobFailed);
        if (!m_data.subID.empty()) {
            ReportDetails();
        }
        return MP_FAILED;
    };
}

bool PluginSubJob::CheckSubJobCompeleteStatus(const AppProtect::SubJobDetails& jobDetails)
{
    auto it = StatusTransferMap.find(jobDetails.jobStatus);
    if (it != StatusTransferMap.end()) {
        SubJobState state = it->second;
        return IsSubJobFinish(state);
    }
    WARNLOG("Get job status failed, Jobstatus=%d, jobId=%s, subJobId=%s.", mp_int32(jobDetails.jobStatus),
        jobDetails.jobId.c_str(), jobDetails.subJobId.c_str());
    return false;
}

void PluginSubJob::NotifyJobDetail(const AppProtect::SubJobDetails& jobDetail)
{
    auto it = StatusTransferMap.find(jobDetail.jobStatus);
    if (it != StatusTransferMap.end()) {
        ChangeState(it->second);
    }
}

bool PluginSubJob::NeedReportJobDetail()
{
    return false;
}

mp_int32 PluginSubJob::Abort()
{
    INFOLOG("Abort jobId=%s, subJobId=%s, state=%d", m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status);
    if (m_data.subType == SubJobType::type::POST_SUB_JOB) {
        WARNLOG("Post job cound not be aborted, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return MP_FAILED;
    }
    auto iter = m_abortHandleMap.find(SubJobState(m_data.status));
    if (iter != m_abortHandleMap.end()) {
        iter->second();
    }
    INFOLOG("Abort jobId=%s, subJobId=%s succ.", m_data.mainID.c_str(), m_data.subID.c_str());
    return MP_SUCCESS;
}

void PluginSubJob::ReportDetails()
{
    AppProtect::SubJobDetails jobInfo;
    jobInfo.jobId = m_data.mainID;
    jobInfo.subJobId = m_data.subID;
    // update stage
    auto iter = std::find_if(StatusTransferMap.begin(), StatusTransferMap.end(),
        [this](const std::pair<AppProtect::SubJobStatus::type, SubJobState> iter) {
        return iter.second == SubJobState(m_data.status);
    });
    if (iter == StatusTransferMap.end()) {
        WARNLOG("Cannot find relative subjob, jobId=%s, subJobId=%s, status for subjob state %d.",
            jobInfo.jobId.c_str(), jobInfo.subJobId.c_str(), m_data.status);
        return;
    }
    jobInfo.jobStatus = iter->first;
    Json::Value detail;
    StructToJson(jobInfo, detail);
    std::string context = detail.toStyledString();
    // report to DME
    DmeRestClient::HttpReqParam param("PUT", "/v1/dme-unified/tasks/details", context);
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Report Detail failed, jobId=%s, subJobId=%s.", jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
    }
}

bool PluginSubJob::AbortTimeoutHandler()
{
    WARNLOG("Update jobId=%s, subJobId=%s detail timeout.", m_data.mainID.c_str(), m_data.subID.c_str());
    m_data.status = mp_uint32(SubJobState::SubJobComplete);
    ReportDetails();
    return true;
}

void PluginSubJob::ChangeState(SubJobState state)
{
    {
        std::lock_guard<std::mutex> lk(m_mutexChangeState);
        if (m_data.status == mp_uint32(state)) {
            return;
        }
        INFOLOG("JobId=%s, subjobId=%s, form status=%d change to status=%d.",
            m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status, mp_uint32(state));
        m_data.status = mp_uint32(state);
    }

    if (IsSubJobFinish(state)) {
        INFOLOG("JobId=%s, subjobId=%s finished. status=%d", m_data.mainID.c_str(),
            m_data.subID.c_str(), m_data.status);
        RemovePluginTimer();
        return;
    }

    // update m_pluginUdTimer
    if (m_pluginUdTimer.get() == nullptr) {
        auto service  =
            servicecenter::ServiceFactory::GetInstance()->GetService<timerservice::ITimerService>("ITimerService");
        if (service) {
            m_pluginUdTimer = service->CreateTimer();
        } else {
            ERRLOG("Create plugin update detail timer failed, jobId=%s, subJobId=%s.",
                m_data.mainID.c_str(), m_data.subID.c_str());
            return;
        }
    }
    if (m_pluginUdTimeId > 0) {
        if (!m_pluginUdTimer->RemoveTimeoutExecutor(m_pluginUdTimeId)) {
            ERRLOG("Failed to remove timeout executor %d, jobId=%s, subJobId=%s.",
                m_pluginUdTimeId, m_data.mainID.c_str(), m_data.subID.c_str());
            return;
        }
    }
    auto iter = m_pluginUdTimeoutHandler.find(state);
    if (iter != m_pluginUdTimeoutHandler.end()) {
        INFOLOG("Add timeout handler, Timer object:%x", m_pluginUdTimer);
        m_pluginUdTimeId = m_pluginUdTimer->AddTimeoutExecutor(iter->second, PLUGIN_UPDATE_DETAIL_TIMEOUT);
        m_pluginUdTimer->Start();
    }
}

void PluginSubJob::AbortBeforePluginRun()
{
    INFOLOG("Job jobId=%s, subJobId=%s, receive abort before running.", m_data.mainID.c_str(), m_data.subID.c_str());
    ChangeState(SubJobState::SubJobComplete);
}

void PluginSubJob::NotifyPluginAbort()
{
    INFOLOG("NotifyPluginAbort, jobId=%s, subJobId=%s, state=%d.", m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status);
    // notify plugin to stop
    mp_int32 ret = SendAbortToPlugin();
    if (ret == MP_SUCCESS) {
        // when notify success, waitting for task aborting progress
        ChangeState(SubJobState::Aborting);
        return;
    }
    // if no need for reply of abort, go to complete stage direct
    ChangeState(SubJobState::SubJobComplete);
}

mp_int32 PluginSubJob::GetSignedFromJobDetail(AppProtect::SubJobStatus::type jobStatus)
{
    switch (jobStatus) {
        case SubJobStatus::type::COMPLETED:
            return MP_SUCCESS;

        case SubJobStatus::type::ABORTED:
        case SubJobStatus::type::ABORTED_FAILED:
        case SubJobStatus::type::FAILED:
        case SubJobStatus::type::FAILED_NOTRY:
            return MP_FAILED;
        
        default:
            return MP_EAGAIN;
    }
}

bool PluginSubJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    return true;
}

Executor PluginSubJob::GetEmptyExcutor()
{
    auto mainID = m_data.mainID;
    auto subID = m_data.subID;
    return [mainID, subID](int32_t) {
        INFOLOG("Job jobId=%s, subJobId=%s, GetEmptyExcutor.", mainID.c_str(), subID.c_str());
        return MP_SUCCESS;
    };
}

Executor PluginSubJob::GetReportExecutor(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result,
    const PluginJobData& data)
{
    return [event, result, &data](int32_t resultCode) {
        return ReportAction(event, result, resultCode, data);
    };
}

mp_int32 PluginSubJob::ReportAction(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result,
    int32_t resultCode, const PluginJobData& data)
{
    INFOLOG("Subjob ReportAction jobId=%s subJobId=%s: event:%d result:%d resultCode:%d.",
        data.mainID.c_str(), data.subID.c_str(), event, result, resultCode);

    if (DetermineWhetherToReportAction(event, result, resultCode, data) == false) {
        INFOLOG("Subjob no need report.");
        return MP_FAILED;
    }
 
    static std::set<std::pair<ActionEvent, EventResult>> ReportMap = {
        {ActionEvent::EXEC_POST_SUBJOB, EventResult::EXECUTING},
        {ActionEvent::EXEC_POST_SUBJOB, EventResult::FAILED},
        {ActionEvent::EXEC_BUSI_SUBJOB, EventResult::FAILED},
        {ActionEvent::EXEC_POST_SCRIPT, EventResult::EXEC_SCRIPT_SUCCESS},
        {ActionEvent::EXEC_POST_SCRIPT, EventResult::EXEC_SCRIPT_FAILED},
        {ActionEvent::MOUNT_NAS, EventResult::FAILED}};

    std::pair<ActionEvent, EventResult> key{event, result};
    auto it = ReportMap.find(key);
    if (it != ReportMap.end()) {
        auto detail = ReportJobDetailFactory::GetInstance()->GeneratorSubJobDetail(event, result, resultCode, data);
        ReportJobDetailFactory::GetInstance()->SendDetailToDme(detail, -1);
        return MP_SUCCESS;
    }
    return MP_SUCCESS;
}

bool PluginSubJob::DetermineWhetherToReportAction(PluginSubJob::ActionEvent event, PluginSubJob::EventResult result,
    int32_t resultCode, const PluginJobData& data)
{
    if (event != ActionEvent::EXEC_POST_SCRIPT) {
        return true;
    }

    // 自定义后置脚本为空时，不上报任务label
    if (data.scriptFileName.empty()) {
        INFOLOG("Post Script not exist, jobId=%s", data.mainID.c_str());
        return false;
    }
 
    INFOLOG("Post Script(strScriptFileName=%s) is exist, jobId=%s", data.scriptFileName.c_str(), data.mainID.c_str());
    return true;
}

bool PluginSubJob::IsSubJobFinish(SubJobState state)
{
    return (state == SubJobState::PrepareFailed) ||
            (state == SubJobState::SubJobComplete) ||
            (state == SubJobState::SubJobFailed);
}

void PluginSubJob::RemovePluginTimer()
{
    if (m_pluginUdTimer) {
        m_pluginUdTimer->RemoveTimeoutExecutor(m_pluginUdTimeId);
        m_pluginUdTimer->Stop();
        auto service  =
            servicecenter::ServiceFactory::GetInstance()->GetService<timerservice::ITimerService>("ITimerService");
        if (service) {
            service->DeleteTimer(m_pluginUdTimer);
        }
        m_pluginUdTimer.reset();
    }
}
}  // namespace AppProtect
