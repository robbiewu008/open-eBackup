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
#ifndef _RE_REGISTER_HOST_H_
#define _RE_REGISTER_HOST_H_

#include "common/Types.h"
#include "common/Utils.h"

class ReRegisterHost {
public:
    ReRegisterHost() { }
    ~ReRegisterHost() { }
    mp_int32 Handle();

private:
    mp_int32 InitParam();
    mp_int32 GenerateAgentIP(bool bUseManager);
    mp_int32 GetChoice();
    mp_int32 SetNewNetParam();
    mp_void RollbackNetParam();

private:
    std::vector<mp_string> m_srcIpv4List;
    std::vector<mp_string> m_srcIpv6List;

    mp_string m_pmIpList;
    mp_int32 m_pmPort = 25082;
    mp_string m_pmManagerIpList;
    mp_int32 m_pmManagerPort = 25081;
    bool m_bUseManager = false;
    mp_string m_nginxIp;

    mp_string m_oldPmIpList;
    mp_int32 m_oldPmPort = 25082;
    mp_string m_oldNginxIp;
    mp_int32 m_oldNginxPort = 59526;
};

#endif