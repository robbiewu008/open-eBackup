#include "apps/vmwarenative/TaskStepPrepareAfsBitmap.h"

#include "common/ErrorCode.h"
#include "common/JsonUtils.h"
#include "common/Path.h"
#include "common/Utils.h"
#include "message/tcp/CDppMessage.h"
#include "plugins/DataProcessClientHandler.h"

TaskStepPrepareAfsBitmap::TaskStepPrepareAfsBitmap(
    const mp_string &id, const mp_string &taskId, const mp_string &name, mp_int32 ratio, mp_int32 order)
    : TaskStepVMwareNative(id, taskId, name, ratio, order)
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
    m_nfsMediaObj = std::make_shared<TaskStepPrepareVMwareNasMedia>(id, taskId, name, ratio, order);
}

TaskStepPrepareAfsBitmap::~TaskStepPrepareAfsBitmap()
{
    m_reqMsgToDataProcess.clear();
    m_respMsgFromDataProcess.clear();
}

mp_int32 TaskStepPrepareAfsBitmap::Init(const Json::Value &param)
{
    m_reqMsgToDataProcess = param;

    // check whether the VDDK lib is inited
    if (!IsVddkLibInited()) {
        ERRLOG("The VDDK lib has not been inited, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }

    if (!m_nfsMediaObj) {
        ERRLOG("The VMwareNasMedia is nullptr, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    // 初始化存储信息
    if (m_nfsMediaObj->Init(param) != MP_SUCCESS) {
        ERRLOG("The VMwareNasMedia init failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareAfsBitmap::UmountNasMedia()
{
    mp_string strServiceType = m_nfsMediaObj->m_accessNASOverFC ? "vmware_fc" : "vmware";
    std::vector<mp_string> vecRst;
    std::ostringstream ossParam;
    ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_STORAGE_IP << STR_EQUAL
             << m_nfsMediaObj->GetNasStorageIP() << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << STR_EQUAL
             << m_nfsMediaObj->GetNasSharePath() << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL
             << m_nfsMediaObj->GetParentTaskId() << NODE_COLON << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId
             << NODE_COLON;
    if (!m_nfsMediaObj->GetNasStorageIP().empty() &&
        m_nfsMediaObj->UmountNasMedia(ossParam.str(), vecRst) != MP_SUCCESS) {
        WARNLOG("Unable to umount current control nas storage '%s'.", m_nfsMediaObj->GetNasStorageIP().c_str());
        ossParam.str("");
        vecRst.clear();
        ossParam << SERVICE_TYPE << STR_EQUAL << strServiceType << NODE_COLON << VMWAREDEF::KEY_STORAGE_IP << STR_EQUAL
                 << m_nfsMediaObj->GetOtherStorageIP() << NODE_COLON << VMWAREDEF::KEY_NAS_FILESYSTEM_NAME << STR_EQUAL
                 << m_nfsMediaObj->GetNasSharePath() << NODE_COLON << VMWAREDEF::KEY_PARENTTASK_ID << STR_EQUAL
                 << m_nfsMediaObj->GetParentTaskId() << NODE_COLON << VMWAREDEF::KEY_BACKUP_ID << STR_EQUAL << m_taskId
                 << NODE_COLON;
        if (!m_nfsMediaObj->GetOtherStorageIP().empty() &&
            m_nfsMediaObj->UmountNasMedia(ossParam.str(), vecRst) != MP_SUCCESS) {
            ERRLOG("Unable to umount backend nas storage '%s'.", m_nfsMediaObj->GetOtherStorageIP().c_str());
            return ERROR_DEVICE_NAS_MOUNT_FAILED;
        }
    }
    return MP_SUCCESS;
}

mp_int32 TaskStepPrepareAfsBitmap::Run()
{
    // 实现挂载NFS设备
    if (m_nfsMediaObj->Run() != MP_SUCCESS) {
        ERRLOG("The VMwareNasMedia::Run failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    mp_int32 iRet = DataProcessLogic(m_reqMsgToDataProcess,
        m_respMsgFromDataProcess,
        EXT_CMD_VMWARENATIVE_BACKUP_AFS,
        EXT_CMD_VMWARENATIVE_BACKUP_AFS_ACK);
    if (iRet != MP_SUCCESS) {
        std::string errMsg;
        GET_JSON_STRING(m_respMsgFromDataProcess, MANAGECMD_KEY_ERRORDETAIL, errMsg);
        if (!errMsg.empty()) {
            TaskContext::GetInstance()->SetValueString(m_taskId, KEY_ERRMSG, errMsg);
        }
        ERRLOG("Data process logic exec failure, task id '%s', errMsg:%s.", m_taskId.c_str(), errMsg.c_str());
    } else {
        INFOLOG("Send bodyMsg: %s", m_respMsgFromDataProcess.toStyledString().c_str());
        m_respMsg = m_respMsgFromDataProcess;
    }
    // 卸载NFS
    (void)UmountNasMedia();
    DBGLOG("Run exit TaskStepPrepareAfsBitmap");
    return iRet;
}
