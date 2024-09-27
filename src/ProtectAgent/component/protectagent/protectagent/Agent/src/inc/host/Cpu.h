/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Cpu.h
 * @brief  Contains function declarations CryptAlg Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#ifndef __AGENT_CPU_H__
#define __AGENT_CPU_H__

#include <vector>
#include <map>
#include <sstream>
#include <fstream>
#include <string>
#ifdef LINUX
#include <sys/utsname.h>
#endif
#include "common/Types.h"
#include "common/Defines.h"

#ifdef WIN32
    const mp_string WIN_CPU_ARCH                    = "Architecture";
    const mp_string WIN_CPU_CLOCK_SPEED             = "CurrentClockSpeed";
    const mp_string WIN_CPU_NAME                    = "Name";
    const mp_string WIN_CPU_CORES_NUM               = "NumberOfCores";
    const mp_string WIN_CPU_L3_CACHE_SIZE_KB        = "L3CacheSize";
    const mp_string WIN_CPU_CONTAINER               = "Container";
#else
    const mp_string ARM_ARCHITECTURE_64             = "aarch64";
    const mp_string ARM_ARCHITECTURE_32             = "aarch32";
    const mp_string LINUX_CPU_MODEL_NAME            = "model name";
    const mp_string LINUX_CPU_MHZ                   = "cpu MHz";
    const mp_string LINUX_CPU_CACHE_SIZE_KB         = "cache size";
    const mp_string LINUX_CPU_PHY_ID                = "physical id";
    const mp_string LINUX_CPU_CORES                 = "siblings";  // 包含超线程的cpu cores
    const mp_string LINUX_CPU_POWER_MGT             = "power management";
#endif

typedef struct tag_cpu_info {
    mp_string   arch;
    mp_string   modelName;
    mp_string   mhz;       // unit: MHz
    mp_string   cache;     // unit: KB
    mp_uint32   coreNum;
}cpu_info_t;

class AGENT_API CCpu {
public:

    CCpu()
    {
        delim = ":";
        cpuInfoKey = "";
        m_cpuArch = "";
        cpuInfoFile = "/proc/cpuinfo";
        InitWinCpuArchMap();
    }
    ~CCpu() {}

    mp_int32 GetCpuInfo(std::vector<cpu_info_t>& cpus);

    void SetCpuInfoKey(const mp_string& key) {cpuInfoKey = key;}
    void SetDelim(const mp_string& newDelim) {delim = newDelim;}
    void SetCpuInfoFile(const mp_string& file) {cpuInfoFile = file;}
    void InitCpuInfo(cpu_info_t& cpu);
    void InitWinCpuArchMap();

    mp_bool GetCpuArch(const mp_string& archMapKey);
    mp_bool GetCpuInfoValue(const mp_string& infoStr, mp_string& Value);

    static mp_uint32 StringToUnsignedNumber32(const mp_string& originStr);

#ifdef WIN32
    void AnalyzeCpuInfo(std::vector<cpu_info_t>& cpus,  std::     vector<mp_string>& vecPsResult);
#else
    mp_int32 GetARMLinuxCpuInfo(std::vector<cpu_info_t>& cpus);
    mp_int32 GetX86LinuxCpuInfo(std::vector<cpu_info_t>& cpus);
#endif

private:
    mp_string cpuInfoKey;
    mp_string cpuInfoValue;
    mp_string delim;
    mp_string cpuInfoFile;
    mp_string m_cpuArch;
    std::map<mp_string, mp_string> m_winCpuArchMap;
};

#endif // __AGENT_CPU_H__
