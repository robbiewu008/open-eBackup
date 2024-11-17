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
#ifndef AGENTCLI_CHAGNEIP_H
#define AGENTCLI_CHAGNEIP_H

#include <vector>
#include "common/Types.h"
#include "common/Ip.h"
#ifdef WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <unistd.h>
#endif

#define RESTART_WEB_SERVICE_HINT "Restart the web service to reload the configuration file."
#define INPUT_IP_HINT "Input new IP address:"
#define INVALID_IP_ADDRESS "Invalid IP address."
#define NOT_LOCAL_IP_ADDRESS "Input IP address is not local."
#define SAME_IP_ADDRESS "Input IP address is as same as current IP address."
#define CHANGE_IP_SUCCESS \
    "Ip address of Nginx changed successfully, and it will be taken effect after Agent was started."

static const mp_string LOCAL_IP_ADDRESS = "127.0.0.1";
static const mp_string LOCAL_IP6_ADDRESS = "::1";
static const mp_string EIGHT_BALNK = "        ";
static const mp_uchar  MAX_IP_COUNT = 200;

class ChangeIP {
public:
    static mp_int32 Handle();

private:
    static mp_int32 GetIPAddress(mp_string& strIP);
    static mp_int32 SetIPAddress(const mp_string& strIP);
    static mp_int32 GetLocalIPs(std::vector<mp_string>& vecIPs);
    static mp_bool IsLocalIP(const mp_string& strIP);
    static mp_int32 RestartNginx();
    static mp_int32 ChangeIPAfter(mp_string& strInput);
#ifdef WIN32
    static mp_int32 GetAdapterAddr(PIP_ADAPTER_ADDRESSES& pCurrAddresses);
    static mp_int32 GetLocalIPsWin(std::vector<mp_string>& vecIPs);
#elif defined LINUX
    static mp_int32 GetLocalIPsLinux(std::vector<mp_string>& vecIPs);
#else
    static mp_int32 GetLocalIPsOther(std::vector<mp_string>& vecIPs);
#endif

private:
    static const mp_uchar CHANGEIP_NUM_100 = 100;
};

#endif
