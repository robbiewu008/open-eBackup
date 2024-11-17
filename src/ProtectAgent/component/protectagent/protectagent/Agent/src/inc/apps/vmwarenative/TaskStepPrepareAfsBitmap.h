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
#ifndef __AGENT_TASKSTEP_BACKUP_AFSBITMAP_H__
#define __AGENT_TASKSTEP_BACKUP_AFSBITMAP_H__

#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "TaskStepPrepareVMwareNasMedia.h"

static const mp_string STEPNAME_BACKUP_AFSBITMAP = "TaskStepPrepareAfsBitmap";

class TaskStepPrepareAfsBitmap : public TaskStepVMwareNative {
public:
    TaskStepPrepareAfsBitmap(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    virtual ~TaskStepPrepareAfsBitmap();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();

protected:
    mp_int32 UmountNasMedia();
    std::shared_ptr<TaskStepPrepareVMwareNasMedia> m_nfsMediaObj;
};

#endif
