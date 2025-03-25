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
#ifndef AGENT_BACKUP_STEP_RMAN_H
#define AGENT_BACKUP_STEP_RMAN_H
#include <map>
#include <vector>

#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"

static const mp_string STEPNAME_NATIVEBACKUP = "TaskStepOracleNativeBackup";
class TaskStepOracleNativeBackup : public TaskStepOracleNative {
public:
    TaskStepOracleNativeBackup(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeBackup();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Redo(mp_string& innerPID);
    mp_int32 Cancel();
    mp_int32 RefreshStepInfo();

private:
    mp_int32 channel;
    mp_int32 level;
    mp_int32 limitSpeed;
    mp_int32 truncateLog;
    mp_int32 storType;
    mp_string encAlgo;
    mp_string encKey;
    mp_int32 archiveLogKeepDays;
private:
    mp_int32 BuildRmanBackupScriptParam(mp_string& strParam);
    typedef mp_int32 (TaskStepOracleNativeBackup::*FUNC_ANALYSEBACKINFO)(
        const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    std::map<mp_string, FUNC_ANALYSEBACKINFO> HandlerBackupRsts;
    std::map<mp_string, FUNC_ANALYSEBACKINFO> HandlerExtendParam;

    mp_int32 AnalyseOracleBackupRst(std::vector<mp_string>& vecResult, Json::Value& backupRst);

    mp_int32 DataBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 LogBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 HistorySpeedBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 BackupSizeBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 PfileParamBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 ExtendParamBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 BackupLevelBackupHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 FilelistHandler(const std::vector<mp_string>& backupRstInfo, Json::Value& backupRst);
    mp_int32 RunExecScript(int bakcupMode, const mp_string& strParam, std::vector<mp_string>& vecResult);
    void UpdateOracleDbInfo();
};

#endif
