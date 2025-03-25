/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 * Description: 备份/恢复任务CLeanup VDDK步骤
 * Author: w00558987
 * Version: 0.1
 * Create: 2021-03-29
 *
 */

#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_CLEANUP_VMWAREDISKLIB_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_CLEANUP_VMWAREDISKLIB_H

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_CLEANUP_VMWAREDISKLIB = "TaskStepCleanUpVMwareDiskLib";
class TaskStepCleanUpVMwareDiskLib : public TaskStepVMwareNative {
public:
    TaskStepCleanUpVMwareDiskLib(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepCleanUpVMwareDiskLib();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
