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
#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_TARGETLUN_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_PREPARE_TARGETLUN_H

#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "apps/vmwarenative/VMwareDef.h"

static const mp_string STEPNAME_PREPARE_TARGETLUN = "TaskStepPrepareTargetLun";
class TaskStepPrepareTargetLun : public TaskStepVMwareNative {
public:
    TaskStepPrepareTargetLun(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepPrepareTargetLun();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
