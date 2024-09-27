/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Ip.h
 * @brief  The implemention about Ip
 * @version 1.0.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_IP_H__
#define __AGENT_IP_H__

#include "common/Defines.h"
#include <vector>

const mp_string DELIM = ".";
static const mp_int32 MAX_PORT_NUM = 65535;

static const mp_int32 IPV4_NUMERIC_LEN  = 4;
static const mp_int32 IPV6_NUMERIC_LEN  = 16;
static const mp_string BIND_IP_TAG = "listen";
static const mp_string NUM_STRING = "0123456789";
static const mp_string ANY_IP = "0.0.0.0";
class AGENT_API CIP
{
public:
    static bool isIPV4;
    static mp_int32 GetHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
    static mp_bool IsIPV4(const mp_string& strIpAddr);
    static mp_int32 IPV4StrToUInt(mp_string& strIpAddr, mp_uint32& uiIpAddr);
    static mp_int32 IPV4StrToUInt(mp_string& strIpAddr, mp_void* pIpAddr, mp_uint32 uiLen);
    static mp_int32 IPV4UIntToStr(mp_uint32 uiIpAddr, mp_string& strIpAddr);
    static mp_int32 IPV4UIntToStr(mp_void* pIpAddr, mp_string& strIpAddr);
    static mp_int32 IPV6StrToUInt(mp_string& strIpAddr, mp_void* pIpAddr, mp_uint32 uiLen);
    static mp_int32 IPV6UIntToStr(mp_void* pIpAddr, mp_string& strIpAddr);
    static mp_string ParseIPv6(const std::string& ip, bool delOrAddSign);
    static mp_bool CheckIsIPv6OrIPv4(const std::string& strIP);
    static mp_bool IsIPv6(const std::string& strIP);
    static mp_int32 GetListenIPAndPort(mp_string& strIP, mp_string& strPort);
    static std::string FormatFullUrl(const std::string& fullUrl);
    static mp_int32 CheckIsDoradoEnvironment(mp_bool& isDorado);
    static mp_int32 GetBuildINEnvironmentType(mp_string& environmentType);
    static mp_int32 GetInstallScene(mp_string& strSceneType);
    static mp_int32 GetHostEnv(const mp_string& strType, mp_string& strEnv);
    static mp_int32 GetHostEip(mp_string& eip);
    static mp_int32 GetApplications(mp_string& applications);
    static mp_int32 GetFloatingIp(mp_string& floatingIp);
    static mp_int32 GetAvailableZone(mp_string& az);
    static mp_int32 GetIsSharedAgent(mp_string& isShared);
    static mp_int32 GetInstallMode(mp_string& mode);
private:
    static mp_void SplitIPV4(const mp_string& strIpAddr, std::vector<mp_string>& vecOutput);
#ifdef WIN32
    static mp_int32 GetWindowsHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
#elif defined LINUX
    static mp_int32 GetLinuxHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
#else
    static mp_int32 GetUnixHostIPList(std::vector<mp_string>& ipv4List, std::vector<mp_string>& ipv6List);
#endif
    static mp_bool IsNumber(mp_string str);
    static mp_int32 GetUIntIpAddr(mp_string& strIpAddr, mp_uint32& uiIpAddr);
    static mp_void GetListenPort(const mp_string& nginxListenPort, mp_string& strPort);
    static mp_int32 GetValueFromConfFile(const mp_string& key, mp_string& val);
};

#endif  // __AGENT_UNIQUEID_H__
