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
#include "common/CleanMemPwd.h"

namespace Module {

using namespace std;

namespace {
    const int sensitiveDateWipeTimes = 3;
}

void CleanMemoryPwd(string & pwd)
{
    /* 敏感数据处理，pwd进行3次覆写 */
    for (int loop = 0; loop < sensitiveDateWipeTimes; loop++) {
        for (string::size_type i = 0; i < pwd.size(); ++i)
        {
            pwd[i] = (char)0xcc;
        }
    }
}

void CleanMemoryPwd(wstring & pwd)
{
    for (int loop = 0; loop < sensitiveDateWipeTimes; loop++) {
        for (wstring::size_type i = 0; i < pwd.size(); ++i)
        {
            pwd[i] = (char)0xcc;
        }
    }
}

} // namespace Module