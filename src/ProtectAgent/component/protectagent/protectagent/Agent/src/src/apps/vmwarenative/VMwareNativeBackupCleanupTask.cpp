/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeBackupCleanupTask.cpp
 * @author w00558987
 * @brief 提供VMwareNative高级备份资源清理功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#include "apps/vmwarenative/VMwareNativeBackupCleanupTask.h"
#include "apps/vmwarenative/TaskStepVMwareNative.h"
#include "apps/vmwarenative/TaskStepUmountVMwareNasMedia.h"

using namespace std;
VMwareNativeBackupCleanupTask::VMwareNativeBackupCleanupTask(const mp_string &taskID) : VMwareNativeTask(taskID)
{
    m_taskName = "VMwareNativeBackupCleanupTask";
    // obtain backend storage type of current task: 0-nas, 1-iscsi
    mp_int32 iRet = TaskContext::GetInstance()->GetValueInt32(m_taskId, KEY_STORAGE_PROTOCOL, m_storageProtocol);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Task(%s) get backend storage type failed.", m_taskId.c_str());
    }

    CreateTaskStep();
}

VMwareNativeBackupCleanupTask::~VMwareNativeBackupCleanupTask()
{}

mp_int32 VMwareNativeBackupCleanupTask::InitTaskStep(const Json::Value &param)
{
    mp_int32 iRet = MP_FAILED;
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI == m_storageProtocol) {
        iRet = ReleaseIscsiMedia(param);
    } else if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_NAS == m_storageProtocol) {
        iRet = UmountBackendNasMedia(param);
    } else {
        ERRLOG("Unknow backend storage type: '%d', task id '%s'.", m_storageProtocol, m_taskId.c_str());
        return iRet;
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init backend storage media failed, task id '%s'.", m_taskId.c_str());
        return iRet;
    }

    return iRet;
}

mp_void VMwareNativeBackupCleanupTask::CreateTaskStep()
{
    if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_ISCSI == m_storageProtocol) {
    } else if (VMWAREDEF::VMWARE_STORAGE_PROTOCOL_NAS == m_storageProtocol) {
        ADD_TASKSTEP(
            TaskStepUmountVMwareNasMedia, STEPNAME_UMOUNT_VMWARENASMEDIA, TASK_STEP_INTERVAL_HUNDERED, m_steps);
    }
}

mp_int32 VMwareNativeBackupCleanupTask::ReleaseIscsiMedia(const Json::Value &param)
{
    mp_int32 ret = MP_SUCCESS;
    return ret;
}

mp_int32 VMwareNativeBackupCleanupTask::UmountBackendNasMedia(const Json::Value &param)
{
    // trigger nas media preparation
    if (MP_SUCCESS != InitTaskStepParam(param, "", STEPNAME_UMOUNT_VMWARENASMEDIA)) {
        ERRLOG("Init TaskStepUmountVMwareNasMedia failed, task id '%s'.", m_taskId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}
