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
#ifndef __AGENT_UNIQUEID_H__
#define __AGENT_UNIQUEID_H__

#include "common/Defines.h"
#include "common/CMpThread.h"

// 临时文件id相关
class AGENT_API CUniqueID {
public:
    static CUniqueID& GetInstance();
    ~CUniqueID()
    {
        CMpThread::DestroyLock(&m_uniqueIDMutex);
    }
    mp_void Init()
    {}
    mp_string GetString();
    mp_ulong GetInt();
    mp_string GetTimestamp();

private:
    CUniqueID()
    {
        m_iUniqueID = 0;
        CMpThread::InitLock(&m_uniqueIDMutex);
    }
    static CUniqueID m_instance;  // 单例对象
    mp_uint64 m_iUniqueID;
    thread_lock_t m_uniqueIDMutex;
};

#endif  // __AGENT_UNIQUEID_H__
