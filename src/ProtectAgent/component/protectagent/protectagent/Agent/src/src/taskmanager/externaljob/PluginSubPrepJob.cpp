/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  plugin sub job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */

#include "taskmanager/externaljob/PluginSubPrepJob.h"
#include "common/Log.h"
#include "common/JsonUtils.h"
#include "message/tcp/CSocket.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "host/host.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"
#include "apps/appprotect/plugininterface/ApplicationProtectBaseDataType_types.h"

namespace AppProtect {
const mp_int32 CHECK_HOST_RETRY_TIMES = 3;
const mp_int32 CHECK_HOST_TIMEOUT = 1000; // 1s
namespace {
    const mp_int32 GENERATE_TIMEOUT = 5 * 60 * 1000;
}

void PluginSubPrepJob::NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails)
{
    m_ret.store(GetSignedFromJobDetail(jobDetails.jobStatus));
    m_sem.Signal();
    PluginSubJob::NotifyJobDetail(jobDetails);
}

mp_int32 PluginSubPrepJob::ExecBackupJob()
{
    ActionResult ret;
    SetAgentsToExtendInfo(m_data.param);
    BackupJob jobParam;
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Backup pre jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncBackupPrerequisite, ret, jobParam);
    return ret.code;
}

mp_int32 PluginSubPrepJob::ExecRestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    for (Json::ArrayIndex index = 0; index < m_data.param["copies"].size(); ++index) {
        std::vector<Json::Value> vecJsonArchiveIp;
        Json::Value vecJsonConnectArchiveIp;
        if (!m_data.param["copies"][index].isObject() || !m_data.param["copies"][index]["type"].isString() ||
            m_data.param["copies"][index]["type"].asString() != "s3Archive") {
            continue;
        }
        INFOLOG("It is a archive-restore job.Start to check connection with archive.");
        for (Json::ArrayIndex index1 = 0; index1 < m_data.param["copies"][index]["repositories"].size(); ++index1) {
            if (!m_data.param["copies"][index]["repositories"][index1].isObject() ||
                !m_data.param["copies"][index]["repositories"][index1]["protocol"].isInt() ||
                m_data.param["copies"][index]["repositories"][index1]["protocol"].asInt() !=
                RepositoryProtocolType::type::S3) {
                continue;
            }
            CJsonUtils::GetJsonArrayJson(
                m_data.param["copies"][index]["repositories"][index1]["extendInfo"], "service_info", vecJsonArchiveIp);
            CheckArchiveConnectIp(vecJsonArchiveIp, vecJsonConnectArchiveIp);
            m_data.param["copies"][index]["repositories"][index1]["extendInfo"]["service_info"] =
                vecJsonConnectArchiveIp;
        }
    }
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Restore pre jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncRestorePrerequisite, ret, jobParam);
    return ret.code;
}

mp_int32 PluginSubPrepJob::ExecInrestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    JsonToStruct(m_data.param, jobParam);
    INFOLOG("Instance restore pre jobId=%s.", jobParam.jobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncInstantRestorePrerequisite, ret, jobParam);
    return ret.code;
}

Executor PluginSubPrepJob::GetPluginCall()
{
    std::map<MainJobType, Executor> ExcuterMap = {
        { MainJobType::BACKUP_JOB, [this](int32_t)
            {
                return ExecBackupJob();
            }
        },
        { MainJobType::RESTORE_JOB, [this](int32_t)
            {
                return ExecRestoreJob();
            }
        },
        { MainJobType::INSTANT_RESTORE_JOB, [this](int32_t)
            {
                return ExecInrestoreJob();
            }
        }
    };
    auto it = ExcuterMap.find(m_data.mainType);
    if (it != ExcuterMap.end()) {
        return it->second;
    }
    return GetEmptyExcutor();
}

Executor PluginSubPrepJob::GetWait()
{
    std::set<MainJobType> ExcuterSet = {
        MainJobType::BACKUP_JOB,
        MainJobType::RESTORE_JOB,
        MainJobType::INSTANT_RESTORE_JOB
    };
    auto it = ExcuterSet.find(m_data.mainType);
    if (it != ExcuterSet.end()) {
        return [this](int32_t) {
            return WaitPluginNotify();
        };
    }
    INFOLOG("Pre job jobId=%s, subJobId=%s No Need to wait", m_data.mainID.c_str(), m_data.subID.c_str());
    return GetEmptyExcutor();
}

mp_int32 PluginSubPrepJob::WaitPluginNotify()
{
    StartReportTiming();
    mp_int32 ret = MP_SUCCESS;
    while (true) {
        if (m_sem.TimedWait(GENERATE_TIMEOUT)) {
            ret = m_ret.load();
            DBGLOG("Prerequisite job, jobId=%s receive ret: %d.", m_data.mainID.c_str(), ret);
            if (ret == MP_EAGAIN) {
                continue;
            }
            INFOLOG("Prerequisite job, jobId=%s finish, ret: %d.", m_data.mainID.c_str(), ret);
        } else {
            WARNLOG("Prerequisite job, jobId=%s timeout.", m_data.mainID.c_str());
            ret = MP_FAILED;
        }
        break;
    }
    return ret;
}

bool PluginSubPrepJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("jobId=%s notify plugin reload, m_pluginPID:%s newPluginPID:%s.",
        m_data.mainID.c_str(), m_pluginPID.c_str(), newPluginPID.c_str());
    if (!m_pluginPID.empty() && m_pluginPID != newPluginPID) {
        m_ret.store(MP_REDO);
        m_sem.Signal();
        return true;
    }
    return false;
}

mp_int32 PluginSubPrepJob::NotifyPauseJob()
{
    INFOLOG("Main Job jobId=%s, pause job.", m_data.mainID.c_str());
    m_ret.store(MP_FAILED);
    m_sem.Signal();
    return MP_SUCCESS;
}

mp_void PluginSubPrepJob::CheckArchiveConnectIp(
    const std::vector<Json::Value>& vecJsonArchiveIp, Json::Value& vecJsonConnectArchiveIp)
{
    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed when connect archive.");
        return;
    }

    for (mp_int32 i = 0; i < vecJsonArchiveIp.size(); ++i) {
        mp_string archiveIp;
        mp_int32 port;
        CJsonUtils::GetJsonString(vecJsonArchiveIp[i], "ip", archiveIp);
        CJsonUtils::GetJsonInt32(vecJsonArchiveIp[i], "port", port);
        for (mp_int32 j = 0; j < CHECK_HOST_RETRY_TIMES; ++j) {
            if (httpClient->TestConnectivity(archiveIp, std::to_string(port))) {
                vecJsonConnectArchiveIp.append(vecJsonArchiveIp[i]);
                INFOLOG("Check connection with archive success, ip=%s, port=%d.", archiveIp.c_str(), port);
                break;
            } else {
                WARNLOG("Check connection with archive fail, ip=%s, port=%d.", archiveIp.c_str(), port);
            }
        }
    }
}
}
