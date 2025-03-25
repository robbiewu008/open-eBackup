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
#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_BACKUP_DATABLOCK_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_BACKUP_DATABLOCK_H

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_BACKUP_DATABLOCK = "TaskStepBackupDataBlock";
class TaskStepBackupDataBlock : public TaskStepVMwareNative {
public:
    TaskStepBackupDataBlock(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepBackupDataBlock();

    mp_int32 Init(const Json::Value &param);
    EXTER_ATTACK mp_int32 Run();
    mp_int32 Cancel(Json::Value &respParam);
    mp_int32 Stop(const Json::Value &param);
    mp_int32 Update(Json::Value &param, Json::Value &respParam);
    mp_int32 Finish(Json::Value &param, Json::Value &respParam);

private:
    mp_void InvokeFinishRequest();
    mp_bool m_bDiskBackupCompleted;
    mp_string m_strParentTaskId;
    mp_string m_strDiskId;
    mp_long m_invokedTime;
    mp_int32 m_timeInterval;
    mp_int32 m_threadSleepMilliSeconds;
};

#endif
