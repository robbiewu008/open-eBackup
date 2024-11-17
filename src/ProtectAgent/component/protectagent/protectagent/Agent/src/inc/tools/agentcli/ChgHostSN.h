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
#ifndef _AGENTCLI_CHGHOSTSN_H_
#define _AGENTCLI_CHGHOSTSN_H_

#include "common/Types.h"

#include <vector>

static const mp_uchar HOSTSN_LEN = 32;
#define INPUT_HOSTSN_CHG "Please Input HostSN:"

class ChgHostSN {
public:
    static mp_int32 Handle();

private:
    static mp_string m_ChghostsnFile;

private:
    static mp_int32 CheckUserPwd();
    static mp_int32 GetHostSNNum(mp_string& strInput);
    static mp_int32 ChownHostSn(mp_string& strInput);
    static mp_int32 ModifyHostSN(std::vector<mp_string>& vecResult, mp_string& strInput);
};

#endif  // _AGENTCLI_CHGHOSTSN_H_
