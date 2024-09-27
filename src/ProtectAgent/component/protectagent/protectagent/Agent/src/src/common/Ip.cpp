/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Ip.cpp
 * @brief  Contains function declarations Get Ip Info
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */

#include "common/Ip.h"
#include <sstream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <string>
#if defined WIN32
#include <ws2tcpip.h>
#include <iomanip>
#elif defined LINUX
#include <ifaddrs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif
#include <cstdio>
#include <cstdlib>
#include "common/Log.h"
#include "common/Path.h"
#include "common/File.h"
#include "common/Utils.h"
#include "common/CSystemExec.h"
#include "message/tcp/CSocket.h"

using namespace std;
bool CIP::isIPV4 = false;
namespace {
const mp_uchar IP_NUM_2 = 2;
const mp_uchar IP_NUM_4 = 4;
const mp_uchar IP_NUM_7 = 7;
const mp_uchar IP_NUM_16 = 16;
const mp_uchar IP_NUM_46 = 46;
const mp_uchar IP_NUM_255 = 255;
const mp_string CIP_DORADO_ENVIRONMENT = "0";
const mp_string CIP_IN_AGENT_TYPE = "1";
const mp_string CIP_ENVIRONMENT_TYPE = "ENVIRONMENT_TYPE";
const int CIP_ENVIRONMENT_TYPE_MINISIZE = 2;
const mp_string HOST_ENV_DEPLOYTYPE = "DEPLOY_TYPE";
const mp_string HOST_ENV_HOSTNAME = "MY_POD_NAME";
const mp_string DOMAIN_NAME_SUFFIX = ".protectengine.dpa.svc.cluster.local";
const mp_string EIPINFO = "EIP=";
const mp_string FLOATING_IP = "FLOATING_IP";
const mp_string AVAILABLE_ZONE = "AVAILABLE_ZONE";
const mp_string IS_SHARED = "IS_SHARED";
const mp_string INSTALLATION_MODE = "INSTALLATION_MODE";
}  // namespace

mp_int32 CIP::GetHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    mp_int32 iRet = MP_SUCCESS;
#ifdef WIN32
    mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(1, 1);
    if (WSAStartup(wVersionRequested, &wsaData) != 0) {
        mp_int32 err = GetOSError();
        ERRLOG("WSAStartup failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1) {
        ERRLOG("WSADATA wVersion failed, LOBYTE=%d, HIBYTE=%d.", LOBYTE(wsaData.wVersion), HIBYTE(wsaData.wVersion));
        WSACleanup();
        return MP_FAILED;
    }
    iRet = GetWindowsHostIPList(ipv4List, ipv6List);
    WSACleanup();
#elif defined LINUX
    iRet = GetLinuxHostIPList(ipv4List, ipv6List);
#else
    iRet = GetUnixHostIPList(ipv4List, ipv6List);
#endif
    if (iRet != MP_SUCCESS || (ipv4List.empty() && ipv6List.empty())) {
        ERRLOG("GetHostIPList failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

#ifdef WIN32
mp_int32 CIP::GetWindowsHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
    // ipv4
    char hostname[IP_NUM_255] = { 0 };
    gethostname(hostname, sizeof(hostname));
    PHOSTENT hostinfo = gethostbyname(hostname);
    if (hostinfo == nullptr) {
        mp_int32 err = GetOSError();
        ERRLOG("gethostbyname failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        WSACleanup();
        return MP_FAILED;
    }
    while (*(hostinfo->h_addr_list) != nullptr) {
        const char* ip = inet_ntoa(*(struct in_addr *) *hostinfo->h_addr_list);
        ipv4List.push_back(ip);
        hostinfo->h_addr_list++;
    }

    // ipv6
    struct addrinfo hint = { AI_PASSIVE, AF_INET6, SOCK_STREAM, 0, 0, nullptr, nullptr, nullptr };
    struct addrinfo *ailist = nullptr;
    const char port[] = "10086";
    if (getaddrinfo(hostname, port, &hint, &ailist) < 0 || ailist == nullptr) {
        mp_int32 err = GetOSError();
        ERRLOG("getaddrinfo failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        WSACleanup();
        return MP_FAILED;
    }

    struct sockaddr_in6 *sinp6;
    for (addrinfo* aip = ailist; NULL != aip; aip = aip->ai_next) {
        aip->ai_family = AF_INET6;
        sinp6 = (struct sockaddr_in6 *)aip->ai_addr;
        if (nullptr == sinp6 && NULL != sinp6) {
            continue;
        }
        char addressBuffer[IP_NUM_46];
        inet_ntop(AF_INET6, sinp6, addressBuffer, IP_NUM_46);
        if (strcmp(addressBuffer, "::") != 0 && strcmp(addressBuffer, "::1") != 0) {  // like 127.0.0.1, 0.0.0.0
            COMMLOG(OS_LOG_DEBUG, "find ipv6 :%s.", addressBuffer);
            mp_string strAddressBuffer = addressBuffer;
            ipv6List.emplace_back(strAddressBuffer);
        }
    }
    return MP_SUCCESS;
}
#elif defined LINUX
mp_int32 CIP::GetLinuxHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    struct ifaddrs* ifAddr = nullptr;
    struct ifaddrs* ifAddrHead = nullptr;
    void* tmpAddrPtr = nullptr;
    if (getifaddrs(&ifAddr) != 0) {
        mp_int32 err = GetOSError();
        mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
        ERRLOG("getifaddrs failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    // save the origin pointer
    ifAddrHead = ifAddr;
    while (ifAddr != nullptr) {
        if ((ifAddr->ifa_addr != nullptr) && (ifAddr->ifa_addr->sa_family == AF_INET)) {  // check it is IP4
            // is a valid IP4 Address
            tmpAddrPtr = &((struct sockaddr_in*)ifAddr->ifa_addr)->sin_addr;
            char addressBuffer[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, tmpAddrPtr, addressBuffer, INET_ADDRSTRLEN);
            if ((strcmp(addressBuffer, "127.0.0.1") != 0) && (strcmp(addressBuffer, "0.0.0.0") != 0)) {
                COMMLOG(OS_LOG_DEBUG, "find ipv4 :%s.", addressBuffer);
                mp_string strAddressBuffer = addressBuffer;
                ipv4List.emplace_back(strAddressBuffer);
            }
        } else if ((ifAddr->ifa_addr != nullptr) && (ifAddr->ifa_addr->sa_family == AF_INET6)) {  // check it is IP6
            // is a valid IP6 Address
            tmpAddrPtr = &((struct sockaddr_in6*)ifAddr->ifa_addr)->sin6_addr;
            char addressBuffer[INET6_ADDRSTRLEN];
            inet_ntop(AF_INET6, tmpAddrPtr, addressBuffer, INET6_ADDRSTRLEN);
            if (strcmp(addressBuffer, "::") != 0 && strcmp(addressBuffer, "::1") != 0) {  // like 127.0.0.1, 0.0.0.0
                COMMLOG(OS_LOG_DEBUG, "find ipv6 :%s.", addressBuffer);
                mp_string strAddressBuffer = addressBuffer;
                ipv6List.emplace_back(strAddressBuffer);
            }
        }
        ifAddr = ifAddr->ifa_next;
    }

    if (ifAddrHead) {
        freeifaddrs(ifAddrHead);
        ifAddrHead = NULL;
    }
    return MP_SUCCESS;
}
#else
mp_int32 CIP::GetUnixHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List)
{
    mp_char szErr[MAX_ERROR_MSG_LEN] = { 0 };
    static const mp_int32 ipv4charNum = 5;
    static const mp_int32 ipv6charNum = 6;
    FILE* fp = popen("ifconfig -a", "r");
    if (fp == NULL) {
        mp_int32 err = GetOSError();
        ERRLOG("popen failed, errno[%d]:%s.", err, GetOSStrErr(err, szErr, sizeof(szErr)));
        return MP_FAILED;
    }
    while (!feof(fp)) {
        char tmpBuf[2000] = { 0 };
        fgets(tmpBuf, sizeof(tmpBuf), fp);
        if (strlen(tmpBuf) >= 0) {
            tmpBuf[strlen(tmpBuf) - 1] = 0;
        }
        char* temp = NULL;
        bool bFlag = (tmpBuf[0] == 0) || (tmpBuf[0] == '\n');
        if (bFlag) {
            continue;
        }
        if (temp = strstr(tmpBuf, "inet ")) {
            temp = temp + ipv4charNum;
            mp_string str(temp);
            mp_int32 pos = str.find(" ");
            mp_string ipv4 = str.substr(0, pos);
            if (ipv4 == "127.0.0.1" || ipv4 == "0.0.0.0") {
                continue;
            }
            ipv4List.push_back(ipv4);
        }
        if (temp = strstr(tmpBuf, "inet6 ")) {
            temp = temp + ipv6charNum;
            mp_string str(temp);
            mp_int32 pos = str.find("%");
            mp_string ipv6 = str.substr(0, pos);
            ipv6List.push_back(ipv6);
        }
    }
    pclose(fp);
    return MP_SUCCESS;
}
#endif

/* ------------------------------------------------------------
Description  :判断IPV4地址是否正确
Input        :      strIpAddr---IP地址
Return       :   MP_TRUE---IPV4正确
                  MP_FALSE---IPV4错误
------------------------------------------------------------- */
mp_bool CIP::IsIPV4(const mp_string& strIpAddr)
{
    vector<mp_string> vecOut;
    SplitIPV4(strIpAddr, vecOut);
    if (vecOut.size() != IP_NUM_4) {
        return MP_FALSE;
    }

    mp_int32 i255 = 0;
    mp_int32 i0 = 0;
    for (vector<mp_string>::iterator it = vecOut.begin(); it != vecOut.end(); ++it) {
        if (!IsNumber(it->c_str())) {
            return MP_FALSE;
        }
        mp_int32 iValue = atoi(it->c_str());
        if (iValue < 0 || iValue > IP_NUM_255) {
            return MP_FALSE;
        }
        if (iValue == 0) {
            i0++;
        }
        if (iValue == IP_NUM_255) {
            i255++;
        }
    }

    // 全0或全255返回错误
    if (i0 == IP_NUM_4 || i255 == IP_NUM_4) {
        return MP_FALSE;
    }

    // 其他判断
    return MP_TRUE;
}

mp_int32 CIP::GetUIntIpAddr(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    in_addr addr;
    const mp_int32 notValidIp = -2;
    mp_int32 iRet = inet_pton(AF_INET, strIpAddr.c_str(), &addr);
    if (iRet != 1) {
        if (iRet == 0) {
            return notValidIp;
        } else {  // return -1
            COMMLOG(OS_LOG_ERROR, "Convert ipv4 string to numeric failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
    }
    CHECK_NOT_OK(memcpy_s(&uiIpAddr, sizeof(uiIpAddr), &addr, sizeof(addr)));
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV4地址字符串转换为无符号整形，支持windows/linux，unix需要时进行扩展并测试
Input        : strIpAddr -- IPV4字符串
Output       : uiIpAddr -- 保存转换之后的无符号整形数字
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 失败
               -2 -- 待转换的ipv4字符串格式非法
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIP::IPV4StrToUInt(mp_string& strIpAddr, mp_uint32& uiIpAddr)
{
    if (GetUIntIpAddr(strIpAddr, uiIpAddr) != MP_SUCCESS) {
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV4地址字符串转换为无符号整形，支持windows/linux，unix需要时进行扩展并测试
Input        : strIpAddr -- IPV4字符串
               uiLen -- pIpAddr缓冲区长度
Output       : pIpAddr -- 保存转换之后的无符号整形数字
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 失败
               -2 -- 待转换的ipv4字符串格式非法
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIP::IPV4StrToUInt(mp_string& strIpAddr, mp_void* pIpAddr, mp_uint32 uiLen)
{
    const mp_int32 notValidIp = -2;
#ifdef WIN32
    in_addr addr;
    mp_int32 iRet = inet_pton(AF_INET, strIpAddr.c_str(), &addr);
    if (iRet != 1) {
        if (iRet == 0) {
            return notValidIp;
        } else {
            COMMLOG(OS_LOG_ERROR, "Convert ipv4 string to numeric failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
    }
    CHECK_NOT_OK(memcpy_s(pIpAddr, uiLen, &addr, sizeof(addr)));

#else
    if (uiLen < IPV4_NUMERIC_LEN) {
        COMMLOG(OS_LOG_ERROR, "IpAddr buff is less than 4 bypes.");
        return MP_FAILED;
    }

    in_addr addr;
    mp_int32 iRet = inet_pton(AF_INET, strIpAddr.c_str(), &addr);
    if (iRet != 1) {
        if (iRet == 0) {
            return notValidIp;
        } else {
            COMMLOG(OS_LOG_ERROR, "Convert ipv4 string to numeric failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
    }
    CHECK_NOT_OK(memcpy_s(pIpAddr, static_cast<size_t>(uiLen), &addr, sizeof(addr)));
#endif
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV4地址无符号整形转换为字符串，支持windows/linux，unix需要时进行扩展并测试
Input        : uiIpAddr -- 待转换的IPV4无符号整形
Output       : strIpAddr -- 保存转换之后的字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CIP::IPV4UIntToStr(mp_uint32 uiIpAddr, mp_string& strIpAddr)
{
    mp_char str[INET_ADDRSTRLEN] = {0};
    const char* ptr = inet_ntop(AF_INET, &uiIpAddr, str, sizeof(str));
    if (ptr == NULL) {
        COMMLOG(OS_LOG_ERROR, "Convert ipv4 numeric to string failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    strIpAddr = ptr;
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV4地址无符号整形转换为字符串，支持windows/linux，unix需要时进行扩展并测试
Input        : uiIpAddr -- 待转换的IP无符号整形指针
Output       : strIpAddr -- 保存转换之后的字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CIP::IPV4UIntToStr(mp_void* pIpAddr, mp_string& strIpAddr)
{
    mp_char str[INET_ADDRSTRLEN] = {0};
    const char* ptr = inet_ntop(AF_INET, pIpAddr, str, sizeof(str));
    if (ptr == NULL) {
        COMMLOG(OS_LOG_ERROR, "Convert ipv6 numeric to string failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }

    strIpAddr = ptr;
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV6地址字符串转换为无符号整形，支持windows/linux，unix需要时进行扩展并测试
Input        : strIpAddr -- IPV6字符串
               uiLen -- uiIpAddr缓冲区长度
Output       : uiIpAddr -- 保存转换之后的无符号整形数字
Return       : MP_SUCCESS -- 成功
               MP_FAILED -- 失败
               -2 -- 待转换的ipv4字符串格式非法
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIP::IPV6StrToUInt(mp_string& strIpAddr, mp_void* pIpAddr, mp_uint32 uiLen)
{
    // at least 16 bytes
    if (uiLen < IPV6_NUMERIC_LEN) {
        COMMLOG(OS_LOG_ERROR, "IpAddr buff is less than 16 bypes.");
        return MP_FAILED;
    }

    in6_addr addr;
    const mp_int32 notValidIp = -2;
    mp_int32 iRet = inet_pton(AF_INET6, strIpAddr.c_str(), &addr);
    if (iRet != 1) {
        if (iRet == 0) {
            return notValidIp;
        } else {
            COMMLOG(OS_LOG_ERROR, "Convert ipv6 string to numeric failed, errno[%d]:%s.", errno, strerror(errno));
            return MP_FAILED;
        }
    }
    CHECK_NOT_OK(memcpy_s(pIpAddr, static_cast<size_t>(uiLen), &addr, sizeof(addr)));
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : IpV6地址无符号整形转换为字符串，支持windows/linux，unix需要时进行扩展并测试
Input        : uiIpAddr -- 待转换的IPV6无符号整形
Output       : strIpAddr -- 保存转换之后的字符串
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CIP::IPV6UIntToStr(mp_void* pIpAddr, mp_string& strIpAddr)
{
    mp_char str[INET6_ADDRSTRLEN] = { 0 };
    const char* ptr = inet_ntop(AF_INET6, pIpAddr, str, sizeof(str));
    if (NULL == ptr) {
        COMMLOG(OS_LOG_ERROR, "Convert ipv6 numeric to string failed, errno[%d]:%s.", errno, strerror(errno));
        return MP_FAILED;
    }
    strIpAddr = ptr;
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  :分割IPV4
Input        :      strIpAddr---IP地址
Output       :    vecOutput---输出分割后的值


------------------------------------------------------------- */
mp_void CIP::SplitIPV4(const mp_string& strIpAddr, vector<mp_string>& vecOutput)
{
    mp_string strIp = strIpAddr;
    mp_string::size_type pos = strIp.find(DELIM);
    while (pos != mp_string::npos) {
        mp_string strOut = strIp.substr(0, pos);
        strIp = strIp.substr(pos + 1);
        pos = strIp.find(DELIM);
        vecOutput.emplace_back(strOut);
    }
    vecOutput.push_back(strIp);
}

/* ------------------------------------------------------------
Description  :判断传入的字符串是否是10进制的
                  只包含'0'-'9'的数字
Input        :      str为传入的字符串
Return       :    如果是只包含'0'-'9'的十进制则返回MP_TRUE


------------------------------------------------------------- */
mp_bool CIP::IsNumber(mp_string str)
{
    if (str.size() == 0) {
        return MP_FALSE;
    }

    if (str.size() > 1 && str[0] == '0') {
        return MP_FALSE;
    }

    for (mp_string::iterator iter = str.begin(); iter != str.end(); ++iter) {
        if (*iter < '0' || *iter > '9') {
            return MP_FALSE;
        }
    }

    return MP_TRUE;
}

/* ------------------------------------------------------------
Description  : 检查IP是否为IPv6格式
Input        : ip -- 需要校验的IP地址
Return       : true: IPv6, false: 其他
------------------------------------------------------------- */
mp_bool CIP::IsIPv6(const std::string& ip)
{
    mp_int32 len = ip.length();
    mp_int32 count_colon = 0;  // 计数冒号
    mp_int32 count_bit = 0;    // 计数每组位数，初始化为 0
    for (mp_int32 i = 0; i < len; i++) {
        mp_int32 bFlag = (ip[i] < 'a' || ip[i] > 'f') && (ip[i] < 'A' || ip[i] > 'F') && (ip[i] < '0' || ip[i] > '9') &&
                         (ip[i] != ':');
        if (bFlag) {
            return false;
        }
        count_bit++;  // 如果字符合法，则位数加1
        if (ip[i] == ':') {
            count_bit = 0;  // 位数清0，重新计数
            count_colon++;  // 冒号数加1
        }
        if (count_bit > IP_NUM_4) {
            return false;
        }
    }
    if (count_colon > IP_NUM_7) {
        return false;
    }
    return true;
}

/* ------------------------------------------------------------
Description  : 检查IP是否为IPv6或IPv4格式
Input        : ip -- 需要校验的IP地址
Return       : true: IPv6 or IPv4, false: 主机名或其他
------------------------------------------------------------- */
mp_bool CIP::CheckIsIPv6OrIPv4(const std::string& ip)
{
    if (IsIPV4(ip)) {
        isIPV4 = true;
        return true;
    }
    if (IsIPv6(ip)) {
        isIPV4 = false;
        return true;
    }
    return false;
}

/* ------------------------------------------------------------
Description  : 为IPv6添加(true)或删除(false)符号[], 如果是IPv4, 直接返回.
Input        : ip -- 需要转换的IP地址
               delOrAddSign -- true:  添加符号[]
                               false: 删除符号[]
Return       : true: IPv6, false: IPv4
------------------------------------------------------------- */
mp_string CIP::ParseIPv6(const std::string& ip, bool delOrAddSign)
{
#ifdef SOLARIS
    unsigned int countNum = 0;
    countNum = count(ip.begin(), ip.end(), ':');
    if (countNum < 1) {
        // this is IPV4
        return ip;
    }
#else
    unsigned int countNum = count(ip.begin(), ip.end(), ':');
    if (countNum < 1) {
        // this is IPV4
        return ip;
    }
#endif

    mp_string ipTmp = ip;
    mp_string::size_type pos = ipTmp.find("[");
    if (pos == 0) {
        if (!delOrAddSign) {  // 删除符号[
            ipTmp = ipTmp.substr(pos + 1);
        }
    }
    pos = ipTmp.find("]");

    mp_bool bFlag = mp_string::npos != pos && ipTmp.length() - 1 == pos;
    if (bFlag) {
        if (!delOrAddSign) {  // 删除符号]
            ipTmp = ipTmp.substr(0, pos);
        }
    }
    COMMLOG(OS_LOG_DEBUG, "ipTmp:%s", ipTmp.c_str());
    return ipTmp;
}

// nginxListenPort -- 类似“listen       100.136.139.220:59526 ssl;”输入
mp_void CIP::GetListenPort(const mp_string& nginxListenPort, mp_string& strPort)
{
    strPort = "";
    mp_string::size_type iColPos = nginxListenPort.find_last_of(CHAR_COLON);
    if (iColPos != mp_string::npos) {
        mp_size iColSpacePos = nginxListenPort.find_first_of(STR_SPACE, iColPos + 1);  // 找到空格位置
        if (iColSpacePos != mp_string::npos) {
            strPort = nginxListenPort.substr(iColPos + 1, iColSpacePos - iColPos);
        }
    }

    strPort = CMpString::Trim(strPort);
}

/* ------------------------------------------------------------
Description  : nginx/conf/nginx.conf中提取Agent监听的IP和Port
Input        : ip -- 返回获取的IP  port -- 返回获取的port
Return       : MP_FAILED: 获取失败， MP_SUCCESS：获取成功
------------------------------------------------------------- */
mp_int32 CIP::GetListenIPAndPort(mp_string& strIP, mp_string& strPort)
{
    mp_string strNginxConfFile = CPath::GetInstance().GetNginxConfFilePath(AGENT_NGINX_CONF_FILE);
    if (!CMpFile::FileExist(strNginxConfFile)) {
        COMMLOG(OS_LOG_ERROR,
            "GetIPAddress: Nginx config file does not exist, path is \"%s\"",
            AGENT_NGINX_CONF_FILE.c_str());
        return MP_FAILED;
    }

    vector<mp_string> vecRlt;
    mp_int32 iRet = CMpFile::ReadFile(strNginxConfFile, vecRlt);
    if (iRet != MP_SUCCESS || vecRlt.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "Read nginx config file failed, iRet = %d, size of vecRlt is %d.", iRet, vecRlt.size());
        return MP_FAILED;
    }

    for (mp_uint32 i = 0; i < vecRlt.size(); i++) {
        mp_string strTmp = vecRlt[i];
        mp_string::size_type pos = strTmp.find(BIND_IP_TAG, 0);
        if (pos != mp_string::npos) {
            pos += strlen(BIND_IP_TAG.c_str());
            mp_string::size_type iColPos = strTmp.find_last_of(CHAR_COLON);
            if (iColPos != mp_string::npos) {
                strIP = strTmp.substr(pos, iColPos - pos);
                strIP = CMpString::Trim(strIP);
                strIP = CIP::ParseIPv6(strIP, false);
            } else {
                strIP = ANY_IP;
            }
            GetListenPort(strTmp, strPort);
            break;
        }
    }
    mp_string deploytypeEnv;
    GetHostEnv(HOST_ENV_DEPLOYTYPE, deploytypeEnv);
    if (deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT ||
        deploytypeEnv == HOST_ENV_DEPLOYTYPE_HYPERDETECT_NO_BRAND ||
        deploytypeEnv ==  HOST_ENV_DEPLOYTYPE_HYPERDETECT_CYBER_ENGINE) {
        mp_string hostnameEnv;
        GetHostEnv(HOST_ENV_HOSTNAME, hostnameEnv);
        hostnameEnv += DOMAIN_NAME_SUFFIX;
        strIP = std::move(hostnameEnv);
    }
    return MP_SUCCESS;
}

std::string CIP::FormatFullUrl(const std::string& fullUrl)
{
    COMMLOG(OS_LOG_DEBUG, "start to format ip: %s", fullUrl.c_str());
#ifdef SOLARIS
    unsigned int countNum = 0;
    countNum = count(fullUrl.begin(), fullUrl.end(), ':');
#else
    unsigned int countNum = count(fullUrl.begin(), fullUrl.end(), ':');
#endif
    if (countNum <= 1) {
        // this is IPV4
        return fullUrl;
    } else {
        // this is IPV6
        if (fullUrl.find('[') != std::string::npos) {
            return fullUrl;
        }
        std::string ipv6str = "[" + fullUrl + "]";
        return ipv6str;
    }
}

mp_int32 CIP::CheckIsDoradoEnvironment(mp_bool& isDorado)
{
    mp_string sceneType;
    mp_int32 iRet = GetInstallScene(sceneType);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Get InstallScene type failed");
        return MP_FAILED;
    }
    if (sceneType == CIP_IN_AGENT_TYPE) {
        mp_string environmentType;
        iRet = GetBuildINEnvironmentType(environmentType);
        if (iRet != MP_SUCCESS) {
            ERRLOG("Get BuildINEnvironment type failed");
            return MP_FAILED;
        }
        if (environmentType == CIP_DORADO_ENVIRONMENT) {
            isDorado = true;
            return MP_SUCCESS;
        }
    }
    isDorado = false;
    return MP_SUCCESS;
}

mp_int32 CIP::GetInstallScene(mp_string& strSceneType)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_bool iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The testcfg.tmp file does not exist.");
        return MP_FAILED;
    }

    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    mp_string line;
    mp_string strSceneText;
    mp_string strText = "BACKUP_SCENE=";

    if (!stream.is_open()) {
        ERRLOG("The testcfg.tmp file can't open");
        return MP_FAILED;
    }

    while (getline(stream, line)) {
        if (line.find(strText.c_str()) != std::string::npos) {
            strSceneText = line;
            break;
        }
    }
    stream.close();

    std::size_t start = strSceneText.find("=", 0);
    if (start == std::string::npos) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    strSceneType = strSceneText.substr(start + 1);

    return MP_SUCCESS;
}

mp_int32 CIP::GetApplications(mp_string& applications)
{
    INFOLOG("Begin query applications");
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_bool iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The testcfg.tmp file does not exist.");
        return MP_FAILED;
    }
 
    std::ifstream stream;
    stream.open(strFilePath.c_str(), std::ifstream::in);
    mp_string line;
    mp_string strApplicationText;
    mp_string strText = "APPLICATION_INFO=";
 
    if (!stream.is_open()) {
        ERRLOG("The testcfg.tmp file can't open");
        return MP_FAILED;
    }
    while (getline(stream, line)) {
        if (line.find(strText.c_str()) != std::string::npos) {
            strApplicationText = line;
            break;
        }
    }
    stream.close();
    std::size_t start = strApplicationText.find("=", 0);
    if (start == std::string::npos) {
        ERRLOG("The testcfg.tmp file format error");
        return MP_FAILED;
    }
    applications = strApplicationText.substr(start + 1);
 
    return MP_SUCCESS;
}

mp_int32 CIP::GetHostEnv(const mp_string& strType, mp_string& strEnv)
{
    char *tmpEnv = getenv(strType.c_str());
    if (tmpEnv == nullptr) {
        return MP_FAILED;
    }
    strEnv = tmpEnv;
    return MP_SUCCESS;
}

mp_int32 CIP::GetBuildINEnvironmentType(mp_string& environmentType)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_int32 iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The testcfg.tmp file does not exist.");
        return MP_FAILED;
    }
    std::vector<mp_string> confInfovec;
    iRet = CMpFile::ReadFile(strFilePath, confInfovec);
    if (iRet != MP_SUCCESS || confInfovec.empty()) {
        ERRLOG("Read test conf info failed");
        return MP_FAILED;
    }
    for (const auto& item : confInfovec) {
        if (item.find(CIP_ENVIRONMENT_TYPE) != std::string::npos) {
            std::vector<mp_string> EnvironmentTypePair;
            CMpString::StrSplit(EnvironmentTypePair, item, '=');
            if (EnvironmentTypePair.size() < CIP_ENVIRONMENT_TYPE_MINISIZE) {
                ERRLOG("Read test conf EnvironmentType info failed");
                return MP_FAILED;
            }
            environmentType = *(EnvironmentTypePair.rbegin());
            return MP_SUCCESS;
        }
    }
    return MP_FAILED;
}

mp_int32 CIP::GetHostEip(mp_string& eip)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_int32 iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The testcfg.tmp file does not exist.");
        return MP_FAILED;
    }
    std::vector<mp_string> confInfovec;
    iRet = CMpFile::ReadFile(strFilePath, confInfovec);
    if (iRet != MP_SUCCESS || confInfovec.empty()) {
        ERRLOG("Read test conf info failed");
        return MP_FAILED;
    }
    for (const auto& item : confInfovec) {
        if (item.find(EIPINFO) != std::string::npos) {
            std::vector<mp_string> eipPair;
            CMpString::StrSplit(eipPair, item, '=');
            if (eipPair.size() < CIP_ENVIRONMENT_TYPE_MINISIZE) {
                ERRLOG("Read test conf eip info failed");
                return MP_FAILED;
            }
            eip = *(eipPair.rbegin());
            return MP_SUCCESS;
        }
    }
    return MP_FAILED;
}

mp_int32 CIP::GetFloatingIp(mp_string& floatingIp)
{
    if (GetValueFromConfFile(FLOATING_IP, floatingIp) != MP_SUCCESS) {
        ERRLOG("Read floating ip failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CIP::GetAvailableZone(mp_string& az)
{
    if (GetValueFromConfFile(AVAILABLE_ZONE, az) != MP_SUCCESS) {
        ERRLOG("Read available zone failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CIP::GetIsSharedAgent(mp_string& isShared)
{
    if (GetValueFromConfFile(IS_SHARED, isShared) != MP_SUCCESS) {
        ERRLOG("Read available zone failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CIP::GetInstallMode(mp_string& mode)
{
    if (GetValueFromConfFile(INSTALLATION_MODE, mode) != MP_SUCCESS) {
        ERRLOG("Read available zone failed");
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 CIP::GetValueFromConfFile(const mp_string& key, mp_string& val)
{
    mp_string strFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    mp_int32 iRet = CMpFile::FileExist(strFilePath);
    if (iRet != MP_TRUE) {
        ERRLOG("The file %s does not exist.", CFG_RUNNING_PARAM.c_str());
        return MP_FAILED;
    }
    std::vector<mp_string> confInfovec;
    iRet = CMpFile::ReadFile(strFilePath, confInfovec);
    if (iRet != MP_SUCCESS || confInfovec.empty()) {
        ERRLOG("Read file %s failed", CFG_RUNNING_PARAM.c_str());
        return MP_FAILED;
    }
    for (const auto& item : confInfovec) {
        if (item.find(key) != std::string::npos) {
            std::vector<mp_string> keyValPair;
            CMpString::StrSplit(keyValPair, item, '=');
            if (keyValPair.size() < CIP_ENVIRONMENT_TYPE_MINISIZE) {
                ERRLOG("Read key %s from file %s failed", key.c_str(), CFG_RUNNING_PARAM.c_str());
                return MP_FAILED;
            }
            val = *(keyValPair.rbegin());
            return MP_SUCCESS;
        }
    }
    ERRLOG("Key %s does not exists in file %s.", key.c_str(), CFG_RUNNING_PARAM.c_str());
    return MP_FAILED;
}