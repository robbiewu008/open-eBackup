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
#ifndef _AGENTCLI_STARTNGINX_H_
#define _AGENTCLI_STARTNGINX_H_

#include <vector>
#include "common/Types.h"

class StartNginx {
public:
    static mp_int32 Handle(const mp_string& actionType);

private:
    static mp_int32 ExecNginxStart();
    static mp_int32 ReloadNginx();
    static mp_int32 NginxStartHandle(const mp_string& strNginxPWDFile, std::vector<mp_string>& vecInput);
    static mp_int32 GetPassword(mp_string& CipherStr);

#ifdef WIN32
    static DWORD WINAPI WriteParamFunc(void* pPipe);
    static int SetEnvWithWin(mp_string& envVal);
#else
    static mp_void* WriteParamFunc(void* pPipe);
#endif
private:
    static mp_string m_startNginxMode;
};

#endif
