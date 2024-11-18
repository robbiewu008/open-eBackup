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
#ifndef WAL_BACKUP_H
#define WAL_BACKUP_H

#include "common/Types.h"
#include "taskmanager/externaljob/Job.h"


namespace AppProtect {
class PluginLogBackup {
public:
    PluginLogBackup()
    {}

    ~PluginLogBackup()
    {}

    mp_int32 AssembleCopyInfo(const mp_string &jobId, Json::Value &jobValue);
    mp_int32 LogBackup(PluginJobData &data);
    mp_int32 AssembleRepository(PluginJobData &data);

private:
    mp_int32 AssembleCopyInfo_Ex(const std::vector<mp_string> &vecAgentLastBackupMeta,
        const std::vector<mp_string> &vecDmeLastBackupMeta, const mp_string &mountPath, const mp_string &mainID,
        Json::Value &jobValue);
    mp_int32 ReadMetaData(mp_string &lastLogBackupPath, const mp_string &mainID,
        std::vector<mp_string> &vecOutput);
    mp_int32 CreateLogDir(const std::vector<mp_string> &vecOutput, const mp_string &latestDataCopyId,
    const mp_string &latestLogCopyName, const PluginJobData &data);
    mp_int32 WriteDMELastBackupMeta(mp_string &lastLogBackupPath, const mp_string &latestDataCopyId,
        const mp_string &latestLogCopyName);
    mp_bool IsLogBackupJob(const PluginJobData &data);
    mp_int32 GetExtendInfo(const PluginJobData &data, mp_string &latestDataCopyId, mp_string &latestLogCopyName);
    mp_int32 NormalBackup(const PluginJobData &data);
    mp_int32 LatestLogCopyNameInconsistent(const std::vector<mp_string> &vecOutput, const PluginJobData &data);
    mp_int32 CrossDataCopyBackup(const PluginJobData &data);
    mp_int32 WriteToFile(const mp_string& strFilePath, const std::vector<mp_string>& vecInput);
private:
    mp_string m_logRepoMountPoint;
};
}
#endif