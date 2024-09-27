#ifndef __AGENT_GPU_H__
#define __AGENT_GPU_H__

#include <vector>
#include "common/Types.h"
#include "common/Defines.h"
#include "host/Cpu.h"

#ifdef WIN32
const mp_string WIN32_GPU_NAME = "Name";
const mp_string WIN32_GPU_RAM = "AdapterRAM";
#else
const mp_string LINUX_GPU_VGA_NAME = "controller:";
const mp_string LINUX_GPU_VGA_NAME_NO_DELIM = "controller";
const mp_string LINUX_GPU_VGA_SUB_NAME = "Subsystem";
const mp_string LINUX_GPU_VGA_MEMORY = "Memory";
const mp_string LINUX_GPU_VGA_NON_PREF_MEM = "non-prefetchable";
const mp_string LINUX_GPU_VGA_PREF_MEM = "prefetchable";
#endif

typedef struct tag_gpu_info {
    mp_string controllerName;
#ifdef WIN32
    mp_uint32 gpuRAM;  // KB
#else
    mp_string subSystem;
    mp_uint32 prefMem32;     // KB
    mp_uint32 prefMem64;     // KB
    mp_uint32 nonPrefMem32;  // KB
    mp_uint32 nonPrefMem64;  // KB
#endif
} gpu_info_t;

class AGENT_API CGpu {
public:
    CGpu()
    {
        gpuMemSizeKey = "[size=";
        gpuDeviceIdKey = "\"VGA compatible controller\"";
    }
    ~CGpu()
    {}
    mp_bool GetGpuControllerName(mp_string& ctrlName, const mp_string& gpuInfoStr);
    mp_bool GetGpuMem(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr);
    void SetGpuKey(const mp_string& key)
    {
        gpuDeviceIdKey = key;
    }
    void SetGpuMemSizeKey(const mp_string& key)
    {
        gpuMemSizeKey = key;
    }
    void InitGpuInfo(gpu_info_t& gpuInfo);
    mp_int32 GetGpuInfo(std::vector<gpu_info_t>& gpus);

#ifdef WIN32
    mp_int32 AnalyzeGpuInfo(std::vector<gpu_info_t>& gpus, std::vector<mp_string> vecPowerShellResult);

#else
    mp_int32 GetX86LinuxGpuInfo(std::vector<gpu_info_t>& gpus);
    mp_int32 GetARMLinuxGpuInfo(std::vector<gpu_info_t>& gpus);

private:
    mp_uint32 GetMemSize(const mp_string& gpuInfoStr);
    mp_int32 GetGpuDeviceId();
    mp_int32 ExtractGpuInfo(gpu_info_t& gpuInfo, std::vector<mp_string>& cmdEchoVector);
    mp_bool AnalyzeGpuMemBits(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr, const mp_bool isPrefMem = MP_TRUE);
    mp_bool GetGpuSubSystem(mp_string& gpuSubSystem, const mp_string& gpuInfoStr);

private:
    mp_string cpuArch;
    std::vector<mp_string> gpuIdsVector;

#endif

private:
    mp_string gpuMemSizeKey;
    mp_string gpuDeviceIdKey;
};

#endif  // __AGENT_GPU_H__
