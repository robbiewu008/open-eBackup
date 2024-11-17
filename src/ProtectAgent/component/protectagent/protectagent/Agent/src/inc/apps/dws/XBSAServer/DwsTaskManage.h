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
#ifndef DWS_TASK_MANAGE_H_
#define DWS_TASK_MANAGE_H_
#include "apps/dws/XBSAServer/AppTaskManage.h"

class DwsTaskManage : public AppTaskManage {
public:
    DwsTaskManage() : AppTaskManage() {}
    mp_int32 UpdateTaskWhenCreateObject(const BsaObjectDescriptor &objDesc) override;
    mp_int32 UpdateTaskWhenQueryObject(const BsaQueryDescriptor &objDesc) override;
    mp_void AllocFilesystem(BsaObjInfo &objInfo) override;
private:
    mp_int32 GetTaskInfoLockedInner();
    mp_int32 GetDwsHosts();
    mp_string GetDwsHostDbFilePath();
    mp_string ParseDwsHostname(const mp_string &objName);
    mp_void UpdateDwsHosts();
private:
    std::map<mp_string, FsKeyInfo> m_dwsHostMap;
};

#endif