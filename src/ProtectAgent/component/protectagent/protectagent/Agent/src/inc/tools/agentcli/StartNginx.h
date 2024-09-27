/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file StartNginx.h
 * @brief  Contains function declarations StartNginx
 * @version 1.0.0
 * @date 2020-06-27
 * @author wangguitao 00510599
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
