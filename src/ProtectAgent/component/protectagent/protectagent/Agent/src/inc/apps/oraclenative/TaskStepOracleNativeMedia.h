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
#ifndef AGENT_BACKUP_STEP_PREPARE_MEDIA_H
#define AGENT_BACKUP_STEP_PREPARE_MEDIA_H
#include <vector>

#include "apps/oraclenative/TaskStepOracleNative.h"
#include "common/Types.h"
#include "device/BackupMedium.h"

class TaskStepOracleNativeMedia : public TaskStepOracleNative {
public:
    TaskStepOracleNativeMedia(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepOracleNativeMedia();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();

protected:
    BackupMedium backupMedium;

    // initial data or logs information
    virtual mp_int32 InitialVolumes(mp_string& diskList);
    // after scan disk, check the volumes whether it is valid
    mp_int32 PrepareBackupMedium(const mp_string& diskList);

    virtual mp_int32 PrepareMediumInfo();
};

#endif

