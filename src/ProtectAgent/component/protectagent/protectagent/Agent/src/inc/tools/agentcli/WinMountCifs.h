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
#ifndef _WIN_MOUNT_CIFS_H_
#define _WIN_MOUNT_CIFS_H_

#include "common/Types.h"
#include "common/Utils.h"

#ifdef WIN32
class WinMountCifs {
public:
    WinMountCifs()
    {
        ZeroMemory(&stStartupInfo, sizeof(stStartupInfo));
        stStartupInfo.cb = sizeof(stStartupInfo);
        stStartupInfo.dwFlags = STARTF_USESHOWWINDOW;
        stStartupInfo.wShowWindow = SW_HIDE;

        ZeroMemory(&stProcessInfo, sizeof(stProcessInfo));
    }
    ~WinMountCifs()
    {
        CloseHandle(stProcessInfo.hThread);
        CloseHandle(stProcessInfo.hProcess);
    }
    mp_int32 Handle(const mp_string& strCMD);

private:
    mp_int32 WriteInput();
    mp_int32 GetPwd(mp_string &strPwd);

private:
    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = { 0 };
    STARTUPINFO stStartupInfo;
    PROCESS_INFORMATION stProcessInfo;
};
#endif // WIN32

#endif