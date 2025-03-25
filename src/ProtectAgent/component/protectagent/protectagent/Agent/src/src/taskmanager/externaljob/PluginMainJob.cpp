#include <memory>
#include <algorithm>
#ifndef WIN32
#include <unistd.h>
#include <fcntl.h>
#endif
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "common/Uuid.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "securecom/RootCaller.h"
#include "message/curlclient/DmeRestClient.h"
#include "message/curlclient/RestClientCommon.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "pluginfx/ExternalPlugin.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "taskmanager/externaljob/PluginLogBackup.h"
#include "taskmanager/externaljob/PluginAnyTimeRestore.h"
#include "taskmanager/externaljob/PluginMainJob.h"

namespace AppProtect {
namespace {
    constexpr int32_t MAIN_JOB_REPORT_INTERVAL = 1;
    constexpr int32_t INC_TO_FULL_TETRY_TIMES = 3;
    constexpr int32_t INC_TO_FULL_TETRY_INTERVAL = 3 * 1000;
    constexpr int32_t INC_TO_DIFF_TETRY_TIMES = 3;
    constexpr int32_t INC_TO_DIFF_TETRY_INTERVAL = 3 * 1000;
    constexpr int32_t BANDWIDTH_MIN = 1; // unit：MB/s
    static std::map<std::pair<PluginMainJob::ActionEvent, PluginMainJob::EventResult>, int32_t> ReportMap = {
        {{PluginMainJob::ActionEvent::MOUNT_NAS, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::CHECK_BACKUP_TYPE)},
        {{PluginMainJob::ActionEvent::GENE_POST_JOB, PluginMainJob::EventResult::START},
            static_cast<int32_t>(MainJobState::INITIALIZING)},
        {{PluginMainJob::ActionEvent::GENE_POST_JOB, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::FAILED)},
        {{PluginMainJob::ActionEvent::MAIN_JOB_FINISHED, PluginMainJob::EventResult::SUCCESS},
            static_cast<int32_t>(MainJobState::COMPLETE)},
        {{PluginMainJob::ActionEvent::MAIN_JOB_FINISHED, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::FAILED)},
        {{PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB, PluginMainJob::EventResult::EXECUTING},
            static_cast<int32_t>(MainJobState::PRE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::EXEC_PRE_SUBJOB, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::PRE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::EXEC_PRE_SCRIPTR, PluginMainJob::EventResult::SUCCESS},
            static_cast<int32_t>(MainJobState::PRE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::EXEC_PRE_SCRIPTR, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::PRE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB, PluginMainJob::EventResult::EXECUTING},
            static_cast<int32_t>(MainJobState::GENERATE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::EXEC_GENE_SUBJOB, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::GENERATE_JOB_RUNNING)},
        {{PluginMainJob::ActionEvent::CONFIG_QOS, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::CHECK_BACKUP_TYPE)},
        {{PluginMainJob::ActionEvent::PLUGIN_LOG_BACKUP, PluginMainJob::EventResult::FAILED},
            static_cast<int32_t>(MainJobState::CHECK_BACKUP_TYPE)}};
}  // namespace

PluginMainJob::PluginMainJob(const PluginJobData& data) : Job(data)
{}

PluginMainJob::~PluginMainJob()
{}

mp_int32 PluginMainJob::Initialize()
{
    LOGGUARD("");
    mp_int32 ret = Job::Initialize();
    if (ret != MP_SUCCESS) {
        if (ret == ERROR_COMMON_INVALID_PARAM) {
            ERRLOG("Failed to initialize job, jobId=%s.", m_data.mainID.c_str());
            AppProtect::AppProtectJobHandler::GetInstance()->ReportCheckFailed(m_data, ERROR_COMMON_INVALID_PARAM);
        }
        return MP_FAILED;
    }

    // initial main job state transition
    auto initAction = std::make_shared<mainJobAction>(
        "Initialing", MainJobState::INITIALIZING, MainJobState::FAILED, MainJobState::CHECK_BACKUP_TYPE);
    auto checkAction = std::make_shared<mainJobAction>(
        "checkBackupType", MainJobState::CHECK_BACKUP_TYPE, MainJobState::FAILED, MainJobState::PRE_JOB_RUNNING);
    auto preAction = std::make_shared<mainJobAction>(
        "ExecPreJob", MainJobState::PRE_JOB_RUNNING, MainJobState::FAILED, MainJobState::GENERATE_JOB_RUNNING);
    auto genAction = std::make_shared<mainJobAction>(
        "ExecGenJob", MainJobState::GENERATE_JOB_RUNNING, MainJobState::FAILED, MainJobState::COMPLETE);

    if (initAction == nullptr || checkAction == nullptr || preAction == nullptr || genAction == nullptr) {
        ERRLOG("make state action failed");
        return MP_FAILED;
    }

    // initial job action function
    initAction->OnTransition = MakeAction([this]() { return GenerateMainJob(); }, ActionEvent::GENE_POST_JOB);
    checkAction->OnEnter = MakeAction([this]() { return MountNas(); }, ActionEvent::MOUNT_NAS);
    checkAction->OnTransition = MakeAction([this]() { return CheckBackupJobType(); }, ActionEvent::CHECK_BACKUP_TYPE);
    checkAction->OnExit = MakeAction([this]() { return SetQosStrategy(); }, ActionEvent::CONFIG_QOS);
    preAction->OnEnter = MakeAction([this]() { return LogBackup(); }, ActionEvent::PLUGIN_LOG_BACKUP);
    preAction->OnTransition = MakeAction([this]() { return ExecutePreScript(); }, ActionEvent::EXEC_PRE_SCRIPTR);
    preAction->OnExit = MakeAction([this]() { return ExecutePreSubJob(); }, ActionEvent::EXEC_PRE_SUBJOB);
    preAction->OnFailed = MakeAction([this]() { return UmountNas(); }, ActionEvent::UNMOUNT_NAS);
    genAction->OnTransition = MakeAction([this]() { return ExecGenerateSubJob(); }, ActionEvent::EXEC_GENE_SUBJOB);
    genAction->OnFailed = MakeAction([this]() { return UmountNas(); }, ActionEvent::UNMOUNT_NAS);

    m_mapStates.insert(std::pair<MainJobState, mainJobActionPtr>(MainJobState::INITIALIZING, initAction));
    m_mapStates.insert(std::pair<MainJobState, mainJobActionPtr>(MainJobState::CHECK_BACKUP_TYPE, checkAction));
    m_mapStates.insert(std::pair<MainJobState, mainJobActionPtr>(MainJobState::PRE_JOB_RUNNING, preAction));
    m_mapStates.insert(std::pair<MainJobState, mainJobActionPtr>(MainJobState::GENERATE_JOB_RUNNING, genAction));
    // intialize abort handler
    m_abortHandleMap.insert(std::make_pair(MainJobState::UNDEFINE,
        std::bind(&PluginMainJob::AbortBeforePreJobComplete, this)));
    m_abortHandleMap.insert(std::make_pair(MainJobState::INITIALIZING,
        std::bind(&PluginMainJob::AbortBeforePreJobComplete, this)));
    m_abortHandleMap.insert(std::make_pair(MainJobState::CHECK_BACKUP_TYPE,
        std::bind(&PluginMainJob::AbortBeforePreJobComplete, this)));
    m_abortHandleMap.insert(std::make_pair(MainJobState::GENERATE_JOB_RUNNING,
        std::bind(&PluginMainJob::NotifyPluginAbort, this)));
    m_abortHandleMap.insert(std::make_pair(MainJobState::PRE_JOB_RUNNING,
        std::bind(&PluginMainJob::NotifyPluginAbort, this)));
    return UpdateStatus(mp_uint32(MainJobState::INITIALIZING));
}

StateAction PluginMainJob::MakeAction(const StateAction& action, ActionEvent event)
{
    return [this, action, event]() {
        BeforeAction(event, EventResult::START, MP_SUCCESS);
        auto ret = action();
        if (ret == MP_SUCCESS) {
            AfterAction(event, EventResult::SUCCESS, ret);
        } else {
            AfterAction(event, EventResult::FAILED, ret);
        }
        return ret;
    };
}

mp_int32 PluginMainJob::BeforeAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    LogBeforeAction(event, result, resultCode);
    ReportBeforeAction(event, result, resultCode);
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::AfterAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    LogAfterAction(event, result, resultCode);
    ReportAfterAction(event, result, resultCode);
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::LogBackup()
{
    if (Job::IsLogBackupJob()) {
        PluginLogBackup walBackupHandler;
        mp_int32 iRet = walBackupHandler.LogBackup(m_data);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Fail to exec main log backup for job: %s", m_data.mainID.c_str());
            return ERROR_INTERNAL;
        }
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::LogBeforeAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    LogJobAction(event, result, resultCode);
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::LogAfterAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    LogJobAction(event, result, resultCode);
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::LogJobAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    static std::map<ActionEvent, std::string> ActionEventMap = {
        {ActionEvent::JOB_START_EXEC, "Main Job initialize"},
        {ActionEvent::GENE_POST_JOB, "Main Job generate post job"},
        {ActionEvent::MOUNT_NAS, "Main Job mount nas"},
        {ActionEvent::CHECK_BACKUP_TYPE, "Main Job check backup type"},
        {ActionEvent::CONFIG_QOS, "Main Job config qos strategy"},
        {ActionEvent::EXEC_PRE_SCRIPTR, "Main Job exectue pre script"},
        {ActionEvent::EXEC_PRE_SUBJOB, "Main Job exectue pre subjob"},
        {ActionEvent::EXEC_GENE_SUBJOB, "Main Job exectue generate subjob"},
        {ActionEvent::UNMOUNT_NAS, "Main Job unmount nas"}};

    static std::map<EventResult, std::string> EventResultMap = {
        {EventResult::START, "start"},
        {EventResult::EXECUTING, "executing"},
        {EventResult::SUCCESS, "success"},
        {EventResult::FAILED, "failed"}};
    
    auto it = ActionEventMap.find(event);
    if (it == ActionEventMap.end()) {
        return MP_SUCCESS;
    }
        
    if (result != EventResult::FAILED) {
        INFOLOG("jobId=%s:%s %s.",
            m_data.mainID.c_str(),
            it->second.c_str(),
            EventResultMap[result].c_str());
    } else {
        ERRLOG("jobId=%s:%s %s errCode:%d.",
            m_data.mainID.c_str(),
            it->second.c_str(),
            EventResultMap[result].c_str(),
            resultCode);
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::ReportAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode, const PluginJobData& data)
{
    INFOLOG("ReportAction jobId=%s: event:%d result:%d resCode:%d.", data.mainID.c_str(), event, result, resultCode);
    if (DetermineWhetherToReportAction(event, result, resultCode, data) == false) {
        INFOLOG("Job no need report.");
        return MP_FAILED;
    }
    std::pair<ActionEvent, EventResult> key{event, result};
    auto it = ReportMap.find(key);
    if (it != ReportMap.end()) {
        ReportJobDetail(event, result, resultCode, data);
    }
    return MP_SUCCESS;
}

bool PluginMainJob::DetermineWhetherToReportAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode, const PluginJobData& data)
{
    // 增量转全量场景，内部已经上报过主任务状态到DME，此处不再上报，以免上报的状态冲突
    if (resultCode == MP_INC_TO_FULL || resultCode == MP_INC_TO_DIFF) {
        return false;
    }

    if (event != ActionEvent::EXEC_PRE_SCRIPTR) {
        return true;
    }

    // 自定义前置脚本为空时，不上报任务label
    if (data.scriptFileName.empty()) {
        INFOLOG("Pre Script not exist, jobId=%s", data.mainID.c_str());
        return false;
    }
 
    INFOLOG("Pre Script(strScriptFileName=%s) is exist, jobId=%s", data.scriptFileName.c_str(), data.mainID.c_str());
    return true;
}

mp_int32 PluginMainJob::ReportBeforeAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    return ReportAction(event, result, resultCode, m_data);
}

mp_int32 PluginMainJob::ReportAfterAction(PluginMainJob::ActionEvent event, PluginMainJob::EventResult result,
    mp_int32 resultCode)
{
    return ReportAction(event, result, resultCode, m_data);
}

mp_int32 PluginMainJob::ReportInSubJob(PluginMainJob::ActionEvent event,
    PluginMainJob::EventResult result, mp_int32 resultCode, const PluginJobData& data)
{
    return ReportAction(event, result, resultCode, data);
}

mp_void PluginMainJob::ReportJobDetail(
    ActionEvent event, EventResult result, mp_int32 resultCode, const PluginJobData& data)
{
    auto detail = ReportJobDetailFactory::GetInstance()->GeneratorMainJobDetail(event, result, resultCode, data);
    int32_t stage = static_cast<int32_t>(MainJobState::INITIALIZING);
    auto it = ReportMap.find({event, result});
    if (it != ReportMap.end()) {
        stage = it->second;
    }
    ReportJobDetailFactory::GetInstance()->SendDetailToDme(detail, stage);
}

Executor PluginMainJob::GetMainJobExec()
{
    return [this](mp_int32) {
        auto ret = ExecMainJob();
        if (MainJobState(m_data.status) == MainJobState::COMPLETE) {
            return MP_SUCCESS;
        } else {
            return ret;
        }
    };
}

mp_int32 PluginMainJob::Exec()
{
    INFOLOG("Start to exec sub job, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    if (m_executor) {
        return m_executor(MP_SUCCESS);
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::ExecMainJob()
{
    mp_int32 iRet = MP_SUCCESS;
    while (MainJobState(m_data.status) != MainJobState::FAILED) {
        if (MainJobState(m_data.status) == MainJobState::COMPLETE) {
            // 增加ID和类型打印
            INFOLOG("jobId=%s execute successfully in Agent scope", m_data.mainID.c_str());
            break;
        }
        // check current job state valid
        mainJobActionMap::iterator iter = m_mapStates.find(MainJobState(m_data.status));
        if (iter == m_mapStates.end()) {
            ERRLOG("main job state map don't contain job state=%d, jobId=%s", m_data.status, m_data.subID.c_str());
            return MP_FAILED;
        }
        // execute state transition
        mainJobActionPtr curAction = iter->second;
        MainJobState curState = curAction->Transition();
        iRet = UpdateStatus(mp_uint32(curState));
        if (iRet != MP_SUCCESS) {
            ERRLOG("UpdateStatus faild");
            break;
        }
        iRet = curAction->GetLastResult();
    }

    return iRet;
}

mp_int32 PluginMainJob::Abort()
{
    INFOLOG("Abort job jobId=%s, current state is %d", m_data.mainID.c_str(), m_data.status);
    auto iter = m_abortHandleMap.find(MainJobState(m_data.status));
    if (iter != m_abortHandleMap.end()) {
        iter->second();
    }
    return MP_SUCCESS;
}

void PluginMainJob::NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails)
{
    INFOLOG("Notify job detail, jobId=%s, subJobId=%s, status=%d, curState=%d",
        jobDetails.jobId.c_str(),
        jobDetails.subJobId.c_str(),
        mp_int32(jobDetails.jobStatus),
        mp_int32(m_data.status));
    for (const auto &it : jobDetails.logDetail) {
        if (it.errorCode == ERROR_PLUGIN_CANNOT_EXEC_ON_AGENT) {
            Json::Value JsDetail;
            StructToJson(it, JsDetail);
            m_data.logDetail.push_back(JsDetail);
            break;
        }
    }
    if (MainJobState(m_data.status) == MainJobState::PRE_JOB_RUNNING && m_pPrepJob != nullptr) {
        m_pPrepJob->NotifyJobDetail(jobDetails);
    } else if (MainJobState(m_data.status) == MainJobState::GENERATE_JOB_RUNNING && m_pGeneJob != nullptr) {
        if (jobDetails.jobStatus == SubJobStatus::type::COMPLETED) {
            m_data.logDetail.clear();
            for (const auto &it : jobDetails.logDetail) {
                Json::Value JsDetail;
                StructToJson(it, JsDetail);
                m_data.logDetail.push_back(JsDetail);
            }
        }
        m_pGeneJob->NotifyJobDetail(jobDetails);
        switch (jobDetails.jobStatus) {
            case SubJobStatus::type::COMPLETED:
                UpdateStatus(mp_uint32(MainJobState::COMPLETE));
                break;
            case SubJobStatus::type::ABORTED:
            case SubJobStatus::type::ABORTED_FAILED:
            case SubJobStatus::type::FAILED:
            case SubJobStatus::type::FAILED_NOTRY:
                UpdateStatus(mp_uint32(MainJobState::FAILED));
                break;
            default:
                break;
        }
    } else if (MainJobState(m_data.status) == MainJobState::ABORTING) {
        if (jobDetails.jobStatus == SubJobStatus::type::ABORTED) {
            UpdateStatus(mp_uint32(MainJobState::COMPLETE));
        }
    }
}

mp_int32 PluginMainJob::GenerateMainJob()
{
    LOGGUARD("");
    SubJob stSubJob;
    stSubJob.jobId = m_data.mainID;
    stSubJob.jobType = SubJobType::type::POST_SUB_JOB;
    stSubJob.jobName = m_data.mainID + "_PostJob";
    stSubJob.jobPriority = 0xFFFF;
    if (!m_data.param["extendInfo"]["multiPostJob"].isNull() &&
        m_data.param["extendInfo"]["multiPostJob"].isString()) {
        mp_string multiPostJob = m_data.param["extendInfo"]["multiPostJob"].asString();
        stSubJob.policy =
            (multiPostJob.compare("true") == 0) ? ExecutePolicy::EVERY_NODE_ONE_TIME : ExecutePolicy::ANY_NODE;
    } else {
        stSubJob.policy = ExecutePolicy::ANY_NODE;
    }
    stSubJob.ignoreFailed = false;
    std::vector<AppProtect::SubJob> jobs;
    jobs.push_back(stSubJob);
    Json::Value jobValue;
    StructToJson(jobs, jobValue);

    // report to DME
    auto dmeClient = DmeRestClient::GetInstance();
    if (dmeClient == nullptr) {
        ERRLOG("get dme rest client faield");
        return MP_FAILED;
    }
    HttpResponse response;
    DmeRestClient::HttpReqParam param("POST",
        "/v1/dme-unified/tasks/sub-tasks", jobValue.toStyledString());
    param.mainJobId = m_data.mainID;
    mp_int32 iRet = dmeClient->SendKeyRequest(param, response);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("add new job failed, jobId=%s, errorCode=%s, errorMessage=%s.",
            m_data.mainID.c_str(),
            errMsg.errorCode.c_str(),
            errMsg.errorMessage.c_str());
        return CMpString::SafeStoi(errMsg.errorCode);
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::ExecutePreScript()
{
    DBGLOG("Begin to exec pre script, jobId=%s", m_data.mainID.c_str());
    mp_string strScriptFileName = "";
    if (m_data.param.isMember(KEY_TASKPARAMS) && m_data.param[KEY_TASKPARAMS].isMember(KEY_SCRIPTS)
        && m_data.param[KEY_TASKPARAMS][KEY_SCRIPTS].isMember(KEY_PRE_SCRIPTS)) {
        CJsonUtils::GetJsonString(m_data.param[KEY_TASKPARAMS][KEY_SCRIPTS], KEY_PRE_SCRIPTS, strScriptFileName);
    }
    if (strScriptFileName.empty()) {
        INFOLOG("PreScript not exist, jobId=%s", m_data.mainID.c_str());
        return MP_SUCCESS;
    }
    m_data.scriptFileName = strScriptFileName;
    mp_int32 iRet = Job::ExecScript(strScriptFileName);
    if (iRet != MP_SUCCESS) {
        return iRet;
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::ExecutePreSubJob()
{
    LOGGUARD("");
    // generate prejob and execute pre sub job
    PluginJobData jobData = {m_data.appType,
        m_data.mainID,
        m_data.mainID + "_pre",
        m_data.param,
        m_data.mainType,
        SubJobType::type::PRE_SUB_JOB};

    m_pPrepJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    if (m_pPrepJob.get() == nullptr) {
        ERRLOG("ExecutePreSubJob faild, pointer is null.");
        return MP_FAILED;
    }
    return m_pPrepJob->Exec();
}

Executor PluginMainJob::GetReportExecutor(ActionEvent event, EventResult result, const PluginJobData& data)
{
    return [event, result, &data](int32_t resultCode) { return ReportInSubJob(event, result, resultCode, data); };
}

mp_int32 PluginMainJob::ExecGenerateSubJob()
{
    LOGGUARD("");
    // generate gen sub job and execute gen sub job
    PluginJobData jobData = {m_data.appType,
        m_data.mainID,
        m_data.mainID + "_gen",
        m_data.param,
        m_data.mainType,
        SubJobType::type::GENERATE_SUB_JOB};
    m_pGeneJob = PluginJobFactory::GetInstance()->CreatePluginJob(jobData);
    if (m_pGeneJob.get() == nullptr) {
        ERRLOG("ExecGenerateSubJob faild, pointer is null.");
        return MP_FAILED;
    }
    return m_pGeneJob->Exec();
}

mp_int32 PluginMainJob::UpdateStatus(mp_uint32 status)
{
    m_data.status = status;
    return JobStateDB::GetInstance().UpdateStatus(m_data.mainID, m_data.subID, m_data.status);
}

mp_int32 PluginMainJob::CanbeRunInLocalNode()
{
    LOGGUARD("");
    DBGLOG("Start to check CanbeRunInLocalNode.");
    ActionResult ret;
    if (m_data.mainType == MainJobType::BACKUP_JOB) {
        BackupJob backupJob;
        SetAgentsToExtendInfo(m_data.param);
        JsonToStruct(m_data.param, backupJob);
        if (IsDataturboOpen(m_data)) {
            mp_int32 iRet = CheckESN();
            if (iRet != MP_SUCCESS) {
                INFOLOG("ESN check failed.");
            }
        }
        ProtectServiceCall(&ProtectServiceIf::AllowBackupInLocalNode, ret, backupJob, BackupLimit::NO_LIMIT);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow backup in local node failed, error=%d",
                m_data.mainID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::RESTORE_JOB) {
        RestoreJob restoreJob;
        JsonToStruct(m_data.param, restoreJob);
        ProtectServiceCall(&ProtectServiceIf::AllowRestoreInLocalNode, ret, restoreJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow restore in local node failed, error=%d",
                m_data.mainID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::CHECK_COPY_JOB) {
        CheckCopyJob checkCopyJob;
        JsonToStruct(m_data.param, checkCopyJob);
        ProtectServiceCall(&ProtectServiceIf::AllowCheckCopyInLocalNode, ret, checkCopyJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("Check jobId=%s can be allow check copy in local node failed, error=%d",
                m_data.mainID.c_str(), ret.code);
        }
    } else if (m_data.mainType == MainJobType::DELETE_COPY_JOB && m_data.appType == "HCSCloudHost") {
        DelCopyJob delCopyJob;
        JsonToStruct(m_data.param, delCopyJob);
        ProtectServiceCall(&ProtectServiceIf::AllowDelCopyInLocalNode, ret, delCopyJob);
        if (ret.code != MP_SUCCESS) {
            ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
            ERRLOG("AllowDelCopyInLocalNode failed, jobId=%s error=%d", m_data.mainID.c_str(), ret.code);
        }
    }
    JobStateDB::GetInstance().UpdateRunEnable(m_data.mainID, ret.code);
    return ret.code;
}

mp_int32 PluginMainJob::CheckESN()
{
    mp_string preESN;
    if (m_data.param.isMember("extendInfo") && m_data.param["extendInfo"].isMember("esn")) {
        preESN = m_data.param["extendInfo"]["esn"].asString();
    } else {
        ERRLOG("ESN is not member of Param.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    if (preESN.empty()) {
        ERRLOG("esn from PM is empty.");
        return ERROR_COMMON_INVALID_PARAM;
    }
    mp_string agentSavedESN;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_BACKUP_ESN, agentSavedESN);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Call CConfigXmlParser function GetValueString failed, ret is %d.", iRet);
        return iRet;
    }
    if (agentSavedESN.empty()) {
        iRet = CConfigXmlParser::GetInstance().SetValue(CFG_BACKUP_SECTION, CFG_BACKUP_ESN, preESN);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Call CConfigXmlParser function SetValue failed, ret is %d.", iRet);
            return iRet;
        }
    } else if (agentSavedESN != preESN) {
        ERRLOG("ESN of PM distributed: %s do not match with agent Saved ESN: %s.",
            preESN.c_str(), agentSavedESN.c_str());
        return ERROR_ESN_MISMATCH;
    }
    DBGLOG("Check ESN success.");
    return MP_SUCCESS;
}
 
bool PluginMainJob::IsDataturboOpen(const PluginJobData &data)
{
    if (data.param.isMember("taskParams") && data.param["taskParams"].isMember("dataLayout")) {
        if (data.param["taskParams"]["dataLayout"].isMember("srcDeduption")) {
            if (data.param["taskParams"]["dataLayout"]["srcDeduption"].asBool()) {
                DBGLOG("The dataturbo switch of backup-job is opened.");
                return true;
            }
        }
    }
    DBGLOG("The dataturbo switch of backup-job is closed.");
    return false;
}
 
void PluginMainJob::GetPermission(AppProtect::JobPermission &jobPermit)
{
    if (!m_data.param.isMember("appInfo") || !m_data.param["appInfo"].isObject()) {
        ERRLOG("Json has no appInfo, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return;
    }
    Application application;
    auto appInfo = m_data.param["appInfo"];
    JsonToStruct(appInfo, application);

    if (!m_data.param.isMember("envInfo") || !m_data.param["envInfo"].isObject()) {
        ERRLOG("Json has no envInfo, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return;
    }
    ApplicationEnvironment appEnv;
    auto envInfo = m_data.param["envInfo"];
    JsonToStruct(envInfo, appEnv);

    ProtectServiceNormalCall(&ProtectServiceIf::QueryJobPermission, jobPermit, appEnv, application);
}

mp_int32 PluginMainJob::CheckBackupJobType()
{
    LOGGUARD("Check backup job type, jobId=%s", m_data.mainID.c_str());
    if (m_data.mainType != MainJobType::BACKUP_JOB) {
        return MP_SUCCESS;  // 仅备份任务
    }

    ActionResult ret;
    BackupJob jobParam;
    SetAgentsToExtendInfo(m_data.param);
    JsonToStruct(m_data.param, jobParam);
    if ((jobParam.jobParam.backupType == BackupJobType::INCREMENT_BACKUP) ||
        (jobParam.jobParam.backupType == BackupJobType::PERMANENT_INCREMENTAL_BACKUP) ||
        (jobParam.jobParam.backupType == BackupJobType::DIFF_BACKUP) ||
        ((jobParam.jobParam.backupType == BackupJobType::LOG_BAKCUP) && IsCheckBackupJobType())) {
        ProtectServiceCall(&ProtectServiceIf::CheckBackupJobType, ret, jobParam);
    }
    if ((ret.code != MP_SUCCESS) && (ret.bodyErr == ERR_INC_TO_FULL || ret.bodyErr == ERR_LOG_TO_FULL)) {
        UmountNas();
        // need switch from increment backup to full backup
        ReportDmeIncToFull();
        StopReportJobDetail();
        SetJobRetry(true);
        return MP_INC_TO_FULL;
    } else if (ret.code != MP_SUCCESS && ret.bodyErr == ERR_INC_TO_DIFF) {
        UmountNas();
        // need switch from increment backup to diff backup
        ReportDmeIncToDiff();
        StopReportJobDetail();
        SetJobRetry(true);
        return MP_INC_TO_DIFF;
    }
    return ret.code;
}

void PluginMainJob::ReportDmeIncToFull()
{
    LOGGUARD("");
    Json::Value detailValue;
    AppProtect::SubJobDetails detailSt;
    detailSt.jobId = m_data.mainID;
    detailSt.jobStatus = SubJobStatus::type::FAILED;
    StructToJson(detailSt, detailValue);
    detailValue["taskStage"] = static_cast<mp_int32>(AppProtect::DmeTaskStage::SWITCH_INCR_TO_FULL);
    detailValue["taskStatus"] = static_cast<mp_int32>(AppProtect::DmeTaskStatus::FAILED);

    std::string context = detailValue.toStyledString();
    // report to DME
    DmeRestClient::HttpReqParam param("PUT", "/v1/dme-unified/tasks/details", context);
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    int retryTimes = INC_TO_FULL_TETRY_TIMES;
    while (retryTimes > 0) {
        mp_int32 ret = DmeRestClient::GetInstance()->SendRequest(param, response);
        if (ret == MP_SUCCESS) {
            return;
        }
        CMpTime::DoSleep(INC_TO_FULL_TETRY_INTERVAL);
        retryTimes--;
    }
    ERRLOG("Notify jobId=%s switch from increment to full failed.", m_data.mainID.c_str());
}

void PluginMainJob::ReportDmeIncToDiff()
{
    LOGGUARD("");
    Json::Value detailValue;
    AppProtect::SubJobDetails detailSt;
    detailSt.jobId = m_data.mainID;
    detailSt.jobStatus = SubJobStatus::type::FAILED;
    StructToJson(detailSt, detailValue);
    detailValue["taskStage"] = static_cast<mp_int32>(AppProtect::DmeTaskStage::SWITCH_INCR_TO_DIFF);
    detailValue["taskStatus"] = static_cast<mp_int32>(AppProtect::DmeTaskStatus::FAILED);

    std::string context = detailValue.toStyledString();
    // report to DME
    DmeRestClient::HttpReqParam param("PUT", "/v1/dme-unified/tasks/details", context);
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    int retryTimes = INC_TO_DIFF_TETRY_TIMES;
    while (retryTimes > 0) {
        mp_int32 ret = DmeRestClient::GetInstance()->SendRequest(param, response);
        if (ret == MP_SUCCESS) {
            return;
        }
        CMpTime::DoSleep(INC_TO_DIFF_TETRY_INTERVAL);
        retryTimes--;
    }
    ERRLOG("Notify jobId=%s switch from increment to diff failed.", m_data.mainID.c_str());
}

void PluginMainJob::StopReportJobDetail()
{
    m_reportFlag = false;
}

bool PluginMainJob::NeedReportJobDetail()
{
    DBGLOG("PluginMainJob NeedReportJobDetail,jobId=%s subJobId=%s, m_reportFlag:%d",
        m_data.mainID.c_str(),
        m_data.subID.c_str(),
        m_reportFlag);
    if (!m_reportFlag) {
        return false;
    }
    if (IsTimeoutLastJobReport()) {
        return true;
    }
    return false;
}

bool PluginMainJob::IsTimeoutLastJobReport()
{
    auto now = std::chrono::steady_clock::now();
    if (now - m_timePoint >= std::chrono::minutes(MAIN_JOB_REPORT_INTERVAL)) {
        return true;
    }
    return false;
}

void PluginMainJob::FetchJobDetail(AppProtect::SubJobDetails& jobDetails)
{
    if (IsTimeoutLastJobReport()) {
        GeneratorMainJobDetail(jobDetails);
    }
}

void PluginMainJob::GeneratorMainJobDetail(AppProtect::SubJobDetails& detail)
{
    detail.jobId = m_data.mainID;
    detail.__set_jobStatus(SubJobStatus::RUNNING);
}

void PluginMainJob::AbortBeforePreJobComplete()
{
    INFOLOG("Job jobId=%s receive abort req under state %d", m_data.mainID.c_str(), m_data.status);
    UpdateStatus(mp_uint32(MainJobState::COMPLETE));
}

void PluginMainJob::NotifyPluginAbort()
{
    // 通知插件停止任务
    mp_int32 ret = SendAbortToPlugin();
    if (ret == MP_SUCCESS) {
        // 表示插件已经收到中止请求，并开始中止任务。Agent等待插件上报中止进度
        UpdateStatus(mp_uint32(MainJobState::ABORTING));
        return;
    }
    // 如果不需要再等插件的回复，直接进入中止完成阶段
    UpdateStatus(mp_uint32(MainJobState::COMPLETE));
}

bool PluginMainJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("Job jobId=%s receive Plugin reload event", m_data.mainID.c_str());

    if (MainJobState(m_data.status) == MainJobState::PRE_JOB_RUNNING && m_pPrepJob != nullptr) {
        m_pPrepJob->NotifyPluginReloadImpl(appType, newPluginPID);
    } else if (MainJobState(m_data.status) == MainJobState::GENERATE_JOB_RUNNING && m_pGeneJob != nullptr) {
        m_pGeneJob->NotifyPluginReloadImpl(appType, newPluginPID);
    }
    return true;
}

mp_int32 PluginMainJob::SetQosStrategy()
{
    LOGGUARD("");
    if (m_data.mainType != MainJobType::BACKUP_JOB) {
        return MP_SUCCESS;  // 仅备份任务
    }
    if (!m_data.param.isMember("taskParams") || !m_data.param["taskParams"].isObject()) {
        ERRLOG("Json has no taskParams, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return ERR_OPERATION_FAILED;
    }
    auto taskParams = m_data.param["taskParams"];
    if (!taskParams.isMember("qos") || !taskParams["qos"].isObject() || taskParams["qos"].isNull()) {
        WARNLOG("Json has no qos, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return MP_SUCCESS;
    }
    auto qos = taskParams["qos"];
    if (!qos.isMember("bandwidth") && !qos["bandwidth"].isInt()) {
        ERRLOG("Json qos have no bandwidth key or value is invalid, jobId=%s, subJobId=%s.", m_data.mainID.c_str(),
            m_data.subID.c_str());
        return ERR_OPERATION_FAILED;
    }
    mp_int32 bandwidth = qos["bandwidth"].asInt();
    BackupJob jobParam;
    JsonToStruct(m_data.param, jobParam);
    mp_int32 dataRepositorieSize = 0;
    for (const auto iter : jobParam.repositories) {
        if (iter.repositoryType == RepositoryDataType::type::DATA_REPOSITORY ||
            iter.repositoryType == RepositoryDataType::type::LOG_REPOSITORY) {
            dataRepositorieSize++;
        }
    }
    mp_int32 averageBandwidth = BANDWIDTH_MIN;
    if (bandwidth != 0 && dataRepositorieSize != 0) {
        averageBandwidth = bandwidth / dataRepositorieSize;
    }
    averageBandwidth = averageBandwidth > BANDWIDTH_MIN ? averageBandwidth : BANDWIDTH_MIN;
    DBGLOG("Bandwidth:%d dataRepositorieSize:%d averageBandwidth:%d", bandwidth, dataRepositorieSize, averageBandwidth);
    qos["bandwidth"] = averageBandwidth;
    Json::Value qosJsonValue;
    qosJsonValue["bandwidthMin"] = BANDWIDTH_MIN;
    qosJsonValue["bandwidthMax"] = averageBandwidth;
    m_data.param["taskParams"]["qos"]["bandwidth"] = averageBandwidth;

    return SendQosRequest(qosJsonValue);
}

mp_int32 PluginMainJob::SendQosRequest(const Json::Value &qosJsonValue)
{
    // report to DME
    auto dmeClient = DmeRestClient::GetInstance();
    if (dmeClient == nullptr) {
        ERRLOG("Get dme rest client faield, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
        return ERR_OPERATION_FAILED;
    }
    mp_string url;
    if (m_data.param["taskId"].isString()) {
        url = "/v1/dme-unified/tasks/qos?task_id=" + m_data.param["taskId"].asString();
    }
    DmeRestClient::HttpReqParam param("PUT", url, qosJsonValue.toStyledString());
    param.mainJobId = m_data.mainID;
    HttpResponse response;
    mp_int32 iRet = dmeClient->SendRequest(param, response);
    mp_int32 statusCode = response.statusCode;
    if ((iRet != MP_SUCCESS) && (statusCode != SC_OK)) {
        ERRLOG("Send url %s faield, ret=%d, statusCode=%d, jobId=%s, subJobId=%s.", url.c_str(), iRet, statusCode,
            m_data.mainID.c_str(), m_data.subID.c_str());
        return ERR_OPERATION_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 PluginMainJob::NotifyPauseJob()
{
    if (MainJobState(m_data.status) == MainJobState::PRE_JOB_RUNNING && m_pPrepJob != nullptr) {
        m_pPrepJob->NotifyPauseJob();
    } else if (MainJobState(m_data.status) == MainJobState::GENERATE_JOB_RUNNING && m_pGeneJob != nullptr) {
        m_pGeneJob->NotifyPauseJob();
    }
    return MP_SUCCESS;
}

mp_bool PluginMainJob::IsCheckBackupJobType()
{
    if (m_data.param["extendInfo"].isMember("isCheckBackupJobType") &&
        m_data.param["extendInfo"]["isCheckBackupJobType"].isString()) {
        mp_string isCheckBackupJobType = m_data.param["extendInfo"]["isCheckBackupJobType"].asString();
        return (isCheckBackupJobType.compare("true") == 0);
    }
    return MP_FALSE;
}
}  // namespace AppProtect
