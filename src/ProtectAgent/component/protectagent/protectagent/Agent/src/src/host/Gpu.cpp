/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Gpu.cpp
 * @brief  Contains function declarations GPU INFO
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */
#include "host/Gpu.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "securecom/SecureUtils.h"

using namespace std;

void CGpu::InitGpuInfo(gpu_info_t& gpuInfo)
{
    gpuInfo.controllerName = "";
#ifdef WIN32
    gpuInfo.gpuRAM = 0;
#else
    gpuInfo.subSystem = "";
    gpuInfo.prefMem32 = 0;
    gpuInfo.prefMem64 = 0;
    gpuInfo.nonPrefMem32 = 0;
    gpuInfo.nonPrefMem64 = 0;
#endif
}

#ifdef WIN32
namespace {
const mp_string GET_GPU_INFO_BAT = "cpu_gpu_info.bat";
const mp_string GPU_PARAM        = "gpu";
}
/*------------------------------------------------------------
Description  :��������Կ���Ϣ��֧��Windows/Linux
Output       : gpus --- �����ѯ����gpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
-------------------------------------------------------------*/
mp_int32 CGpu::GetGpuInfo(vector<gpu_info_t>& gpus)
{
    LOGGUARD("");
    mp_string strScriptName = GET_GPU_INFO_BAT;
    mp_string strScriptParam = GPU_PARAM;
    vector<mp_string> vecResult;

    COMMLOG(OS_LOG_DEBUG,  "Begin query gpu info.");

    mp_int32 iRet = SecureCom::SysExecScript(strScriptName, strScriptParam, &vecResult);
    if (iRet != MP_SUCCESS) {
        mp_int32 iRettmp = ErrorCode::GetInstance().GetErrorCode(iRet);
        COMMLOG(OS_LOG_ERROR,  "Exec script cpu_gpu_info.bat failed, initial return code is %d,"
                " tranformed return code is %d", iRet, iRettmp);
        iRet = iRettmp;
        return iRet;
    }

    if (vecResult.empty()) {
        COMMLOG(OS_LOG_ERROR, "The gpu result info is empty.");
        return iRet;
    }

    COMMLOG(OS_LOG_INFO, "Invoke cpu_gpu_info.bat succ.");

    iRet = AnalyzeGpuInfo(gpus, vecResult);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get gpu info from powershell failed.");
        return iRet;
    }

    return MP_SUCCESS;
}

mp_int32 CGpu::AnalyzeGpuInfo(vector<gpu_info_t>& gpus, vector<mp_string> vecPowerShellResult)
{
    vector<mp_string>::iterator it = vecPowerShellResult.begin();
    gpu_info_t gpu;
    mp_int32 iRet = MP_FAILED;
    InitGpuInfo(gpu);

    for (; it != vecPowerShellResult.end(); ++it) {
        if (MP_FALSE == GetGpuMem(gpu, (*it))) {
            COMMLOG(OS_LOG_ERROR, "Get gpu RAM failed.");
            continue;
        }

        if (MP_FALSE == GetGpuControllerName(gpu.controllerName, (*it))) {
            COMMLOG(OS_LOG_ERROR, "Get gpu controller name failed.");
            continue;
        }

        // get a single gpu info, and push it into vector for parse another gpu
        if ((*it).find("Container") != std::string::npos) {
            gpus.push_back(gpu);
            COMMLOG(OS_LOG_INFO, "Get a gpu info, controller name is: %s, RAM is: %u.",
                    gpu.controllerName.c_str(), gpu.gpuRAM);
            InitGpuInfo(gpu);
            iRet = MP_SUCCESS;
        }
    }

    return iRet;
}

mp_bool CGpu::GetGpuMem(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr)
{
    std::size_t found = gpuInfoStr.find(WIN32_GPU_RAM);
    if (found == std::string::npos || found != 0) { // there are may be many "AdapterRAM" string
        return MP_TRUE;
    }

    found = gpuInfoStr.find(":");
    if (found == std::string::npos) {
        gpuInfo.gpuRAM = 0; // can not extract RAM from WMI, or WMI output format is unkonwn
        COMMLOG(OS_LOG_ERROR, "The powershell output info of [AdapterRAM:] is not support.");
        return MP_FALSE;
    }
    const mp_char skipLeftSpace = 2;
    mp_string adapterRamStr = gpuInfoStr.substr(found + skipLeftSpace); // +2 for trim left space

    gpuInfo.gpuRAM = CCpu::StringToUnsignedNumber32(adapterRamStr);
    const mp_int32 killoBytes = 1024;
    gpuInfo.gpuRAM = (mp_uint32)(gpuInfo.gpuRAM / killoBytes); // convert bytes to KB
    COMMLOG(OS_LOG_DEBUG,  "Get a GPU AdapterRAM [%u KB].", gpuInfo.gpuRAM);
    return MP_TRUE;
}

mp_bool CGpu::GetGpuControllerName(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    std::size_t found = gpuInfoStr.find(WIN32_GPU_NAME);
    if (found == std::string::npos || found != 0) { // there are many keys contains "*Name*"
        return MP_TRUE;
    }

    found = gpuInfoStr.find(":");
    if (found == std::string::npos) {
        ctrlName = "";
        COMMLOG(OS_LOG_ERROR, "The powershell output info of [Name:] is not support.");
        return MP_FALSE;
    }
    const mp_char skipLeftSpace = 2;
    ctrlName = gpuInfoStr.substr(found + skipLeftSpace);  // +2 for trim left space
    COMMLOG(OS_LOG_DEBUG,  "Get a GPU controller, its name is %s.", ctrlName.c_str());
    return MP_TRUE;
}

#else
/*------------------------------------------------------------
Description  :��������Կ���Ϣ��֧��Windows/Linux��
              AIX/Solaris/HPUX��Ҫ֧��ʱ��Ҫ�޸ĸ÷���
Output       : gpus --- �����ѯ����gpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
Create By    : zhuyuanjie z00455045
-------------------------------------------------------------*/
mp_int32 CGpu::GetGpuInfo(vector<gpu_info_t>& gpus)
{
    CCpu cpuObj;
    cpuObj.GetCpuArch(cpuArch);

    if (cpuArch == ARM_ARCHITECTURE_64 || cpuArch == ARM_ARCHITECTURE_32) {
        SetGpuKey("\"Display controller\"");
        return GetARMLinuxGpuInfo(gpus);
    } else {
        SetGpuKey("\"VGA compatible controller\"");
        return GetX86LinuxGpuInfo(gpus);
    }
}

/*------------------------------------------------------------
Description  : use cmd "lspci" to get GPU info
Output       : gpus --- �����ѯ����gpu��Ϣ
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
Create By    : zhuyuanjie z00455045
-------------------------------------------------------------*/
mp_int32 CGpu::GetX86LinuxGpuInfo(vector<gpu_info_t>& gpus)
{
    if (MP_SUCCESS != GetGpuDeviceId()) {
        COMMLOG(OS_LOG_ERROR, "Get gpu device id failed.");
        return MP_FAILED;
    }

    vector<mp_string>::iterator it = gpuIdsVector.begin();
    gpu_info_t gpuInfo;
    for (; it != gpuIdsVector.end(); ++it) {
        vector<mp_string> cmdEchoStringVec;
        mp_string cmdStr = "lspci -v -s " + (*it);
        mp_int32 iRet = CSystemExec::ExecSystemWithEcho(cmdStr, cmdEchoStringVec);
        if (iRet != MP_SUCCESS) {
            COMMLOG(OS_LOG_ERROR, "Execute system cmd [%s] with echo failed, ret = %d.", cmdStr.c_str(), iRet);
            return iRet;
        }

        InitGpuInfo(gpuInfo);
        if (MP_FAILED == ExtractGpuInfo(gpuInfo, cmdEchoStringVec)) {
            COMMLOG(OS_LOG_ERROR, "Get gpu info failed.");
            return MP_FAILED;
        }

        gpus.push_back(gpuInfo);
        cmdEchoStringVec.clear();
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : get all GPU [domian:bus:slot] ids, such as 00:0f.0
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
Create By    : zhuyuanjie z00455045
-------------------------------------------------------------*/
mp_int32 CGpu::GetGpuDeviceId()
{
    vector<mp_string> cmdEchoStringVec;
    mp_string cmdStr = "lspci | grep " + gpuDeviceIdKey;

    mp_int32 iRet = CSystemExec::ExecSystemWithEcho(cmdStr, cmdEchoStringVec);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Execute lspci command with echo failed! iRet = %d.", iRet);
        return iRet;
    }

    gpuIdsVector.clear();
    vector<mp_string>::iterator it = cmdEchoStringVec.begin();
    for (; it != cmdEchoStringVec.end(); ++it) {
        std::size_t found = (*it).find(" ");
        if (found != std::string::npos) {
            mp_string gpuIdStr = (*it).substr(0, found);
            gpuIdsVector.push_back(gpuIdStr);
            COMMLOG(OS_LOG_DEBUG,  "Get a GPU device, and its [domian:bus:slot] is %s.", gpuIdStr.c_str());
        }
    }

    if (gpuIdsVector.size() == 0) {
        COMMLOG(OS_LOG_ERROR, "lspci not exists or [lspci | grep %s] find no VGA info.", gpuDeviceIdKey.c_str());
        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  : extract GPU info from the echo of "lspci -v -s 00:0f.0"
Return       : MP_SUCCESS --- �ɹ�
               ��MP_SUCCESS --- ʧ��
Create By    : zhuyuanjie z00455045
-------------------------------------------------------------*/
mp_int32 CGpu::ExtractGpuInfo(gpu_info_t& gpuInfo, vector<mp_string>& cmdEchoVector)
{
    if (cmdEchoVector.empty()) {
        COMMLOG(OS_LOG_ERROR, "lspci -v -s return empty value.");
        return MP_FAILED;
    }

    vector<mp_string>::iterator it = cmdEchoVector.begin();
    for (; it != cmdEchoVector.end(); ++it) {
        if (MP_FALSE == GetGpuControllerName(gpuInfo.controllerName, (*it))) {
            COMMLOG(OS_LOG_ERROR, "Get gpu controller name failed.");
            continue; // continue to parse other gpu info
        }

        if (MP_FALSE == GetGpuSubSystem(gpuInfo.subSystem, (*it))) {
            COMMLOG(OS_LOG_ERROR, "Get gpu subsystem item failed.");
            continue;
        }

        if (MP_FALSE == GetGpuMem(gpuInfo, (*it))) {
            COMMLOG(OS_LOG_DEBUG,  "Get gpu memory info failed.");
            continue;
        }
    }

    return MP_SUCCESS;
}

mp_bool CGpu::GetGpuControllerName(mp_string& ctrlName, const mp_string& gpuInfoStr)
{
    std::size_t found = gpuInfoStr.find(LINUX_GPU_VGA_NAME_NO_DELIM);
    if (found == std::string::npos) {
        return MP_TRUE;
    }

    mp_string devNameKey = LINUX_GPU_VGA_NAME;  // both arm and x86 Linux has the key
    found = gpuInfoStr.find(devNameKey);
    if (found == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "lspci output the info of [controller:] is not support.");
        return MP_FALSE;
    }

    ctrlName = gpuInfoStr.substr(found + devNameKey.size() + 1);  // trim left space
    COMMLOG(OS_LOG_DEBUG,  "Get a GPU controller, its name is %s.", ctrlName.c_str());
    return MP_TRUE;
}

mp_bool CGpu::GetGpuSubSystem(mp_string& gpuSubSystem, const mp_string& gpuInfoStr)
{
    std::size_t found = gpuInfoStr.find(LINUX_GPU_VGA_SUB_NAME);
    if (found == std::string::npos) {
        return MP_TRUE; // not the wanted info
    }

    found = gpuInfoStr.find_first_of(":");
    if (found == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "lspci output the info of [Subsystem:] is not support.");
        return MP_FALSE;
    }

    const mp_char skipLeftSpace = 2;
    mp_string tmpStr = gpuInfoStr.substr(found + skipLeftSpace); // trim left space
    gpuSubSystem = tmpStr;
    COMMLOG(OS_LOG_DEBUG,  "Get a GPU subSystem device is %s.", gpuSubSystem.c_str());
    return MP_TRUE;
}

mp_bool CGpu::GetGpuMem(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr)
{
    mp_bool isPrefMem = MP_FALSE;
    mp_bool isNonPrefMem = MP_FALSE;

    std::size_t found = gpuInfoStr.find(LINUX_GPU_VGA_MEMORY);
    if (found == std::string::npos) {
        return MP_TRUE; // not the wanted info
    }

    // judge the "non-prefetchable" should be executed ahead "prefetchable"
    found = gpuInfoStr.find(LINUX_GPU_VGA_NON_PREF_MEM);
    if (found != std::string::npos) {
        isNonPrefMem = MP_TRUE;
        goto GPU_MEM_BITS;
    }

    found = gpuInfoStr.find(LINUX_GPU_VGA_PREF_MEM);
    if (found != std::string::npos) {
        isPrefMem = MP_TRUE;
        goto GPU_MEM_BITS;
    }

GPU_MEM_BITS:

    if (isPrefMem) {
        return ((AnalyzeGpuMemBits(gpuInfo, gpuInfoStr, MP_TRUE) == MP_FALSE) ? MP_FALSE : MP_TRUE);
    } else if (isNonPrefMem) {
        return ((AnalyzeGpuMemBits(gpuInfo, gpuInfoStr, MP_FALSE) == MP_FALSE) ? MP_FALSE : MP_TRUE);
    }

    COMMLOG(OS_LOG_ERROR, "Not support the GPU format about GPU_MEM_BITS.");
    return MP_FALSE;
}

mp_bool CGpu::AnalyzeGpuMemBits(gpu_info_t& gpuInfo, const mp_string& gpuInfoStr, const mp_bool isPrefMem)
{
    mp_bool is32BitsMem = MP_FALSE;
    mp_bool is64BitsMem = MP_FALSE;

    std::size_t found = gpuInfoStr.find("32-bit");
    if (found != std::string::npos) {
        is32BitsMem = MP_TRUE;
        goto GPU_MEM_SIZE;
    }

    found = gpuInfoStr.find("64-bit");
    if (found != std::string::npos) {
        is64BitsMem = MP_TRUE;
        goto GPU_MEM_SIZE;
    }

GPU_MEM_SIZE:

    mp_uint32 memSize = GetMemSize(gpuInfoStr);
    if (memSize == 0) {
        return MP_FALSE; // error info has print in GetMemSize()
    }

    if (is32BitsMem) {
        if (isPrefMem == MP_TRUE) {
            gpuInfo.prefMem32 += memSize;
        } else {
            gpuInfo.nonPrefMem32 += memSize;
        }
        return MP_TRUE;
    } else if (is64BitsMem) {
        if (isPrefMem == MP_TRUE) {
            gpuInfo.prefMem64 += memSize;
        } else {
            gpuInfo.nonPrefMem64 += memSize;
        }
        return MP_TRUE;
    }

    COMMLOG(OS_LOG_ERROR, "Not support the GPU format about GPU_MEM_SIZE.");
    return MP_FALSE;
}

mp_uint32 CGpu::GetMemSize(const mp_string& gpuInfoStr)
{
    mp_uint32 memUnitConvert = 1;
    const mp_int32 Mbytes = 1024;

    std::size_t found = gpuInfoStr.find(gpuMemSizeKey);
    if (found == std::string::npos) {
        COMMLOG(OS_LOG_ERROR, "Not support the GPU format of [Memory \"%s\"].", gpuMemSizeKey.c_str());
        return 0; // 0 indicates the [Memory] parse failed
    }

    std::size_t found2 = gpuInfoStr.find("M]"); // GPU memory unit in this found is MB
    if (found2 != std::string::npos) {
        memUnitConvert = Mbytes;
    } else {
        found2 = gpuInfoStr.find("K]"); // GPU memory unit in this found is KB
    }

    mp_string memSizeStr = gpuInfoStr.substr(found + gpuMemSizeKey.size(), found2 - found - gpuMemSizeKey.size());
    mp_uint32 memSize = CCpu::StringToUnsignedNumber32(memSizeStr);
    memSize = memSize * memUnitConvert;
    COMMLOG(OS_LOG_DEBUG,  "Get a gpu memory item, size is %d KB.", memSize);
    return memSize;
}

mp_int32 CGpu::GetARMLinuxGpuInfo(vector<gpu_info_t>& gpus)
{
    return GetX86LinuxGpuInfo(gpus);
}

#endif
