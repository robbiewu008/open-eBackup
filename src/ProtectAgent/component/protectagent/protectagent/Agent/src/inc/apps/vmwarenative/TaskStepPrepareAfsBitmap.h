/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2024-2024. All rights reserved.
 *
 * @file TaskStepPrepareAfsBitmap.h
 * @brief  Contains function declarations TaskStepPrepareAfsBitmap
 * @version 1.0.0
 * @date
 * @author
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
