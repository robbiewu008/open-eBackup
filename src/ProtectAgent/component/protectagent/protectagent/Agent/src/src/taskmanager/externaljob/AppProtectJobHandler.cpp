#include "taskmanager/externaljob/PluginMainJob.h"
#include "taskmanager/externaljob/PluginJobFactory.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "taskmanager/externaljob/ClearMountPointsJob.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/JobTimer.h"
#include "taskmanager/externaljob/PluginSubJob.h"
#include "apps/appprotect/CommonDef.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "message/curlclient/DmeRestClient.h"
#include "message/tcp/CSocket.h"
#include "message/curlclient/RestClientCommon.h"
#include "message/curlclient/PmRestClient.h"
#include "message/curlclient/OSAClient.h"
#include "common/Log.h"
#include "common/Ip.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/JsonHelper.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "securecom/RootCaller.h"
#include "alarm/AlarmMgr.h"
#include "alarm/AlarmCode.h"
#include "common/Utils.h"
#include "pluginfx/ExternalPluginManager.h"
#include "pluginfx/ExternalFileClientManager.h"
#include "host/host.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "servicecenter/timerservice/include/ITimerService.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "taskmanager/filter/RemoteHostFilterFactory.h"
#include "taskmanager/externaljob/AppProtectJobHandler.h"

using namespace servicecenter;
namespace {
constexpr int ZERO = 0;
constexpr int TWO = 2;
constexpr int FOUR = 4;
constexpr int ONE_SECOND = 1;
constexpr int FIVE_SECONDS = 5;
constexpr int TEN_SECONDS = 10;
constexpr int FIFTEEN_SECONDS = 15;
constexpr int TWENTY_SECONDS = 20;
constexpr int JOB_CNT_FIVE = 5;
constexpr int JOB_CNT_TEN = 10;
constexpr int JOB_CNT_TWENTY = 20;
constexpr int JOB_CNT_FORTY = 40;
constexpr int JOB_DETAIL_REPORT_INTERVAL = 60000;
constexpr int JOB_HEARTBEAT_INTERVAL = 60000;
constexpr int ONE_THOUSAND_MILLISECONDS = 1000;
constexpr int MAX_SUBSCRIBE_JOB_CNT = 10;
constexpr int UNINIT_CHECK_RUN_IN_LOCAL = -999;
constexpr int DATATURBO_CHECK_CONNECT_PORT = 12300;
#ifdef WIN32
constexpr int CHECK_SHARE_SERVICE_PORT = 445; // windows cifs端口号默认445
#else
constexpr int CHECK_SHARE_SERVICE_PORT = 111; // nfs端口号默认111
#endif
constexpr int CHECK_TIMEOUT_MILLS = 3000; // socket超时时间3000毫秒
constexpr int MAX_THEADS_NUMBER = 20;
const mp_string NAS_SHARE_APP_TYPE = "NasShare";
const mp_string NAS_FILESYSTEM_APP_TYPE = "NasFileSystem";
const mp_string OBJECT_SET_APP_TYPE = "ObjectSet";
const mp_string OBJECT_STORAGE_APP_TYPE = "ObjectStorage";
}  // namespace

namespace AppProtect {
mp_bool CacheHostIp::UpdateCacheIp(const mp_string& localIp, const mp_string& doradoIp,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    if (CSocket::CheckHostLinkStatus(localIp, doradoIp, CHECK_SHARE_SERVICE_PORT, CHECK_TIMEOUT_MILLS) == MP_SUCCESS) {
        std::lock_guard<std::mutex> lock(m_mapMutex);
        m_cacheIpMap[localIp + m_ipSeparator + doradoIp] = std::make_pair(MP_TRUE, time(0));
        validLocalIps.push_back(localIp);
        validDoradoIps.push_back(doradoIp);
        DBGLOG("Host ip(%s) and ip(%s) is reachable.", localIp.c_str(), doradoIp.c_str());
        return MP_TRUE;
    } else {
        // 如果111端口无法连通，有可能是dataturbo的逻辑端口IP，使用dataturbo的12300端口重新检查
        if (CSocket::CheckHostLinkStatus(localIp, doradoIp, DATATURBO_CHECK_CONNECT_PORT) == MP_SUCCESS) {
            std::lock_guard<std::mutex> lock(m_mapMutex);
            m_cacheIpMap[localIp + m_ipSeparator + doradoIp] = std::make_pair(MP_TRUE, time(0));
            validLocalIps.push_back(localIp);
            validDoradoIps.push_back(doradoIp);
            DBGLOG("Host ip(%s) and ip(%s) is reachable.", localIp.c_str(), doradoIp.c_str());
            return MP_TRUE;
        }
        std::lock_guard<std::mutex> lock(m_mapMutex);
        m_cacheIpMap[localIp + m_ipSeparator + doradoIp] = std::make_pair(MP_FALSE, time(0));
        DBGLOG("Host ip(%s) and ip(%s) is not reachable.", localIp.c_str(), doradoIp.c_str());
        return MP_FALSE;
    }
}

mp_void CacheHostIp::ClearAll()
{
    m_cacheIpMap.clear();
    m_lastCleanTime = time(0);
}

mp_void CacheHostIp::GetAndUpdateIpList(const std::set<mp_string>& doradoIps, const std::vector<mp_string> &localIps,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
     // 多个任务一起更新ip，等待第一个任务更新完后再进入
    std::lock_guard<std::mutex> lock(m_thraedMutex);
    if (abs(time(0) - m_lastCleanTime) > m_cleanInterval ||
        CheckAllStatus() != MP_TRUE) {
        ClearAll();
    }
    std::vector<std::pair<std::string, std::string>> needUpdateIps;
    for (auto localIp: localIps) {
        for (auto doradoIp: doradoIps) {
            mp_bool good = MP_FALSE;
            auto ipKey = localIp + m_ipSeparator + doradoIp;
            auto it = m_cacheIpMap.find(ipKey);
            if (it == m_cacheIpMap.end() || abs(time(0) - it->second.second) > m_checkInterval) {
                needUpdateIps.push_back(std::make_pair(localIp, doradoIp));
            } else if (it->second.first == MP_TRUE) {
                DBGLOG("Ip(%s) is reachable, time eclapse(%d).", ipKey.c_str(), time(0) - it->second.second);
                validLocalIps.push_back(localIp);
                validDoradoIps.push_back(doradoIp);
            } else {
                DBGLOG("Ip(%s) is not reachable, time eclapse(%d).", ipKey.c_str(), time(0) - it->second.second);
            }
        }
    }
    std::pair<std::string, std::string> ipPair;
    for (int index = 0; index < needUpdateIps.size();) {
        ipPair = needUpdateIps[index];
        // 线程数超过20，等待第一个线程结束后，再继续添加新线程
        if (m_threads.size() < MAX_THEADS_NUMBER) {
            m_threads.push_back(std::thread([&, ipPair] {
                UpdateCacheIp(ipPair.first, ipPair.second, std::ref(validLocalIps), std::ref(validDoradoIps));
            }));
            index++;
        } else {
            auto it = m_threads.begin();
            (*it).join();  // 等待线程退出
            m_threads.erase(it);
        }
    }
    for (auto& thr : m_threads) {
        thr.join();  // 等待线程退出
    }
    m_threads.clear();
    if (!validDoradoIps.empty()) {
        return;
    }
    // 避免不存在连通的ip情况下长时间不更新
    ClearAll();
}

mp_bool CacheHostIp::CheckAllStatus()
{
    for (const auto& item: m_cacheIpMap) {
        if (item.second.first == MP_TRUE) {
            return MP_TRUE;
        }
    }
    WARNLOG("No ip can do job clean cache");
    return MP_FALSE;
}

mp_int32 CacheHostIp::GetIpList(const std::set<mp_string>& doradoIps,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    LOGGUARD("");

    // 连通性检查
    std::vector<mp_string> hostIpv4List;
    std::vector<mp_string> hostIpv6List;
    mp_int32 iRet = CIP::GetHostIPList(hostIpv4List, hostIpv6List);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get Host ip failed");
        return iRet;
    }
    std::set<mp_string> doradoIpv4Set;
    std::set<mp_string> doradoIpv6Set;
    for (const auto& ip : doradoIps) {
        if (CIP::IsIPV4(ip)) {
            doradoIpv4Set.insert(ip);
        } else if (CIP::IsIPv6(ip)) {
            doradoIpv6Set.insert(ip);
        } else {
            ERRLOG("Dorado ip(%s) is not ipv4 or ipv6", ip.c_str());
        }
    }
    if (!doradoIpv4Set.empty()) {
        GetAndUpdateIpList(doradoIpv4Set, hostIpv4List, validLocalIps, validDoradoIps);
    }
    if (!doradoIpv6Set.empty()) {
        GetAndUpdateIpList(doradoIpv6Set, hostIpv6List, validLocalIps, validDoradoIps);
    }

    Deduplicate(validDoradoIps);
    Deduplicate(validLocalIps);
    return MP_SUCCESS;
}

void CacheHostIp::Deduplicate(std::vector<mp_string>& ips)
{
    std::set<mp_string> tmp{ips.begin(), ips.end()};
    ips.assign(tmp.begin(), tmp.end());
}

std::mutex AppProtectJobHandler::m_jobHandlerMutex;
AppProtectJobHandler* AppProtectJobHandler::m_appProtectHandler = nullptr;

AppProtectJobHandler* AppProtectJobHandler::GetInstance()
{
    if (m_appProtectHandler == nullptr) {
        std::lock_guard<std::mutex> lock(m_jobHandlerMutex);
        m_appProtectHandler = new (std::nothrow) AppProtectJobHandler();
        if (m_appProtectHandler == nullptr) {
            ERRLOG("new AppProtectJobHandler failed.");
            return nullptr;
        }

        if (m_appProtectHandler->Initialize() != MP_SUCCESS) {
            ERRLOG("initialize AppProtectJobHandler failed.");
            return nullptr;
        }

        return m_appProtectHandler;
    }

    return m_appProtectHandler;
}

AppProtectJobHandler::~AppProtectJobHandler()
{
    m_jobPool->DestoryPool();
    StopGettingJobSrv();
    if (m_timer) {
        m_timer->RemoveTimeoutExecutor(m_timeId);
        m_timer->Stop();
    }

    if (m_heartBeatTimer) {
        m_heartBeatTimer->RemoveTimeoutExecutor(m_heatBeatID);
        m_heartBeatTimer->Stop();
    }
}

mp_int32 AppProtectJobHandler::WakeUpJob(const mp_string& taskId, const Json::Value& mainJobJson,
    WakeupJobResult& result, CResponseMsg &rsp)
{
    INFOLOG("Begin to wakeup job, jobId=%s, receive main job json", taskId.c_str());
    if (!mainJobJson.isObject() || !mainJobJson.isMember(NOTIFY_DMEIPLISTS)) {
        ERRLOG("Param is illegal, jobId=%s.", taskId.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> ipList;
    GET_ARRAY_STRING_WITHOUT_BRACES(mainJobJson[NOTIFY_DMEIPLISTS], ipList);
    mp_string appType;
    GET_JSON_STRING(mainJobJson, "appType", appType);

    MainJobInfoPtr mainJob = std::make_shared<PluginJobData>();
    mainJob->mainID = taskId;
    mainJob->dmeIps = ipList;
    mainJob->appType = appType;

    auto ins = DmeRestClient::GetInstance();
    if (!ins) {
        ERRLOG("DmeRestClient ins is not exist when wakeup job, jobId=%s.", taskId.c_str());
        return MP_FAILED;
    }

    ins->UpdateDmeAddr(taskId, ipList);

    EnableReportHeartBeat();

    if (AddAcquireInfo(mainJob) == MP_FAILED) {
        ERRLOG("Add acquire info fail, jobId=%s.", taskId.c_str());
        rsp.SetHttpStatus(SC_SERVICE_UNAVAILABLE);
        return MP_FAILED;
    }

    if (JudgeSubJobCnt(appType) != MP_SUCCESS) {
        result.agentStatus = static_cast<uint32_t>(AgentServiceStatus::LOAD_FULL);
        INFOLOG("Agent load is full for apptype %s.", appType.c_str());
    }

    INFOLOG("Wakeup job succ, jobId=%s", taskId.c_str());
    return MP_SUCCESS;
}

mp_int32 AppProtectJobHandler::AbortJob(const std::string& mainTaskId, const std::string& subtaskId)
{
    INFOLOG("Begin to abort job, jobId=%s, subJobId=%s", mainTaskId.c_str(), subtaskId.c_str());
    auto runJobs = GetRunJobs();
    // check if there is a running job
    for (const auto& tempJob : runJobs) {
        if ((tempJob->GetData().mainID == mainTaskId) && (tempJob->GetData().subID == subtaskId)) {
            return tempJob->Abort();
        }
    }
    WARNLOG("Abort Job failed, which isn't running, jobId=%s, subJobId=%s.", mainTaskId.c_str(), subtaskId.c_str());
    return ERR_OBJ_NOT_EXIST;
}

mp_void AppProtectJobHandler::AbortJob(const std::string& jobId)
{
    // 考虑到agent无法收到dme下发的abort命令的情况，当agent发现任务不存在时，终止掉该任务下所有正在运行的任务
    INFOLOG("Begin to abort job, jobId=%s.", jobId.c_str());
    std::vector<std::shared_ptr<Job>> tJobs = GetRunJobs();
    std::map<mp_string, std::shared_ptr<Job>> abortJobs;
    for (std::shared_ptr<Job> pJob : tJobs) {
        if (pJob->GetData().mainID == jobId) {
            if (pJob->IsMainJob() && (pJob->GetData().status == mp_uint32(MainJobState::PRE_JOB_RUNNING) ||
                pJob->GetData().status == mp_uint32(MainJobState::GENERATE_JOB_RUNNING))) {
                DBGLOG("Find mainjob waiting to be aborted by id, jobId=%s, subJobId=%s, status=%d.",
                    pJob->GetData().mainID.c_str(), pJob->GetData().subID.c_str(), pJob->GetData().status);
                abortJobs[pJob->GetData().mainID] = pJob;
                pJob->Abort();
            } else if (pJob->GetData().status == mp_uint32(SubJobState::Running)) {
                DBGLOG("Find subjob waiting to be aborted by id, jobId=%s, subJobId=%s, status=%d.",
                    pJob->GetData().mainID.c_str(), pJob->GetData().subID.c_str(), pJob->GetData().status);
                abortJobs[pJob->GetData().subID] = pJob;
                pJob->Abort();
            }
        }
    }
    std::unique_lock<std::mutex> lock(m_mutexOFAbortJob);
    if (!abortJobs.empty() && m_abortJobs.find(jobId) == m_abortJobs.end()) {
        m_abortJobs[jobId] = std::make_pair(abortJobs, nullptr);
    }
}

mp_void AppProtectJobHandler::ExecuteAbortJobForUmount(const AppProtect::SubJobDetails& jobInfo)
{
    bool abortEmpty = false;
    std::shared_ptr<MountPointInfo> mountPointInfo = nullptr;
    {
        std::unique_lock<std::mutex> lock(m_mutexOFAbortJob);
        if (m_abortJobs.find(jobInfo.jobId) == m_abortJobs.end()) {
            DBGLOG("Can not find job in the abortJobs map, jobId=%s subjobId=%s JOBSTATUS=%d.",
                jobInfo.jobId.c_str(), jobInfo.subJobId.c_str(), mp_int32(jobInfo.jobStatus));
            return;
        }
        INFOLOG("Find job in the abortJobs map, jobId=%s subjobId=%s JOBSTATUS=%d.",
            jobInfo.jobId.c_str(), jobInfo.subJobId.c_str(), mp_int32(jobInfo.jobStatus));

        bool jobFinish = false;
        mp_string jobId = jobInfo.subJobId;
        if (jobInfo.subJobId.empty()) {
            jobId = jobInfo.jobId;
        }

        if (jobInfo.jobStatus >= AppProtect::SubJobStatus::COMPLETED &&
            m_abortJobs[jobInfo.jobId].first.find(jobId) != m_abortJobs[jobInfo.jobId].first.end()) {
            INFOLOG("ExecuteAbortJobForUmount erase, jobId=%s subjobId=%s JOBSTATUS=%d.",
                jobInfo.jobId.c_str(), jobInfo.subJobId.c_str(), mp_int32(jobInfo.jobStatus));
            m_abortJobs[jobInfo.jobId].first.erase(jobId);
        }

        abortEmpty = m_abortJobs[jobInfo.jobId].first.empty();
        if (abortEmpty) {
            mountPointInfo = m_abortJobs[jobInfo.jobId].second;
            m_abortJobs.erase(jobInfo.jobId);
        }
    }
    if (abortEmpty) {
        INFOLOG("All subjobs have been aborted, jobId=%s, subjobId=%s, now begin to umount.",
            jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        Umount(jobInfo.jobId, mountPointInfo);
    }
}

mp_int32 AppProtectJobHandler::ReportJobDetails(
    AppProtect::ActionResult& _return, const AppProtect::SubJobDetails& jobInfo)
{
    ExecuteAbortJobForUmount(jobInfo);
    mp_int32 jobStage = MP_FAILED;
    std::shared_ptr<Job> pJob = nullptr;
    {
        pJob = GetRunJobById(jobInfo.jobId, jobInfo.subJobId);
        if (pJob) {
            if (pJob->IsCompleted(pJob->GetData().status) && !PluginSubJob::CheckSubJobCompeleteStatus(jobInfo)) {
                WARNLOG("Change status failed, jobId=%s subjobId=%s.", jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
                _return.code = RPC_ACTION_CHANGE_STATUS_INTERNAL_ERROR;
                return _return.code;
            }
            pJob->NotifyJobDetail(jobInfo);
            _return.code = MP_SUCCESS;
            jobStage = pJob->GetData().status;
        } else {
            ERRLOG("Job can not found, jobId=%s subjobId=%s.", jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
            _return.message = "job can not found in AppProtectJobHandler";
            _return.code = RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
            return _return.code;
        }
    }
    if (pJob->GetData().IsSanClientMount() && jobInfo.jobStatus == AppProtect::SubJobStatus::COMPLETED &&
        pJob->GetData().subType == SubJobType::type::POST_SUB_JOB) {
            Umount(jobInfo.jobId);
    }
    if (!IsPluginReportDetailSendToDME(jobInfo)) {
        INFOLOG("No need to report, jobId=%s, subJobId=%s.", jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        return _return.code;
    }
    AppProtect::SubJobDetails jobInfoNew = jobInfo;
    if (pJob->IsFailed()) {
        if (redoJobByPid(pJob)) {  // 上报子任务失败时，若dataturbo或FileClient的进程号发生变化，重做任务
            return _return.code;
        }
        CheckAndFreshFailedJobInfo(jobInfoNew);
    }
    if (ReportJobDetailsToDME(jobInfoNew, jobStage) != MP_SUCCESS) {
        ERRLOG("Send to DME failed, jobId=%s, subJobId=%s.", jobInfo.jobId.c_str(), jobInfo.subJobId.c_str());
        if (!jobInfo.subJobId.empty()) {
            _return.code = RPC_ACTION_EXECUTIVE_INTERNAL_ERROR;
            _return.message = "send to DME failed";
        }
    } else {
        pJob->UpdateReportTimepoint();
        if (pJob->IsCompleted(jobStage)) {
            pJob->StopReportTiming();
            INFOLOG("SubJob have reported complate status success, main jobId=%s, sub jobId=%s, jobStatus=%d.",
                jobInfo.jobId.c_str(), jobInfo.subJobId.c_str(), mp_int32(jobInfo.jobStatus));
        }
    }
    return _return.code;
}

mp_int32 AppProtectJobHandler::ReportAsyncJobDetails(AppProtect::ActionResult& _return, const std::string &jobId,
    mp_int32 code, const AppProtect::ResourceResultByPage& results)
{
    INFOLOG("Enter the ReportAsyncJobDetails.");
    HttpReqCommonParam req;
    req.method = "POST";
    req.url = "/v2/internal/resources/" + jobId + "/action/update";
    Json::Value body;
    body["id"] = jobId;
    body["code"] = code;
    StructToJson(results, body["results"]);
    req.body = body.toStyledString();
    HttpResponse response;
    DBGLOG("ReportAsyncJobDetails Rest req: %s", req.body.c_str());
    mp_int32 ret = PmRestClient::GetInstance().SendRequest(req, response);
    DBGLOG("The response body is : %s", response.body.c_str());
    Json::Value rspValue;
    CHECK_FAIL_EX(CJsonUtils::ConvertStringtoJson(response.body, rspValue));
    if (ret != MP_SUCCESS) {
        _return.code = rspValue["code"].asInt();
        _return.bodyErr = rspValue["bodyErr"].asInt();
        _return.message = rspValue["message"].asString();
        ERRLOG("Send request for Report Details to PM failed, jobId=%s, ret=%d, code=%d, bodyErr=%d, message=%s.",
            jobId.c_str(), ret, _return.code, _return.bodyErr, _return.message.c_str());
    }
    return _return.code;
}

mp_bool AppProtectJobHandler::redoJobByPid(const std::shared_ptr<Job>& pJob)
{
    if (pJob->IsMainJob()) {
        return MP_FALSE;
    }
    if (IsDataturboOpen(pJob->GetData())) {
        mp_string dataturboPid;
        GetDataTurboPid(dataturboPid);
        if (!dataturboPid.empty() && !pJob->GetData().culDataturboPid.empty() &&
            dataturboPid != pJob->GetData().culDataturboPid) {
            WARNLOG("dataturboPid changed,current is :%s.", dataturboPid.c_str());
            pJob->PauseJob();
            pJob->StopReportTiming();
            return MP_TRUE;
        }
    }
    if (pJob->GetData().IsFileClientMount()) {
        mp_int32 culFileClientPid = ExternalFileClientManager::GetInstance().AcquirePidByProcess();
        mp_int32 savedFileClientPid = ExternalFileClientManager::GetInstance().GetFileClientPid();
        if (culFileClientPid != 0  && culFileClientPid != savedFileClientPid) {
            WARNLOG("FileClientPid changed,current is :%d.", culFileClientPid);
            ExternalFileClientManager::GetInstance().SetFileClientPid(culFileClientPid);
            ExternalFileClientManager::GetInstance().SetCGroupByConfig();
            pJob->PauseJob();
            pJob->StopReportTiming();
            return MP_TRUE;
        }
    }
    return MP_FALSE;
}

void AppProtectJobHandler::CheckAndFreshFailedJobInfo(AppProtect::SubJobDetails& subJobInfo)
{
    if (AppProtect::PrepareFileSystem::IsMountChainGood(subJobInfo.jobId)) {
        INFOLOG("Job mount chain is good, jobId=%s subjobId=%s.",
            subJobInfo.jobId.c_str(), subJobInfo.subJobId.c_str());
        return;
    }
    ERRLOG("Job mount chain is not good, jobId=%s subjobId=%s.", subJobInfo.jobId.c_str(), subJobInfo.subJobId.c_str());

    Json::Value jsonValueOld;
    StructToJson(subJobInfo, jsonValueOld);
    std::string jsonRepStr;
    WipeSensitiveForJsonData(jsonValueOld.toStyledString(), jsonRepStr);
    DBGLOG("Before update: %s.", jsonRepStr.c_str());
    
    AppProtect::LogDetail logDetail;
    logDetail.__set_description(AGENT_ACCESS_REMOTE_STORAGE_FAILED_LABEL);
    logDetail.__set_level(JobLogLevel::type::TASK_LOG_ERROR);
    logDetail.__set_timestamp(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
    std::vector<AppProtect::LogDetail> logDetailList;
    logDetailList.push_back(logDetail);
    subJobInfo.__set_logDetail(logDetailList);

    Json::Value jsonValueNew;
    StructToJson(subJobInfo, jsonValueNew);
    jsonRepStr = "";
    WipeSensitiveForJsonData(jsonValueNew.toStyledString(), jsonRepStr);
    DBGLOG("After update: %s.", jsonRepStr.c_str());
}

AppProtectJobHandler::AppProtectJobHandler() : m_started(false)
{}

bool AppProtectJobHandler::ReportJobDetailsImpl()
{
    LOGGUARD("");
    auto tmpList = GetRunJobs();
    for (const auto& job : tmpList) {
        mp_int32 jobStage = job->GetData().status;
        if (!PreHandleJobReport(job, jobStage)) {
            continue;
        }
        auto result = HandlerJobReport(job, jobStage);
        PostHandlerJobReport(job, result, jobStage);
    }
    return true;
}

void AppProtectJobHandler::EnableReportHeartBeat()
{
    std::unique_lock<std::mutex> lck(m_heartBeatMutex);
    m_enableHeartBeat = true;
    m_heartBeatCond.notify_all();
}

void AppProtectJobHandler::WaitEnableHeartBeat()
{
    std::unique_lock<std::mutex> lck(m_heartBeatMutex);
    while (!m_enableHeartBeat) {
        m_heartBeatCond.wait(lck);
        INFOLOG("Heart beat have been enabled.");
    }
}

bool AppProtectJobHandler::ReportHeartBeatImpl()
{
    LOGGUARD("");
    WaitEnableHeartBeat();

    DmeRestClient::HttpReqParam req;
    req.method = "PUT";
    req.url = "/v1/dme-unified/manage/agents/" + GetNodeId() + "/check";
    HttpResponse response;
    mp_int32 ret = DmeRestClient::GetInstance()->SendRequest(req, response);
    if ((ret == MP_SUCCESS) && (response.statusCode == SC_OK)) {
        INFOLOG("Send heartbeat url(%s) to dme success.", req.url.c_str());
        return true;
    }
    WARNLOG("Send heartbeat url(%s) to dme failed, ret=%d, statusCode=%d.",
        req.url.c_str(), ret, response.statusCode);
    return true;
}

bool AppProtectJobHandler::HandlerJobReport(const std::shared_ptr<Job>& job, mp_int32 jobStage)
{
    AppProtect::SubJobDetails jobInfosToSend;
    job->FetchJobDetail(jobInfosToSend);
    DBGLOG("Report job detail, jobId=%s subJobId=%s jobStage:%d",
        job->GetData().mainID.c_str(),
        job->GetData().subID.c_str(),
        jobStage);
    AppProtect::ActionResult _return;
    _return.code = ReportJobDetailsToDME(jobInfosToSend, jobStage);
    if (_return.code != MP_SUCCESS) {
        // when job report failed in last ten mins,should cancel this job
        ERRLOG("Report main job detail failed, jobId=%s", jobInfosToSend.jobId.c_str());
        return false;
    } else {
        job->UpdateReportTimepoint();
        return true;
    }
}

bool AppProtectJobHandler::PreHandleJobReport(const std::shared_ptr<Job>& job, mp_int32 jobStage)
{
    if (ShouldCheckJobReportTimeout(job)) {
        INFOLOG("Job Check timeout, jobId=%s, subJobId=%s.",
            job->GetData().mainID.c_str(), job->GetData().subID.c_str());
        if (job->IsReportTimeout()) {
            job->PauseJob();
            job->StopReportTiming();
            INFOLOG("Job report time out, pause job now, jobId=%s, subJobId=%s.",
                job->GetData().mainID.c_str(), job->GetData().subID.c_str());
        }
    }

    if (!job->NeedReportJobDetail()) {
        return false;
    }
    return true;
}

bool AppProtectJobHandler::ShouldCheckJobReportTimeout(const std::shared_ptr<Job>& job)
{
    return ((job->IsMainJob() && !job->IsCompleted()) ||
        (!job->IsMainJob() && job->IsStartTimeing()));
}

mp_void AppProtectJobHandler::PostHandlerJobReport(const std::shared_ptr<Job>& job,
    bool reportResult, mp_int32 jobStage)
{
    if (reportResult && (jobStage == mp_uint32(MainJobState::COMPLETE) ||
        jobStage == mp_uint32(MainJobState::FAILED)) && job->IsMainJob()) {
        job->StopReportJobDetail();
        INFOLOG("Job have reported success, stop report job detail, main jobId=%s",
            job->GetData().mainID.c_str());
    }
}

mp_void AppProtectJobHandler::ReportCheckFailed(const PluginJobData& jobData, mp_int32 errCode)
{
    ERRLOG("Job check failed. jobId=%s, subJobId=%s, errCode=%d.",
        jobData.mainID.c_str(), jobData.subID.c_str(), errCode);
    Json::Value detail;
    detail["taskId"] = jobData.mainID;
    detail["subTaskId"] = jobData.subID;
    detail["taskStatus"] = mp_int32(DmeTaskStatus::FAILED);
    detail["taskStage"] = mp_int32(DmeTaskStage::CHECK_FAILED);
    Json::Value jValueNodeID;
    jValueNodeID["nodeId"] = GetNodeId();
    detail["extendInfo"] = std::move(jValueNodeID);
    Json::Value jValueLogDetail;
    jValueLogDetail["logDetail"] = errCode;
    for (mp_string str : jobData.vecErrorParams) {
        jValueLogDetail["logDetailParam"].append(str);
    }
    detail["logDetail"].append(std::move(jValueLogDetail));

    DmeRestClient::HttpReqParam param("PUT",
        "/v1/dme-unified/tasks/details", detail.toStyledString());
    param.mainJobId = jobData.mainID;
    HttpResponse response;
    if (DmeRestClient::GetInstance()->SendRequest(param, response) == MP_SUCCESS &&
        response.statusCode == SC_OK) {
        INFOLOG("ReportCheckFailed success, jobId=%s, subJobId=%s.", jobData.mainID.c_str(), jobData.subID.c_str());
    } else {
        ERRLOG("ReportCheckFailed failed, jobId=%s, subJobId=%s.", jobData.mainID.c_str(), jobData.subID.c_str());
    }
}

bool AppProtectJobHandler::IsPluginReportDetailSendToDME(const AppProtect::SubJobDetails& jobInfo)
{
    // sub job report to dme direct
    if (!jobInfo.subJobId.empty()) {
        return true;
    }

    if (!jobInfo.logDetail.empty()) {
        return true;
    }
    return false;
}

mp_int32 AppProtectJobHandler::ReportJobDetailsToDME(const AppProtect::SubJobDetails& jobInfo, mp_int32 jobStage)
{
    INFOLOG("Report job detail Main jobId=%s subJobId=%s stage:%d",
        jobInfo.jobId.c_str(),
        jobInfo.subJobId.c_str(),
        jobStage);
    return ReportJobDetailFactory::GetInstance()->SendDetailToDme(jobInfo, jobStage);
}

EXTER_ATTACK mp_int32 AppProtectJobHandler::Initialize()
{
    LOGGUARD("");

    InitializeRunEnvVar();

    m_ipCacheObj.SetAgentType(m_isDorado);
    auto dmeIns = DmeRestClient::GetInstance();
    if (dmeIns == nullptr) {
        ERRLOG("Get DmeClient Ins failed.");
        return MP_FAILED;
    }

    if (InitializeTimer() != MP_SUCCESS) {
        ERRLOG("Init Job Timer failed.");
        return MP_FAILED;
    }

    m_jobPool.reset(new (std::nothrow) JobPool());
    if (!m_jobPool) {
        ERRLOG("Init job pool failed.");
        return MP_FAILED;
    }
    mp_int32 cntOfMainJob = JOB_CNT_TWENTY;
    if (m_isDorado) {
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, MAIN_JOB_CNT_MAX_INNER, cntOfMainJob);
    } else {
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, MAIN_JOB_CNT_MAX, cntOfMainJob);
    }
    m_jobPool->CreatePool(cntOfMainJob + JOB_CNT_TEN);
    m_clearMountPoints->ClearMountPointsTimer();
#ifdef WIN32
    m_clearMountPoints->ClearMountPoints();
#endif // WIN32
    StartRedoProcess();
    return StartGettingJobSrv();
}

mp_int32 AppProtectJobHandler::InitializeRunEnvVar()
{
    if (CIP::CheckIsDoradoEnvironment(m_isDorado) != MP_SUCCESS) {
        ERRLOG("Get CheckIsDoradoEnvironment Failed.");
        return MP_FAILED;
    }
    INFOLOG("Current Agent type %d.", m_isDorado);
    
    if (!m_isDorado) {
        return MP_SUCCESS;
    }
    // if is dorado, get the netplane info
    if (GetDoraDoLanNet(m_netplanInfo) != MP_SUCCESS) {
        ERRLOG("Add dorado lan info failed.");
        return MP_FAILED;
    }

    mp_string deployType;
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DEPLOY_TYPE, deployType);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get deploy type failed.");
        return MP_FAILED;
    }
    m_deployType = deployType;
    INFOLOG("Current deploy type is %s.", deployType.c_str());

    return MP_SUCCESS;
}

mp_int32 AppProtectJobHandler::InitializeTimer()
{
    auto service = ServiceFactory::GetInstance()->GetService<timerservice::ITimerService>("ITimerService");
    if (service) {
        m_timer = service->CreateTimer();
        if (m_timer == nullptr) {
            ERRLOG("Init timer failed.");
            return MP_FAILED;
        }
        m_timeId = m_timer->AddTimeoutExecutor(
            [this]() -> bool { return ReportJobDetailsImpl(); }, JOB_DETAIL_REPORT_INTERVAL);
        m_timer->Start();
 
        m_heartBeatTimer = service->CreateTimer();
        if (m_heartBeatTimer == nullptr) {
            ERRLOG("Init heartbeat timer failed.");
            return MP_FAILED;
        }
        m_heatBeatID = m_heartBeatTimer->AddTimeoutExecutor(
            [this]() -> bool { return ReportHeartBeatImpl(); }, JOB_HEARTBEAT_INTERVAL);
        m_heartBeatTimer->Start();
        return MP_SUCCESS;
    }
    return MP_FAILED;
}

mp_int32 AppProtectJobHandler::AddAcquireInfo(const MainJobInfoPtr mainJob)
{
    LOGGUARD("");

    std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
    auto pos = std::find_if(m_vecAcquireInfo.begin(), m_vecAcquireInfo.end(), [&](const MainJobInfoPtr& item) -> bool {
        return mainJob->mainID == item->mainID;
    });
    if (pos != std::end(m_vecAcquireInfo)) {
        INFOLOG("AcquireInfo exists. jobId=%s, cnt: %d.", mainJob->mainID.c_str(), m_vecAcquireInfo.size());
        return MP_SUCCESS;
    }
    if (JudgeMainJobCnt() != MP_SUCCESS) {
        WARNLOG("Too many job running, AddAcquireInfo failed. jobId=%s.", mainJob->mainID.c_str());
        return MP_FAILED;
    }
    m_vecAcquireInfo.push_back(mainJob);
    INFOLOG("AddAcquireInfo suc. %s", mainJob->mainID.c_str());
    return MP_SUCCESS;
}

mp_int32 AppProtectJobHandler::GetUbcIpsByMainJobId(const mp_string mainJobId, std::vector<mp_string> &ubcIps)
{
    mp_int32 iRet = DmeRestClient::GetInstance()->GetDmeAddrByMainId(mainJobId, ubcIps);
    return iRet;
}

std::vector<MainJobInfoPtr> AppProtectJobHandler::GetAllAcquireInfo() const
{
    std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
    return m_vecAcquireInfo;
}

mp_void AppProtectJobHandler::DelAcquireInfo(const MainJobInfoPtr& mainJob)
{
    INFOLOG("Begin to delete main job, jobId=%s", mainJob->mainID.c_str());
    DmeRestClient::GetInstance()->DeleteDmeAddrByMainId(mainJob->mainID);
    std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
    auto pos = std::remove(m_vecAcquireInfo.begin(), m_vecAcquireInfo.end(), mainJob);
    if (pos != std::end(m_vecAcquireInfo)) {
        INFOLOG("Del AcquireInfo, jobID=%s.", mainJob->mainID.c_str());
        m_vecAcquireInfo.erase(pos, m_vecAcquireInfo.end());
    }
}

mp_int32 AppProtectJobHandler::AddMainJobInfo(const Json::Value mainJob)
{
    LOGGUARD("");
    std::lock_guard<std::mutex> lk(m_mutexOfMainJobs);
    auto pos = std::find_if(m_mainJobs.begin(), m_mainJobs.end(), [&](const Json::Value& item) -> bool {
        return mainJob["taskId"].asString() == item["taskId"].asString();
    });
    if (pos != std::end(m_mainJobs)) {
        INFOLOG("MainJobInfo exists. jobId=%s, cnt: %d.", mainJob["taskId"].asString().c_str(), m_mainJobs.size());
        return MP_SUCCESS;
    }
    m_mainJobs.push_back(mainJob);
    INFOLOG("AddMainJobInfo suc. %s", mainJob["taskId"].asString().c_str());
    return MP_SUCCESS;
}

std::vector<Json::Value> AppProtectJobHandler::GetMainJobs() const
{
    std::lock_guard<std::mutex> lk(m_mutexOfMainJobs);
    return m_mainJobs;
}

mp_void AppProtectJobHandler::DelMainJobInfo(const mp_string& mainJobId)
{
    INFOLOG("Begin to delete the main job information, jobId=%s", mainJobId.c_str());
    std::lock_guard<std::mutex> lk(m_mutexOfMainJobs);
    auto pos = std::remove_if(m_mainJobs.begin(), m_mainJobs.end(), [&](const Json::Value& item) -> bool {
        return item && item["taskId"].asString() == mainJobId;
    });
    if (pos != std::end(m_mainJobs)) {
        INFOLOG("Del mainJobInfo, jobID=%s.", mainJobId.c_str());
        m_mainJobs.erase(pos, m_mainJobs.end());
    }
}

std::vector<std::shared_ptr<Job>> AppProtectJobHandler::GetRunJobs() const
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    return m_runJobs;
}

mp_int32 AppProtectJobHandler::GetRunningJobsCount()
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    mp_int32 runJob = 0;
    for (const auto& pJob : m_runJobs) {
        if (pJob->GetData().status == static_cast<mp_uint32>(AppProtect::SubJobState::Running) ||
            pJob->GetData().status == static_cast<mp_uint32>(AppProtect::SubJobState::Aborting) ||
            pJob->GetData().status == static_cast<mp_uint32>(AppProtect::SubJobState::PrepareComplete) ||
            pJob->GetData().status == static_cast<mp_uint32>(AppProtect::SubJobState::UNDEFINE)) {
            runJob++;
        }
    }
    return runJob;
}

mp_void AppProtectJobHandler::DelRunJob(MainJobInfoPtr mainJob)
{
    INFOLOG("Begin to delete sub Job, jobId=%s, subjobId=%s.",  mainJob->mainID.c_str(), mainJob->subID.c_str());
    bool needStopPlugin = true;
    mp_string delObjAppType;     // the apptype of deleted job
    mp_string delObjPluginName;  // the pluginName of deleted job
    {
        std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
        auto iter = m_runJobs.begin();
        for (; iter != m_runJobs.end();) {
            if ((*iter)->GetData().mainID == mainJob->mainID) {
                INFOLOG("Delete job from run jobs, jobId=%s, subJobId=%s.",
                    (*iter)->GetData().mainID.c_str(), (*iter)->GetData().subID.c_str());
                delObjAppType = (*iter)->GetData().appType;
                iter = m_runJobs.erase(iter);
                continue;
            }
            ++iter;
        }

        if (delObjAppType.empty()) {
            WARNLOG("Delete object apptype is empty, jobId=%s.", mainJob->mainID.c_str());
            return;
        }

        mp_int32 ret = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(
            delObjAppType, delObjPluginName);
        if (ret != MP_SUCCESS) {
            WARNLOG("Fail to get plugin name for apptype=%s, jobId=%s, subjobId=%s.",
                delObjAppType.c_str(), mainJob->mainID.c_str(), mainJob->subID.c_str());
            return;
        }

        for (const auto &job : m_runJobs) {
            mp_string runJobPluginName; // the pluginName of other running jobs with the same pluginName
            ret = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(
                job->GetData().appType, runJobPluginName);
            if (ret != MP_SUCCESS) {
                WARNLOG("Fail to get plugin name for apptype %s, jobId=%s, subJobId=%s.",
                    job->GetData().appType.c_str(), job->GetData().mainID.c_str(), job->GetData().subID.c_str());
                continue;
            }
            if (runJobPluginName == delObjPluginName) {
                // There is other job with the same appType which not belong to mainJob running, don't stop plugin
                DBGLOG("Do not stop plugin %s.", delObjAppType.c_str());
                needStopPlugin = false;
                break;
            }
        }
    }

    if (needStopPlugin) {
        ExternalPluginManager::GetInstance().ReleasePlugin(delObjAppType);
    }
}

mp_void AppProtectJobHandler::GetMountPoints(const mp_string& mainID, std::vector<mp_string>& vecCacheMountPoints,
    std::vector<mp_string>& vecNonCacheMountPoints, bool& isFileClientMount)
{
    LOGGUARD("");
    std::set<mp_string> setNonCacheMountPoints;
    std::set<mp_string> setCacheMountPoints;
    {
        std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
        for (const auto& item : m_runJobs) {
            if (!item) {
                continue;
            }
            /* 适配FC链路的卸载 */
            std::multimap<RepositoryDataType::type, std::vector<mp_string>> tmpMountPoints =
                item->GetData().mountPoints;
            if (item->GetData().IsCurAgentFcOn() && !item->GetData().IsSanClientMount()) {
                tmpMountPoints = item->GetData().dtbMountPoints;
            }
            for (const auto& mountPoint : tmpMountPoints) {
                if (item->GetData().mainID == mainID) {
                    isFileClientMount =item->GetData().IsFileClientMount();
                    mountPoint.first == RepositoryDataType::type::CACHE_REPOSITORY
                        ? setCacheMountPoints.insert(mountPoint.second.begin(), mountPoint.second.end())
                        : setNonCacheMountPoints.insert(mountPoint.second.begin(), mountPoint.second.end());
                }
            }
        }
    }

    vecCacheMountPoints.assign(setCacheMountPoints.begin(), setCacheMountPoints.end());
    vecNonCacheMountPoints.assign(setNonCacheMountPoints.begin(), setNonCacheMountPoints.end());
}

mp_void AppProtectJobHandler::Umount(const mp_string& mainID, std::shared_ptr<MountPointInfo> abortMountInfo)
{
    LOGGUARD("");
    std::vector<mp_string> vecCacheMountPoints;
    std::vector<mp_string> vecNonCacheMountPoints;
    bool isFileClientMount = false;
    if (abortMountInfo == nullptr) {
        GetMountPoints(mainID, vecCacheMountPoints, vecNonCacheMountPoints, isFileClientMount);
    } else {
        abortMountInfo->GetMountPoints(vecCacheMountPoints, vecNonCacheMountPoints, isFileClientMount);
        INFOLOG("Umount jobId=%s, cache mount point size=%d, non-cache mount point size=%d, isFileClientMount=%d.",
            mainID.c_str(), vecCacheMountPoints.size(), vecNonCacheMountPoints.size(), isFileClientMount);
    }
    m_createRepositoryHandler = std::make_shared<AppProtect::RepositoryFactory>();
    std::shared_ptr<Repository> pDataRepository =
        m_createRepositoryHandler->CreateRepository(RepositoryDataType::type::DATA_REPOSITORY);
    DBGLOG("Mount datapoint size %d", vecNonCacheMountPoints.size());
    pDataRepository->Umount(vecNonCacheMountPoints, mainID, isFileClientMount);
    std::shared_ptr<Repository> pCacheRepository =
        m_createRepositoryHandler->CreateRepository(RepositoryDataType::type::CACHE_REPOSITORY);
    DBGLOG("Mount cachepoint size %d", vecCacheMountPoints.size());
    pCacheRepository->Umount(vecCacheMountPoints, mainID, isFileClientMount);
}

std::shared_ptr<Job> AppProtectJobHandler::GetRunJobById(const mp_string& mainJobId, const mp_string& subJobId) const
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    for (const auto& item : m_runJobs) {
        if (item && item->GetData().mainID == mainJobId && item->GetData().subID == subJobId) {
            DBGLOG("Find job. jobId=%s, subJobId=%s.", mainJobId.c_str(), subJobId.c_str());
            return item;
        }
    }
    return nullptr;
}

std::shared_ptr<Job> AppProtectJobHandler::GetPostJobByMainId(const mp_string& mainJobId) const
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    for (const auto& item : m_runJobs) {
        if (item && item->GetData().mainID == mainJobId) {
            DBGLOG("Find post job. jobId=%s.", mainJobId.c_str());
            return item;
        }
    }
    return nullptr;
}

mp_int32 AppProtectJobHandler::StartGettingJobSrv()
{
    LOGGUARD("");
    m_started = true;
    m_thread.reset(new (std::nothrow) std::thread([this]() {
        DBGLOG("Start get job srv thread.");
        while (m_started) {
            ReloadPluginConfigInfo();
            auto mainJobInfos = GetAllAcquireInfo();
            for (const auto& item : mainJobInfos) {
                Run(item);
            }
            // default sleep 1 seconds
            SleepForMS(GetRotationTime());
        }
        DBGLOG("End get job srv thread.");
    }));
    if (!m_thread) {
        ERRLOG("Init Get Job thread failed.");
        m_started = false;
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_void AppProtectJobHandler::StopGettingJobSrv()
{
    m_started = false;
    if (m_thread && m_thread->joinable()) {
        m_thread->join();
        m_thread.reset();
    }
}

mp_void AppProtectJobHandler::StartRedoProcess()
{
    LOGGUARD("");
    std::vector<PluginJobData> result;
    mp_int32 iRet = JobStateDB::GetInstance().QueryAllJob(result);
    if (iRet != MP_SUCCESS) {
        ERRLOG("QueryAllJob failed.");
        return;
    }
    INFOLOG("Start Redo Process.");
    std::set<mp_string> setAppType;
    std::map<mp_string, PluginJobData> mapJobData;
    for (const auto& jobData : result) {
        setAppType.insert(jobData.appType);
        if (mapJobData.find(jobData.mainID) == mapJobData.end()) {
            mapJobData.insert(std::make_pair(jobData.mainID, jobData));
        }
        // main job
        if (jobData.subID.empty()) {
            mapJobData[jobData.mainID] = jobData;
        }
    }
    // stop all plugin
    for (auto strAppType : setAppType) {
        INFOLOG("Stop plugin %s.", strAppType.c_str());
        ExternalPluginManager::GetInstance().StopPluginEx(strAppType);
    }
    // redo job
    for (auto iter = mapJobData.begin(); iter != mapJobData.end(); ++iter) {
        RedoJob(iter->second);
    }
}

mp_void AppProtectJobHandler::RedoJob(const PluginJobData& jobData)
{
    DmeRestClient::GetInstance()->UpdateDmeAddr(jobData.mainID, jobData.dmeIps);
    EnableReportHeartBeat();

    std::vector<std::shared_ptr<Job>> pVecJob = AcquireRunningJob(jobData);
    AddAcquireInfo(std::make_shared<PluginJobData>(jobData));
    for (auto pJob : pVecJob) {
        INFOLOG("Redo job, jobId=%s, subJobId=%s, main job state=%d.",
            pJob->GetData().mainID.c_str(), pJob->GetData().subID.c_str(), jobData.status);
        if (m_isDorado == MP_TRUE) {
            InnerAgentAdjustJobParam(pJob);
        }
        if (pJob->IsMainJob() && pJob->IsCompleted()) {
            RedoFinishedMainJob(pJob, jobData);
            INFOLOG("Finised Job not add to jobPool %s.", jobData.mainID.c_str());
            continue;
        }
        if (jobData.status > mp_uint32(MainJobState::INITIALIZING) && pJob->IsMainJob()) {
            if (pJob->UpdateStatus(jobData.status) != MP_SUCCESS || pJob->MountNas() != MP_SUCCESS) {
                ERRLOG("Job prepare redo failed. jobId=%s.", jobData.mainID.c_str());
                continue;
            }
        }
        pJob->FilterRemoteHost(RemoteHostFilterFactory::CreateFilterAction(PORT_TYPE_FILTER), m_isDorado);
        {
            std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
            auto pos = std::find_if(m_runJobs.begin(), m_runJobs.end(), [&](const std::shared_ptr<Job>& item) -> bool {
                return pJob->GetData().mainID == item->GetData().mainID &&
                       pJob->GetData().subID == item->GetData().subID;
            });
            if (pos == std::end(m_runJobs)) {
                m_runJobs.push_back(pJob);
                INFOLOG("Add redo job. jobId=%s, subJobId=%s.",
                    pJob->GetData().mainID.c_str(), pJob->GetData().subID.c_str());
            }
        }
        m_jobPool->PushJob(pJob);
    }
}

void AppProtectJobHandler::RedoFinishedMainJob(std::shared_ptr<Job> job, const PluginJobData& jobData)
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    job->UpdateStatus(jobData.status);
    job->StopReportTiming();
    m_runJobs.push_back(job);
    INFOLOG("Add redo finished job. jobId=%s, subJobId=%s.",
        job->GetData().mainID.c_str(), job->GetData().subID.c_str());
}

std::vector<std::shared_ptr<Job>> AppProtectJobHandler::AcquireRunningJob(const PluginJobData& jobData)
{
    LOGGUARD("");
    std::vector<std::shared_ptr<Job>> jobs;

    HttpResponse response;
    DmeRestClient::HttpReqParam reqParam;
    reqParam.method = mp_string("GET");
    std::ostringstream url;
    if (jobData.appType == NAS_FILESYSTEM_APP_TYPE && m_isDorado) {
        // 内置代理+NAS应用场景下，该请求转向protectengine.dpa.svc.cluster.local
        url << "/v1/internal/dme-unified/tasks/" << jobData.mainID << "?"
            << "node_id=" << GetNodeId() << "&task_status=1&task_num=0";
    } else {
        url << "/v1/dme-unified/tasks/" << jobData.mainID << "?"
            << "node_id=" << GetNodeId() << "&task_status=1&task_num=0";
    }
    reqParam.url = url.str();
    reqParam.mainJobId = jobData.mainID;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("QueryJobFromDME failed, errcode: %d, jobId=%s.", iRet, jobData.mainID.c_str());
        return jobs;
    }
    
    if (response.statusCode != SC_OK) {
        ERRLOG("QueryJobFromDME failed");
    } else {
        ParseRsp(response.body, jobs);
    }
    return jobs;
}

EXTER_ATTACK mp_int32 AppProtectJobHandler::JudgeMainJobCnt()
{
    LOGGUARD("");
    mp_int32 cntOfMainJob = JOB_CNT_TWENTY;
    if (m_isDorado) {
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, MAIN_JOB_CNT_MAX_INNER, cntOfMainJob);
    } else {
        CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION, MAIN_JOB_CNT_MAX, cntOfMainJob);
    }
    mp_int32 nRunMainJob = 0;
    {
        std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
        for (auto pJob : m_runJobs) {
            if (pJob->IsMainJob() && pJob->GetData().status != mp_uint32(MainJobState::FAILED)) {
                nRunMainJob++;
            }
        }
    }
    if (nRunMainJob >= cntOfMainJob) {
        WARNLOG("The cnt of main job is too large. %d.", nRunMainJob);
        if (m_mainJobTimer.IsOverInterval()) {
            AlarmMgr::GetInstance().SendEvent(RUN_EVENT_JOB_LONG_TIME_FULL_LOAD);
        }
        return MP_FAILED;
    }
    m_mainJobTimer.StopTimer();
    return MP_SUCCESS;
}

// 检查Agent子任务是否并行运行达到最大值
EXTER_ATTACK mp_int32 AppProtectJobHandler::JudgeSubJobCnt(const mp_string &appType)
{
    LOGGUARD("");
    mp_int32 iRet = 0;
    mp_int32 maxSubJob = 0;
    // 读取配置
    iRet = ExternalPluginManager::GetInstance().GetParseManager()->GetSubJobCntByAppType(appType, maxSubJob);
    INFOLOG("Read Plugin config, result is %d, appType is %s,maxSubJob is %d.",
            iRet, appType.c_str(), maxSubJob);
    if (iRet != MP_SUCCESS) {
        if (m_isDorado) {
            if (CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION,
                SUB_JOB_CNT_MAX_INNER, maxSubJob) != MP_SUCCESS) {
                WARNLOG("Get frame sub_job_cnt_max_inner failed, use default value: %d.", JOB_CNT_FORTY);
                maxSubJob = JOB_CNT_FORTY;
            }
        } else {
            if (CConfigXmlParser::GetInstance().GetValueInt32(CFG_FRAME_THRIFT_SECTION,
                SUB_JOB_CNT_MAX, maxSubJob) != MP_SUCCESS) {
                WARNLOG("Get frame sub_job_cnt_max failed, use default value: %d.", JOB_CNT_FORTY);
                maxSubJob = JOB_CNT_FORTY;
            }
            WARNLOG("Get sub job cnt max of %s failed, use general sub_job_cnt_max: %d.", appType.c_str(), maxSubJob);
        }
    }
    // 统计
    mp_int32 jobCounts = GetRunningJobsCount();
    if (jobCounts >= maxSubJob) {
        WARNLOG("Too many running sub job. (%d jobs running, limit is %d)", jobCounts, maxSubJob);
        if (m_subJobTimer.IsOverInterval()) {
            AlarmMgr::GetInstance().SendEvent(RUN_EVENT_JOB_LONG_TIME_FULL_LOAD);
        }
        return MP_FAILED;
    }
    m_subJobTimer.StopTimer();
    return MP_SUCCESS;
}

mp_void AppProtectJobHandler::UpdateMainJobOverTime(const MainJobInfoPtr& mainJobInfo)
{
    // when agent restart, the job born time is set with the now
    auto timeCount = std::chrono::steady_clock::now().time_since_epoch().count();
    mainJobInfo->timeBorn = timeCount;
}

mp_int32 AppProtectJobHandler::Run(const MainJobInfoPtr& mainJobInfo)
{
    LOGGUARD("");
    std::vector<std::shared_ptr<Job>> jobs;
    mp_uint32 iRet = AcquireJob(mainJobInfo, GetNodeId(), jobs);
    mainJobInfo->UpdateCurrentAcquireInterval();
    if (iRet != MP_SUCCESS) {
        if (iRet == ERR_OBJ_NOT_EXIST) {
            INFOLOG("When AcquireJob main job, jobId=%s, which has completed.", mainJobInfo->mainID.c_str());
            EndJob(mainJobInfo);
            return iRet;
        }
        if (iRet != MP_UNTREATED) {           // if treated.
            mainJobInfo->UpdateNextAcquireInterval(false);
        }
        // 主任务获取不到子任务或者子任务并发数被占满时 更新主任务超时时间
        if (iRet == ERR_INVALID_PARAM || iRet == ERR_OPERATION_FAILED || iRet == MP_BUSY) {
            UpdateMainJobOverTime(mainJobInfo);
            return iRet;
        }

        if (iRet == ERR_NETWORK_EXCEPTION) {
            mainJobInfo->SetNetworkErrorOccur();
        } else {
            mainJobInfo->SetNoNetworkErrorOccur();
        }
        if (mainJobInfo->IsNetworkLongTimeError()) {
            INFOLOG("Delete job due to network long time error, jobId=%s.", mainJobInfo->mainID.c_str());
            Umount(mainJobInfo->mainID);
            ClearJobInMemory(mainJobInfo);
        }

        // when the main job can't query the subjob for 6h, agent will stop the main job
        // when query sucessfully, only refresh the job generate time in memory
        if (!CheckSubJobsRunning(mainJobInfo) && mainJobInfo->IsOverTime()) {
            INFOLOG("Delete job due to over time, jobId=%s.", mainJobInfo->mainID.c_str());
            ClearJobInMemory(mainJobInfo);
        }
        return iRet;
    }
    mainJobInfo->UpdateNextAcquireInterval(true);
    mainJobInfo->SetNoNetworkErrorOccur();
    UpdateMainJobOverTime(mainJobInfo);
    CheckAndSubcribeJobs(jobs);
    return MP_SUCCESS;
}

mp_void AppProtectJobHandler::AbortUmountProcess(const mp_string& mainID)
{
    if (m_abortJobs.find(mainID) == m_abortJobs.end()) {
        Umount(mainID);
    } else if (m_abortJobs[mainID].second == nullptr) {
        auto abortMountInfo = std::make_shared<MountPointInfo>(mainID);
        std::vector<mp_string> vecCacheMountPoints;
        std::vector<mp_string> vecNonCacheMountPoints;
        bool isFileClientMount = false;
        GetMountPoints(mainID, vecCacheMountPoints, vecNonCacheMountPoints, isFileClientMount);
        INFOLOG("Abort mount datapoint size %d, cachepoint size %d",
            vecNonCacheMountPoints.size(), vecCacheMountPoints.size());
        abortMountInfo->SetMountPoints(vecCacheMountPoints, vecNonCacheMountPoints, isFileClientMount);
        std::unique_lock<std::mutex> lock(m_mutexOFAbortJob);
        m_abortJobs[mainID].second = abortMountInfo;
    }
}

void AppProtectJobHandler::EndJob(const MainJobInfoPtr& mainJobInfo)
{
    auto mainJobInfos = GetMainJobs();
    AbortJob(mainJobInfo->mainID);
    AbortUmountProcess(mainJobInfo->mainID);
    auto pos = std::find_if(mainJobInfos.begin(), mainJobInfos.end(), [&](const Json::Value& item) -> bool {
        return mainJobInfo->mainID == item["taskId"].asString();
    });
    if (pos != std::end(mainJobInfos)) {
        INFOLOG("MainJob is exists in runJobs. jobId=%s, cnt: %d.",
            mainJobInfo->mainID.c_str(),
            mainJobInfos.size());
        ReportJobDetailFactory::GetInstance()->ReportMainAndPostJobInformation(*pos);
    }
    JobStateDB::GetInstance().DeleteRecord(mainJobInfo->mainID);
    ClearJobInMemory(mainJobInfo);
}

bool AppProtectJobHandler::CheckSubJobsRunning(MainJobInfoPtr mainJob)
{
    std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
    auto iter = m_runJobs.begin();
    for (; iter != m_runJobs.end();) {
        if ((*iter)->GetData().mainID == mainJob->mainID && !(*iter)->GetData().subID.empty()) {
            DBGLOG("Check sub job is running, jobId=%s, subJobId=%s.",
                (*iter)->GetData().mainID.c_str(), (*iter)->GetData().subID.c_str());
            return true;
        }
        ++iter;
    }
    return false;
}

void AppProtectJobHandler::ClearJobInMemory(const MainJobInfoPtr& mainJobInfo)
{
    DelAcquireInfo(mainJobInfo);
    DelRunJob(mainJobInfo);
    DelMainJobInfo(mainJobInfo->mainID);
}

bool AppProtectJobHandler::IsNasLiveMountJob(std::shared_ptr<Job> job)
{
    if ((job->GetData().appType == NAS_SHARE_APP_TYPE || job->GetData().appType == NAS_FILESYSTEM_APP_TYPE)
        && job->GetData().mainType == MainJobType::LIVEMOUNT_JOB) {
        INFOLOG("Job is 'NasShare' or 'NasFileSystem' livemount, jobId=%s, subJobId=%s",
            job->GetData().mainID.c_str(), job->GetData().subID.c_str());
        return true;
    }
    INFOLOG("Job is not 'NasShare' or 'NasFileSystem' livemount, jobId=%s, subJobId=%s",
            job->GetData().mainID.c_str(), job->GetData().subID.c_str());
    return false;
}

mp_int32 AppProtectJobHandler::ParseValidLocalIps(std::shared_ptr<Job> job, std::vector<mp_string>& validLocalIps)
{
    mp_string eip;
    CConfigXmlParser::GetInstance().GetValueString(CFG_MOUNT_SECTION, CFG_EIP, eip);
    if (!eip.empty()) {
        validLocalIps.push_back(eip);
        return MP_SUCCESS;
    } else if (job->GetData().appType == "CloudBackupFileSystem" ||
        (job->GetData().IsCurAgentFcOn() && !job->GetData().IsSanClientMount()) || job->GetData().IsFileClientMount()) {
        return MP_SUCCESS;
    }
    std::multimap<mp_string, std::vector<mp_string>> doradoIp;
    mp_int32 iRet = ParseRspDoradoIP(job->GetData().param, doradoIp);
    if (iRet != MP_SUCCESS) {
        ReportCheckFailed(job->GetData(), iRet);
        return MP_FAILED;
    }

    if (m_isDorado == MP_TRUE) {
        mp_string net;
        if (GetDoraDoLanNet(net) != MP_SUCCESS) {
            ERRLOG("Get container backup ips fail.");
            return MP_FAILED;
        }
        if (IsAgentUsingLoopbackIpMount(job)) {
            // if nas livemount, not replace to 127
            if (!IsNasLiveMountJob(job)) {
                validLocalIps = m_containerBackendIps;
            }
        } else {
#ifdef LINUX
            // only inner agent(linux) needs
            AddIpPolicy(doradoIp);
#endif
            std::vector<mp_string> validDoradoIps;
            iRet = CheckIpConnection(doradoIp, validLocalIps, validDoradoIps);
            if (iRet != MP_SUCCESS) {
                validLocalIps = m_containerBackendIps;
            }
            job->FilerUnvalidDoradoIps(validDoradoIps);
        }

        InnerAgentAdjustJobParam(job);
        job->AddDoradoIpToExtendInfo(doradoIp);
    } else {
        std::vector<mp_string> validDoradoIps;
        iRet = CheckIpConnection(doradoIp, validLocalIps, validDoradoIps);
        if (iRet != MP_SUCCESS) {
            ReportCheckFailed(job->GetData(), iRet);
            return MP_FAILED;
        }
        job->FilerUnvalidDoradoIps(validDoradoIps);
    }
    return MP_SUCCESS;
}

#ifdef LINUX
void AppProtectJobHandler::AddIpPolicy(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp)
{
    std::set<mp_string> doradoIps;
    for (const auto &it : doradoIp) {
        for (const auto &ip : it.second) {
            doradoIps.insert(ip);
        }
    }

    for (const auto &ip : doradoIps) {
        DBGLOG("Add ip policy for %s", ip.c_str());
        IpPolicyParams params;
        params.destinationIp = ip;
        OSAClient osaClient;
        int32_t ret = osaClient.ModifyIpPolicy(params);
        if (ret != MP_SUCCESS) {
            WARNLOG("Add ip policy for %s failed, error code %d", ip.c_str(), ret);
            continue;
        }
    }
}
#endif

mp_void AppProtectJobHandler::CheckAndSubcribeJobs(const std::vector<std::shared_ptr<Job>>& jobs)
{
    for (const auto& job : jobs) {
        m_acquireBusy = true;
        if ((job->IsMainJob() && JudgeMainJobCnt() != MP_SUCCESS) ||
            JudgeSubJobCnt(job->GetData().appType) != MP_SUCCESS) {
            WARNLOG("Too many jobs are being executed, jobId=%s, subJobId=%s please wait...",
                job->GetData().mainID.c_str(), job->GetData().subID.c_str());
            break;
        }
        std::vector<mp_string> validLocalIps;
        if (ParseValidLocalIps(job, validLocalIps) != MP_SUCCESS) {
            continue;
        }
        if (job->IsMainJob() && AllowSubcribe(job) != MP_SUCCESS) {
            continue;
        }
        mp_uint32 iRet = CheckCanBeRunInLocal(job);
        if (iRet != MP_SUCCESS) {
            if (iRet != MP_TASK_FAILED_NO_REPORT) {
                ReportCheckFailed(job->GetData(), iRet);
            }
            continue;
        }
        if (Subcribe(job, GetNodeId(), validLocalIps) != MP_SUCCESS) {
            continue;
        }
        job->FilterRemoteHost(RemoteHostFilterFactory::CreateFilterAction(PORT_TYPE_FILTER), m_isDorado);
        {
            std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
            m_runJobs.push_back(job);
            INFOLOG("Add run job. jobId=%s, subJobId=%s.",
                job->GetData().mainID.c_str(), job->GetData().subID.c_str());
        }
        m_jobPool->PushJob(job);
        UpdateJobAgentSize(job);

        if (!job->IsMainJob() && IsDataturboOpen(job->GetData())) {
            mp_string dataturboPid;
            GetDataTurboPid(dataturboPid);
            job->SetDataturboPid(dataturboPid);
        }
        break;
    }
}

mp_void AppProtectJobHandler::GetDataTurboPid(mp_string& pid)
{
#ifdef WIN32
    mp_string strCmd = "cmd.exe /c tasklist | findstr dpc.exe";
    std::vector<mp_string> vecRlt;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.size() == 0) {
        WARNLOG("The system not install dataturbo or the service is not running.");
        SleepForMS(TEN_SECONDS * ONE_THOUSAND_MILLISECONDS);
        iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRlt);
        if (iRet != MP_SUCCESS || vecRlt.size() == 0) {
            WARNLOG("The system not install dataturbo or the service is not running after 10 seconds.");
            return;
        }
    }
    std::stringstream ss(vecRlt[0]);
    mp_string pname;
    ss >> pname;
    ss >> pid;
#else
    mp_string strCmd = "ps -ef | grep /opt/oceanstor/dataturbo/bin/dpc | grep -v grep | awk '{print $2}'";
    std::vector<mp_string> vecRes;
    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecRes);
    if (iRet != MP_SUCCESS || vecRes.size() == 0) {
        WARNLOG("The system not install dataturbo or the service is not running.");
        return;
    }
    pid = vecRes[0];
#endif
    INFOLOG("the current dataturbo pid is:%s.", pid.c_str());
    return;
}

bool AppProtectJobHandler::IsDataturboOpen(const PluginJobData &data)
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

mp_void AppProtectJobHandler::UpdateJobAgentSize(std::shared_ptr<Job> job)
{
    Json::Value jobInfo = job->GetData().param;
    mp_uint32 agentsSize = 0;
    if (jobInfo.isMember("agents") && jobInfo["agents"].isArray()) {
        agentsSize = jobInfo["agents"].size();
    } else {
        WARNLOG("There is no filed agents for mainjob=%s.", job->GetData().mainID.c_str());
        return;
    }
    std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
    for (auto &acquireInfo : m_vecAcquireInfo) {
        if (job->GetData().mainID == acquireInfo->mainID) {
            INFOLOG("Update acquire info for jobId=%s, F: %d.", job->GetData().mainID.c_str(), agentsSize);
            acquireInfo->agentsSize = agentsSize;
            break;
        }
    }
}

mp_int32 AppProtectJobHandler::CheckCanBeRunInLocal(std::shared_ptr<Job> job)
{
    if (!job) {
        return ERR_PLUGIN_AUTHENTICATION_FAILED;
    }
    /*
        当为主任务时，直接从m_vecAcquireInfo中取状态;
        当为子任务时，调用插件接口判断是否可执行。
    */
    if (!job->IsMainJob()) {
        return job->CanbeRunInLocalNode();
    }

    mp_int32 curCanRunStatus = UNINIT_CHECK_RUN_IN_LOCAL;
    {
        std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
        for (const MainJobInfoPtr& info : m_vecAcquireInfo) {
            if (info->mainID == job->GetData().mainID) {
                curCanRunStatus = info->canRunStatus;
                break;
            }
        }
    }
    if (curCanRunStatus == UNINIT_CHECK_RUN_IN_LOCAL) {
        curCanRunStatus = job->CanbeRunInLocalNode();
    }
    {
        std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
        for (MainJobInfoPtr& info : m_vecAcquireInfo) {
            if (info->mainID == job->GetData().mainID) {
                info->canRunStatus = curCanRunStatus;
                break;
            }
        }
    }
    return curCanRunStatus;
}

Json::Value AppProtectJobHandler::ParseValidAgentsJson(std::shared_ptr<Job> job)
{
    std::map<mp_string, Json::Value> mapJsonAgent;
    if (job->GetData().param.isMember("agents") && job->GetData().param["agents"].isArray()) {
        for (Json::ArrayIndex index = 0; index < job->GetData().param["agents"].size(); ++index) {
            Json::Value jAgent = job->GetData().param["agents"][index];
            if (jAgent.isObject() && jAgent.isMember("status") && jAgent["status"].isInt() &&
                jAgent["status"].asInt() == 1 && jAgent["id"].isString()) {
                mapJsonAgent.insert(std::make_pair(jAgent["id"].asString(), jAgent));
            }
        }
    }
    if (job->GetData().param.isMember("failedAgents") && job->GetData().param["failedAgents"].isArray()) {
        for (Json::ArrayIndex index = 0; index < job->GetData().param["failedAgents"].size(); ++index) {
            Json::Value jAgent = job->GetData().param["failedAgents"][index];
            if (jAgent["id"].isString()) {
                mapJsonAgent.erase(jAgent["id"].asString());
            }
        }
    }
    Json::Value jAgents;
    for (auto iter = mapJsonAgent.begin(); iter != mapJsonAgent.end(); ++iter) {
        jAgents["agents"].append(iter->second);
    }
    return std::move(jAgents);
}

mp_int32 AppProtectJobHandler::AllowSubcribe(std::shared_ptr<Job> job)
{
    Json::Value jAgents = ParseValidAgentsJson(job);
    if (!jAgents.isNull() || (jAgents["agents"].size() <= 1)) {
        return MP_SUCCESS;
    }

    DmeRestClient::HttpReqParam reqParam = {"POST",
        "/v1/dme-unified/tasks/statistic", jAgents.toStyledString()};
    reqParam.mainJobId = job->GetData().mainID;
    HttpResponse response;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS || response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        WARNLOG("Query statistic failed, jobId=%s, errorCode=%s, errorMessage=%s.",
            job->GetData().mainID.c_str(), errMsg.errorCode.c_str(), errMsg.errorMessage.c_str());
        return MP_SUCCESS;
    }
    Json::Value rspValue;
    CHECK_FAIL_EX(CJsonUtils::ConvertStringtoJson(response.body, rspValue));
    mp_int32 nLocalJobNum = 0;
    std::vector<mp_int32> vecOtherJobNum;
    for (Json::ArrayIndex index = 0; index < rspValue.size(); ++index) {
        Json::Value jAgent = rspValue[index];
        if (jAgent["id"].isString() && jAgent["id"].asString() == GetNodeId() &&
            jAgent["tasks"]["main"].isInt()) {
            nLocalJobNum = jAgent["tasks"]["main"].asInt();
        } else if (jAgent["tasks"]["main"].isInt()) {
            vecOtherJobNum.push_back(jAgent["tasks"]["main"].asInt());
        }
    }
    if (vecOtherJobNum.empty() || nLocalJobNum <= *min_element(vecOtherJobNum.begin(), vecOtherJobNum.end())) {
        return MP_SUCCESS;
    } else {
        WARNLOG("Too many jobs are being executed, jobId=%s please wait...", job->GetData().mainID.c_str());
        return MP_FAILED;
    }
}

/**
 * the cnt of job less than or equals 5, return jobCnt*2+1 secnds
 * the cnt of job more than 5 and less than 20, return 10+jobCnt/2-1 secnds
 * the cnt of job more than 20, return 20+jobCnt/4 secnds
 * @return mp_int32
 */
mp_int32 AppProtectJobHandler::GetRotationTime()
{
    mp_int32 rotationTime = TWO * ONE_THOUSAND_MILLISECONDS;
    DBGLOG("Current running rotationTime:%d", rotationTime);
    return rotationTime;
}

mp_int32 AppProtectJobHandler::AcquireJob(
    const MainJobInfoPtr& mainJob, const mp_string& nodeID, std::vector<std::shared_ptr<Job>>& jobs)
{
    LOGGUARD("");
    if (!mainJob) {
        return MP_FAILED;
    }
    if (!mainJob->IsNeedTriggerAcquire()) {
        DBGLOG("No need get job now %s.", mainJob->mainID.c_str());
        return MP_UNTREATED;
    }
    if (JudgeSubJobCnt(mainJob->appType) != MP_SUCCESS) {
        WARNLOG("Too many Apptype(%s) jobs are being executed, jobId=%s.",
            mainJob->appType.c_str(), mainJob->mainID.c_str());
        return MP_BUSY;
    }
    std::ostringstream url;
    if (m_isDorado && mainJob->appType == NAS_FILESYSTEM_APP_TYPE) {
        // 内置代理场景下，该请求转向protectengine.dpa.svc.cluster.local
        url << "/v1/internal/dme-unified/tasks/" << mainJob->mainID << "?"
            << "node_id=" << nodeID << "&task_status=0&task_num=" << mainJob->agentsSize;
    } else {
        url << "/v1/dme-unified/tasks/" << mainJob->mainID << "?"
            << "node_id=" << nodeID << "&task_status=0&task_num=" << mainJob->agentsSize;
    }
    HttpResponse response;
    DmeRestClient::HttpReqParam reqParam;
    reqParam.method = mp_string("GET");
    reqParam.url = url.str();
    reqParam.mainJobId = mainJob->mainID;

    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS) {
        if (response.curlErrCode == CURLE_OPERATION_TIMEDOUT || response.curlErrCode == CURLE_COULDNT_CONNECT) {
            iRet = ERR_NETWORK_EXCEPTION;
        }
        ERRLOG("Acquire job failed, errcode: %d, jobId=%s.", iRet, mainJob->mainID.c_str());
        return iRet;
    }

    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        iRet = CMpString::SafeStoi(errMsg.errorCode);
    } else {
        iRet = ParseRsp(response.body, jobs);
    }

    if (iRet != ERR_OBJ_NOT_EXIST && iRet != ERR_OPERATION_FAILED && iRet != MP_SUCCESS) {
        ERRLOG("Acquire job failed, errcode: %d, jobId=%s.", iRet, mainJob->mainID.c_str());
    }
    return iRet;
}

mp_int32 AppProtectJobHandler::Subcribe(
    std::shared_ptr<Job> job, const mp_string& nodeID, const std::vector<mp_string>& agentIpLists)
{
    mp_string jobID = job->IsMainJob() ? job->GetData().mainID : job->GetData().subID;
    mp_string strUrl = ComposeUrl(jobID, nodeID, agentIpLists);
    if (strUrl.empty()) {
        ERRLOG("Subcribe failed, get url failed. jobId=%s.", jobID.c_str());
        return MP_FAILED;
    }

    HttpResponse response;
    DmeRestClient::HttpReqParam reqParam;
    reqParam.method = mp_string("GET");
    reqParam.url = strUrl;
    reqParam.mainJobId = job->GetData().mainID;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(reqParam, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Subcribe failed. jobId=%s.", jobID.c_str());
        return iRet;
    }

    if (response.statusCode != SC_OK) {
        RestClientCommon::RspMsg errMsg;
        RestClientCommon::ConvertStrToRspMsg(response.body, errMsg);
        ERRLOG("Subcribe failed, errorCode=%s, errorMessage=%s, jobId=%s.",
            errMsg.errorCode.c_str(), errMsg.errorMessage.c_str(), jobID.c_str());
        return CMpString::SafeStoi(errMsg.errorCode);
    } else {
        INFOLOG("Subcribe success, jobId=%s.", jobID.c_str());
        return MP_SUCCESS;
    }
}

mp_int32 AppProtectJobHandler::GetDoraDoLanNet(mp_string& net)
{
#ifndef WIN32
    // 普通容器后端挂载IP为127.0.0.1
    m_containerBackendIps.clear();
    m_containerBackendIps.push_back("127.0.0.1");
    net = "127.0.0.0/8";
    
#endif
    return MP_SUCCESS;
}

void AppProtectJobHandler::InnerAgentAdjustJobParam(std::shared_ptr<Job> job)
{
    if (m_isDorado == MP_FALSE) {
        return;
    }

    if (m_deployType == HOST_ENV_DEPLOYTYPE_E6000 ||
        m_deployType == HOST_ENV_DEPLOYTYPE_DATABACKUP) {
        return;
    }

    // In inner agent scenarios, not replacing remoteHost when it is 'NasShare' or 'NasFileSystem' livemount job.
    DBGLOG("Job appType: %s, mainType: %d, jobId: %s, subJobId: %s",
        job->GetData().appType.c_str(),
        static_cast<mp_uint32>(job->GetData().mainType),
        job->GetData().mainID.c_str(), job->GetData().subID.c_str());
    if (IsNasLiveMountJob(job)) {
        INFOLOG("Not replacing remoteHost when it is 'NasShare' or 'NasFileSystem' livemount job.");
        return;
    }

    INFOLOG("Replace remote host for inner agent jobs.");
    job->ReplaceRemoteHost(m_containerBackendIps);
    return;
}

bool AppProtectJobHandler::IsAgentUsingLoopbackIpMount(std::shared_ptr<Job> job)
{
    if (m_isDorado == MP_FALSE) {
        return false;
    }
    if (m_deployType == HOST_ENV_DEPLOYTYPE_E6000 ||
        m_deployType == HOST_ENV_DEPLOYTYPE_DATABACKUP) {
        return false;
    }

    if (job->IsCrossNodeORCifsMount()) {
        return false;
    }

    return true;
}

mp_string AppProtectJobHandler::ComposeUrl(const mp_string& jobID,
    const mp_string& nodeID, const std::vector<mp_string>& agentIpLists)
{
    std::ostringstream url;
    if (agentIpLists.empty()) {
        url << "/v1/dme-unified/tasks/" << jobID << "/lock?"<< "node_id=" << nodeID;
    } else if (m_netplanInfo.empty()) {
        url << "/v1/dme-unified/tasks/" << jobID << "/lock?" << "node_id=" << nodeID << "&agent_ip_list="
            << CMpString::StrJoin(agentIpLists, mp_string(","));
    } else {
        url << "/v1/dme-unified/tasks/" << jobID << "/lock?" << "node_id=" << nodeID << "&agent_ip_list=" <<
            m_netplanInfo << "," << CMpString::StrJoin(agentIpLists, mp_string(","));
    }
    return url.str();
}

mp_int32 AppProtectJobHandler::CheckIpConnection(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp,
    std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps)
{
    LOGGUARD("");
    std::set<mp_string> doradoIps;
    for (const auto &it : doradoIp) {
        for (const auto &ip : it.second) {
            doradoIps.insert(ip);
        }
    }
    m_ipCacheObj.GetIpList(doradoIps, validLocalIps, validDoradoIps);
    std::vector<mp_string> availableHostIp;
    for (const auto& it : doradoIp) {
        if (it.second.empty()) {
            continue;
        }
        for (const auto& ip : it.second) {
            if (std::find(validDoradoIps.begin(), validDoradoIps.end(), ip) != validDoradoIps.end()) {
                availableHostIp.push_back(ip);
            }
        }
    }
    if (availableHostIp.empty()) {
        ERRLOG("Repository have no available ip");
        return ERR_DISCONNECT_STORAGE_NETWORK;
    }

    DBGLOG("Repository have available ip");
    return validLocalIps.empty() ? ERR_DISCONNECT_STORAGE_NETWORK : MP_SUCCESS;
}

mp_string AppProtectJobHandler::GetNodeId()
{
    CHost host;
    mp_int32 iRet = host.GetHostSN(m_nodeId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, iRet %d.", iRet);
    }
    return m_nodeId;
}

mp_int32 AppProtectJobHandler::ParseRspDoradoIP(const Json::Value& value,
    std::multimap<mp_string, std::vector<mp_string>>& doradoIp)
{
    LOGGUARD("");
    mp_int32 taskType = -1;
    GET_JSON_INT32(value, TASKTYPE, taskType);
    std::map<Json::ArrayIndex, std::vector<Json::Value>> mapJsonRep;
    if ((MainJobType)taskType == MainJobType::BACKUP_JOB) {
        std::vector<Json::Value> vecJsonRep;
        CJsonUtils::GetJsonArrayJson(value, "repositories", vecJsonRep);
        mapJsonRep.insert(std::make_pair(0, vecJsonRep));
    } else {
        for (Json::ArrayIndex index = 0; index < value["copies"].size(); ++index) {
            std::vector<Json::Value> vecJsonRep;
            CJsonUtils::GetJsonArrayJson(value["copies"][index], "repositories", vecJsonRep);
            mapJsonRep.insert(std::make_pair(index, vecJsonRep));
        }
    }
    for (auto iter = mapJsonRep.begin(); iter != mapJsonRep.end(); ++iter) {
        for (auto jsonRep : iter->second) {
            StorageRepository stRep;
            JsonToStruct(jsonRep, stRep);
            std::vector<mp_string> hostList;
            hostList.reserve(stRep.remoteHost.size());
            for (auto host : stRep.remoteHost) {
                hostList.emplace_back(host.ip);
            }
            doradoIp.insert(make_pair(stRep.remotePath, hostList));
        }
    }
    mp_bool isAllIpEmpty = MP_TRUE;
    for (const auto &it : doradoIp) {
        if (it.second.empty()) {
            WARNLOG("Repository dorado ip is empty, remotePath=%s.", it.first.c_str());
        } else {
            isAllIpEmpty = MP_FALSE;
        }
    }
    return isAllIpEmpty ? ERR_DISCONNECT_STORAGE_NETWORK : MP_SUCCESS;
}

mp_int32 AppProtectJobHandler::ParseRsp(const mp_string& respParam, std::vector<std::shared_ptr<Job>>& jobs)
{
    LOGGUARD("");
    Json::Value root;
    if (!JsonHelper::JsonStringToJsonValue(respParam, root)) {
        return MP_FAILED;
    }
    std::vector<Json::Value> vecValues;
    if (!root.isArray()) {
        DBGLOG("Response is not array");
        return MP_FAILED;
    }
    GET_ARRAY_JSON(root, vecValues);
    for (const auto& item : vecValues) {
        bool isSubJob = false;
        if (CheckParam(item, isSubJob) != MP_SUCCESS) {
            continue;
        }
        if (!isSubJob) {
            AddMainJobInfo(item);
        }
        
        Application appInfo;
        JsonToStruct(item[APPINFO], appInfo);

        PluginJobData pluginData = {appInfo.subType,
            item[TASKID].asString(),
            isSubJob ? item[SUBTASKID].asString() : mp_string(""),
            item,
            static_cast<MainJobType>(item[TASKTYPE].asInt()),
            isSubJob ? (SubJobType::type)(item[SUBTASKTYPE].asInt()) : SubJobType::type::PRE_SUB_JOB};
        if (!HandleRetryJob(pluginData)) {
            continue;
        }

        {
            std::lock_guard<std::mutex> lk(m_mutexOfAcquireInfo);
            for (auto pInfo : m_vecAcquireInfo) {
                if (pluginData.mainID == pInfo->mainID) {
                    pluginData.dmeIps = pInfo->dmeIps;
                    break;
                }
            }
        }
        auto pJob = PluginJobFactory::GetInstance()->CreatePluginJob(pluginData);
        if (pJob.get() != nullptr && pJob->Initialize() == MP_SUCCESS) {
            jobs.push_back(pJob);
        }
    }
    return MP_SUCCESS;
}

mp_int32 AppProtectJobHandler::CheckParam(const Json::Value& value, bool& isSubJob)
{
    LOGGUARD("");
    isSubJob = false;
    mp_string taskid;
    mp_string subTaskId;
    mp_int32 taskType = -1;
    // check out info of mainjob
    GET_JSON_STRING(value, TASKID, taskid);
    GET_JSON_INT32(value, TASKTYPE, taskType);
    if (taskid.empty()) {
        ERRLOG("Params of mainjob id is empty.");
        return MP_FAILED;
    }

    // Check out info of subjob
    if (value.isMember(SUBTASKID) && value[SUBTASKID].isString()) {
        subTaskId = value[SUBTASKID].asString();
        if (!subTaskId.empty()) {
            isSubJob = true;
        }
    }

    if (isSubJob && (!value.isMember(SUBTASKTYPE) || !value[SUBTASKTYPE].isInt())) {
        ERRLOG("Params of subtask type is illegal. jobId=%s, subJobId=%s.", taskid.c_str(), subTaskId.c_str());
        return MP_FAILED;
    }

    // check out info of appication
    if (!value.isMember(APPINFO) || !value[APPINFO].isObject()) {
        ERRLOG("Params of app info is illegal. jobId=%s, subJobId=%s.", taskid.c_str(), subTaskId.c_str());
        return MP_FAILED;
    }
    DBGLOG("jobId=%s, subJobId=%s.", taskid.c_str(), subTaskId.c_str());
    return MP_SUCCESS;
}

bool AppProtectJobHandler::HandleRetryJob(const PluginJobData& data)
{
    auto mainID = data.mainID;
    auto subjobID = data.subID;
    INFOLOG("handle retry job, jobId=%s, subJobId=%s.", mainID.c_str(), subjobID.c_str());
    auto job = GetRunJobById(mainID, subjobID);
    if (!job) {
        return true;
    }
    if (job->NeedRetry() || job->IsCompleted()) {
        if (job->IsCompleted()) {
            WARNLOG("Job main job ID[%s] and subjob ID[%s] completed, but still acquired by agent, "
                "it was reset by ubc, now remove it from run job list.", mainID.c_str(), subjobID.c_str());
        }
        std::lock_guard<std::mutex> lk(m_mutexOfRunJob);
        m_runJobs.erase(std::remove_if(m_runJobs.begin(), m_runJobs.end(), [&mainID, &subjobID](
            const std::shared_ptr<Job>& item) {
                if (item && item->GetData().mainID == mainID && item->GetData().subID == subjobID) {
                    INFOLOG("Del retry job, main job ID[%s] and subjob ID[%s]", mainID.c_str(), subjobID.c_str());
                    return true;
                }
                return false;
        }),
            m_runJobs.end());
        return true;
    }
    return false;
}

mp_void AppProtectJobHandler::NotifyPluginReload(const mp_string& pluginName, const mp_string& newPluginPID)
{
    INFOLOG("Plugin %s Reload, notify to jobs", pluginName.c_str());
    std::vector<std::shared_ptr<Job>> tJobs = GetRunJobs();
    for (std::shared_ptr<Job> pJob : tJobs) {
        mp_string jobPluginName;
        mp_int32 ret = ExternalPluginManager::GetInstance().GetParseManager()->GetPluginNameByAppType(
            pJob->GetData().appType, jobPluginName);
        if (ret != MP_SUCCESS) {
            WARNLOG("when reload the plugin, failed to get plugin name for apptype=%s.",
                pJob->GetData().appType.c_str());
            continue;
        }
        INFOLOG("The jobpluginName of apptype %s is %s, the pluginName is %s", pJob->GetData().appType.c_str(),
            jobPluginName.c_str(), pluginName.c_str());
        if (jobPluginName != pluginName || pJob->NotifyPluginReloadImpl(pluginName, newPluginPID)) {
            continue;
        }
        // 只有业务和后置子任务调用thrift接口完成后会退出线程池执行，需要重新下发任务池
        pJob->ResetJob();
        m_jobPool->PushJob(pJob);
        INFOLOG("NotifyPluginReload jobId=%s and subJobId=%s redo again",
            pJob->GetData().mainID.c_str(), pJob->GetData().subID.c_str());
    }
}

void AppProtectJobHandler::ReloadPluginConfigInfo()
{
    static std::chrono::time_point<std::chrono::steady_clock> lastUpdateTime = std::chrono::steady_clock::now();
    std::chrono::time_point<std::chrono::steady_clock> nowTime = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(nowTime - lastUpdateTime);
    if (m_acquireBusy && duration.count() > TEN_SECONDS) {
        ExternalPluginManager::GetInstance().GetParseManager()->Init();
        lastUpdateTime = nowTime;
    }
    m_acquireBusy = false;
}
}  // namespace AppProtect
