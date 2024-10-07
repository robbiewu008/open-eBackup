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
#ifndef UUID_H
#define UUID_H

#include <string>
#include <uuid/uuid.h>

namespace VirtPlugin {
class Uuid {
public:
    /**
     *  @brief 生成UUID
     *
     *  @param void
     *  @return 输出格式为 "CD042510-B090-4E2B-9AA0-FE43C0F991E0"的UUID字符串
     */
    static std::string GenerateUuid();
};
}

#endif // UUID_H