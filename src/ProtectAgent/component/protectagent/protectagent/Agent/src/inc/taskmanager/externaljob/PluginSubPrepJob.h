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
#ifndef _PLUGIN_SUB_PREP_JOB_H_
#define _PLUGIN_SUB_PREP_JOB_H_

#include <atomic>
#include "common/Semaphore.h"
#include "taskmanager/externaljob/PluginSubJob.h"
#include "jsoncpp/include/json/value.h"
#include "jsoncpp/include/json/json.h"

namespace AppProtect {

class ProtectServiceIf;

class PluginSubPrepJob : public PluginSubJob {
public:
    PluginSubPrepJob(const PluginJobData& data) : PluginSubJob(data)
    {
        std::atomic_init(&m_ret, MP_SUCCESS);
    }
    ~PluginSubPrepJob()
    {}

    void ReportDetails(){};
    void NotifyJobDetail(const AppProtect::SubJobDetails& jobDetails) override;
    Executor GetPluginCall() override;;
    Executor GetWait();
    bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);
    mp_int32 NotifyPauseJob() override;
    mp_void CheckArchiveConnectIp(
        const std::vector<Json::Value>& vecJsonArchiveIp, Json::Value& vecJsonConnectArchiveIp);

protected:
    mp_int32 ExecBackupJob();
    mp_int32 ExecRestoreJob();
    mp_int32 ExecInrestoreJob();
    mp_int32 WaitPluginNotify();
private:
    Semaphore m_sem;
    std::atomic<mp_int32> m_ret;
};

}  // namespace AppProtect

#endif
