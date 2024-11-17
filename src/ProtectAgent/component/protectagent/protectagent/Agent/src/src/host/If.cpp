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
#ifdef WIN32
#pragma comment(lib, "Iphlpapi.lib")
#include <winsock2.h>
#include <Iphlpapi.h>
#include <windows.h>
#else
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#ifdef AIX
#include <arpa/inet.h>
#elif defined SOLARIS
#include <sys/sockio.h>
#endif // SOLARIS
#include <net/if.h>
#include <net/if_arp.h>
#include <unistd.h>
#include <netdb.h>
#endif // WIN32
#include "securec.h"
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Ip.h"
#include "common/Utils.h"
#include "common/ErrorCode.h"
#include "host/If.h"

using namespace std;
namespace {
const mp_char ADDRESS_2 = 2;
const mp_char ADDRESS_3 = 3;
const mp_char ADDRESS_4 = 4;
const mp_char ADDRESS_5 = 5;
const mp_char ADDRESS_6 = 6;
}

#define PRINT_ERROR_INFO_RETURN(iErr, szErr, format) do { \
    iErr = GetOSError();  \
    COMMLOG(OS_LOG_ERROR, format, iErr, GetOSStrErr(iErr, szErr, sizeof(szErr))); \
    return MP_FAILED; \
} while (0)

#define PRINT_ERROR_INFO_CONTINUE(iErr, szErr, format, intrface) { \
    iErr = GetOSError();  \
    COMMLOG(OS_LOG_ERROR, format, iErr, GetOSStrErr(iErr, szErr, sizeof(szErr))); \
    ++(intrface); \
    continue; \
}

#ifdef WIN32
/*------------------------------------------------------------
Description  :获得所有网卡信息，支持Windows/Linux
Output       : ifs --- 保存查询到的网卡信息
Return       : MP_SUCCESS --- 成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIf::GetAllIfInfo(vector<if_info_t>& ifs)
{
    mp_int32 iErr;
    mp_char szErr[ERR_INFO_SIZE] = {0};
    mp_char buf[BUF_LEN] = {0};

    PIP_ADAPTER_INFO  pAdapterInfo = (PIP_ADAPTER_INFO)malloc(sizeof(IP_ADAPTER_INFO));
    if (NULL == pAdapterInfo) {
        PRINT_ERROR_INFO_RETURN(iErr, szErr, "Malloc failed, errno[%d]:%s.");
    }

    ULONG uiLen = sizeof(IP_ADAPTER_INFO);      // get adapter list
    DWORD dwRetVal = GetAdaptersInfo(pAdapterInfo, &uiLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        free(pAdapterInfo);
        pAdapterInfo = (PIP_ADAPTER_INFO)malloc(uiLen);
        if (pAdapterInfo == NULL) {
            PRINT_ERROR_INFO_RETURN(iErr, szErr, "Malloc failed, errno[%d]:%s.");
        }

        dwRetVal = GetAdaptersInfo(pAdapterInfo, &uiLen);
    }

    if (dwRetVal != NO_ERROR) {
        free(pAdapterInfo);
        COMMLOG(OS_LOG_ERROR, "Call GetAdaptersInfo api failed, ret %d", dwRetVal);
        PRINT_ERROR_INFO_RETURN(iErr, szErr, "Call GetAdaptersInfo api failed, errno[%d]:%s.");
    }

    while (pAdapterInfo) {  // loop the list of adapter, get name, description, ip and mask
        if_info_t ifInfo;

        ifInfo.strName = pAdapterInfo->AdapterName;
        ifInfo.strDesc = pAdapterInfo->Description;
        ifInfo.strIp = pAdapterInfo->IpAddressList.IpAddress.String;
        ifInfo.strNetMask = pAdapterInfo->IpAddressList.IpMask.String;

        mp_int32 iRet = sprintf_s(buf, BUF_LEN, "%02X-%02X-%02X-%02X-%02X-%02X", pAdapterInfo->Address[0],
            pAdapterInfo->Address[1], pAdapterInfo->Address[ADDRESS_2], pAdapterInfo->Address[ADDRESS_3],
            pAdapterInfo->Address[ADDRESS_4], pAdapterInfo->Address[ADDRESS_5]);
        if (iRet == MP_FAILED) {
            free(pAdapterInfo);
            COMMLOG(OS_LOG_ERROR, "Call sprintf_s failed, ret %d.", iRet);
            return MP_FAILED;
        }
        ifInfo.strMac = buf;

        ifs.push_back(ifInfo);
        pAdapterInfo = pAdapterInfo->Next;
    }
    free(pAdapterInfo);

    return MP_SUCCESS;
}
#elif defined LINUX

#define PARSE_IFS_REDUCE_CC(fd, maclist) { \
    if (!ioctl(fd, SIOCGIFFLAGS, reinterpret_cast<mp_char*>(&maclist[intrface]))) {  \
        if (maclist[intrface].ifr_flags & IFF_LOOPBACK) { \
            ++intrface; \
            ifs.push_back(ifInfo); \
            continue; \
        } \
        if (!(ioctl(fd, SIOCGIFHWADDR, reinterpret_cast<mp_char*>(&maclist[intrface])))) { \
            for (mp_uint32 j = 0; j < ADDRESS_6; ++j) { \
                myhaddr[j] = (mp_uchar)(maclist[intrface].ifr_hwaddr.sa_data[j]); \
            } \
        } else { \
            PRINT_ERROR_INFO_CONTINUE(iErr, szErr, "Ioctl SIOCGIFHWADDR failed, errno[%d]:%s.", intrface);  \
        } \
    } else { \
        PRINT_ERROR_INFO_CONTINUE(iErr, szErr, "Execute ioctl SIOCGIFFLAGS failed, errno[%d]:%s.", intrface); \
    } \
}

mp_int32 CIf::ParseIfs(mp_int32 fd, std::vector<if_info_t> &ifs, struct ifreq maclist[], mp_int32 listLen,
    mp_uint32 num)
{
    mp_uint64 intrface = 0;
    mp_uchar myhaddr[ADDRESS_6] = {0};
    mp_char buf[BUF_LEN] = {0};
    mp_int32 iErr = 0;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    while (intrface < num) {
        if_info_t ifInfo;

        ifInfo.strName = maclist[intrface].ifr_name; // get nic name
        mp_uint32 uiAddr = ((struct sockaddr_in*)&maclist[intrface].ifr_addr)->sin_addr.s_addr; // get nic ip addr
        if (CIP::IPV4UIntToStr(uiAddr, ifInfo.strIp) != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Convert ip string failed.");
            ++intrface;
            continue;
        }

        struct ifreq ifReqTmp;  // get nic net mask
        CHECK_NOT_OK(memset_s(&ifReqTmp, sizeof(ifreq), 0x00, sizeof(ifreq)));
        CHECK_NOT_OK(memcpy_s(ifReqTmp.ifr_name, sizeof(ifReqTmp.ifr_name), maclist[intrface].ifr_name,
                              sizeof(maclist[intrface].ifr_name)));
        if (!ioctl(fd, SIOCGIFNETMASK, &ifReqTmp)) {
            mp_uint32 uiMask = ((struct sockaddr_in*)&ifReqTmp.ifr_addr)->sin_addr.s_addr;
            if (CIP::IPV4UIntToStr(uiMask, ifInfo.strNetMask) != MP_SUCCESS) {
                COMMLOG(OS_LOG_ERROR, "Convert mask string failed.");
                ++intrface;
                continue;
            }
        } else {
            PRINT_ERROR_INFO_CONTINUE(iErr, szErr, "Execute ioctl SIOCGIFNETMASK failed, errno[%d]:%s.", intrface)
        }

        PARSE_IFS_REDUCE_CC(fd, maclist)
        mp_int32 iRet = sprintf_s(buf, BUF_LEN, "%02X-%02X-%02X-%02X-%02X-%02X", myhaddr[0], myhaddr[1],
            myhaddr[ADDRESS_2], myhaddr[ADDRESS_3], myhaddr[ADDRESS_4], myhaddr[ADDRESS_5]);
        if (MP_FAILED == iRet) {
            close(fd);
            COMMLOG(OS_LOG_ERROR, "Call sprintf_s failed, ret %d.", iRet);
            return MP_FAILED;
        }

        ifInfo.strMac = buf;
        ifs.push_back(ifInfo);
        ++intrface;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :获得所有网卡信息，支持Windows/Linux；包括回环地址信息；
              AIX/Solaris/HPUX需要支持时需要修改该方法
Output       : ifs --- 保存查询到的网卡信息
Return       : MP_SUCCESS---成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIf::GetAllIfInfo(vector<if_info_t>& ifs)
{
    struct ifreq maclist[MAXINTERFACES];
    struct ifconf ifc;
    mp_int32 iErr = 0;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    CHECK_NOT_OK(memset_s(maclist, sizeof(maclist), 0x00, sizeof(maclist)));
    CHECK_NOT_OK(memset_s(&ifc, sizeof(struct ifconf), 0x00, sizeof(struct ifconf)));

    mp_int32 fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        PRINT_ERROR_INFO_RETURN(iErr, szErr, "Invoke socket api failed, errno[%d]:%s.");
    }

    ifc.ifc_len = sizeof(maclist);
    ifc.ifc_buf = (caddr_t) maclist;
    if (ioctl (fd, SIOCGIFCONF, reinterpret_cast<mp_char*>(&ifc)) != 0) { // get interface num
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "ioctl excute SIOCGIFCONF failed, errno[%d]:%s.",
                iErr, GetOSStrErr(iErr, szErr, sizeof(szErr)));
        close(fd);
        return MP_FAILED;
    }

    mp_uint32 num = static_cast<mp_uint32>(ifc.ifc_len) / sizeof(struct ifreq);
    if (ParseIfs(fd, ifs, maclist, MAXINTERFACES, num) != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "ParseIfs failed.");
        close(fd);
        return MP_FAILED;
    }

    close(fd);
    return MP_SUCCESS;
}
#else
mp_int32 CIf::GetAllIfInfo(vector<if_info_t>& ifs)
{
    (mp_void)ifs;
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
}
#endif

