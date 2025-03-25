/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepInitVMwareDiskLib.h
 * @brief  Contains function declarations for TaskStepInitVMwareDiskLib
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H
#define AGENT_BACKUP_STEP_VMWARENATIVE_INIT_VMWAREDISKLIB_H

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_INIT_VMWAREDISKLIB = "TaskStepInitVMwareDiskLib";
class TaskStepInitVMwareDiskLib : public TaskStepVMwareNative {
public:
    TaskStepInitVMwareDiskLib(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepInitVMwareDiskLib();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
