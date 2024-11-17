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
#include "apps/vmwarenative/TaskStepRecoveryPreparation.h"
#include "plugins/DataProcessClientHandler.h"
#include "message/tcp/CDppMessage.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Path.h"

TaskStepRecoveryPreparation::TaskStepRecoveryPreparation(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

TaskStepRecoveryPreparation::~TaskStepRecoveryPreparation()
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

mp_int32 TaskStepRecoveryPreparation::Init(const Json::Value &param)
{
    m_reqMsgToDataProcess = param;
    m_stepStatus = STATUS_INITIAL;

    return MP_SUCCESS;
}

mp_int32 TaskStepRecoveryPreparation::Run()
{
    mp_int32 iRet = DataProcessLogic(m_reqMsgToDataProcess, m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION,
        EXT_CMD_VMWARENATIVE_RECOVERY_PREPARATION_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }
    // set VDDK initization status to taskcontext
    TaskContext::GetInstance()->SetValueString(m_taskId, VMWAREDEF::PARAM_VDDKLIB_INIT_STATUS, "true");
    return iRet;
}
