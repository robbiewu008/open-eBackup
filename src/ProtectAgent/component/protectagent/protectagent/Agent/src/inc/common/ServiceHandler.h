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
#ifndef _SERVICE_HANDLER_HANDLER_H_
#define _SERVICE_HANDLER_HANDLER_H_

#ifdef WIN32
#include "common/Defines.h"

static const mp_int32 START_SERVICE_TIMEOUT = 60000;  // 1000 * 60
static const mp_int32 STOP_SERVICE_TIMEOUT  = 60000;  // 1000 * 60

static const mp_int32 WIN_SERVICE_PRARM_NUM = 5;

class AGENT_API CWinServiceHanlder {
public:
    static mp_bool InstallService(
        mp_string strBinPath, mp_string strServcieName, mp_string strWorkingUser, mp_string strWorkingUserPwd);
    static mp_bool UninstallService(mp_string strServcieName);
    static mp_void UpdateServiceStatus(SERVICE_STATUS_HANDLE hServiceStatus, DWORD dwState, DWORD dwTimeOut);
    static mp_bool ChangeServiceUser(const mp_string &strServcieName, const mp_string &strWorkingUser,
        const mp_string &strWorkingUserPwd);

private:
    static mp_bool IsServiceExist(mp_string strServcieName);
};

#endif
#endif
