/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepScanDisk.h
 * @brief  Contains function declarations for TaskStepScanDisk
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_SCANDISK_H
#define AGENT_BACKUP_STEP_SCANDISK_H
#include "common/Types.h"
#include "taskmanager/TaskStep.h"

static const mp_string STEPNAME_SCANDISK = "TaskStepScanDisk";
class TaskStepScanDisk : public TaskStep {
public:
    TaskStepScanDisk(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepScanDisk();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Cancel(Json::Value& respParam);
    mp_int32 Stop(const Json::Value& param);
    mp_int32 Redo(mp_string& innerPID);
    mp_int32 Update(const Json::Value& param);
    mp_int32 Update(Json::Value& param, Json::Value& respParam);
    mp_int32 Finish(const Json::Value& param);
    mp_int32 Finish(Json::Value& param, Json::Value& respParam);
};

#endif
