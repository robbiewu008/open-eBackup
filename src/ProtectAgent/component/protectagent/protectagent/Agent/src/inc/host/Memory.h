/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Memory.h
 * @brief  Contains function declarations for Memory
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_MEMORY_H__
#define __AGENT_MEMORY_H__

#include <vector>
#include "common/Types.h"
#include "common/Defines.h"

const mp_int32 KBYTES  = 1024;

typedef struct tag_memory_info {
    mp_uint64 uiMemPhyTotal;  // physical mem, kB
    mp_uint64 uiMemPhyFree;
    mp_uint64 uiMemPhyUsed;
    mp_uint64 uiMemVirTotal;  // virtual mem, kB
    mp_uint64 uiMemVirFree;
    mp_uint64 uiMemVirUsed;
} memory_info_t;

class AGENT_API CMemory {
public:
    static mp_int32 GetMemoryInfo(memory_info_t& memInfo);

private:
    static mp_int32 AnalyseMemInfo(mp_string& strMemInfo, memory_info_t& memInfo);
    static mp_int32 GetMemSize(mp_string& strMemInfo, mp_uint64& uiMemSize);
};

#endif // __AGENT_MEMORY_H__

