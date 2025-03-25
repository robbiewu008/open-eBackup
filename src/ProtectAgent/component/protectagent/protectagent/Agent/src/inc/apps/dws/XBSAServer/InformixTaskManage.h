/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 *
 * Description: app task manager.
 * Create: 2023-10-22
 * Author: jwx966562
*/
#ifndef INFORMIX_TASK_MANAGE_H_
#define INFORMIX_TASK_MANAGE_H_

#include "apps/dws/XBSAServer/AppTaskManage.h"

class InformixTaskManage : public AppTaskManage {
public:
    InformixTaskManage() {}
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc); // 备份
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc); // 恢复
    mp_void AllocFilesystem(BsaObjInfo &objInfo);
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, QueryObjectResult &rsp);
    mp_bool FillQuryRsp(mp_long bsaHandle, const BsaObjInfo &queryReslt, GetNextQueryObjectResult &rsp);

private:
    mp_int32 GetTaskInfoLockedInner(); // 解析任务信息
    mp_int32 GetInstanceName(const mp_string& pathName, mp_string& instanceName);
};

#endif