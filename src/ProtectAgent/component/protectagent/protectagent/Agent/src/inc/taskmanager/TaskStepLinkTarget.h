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
