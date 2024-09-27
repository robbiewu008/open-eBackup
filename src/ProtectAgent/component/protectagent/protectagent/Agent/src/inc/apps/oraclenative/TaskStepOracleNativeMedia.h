/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepOracleNativeMedia.h
 * @brief  Contains function declarations for TaskStepOracleNativeMedia
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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

