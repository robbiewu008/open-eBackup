/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepRecoveryPreparation.h
 * @brief  Contains function declarations TaskStepRecoveryPreparation
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
 */
#ifndef __AGENT_TASKSTEP_RECOVERY_PREPARATION_H__
#define __AGENT_TASKSTEP_RECOVERY_PREPARATION_H__

#include "apps/vmwarenative/TaskStepVMwareNative.h"

static const mp_string STEPNAME_RECOVERY_PREPARATION = "TaskStepRecoveryPreparation";
class TaskStepRecoveryPreparation : public TaskStepVMwareNative {
public:
    TaskStepRecoveryPreparation(
        const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order);
    ~TaskStepRecoveryPreparation();

    mp_int32 Init(const Json::Value &param);
    mp_int32 Run();
};

#endif
