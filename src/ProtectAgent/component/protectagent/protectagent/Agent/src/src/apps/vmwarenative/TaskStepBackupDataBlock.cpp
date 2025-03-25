/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file TaskStepBackupDataBlock.cpp
 * @brief  Contains function declarations for TaskStepBackupDataBlock
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "apps/vmwarenative/TaskStepBackupDataBlock.h"
#include <ctime>
#include "plugins/DataProcessClientHandler.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/JsonUtils.h"
namespace {
const mp_int32 INNER_BACKUP_FINISHDISK_CMDNO = 1084;
const mp_int32 BACKUP_THREAD_SLEEP_MILLISECONDS = 5000;
}
TaskStepBackupDataBlock::TaskStepBackupDataBlock(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_bDiskBackupCompleted = MP_FALSE;
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
}

TaskStepBackupDataBlock::~TaskStepBackupDataBlock()
{
    m_bDiskBackupCompleted = MP_FALSE;
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
}

mp_int32 TaskStepBackupDataBlock::Init(const Json::Value &param)
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
        m_threadSleepMilliSeconds = BACKUP_THREAD_SLEEP_MILLISECONDS;
        COMMLOG(OS_LOG_WARN,
            "Unable to obtain thread sleep millisecond value from config file, will use default value: '%d'",
            m_threadSleepMilliSeconds);
    }

    return MP_SUCCESS;
}

EXTER_ATTACK mp_int32 TaskStepBackupDataBlock::Run()
{
    mp_int32 iRet = MP_FAILED;
    m_stepStatus = STATUS_INPROGRESS;
    iRet = DataProcessLogic(m_reqMsgToDataProcess,
        m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK,
        EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    // let current thread keep running until the vm backup was completed
    INFOLOG("Vm backup task '%s' is completed: '%d'.", m_taskId.c_str(), m_bDiskBackupCompleted);
    while (!m_bDiskBackupCompleted) {
        // monitor current task's interface invoking time interval
        mp_uint64 currentTime = CMpTime::GetTimeUsec() / SECOND_AND_MICROSECOND_TIMES;
        if (m_invokedTime > 0 && currentTime - m_invokedTime > m_timeInterval) {
            ERRLOG("Task '%s' timeout in '%d' seconds, current time '%ld', will trigger disk backup finish request.",
                m_taskId.c_str(), m_timeInterval, currentTime);
            // send disk backup finish request to dp service
            InvokeFinishRequest();
        }
        DoSleep(m_threadSleepMilliSeconds);
    }

    return iRet;
}

mp_int32 TaskStepBackupDataBlock::Stop(const Json::Value &param)
{
    m_bDiskBackupCompleted = MP_TRUE;
    m_stepStatus = STATUS_COMPLETED;
    return MP_SUCCESS;
}

// both backup and query backup progress will step into this function
mp_int32 TaskStepBackupDataBlock::Update(Json::Value &param, Json::Value &respParam)
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

    mp_int32 iReqCmd = 0;
    GET_JSON_INT32(param, MANAGECMD_KEY_CMDNO, iReqCmd);

    switch (iReqCmd) {
        case MANAGE_CMD_NO_VMWARENATIVE_RUN_BACKUP:
            iRet = DataProcessLogic(param, respParam,
                EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK, EXT_CMD_VMWARENATIVE_RUN_BACKUP_DATABLOCK_ACK);
            break;
        case MANAGE_CMD_NO_VMWARENATIVE_QUERY_BACKUP_PROGRESS: {
            GET_JSON_STRING(param, PARAM_KEY_VOLUME_DISKID, m_strDiskId);
            GET_JSON_STRING(param, MANAGECMD_KEY_PARENT_TASKID, m_strParentTaskId);
            iRet = DataProcessLogic(param, respParam,
                EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS, EXT_CMD_VMWARENATIVE_QUERY_BACKUPPROGRESS_ACK);
            break;
        }
        default:
            ERRLOG("Unknown request command '%d', task id '%s'.", iReqCmd, m_taskId.c_str());
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


mp_int32 TaskStepBackupDataBlock::Cancel(Json::Value &respParam)
{
    m_bDiskBackupCompleted = MP_TRUE;
    m_stepStatus = STATUS_COMPLETED;
    return MP_SUCCESS;
}

// should distingush datablock level or vm level backup finish
mp_int32 TaskStepBackupDataBlock::Finish(Json::Value &param, Json::Value &respParam)
{
    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        m_bDiskBackupCompleted = MP_TRUE;
        m_stepStatus = STATUS_FAILED;
        return MP_FAILED;
    }

    // get request command of host agent
    mp_int32 iReqCmd = 0;
    GET_JSON_INT32(param, MANAGECMD_KEY_CMDNO, iReqCmd);
    mp_int32 iRet = DataProcessLogic(param, respParam,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP,
        EXT_CMD_VMWARENATIVE_FINISH_DATABLOCKBACKUP_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        m_bDiskBackupCompleted = MP_TRUE;
        m_stepStatus = STATUS_FAILED;
        return iRet;
    }

    // update state flags
    m_bDiskBackupCompleted = MP_TRUE;
    m_stepStatus = STATUS_COMPLETED;
    INFOLOG("task=%s is finished.", m_taskId.c_str());
    return iRet;
}

mp_void TaskStepBackupDataBlock::InvokeFinishRequest()
{
    ERRLOG("Error occurs in disk level task '%s'[parent task '%s'], will trigger finish action.", m_taskId.c_str(),
        m_strParentTaskId.c_str());
    Json::Value reqContent;
    Json::Value respParam;
    reqContent["FinishDisk"] = true;
    reqContent[PARAM_KEY_VOLUME_DISKID] = m_strDiskId;
    reqContent[MANAGECMD_KEY_TASKID] = m_taskId;
    reqContent[MANAGECMD_KEY_PARENT_TASKID] = m_strParentTaskId;
    reqContent[MANAGECMD_KEY_CMDNO] = INNER_BACKUP_FINISHDISK_CMDNO;
    Finish(reqContent, respParam);
}

