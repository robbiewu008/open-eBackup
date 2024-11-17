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
#include "host/Cpu.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"
using namespace std;

/*------------------------------------------------------------
Description  : Get cpu architecture
Output       : archMapId --- windows cpu arch mapping key
Return       : MP_TRUE  --- �ɹ�
               MP_FALSE --- ʧ��
-------------------------------------------------------------*/
mp_bool CCpu::GetCpuArch(const mp_string& archMapKey)
{
#if defined WIN32
    if (archMapKey.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "The arch map table is empty.");
        return MP_FALSE;
    }
    std::map<mp_string, mp_string>::iterator it = m_winCpuArchMap.find(archMapKey);
    if (it == m_winCpuArchMap.end()) {
        COMMLOG(OS_LOG_ERROR,
            "Can not get cpu architecture type from m_winCpuArchMap according to key: %s.",
            archMapKey.c_str());
        return MP_FALSE;
    }
    m_cpuArch = (mp_string)(it->second);
#elif defined LINUX
    (mp_void) archMapKey;
    struct utsname buf;
    if (uname(&buf) != 0) {
        COMMLOG(OS_LOG_ERROR, "Get cpu architecture failed.");
        return MP_FALSE;
    }
    m_cpuArch = buf.machine;
#else
    (mp_void) archMapKey;
    m_cpuArch = "not support";
#endif

    COMMLOG(OS_LOG_INFO, "cpu architecture is %s", m_cpuArch.c_str());
    return MP_TRUE;
}

/*------------------------------------------------------------
Description  :Extract Cpu info according to keyword : cpuInfoKey
Output       : Value : the returned value according to cpuInfoKey
Return       : MP_TRUE  --- �ɹ�
               MP_FALSE --- ʧ��
-------------------------------------------------------------*/
mp_bool CCpu::GetCpuInfoValue(const mp_string& infoStr, mp_string& Value)
{
    cpuInfoValue = "";
    if (infoStr.size() == 0 || cpuInfoKey.size() == 0) {
        return MP_FALSE;
    }

    std::size_t found = infoStr.find(cpuInfoKey);
    if (found == std::string::npos) {
        return MP_FALSE;
    }

    std::size_t found2 = infoStr.find(delim, found + cpuInfoKey.size());
    if (found2 == std::string::npos) {
        return MP_FALSE;
    }
    const mp_char skipLeftSpace = 2;
    cpuInfoValue = infoStr.substr(found2 + skipLeftSpace);  // trim left space
    Value = cpuInfoValue;

    return MP_TRUE;
}

mp_uint32 CCpu::StringToUnsignedNumber32(const mp_string& originStr)
{
    mp_uint32 ul;
    std::stringstream ss(originStr, ios_base::in);
    ss >> ul;
    return ul;
}

void CCpu::InitCpuInfo(cpu_info_t& cpu)
{
    cpu.arch = "";
    cpu.cache = "";
    cpu.mhz = "";
    cpu.modelName = "";
    cpu.coreNum = 1;
}

void CCpu::InitWinCpuArchMap()
{
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("0", "x86"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("1", "MIPS"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("2", "Alpha"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("3", "PowerPC"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("5", "ARM"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("6", "ia64"));
    m_winCpuArchMap.insert(std::pair<mp_string, mp_string>("9", "x64"));
}

#ifdef WIN32

const mp_string GET_CPU_INFO_BAT = "cpu_gpu_info.bat";
const mp_string CPU_PARAM = "cpu";

/*------------------------------------------------------------
Description  :�������Cpu��Ϣ��֧��Windows/Linux
Output       : cpus --- �����ѯ����cpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CCpu::GetCpuInfo(vector<cpu_info_t>& cpus)
{
    LOGGUARD("");
    mp_int32 iRettmp = MP_SUCCESS;
    mp_string strScriptName = GET_CPU_INFO_BAT;
    mp_string strScriptParam = CPU_PARAM;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG, "Begin query cpu info.");

    mp_int32 iRet = SecureCom::SysExecScript(strScriptName, strScriptParam, &vecResult);
    if (iRet != MP_SUCCESS) {
        iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,
            "Exec script cpu_gpu_info.bat failed, initial return code is %d,"
            " tranformed return code is %d",
            iRet,
            iRettmp);
        iRet = iRettmp;
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The cpu info from powershell is empty.");
        return iRet;
    }

    AnalyzeCpuInfo(cpus, vecResult);

    if (cpus.empty()) {
        COMMLOG(OS_LOG_ERROR, "Parse cpu info failed.");
        return MP_FAILED;
    } else {
        COMMLOG(OS_LOG_INFO, "Totally get %u cpus.", cpus.size());
    }

    return MP_SUCCESS;
}

void CCpu::AnalyzeCpuInfo(vector<cpu_info_t>& cpus, vector<mp_string>& vecPsResult)
{
    vector<mp_string>::iterator it = vecPsResult.begin();
    cpu_info_t cpu;
    InitCpuInfo(cpu);
    mp_string archMapKey;
    for (; it != vecPsResult.end(); it++) {
        COMMLOG(OS_LOG_DEBUG, "cpu input info is \"%s\"", (*it).c_str());
        SetCpuInfoKey(WIN_CPU_ARCH);
        if (GetCpuInfoValue((*it), archMapKey) == MP_TRUE) {
            if (GetCpuArch(archMapKey) == MP_TRUE) {
                cpu.arch = m_cpuArch;
            } else {
                cpu.arch = "";
            }
        }

        SetCpuInfoKey(WIN_CPU_CLOCK_SPEED);  // MHz
        GetCpuInfoValue((*it), cpu.mhz);
        SetCpuInfoKey(WIN_CPU_NAME);
        GetCpuInfoValue((*it), cpu.modelName);
        SetCpuInfoKey(WIN_CPU_CORES_NUM);
        mp_string coreNumStr;
        if (GetCpuInfoValue((*it), coreNumStr) == MP_TRUE) {
            cpu.coreNum = StringToUnsignedNumber32(coreNumStr);
        }

        SetCpuInfoKey(WIN_CPU_L3_CACHE_SIZE_KB);  // KB
        GetCpuInfoValue((*it), cpu.cache);

        // get a single cpu info, and push it into vector for parse another cpu
        if ((*it).find(WIN_CPU_CONTAINER) != std::string::npos) {
            cpus.push_back(cpu);
            DBGLOG("cpu name: %s, arch: %s, cores: %u, cache: %s, mhz: %s",
                cpu.modelName.c_str(),
                cpu.arch.c_str(),
                cpu.coreNum,
                cpu.cache.c_str(),
                cpu.mhz.c_str());
            InitCpuInfo(cpu);
        }
    }
}

#else
/*------------------------------------------------------------
Description  :�������Cpu��Ϣ��֧��Windows/Linux��
              AIX/Solaris/HPUX��Ҫ֧��ʱ��Ҫ�޸ĸ÷���
Output       : cpus --- �����ѯ����cpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
-------------------------------------------------------------*/
mp_int32 CCpu::GetCpuInfo(vector<cpu_info_t>& cpus)
{
    if (MP_FALSE == GetCpuArch("")) {
        COMMLOG(OS_LOG_ERROR, "Get cpu arch failed, thus stopping to get cup info.");
        return MP_FAILED;
    }

    if (m_cpuArch == ARM_ARCHITECTURE_64 || m_cpuArch == ARM_ARCHITECTURE_32) {  // ARM architecture type "aarch64"
        return GetARMLinuxCpuInfo(cpus);
    }

    if (GetX86LinuxCpuInfo(cpus) == MP_FAILED) {
        return MP_FAILED;
    }

    if (cpus.empty()) {
        COMMLOG(OS_LOG_ERROR, "Parse cpu info failed.");
        return MP_FAILED;
    }

    COMMLOG(OS_LOG_INFO, "Totally get %u cpus info.", cpus.size());
    return MP_SUCCESS;
}

#define GET_CPU_CORES(infoString, tmpString)                                                                           \
    do {                                                                                                               \
        SetCpuInfoKey(LINUX_CPU_CORES);                                                                                \
        if (GetCpuInfoValue(infoString, tmpString) == MP_TRUE) {                                                       \
            cpu.coreNum = StringToUnsignedNumber32(tmpString);                                                         \
            tmpString = "";                                                                                            \
        }                                                                                                              \
    } while (0)

/*------------------------------------------------------------
Description  :Get X86 cpu info by reading from /proc/cpuinfo
              AIX/Solaris/HPUX��Ҫ֧��ʱ��Ҫ�޸ĸ÷���
Output       : cpus --- �����ѯ����cpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
-------------------------------------------------------------*/
mp_int32 CCpu::GetX86LinuxCpuInfo(vector<cpu_info_t>& cpus)
{
    std::ifstream ifs(cpuInfoFile.c_str(), std::ifstream::in);
    if (!ifs.is_open()) {
        COMMLOG(OS_LOG_ERROR, "Open the specified file failed.");
        return MP_FAILED;
    }

    const mp_uint32 LINE_LENGTH = 1024;
    mp_bool isNewCPU = MP_FALSE;
    char buffer[LINE_LENGTH];
    string infoString;
    string phyId = "";
    cpu_info_t cpu;
    InitCpuInfo(cpu);

    while (ifs.getline(buffer, LINE_LENGTH)) {
        infoString = buffer;
        mp_string tmpString;

        SetCpuInfoKey(LINUX_CPU_MODEL_NAME);
        GetCpuInfoValue(infoString, cpu.modelName);

        SetCpuInfoKey(LINUX_CPU_MHZ);
        GetCpuInfoValue(infoString, cpu.mhz);

        SetCpuInfoKey(LINUX_CPU_CACHE_SIZE_KB);
        GetCpuInfoValue(infoString, cpu.cache);

        SetCpuInfoKey(LINUX_CPU_PHY_ID);
        if (GetCpuInfoValue(infoString, tmpString) == MP_TRUE) {
            if (phyId == "" || StringToUnsignedNumber32(phyId) < StringToUnsignedNumber32(tmpString)) {
                isNewCPU = MP_TRUE;
                phyId = tmpString;
            }

            isNewCPU = ((isNewCPU == MP_TRUE) ? MP_TRUE : MP_FALSE);
            tmpString = "";
        }

        GET_CPU_CORES(infoString, tmpString);
        // at least finish analyzing a logical Cpu
        if (infoString.find(LINUX_CPU_POWER_MGT) != std::string::npos) {
            if (isNewCPU == MP_TRUE || phyId == "") {
                cpu.arch = m_cpuArch;
                cpus.push_back(cpu);
                COMMLOG(OS_LOG_DEBUG,
                    "cpu name: %s, arch: %s, cores: %u, cache: %s, mhz: %s",
                    cpu.modelName.c_str(),
                    cpu.arch.c_str(),
                    cpu.coreNum,
                    cpu.cache.c_str(),
                    cpu.mhz.c_str());
                isNewCPU = MP_FALSE;
            }

            InitCpuInfo(cpu);
        }
    }

    return MP_SUCCESS;
}

mp_int32 CCpu::GetARMLinuxCpuInfo(vector<cpu_info_t>& cpus)
{
    // ARM CPU format is not the same as x86
    (mp_void) cpus;
    return MP_FAILED;
}
#endif
