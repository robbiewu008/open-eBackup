/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file ExternalPluginParse.h
 * @brief  The implemention about ExternalPluginParse.h
 * @version 1.1.0
 * @date 2024-07-16
 * @author dengbowen d30043983
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