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
#ifndef UNIQUEID_H
#define UNIQUEID_H

#include "common/Thread.h"
#include "define/Types.h"
#include "common/Defines.h"

namespace GeneralDB {
class UniqueId {
public:
    static UniqueId& GetInstance()
    {
        static UniqueId uniqueIdInstance;
        return uniqueIdInstance;
    }
    ~UniqueId()
    {
    }
    mp_void Init()
    {}
    /**
     *  @brief 生成唯一Pid 有时间搓+随机数组成，格式为"111111111_111"
     *  @param uniqueId [OUT] 生成的pid
     *  @return 错误码 MP_SUCCESS 成功 MP_FAILED 失败
     * */
    mp_int32 GenerateUniqueID(mp_string &uniqueId);
private:
    UniqueId()
    {
        m_iUniqueID = 0;
    }
    mp_int32 GetRandom(mp_uint64& num);
    /**
     *  @brief 获取当前时间戳
     *  @return 生成的当前时间戳
     * */
    mp_string GetTimestamp();
    mp_int32 m_iUniqueID;
    Module::thread_lock_t m_uniqueIDMutex;
};
}
#endif
