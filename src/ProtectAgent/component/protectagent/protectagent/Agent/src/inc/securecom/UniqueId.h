/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file UniqueId.h
 * @brief  Contains function declarations UniqueId
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
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
