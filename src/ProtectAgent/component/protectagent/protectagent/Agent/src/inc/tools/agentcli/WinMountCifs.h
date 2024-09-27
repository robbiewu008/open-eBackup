/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2022. All rights reserved.
 *
 * @file WinMountCifs.h
 * @brief mount cifs on windows
 * @version 1.0.0
 * @date 2022-07-07
 * @author kWX884906
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