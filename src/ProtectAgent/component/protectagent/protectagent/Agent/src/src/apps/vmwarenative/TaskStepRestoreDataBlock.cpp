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
#include "apps/vmwarenative/TaskStepRestoreDataBlock.h"
#include "plugins/DataProcessClientHandler.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/JsonUtils.h"
namespace {
const mp_int32 INNER_RESTORE_FINISHDISK_CMDNO = 1096;
const mp_int32 RECOVERY_THREAD_SLEEP_MILLISECONDS = 5000;
}

TaskStepRestoreDataBlock::TaskStepRestoreDataBlock(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_bDiskRestoreCompleted = MP_FALSE;
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
}

TaskStepRestoreDataBlock::~TaskStepRestoreDataBlock()
{
    m_bDiskRestoreCompleted = MP_FALSE;
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
}

mp_int32 TaskStepRestoreDataBlock::Init(const Json::Value &param)
{
    m_stepStatus = STATUS_INITIAL;
    m_reqMsgToDataProcess = param;

    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }

    // obtain each api's invoking time
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_DATAPROCESS_SECTION, CFG_API_INVOKING_TIME_INTERVAL, m_timeInterval);
    if (iRet != MP_SUCCESS) {
        m_timeInterval = KEY_APIINVOKE_TIMEINTERVAL_VALUE;
        COMMLOG(OS_LOG_WARN,
            "Unable to obtain api invoking time interval value from config file, will use default value: '%d'",
            m_timeInterval);
    }

    iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_DATAPROCESS_SECTION, CFG_THREAD_SLEEP_MILLISECONDS, m_threadSleepMilliSeconds);
    if (iRet != MP_SUCCESS) {
        m_threadSleepMilliSeconds = RECOVERY_THREAD_SLEEP_MILLISECONDS;
        COMMLOG(OS_LOG_WARN,
            "Unable to obtain thread sleep millisecond value from config file, will use default value: '%d'",
            m_threadSleepMilliSeconds);
    }

    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 TaskStepRestoreDataBlock::Run()
{
    mp_int32 iRet = MP_FAILED;
    m_stepStatus = STATUS_INPROGRESS;

    iRet = DataProcessLogic(m_reqMsgToDataProcess,
        m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK,
        EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    // let current thread keep running until the vm restore was completed
    INFOLOG("Vm restore task '%s' is completed: '%d'.", m_taskId.c_str(), m_bDiskRestoreCompleted);
    while (!m_bDiskRestoreCompleted) {
        // monitor current task's interface invoking time interval
        mp_uint64 currentTime = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
        if (m_invokedTime > 0 && currentTime - m_invokedTime > m_timeInterval) {
            ERRLOG("Task '%s' timeout in '%d' seconds, current time '%ld', will trigger disk restore finish request.",
                m_taskId.c_str(), m_timeInterval, currentTime);
            // send disk restore finish request to dp service
            InvokeFinishRequest();
        }
        DoSleep(m_threadSleepMilliSeconds);
    }

    return iRet;
}

mp_int32 TaskStepRestoreDataBlock::Stop(const Json::Value &param)
{
    m_bDiskRestoreCompleted = MP_TRUE;
    m_stepStatus = STATUS_COMPLETED;
    return MP_SUCCESS;
}

// both restore and query resotre progress will step into this function
mp_int32 TaskStepRestoreDataBlock::Update(Json::Value &param, Json::Value &respParam)
{
    mp_int32 iRet = MP_FAILED;
    m_stepStatus = STATUS_INPROGRESS;

    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        iRet = MP_FAILED;
        m_stepStatus = STATUS_FAILED;
        return iRet;
    }

    // obtain api invoking time
    mp_string strInvokedTime;
    iRet = TaskContext::GetInstance()->GetValueString(m_taskId, KEY_APIINVOKE_TIMESTAMP, strInvokedTime);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Task %s context has no key %s.", m_taskId.c_str(), KEY_APIINVOKE_TIMESTAMP.c_str());
        m_stepStatus = STATUS_FAILED;
        return iRet;
    } else {
        m_invokedTime = atol(strInvokedTime.c_str());
    }

    // get request command
    mp_int32 iReqCmd = 0;
    GET_JSON_INT32(param, MANAGECMD_KEY_CMDNO, iReqCmd);
    switch (iReqCmd) {
        case MANAGE_CMD_NO_VMWARENATIVE_RUN_RECOVERY:
            iRet = DataProcessLogic(param,
                respParam,
                EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK,
                EXT_CMD_VMWARENATIVE_RUN_RECOVERY_DATABLOCK_ACK);
            break;
        case MANAGE_CMD_NO_VMWARENATIVE_QUERY_RECOVERY_PROGRESS: {
            GET_JSON_STRING(param, PARAM_KEY_VOLUME_DISKID, m_strDiskId);
            GET_JSON_STRING(param, MANAGECMD_KEY_PARENT_TASKID, m_strParentTaskId);
            iRet = DataProcessLogic(param,
                respParam,
                EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS,
                EXT_CMD_VMWARENATIVE_QUERY_RECOVERYPROGRESS_ACK);

            break;
        }
        default:
            ERRLOG("Unknown request command '%d' of task '%s'.", iReqCmd, m_taskId.c_str());
            iRet = MP_FAILED;
            break;
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        m_stepStatus = STATUS_FAILED;
        return iRet;
    }

    return iRet;
}

mp_int32 TaskStepRestoreDataBlock::Cancel(Json::Value &respParam)
{
    m_bDiskRestoreCompleted = MP_TRUE;
    m_stepStatus = STATUS_FAILED;
    return MP_SUCCESS;
}

// should distingush datablock level or vm level restore finish
mp_int32 TaskStepRestoreDataBlock::Finish(Json::Value &param, Json::Value &respParam)
{
    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        m_bDiskRestoreCompleted = MP_TRUE;
        m_stepStatus = STATUS_FAILED;
        return MP_FAILED;
    }

    // get request command
    mp_int32 iReqCmd = 0;
    GET_JSON_INT32(param, MANAGECMD_KEY_CMDNO, iReqCmd);
    mp_int32 iRet = DataProcessLogic(param,
        respParam,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKRECOVERY_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        m_bDiskRestoreCompleted = MP_TRUE;
        m_stepStatus = STATUS_FAILED;
        return iRet;
    }

    // update state flags
    m_bDiskRestoreCompleted = MP_TRUE;
    m_stepStatus = STATUS_COMPLETED;
    return iRet;
}

mp_void TaskStepRestoreDataBlock::InvokeFinishRequest()
{
    ERRLOG("Error occurs in disk level task '%s'[parent task '%s'], will trigger finish action.",
        m_taskId.c_str(),
        m_strParentTaskId.c_str());
    Json::Value reqContent;
    Json::Value respParam;
    reqContent["FinishDisk"] = true;
    reqContent[PARAM_KEY_VOLUME_DISKID] = m_strDiskId;
    reqContent[MANAGECMD_KEY_TASKID] = m_taskId;
    reqContent[MANAGECMD_KEY_PARENT_TASKID] = m_strParentTaskId;
    reqContent[MANAGECMD_KEY_CMDNO] = INNER_RESTORE_FINISHDISK_CMDNO;
    Finish(reqContent, respParam);
}
