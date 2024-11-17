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

