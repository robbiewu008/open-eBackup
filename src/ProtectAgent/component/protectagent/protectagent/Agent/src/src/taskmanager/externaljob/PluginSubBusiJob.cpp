/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  plugin sub job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */
#include "taskmanager/externaljob/PluginSubBusiJob.h"
#include "common/Log.h"
#include "apps/appprotect/plugininterface/ProtectService.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "host/host.h"
#include "taskmanager/externaljob/ReportJobDetailFactory.h"

namespace AppProtect {
mp_int32 PluginSubBusiJob::ExecBackupJob()
{
    ActionResult ret;
    BackupJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Backup business job, jobId=%s, subJobId=%s.", subJobParam.jobId.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncExecuteBackupSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecRestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Restore business job, jobId=%s, subJobId=%s.", m_data.mainID.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncExecuteRestoreSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecLivemountJob()
{
    ActionResult ret;
    LivemountJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Livemount business job, jobId=%s, subJobId=%s.", subJobParam.jobId.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncExecuteLivemountSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecCancelLivemountJob()
{
    ActionResult ret;
    CancelLivemountJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Cancel livemount business job, jobId=%s, subJobId=%s.",
        subJobParam.jobId.c_str(),
        subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncExecuteCancelLivemountSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecDelCopyJob()
{
    ActionResult ret;
    DelCopyJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Delcopy business job, jobId=%s, subJobId=%s.", subJobParam.jobId.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncDelCopySubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecCheckCopyJob()
{
    ActionResult ret;
    CheckCopyJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Checkcopy business job, jobId=%s, subJobId=%s.", subJobParam.jobId.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncCheckCopySubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecInrestoreJob()
{
    ActionResult ret;
    RestoreJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG("Instant restore business job, jobId=%s, subJobId=%s.",
        subJobParam.jobId.c_str(),
        subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncExecuteInstantRestoreSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

mp_int32 PluginSubBusiJob::ExecBuildIndexJob()
{
    ActionResult ret;
    BuildIndexJob jobParam;
    SubJob subJobParam;
    JsonToStruct(m_data.param, jobParam);
    JsonToStruct(m_data.param, subJobParam);
    INFOLOG(
        "Build index business job, jobId=%s, subJobId=%s.", subJobParam.jobId.c_str(), subJobParam.subJobId.c_str());
    ProtectServiceCall(&ProtectServiceIf::AsyncBuildIndexSubJob, ret, jobParam, subJobParam);
    return ret.code;
}

void PluginSubBusiJob::NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails)
{
    if (jobDetails.jobStatus == SubJobStatus::type::FAILED) {
        SetJobRetry(true);
    }
    PluginSubJob::NotifyJobDetail(jobDetails);
}

Executor PluginSubBusiJob::GetPluginCall()
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
        { MainJobType::DELETE_COPY_JOB, [this](int32_t)
            {
                return ExecDelCopyJob();
            }
        },
        { MainJobType::BUILD_INDEX_JOB, [this](int32_t)
            {
                return ExecBuildIndexJob();
            }
        },
        { MainJobType::INSTANT_RESTORE_JOB, [this](int32_t)
            {
                return ExecInrestoreJob();
            }
        },
        { MainJobType::CHECK_COPY_JOB, [this](int32_t)
            {
                return ExecCheckCopyJob();
            }
        },
        { MainJobType::LIVEMOUNT_JOB, [this](int32_t)
            {
                return ExecLivemountJob();
            }
        },
        { MainJobType::CANCEL_LIVEMOUNT_JOB, [this](int32_t)
            {
                return ExecCancelLivemountJob();
            }
        }
    };
    auto it = ExcuterMap.find(m_data.mainType);
    if (it != ExcuterMap.end()) {
        return it->second;
    }
    return GetEmptyExcutor();
}

bool PluginSubBusiJob::NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID)
{
    INFOLOG("NotifyPluginReloadImpl jobId=%s suJobId:%s notify plugin reload m_data.status:%d \
        m_pluginPID:%s newPluginPID:%s.",
        m_data.mainID.c_str(), m_data.subID.c_str(), m_data.status, m_pluginPID.c_str(), newPluginPID.c_str());
    if (m_data.status == mp_uint32(SubJobState::UNDEFINE) ||
        m_data.status == mp_uint32(SubJobState::SubJobComplete) ||
        m_data.status == mp_uint32(SubJobState::SubJobFailed)) {
        DBGLOG("No need redo again, jobId=%s, subJobId=%s, status=%d.", m_data.mainID.c_str(), m_data.subID.c_str(),
            m_data.status);
        return true;
    }
    if (!m_pluginPID.empty() && m_pluginPID != newPluginPID) {
        return false;
    }
    return true;
}

mp_int32 PluginSubBusiJob::NotifyPauseJob()
{
    INFOLOG("After Pause job, set job failed, jobId=%s subJobId=%s", m_data.mainID.c_str(), m_data.subID.c_str());
    SetJobRetry(true);
    ChangeState(SubJobState::SubJobFailed);
    return MP_SUCCESS;
}

mp_int32 PluginSubBusiJob::ExecAllowBackupInLocalNode()
{
    INFOLOG("Job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    ActionResult ret;
    BackupJob backupJob;
    SubJob subJob;
    SetAgentsToExtendInfo(m_data.param);
    JsonToStruct(m_data.param, backupJob);
    JsonToStruct(m_data.param, subJob);
    ProtectServiceCall(&ProtectServiceIf::AllowBackupSubJobInLocalNode, ret, backupJob, subJob);
    if (ret.code != MP_SUCCESS) {
        ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
        ERRLOG("Check jobId=%s can be allow backup in local node failed, subJobId=%s, error=%d",
            m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
    }
    return ret.code;
}

bool PluginSubBusiJob::CheckAgentFailed(mp_string &nodeId)
{
    if (nodeId.empty()) {
        return true;
    }
    Json::Value failedAgents;
    if (m_data.param.isMember("failedAgents")) {
        failedAgents = m_data.param["failedAgents"];
    }
    for (int i = 0; i < failedAgents.size(); i++) {
        if (failedAgents[i].isObject() && failedAgents[i]["id"].isString() &&
            nodeId.compare(failedAgents[i]["id"].asString()) == 0) {
            return true;
        }
    }
    return false;
}

mp_int32 PluginSubBusiJob::CheckIfCanRun(const Json::Value &roleInfo, BackupLimit::type &policy)
{
    Json::Value agents;
    if (m_data.param.isMember("agents")) {
        agents = m_data.param["agents"];
    }

    for (int i = 0; i < agents.size(); i++) {
        mp_string nodeId = agents[i]["id"].isString() ? agents[i]["id"].asString() : "";
        // 过滤失败节点
        if (CheckAgentFailed(nodeId)) {
            continue;
        }
        mp_int32 tmpRole = 0;
        GetRole(nodeId, roleInfo, tmpRole);
        WARNLOG("Check job, jobId=%s, subJobId=%s, tmpRole=%d, policy = %d.", m_data.mainID.c_str(),
            m_data.subID.c_str(), tmpRole, policy);
        if (tmpRole == (mp_int32)EnvironmentRole::ACTIVE && policy == BackupLimit::FIRST_MASTER ||
            tmpRole == (mp_int32)EnvironmentRole::STANDBY && policy == BackupLimit::FIRST_SLAVE) {
            /*
                在还有其它符合策略的节点可以执行的情况下，当前节点不锁定任务，也不上报错误;
                如策略为优先备、在一主双备的情况下，当前节点为主节点：
                如果Agents中还有备节点，则不能执行任务；
                如果Agents中已经没有备节点，则当前节点可以执行任务。
            */
            WARNLOG("Job can not run for policy not match,jobId=%s, subJobId=%s, policy=%d.",
                m_data.mainID.c_str(), m_data.subID.c_str(), policy);
            return MP_TASK_FAILED_NO_REPORT;
        }
    }
    INFOLOG("Check job can run, jobId=%s subJobId=%s, policy=%d.", m_data.mainID.c_str(), m_data.subID.c_str(), policy);

    policy = BackupLimit::NO_LIMIT;
    return MP_SUCCESS;
}

mp_void PluginSubBusiJob::GetRole(const mp_string &nodeId, const Json::Value &roleInfo, mp_int32 &role)
{
    for (int i = 0; i < roleInfo.size(); i++) {
        mp_string nodeName = roleInfo[i]["uuid"].isString() ? roleInfo[i]["uuid"].asString() : "";
        if (nodeName.compare(nodeId) == 0) {
            auto extendInfo = roleInfo[i]["extendInfo"];
            if (extendInfo.isObject() && extendInfo.isMember("role") && extendInfo["role"].isString()) {
                mp_string strRole = extendInfo["role"].asString();
                role = atoi(strRole.c_str());
            }
            break;
        }
    }
}

bool PluginSubBusiJob::ParseRoleInfo(Json::Value &roleInfo)
{
    roleInfo.clear();
    if (m_data.param.isMember("envInfo")) {
        auto envInfo = m_data.param["envInfo"];
        if (envInfo.isObject() && envInfo.isMember("nodes")) {
            roleInfo = envInfo["nodes"];
        }
    }
    if (roleInfo.isNull()) {
        ERRLOG("Get Role failed.");
        return false;
    }
    return true;
}

mp_void PluginSubBusiJob::ParsePolicy(BackupLimit::type &policy)
{
    if (m_data.param.isMember("extendInfo")) {
        auto extendInfo = m_data.param["extendInfo"];
        if (extendInfo.isObject() && extendInfo.isMember("slave_node_first")) {
            mp_string slaveFirst = extendInfo["slave_node_first"].asString();
            policy = (slaveFirst.compare("true") == 0) ? BackupLimit::FIRST_SLAVE : BackupLimit::FIRST_MASTER;
        }
    }
}

bool PluginSubBusiJob::NeedExecPolicy()
{
    if (!m_data.param.isMember("subTaskParams")) {
        ERRLOG("Cannot find subTaskParams.");
        return false;
    }
    SubJobInfo subJobInfo;
    mp_string jsonStr = m_data.param["subTaskParams"].isString() ? m_data.param["subTaskParams"].asString() : "";
    if (!JsonHelper::JsonStringToStruct(jsonStr, subJobInfo)) {
        ERRLOG("Convert sub job info error.");
        return false;
    }
    DBGLOG("SubJob exec policy, policy=%d, jobId=%s, subJobId=%s.", subJobInfo.policy,
        m_data.mainID.c_str(), m_data.subID.c_str());
    // 子任务在任意节点上执行时，才执行优先主/备策略判断
    if (subJobInfo.policy == static_cast<int>(ExecutePolicy::ANY_NODE)
        || subJobInfo.policy == static_cast<int>(ExecutePolicy::RETRY_OTHER_NODE_WHEN_FAILED)) {
        return true;
    }
    return false;
}

mp_int32 PluginSubBusiJob::CanbeRunInLocalNodeForBackup()
{
    if (!NeedExecPolicy()) {
        return ExecAllowBackupInLocalNode();
    }
    // 1.先查询集群，更新当前节点角色role
    mp_string curNodeId;
    CHost host;
    mp_int32 iRet = host.GetHostSN(curNodeId);
    if (iRet != MP_SUCCESS) {
        ERRLOG("GetHostSN failed, jobId=%s, subJobId=%s, errCode=%d",
            m_data.mainID.c_str(), m_data.subID.c_str(), iRet);
        return ERR_PLUGIN_AUTHENTICATION_FAILED;
    }

    // 2.根据节点ID从PM表中获取角色
    mp_int32 role = (mp_int32)EnvironmentRole::NONE;
    Json::Value roleInfo;
    if (ParseRoleInfo(roleInfo)) {
        GetRole(curNodeId, roleInfo, role);
    }

    // 3.获取策略，针对优先主/备策略进行处理
    BackupLimit::type bkPolicy = BackupLimit::NO_LIMIT;
    ParsePolicy(bkPolicy);
    INFOLOG("CanbeRunInLocalNode, jobId=%s subJobId=%s, role=%d, policy=%d.",
        m_data.mainID.c_str(), m_data.subID.c_str(), role, bkPolicy);
    if (bkPolicy == BackupLimit::FIRST_MASTER || bkPolicy == BackupLimit::FIRST_SLAVE) {
        // 4.判断当前节点是否与策略一致
        if (role == (mp_int32)EnvironmentRole::ACTIVE && bkPolicy == BackupLimit::FIRST_MASTER ||
            role == (mp_int32)EnvironmentRole::STANDBY && bkPolicy == BackupLimit::FIRST_SLAVE) {
            return ExecAllowBackupInLocalNode();
        }

        // 5.在优先主/备策略与当前节点不匹配的情况下，根据Agents与failedAgents列表判断当前节点是否可执行
        CHECK_FAIL_EX(CheckIfCanRun(roleInfo, bkPolicy));
    }

    return ExecAllowBackupInLocalNode();
}

mp_int32 PluginSubBusiJob::CanbeRunInLocalNodeForRestore()
{
    INFOLOG("Restore job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    ActionResult ret;
    RestoreJob restoreJob;
    SubJob subJob;
    JsonToStruct(m_data.param, restoreJob);
    JsonToStruct(m_data.param, subJob);
    ProtectServiceCall(&ProtectServiceIf::AllowRestoreSubJobInLocalNode, ret, restoreJob, subJob);
    if (ret.code != MP_SUCCESS) {
        ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
        ERRLOG("Check jobId=%s can be allow restore in local node failed, subJobId=%s, error=%d",
            m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
    }
    return ret.code;
}

mp_int32 PluginSubBusiJob::CanbeRunInLocalNodeForCheckCopy()
{
    INFOLOG("Check copy job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    ActionResult ret;
    CheckCopyJob checkCopyJob;
    SubJob subJob;
    JsonToStruct(m_data.param, checkCopyJob);
    JsonToStruct(m_data.param, subJob);
    ProtectServiceCall(&ProtectServiceIf::AllowCheckCopySubJobInLocalNode, ret, checkCopyJob, subJob);
    if (ret.code != MP_SUCCESS) {
        ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
        ERRLOG("Check jobId=%s can be allow check copy in local node failed, subJobId=%s, error=%d",
            m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
    }
    return ret.code;
}

mp_int32 PluginSubBusiJob::CanbeRunInLocalNodeForDelCopy()
{
    INFOLOG("Check del job can run,jobId=%s, subJobId=%s.", m_data.mainID.c_str(), m_data.subID.c_str());
    ActionResult ret;
    DelCopyJob delCopyJob;
    SubJob subJob;
    JsonToStruct(m_data.param, delCopyJob);
    JsonToStruct(m_data.param, subJob);
    ProtectServiceCall(&ProtectServiceIf::AllowDelCopySubJobInLocalNode, ret, delCopyJob, subJob);
    if (ret.code != MP_SUCCESS) {
        ret.code = (ret.bodyErr == 0) ? ERR_PLUGIN_AUTHENTICATION_FAILED : ret.bodyErr;
        ERRLOG("Check jobId=%s can be allow del copy in local node failed, subJobId=%s, error=%d",
            m_data.mainID.c_str(), m_data.subID.c_str(), ret.code);
    }
    return ret.code;
}

mp_int32 PluginSubBusiJob::CanbeRunInLocalNode()
{
    LOGGUARD("");
    if (m_data.mainType == MainJobType::BACKUP_JOB) {
        return CanbeRunInLocalNodeForBackup();
    } else if (m_data.mainType == MainJobType::RESTORE_JOB) {
        return CanbeRunInLocalNodeForRestore();
    } else if (m_data.mainType == MainJobType::CHECK_COPY_JOB) {
        return CanbeRunInLocalNodeForCheckCopy();
    } else if (m_data.mainType == MainJobType::DELETE_COPY_JOB && m_data.appType == "HCSCloudHost") {
        return CanbeRunInLocalNodeForDelCopy();
    }
    return MP_SUCCESS;
}
}  // namespace AppProtect
