/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Memory.cpp
 * @brief  Contains function declarations memory
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "host/Memory.h"
#include "common/Log.h"
#include "common/Utils.h"
#include "common/CSystemExec.h"
using namespace std;

#ifdef WIN32
/*------------------------------------------------------------
Description  :获得所有内存信息，支持Windows/Linux;
              获取的物理内存、虚拟内存信息与Windows操作系统systeminfo命令显示内容一致；

Input        : 无
Output       : memInfo --- 保存查询到的内存信息
Return       : MP_SUCCESS --- 成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CMemory::GetMemoryInfo(memory_info_t& memInfo)
{
    BOOL bRet = TRUE;
    mp_int32 iErr = 0;
    mp_char szErr[ERR_INFO_SIZE] = {0};

    COMMLOG(OS_LOG_DEBUG, "Begin get memory info");
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);
    bRet = GlobalMemoryStatusEx (&statex);
    if (!bRet) {
        iErr = GetOSError();
        COMMLOG(OS_LOG_ERROR, "Get global memory status failed, errno[%d]:%s.", iErr,
                GetOSStrErr(iErr, szErr, sizeof(szErr)));
        return MP_FAILED;
    }

    memInfo.uiMemPhyTotal = statex.ullTotalPhys / KBYTES;
    memInfo.uiMemPhyFree = statex.ullAvailPhys / KBYTES;
    memInfo.uiMemPhyUsed = memInfo.uiMemPhyTotal - memInfo.uiMemPhyFree;
    memInfo.uiMemVirTotal = statex.ullTotalPageFile / KBYTES;
    memInfo.uiMemVirFree = statex.ullAvailPageFile / KBYTES;
    memInfo.uiMemVirUsed = memInfo.uiMemVirTotal - memInfo.uiMemVirFree;
    COMMLOG(OS_LOG_DEBUG, "Get memory info succ, phy total %lld, phy free %lld, phy used %lld, "
            "vir total %lld, vir free %lld,  vir used %lld.", memInfo.uiMemPhyTotal, memInfo.uiMemPhyFree,
            memInfo.uiMemPhyUsed, memInfo.uiMemVirTotal, memInfo.uiMemVirFree, memInfo.uiMemVirUsed);

    return MP_SUCCESS;
}

#else
/*------------------------------------------------------------
Description  :获得所有内存信息，支持Windows/Linux；
              AIX/Solaris/HPUX需要支持时需要修改该方法
Linux:
// cat /proc/meminfo | grep -e 'MemTotal' -e 'MemFree'
// MemTotal:         957256 kB
// MemFree:          186964 kB
// SwapTotal:       1534168 kB
// SwapFree:        1532048 kB

Input        : 无
Output       : memInfo --- 保存查询到的内存信息
Return       : MP_SUCCESS --- 成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CMemory::GetMemoryInfo(memory_info_t& memInfo)
{
    mp_string strCmd;
    mp_int32 iRet;
    strCmd = "cat /proc/meminfo | grep -e 'MemTotal' -e 'MemFree' -e 'SwapTotal' -e 'SwapFree'";
    vector<mp_string> vecMemInfo;
    iRet = CSystemExec::ExecSystemWithEcho(strCmd, vecMemInfo);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "CSystemExec::ExecSystemWithEcho failed, strCmd is %s, iRet is %d.",
                strCmd.c_str(), iRet);
        return iRet;
    }

    if (vecMemInfo.empty()) {
        COMMLOG(OS_LOG_ERROR, "Get memory info failed");
        return MP_FAILED;
    }

    for (vector<mp_string>::iterator iter = vecMemInfo.begin(); iter != vecMemInfo.end(); ++iter) {
        iRet = AnalyseMemInfo(*iter, memInfo);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Analyse meminfo failed.");
            return iRet;
        }
    }

    memInfo.uiMemPhyUsed = memInfo.uiMemPhyTotal - memInfo.uiMemPhyFree;
    memInfo.uiMemVirUsed = memInfo.uiMemVirTotal - memInfo.uiMemVirFree;
    return MP_SUCCESS;
}

#define GET_CHECK_MEM_SIZE(errorinfo) do { \
    iRet = GetMemSize(vecStrs[1], uiMemSize);  \
    if (iRet != MP_SUCCESS) {  \
    COMMLOG(OS_LOG_ERROR, errorinfo, \
            strMemInfo.c_str(), vecStrs[1].c_str());  \
    return iRet; \
    } \
} while (0)

/*------------------------------------------------------------
Description  :解析cat meminfo文件获得的内存信息字符串，支持Linux；
              AIX/Solaris/HPUX需要支持时需要适配修改该方法
Linux:
// cat /proc/meminfo | grep -e 'MemTotal' -e 'MemFree'
// MemTotal:         957256 kB
// MemFree:          186964 kB
// SwapTotal:       1534168 kB
// SwapFree:        1532048 kB

Input        : strMemInfo ---  包含内存大小的字符串，类似"MemTotal:         957256 kB"
Output       : memInfo --- 保存解析的内存信息
Return       : MP_SUCCESS --- 成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CMemory::AnalyseMemInfo(mp_string& strMemInfo, memory_info_t& memInfo)
{
    vector<mp_string> vecStrs;
    mp_int32 iRet;

    CMpString::StrSplit(vecStrs, strMemInfo, ':');
    const mp_int32 vecMemInfoSize = 2;
    if (vecStrs.size() != vecMemInfoSize) {
        COMMLOG(OS_LOG_ERROR, "The format of the memory info string is incorrect, meminfo %s.",
                strMemInfo.c_str());
        return MP_FAILED;
    }

    mp_uint64 uiMemSize = 0;
    mp_string strPhyTotal = "MemTotal";
    mp_string strPhyFree = "MemFree";
    mp_string strVirTotal = "SwapTotal";
    mp_string strVirFree = "SwapFree";
    if (vecStrs[0] == strPhyTotal) {
        GET_CHECK_MEM_SIZE("Get total physical memory size failed, mem size str %s, mem size str %s.");
        memInfo.uiMemPhyTotal = uiMemSize;
    } else if (vecStrs[0] == strPhyFree) {
        GET_CHECK_MEM_SIZE("Get free physical memory size failed, mem size str %s, mem size str %s.");
        memInfo.uiMemPhyFree = uiMemSize;
    } else if (vecStrs[0] == strVirTotal) {
        GET_CHECK_MEM_SIZE("Get total virtual memory size failed, mem size str %s, mem size str %s.");
        memInfo.uiMemVirTotal = uiMemSize;
    } else if (vecStrs[0] == strVirFree) {
        GET_CHECK_MEM_SIZE("Get free virtual memory size failed, mem size str %s, mem size str %s.");
        memInfo.uiMemVirFree = uiMemSize;
    } else {
        COMMLOG(OS_LOG_ERROR, "The key of memory info is incorrect, mem size str %s.", strMemInfo.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :解析cat meminfo文件获得的内存信息字符串；
Input        : strMemInfo --- 包含内存大小的字符串，类似"957256 kB"
Output       : uiMemSize --- 保存解析的内存大小
Return       : MP_SUCCESS --- 成功
               非MP_SUCCESS --- 失败
Create By    : yangwenjun 00275736
Modification : 无
-------------------------------------------------------------*/
mp_int32 CMemory::GetMemSize(mp_string& strMemInfo, mp_uint64& uiMemSize)
{
    mp_string str = strMemInfo;

    (mp_void)CMpString::Trim(str.c_str());
    // remove 'kB' in string
    const mp_char skipKB = 3;
    str = str.substr(0, strlen(str.c_str()) - skipKB);
    uiMemSize = (mp_uint64)atol(str.c_str());
    COMMLOG(OS_LOG_DEBUG,  "Get memory size succ, size %lld.", uiMemSize);
    return MP_SUCCESS;
}

#endif

