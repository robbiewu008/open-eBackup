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
#include "WinVolumeRestore.h"
#include <iostream>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <string>
#include "define/Types.h"
#include "log/Log.h"
#include "win32/Registry.h"
#include "win32/BCD.h"
#include "win32/WMI.h"
#include "FileSystemUtil.h"
#include "file_resource/Win32Handler.h"
#include "JsonHelper.h"
#include "common/EnvVarManager.h"

using namespace PluginUtils;
using namespace volumeprotect;
using namespace volumeprotect::task;
using namespace AppProtect;
using namespace Module;

namespace FilePlugin {

namespace {
    const auto MODULE = "WinVolumeRestore";
    const std::string BMR_SUCCEED_FLAG_SUFFIX = "_bmr_succeed";
    constexpr int ERR_DEVICE_IS_BUSY = 32;
    const std::string WIN_VOLUME_RESTORE_TYPE = "1";
    const int EFI = 0; // EFI system partition or bios system reserved
    const int RECOVERY_VOLUME = 1;
    const int SYSTEM_VOLUME = 2;
    const std::string UEFI = "UEFI";
    const std::string BIOS = "BIOS";
    const std::string UNKNOW = "UNKNOW";
    const std::wstring W_EFI_SYSTEM_PARTITION = L"EFI System Partition";
}

int WinVolumeRestore::PrerequisiteJobInner()
{
    DBGLOG("Begin to exeute windows volume restore requisite job: %s", m_jobId.c_str());
    PreInitStateHandles();
    m_nextState = static_cast<int>(FilePlugin::State::STATE_PRE_INIT);
    return RunStateMachine();
}

void WinVolumeRestore::PreInitStateHandles()
{
    m_stateHandles[static_cast<int>(FilePlugin::State::STATE_PRE_INIT)] =
        std::bind(&WinVolumeRestore::PrerequisiteJobInit, this);
    m_stateHandles[static_cast<int>(FilePlugin::State::STATE_PRE_CHECK_BEFORE_RESTORE)] =
        std::bind(&WinVolumeRestore::CheckBeforeRestore, this);
    m_stateHandles[static_cast<int>(FilePlugin::State::STATE_PRE_VOLUME_MAPPING)] =
        std::bind(&WinVolumeRestore::VolumeMapping, this);
    m_stateHandles[static_cast<int>(FilePlugin::State::STATE_PRE_DISMOUNT_VOL)] =
        std::bind(&WinVolumeRestore::DismountVolumes, this);
}

int WinVolumeRestore::PrerequisiteJobInit()
{
    if (!InitInfo()) {
        return Module::FAILED;
    }

    m_nextState = static_cast<int>(FilePlugin::State::STATE_PRE_CHECK_BEFORE_RESTORE);
    return Module::SUCCESS;
}

bool WinVolumeRestore::InitRepoInfo()
{
    bool ret = VolumeRestore::InitRepoInfo();
    m_volMappingPath = PluginUtils::PathJoin(m_cacheFsPath, "vol_mapping.json");
    m_volMetaPath = PluginUtils::PathJoin(m_metaFsPath, "volumes.json");
    return ret;
}

int WinVolumeRestore::CheckBeforeRestore()
{
    if (m_enableBareMetalRestore) {
        if (!IsWinPE()) {  // 开启了BMR开关，但是并非WinPE环境
            ERRLOG("BMR is enabled, but it's not a WinPE environment");
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR, "file_plugin_bmr_not_a_WINPE_label");
            return Module::FAILED;
        }
        std::string bootType = PluginUtils::GetBootTypeForWinPE();
        if (HasEFIPartitionInCopy() && bootType != UEFI) {
            ERRLOG("System boot mode inconsistency, target is not boot by UEFI");
            return Module::FAILED;
        }
        if (!HasEFIPartitionInCopy() && bootType != BIOS) {
            ERRLOG("System boot mode inconsistency, target is not boot by BIOS");
            return Module::FAILED;
        }
        ReportBMRConfigurationLabel();
    }
    m_nextState = static_cast<int>(FilePlugin::State::STATE_PRE_VOLUME_MAPPING);
    return Module::SUCCESS;
}

bool WinVolumeRestore::IsWinPE()
{
    static char buffer[MAX_PATH];
    DWORD ret = ::GetEnvironmentVariableA("WINDIR", buffer, MAX_PATH);
    if (ret == 0 || ret > MAX_PATH) {
        WARNLOG("Failed get environment variable 'WINDIR', errno: %d", ::GetLastError());
    } else {
        DBGLOG("'WINDIR' environment variable is: %s", buffer);
        if (_stricmp(buffer, "X:\\Windows") == 0) {
            INFOLOG("WinPE environment detected, BMR is enabled");
            return true;
        }
    }

    try {
        std::wstring regVal =
            Win32::RegGetString(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control", L"SystemStartOptions");
        DBGLOG("'SystemStartOptions' registry value is: %s", Module::FileSystemUtil::Utf16ToUtf8(regVal).c_str());
        std::transform(regVal.begin(), regVal.end(), regVal.begin(), ::toupper);
        if (regVal.find(L"MININT") != std::wstring::npos) {
            INFOLOG("WinPE environment detected, BMR is enabled");
            return true;
        }
    } catch (const Win32::RegistryError &e) {
        ERRLOG("Failed to get SystemStartOptions from registry, err: %d", e.ErrorCode());
        return false;
    }

    WARNLOG("No WinPE environment detected, BMR is disabled");
    return false;
}

int WinVolumeRestore::VolumeMapping()
{
    auto cpVols = GetVolumesFromCopy();  //  副本中的卷
    auto envVols = GetVolumesFromEnv();  //  环境中的卷
    if (cpVols.empty() || envVols.empty()) {
        ERRLOG("No volume needs to be mapped");
        return Module::FAILED;
    }

    for (const auto &vol : cpVols) {
        INFOLOG("Copy volume: %s, size: %llu", FileSystemUtil::Utf16ToUtf8(vol.volumeName).c_str(), vol.totalSize);
    }
    for (const auto &vol : envVols) {
        INFOLOG("Env volume: %s, size: %llu, type: %d, drivePath: %s",
            FileSystemUtil::Utf16ToUtf8(vol.volumeName).c_str(),
            vol.totalSize,
            vol.partitionType,
            FileSystemUtil::Utf16ToUtf8(vol.drivePath).c_str(),
            FileSystemUtil::Utf16ToUtf8(vol.driveLetter).c_str());
    }

    m_volumesMap.clear();
    bool ret = ManualVolumeMapping(cpVols, envVols);
    if (!ret || m_volumesMap.empty()) {
        return Module::FAILED;
    }

    if (!SaveVolumeMapping()) {
        ERRLOG("Save volume mapping failed");
        return Module::FAILED;
    }

    m_nextState = static_cast<int>(FilePlugin::State::STATE_NONE);
    return Module::SUCCESS;
}

// 裸机恢复场景由程序根据副本中的卷和主机上的卷自动进行匹配，要求环境卷信息和副本卷信息完全一致
bool WinVolumeRestore::AutoVolumeMapping(
    const std::vector<WinVolumeInfo> &copyVols, const std::vector<WinVolumeInfo> &envVols)
{
    for (auto &srcVol : copyVols) {
        auto iter = std::find_if(envVols.begin(), envVols.end(), [&srcVol](const auto &tgtVol) {
            return srcVol.volumeName == tgtVol.volumeName && srcVol.totalSize <= tgtVol.totalSize;
        });
        if (iter == envVols.end()) {
            WARNLOG("Can't find target volume for %s", FileSystemUtil::Utf16ToUtf8(srcVol.volumeName).c_str());
            continue;
        }
        m_volumesMap[srcVol.volumeName] = *iter;
    }

    return true;
}

// 手动恢复场景由用户指定卷的映射关系
bool WinVolumeRestore::ManualVolumeMapping(
    const std::vector<WinVolumeInfo> &copyVols, const std::vector<WinVolumeInfo> &envVols)
{
    bool isWinPE = IsWinPE();
    for (auto &volPair : m_restoreInfoSet) {
        // 检查副本中是否存在用户指定的源卷
        auto src = find_if(copyVols.begin(), copyVols.end(), [&volPair](const auto &copyVol) {
            DBGLOG("check volume: %s, %s",
                volPair.volumeName.c_str(),
                FileSystemUtil::Utf16ToUtf8(copyVol.volumeName).c_str());
            return volPair.volumeName.find(FileSystemUtil::Utf16ToUtf8(copyVol.volumeName)) != std::string::npos;
        });
        if (src == copyVols.end()) {
            ERRLOG("Volume %s not exists in copy", volPair.volumeName.c_str());
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
                "file_plugin_volume_backup_selected_path_not_exist_label", volPair.volumeName.c_str()); // todo
            return false;
        }
        // efi 单独处理
        if (volPair.dataDstPath == EFI_SYSTEM_PARTITION) {
            WinVolumeInfo efiInfo;
            efiInfo.volumeName = FileSystemUtil::Utf8ToUtf16(EFI_SYSTEM_PARTITION);
            efiInfo.drivePath = FileSystemUtil::Utf8ToUtf16(GetEFIDrivePath());
            m_volumesMap[src->volumeName] = efiInfo;
            continue;
        }

        // 检查环境中是否存在用户指定的目标卷
        auto tgt = find_if(envVols.begin(), envVols.end(), [&volPair](const auto &envVol) {
            DBGLOG("check volume: %s", FileSystemUtil::Utf16ToUtf8(envVol.volumeName).c_str());
            return FileSystemUtil::Utf16ToUtf8(envVol.volumeName).find(volPair.dataDstPath) != std::string::npos;
        });
        if (tgt == envVols.end()) {
            ERRLOG("Volume %s not exists in environment", volPair.dataDstPath.c_str());
            ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
                "file_plugin_volume_backup_selected_path_not_exist_label", volPair.dataDstPath.c_str());
            continue;
        }

        // 检查源卷与目标卷的大小
        if (src->totalSize > tgt->totalSize) {
            ERRLOG("Source volume %s is larger than target volume %s",
                volPair.volumeName.c_str(),
                volPair.dataDstPath.c_str());
            return false;
        }
        m_volumesMap[src->volumeName] = *tgt;
    }

    return true;
}

std::vector<WinVolumeInfo> WinVolumeRestore::GetVolumesFromEnv()
{
    Win32Handler win32Handler;
    std::vector<WinVolumeInfo> volumes = win32Handler.GetAllVolumes();
    return volumes;
}

std::vector<WinVolumeInfo> WinVolumeRestore::GetVolumesFromCopy()
{
    // 获取副本中的卷列表
    std::string vols;
    std::vector<WinVolumeInfo> volumes;
    if (!ReadFile(m_volMetaPath, vols)) {
        ERRLOG("Read volume meta filefailed.errcode: %d", GetLastError());
        return volumes;
    }

    Json::Value volsJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(vols, volsJson)) {
        ERRLOG("Parse volumes.json failed");
        return volumes;
    }

    Win32Handler win32Handler;
    for (auto &element : volsJson) {
        StringVolumeInfo strVol;
        if (!Module::JsonHelper::JsonStringToStruct(element.asString(), strVol)) {
            ERRLOG("Parse string volume info failed");
            return volumes;
        }
        WinVolumeInfo vol = win32Handler.ConvertStringVolumeInfo(strVol);
        volumes.push_back(vol);
    }
    return volumes;
}

bool WinVolumeRestore::SaveVolumeMapping()
{
    if (m_volumesMap.empty()) {
        INFOLOG("No volume mapping info, skip save");
        return true;
    }

    Win32Handler win32Handler;
    Json::Value volPairsJson;
    for (auto &vol : m_volumesMap) {
        INFOLOG("check1: %s", FileSystemUtil::Utf16ToUtf8(vol.second.drivePath).c_str());
        StringVolumeInfo strVol = win32Handler.ConvertVolumeInfo(vol.second);
        win32Handler.LogStringVolumeInfo(strVol);

        std::string content;
        if (!Module::JsonHelper::StructToJsonString(strVol, content)) {
            ERRLOG("Convert volume mapping to json failed");
            continue;
        }
        volPairsJson[FileSystemUtil::Utf16ToUtf8(vol.first)] = content;
    }

    Json::FastWriter jsonWriter;
    std::string volsMapping = jsonWriter.write(volPairsJson);
    INFOLOG("save volume mapping: %s", volsMapping.c_str());

    if (!CreateDirectory(GetPathName(m_volMappingPath))) {
        ERRLOG("CreateDirectory %s failed", m_volMappingPath.c_str());
        return false;
    }

    return WriteFile(m_volMappingPath, volsMapping);
}

//  从vol_mapping.json加载m_volumesMap对象
bool WinVolumeRestore::LoadVolumeMapping()
{
    std::string volsMapping;
    if (!ReadFile(m_volMappingPath, volsMapping)) {
        ERRLOG("Read volume mapping failed");
        return false;
    }
    INFOLOG("load volume mapping: %s", volsMapping.c_str());

    Json::Value volPairsJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(volsMapping, volPairsJson)) {
        ERRLOG("Parse vol_mapping.json failed");
        return false;
    }
    Win32Handler win32Handler;

    for (Json::Value::const_iterator it = volPairsJson.begin(); it != volPairsJson.end(); ++it) {
        StringVolumeInfo strVol;
        if (!Module::JsonHelper::JsonStringToStruct((*it).asString(), strVol)) {
            ERRLOG("Parse string volume info failed");
            return false;
        }
        
        WinVolumeInfo vol = win32Handler.ConvertStringVolumeInfo(strVol);
        
        m_volumesMap[FileSystemUtil::Utf8ToUtf16(it.key().asString())] = vol;
    }
    return true;
}

int WinVolumeRestore::DismountVolumes()
{
    INFOLOG("Enter DismountVolumes");
    for (auto &tgtVol : m_volumesMap) {
        DismountVolume(tgtVol.second.volumeName);
    }

    m_nextState = static_cast<int>(State::STATE_NONE);
    INFOLOG("Leave DismountVolumes");
    return Module::SUCCESS;
}

int WinVolumeRestore::DismountVolume(const std::wstring &wDevicePath)
{
    HANDLE hDevice =
        ::CreateFileW(wDevicePath.c_str(), 0, FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, OPEN_EXISTING, 0, nullptr);
    if (hDevice == INVALID_HANDLE_VALUE) {
        ERRLOG("Open volume failed, errno: %d", ::GetLastError());
        return Module::FAILED;
    }
    std::shared_ptr<void> defer(nullptr, [&](...) { ::CloseHandle(hDevice); });

    DWORD bytesReturned = 0;
    if (!::DeviceIoControl(hDevice, FSCTL_LOCK_VOLUME, nullptr, 0, nullptr, 0, &bytesReturned, nullptr)) {
        ERRLOG("Lock volume failed, errno: %d", ::GetLastError());
        return Module::FAILED;
    }

    int ret = Module::SUCCESS;
    if (!::DeviceIoControl(hDevice, FSCTL_DISMOUNT_VOLUME, nullptr, 0, nullptr, 0, &bytesReturned, nullptr)) {
        ERRLOG("Dismount volume failed, errno: %d", ::GetLastError());
        ret = Module::FAILED;
    }

    if (!::DeviceIoControl(hDevice, FSCTL_UNLOCK_VOLUME, nullptr, 0, nullptr, 0, &bytesReturned, nullptr)) {
        ERRLOG("Lock volume failed, errno: %d", ::GetLastError());
        ret = Module::FAILED;
    }

    return ret;
}

bool WinVolumeRestore::ScanVolumesToGenerateTask()
{
    m_volumesMap.clear();
    if (!LoadVolumeMapping()) {
        ERRLOG("Load volume mapping failed");
        return false;
    }

    uint64_t totalSize = 0;
    int volumeCount = 0;
    std::vector<WinVolumeInfo> cpVols = GetVolumesFromCopy();
    for (auto &volPair : m_volumesMap) {
        RestoreInfo restoreInfo;
        restoreInfo.volumeName = FileSystemUtil::Utf16ToUtf8(volPair.first);
        // 把 G: 这种盘符改为 卷路径 //./HarddiskVolume1
        restoreInfo.dataDstPath = FileSystemUtil::Utf16ToUtf8(volPair.second.drivePath);
        restoreInfo.volumeId = FileSystemUtil::Utf16ToUtf8(volPair.second.driveLetter);
        INFOLOG("set data dst path: %s", restoreInfo.dataDstPath.c_str());

        auto it = find_if(
            cpVols.begin(), cpVols.end(), [&](const WinVolumeInfo &vol) { return volPair.first == vol.volumeName; });
        if (it == cpVols.end()) {
            ERRLOG("Volume %s not exist in copy", restoreInfo.volumeName.c_str());
            return false;
        }

        if (!CreateRestoreSubJob(restoreInfo, SUBJOB_TYPE_DATACOPY_VOLUME)) {
            ERRLOG("Create sub job failed, volumeName:%s, dataDstPath:%s",
                restoreInfo.volumeName.c_str(),
                restoreInfo.dataDstPath.c_str());
            return false;
        }

        volumeCount++;
        totalSize += it->totalSize;
    }

    ReportJobLabel(JobLogLevel::TASK_LOG_INFO,
        "file_plugin_volume_restore_scan_data_completed_label",
        std::to_string(volumeCount),
        FormatCapacity(totalSize),
        std::to_string(volumeCount),
        FormatCapacity(totalSize));
    if (!UpdateCopyPhaseStartTime()) {
        ERRLOG("Updated restore start time failed");
        return false;
    }
    return true;
}

bool WinVolumeRestore::RestartSystem()
{
    HANDLE ToHandle;
    TOKEN_PRIVILEGES tkp;

    // 打开本进程访问token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &ToHandle)) {
        ERRLOG("OpenProcessToken failed, errno: %d", ::GetLastError());
        return false;
    }

    // 修改本进程权限
    LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
    tkp.PrivilegeCount = 1;
    tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
    // 通知系统已修改
    AdjustTokenPrivileges(ToHandle, false, &tkp, 0, nullptr, 0);
    if (::GetLastError() != ERROR_SUCCESS) {
        ERRLOG("AdjustTokenPrivileges failed, errno: %d", ::GetLastError());
        return false;
    }
    // 获得权限后关闭计算机,要实现注销或重启则对应EWX_LOGOFF,EWX_REBOOT
    if (!ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0)) {
        ERRLOG("ExitWindowsEx failed, errno: %d", ::GetLastError());
        return false;
    }

    return true;
}

void WinVolumeRestore::HandleRestoreErrorCode()
{
    if (m_volumeBackupErrorCode == ERR_DEVICE_IS_BUSY) {
        ERRLOG("volume %s is busy", m_restoreInfo.dataDstPath.c_str());
        ReportJobLabel(JobLogLevel::TASK_LOG_ERROR,
            "file_plugin_volume_restore_target_volume_busy_label",
            m_restoreInfo.dataDstPath);
    }
}

bool WinVolumeRestore::CheckBMR(const BareMetalRestoreParam& param)
{
    INFOLOG("get restore job type: %s", param.winVolumeRestoreType.c_str());
    if (param.winVolumeRestoreType != WIN_VOLUME_RESTORE_TYPE) {
        INFOLOG("=====This is BARE_METAL_RESTORE=====");
        return true;
    }
    return false;
}

void WinVolumeRestore::PostProcess()
{
    INFOLOG("exec chkdsk cmd : %s", m_restoreInfo.volumeId.c_str());
    std::string drive = m_restoreInfo.volumeId.substr(0, 1);
    if (std::isalpha(drive[0]) != 0) {
        uint32_t retcode = 0;
        std::string cmd = "cmd.exe /c echo Y | chkdsk " + drive + ": /f";
        Module::ExecWinCmd(cmd, retcode);
    }
}

int WinVolumeRestore::ExecuteTearDownVolume()
{
    INFOLOG("Enter Win ExecuteTearDownVolume");
    if (!ReportBackupCompletionStatus()) {
        return Module::FAILED;
    }

    // 记录裸机恢复成功标志，创建到WinPE的agent安装目录下
    if (!m_enableBareMetalRestore) {
        return Module::SUCCESS;
    }

    //  裸机恢复成功后，修改BCD文件
    if (PluginUtils::IsUEFIBoot()) {
        if (!SetBCDFile()) {
            ERRLOG("SetBCDFile for UEFI boot failed");
            return Module::FAILED;
        }
    } else {
        if (!SetPartitionActive()) {
            ERRLOG("SetPartitionActive failed");
            return Module::FAILED;
        }
        if (!SetBCDFileForBios()) {
            ERRLOG("SetBCDFile for BIOS boot failed");
            return Module::FAILED;
        }
    }

    PluginUtils::CreateDirectory("/tmp");
    std::string installHomePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath();  // 默认是X:
    std::string bmrSucceedFlagFilePath = PluginUtils::PathJoin(installHomePath, m_jobId + BMR_SUCCEED_FLAG_SUFFIX);
    std::ofstream bmrSucceedFlagFile(bmrSucceedFlagFilePath);
    bmrSucceedFlagFile.close();
    return Module::SUCCESS;
}

bool WinVolumeRestore::HasEFIPartitionInCopy()
{
    INFOLOG("Enter HasEFIPartitionInCopy");
    auto cpVols = GetVolumesFromCopy();
    for (auto cpVol:cpVols) {
        if (cpVol.volumeName == W_EFI_SYSTEM_PARTITION) {
            return true;
        }
    }
    return false;
}

bool WinVolumeRestore::GetDriveLetterForEfiAndSystemVolume()
{
    std::string volsMapping;
    if (!ReadFile(m_volMappingPath, volsMapping)) {
        ERRLOG("Read volume mapping failed");
        return false;
    }
    INFOLOG("load volume mapping: %s", volsMapping.c_str());

    Json::Value volPairsJson;
    if (!Module::JsonHelper::JsonStringToJsonValue(volsMapping, volPairsJson)) {
        ERRLOG("Parse vol_mapping.json failed, content: %s", volsMapping.c_str());
        return false;
    }
    Win32Handler win32Handler;
    auto cpVols = GetVolumesFromCopy();
    StringVolumeInfo bootStrVol;
    StringVolumeInfo sysStrVol;

    for (auto cpVol:cpVols) {
        if (cpVol.volumeType == SYSTEM_VOLUME) {
            std::string sysVolume = FileSystemUtil::Utf16ToUtf8(cpVol.volumeName);
            INFOLOG("sysVolume: %s", sysVolume.c_str());

            if (volPairsJson.isMember(sysVolume)) {
                Module::JsonHelper::JsonStringToStruct(volPairsJson[sysVolume].asString(), sysStrVol);
                WinVolumeInfo sys_vol = win32Handler.ConvertStringVolumeInfo(sysStrVol);
                m_sysVolumeLetter = sys_vol.driveLetter;
                INFOLOG("System Volume drive letter: %s", FileSystemUtil::Utf16ToUtf8(m_sysVolumeLetter).c_str());
            } else {
                ERRLOG("System Volume not found in vol_mapping.json");
                return false;
            }
        }

        if (cpVol.volumeType == EFI) {
            std::string bootVolume = FileSystemUtil::Utf16ToUtf8(cpVol.volumeName);
            INFOLOG("bootVolume: %s", bootVolume.c_str());

            if (volPairsJson.isMember(bootVolume)) {
                Module::JsonHelper::JsonStringToStruct(volPairsJson[bootVolume].asString(), bootStrVol);
                WinVolumeInfo bootVol = win32Handler.ConvertStringVolumeInfo(bootStrVol);
                m_bootDriveLetter = bootVol.driveLetter;
                m_bootVolPartNumber = win32Handler.GetPartitionNumber(bootVol.drivePath);
                INFOLOG("boot volume drive letter: %s,boot volume PartNumber: %d",
                    FileSystemUtil::Utf16ToUtf8(m_bootDriveLetter).c_str(), m_bootVolPartNumber);
            } else {
                ERRLOG("boot volume not found in vol_mapping.json");
                return false;
            }
        }
    }

    return true;
}

bool WinVolumeRestore::SetBCDFile()
{
    INFOLOG("Enter SetBCDFile");

    if (!GetDriveLetterForEfiAndSystemVolume()) {
        return false;
    }
    
    std::string bcdEfiDriveLetter = Module::FileSystemUtil::Utf16ToUtf8(m_bootDriveLetter);
    INFOLOG("bcdEfiDriveLetter: %s", bcdEfiDriveLetter.c_str());
    std::string setBootManagerDevice = "Bcdedit /set {bootmgr} device partition=" + bcdEfiDriveLetter;
    INFOLOG("setBootManagerDevice: %s", setBootManagerDevice.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(setBootManagerDevice, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setBootManagerDevice failed, errCode:%d", errCode);
        return false;
    }

    std::string setBootManagerPath = "Bcdedit /set {bootmgr} path \\efi\\microsoft\\boot\\bootmgfw.efi";
    ret = Module::ExecWinCmd(setBootManagerPath, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setBootManagerPath failed, errCode:%d", errCode);
        return false;
    }
    std::string sysVolumeLetter = Module::FileSystemUtil::Utf16ToUtf8(m_sysVolumeLetter);
    std::string setDefaultDeivce = "Bcdedit /set {default} device partition=" + sysVolumeLetter;
    ret = Module::ExecWinCmd(setDefaultDeivce, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultDeivce failed, errCode:%d", errCode);
        return false;
    }

    std::string setDefaultOsDeivce = "Bcdedit /set {default} osdevice partition=" + sysVolumeLetter;
    ret = Module::ExecWinCmd(setDefaultOsDeivce, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultOsDeivce failed, errCode:%d", errCode);
        return false;
    }

    std::string setDefaultPath = "Bcdedit /set {default} path \\windows\\system32\\winload.efi";
    ret = Module::ExecWinCmd(setDefaultPath, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultPath failed, errCode:%d", errCode);
        return false;
    }
    return true;
}

bool WinVolumeRestore::SetPartitionActive()
{
    INFOLOG("Enter PartitionActive for BIOS");
    if (!GetDriveLetterForEfiAndSystemVolume()) {
        return false;
    }

    if (m_bootVolPartNumber == 0) {
        ERRLOG("system Reserved PartNumber is invalied");
        return false;
    }

    int diskNum = 0;
    int partNum = m_bootVolPartNumber;
    std::string selectDisk = "select disk " + std::to_string(diskNum);
    std::string selectPart = "select partition " + std::to_string(partNum);
    std::string setActive = "active";
    std::string exitDiskPart = "exit";

    INFOLOG("Set scirpt for set partition active");
    std::string filepath = "X:\\";
    std::string filename = "bcdSetFile.txt";
    filename = filepath + filename;
    std::ofstream bcdSetFile(filename);
    bcdSetFile << selectDisk << std::endl;
    bcdSetFile << selectPart << std::endl;
    bcdSetFile << setActive << std::endl;
    bcdSetFile << exitDiskPart << std::endl;
    bcdSetFile.close();

    std::string cmd = "diskpart /s " + filename;
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret !=  Module::SUCCESS) {
        return false;
    }
    return true;
}

bool WinVolumeRestore::SetBCDFileForBios()
{
    INFOLOG("Enter SetBCDFile for BIOS");

    if (!GetDriveLetterForEfiAndSystemVolume()) {
        return false;
    }

    std::string sysReservedDriveLetter = Module::FileSystemUtil::Utf16ToUtf8(m_bootDriveLetter);
    std::string setBootManagerDevice = "Bcdedit /set {bootmgr} device partition=" + sysReservedDriveLetter;
    INFOLOG("setBootManagerDevice: %s", setBootManagerDevice.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(setBootManagerDevice, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setBootManagerDevice failed, errCode:%d", errCode);
        return false;
    }
    std::string sysVolumeLetter = Module::FileSystemUtil::Utf16ToUtf8(m_sysVolumeLetter);
    std::string setDefaultDeivce = "Bcdedit /set {default} device partition=" + sysVolumeLetter;
    ret = Module::ExecWinCmd(setDefaultDeivce, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultDeivce failed, errCode:%d", errCode);
        return false;
    }

    std::string setDefaultOsDeivce = "Bcdedit /set {default} osdevice partition=" + sysVolumeLetter;
    ret = Module::ExecWinCmd(setDefaultOsDeivce, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultOsDeivce failed, errCode:%d", errCode);
        return false;
    }

    std::string setDefaultPath = "Bcdedit /set {default} path \\windows\\system32\\winload.exe";
    ret = Module::ExecWinCmd(setDefaultPath, errCode);
    if (ret !=  Module::SUCCESS) {
        ERRLOG("setDefaultPath failed, errCode:%d", errCode);
        return false;
    }
    return true;
}

int WinVolumeRestore::PostJobInner()
{
    if (!InitInfo()) {
        return Module::FAILED;
    }

    if (m_enableBareMetalRestore) {
        std::string installHomePath = Module::EnvVarManager::GetInstance()->GetAgentHomePath();  // 默认是X:
        std::string bmrSucceedFlagFilePath = PluginUtils::PathJoin(installHomePath, m_jobId + BMR_SUCCEED_FLAG_SUFFIX);
        if (PluginUtils::IsPathExists(bmrSucceedFlagFilePath)) {
            ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_completed_label");
            ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_bmr_win_check_disk_label");
        } else {
            WARNLOG("succeed flag %s not detected, BMR fail, not reboot system.", bmrSucceedFlagFilePath.c_str());
        }
    }
    INFOLOG("Exit PostJobInner");
    return Module::SUCCESS;
}

}  // namespace FilePlugin