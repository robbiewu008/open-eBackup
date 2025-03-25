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
#include "WinVolumeLivemount.h"
#include "ApplicationServiceDataType.h"
#include "PluginUtilities.h"
#include "file_resource/Win32Handler.h"
#include "FileSystemUtil.h"
#include "VolumeCopyMountProvider.h"

using namespace std;
using namespace FilePlugin;
using namespace PluginUtils;
using namespace Module::FileSystemUtil;
using namespace volumeprotect;
using namespace volumeprotect::mount;

namespace {
    const std::string DEFAULT_MOUNT_PREFIX = ":\\mnt\\DataBackup\\volume_mount";
    const int NUM2 = 2;
}

bool WinVolumeLivemount::PrepareBasicDirectory(const VolumeLivemountExtend& extendInfo)
{
    // 从meta仓中volume.json拿到副本的资源信息
    if (m_metaRepo == nullptr) {
        ERRLOG("meta repo is invalid");
        return false;
    }
    m_volumesMountRecordRoot = PluginUtils::PathJoin(
        PluginUtils::GetPathName(m_cacheRepo->path[0]), "volumelivemount", m_cloneCopyId, "records");
    if (!PluginUtils::CreateDirectory(m_volumesMountRecordRoot)) {
        ERRLOG("Failed to create basic volume mount directory!");
        return false;
    }
    if (extendInfo.fileSystemShareInfo.empty()) {
        ERRLOG("invalid extendInfo!");
        return false;
    }
    m_livemountDetails = extendInfo.fileSystemShareInfo[0].advanceParams.livemountDetail;
    m_defaultMountPrefix = GetWinSystemDrive() + DEFAULT_MOUNT_PREFIX;
    return true;
}

std::string WinVolumeLivemount::GetWinSystemDrive()
{
    std::string drive;
    static char buffer[MAX_PATH];
    DWORD ret = ::GetEnvironmentVariableA("WINDIR", buffer, MAX_PATH);
    if (ret == 0 || ret > MAX_PATH) {
        WARNLOG("Failed get environment variable 'WINDIR', errno: %d", ::GetLastError());
        drive = "";
    } else {
        DBGLOG("'WINDIR' environment variable is: %s", buffer);
        std::string sysDir(buffer);
        drive = sysDir.substr(0, 1);
    }
    if (drive.empty()) {
        drive = "C";
    }
    return drive;
}

std::vector<WinVolumeInfo> WinVolumeLivemount::GetVolumesFromCopy()
{
    // 获取副本中的卷列表
    std::string vols;
    std::vector<WinVolumeInfo> volumes;
    if (!ReadFile(m_volMetaPath, vols)) {
        ERRLOG("Read volume meta filefailed");
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
        volumes.push_back(std::move(vol));
    }
    return volumes;
}

bool WinVolumeLivemount::MountVolumes()
{
    INFOLOG("Enter MountVolumes!");
    std::vector<WinVolumeInfo> cpVols = GetVolumesFromCopy();
    if (cpVols.empty()) {
        ERRLOG("Not found vols in copy!");
        return false;
    }
    bool mntSuccess = false;
    for (const VolumeLivemountDetail& detail : m_livemountDetails) {
        auto volIt = find_if(cpVols.begin(), cpVols.end(), [&detail](const auto& copyVol) {
            DBGLOG("check volume : %s, %s", detail.volumeName.c_str(), Utf16ToUtf8(copyVol.volumeName).c_str());
            return detail.volumeName == Utf16ToUtf8(copyVol.volumeName);
        });
        if (volIt == cpVols.end()) {
            ERRLOG("Not found volume in copy , %s", detail.volumeName.c_str());
            continue;
        }
        std::string dstPath = detail.dstPath.empty() ? GetWinSystemDrive() +
            PathJoin(DEFAULT_MOUNT_PREFIX, detail.volumeName) : detail.dstPath;
        ProcessDstPath(dstPath);
        if (!CheckTargetIsValid(dstPath)) {
            ERRLOG("target is invalid, %s", dstPath.c_str());
            ReportJobLabel(
                JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_invalid_mount_point", dstPath);
            return false;
        }
        bool ret = MountSingleVolume(*volIt, dstPath);
        if (!ret) {
            ERRLOG("Mount Failed For Volume: %s, %s", detail.volumeName.c_str(), detail.dstPath.c_str());
            continue;
        }
        mntSuccess = true;
    }
    return mntSuccess;
}

// 如果用户输入是盘符 W , 改成 W:
void WinVolumeLivemount::ProcessDstPath(std::string& dstPath) const
{
    if (dstPath.length() == 1 && isalpha(dstPath[0])) {
        dstPath = std::toupper(dstPath[0]);
        PluginUtils::StripEscapeChar(dstPath);
        dstPath += ":";
    }
}

bool WinVolumeLivemount::CheckTargetIsValid(const std::string& dstPath)
{
    //  如果指定的目标路径是一个盘符，必须是不存在的盘符
    if (dstPath.length() == NUM2 && isalpha(dstPath[0]) && dstPath[1] == ':') {
        std::string dstDrive = dstPath.substr(0, 1);
        Win32Handler handler;
        std::vector<WinVolumeInfo> envVols = handler.GetAllVolumes();
        for (const WinVolumeInfo& envVol : envVols) {
            if (Utf16ToUtf8(envVol.driveLetter) == dstDrive) {
                ERRLOG("drive letter is already used by system, %s", dstPath.c_str());
                return false;
            }
        }
        return true;
    }
    // 如果指定的目标路径是一个路径，则必须是空路径
    if (IsDirExist(dstPath)) {
        std::vector<std::string> dirList;
        std::vector<std::string> fileList;
        if (!GetDirListInDirectory(dstPath, dirList)) {
            ERRLOG("GetDirListInDirectory failed, %s", dstPath.c_str());
            return false;
        }
        if (!GetFileListInDirectory(dstPath, fileList)) {
            ERRLOG("GetFileListInDirectory failed, %s", dstPath.c_str());
            return false;
        }
        if (!dirList.empty() || !fileList.empty()) {
            ERRLOG("dir not empty. %s", dstPath.c_str());
            return false;
        }
    }
    if (!PluginUtils::CreateDirectory(dstPath)) {
        ERRLOG("CreateDirectory failed, %s", dstPath.c_str());
        return false;
    }
    return true;
}

bool WinVolumeLivemount::MountShare()
{
    std::string driveInfo;
    INFOLOG("Mount Data Repo.");
    if (!MountNasShare(*m_dataRepo.get(), m_nasShareDataMountTarget, m_livemountPara->extendInfo, driveInfo)) {
        return false;
    }
    // agent通过&拼接了挂载点和网络共享
    // 例如：C:\mnt\databackup\general_type\Windows_CIFS_MOUNT\\129.115.135.13&\\129.115.135.13\mount_1697871948854
    size_t pos = driveInfo.find('&');
    if (pos == std::string::npos) {
        ERRLOG("mount point returned invalid: %s", driveInfo);
        return false;
    }
    m_nasShareDataMountTarget = driveInfo.substr(0, pos);
    INFOLOG("set data repo mount target: %s", m_nasShareDataMountTarget.c_str());
    if (!MountNasShare(*m_metaRepo.get(), m_nasShareMetaMountTarget, m_livemountPara->extendInfo, driveInfo)) {
        return false;
    }
    pos = driveInfo.find('&');
    if (pos == std::string::npos) {
        ERRLOG("mount point returned invalid: %s", driveInfo);
        return false;
    }
    m_nasShareMetaMountTarget = driveInfo.substr(0, pos);
    m_volMetaPath = PathJoin(m_nasShareMetaMountTarget, "Volumes.json");
    INFOLOG("set data repo mount target: %s", m_nasShareMetaMountTarget.c_str());
    return true;
}

bool WinVolumeLivemount::MountSingleVolume(const WinVolumeInfo& volume, const std::string& dstPath)
{
    std::string outputDir = PathJoin(m_volumesMountRecordRoot, Utf16ToUtf8(volume.volumeName));
    if (!PluginUtils::CreateDirectory(outputDir)) {
        ERRLOG("Failed to create basic volume mount directory!");
        return false;
    }
    // Perform mount by invoking FS_Backup mount toolkit
    VolumeCopyMountConfig mountConf {};
    mountConf.copyName = DEFAULT_COPY_NAME;
    mountConf.copyMetaDirPath = PathJoin(m_nasShareMetaMountTarget, Utf16ToUtf8(volume.volumeName));
    mountConf.copyDataDirPath = PathJoin(m_nasShareDataMountTarget, Utf16ToUtf8(volume.volumeName));
    mountConf.outputDirPath = outputDir;
    mountConf.mountTargetPath = dstPath;
    mountConf.readOnly = false;
    mountConf.mountFsType = "VHDX_FIXED";
    mountConf.mountOptions = "";
    mountConf.shareName = m_dataRepo->remoteName;
    INFOLOG("mount volume name: %s data path: %s, meta path: %s, record path : %s, target: %s, mountFsType: %s, shareName: %s",
        mountConf.copyName.c_str(), mountConf.copyDataDirPath.c_str(), mountConf.copyMetaDirPath.c_str(),
        mountConf.outputDirPath.c_str(), mountConf.mountTargetPath.c_str(), mountConf.mountFsType.c_str(), mountConf.shareName.c_str());
    std::unique_ptr<VolumeCopyMountProvider> mountProvider = VolumeCopyMountProvider::Build(mountConf);
    if (mountProvider == nullptr) {
        ERRLOG("failed to build volume mount provider");
        return false;
    }
    if (!mountProvider->Mount()) {
        ERRLOG("volume mount failed, message : %s", mountProvider->GetError().c_str());
        return false;
    }
    ReportJobLabel(JobLogLevel::TASK_LOG_INFO, "file_plugin_volume_livemount_volume_mount_success",
        "(" + Utf16ToUtf8(volume.label) + " " + Utf16ToUtf8(volume.driveLetter) + ")", dstPath);
    INFOLOG("volume copy mount successfully, json record path %s", mountProvider->GetMountRecordPath().c_str());
    std::string mountRecordPath = mountProvider->GetMountRecordPath();
    m_mountedRecords.push_back(mountRecordPath);
    return true;
}