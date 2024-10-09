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
#include "LocalCmdExector.h"
#include "trjsontostruct.h"
#include "log/Log.h"
#include "common/Defines.h"
#include "JobControl.h"

using namespace GeneralDB;
using namespace AppProtect;
namespace {
    const mp_string MODULE_NAME = "JobControl";
    const mp_string ABORT_JOB = "AbortJob";
    const mp_string PAUSE_JOB = "PauseJob";
    const mp_string JOB = "job";
    const mp_string PROTECT_OBJECT = "protectObject";
}

mp_void JobControl::AbortJob(ActionResult& returnValue, const Param& prm)
{
    LOGGUARD("");

    Param shellParam;
    shellParam.param = prm.param;
    shellParam.appType = prm.appType;
    shellParam.cmdType = ABORT_JOB;
    shellParam.jobId = prm.jobId;
    shellParam.subJobId = prm.subJobId;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value result;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(shellParam.appType, prm.param[JOB][PROTECT_OBJECT],
        shellParam.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(shellParam, result, nullptr) != MP_SUCCESS) {
        ERRLOG("Running abort script failed, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
        returnValue.code = INNER_ERROR;
        returnValue.message = "Running AbortJob script failed.";
        return;
    }
    JsonToStruct(result, returnValue);
}

mp_void JobControl::PauseJob(ActionResult& returnValue, const Param& prm)
{
    LOGGUARD("");

    Param shellParam;
    shellParam.param = prm.param;
    shellParam.appType = prm.appType;
    shellParam.cmdType = PAUSE_JOB;
    shellParam.jobId = prm.jobId;
    shellParam.subJobId = prm.subJobId;
    shellParam.isAsyncInterface = MP_FALSE;

    Json::Value result;
    LocalCmdExector::GetInstance().GetGeneralDBScriptDir(shellParam.appType, prm.param[JOB][PROTECT_OBJECT],
        shellParam.scriptDir);
    if (LocalCmdExector::GetInstance().Exec(shellParam, result, nullptr) != MP_SUCCESS) {
        ERRLOG("Running pause script failed, jobId=%s, subJobId=%s.", prm.jobId.c_str(), prm.subJobId.c_str());
        returnValue.code = INNER_ERROR;
        returnValue.message = "Running PauseJob script failed.";
        return;
    }
    JsonToStruct(result, returnValue);
}
