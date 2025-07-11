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
#ifndef EXTERNAL_FILECLIENT_MANAGER_H
#define EXTERNAL_FILECLIENT_MANAGER_H

#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include "common/Types.h"
#include "common/Log.h"
#include "common/Defines.h"

class ExternalFileClientManager {
public:
    static ExternalFileClientManager& GetInstance()
    {
        static ExternalFileClientManager m_Instance;
        return m_Instance;
    }
    mp_int32 Init();
    mp_int32 GetFileClientPid()
    {
        return m_FileClientPid;
    }
    mp_void SetFileClientPid(mp_int32 fileClientPid)
    {
        m_FileClientPid = fileClientPid;
    }
    mp_int32 AcquirePidByProcess();
    mp_int32 AcquireFileClientConfig();
    mp_void SetCGroupByConfig();

private:
    mp_int32 m_FileClientPid = 0;
    mp_int32 m_cpuLimit = -1;     // 单位1%，用于设置 cpu.cfs_quotas_us 缺省值 -1
    mp_int32 m_memoryLimit = -1;  // 单位M，用于设置 memory.limit_in_bytes 缺省值 -1
};

#endif  // EXTERNAL_FILECLIENT_MANAGER_H