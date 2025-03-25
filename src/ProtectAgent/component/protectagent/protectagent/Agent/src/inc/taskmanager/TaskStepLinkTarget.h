/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepLinkTarget.h
 * @brief  Contains function declarations for TaskStepLinkTarget
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef AGENT_BACKUP_STEP_LINKTARGET_H
#define AGENT_BACKUP_STEP_LINKTARGET_H
#include <vector>
#include "common/Types.h"
#include "taskmanager/TaskStep.h"

typedef enum AuthenticationMode_ {
    AUTH_MODE_NONE = 0,
    AUTH_MODE_SIMPLEX,
    AUTH_MODE_DUPLEX,
    AUTH_MODE_BUTT,
} AuthenticationMode;

static const mp_string STEPNAME_LINKTARGET = "TaskStepLinkTarget";
class TaskStepLinkTarget : public TaskStep {
public:
    TaskStepLinkTarget(
        const mp_string& id, const mp_string& taskId, const mp_string& name, mp_int32 ratio, mp_int32 order);
    ~TaskStepLinkTarget();
    mp_int32 Init(const Json::Value& param);
    mp_int32 Run();
    mp_int32 Cancel();
    mp_int32 Cancel(Json::Value &respParam);
    mp_int32 Redo(mp_string& innerPID);
    mp_int32 Stop(const Json::Value& param);
    mp_int32 Update(const Json::Value& param);
    mp_int32 Update(Json::Value& param, Json::Value& respParam);
    mp_int32 Finish(const Json::Value& param);
    mp_int32 Finish(Json::Value& param, Json::Value& respParam);

private:
    mp_string scsiTargetIp;
    mp_uint16 scsiTargetPort;
    AuthenticationMode normalMode;
    AuthenticationMode discoverMode;
    mp_string chapName;
    mp_string chapPwd;
    Json::Value scsiTargets;
};

#endif
