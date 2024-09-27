#include "taskmanager/externaljob/ReportJobDetailFactory.h"

#include <chrono>
#include "message/curlclient/DmeRestClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "host/host.h"
#include "common/Ip.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "message/curlclient/RestClientCommon.h"

#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"

namespace AppProtect {
std::map<int32_t, std::pair<DmeTaskStage, DmeTaskStatus>> JobStateTransfer = {
    {static_cast<int32_t>(AppProtect::MainJobState::INITIALIZING),
        {DmeTaskStage::AGENT_PREPARING, DmeTaskStatus::RUNNING}},
    {static_cast<int32_t>(AppProtect::MainJobState::CHECK_BACKUP_TYPE),
        {DmeTaskStage::AGENT_PREPARING, DmeTaskStatus::RUNNING}},
    {static_cast<int32_t>(AppProtect::MainJobState::PRE_JOB_RUNNING),
        {DmeTaskStage::PRE_TASK_COMPLETE, DmeTaskStatus::RUNNING}},
    {static_cast<int32_t>(AppProtect::MainJobState::GENERATE_JOB_RUNNING),
        {DmeTaskStage::PRE_TASK_COMPLETE, DmeTaskStatus::RUNNING}},
    {static_cast<int32_t>(AppProtect::MainJobState::COMPLETE),
        {DmeTaskStage::GENERATE_TASK_COMPLETE, DmeTaskStatus::COMPLETED}},
    {static_cast<int32_t>(AppProtect::MainJobState::FAILED),
        {DmeTaskStage::NULL_STAGE, DmeTaskStatus::FAILED}}
};

const std::string Log_Param_Agent_ID = "AgentNodeID";
const std::string Log_Param_Agent_IP = "AgentNodeIP";
const std::string Log_Param_Main_Job_ID = "MainJobID";
const std::string Log_Param_Sub_Job_ID = "SubJobID";
const std::string Log_Param_Main_Job_Type = "MainJobType";
const std::string Log_Param_Sub_Job_Type = "SubJobType";
const std::string Script_Result = "ScriptResult";
const std::string NULL_PARAM = "";

struct LogMainKey {
    PluginMainJob::ActionEvent event;
    PluginMainJob::EventResult result;
    friend bool operator<(const AppProtect::LogMainKey l, const AppProtect::LogMainKey r);
};

bool operator<(const AppProtect::LogMainKey l, const AppProtect::LogMainKey r)
{
    return static_cast<int32_t>(l.event) < static_cast<int32_t>(r.event) ||
        (static_cast<int32_t>(l.event) == static_cast<int32_t>(r.event) &&
        static_cast<int32_t>(l.result) < static_cast<int32_t>(r.result));
}

struct LogSubKey {
    PluginSubJob::ActionEvent event;
    PluginSubJob::EventResult result;
    friend bool operator<(const AppProtect::LogSubKey l, const AppProtect::LogSubKey r);
};

bool operator<(const AppProtect::LogSubKey l, const AppProtect::LogSubKey r)
{
    return static_cast<int32_t>(l.event) < static_cast<int32_t>(r.event) ||
        (static_cast<int32_t>(l.event) == static_cast<int32_t>(r.event) &&
        static_cast<int32_t>(l.result) < static_cast<int32_t>(r.result));
}

struct LogData {
    std::string label;
    std::vector<std::string> params;
    AppProtect::JobLogLevel::type level;
    int64_t errorCode;
};

std::map<LogMainKey, LogData> MainJobLogMap = {
    {{PluginMainJob::ActionEvent::GENE_POST_JOB, PluginMainJob::EventResult::START},
        {"agent_execute_prepare_task_success_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginMainJob::ActionEvent::EXEC_PRE_SCRIPTR, PluginMainJob::EventResult::SUCCESS},
        {"agent_execute_pre_script_success_label", {Log_Param_Agent_IP, Script_Result},
            JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginMainJob::ActionEvent::EXEC_PRE_SCRIPTR, PluginMainJob::EventResult::FAILED},
        {"agent_execute_pre_script_fail_label", {Log_Param_Agent_IP, Script_Result},
            JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB, PluginMainJob::EventResult::EXECUTING},
        {"agent_execute_prerequisit_task_success_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB, PluginMainJob::EventResult::FAILED},
        {"agent_execute_prerequisit_task_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB, PluginMainJob::EventResult::EXECUTING},
        {"agent_execute_generate_task_success_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB, PluginMainJob::EventResult::FAILED},
        {"agent_execute_generate_task_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginMainJob::ActionEvent::MOUNT_NAS, PluginMainJob::EventResult::FAILED},
        {"agent_execute_mount_nas_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginMainJob::ActionEvent::CONFIG_QOS, PluginMainJob::EventResult::FAILED},
        {"agent_set_qos_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginMainJob::ActionEvent::PLUGIN_LOG_BACKUP, PluginMainJob::EventResult::FAILED},
        {"agent_execute_mount_nas_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}}
};

std::map<LogSubKey, LogData> SubJobLogMap = {
    {{PluginSubJob::ActionEvent::EXEC_POST_SUBJOB, PluginSubJob::EventResult::EXECUTING},
        {"agent_start_execute_post_task_success_label", {Log_Param_Agent_IP, Log_Param_Sub_Job_ID},
            JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginSubJob::ActionEvent::EXEC_POST_SUBJOB, PluginSubJob::EventResult::FAILED},
        {"agent_execute_post_task_fail_label", {Log_Param_Agent_IP, Log_Param_Sub_Job_ID},
            JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginSubJob::ActionEvent::EXEC_POST_SCRIPT, PluginSubJob::EventResult::EXEC_SCRIPT_FAILED},
        {"agent_execute_post_script_fail_label", {Log_Param_Agent_IP, Script_Result}, JobLogLevel::type::TASK_LOG_WARNING, 0}},
    {{PluginSubJob::ActionEvent::EXEC_POST_SCRIPT, PluginSubJob::EventResult::EXEC_SCRIPT_SUCCESS},
        {"agent_execute_post_script_success_label", {Log_Param_Agent_IP, Script_Result},
            JobLogLevel::type::TASK_LOG_INFO, 0}},
    {{PluginSubJob::ActionEvent::EXEC_BUSI_SUBJOB, PluginSubJob::EventResult::FAILED},
        {"agent_execute_sub_task_fail_label", {Log_Param_Agent_IP, Log_Param_Sub_Job_ID},
            JobLogLevel::type::TASK_LOG_ERROR, 0}},
    {{PluginSubJob::ActionEvent::MOUNT_NAS, PluginSubJob::EventResult::FAILED},
        {"agent_execute_mount_nas_fail_label", {Log_Param_Agent_IP}, JobLogLevel::type::TASK_LOG_ERROR, 0}}
};

static std::string GetJobType(MainJobType jobType)
{
    static std::map<MainJobType, std::string> TypeMap = {
        {MainJobType::BACKUP_JOB, "agent_task_type_backup"},
        {MainJobType::RESTORE_JOB, "agent_task_type_restore"},
        {MainJobType::LIVEMOUNT_JOB, "agent_task_type_mount"},
        {MainJobType::BUILD_INDEX_JOB, "agent_task_type_index_scan"},
        {MainJobType::INSTANT_RESTORE_JOB, "agent_task_type_instant_recovery"}
    };
    auto it = TypeMap.find(jobType);
    if (it != TypeMap.end()) {
        return it->second;
    }
    return "undefine";
}

static std::string GetHostID()
{
    CHost host;
    std::string strSN;
    mp_int32 iRet = host.GetHostSN(strSN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
        return std::string("");
    }
    return std::move(strSN);
}

static std::string GetHostIP()
{
    static std::string listenIP;
    static std::string listenPort;
    if (listenIP.empty() && CIP::GetListenIPAndPort(listenIP, listenPort) != MP_SUCCESS) {
        ERRLOG("Get Host listen IP and port failed.");
        return GetHostID();
    }
    return std::move(listenIP);
}

static std::vector<std::string> FillParams(const std::vector<std::string>& parmas, const PluginJobData& data)
{
    std::vector<std::string> p;
    for (const auto& it : parmas) {
        if (it == Log_Param_Agent_IP) {
            p.push_back(GetHostIP());
        } else if (it == Log_Param_Agent_ID) {
            p.push_back(GetHostID());
        } else if (it == Log_Param_Main_Job_ID) {
            p.push_back(data.mainID);
        } else  if (it == Log_Param_Main_Job_Type) {
            p.push_back(GetJobType(data.mainType));
        } else if (it == Log_Param_Sub_Job_ID) {
            p.push_back(data.subID);
        } else if (it == Script_Result) {
            p.push_back(data.scriptResult);
        } else if (it == NULL_PARAM) {
            continue;
        } else {
            ERRLOG("No such params :%s", it.c_str());
        }
    }
    return p;
}

static void FillMainJobLog(AppProtect::SubJobDetails& detail, PluginMainJob::ActionEvent event,
    PluginMainJob::EventResult result, const PluginJobData& data)
{
    using namespace std::chrono;
    auto jobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    if (jobHandler) {
        auto job = jobHandler->GetRunJobById(detail.jobId, detail.subJobId);
        if (job) {
            mp_bool clearDetail = MP_TRUE;
            AppProtect::LogDetail stLogDetail;
            for (const auto &iter : job->GetData().logDetail) {
                JsonToStruct(iter, stLogDetail);
                detail.logDetail.push_back(stLogDetail);
                // 插件无法执行任务场景不清理LogDetail，IsReportMainJobDetail函数会过滤
                clearDetail = (stLogDetail.errorCode == ERROR_PLUGIN_CANNOT_EXEC_ON_AGENT) ? MP_FALSE : MP_TRUE;
            }
            if (clearDetail) {
                job->ClearLogDetail();  // logdetail上报后，需要将Job中的logdetail清除，防止后面重复上报
            }
        }
    }
    LogMainKey key {event, result};
    auto it = MainJobLogMap.find(key);
    if (it == MainJobLogMap.end()) {
        return;
    }
    auto& logData = it->second;
    AppProtect::LogDetail logDetail;
    logDetail.__set_description(logData.label);
    logDetail.__set_level(logData.level);
    logDetail.__set_params(FillParams(logData.params, data));
    system_clock::duration d = system_clock::now().time_since_epoch();
    milliseconds mil = duration_cast<milliseconds>(d);
    logDetail.__set_timestamp(mil.count());
    detail.logDetail.push_back(logDetail);
}

static void FillSubJobLog(AppProtect::SubJobDetails& detail, PluginSubJob::ActionEvent event,
    PluginSubJob::EventResult result, const PluginJobData& data)
{
    using namespace std::chrono;
    LogSubKey key {event, result};
    auto it = SubJobLogMap.find(key);
    if (it == SubJobLogMap.end()) {
        return;
    }
    auto& logData = it->second;
    AppProtect::LogDetail logDetail;
    logDetail.__set_description(logData.label);
    logDetail.__set_level(logData.level);
    logDetail.__set_params(FillParams(logData.params, data));
    system_clock::duration d = system_clock::now().time_since_epoch();
    milliseconds mil = duration_cast<milliseconds>(d);
    logDetail.__set_timestamp(mil.count());
    detail.logDetail.push_back(logDetail);
}

std::shared_ptr<ReportJobDetailFactory> ReportJobDetailFactory::GetInstance()
{
    static std::shared_ptr<ReportJobDetailFactory> g_instance = std::make_shared<ReportJobDetailFactory>();
    return g_instance;
}

AppProtect::SubJobDetails ReportJobDetailFactory::GeneratorMainJobDetail(PluginMainJob::ActionEvent event,
    PluginMainJob::EventResult result, mp_int32 resultCode, const PluginJobData& data)
{
    AppProtect::SubJobDetails detail;
    detail.__set_jobId(data.mainID);
    detail.__set_subJobId("");

    FillMainJobLog(detail, event, result, data);
    if (result == PluginMainJob::EventResult::FAILED && resultCode > 0) {
        if (detail.logDetail.size() == 1) {
            detail.logDetail.front().errorCode = resultCode;
            detail.logDetail.front().errorParams = data.vecErrorParams;
        }
    }
    if (result == PluginMainJob::EventResult::SUCCESS) {
        for (Json::Value jDetail : data.logDetail) {
            AppProtect::LogDetail stlogDetail;
            JsonToStruct(jDetail, stlogDetail);
            detail.logDetail.push_back(stlogDetail);
        }
    }
    return detail;
}

AppProtect::SubJobDetails ReportJobDetailFactory::GeneratorSubJobDetail(PluginSubJob::ActionEvent event,
    PluginSubJob::EventResult result, mp_int32 resultCode, const PluginJobData& data)
{
    AppProtect::SubJobDetails detail;
    detail.__set_jobId(data.mainID);
    detail.__set_subJobId(data.subID);
    switch (result) {
        case AppProtect::PluginSubJob::EventResult::EXECUTING: {
            detail.__set_jobStatus(SubJobStatus::type::RUNNING);
            break;
        }
        case AppProtect::PluginSubJob::EventResult::FAILED: {
            detail.__set_jobStatus(SubJobStatus::type::FAILED);
            break;
        }
        case AppProtect::PluginSubJob::EventResult::SUCCESS: {
            detail.__set_jobStatus(SubJobStatus::type::COMPLETED);
            break;
        }
        case AppProtect::PluginSubJob::EventResult::EXEC_SCRIPT_SUCCESS: {
            detail.__set_jobStatus(SubJobStatus::type::RUNNING);
            break;
        }
        case AppProtect::PluginSubJob::EventResult::EXEC_SCRIPT_FAILED: {
            detail.__set_jobStatus(SubJobStatus::type::RUNNING);
            break;
        }
        default:
            break;
    }

    FillSubJobLog(detail, event, result, data);
    if ((result == PluginSubJob::EventResult::FAILED || result == PluginSubJob::EventResult::EXEC_SCRIPT_FAILED)
        && resultCode > 0) {
        if (detail.logDetail.size() == 1) {
            detail.logDetail.front().errorCode = resultCode;
            detail.logDetail.front().errorParams = data.vecErrorParams;
        }
    }
    return detail;
}

mp_int32 ReportJobDetailFactory::SendDetailToDme(const AppProtect::SubJobDetails& jobInfo, mp_int32 jobStage)
{
    LOGGUARD("");
    Json::Value detail;
    if (jobInfo.subJobId.empty()) {
        StructToJson(jobInfo, detail);
        TransferJobStageToJson(jobStage, detail);
        if (!IsReportMainJobDetail(jobInfo, detail)) {
            return MP_SUCCESS;
        }
    } else {
        StructToJson(jobInfo, detail);
    }
    JobDetailAddNodeId(jobInfo, detail);
    mp_int32 iRet = SendAndhandleRequest(jobInfo, detail);
    return iRet;
}

mp_int32 ReportJobDetailFactory::SendAndhandleRequest(
    const AppProtect::SubJobDetails& jobInfo, const Json::Value& detail)
{
    DmeRestClient::HttpReqParam param("PUT", "/v1/dme-unified/tasks/details", detail.toStyledString());
    std::string jsonRepStr;
    WipeSensitiveForJsonData(detail.toStyledString(), jsonRepStr);
    DBGLOG("Report job detail is %s.", jsonRepStr.c_str());
    param.mainJobId = jobInfo.jobId;
    HttpResponse response;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    if (iRet == MP_SUCCESS && response.statusCode == SC_OK) {
        if (jobInfo.subJobId.empty()) {
            StopReportMainJobDetail(jobInfo.jobId, detail);
        }
        INFOLOG("Report job detail to dme success, jobId=%s, subJobId=%s.",
            jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        return iRet;
    } else {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        WARNLOG("ReportJobDetail failed, errcode: %s, jobId=%s, subJobId=%s.", errMsg.errorCode.c_str(),
            jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        if (std::stoi(errMsg.errorCode) == ERR_INCONSISTENT_STATUS) {
            WARNLOG("Job have been reset in ubc, need abort the job.Avoid plugin retry, regard it as success.");
            AppProtect::AppProtectJobHandler::GetInstance()->AbortJob(jobInfo.jobId, jobInfo.subJobId);
            return MP_SUCCESS;
        }
        if (std::stoi(errMsg.errorCode) == ERR_OBJ_NOT_EXIST) {
            WARNLOG("Ubc lost present main job, try to retrieve the job by agent.");
            auto mainJobInfos = AppProtectJobHandler::GetInstance()->GetMainJobs();
            auto pos = std::find_if(mainJobInfos.begin(), mainJobInfos.end(), [&](const Json::Value& item) -> bool {
                return jobInfo.jobId == item["taskId"].asString();
            });
            AppProtect::AppProtectJobHandler::GetInstance()->AbortJob(jobInfo.jobId, jobInfo.subJobId);
            if (pos == std::end(mainJobInfos)) {
                INFOLOG("MainJob is not exists. jobId=%s, cnt: %d.", jobInfo.subJobId.c_str(), mainJobInfos.size());
                return MP_SUCCESS;
            }
            ReportMainAndPostJobInformation(*pos);
            return MP_SUCCESS;
        }
        return MP_FAILED;
    }
}

mp_int32 ReportJobDetailFactory::ReportMainAndPostJobInformation(const Json::Value& mainJob)
{
    INFOLOG("Start to report main and post job information to ubc.");
    SubJob stSubJob;
    stSubJob.jobId = mainJob["taskId"].asString();
    stSubJob.jobType = SubJobType::type::POST_SUB_JOB;
    stSubJob.jobName = mainJob["taskId"].asString() + "_PostJob";
    stSubJob.jobPriority = 0xFFFF;
    if (!mainJob["extendInfo"]["multiPostJob"].isNull() && mainJob["extendInfo"]["multiPostJob"].isString()) {
        mp_string multiPostJob = mainJob["extendInfo"]["multiPostJob"].asString();
        stSubJob.policy = (multiPostJob.compare("true") == 0) ? ExecutePolicy::EVERY_NODE_ONE_TIME
                                                              : ExecutePolicy::ANY_NODE;
    } else {
        stSubJob.policy = ExecutePolicy::ANY_NODE;
    }
    stSubJob.ignoreFailed = false;
    Json::Value postJobValue;
    StructToJson(stSubJob, postJobValue);
    Json::Value jobValue;
    jobValue["mainTask"] = mainJob;
    jobValue["postSubTask"] = postJobValue;

    auto dmeClient = DmeRestClient::GetInstance();
    if (dmeClient == nullptr) {
        ERRLOG("get dme rest client faield");
        return MP_FAILED;
    }
    HttpResponse response;
    DmeRestClient::HttpReqParam param("POST", "/v1/dme-unified/tasks/upload", jobValue.toStyledString());
    param.mainJobId = mainJob["taskId"].asString();
    mp_int32 iRet = dmeClient->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Send request report Job Information to dme failed, iRet=%d.", iRet);
        return iRet;
    }
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Report Job Information failed, jobId=%s, errorCode=%s, errorMessage=%s.",
            mainJob["taskId"].asString().c_str(), errMsg.errorCode.c_str(), errMsg.errorMessage.c_str());
        return std::stoi(errMsg.errorCode);
    }
    return MP_SUCCESS;
}

mp_void ReportJobDetailFactory::TransferJobStageToJson(mp_int32 jobMainStage, Json::Value& value)
{
    int32_t jobStage = static_cast<mp_int32>(DmeTaskStage::GENERATE_TASK_COMPLETE);
    int32_t jobStatus = static_cast<mp_int32>(DmeTaskStatus::FAILED);
    auto it = JobStateTransfer.find(jobMainStage);
    if (it != JobStateTransfer.end()) {
        jobStage = static_cast<mp_int32>(it->second.first);
        jobStatus = static_cast<mp_int32>(it->second.second);
    }
    value["taskStage"] = jobStage;
    value["taskStatus"] = jobStatus;
    if (jobStage == static_cast<mp_int32>(DmeTaskStage::NULL_STAGE)) {
        DBGLOG("Remove taskStage field for jpob failed");
        value.removeMember("taskStage");
    }
}

mp_bool ReportJobDetailFactory::IsReportMainJobDetail(const AppProtect::SubJobDetails& jobInfo, Json::Value &detail)
{
    for (const auto &it : jobInfo.logDetail) {
        // 主任务阶段，插件上报该错误码之后，Agent会在TransferJobStageToJson函数中将任务状态由FAILED转换为RUNNING，
        // 所以不直接透传给DME，稍后由Agent整合后统一上报主任务FAILED状态给DME进行任务切换
        if (it.errorCode == ERROR_PLUGIN_CANNOT_EXEC_ON_AGENT &&
            detail["taskStatus"] == static_cast<mp_int32>(DmeTaskStatus::RUNNING)) {
            INFOLOG("No need to report job detail, plugin cannot execute job on agent, jobId=%s, subJobId=%s",
                jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
            return MP_FALSE;
        }
    }
    return MP_TRUE;
}

mp_void ReportJobDetailFactory::JobDetailAddNodeId(const AppProtect::SubJobDetails& jobInfo, Json::Value &detail)
{
    for (const auto &it : jobInfo.logDetail) {
        // 有该错误码，并且任务状态失败，需要添加nodeid信息让DME进行任务切换
        if (it.errorCode == ERROR_PLUGIN_CANNOT_EXEC_ON_AGENT &&
            detail["taskStatus"] == static_cast<mp_int32>(DmeTaskStatus::FAILED)) {
            detail["extendInfo"]["nodeId"] = GetHostID();
            INFOLOG("Plugin cannot execute job on agent, add nodeid to extendInfo, jobId=%s, subJobId=%s",
                jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        }
    }
}

mp_void ReportJobDetailFactory::StopReportMainJobDetail(const mp_string& jobId, const Json::Value& detail)
{
    AppProtect::SubJobDetails jobInfo;
    JsonToStruct(detail, jobInfo);
    if (jobInfo.jobStatus != AppProtect::SubJobStatus::COMPLETED &&
        jobInfo.jobStatus != AppProtect::SubJobStatus::FAILED) {
        return;
    }
    // 主任务上报成功或失败后停止定时器上报
    auto jobHandler = AppProtect::AppProtectJobHandler::GetInstance();
    if (jobHandler) {
        auto job = jobHandler->GetRunJobById(jobId, "");
        if (job) {
            job->StopReportJobDetail();
            INFOLOG("Report main job detail success, stop report timer, jobId=%s, jobStatus=%d.",
                jobId.c_str(), jobInfo.jobStatus);
        }
    }
}
}

