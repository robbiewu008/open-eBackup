/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 *
 * @file VMwareNativeBackupCleanupTask.h
 * @author w00558987
 * @brief 提供VMwareNative高级备份资源清理功能
 * @version 0.1
 * @date 2021-01-05
 *
 */

#ifndef AGENT_VMWARENATIVE_BACKUP_CLEANUP_TASK
#define AGENT_VMWARENATIVE_BACKUP_CLEANUP_TASK

#include "apps/vmwarenative/VMwareNativeTask.h"

class VMwareNativeBackupCleanupTask : public VMwareNativeTask {
public:
    VMwareNativeBackupCleanupTask(const mp_string &taskID);
    virtual ~VMwareNativeBackupCleanupTask();

    mp_int32 InitTaskStep(const Json::Value &param);

private:
    mp_void CreateTaskStep();
    mp_int32 UmountBackendNasMedia(const Json::Value &param);
    mp_int32 ReleaseIscsiMedia(const Json::Value &param);

private:
    mp_int32 m_storageProtocol;
};

#endif
