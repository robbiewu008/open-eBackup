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