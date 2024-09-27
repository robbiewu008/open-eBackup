/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @brief  plugin sub job
 * @version 1.1.0
 * @date 2021-11-19
 * @author kWX884906
 */

#ifndef _PLUGIN_SUB_GENE_JOB_H_
#define _PLUGIN_SUB_GENE_JOB_H_

#include <atomic>
#include "common/Semaphore.h"
#include "taskmanager/externaljob/PluginSubJob.h"

namespace AppProtect {

class ProtectServiceIf;

class PluginSubGeneJob : public PluginSubJob {
public:
    PluginSubGeneJob(const PluginJobData& data) : PluginSubJob(data)
    {
        std::atomic_init(&m_ret, MP_SUCCESS);
    }
    ~PluginSubGeneJob()
    {}

    void ReportDetails(){};
    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override;
    Executor GetPluginCall() override;;
    Executor GetWait() override;
    bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID) override;
    mp_int32 NotifyPauseJob() override;
protected:
    mp_int32 ExecBackupJob();
    mp_int32 ExecRestoreJob();
    mp_int32 ExecDelCopyJob();
    mp_int32 ExecCheckCopyJob();
    mp_int32 ExecLivemountJob();
    mp_int32 ExecCancelLivemountJob();
    mp_int32 ExecInrestoreJob();
    mp_int32 ExecBuildIndexJob();
    mp_int32 WaitPluginNotify();

private:
    mp_void ParserNodeNum(mp_int32& nodeNum);

private:
    Semaphore m_sem;
    std::atomic<mp_int32> m_ret;
};

}  // namespace AppProtect

#endif
