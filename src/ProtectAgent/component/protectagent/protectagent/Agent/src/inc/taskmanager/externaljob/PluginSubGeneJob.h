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
