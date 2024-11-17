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
#ifndef __AGENT_SECURE_IP_H__
#define __AGENT_SECURE_IP_H__

#include "common/Defines.h"
#include <vector>

namespace SecureCom {
class AGENT_API CIP
{
public:
    static mp_int32 CheckHostLinkStatus(const std::vector<mp_string>& hostIpv4List,
    const std::vector<mp_string>& hostIpv6List, std::vector<mp_string>& srcIpv4List,
    std::vector<mp_string>& srcIpv6List);

private:
    static mp_bool CheckHostLinkStatus(const mp_string& strSrcIP, const std::vector<mp_string>& hostIpList);
};
}

#endif
