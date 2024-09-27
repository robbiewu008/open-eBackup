/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepPrepareTargetLun.h
 * @brief  Contains function declarations TaskStepPrepareTargetLun
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
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
