#include "apps/vmwarenative/TaskStepBackupCloseDisk.h"
#include "plugins/DataProcessClientHandler.h"
#include "message/tcp/CDppMessage.h"
#include "common/JsonUtils.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Path.h"

TaskStepBackupCloseDisk::TaskStepBackupCloseDisk(const mp_string &id, const mp_string &taskId, const mp_string &name,
    mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_respMsg.clear();
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
    m_timeInterval = 0;
}

TaskStepBackupCloseDisk::~TaskStepBackupCloseDisk()
{
    m_respMsg.clear();
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_invokedTime = 0;
    m_timeInterval = 0;
}

mp_int32 TaskStepBackupCloseDisk::Init(const Json::Value &param)
{
    m_stepStatus = STATUS_INITIAL;
    m_reqMsgToDataProcess = param;

    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 TaskStepBackupCloseDisk::Run()
{
    mp_int32 iRet = DataProcessLogic(m_reqMsgToDataProcess,
                                     m_respMsg,
                                     EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK,
                                     EXT_CMD_VMWARENATIVE_BACKUP_CLOSEDISK_ACK);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Data process logic exec failure, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    DBGLOG("Run taskStep, respMsgFromDataProcess");
    return iRet;
}
