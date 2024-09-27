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
#ifndef __TASK_H__
#define __TASK_H__

#include "common/Types.h"

class Task {
public:
    Task() : m_result(0){};
    virtual ~Task() = default;

    /**
     *  @brief  任务执行体
     */
    virtual void Exec(){};
    
    /**
     *  @brief  任务执行结果
     */
    inline mp_int32 Result()
    {
        return m_result;
    }

protected:
    mp_int32 m_result;
};

#endif