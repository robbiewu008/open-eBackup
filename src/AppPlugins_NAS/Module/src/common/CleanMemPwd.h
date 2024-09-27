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
#ifndef MODULE_CLEAN_MEM_PWD_H
#define MODULE_CLEAN_MEM_PWD_H

#include <string>
#include <unordered_map>
#include "define/Types.h"
#include "define/Defines.h"

namespace Module {

template<typename T>
inline void FreeContainer(T& p_container)
{
    T empty;
    using std::swap;
    swap(p_container, empty);
}

AGENT_API void CleanMemoryPwd(std::string& pwd);

AGENT_API void CleanMemoryPwd(std::wstring& pwd);

} // namespace Module

#endif