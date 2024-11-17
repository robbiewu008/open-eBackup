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
#ifndef _PLUGIN_SUB_POST_JOB_H_
#define _PLUGIN_SUB_POST_JOB_H_

#include <vector>
#include <stdint.h>
#include "taskmanager/externaljob/PluginSubJob.h"
#include "apps/appprotect/plugininterface/ApplicationProtectPlugin_types.h"

namespace AppProtect {

class ProtectServiceIf;

class PluginSubPostJob : public PluginSubJob {
public:
    PluginSubPostJob(const PluginJobData& data) : PluginSubJob(data)
    {}
    ~PluginSubPostJob()
    {}

    void ReportDetails(){};
    Executor GetPluginCall() override;

    Executor ExecPostScript();

    bool NotifyPluginReloadImpl(const mp_string& appType, const mp_string& newPluginPID);
    mp_int32 NotifyPauseJob() override;
    mp_int32 CanbeRunInLocalNode() override;
#ifndef WIN32
    mp_int32 ScanAndRecordFile();
    mp_int32 TruncateScanResult(std::vector<mp_string> &vecfolderPath);
    mp_int32 SaveScanResult(mp_string &savePath, std::vector<mp_string> &vecFolderPath,
        std::vector<mp_string> &vecFilePath);
    mp_int32 ExtractScanResult(std::vector<mp_string> &vecResult, std::vector<mp_string> &vecFolderPath,
        std::vector<mp_string> &vecFilePath);
#endif
protected:
    mp_int32 ExecBackupJob();
    mp_int32 ExecRestoreJob();
    mp_int32 ExecInrestoreJob();
    mp_int32 ReportCompleted();
    mp_int32 GetJobsExecResult();
    mp_int32 GetJobStatus(const Json::Value& rsp);
    mp_int32 DeleteQosStrategy();

private:
    JobResult::type m_wholeJobResult = JobResult::type::SUCCESS;
};

}  // namespace AppProtect

#endif
