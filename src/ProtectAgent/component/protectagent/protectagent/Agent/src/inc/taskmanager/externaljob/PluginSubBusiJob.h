/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  plugin sub job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */

#ifndef _PLUGIN_SUB_BUSI_JOB_H_
#define _PLUGIN_SUB_BUSI_JOB_H_

#include "taskmanager/externaljob/PluginSubJob.h"
#include "common/JsonHelper.h"

namespace AppProtect {

class ProtectServiceIf;

struct SubJobInfo {
    uint32_t policy {0};
    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMBER_TO_SPECIFIED_NAME(policy, executePolicy)
    END_SERIAL_MEMEBER
};

class PluginSubBusiJob : public PluginSubJob {
public:
    PluginSubBusiJob(const PluginJobData& data) : PluginSubJob(data)
    {}
    ~PluginSubBusiJob()
    {}

    void ReportDetails(){};

    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override;
    Executor GetPluginCall() override;
    bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);
    mp_int32 NotifyPauseJob() override;
    mp_int32 CanbeRunInLocalNode() override;
protected:
    mp_int32 ExecBackupJob();
    mp_int32 ExecRestoreJob();
    mp_int32 ExecLivemountJob();
    mp_int32 ExecCancelLivemountJob();
    mp_int32 ExecDelCopyJob();
    mp_int32 ExecCheckCopyJob();
    mp_int32 ExecInrestoreJob();
    mp_int32 ExecBuildIndexJob();
    mp_int32 ExecAllowBackupInLocalNode();
    bool CheckAgentFailed(mp_string &nodeId);
    mp_int32 CheckIfCanRun(const Json::Value &roleInfo, BackupLimit::type &policy);
    mp_void GetRole(const mp_string &nodeId, const Json::Value &roleInfo, mp_int32 &role);
    bool ParseRoleInfo(Json::Value &roleInfo);
    mp_void ParsePolicy(BackupLimit::type &policy);
    mp_void ReportJobDetail(const AppProtect::SubJobStatus::type &jobStatus, mp_string jobId, mp_string subJobId);
    void GetJobPermission(AppProtect::JobPermission &jobPermit);
    mp_int32 CanbeRunInLocalNodeForBackup();
    mp_int32 CanbeRunInLocalNodeForRestore();
    mp_int32 CanbeRunInLocalNodeForCheckCopy();
    mp_int32 CanbeRunInLocalNodeForDelCopy();
    bool NeedExecPolicy();
};

}  // namespace AppProtect

#endif
