/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Password.h
 * @brief  The implemention 信号量处理类
 * @version 1.0.0.0
 * @date 2019-11-15
 * @author yangwenjun 00275736
 */
#ifndef __AGENT_SEMAF_H__
#define __AGENT_SEMAF_H__

#include "common/Types.h"
#include "common/Defines.h"

class AGENT_API CSemaf
{
public:
    static mp_int32 Init(mp_semaf& semaf);
    static mp_int32 Release(mp_semaf* pSemaf);
    static mp_int32 Wait(mp_semaf* pSemaf);
};

#endif // __AGENT_SEMAF_H__

