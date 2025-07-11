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
#ifndef __AGENT_IF_H__
#define __AGENT_IF_H__

#include <vector>
#include <iostream>
#include <algorithm>
#include "common/Types.h"
#include "common/Defines.h"
#ifndef WIN32
#include <net/if.h>
#endif
const mp_int32 BUF_LEN = 256;
const mp_int32 MAXINTERFACES  = 16;

typedef struct tag_if_info {
    mp_string strName;
    mp_string strDesc;  // for windows only, the description of the adapter.
    mp_string strIp;
    mp_string strNetMask;
    mp_string strMac;
}if_info_t;


class CIf {
public:
    static mp_int32 GetAllIfInfo(std::vector<if_info_t>& ifs);

private:
    static mp_int32 ParseIfs(mp_int32 fd, std::vector<if_info_t> &ifs, struct ifreq maclist[], mp_int32 listLen,
        mp_uint32 num);
};

#endif // __AGENT_IF_H__

