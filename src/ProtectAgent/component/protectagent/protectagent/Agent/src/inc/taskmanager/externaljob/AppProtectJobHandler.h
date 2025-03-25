#ifndef _APP_PROTECT_JOB_HANDLER_H
#define _APP_PROTECT_JOB_HANDLER_H

#include <vector>
#include <memory>
#include <mutex>
#include <atomic>
#include <thread>
#include <map>
#include <set>
#include <chrono>
#include <unordered_set>
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "common/Types.h"
#include "common/JsonUtils.h"
#include "taskmanager/externaljob/Job.h"
#include "taskmanager/externaljob/JobStateDB.h"
#include "taskmanager/externaljob/JobPool.h"
#include "taskmanager/externaljob/ClearMountPointsJob.h"
#include "taskmanager/externaljob/JobTimer.h"
#include "servicecenter/timerservice/include/ITimer.h"
#include "servicecenter/timerservice/include/ITimerService.h"
#include "servicecenter/servicefactory/include/ServiceFactory.h"
#include "servicecenter/services/device/RepositoryFactory.h"
namespace AppProtect {

constexpr int FIVE_MINUTES = 5 * 60 * 1000;
class CacheHostIp {
public:
    mp_void SetAgentType(mp_bool isInner)
    {
        m_isDorado = isInner;
    }
    mp_bool UpdateCacheIp(const mp_string& localIp, const mp_string& doradoIp,
        std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps);
    mp_void GetAndUpdateIpList(const std::set<mp_string>& doradoIps, const std::vector<mp_string> &localIps,
        std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps);
    mp_int32 GetIpList(const std::set<mp_string>& doradoIps,
        std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps);
    mp_void ClearAll();
private:
    void Deduplicate(std::vector<mp_string>& ip);
    mp_bool CheckAllStatus();

private:
    std::map<mp_string, std::pair<mp_int32, mp_int32>> m_cacheIpMap;
    std::mutex m_mapMutex;
    std::vector<std::thread> m_threads;
    std::mutex m_thraedMutex;
    mp_int32 m_checkInterval = 600;
    mp_int32 m_lastCleanTime = 0;
    mp_int32 m_cleanInterval = 3600 * 24;
    mp_string m_ipSeparator = "-";
    mp_bool m_isDorado = MP_TRUE;
};

enum class AgentServiceStatus {
    NORMAL,
    LOAD_FULL
};

struct WakeupJobResult {
    uint32_t agentStatus = static_cast<uint32_t>(AgentServiceStatus::NORMAL);
};

class MountPointInfo {
public:
    MountPointInfo(const std::string& jobId)
        : m_jobId(jobId) {}
    mp_void SetMountPoints(const std::vector<mp_string>& vecCacheMountPoints,
        const std::vector<mp_string>& vecNonCacheMountPoints, const bool isFileClientMount)
    {
        m_vecCacheMountPoints = vecCacheMountPoints;
        m_vecNonCacheMountPoints = vecNonCacheMountPoints;
        m_isFileClientMount = isFileClientMount;
    }
    mp_void GetMountPoints(std::vector<mp_string>& vecCacheMountPoints,
        std::vector<mp_string>& vecNonCacheMountPoints, bool& isFileClientMount)
    {
        vecCacheMountPoints = m_vecCacheMountPoints;
        vecNonCacheMountPoints = m_vecNonCacheMountPoints;
        isFileClientMount = m_isFileClientMount;
    }
private:
    mp_string m_jobId;
    std::vector<mp_string> m_vecCacheMountPoints;
    std::vector<mp_string> m_vecNonCacheMountPoints;
    bool m_isFileClientMount{false};
};

class AppProtectJobHandler {
public:
    static AppProtectJobHandler* GetInstance();
    ~AppProtectJobHandler();
    mp_int32 WakeUpJob(const mp_string& taskId, const Json::Value& mainJob,
        WakeupJobResult& result, CResponseMsg &rsp);
    mp_int32 AbortJob(const std::string &mainTaskId, const std::string &subtaskId);
    mp_void AbortJob(const std::string& jobId);
    mp_int32 ReportJobDetails(AppProtect::ActionResult& _return, const AppProtect::SubJobDetails& jobInfo);
    mp_int32 ReportAsyncJobDetails(AppProtect::ActionResult& _return, const std::string &jobId,
        mp_int32 code, const AppProtect::ResourceResultByPage& results);
    mp_int32 GetUbcIpsByMainJobId(const mp_string mainJobId, std::vector<mp_string>& ubcIps);

    std::shared_ptr<Job> GetRunJobById(const mp_string& mainJobId, const mp_string& subJobId) const;
    std::shared_ptr<Job> GetPostJobByMainId(const mp_string& mainJobId) const;
    mp_void NotifyPluginReload(const mp_string& pluginName, const mp_string& newPluginPID);
    std::vector<std::shared_ptr<Job>> GetRunJobs() const;
    mp_void ReportCheckFailed(const PluginJobData& jobData, mp_int32 errCode);
    std::vector<Json::Value> GetMainJobs() const;

    bool IsDataturboOpen(const PluginJobData &data);
    mp_void GetDataTurboPid(mp_string& pid);

    std::vector<mp_string> GetContainerBackendIps()
    {
        return m_containerBackendIps;
    }

    mp_bool GetAgentType()
    {
        return m_isDorado;
    }
#ifdef WIN32
    std::mutex& GetMutexOfMountCifs()
    {
        return m_mutexOfMountCifs;
    }
private:
    mutable std::mutex m_mutexOfMountCifs;
#endif // WIN32

private:
    mp_int32 Run(const MainJobInfoPtr& mainJobInfo);

    mp_int32 StartGettingJobSrv();

    mp_void StopGettingJobSrv();

    mp_void StartRedoProcess();

    void RedoFinishedMainJob(std::shared_ptr<Job> job, const PluginJobData& jobData);

    mp_void RedoJob(const PluginJobData& jobData);

    std::vector<std::shared_ptr<Job>> AcquireRunningJob(const PluginJobData& jobData);

    mp_string GetNodeId();

    mp_int32 GetRotationTime();

    mp_int32 AcquireJob(const MainJobInfoPtr& mainJob, const mp_string& nodeID,
        std::vector<std::shared_ptr<Job>>& jobs);

    mp_int32 Subcribe(std::shared_ptr<Job> job, const mp_string& nodeID, const std::vector<mp_string>& agentIpLists);

    mp_string ComposeUrl(const mp_string& jobID, const mp_string& nodeID, const std::vector<mp_string>& agentIpLists);
    // check agent ip can connected to storageIP
    mp_int32 CheckIpConnection(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp,
        std::vector<mp_string>& validLocalIps, std::vector<mp_string>& validDoradoIps);

    mp_int32 ParseRsp(const mp_string &respParam, std::vector<std::shared_ptr<Job>>& jobs);

    mp_int32 ParseRspDoradoIP(const Json::Value &value, std::multimap<mp_string, std::vector<mp_string>>& doradoIp);

    mp_int32 CheckParam(const Json::Value& value, bool& isSubJob);

    bool HandleRetryJob(const PluginJobData& data);

    mp_void GetMountPoints(const mp_string& mainID, std::vector<mp_string>& vecCacheMountPoints,
    std::vector<mp_string>& vecNonCacheMountPoints, bool& isFileClientMount);

    mp_void Umount(const mp_string& mainID, std::shared_ptr<MountPointInfo> abortMountInfo = nullptr);

    mp_int32 CheckCanBeRunInLocal(std::shared_ptr<Job> job);

    mp_int32 GetRunningJobsCount();

    mp_void AbortUmountProcess(const mp_string& mainID);

    mp_void ExecuteAbortJobForUmount(const AppProtect::SubJobDetails& jobInfo);

    bool CheckSubJobsRunning(MainJobInfoPtr mainJob);

private:
    // inner agent only
    mp_int32 GetDoraDoLanNet(mp_string& net);

    void InnerAgentAdjustJobParam(std::shared_ptr<Job> job);

    bool IsAgentUsingLoopbackIpMount(std::shared_ptr<Job> job);
private:
    bool ReportJobDetailsImpl();

    void CheckAndFreshFailedJobInfo(AppProtect::SubJobDetails& subJobInfo);

    bool ReportHeartBeatImpl();

    void EnableReportHeartBeat();
    void WaitEnableHeartBeat();

    bool IsPluginReportDetailSendToDME(const AppProtect::SubJobDetails& subJobInfo);

    mp_int32 ReportJobDetailsToDME(const AppProtect::SubJobDetails& jobInfo, mp_int32 jobStage);

    bool PreHandleJobReport(const std::shared_ptr<Job>& job, mp_int32 jobStage);
    
    bool HandlerJobReport(const std::shared_ptr<Job>& job, mp_int32 jobStage);
    
    mp_void PostHandlerJobReport(const std::shared_ptr<Job>& job, bool reportResult, mp_int32 jobStage);

    bool ShouldCheckJobReportTimeout(const std::shared_ptr<Job>& job);

    mp_bool redoJobByPid(const std::shared_ptr<Job>& pJob);

#ifdef LINUX
    void AddIpPolicy(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp);
#endif

private:
    AppProtectJobHandler();
    EXTER_ATTACK mp_int32 Initialize();
    mp_int32 InitializeTimer();
    mp_int32 InitializeRunEnvVar();

    mp_int32 AddAcquireInfo(const MainJobInfoPtr mainJob);
    std::vector<MainJobInfoPtr> GetAllAcquireInfo() const;
    mp_void DelAcquireInfo(const MainJobInfoPtr& mainJob);
    mp_void UpdateMainJobOverTime(const MainJobInfoPtr& mainJobInfo);
    mp_void DelRunJob(MainJobInfoPtr mainJob);
    EXTER_ATTACK mp_int32 JudgeMainJobCnt();
    EXTER_ATTACK mp_int32 JudgeSubJobCnt(const mp_string &appType);
    bool IsNasLiveMountJob(std::shared_ptr<Job> job);
    mp_int32 ParseValidLocalIps(std::shared_ptr<Job> job, std::vector<mp_string>& validLocalIps);
    mp_void CheckAndSubcribeJobs(const std::vector<std::shared_ptr<Job>>& jobs);
    mp_void UpdateJobAgentSize(std::shared_ptr<Job> job);

    Json::Value ParseValidAgentsJson(std::shared_ptr<Job> job);
    mp_int32 AllowSubcribe(std::shared_ptr<Job> job);

    void ReloadPluginConfigInfo();

    mp_int32 AddMainJobInfo(const Json::Value mainJob);
    mp_void DelMainJobInfo(const mp_string& mainJobId);
    void EndJob(const MainJobInfoPtr& mainJobInfo);

    void ClearJobInMemory(const MainJobInfoPtr& mainJobInfo);

private:
    std::unique_ptr<JobPool> m_jobPool;

    static std::mutex m_jobHandlerMutex;
    static AppProtectJobHandler* m_appProtectHandler;

    mutable std::mutex m_mutexOfAcquireInfo;
    std::vector<MainJobInfoPtr> m_vecAcquireInfo;

    mutable std::mutex m_mutexOfRunJob;
    std::vector<std::shared_ptr<Job>> m_runJobs;

    std::mutex m_mutexOFAbortJob;
    // 外层key值为主任务id，内层key值为子任务id
    std::map<mp_string, std::pair<
        std::map<mp_string, std::shared_ptr<Job>>, std::shared_ptr<MountPointInfo>>> m_abortJobs;

    std::atomic<bool> m_started;
    std::unique_ptr<std::thread> m_thread;

    std::shared_ptr<timerservice::ITimer> m_heartBeatTimer;
    std::condition_variable m_heartBeatCond;
    std::mutex m_heartBeatMutex;
    std::atomic<bool> m_enableHeartBeat {false};

    std::shared_ptr<timerservice::ITimer> m_timer;
    int32_t m_timeId;
    int32_t m_heatBeatID;
    CacheHostIp m_ipCacheObj;
    std::unique_ptr<ClearMountPointsJob> m_clearMountPoints;
    JobTimer m_mainJobTimer{600};
    JobTimer m_subJobTimer{600};
    std::shared_ptr<RepositoryFactory> m_createRepositoryHandler;
    mp_string m_nodeId;
    mp_string m_deployType;
    mp_bool m_isDorado = MP_TRUE;
    mp_string m_netplanInfo;
    std::vector<mp_string> m_containerBackendIps;

    mutable std::mutex m_mutexOfMainJobs;
    std::vector<Json::Value> m_mainJobs;

    bool m_acquireBusy {false};
};
}  // namespace AppProtect

#endif
