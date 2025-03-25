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
#ifndef _EXTERNAL_JOB_H
#define _EXTERNAL_JOB_H

#include <chrono>
#include <unordered_map>
#include <stdint.h>
#include <thrift/transport/TTransportException.h>
#include "common/Types.h"
#include "common/ErrorCode.h"
#include "common/Log.h"
#include "host/host.h"
#include "jsoncpp/include/json/json.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"
#include "apps/appprotect/plugininterface/ApplicationProtectFramework_types.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "taskmanager/externaljob/ExecutorBuilder.h"
#include "servicecenter/thriftservice/include/IThriftClient.h"
#include "servicecenter/services/device/PrepareFileSystem.h"
#include "pluginfx/ExternalPluginManager.h"

namespace AppProtect {
static const mp_string KEY_TASKPARAMS = "taskParams";
static const mp_string KEY_SCRIPTS = "scripts";
static const mp_string KEY_PRE_SCRIPTS = "preScript";
static const mp_string KEY_POST_SUCC_SCRIPTS = "postScript";
static const mp_string KEY_POST_FAIL_SCRIPTS = "failPostScript";
static const mp_string STORAGE_TYPE_ARRAY[] = { "meta", "data", "cache", "log", "index" };
static const mp_string SCRIPT_CMD_INJECT_CHARS = "|;&$<>!\n`";
namespace {
constexpr mp_int32 INVOKE_PLUGIN_INTERFACE_TRTRY_TIMES = 3;        // max retry for send to plugin
constexpr mp_int32 INVOKE_PLUGIN_INTERFACE_INTERVALS = 15 * 1000;  // interval for retry send
constexpr mp_int32 PLUGIN_JOB_OVERTIME = 6;                        // 6 hours
// ubc will reset job if no update in 5 min, after 10 min, job will fail.
constexpr mp_int32 NETWIORK_ERROR_IDENTIFY_TIME = 10;              // 10 minute
constexpr mp_int32 ZERO = 0;
constexpr mp_int32 ONE_MINUTE = 1;
constexpr mp_int32 ONE_CNT = 1;
constexpr mp_int32 TWO_MINUTE = 2;
constexpr mp_int32 TWO_CNT = 2;
constexpr mp_int32 THREE_MINUTE = 3;
constexpr mp_int32 THREE_CNT = 3;
}  // namespace

enum class MainJobType {
    UNDEFINE = 0,
    BACKUP_JOB,
    RESTORE_JOB,
    LIVEMOUNT_JOB,
    CANCEL_LIVEMOUNT_JOB,
    BUILD_INDEX_JOB,
    DELETE_COPY_JOB,
    INSTANT_RESTORE_JOB,
    CHECK_COPY_JOB
};

enum class EnvironmentRole {
    /** none */
    NONE = 0,
    /** active node */
    ACTIVE,
    /** standby node */
    STANDBY,
    /** shard node */
    SHARD
};

struct PluginJobData {
    mp_string appType;
    mp_string mainID;
    mp_string subID;
    Json::Value param;
    MainJobType mainType = MainJobType::UNDEFINE;
    SubJobType::type subType = SubJobType::type::PRE_SUB_JOB;
    mp_uint32 status = 0;
    mp_uint32 agentsSize = 0;
    std::multimap<RepositoryDataType::type, std::vector<mp_string>> mountPoints;
    std::vector<mp_string> dmeIps;
    std::vector<Json::Value> logDetail;
    mp_int64 timeBorn = std::chrono::steady_clock::now().time_since_epoch().count();
    mp_int32 canRunStatus = -999;
    std::vector<mp_string> vecErrorParams;
    // dtbMountPoints 参数主要保存FC链路的挂载点信息
    std::multimap<RepositoryDataType::type, std::vector<mp_string>> dtbMountPoints;

    // handle network error
    mp_int64 flagNetErrorOccur = false;
    mp_int64 timeNetErrorStart = std::chrono::steady_clock::now().time_since_epoch().count();

    mp_string scriptFileName;
    mp_string scriptResult;

    // acquire job frenquence control
    uint32_t nextAcquireInterval = 2;
    uint32_t currentAcquireInterval = 0;
    uint32_t acquireAdjustTimes = 0;

    void UpdateNextAcquireInterval(bool acquireSuccess = false);
    void UpdateCurrentAcquireInterval();
    bool IsNeedTriggerAcquire();

    mp_string culDataturboPid;
    // 当前的dataturbo进程id

    bool disbaleCifs = false;

    bool IsOverTime() const
    {
        using namespace std::chrono;
        auto bornTime = steady_clock::time_point(steady_clock::duration(timeBorn));
        return (std::chrono::steady_clock::now() - bornTime) > std::chrono::hours(PLUGIN_JOB_OVERTIME);
    }

    void SetNoNetworkErrorOccur();
    void SetNetworkErrorOccur();
    bool IsNetworkLongTimeError();

    // 判断当前Agent是否开启了FC开关
    mp_bool IsCurAgentFcOn() const;
    mp_bool IsSanClientMount() const;
    mp_bool IsFileClientMount() const;
};
using MainJobInfoPtr = std::shared_ptr<PluginJobData>;
static std::vector<MainJobInfoPtr> m_vecScriptResultInfo;

enum class MainJobState : mp_uint32 {
    UNDEFINE = 0,
    INITIALIZING,
    CHECK_BACKUP_TYPE,
    PRE_JOB_RUNNING,
    GENERATE_JOB_RUNNING,
    ABORTING,
    COMPLETE,
    FAILED
};

struct FilesystemInfo {
    mp_string FilesystemName;
    mp_int64 FilesystemSize;
    mp_string FilesystemMountPath;
    mp_int32 Filesystemtype;
    std::vector<mp_string> mountPoints;
    mp_int32 JobType;
    std::vector<mp_string> BackupCopiesID;
    mp_string LogCopyID;
};

struct LunInfo {
    mp_string Wwpn;
    mp_string LunName;
    mp_string LunId;
    mp_string AgentWwpn;
    mp_string Path;
    mp_string FileioName;
    mp_int64 FilesystemSize;
    std::vector<mp_string> mountPoints;
    mp_string UnidirectionalAuthPwd;
    mp_string Port;
    mp_string JobType;
};

enum class SubJobState : mp_uint32 {
    UNDEFINE = 0,
    PrepareComplete,
    PrepareFailed,
    Running = 10,
    Aborting,
    SubJobComplete,
    SubJobFailed,
};

using FilterAction = std::function<mp_int32(PluginJobData&, mp_bool)>;

class ProtectServiceIf;
class Job : public std::enable_shared_from_this<Job> {
public:
    Job(const PluginJobData& data) : m_data(data)
    {}

    virtual ~Job() = 0;

    virtual mp_int32 Initialize();

    std::shared_ptr<Job> GetPtr()
    {
        return shared_from_this();
    }

    virtual bool IsCompleted();

    bool IsCompleted(mp_int32 jobStage);

    virtual bool IsFailed();

    virtual mp_int32 Exec()
    {
        return m_iRet;
    }

    const PluginJobData& GetData()
    {
        return m_data;
    }

    mp_void ClearLogDetail()
    {
        this->m_data.logDetail.clear();
    }

    virtual mp_int32 UpdateStatus(mp_uint32 status)
    {
        return MP_SUCCESS;
    }

    virtual void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) = 0;

    virtual bool NeedReportJobDetail()
    {
        return false;
    };

    virtual void FetchJobDetail(AppProtect::SubJobDetails& jobDetails) {
    };

    void UpdateReportTimepoint();

    virtual void StartReportTiming();

    void StopReportTiming();

    virtual void StopReportJobDetail()
    {}

    bool IsStartTimeing()
    {
        return m_startTiming;
    }

    virtual bool IsReportTimeout();

    bool IsMainJob()
    {
        return m_data.subID.empty();
    }

    // check the job whether it can be executed in this node
    virtual mp_int32 CanbeRunInLocalNode()
    {
        return MP_SUCCESS;
    }
    virtual mp_int32 Abort()
    {
        return MP_SUCCESS;
    }

    virtual mp_int32 PauseJob();

    // notify job to do someting when job could not commuticate to DME
    virtual mp_int32 NotifyPauseJob()
    {
        return MP_FAILED;
    };

    virtual bool NeedRetry()
    {
        return m_shouldRetry;
    }

    virtual void GetPermission(AppProtect::JobPermission &jobPermit)
    {}

    mp_int32 MountNas();   // mount nas file systerm

    mp_int32 ExecPostScript(const mp_string& scriptNameKey);

    mp_int32 ExecScriptCommon(mp_int32 commandId, const mp_string& scriptName,
        std::vector<mp_string> pvecResul[] = nullptr);
    mp_int32 ReportExcScriptResult(const mp_string& scriptName = "", std::vector<mp_string> pvecResult[] = nullptr);

    mp_int32 ExecScript(const mp_string& scriptName);

    Executor GetMountNasExecutor()
    {
        return [this](int32_t) {
            return MountNas();
        };
    }

    Executor GetUnmountNasExecutor()
    {
        return [this](int32_t) {
            return UmountNas();
        };
    }

    void SetExecutor(const Executor& executor)
    {
        m_executor = executor;
    }

    virtual bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);
    void ResetJob()
    {
        m_pluginPID.clear();
        m_data.status = 0;
    }

    void SetDataturboPid(const mp_string& pid)
    {
        m_data.culDataturboPid=pid;
    }
    void FilerUnvalidDoradoIps(const std::vector<mp_string>& validDoradoIps);
    void FilerUnvalidDoradoIpsEx(Json::Value& jsonRep, const std::vector<mp_string>& validDoradoIps);
    void ReplaceRemoteHost(const std::vector<mp_string>& containerBackendIps);
    void AddDoradoIpToExtendInfo(const std::multimap<mp_string, std::vector<mp_string>> &doradoIp);
    void AddDoradoIpToRepExtendInfo(Json::Value &repo,
        const std::multimap<mp_string, std::vector<mp_string>> &doradoIp);
    void FilterRemoteHost(FilterAction filterAction, mp_bool isInner);

    void SetDisableCifs();
    bool CheckCifsDisable(const Json::Value &jsonRepo);
    bool IsCrossNodeORCifsMount();

public:
    template<typename R, typename T, typename... ARGS1, typename... ARGS2>
    inline R CallPointer(T* ptr, R (T::*f)(ARGS1...), ARGS2 &&... args)
    {
        return (ptr->*f)(std::forward<ARGS2>(args) ...);
    }

    template<typename R, typename T, typename ... ARGS1, typename ... ARGS2>
    inline R Call(std::shared_ptr<T> ptr, R (T::*f)(ARGS1...), ARGS2 &&... args)
    {
        auto t = ptr.get();
        return CallPointer(t, f, std::forward<ARGS2>(args) ...);
    }

    template<typename T, typename ... ARGS1, typename ... ARGS2>
    bool  RetryThriftCall(
        std::shared_ptr<T> ptr, void (T::*MemFunc)(ActionResult& ret, ARGS1 ...), ActionResult& ret, ARGS2 &&... args)
    {
        try {
            Call(ptr, MemFunc, ret, std::forward<ARGS2>(args) ...);
            DBGLOG("Call plugin interface (%s,%s) req success.", m_data.mainID.c_str(), m_data.subID.c_str());
            if (ret.code == RPC_ACTION_EXECUTIVE_INTERNAL_ERROR && ret.bodyErr != 0) {
                ret.code = ret.bodyErr;
                m_data.vecErrorParams = ret.bodyErrParams;
            }
            return true;
        } catch (apache::thrift::transport::TTransportException& ex) {
            ERRLOG("TTransportException. %s Job ID:(%s,%s)", ex.what(), m_data.mainID.c_str(), m_data.subID.c_str());
        } catch (apache::thrift::protocol::TProtocolException& ex) {
            ERRLOG("TProtocolException. %s Job ID:(%s,%s)", ex.what(), m_data.mainID.c_str(), m_data.subID.c_str());
        } catch (apache::thrift::TApplicationException& ex) {
            ERRLOG("TApplicationException. %s Job ID:(%s,%s)", ex.what(), m_data.mainID.c_str(), m_data.subID.c_str());
        } catch (const std::exception& ex) {
            ERRLOG("Standard C++ Exception. %s Job ID:(%s,%s", ex.what(), m_data.mainID.c_str(), m_data.subID.c_str());
        } catch (...) {
            ERRLOG("Unknown exception. Job ID:(%s,%s", m_data.mainID.c_str(), m_data.subID.c_str());
        }
        ret.code = MP_FAILED;
        return false;
    }

    template<typename T, typename ... ARGS1, typename ... ARGS2>
    void ProtectServiceCall(void (T::*MemFunc)(ActionResult& ret, ARGS1...), ActionResult& ret, ARGS2 &&... args)
    {
        int retry_times = INVOKE_PLUGIN_INTERFACE_TRTRY_TIMES;
        ret.code = MP_SUCCESS;
        while (retry_times) {
            auto pClient = GetThriftClient();
            if (!pClient) {
                /*
                * two case here
                * 1.job not send to plugin yet, and not other job using the plugin, no plugin start,
                * not need to notify plugin
                * 2.plugin or frame error,could not get plugin client,we treat as send plugin already
                */
                ret.code = NO_EXTERNAL_PLUGIN_AVAILABLE;
                ERRLOG("Get thrift client failed.");
                return;
            }
            auto protectServiceClient = GetProtectServiceClient(pClient);
            if (!protectServiceClient) {
                ret.code = NO_EXTERNAL_PLUGIN_AVAILABLE;
                ERRLOG("Get Protect service failed.");
                return;
            }
            if (RetryThriftCall(protectServiceClient, MemFunc, ret, std::forward<ARGS2>(args)...)) {
                return;
            }
            INFOLOG("RetryThriftCall retry, notify ExternalPluginManager to check plugin, time:%d", retry_times);
            --retry_times;

            // Nofity ExternalPluginManager to check plugin
            ExternalPluginManager::GetInstance().StartMonitor();
            SleepFor(std::chrono::seconds(1));
        }
        return;
    }

    template<typename T, typename RETV, typename ... ARGS1, typename ... ARGS2>
    void ProtectServiceNormalCall(void (T::*MemFunc)(RETV& ret, ARGS1...),
        RETV& ret, ARGS2 &&... args)
    {
        auto pClient = GetThriftClient();
        if (!pClient) {
            /*
            * two case here
            * 1.job not send to plugin yet, and not other job using the plugin, no plugin start,
            * not need to notify plugin
            * 2.plugin or frame error,could not get plugin client,we treat as send plugin already
            */
            ERRLOG("Get thrift client failed.");
            return;
        }
        
        try {
            std::shared_ptr<ProtectServiceIf> client = std::dynamic_pointer_cast<ProtectServiceIf>(
                pClient->GetConcurrentClientIf<ProtectServiceConcurrentClient>("ProtectService"));
            if (!client) {
                ERRLOG("Get Protect service failed.");
                return;
            }
            Call(client, MemFunc, ret, std::forward<ARGS2>(args)...);
        } catch (apache::thrift::transport::TTransportException &ex) {
            COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        } catch (const std::exception &ex) {
            COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        } catch (...) {
            COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        }

        // Nofity ExternalPluginManager to check plugin
        ExternalPluginManager::GetInstance().StartMonitor();
        return;
    }

    template<typename T, typename RETV, typename ... ARGS1, typename ... ARGS2>
    void ApplicationServiceNormalCall(void (T::*MemFunc)(RETV& ret, ARGS1...),
        RETV& ret, ARGS2 &&... args)
    {
        auto pClient = GetThriftClient();
        if (!pClient) {
            /*
            * two case here
            * 1.job not send to plugin yet, and not other job using the plugin, no plugin start,
            * not need to notify plugin
            * 2.plugin or frame error,could not get plugin client,we treat as send plugin already
            */
            ERRLOG("Get thrift client failed.");
            return;
        }
        
        try {
            std::shared_ptr<ApplicationServiceIf> client = std::dynamic_pointer_cast<ApplicationServiceIf>(
                pClient->GetConcurrentClientIf<ApplicationServiceConcurrentClient>("ApplicationService"));
            if (!client) {
                ERRLOG("Get Protect service failed.");
                return;
            }
            Call(client, MemFunc, ret, std::forward<ARGS2>(args)...);
        } catch (apache::thrift::transport::TTransportException &ex) {
            COMMLOG(OS_LOG_ERROR, "TTransportException. %s", ex.what());
        } catch (const std::exception &ex) {
            COMMLOG(OS_LOG_ERROR, "Standard C++ Exception. %s", ex.what());
        } catch (...) {
            COMMLOG(OS_LOG_ERROR, "Unknown exception.");
        }

        // Nofity ExternalPluginManager to check plugin
        ExternalPluginManager::GetInstance().StartMonitor();
        return;
    }

protected:
    std::shared_ptr<thriftservice::IThriftClient> GetThriftClient();
    std::shared_ptr<ProtectServiceIf> GetProtectServiceClient(
        std::shared_ptr<thriftservice::IThriftClient> pThriftClient);
    mp_int32 SendAbortToPlugin();
    mp_int32 MountNas_Ex(const std::vector<Json::Value>& vecJsonRep, Json::Value& JsonRep_new,
        const JobPermission &jobPermit);
    mp_bool IsManualMount(const Json::Value &JsonManual);
    mp_bool IsNeedShareMount(const Json::Value& jsonRepo);
    mp_int32 UmountNas();  // umount
    void SetJobRetry(bool retry);
    mp_bool IsLogBackupJob();
    mp_void SetAgentsToExtendInfo(Json::Value &param);
    void SetPostScanParam(const StorageRepository& repo, const Json::Value& repoJson);
protected:
    PluginJobData m_data;
    mp_int32 m_iRet = MP_SUCCESS;
    
    bool m_isAgentNeedScan = false;
    // if start timing job detail report
    bool m_startTiming {false};
    // last success job detail report time point
    std::chrono::time_point<std::chrono::steady_clock> m_timePoint {std::chrono::steady_clock::now()};
    // when call thrift success, save pluginId for check plugin reload
    mp_string m_pluginPID;
    using AbortHandleFun = std::function<void()>;
    Executor m_executor {nullptr};

    mp_void StartKeepAliveThread();
    mp_void StopKeepAliveThread();
private:
    virtual mp_int32 PauseJobToPlugin();
    mp_void SplitRepositories();
    mp_int32 SplitRepositoriesParam(Json::Value &jsonRep, Json::Value &JsonRep_new);
    bool m_shouldRetry {false};
    mp_bool IsNonNativeRestore();
    mp_void NonNativeSplitRepo(std::map<Json::ArrayIndex, std::vector<Json::Value>> &mapJsonRep,
        mp_int32 &noNeedMountCopyCount);
    mp_void SplitRepoRemotePath(std::vector<Json::Value> &dataCopyRep);
    mp_bool IsScriptValid(const mp_string& scriptFileName);

    mp_void ReportSubJobRunning();
    bool NeedMount(const Json::Value &jsonRep);
    bool IsNasLiveMountJob();
    void CheckReplaceHost(const std::vector<mp_string>& containerBackendIps, const mp_string& esnLocal,
        Json::Value &JsonRep_new, std::map<Json::ArrayIndex, std::vector<Json::Value>>::iterator& jsonVec);
    std::atomic<bool> m_stopMountKeepAliveTheadFlag {false};
    std::shared_ptr<std::thread> m_mountKeepAliveTh;
};
}  // namespace AppProtect

#endif
