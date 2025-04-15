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
    mp_int32 ScanAndRecordFile();
    mp_int32 GetScanResult(const AppProtect::ScanRepositories &pluginsPaths);
    mp_int32 DoScan(mp_string& scanPath, std::vector<mp_string>& vecFolders, std::vector<mp_string>& vecFiles);
    mp_int32 NormalGetScanRepositories(AppProtect::ScanRepositories &pluginsPaths);
    void GetScanSavePath(const mp_string &path, AppProtect::ScanRepositories &pluginsPaths);
    mp_int32 TruncateScanResult(std::vector<mp_string> &vecfolderPath);
    mp_int32 SaveScanResult(const mp_string &savePath, std::vector<mp_string> &vecFolderPath,
        std::vector<mp_string> &vecFilePath, const mp_string &type);
    mp_int32 ExtractScanResult(std::vector<mp_string> &vecResult, std::vector<mp_string> &vecFolderPath,
        std::vector<mp_string> &vecFilePath);
#ifdef WIN32
    mp_string GetMountPublicPath();
    mp_int32 ScanForInstantlyMountWin(mp_string& path, std::vector<mp_string>& vecResult);
    mp_int32 ScanDirAndFileWin(mp_string &rootPath,
        std::vector<mp_string> &rootfolderpath, std::vector<mp_string> &rootfilepath);
    mp_int32 GetFolderPathWin(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
    mp_int32 GetFilePathWin(mp_string &strFolder, std::vector<mp_string> &vecFolderPath);
    mp_int32 WriteScanResultForInstantlyMountWin(const std::vector<mp_string> &vecParam);
    mp_int32 CheckPathIsValidWin(const mp_string &filePath);
#endif
protected:
    mp_int32 ExecBackupJob();
    mp_int32 ExecRestoreJob();
    mp_int32 ExecInrestoreJob();
    mp_int32 ReportCompleted();
    mp_int32 GetJobsExecResult();
    mp_int32 GetJobStatus(const Json::Value& rsp);
    mp_int32 DeleteQosStrategy();

    bool IsPostScanValid();

private:
    JobResult::type m_wholeJobResult = JobResult::type::SUCCESS;
};

}  // namespace AppProtect

#endif
