#include "apps/vmwarenative/VMwareNativeBackupPreparationTask.h"

#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "taskmanager/TaskStepLinkTarget.h"
#include "taskmanager/TaskStepScanDisk.h"
#include "apps/vmwarenative/TaskStepPrepareVMwareNasMedia.h"
#include "apps/vmwarenative/TaskStepBackupPreparation.h"

using namespace std;
VMwareNativeBackupPreparationTask::VMwareNativeBackupPreparationTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeBackupPreparationTask";
    // obtain backend storage type of current task: 0-nas, 1-iscsi
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_STORAGE_PROTOCOL, m_storageProtocol);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Task(%s) get backend storage type failed.", m_taskId.c_str());
    }

    iRet = TaskContext::GetInstance()->GetValueString(m_taskId, KEY_DISK_TYPE, m_diskType);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Task(%s) get disk type failed.", m_taskId.c_str());
    }

    CreateTaskStep();
}

VMwareNativeBackupPreparationTask::~VMwareNativeBackupPreparationTask()
{}

mp_int32 VMwareNativeBackupPreparationTask::InitTaskStep(const Json::Value &param)
{
    mp_int32 iRet = MP_FAILED;
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI == m_storageProtocol || VMWAREDEF::VMWARE_RDM_DISK == m_diskType) {
        iRet = PrepareIscsiMedia(param);
    }
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_NAS == m_storageProtocol) {
        iRet = PrepareNasMedia(param);
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init backend storage media failed or unknown backend storage type: '%d', task id '%s'.",
               m_storageProtocol, m_taskId.c_str());
        return iRet;
    }

    // trigger preparation task step
    iRet = InitTaskStepParam(param, "", STEPNAME_BACKUP_PREPARATION);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Unable to init recovery preparation task step, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    return iRet;
}

mp_void VMwareNativeBackupPreparationTask::CreateTaskStep()
{
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI == m_storageProtocol || VMWAREDEF::VMWARE_RDM_DISK == m_diskType) {
        ADD_TASKSTEP(TaskStepLinkTarget, STEPNAME_LINKTARGET, TASK_STEP_INTERVAL_TEN, m_steps);
        ADD_TASKSTEP(TaskStepScanDisk, STEPNAME_SCANDISK, TASK_STEP_INTERVAL_TEN, m_steps);
    }
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_NAS == m_storageProtocol) {
        ADD_TASKSTEP(
            TaskStepPrepareVMwareNasMedia, STEPNAME_PREPARE_VMWARENASMEDIA, TASK_STEP_INTERVAL_TWENTY, m_steps);
    }
    ADD_TASKSTEP(TaskStepBackupPreparation, STEPNAME_BACKUP_PREPARATION, TASK_STEP_INTERVAL_EIGHTY, m_steps);
}

mp_int32 VMwareNativeBackupPreparationTask::PrepareIscsiMedia(const Json::Value &param)
{
    static const mp_string KEY_SCSITARGET = "storage";

    // trigger iscsi target linking task step
    if (MP_SUCCESS != InitTaskStepParam(param, KEY_SCSITARGET, STEPNAME_LINKTARGET)) {
        ERRLOG("Init TaskStepLinkTarget failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }

    // trigger lun scanning task step
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_SCANDISK)) {
        ERRLOG("Init TaskStepScanDisk failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 VMwareNativeBackupPreparationTask::PrepareNasMedia(const Json::Value &param)
{
    // trigger nas media preparation
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_PREPARE_VMWARENASMEDIA)) {
        ERRLOG("Init TaskStepPrepareVMwareNasMedia failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
