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

