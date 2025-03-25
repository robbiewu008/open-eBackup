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
#ifndef AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_H
#define AGENT_BACKUP_STEP_ORACLE_EXPIRE_COPY_H
#include "common/Types.h"
#include "apps/oraclenative/TaskStepOracleNative.h"
#include "apps/oraclenative/TaskStepOracleNativeExpireCopy.h"

static const mp_string STEPNAME_NATIVE_EXPIRECOPY = "TaskStepOracleNativeExpireCopy";
class TaskStepOracleNativeExpireCopy : public TaskStepOracleNative {
public:
    TaskStepOracleNativeExpireCopy(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeExpireCopy();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

private:
    mp_string maxTime;
    mp_string minTime;
    mp_string maxScn;
    mp_string minScn;
    mp_int32 BuildScriptParam(mp_string& strParam);
};

#endif
