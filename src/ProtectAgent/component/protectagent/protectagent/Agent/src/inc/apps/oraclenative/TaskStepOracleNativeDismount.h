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
#ifndef AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_H
#define AGENT_BACKUP_STEP_ORACLENATIVE_DISMOUNT_H
#include <vector>
#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"

static const mp_string STEPNAME_ORACLE_NATIVEDISMOUNT = "TaskStepOracleNativeDismount";
class TaskStepOracleNativeDismount : public TaskStepOracleNative {
public:
    TaskStepOracleNativeDismount(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeDismount();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    // 0-clean, 1-dismount
    mp_int32 taskType;
    mp_int32 storType;
    mp_string recoverPath;

    // user and key are used in iscsi CHAP or cifs authentication
    mp_string authUser;
    mp_string authKey;
    Json::Value storageInfo;
    std::vector<mp_string> ipList;
    mp_string logSharePath;
    mp_string dataSharePath;
    
    mp_int32 BuildScriptParam(mp_string& strParam);
    mp_int32 ParseVolumeParameter(const Json::Value& param);
};

#endif
