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
#include "WinVolumeIndex.h"
#include <iostream>
#include <string>
#include "ApplicationServiceDataType.h"
#include "FileSystemUtil.h"
#include "VolumeCommonStruct.h"
#include "VolumeCommonService.h"
#include "VolumeCopyMountProvider.h"
#include "file_resource/Win32Handler.h"
#include "WinVolumeLivemount.h"
#include "WinVolumeRestore.h"
#include "VolumeIndex.h"

using namespace std;
using namespace AppProtect;
using namespace Module;
using namespace PluginUtils;
using namespace FileSystemUtil;
using namespace volumeprotect;
using namespace volumeprotect::mount;

namespace FilePlugin {
namespace {
    const string MODULE = "WinVolumeIndex";
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    const std::string DEFAULT_MOUNT_PREFIX = ":\\mnt\\DataBackup\\volume_mount";
    const std::string VOLUMEINDEX_MOUNT_PATH_ROOT = R"(C:\mnt\databackup\volume_mount)";
    const std::string VOLUMES_JSON = "volumes.json";
    const std::string VOLUME_INDEX = "volumeindex";
    const std::string RECORDS = "records";
    constexpr auto RFI = "rfi";
}
std::mutex mtx;
void WinVolumeIndex::ProcessVolumeScan()
{
    // extract mount config from json
    INFOLOG("Enter ProcessVolumeScan");
    if (!PrepareBasicDirectory()) {
        ERRLOG("failed to extract mount config from json, jobId %s", m_indexPara->jobId.c_str());
    }

    INFOLOG("Enter MountVolumes");
    if (!MountVolumes()) {
        ERRLOG("failed to mount all volumes, jobId %s", m_indexPara->jobId.c_str());
        CleanIndexMounts();
        WinCleanIndexMounts();
        ReportJob(SubJobStatus::FAILED);
        return;
    }

    // scan volume
    if (!VolumeScan()) {
        ERRLOG("failed to scan all volumes, jobId %s", m_indexPara->jobId.c_str());
        ReportJob(SubJobStatus::FAILED);
        return;
    }
    // success
    CleanIndexMounts();
    WinCleanIndexMounts();
    INFOLOG("Scan Finish!");
    return;
}

void WinVolumeIndex::WinCleanIndexMounts()
{
    // 扫描任务完成后卸载卷和共享
    INFOLOG("Enter CleanWinIndexMounts, jobId %s", m_indexPara->jobId.c_str());
    std::vector<WinVolumeInfo> cpVols = GetVolumesFromCopyForInd();
    for (const VolumeIndexDetail& detail : m_indexDetails) {
        auto volIt = find_if(cpVols.begin(), cpVols.end(), [&detail](const auto& copyVol) {
            return detail.volumeName == Utf16ToUtf8(copyVol.volumeName);
        });
        if (volIt == cpVols.end()) {
            ERRLOG("Not found volume in copy , %s", detail.volumeName.c_str());
            continue;
        }
        string mountDir = "volumeindex" + Utf16ToUtf8(((*volIt).driveLetter).substr(0, 1));
        std::string cleanTargetPath = PluginUtils::GetWinSystemDriveForInd() + PathJoin(DEFAULT_MOUNT_PREFIX, mountDir);
        PluginUtils::Remove(cleanTargetPath);
        INFOLOG("clean TargetPath for mount: %s", cleanTargetPath.c_str());
    }
}

bool WinVolumeIndex::VolumeScan()
{
    INFOLOG("Enter VolumeScan");
    for (const std::string& volumePath : m_volumesMountPaths) {
        INFOLOG("volume ScanPath: %s", volumePath.c_str());
        std::string volumeName = volumePath.substr(volumePath.find_last_of(dir_sep) + 1);
        FillScanConfigMetaPath(volumeName);
        FillScanConfigForScan();
        m_scanner = ScanMgr::CreateScanInst(m_scanConfig);
        if (m_scanner == nullptr) {
            ERRLOG("Scanner initiate failed!");
            return false;
        }
        std::string prefix = PluginUtils::GetPathName(volumePath);
        m_scanner->Enqueue(volumePath, volumePath);
        if (SCANNER_STATUS::SUCCESS != m_scanner->Start()) {
            ERRLOG("Start scanner instance failed!");
            if (m_scanner != nullptr) {
                m_scanner->Destroy();
                m_scanner.reset();
            }
            return false;
        }
        MonitorScanner();
    }
    return true;
}

bool WinVolumeIndex::SetupCopiesRepo()
{
    HCP_Log(DEBUG, MODULE) << "Enter GetRepoInfo" << HCPENDLOG;
    if (m_indexPara->copies.empty()) {
        HCP_Log(ERR, MODULE) << "the copies info is empty" << HCPENDLOG;
        return false;
    }
    for (unsigned int i = 0; i < m_indexPara->copies[0].repositories.size(); i++) {
        const auto& repo = m_indexPara->copies[0].repositories[i];
        if (repo.repositoryType == RepositoryDataType::type::DATA_REPOSITORY) {
            m_dataRepo = std::make_shared<StorageRepository>(repo);
            m_cloneCopyId = m_indexPara->copies[0].id;
        } else if (repo.repositoryType == RepositoryDataType::type::CACHE_REPOSITORY) {
            m_cacheRepo = std::make_shared<StorageRepository>(repo);
        } else if (repo.repositoryType == RepositoryDataType::type::META_REPOSITORY) {
            m_metaRepo = std::make_shared<StorageRepository>(repo);
        } else if (repo.repositoryType == RepositoryDataType::type::INDEX_REPOSITORY) {
            m_indexRepo = std::make_shared<StorageRepository>(repo);
        }
    }
    if (m_dataRepo == nullptr
        || m_cacheRepo == nullptr
        || m_metaRepo == nullptr
        || m_indexRepo == nullptr
        || (m_cacheRepo->path).empty()) {
        ERRLOG("data repo or cache repo invalid, jobId: %s, size of index path: %d, size of cache path: %d",
            m_indexPara->jobId.c_str(), m_cacheRepo->path.size());
        return false;
    }
    return true;
}

bool WinVolumeIndex::PrepareBasicDirectory()
{
    // 从meta仓中volume.json拿到副本的资源信息
    m_volumesMountRecordRoot = PluginUtils::PathJoin(
        PluginUtils::GetPathName(m_cacheRepo->path[0]), VOLUME_INDEX, m_indexPara->jobId, RECORDS);
    if (!PluginUtils::CreateDirectory(m_volumesMountRecordRoot)) {
        ERRLOG("Failed to create basic volume mount directory!");
        return false;
    }

    std::vector<VolumeIndexDetail> indexDetail;
    for (unsigned int i = 0; i < m_indexPara->copies.size(); i++) {
        for (auto protectSubObject : m_indexPara->copies[i].protectSubObjects) {
            VolumeIndexDetail volumeIndexDetail {};
            volumeIndexDetail.volumeName = protectSubObject.name;
            volumeIndexDetail.dstPath = "";
            indexDetail.push_back(volumeIndexDetail);
        }
    }
    m_volMetaPath = PluginUtils::PathJoin(m_metaFsPath, VOLUMES_JSON);
    INFOLOG("volMetaPath: %s", m_volMetaPath.c_str());
    m_indexDetails = indexDetail;
    m_defaultMountPrefix = PluginUtils::GetWinSystemDriveForInd() + DEFAULT_MOUNT_PREFIX;
    return true;
}

std::vector<WinVolumeInfo> WinVolumeIndex::GetVolumesFromCopyForInd()
{
    // 获取副本中的卷列表
    std::string vols;
    std::vector<WinVolumeInfo> volumes;
    if (!ReadFile(m_volMetaPath, vols)) {
        ERRLOG("Read volume meta filefailed");
        return volumes;
    }

    Json::Value volsJson;
    try {
        if (!Module::JsonHelper::JsonStringToJsonValue(vols, volsJson)) {
            ERRLOG("Parse volumes.json failed");
            return volumes;
        }
    } catch (const std::exception& e) {
        // 捕获标准库异常
        ERRLOG("Exception caught: %s", e.what());
        return volumes;
    } catch (...) {
        // 捕获其他任何异常
        ERRLOG("Unknown exception caught during JSON parsing");
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

std::string WinVolumeIndex::GetWinSystemDriveForInd()
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


bool WinVolumeIndex::MountVolumes()
{
    INFOLOG("Enter MountVolumes");
    std::vector<WinVolumeInfo> cpVols = GetVolumesFromCopyForInd();
    if (cpVols.empty()) {
        ERRLOG("Not found vols in copy!,errcode: %d", ::GetLastError());
        return false;
    }
    bool mntSuccess = false;
    for (const VolumeIndexDetail& detail : m_indexDetails) {
        auto volIt = find_if(cpVols.begin(), cpVols.end(), [&detail](const auto& copyVol) {
            return detail.volumeName == Utf16ToUtf8(copyVol.volumeName);
        });
        if (volIt == cpVols.end()) {
            ERRLOG("Not found volume in copy , %s", detail.volumeName.c_str());
            continue;
        }
        if ((*volIt).volumeName == FileSystemUtil::Utf8ToUtf16(EFI_SYSTEM_PARTITION)) {
            continue; // EFI system partition
        } else if ((*volIt).partitionName == L"Recovery" && (*volIt).driveLetter == L"Unknown") {
            continue; // recovery
        }
        string mountDir = "volumeindex" + Utf16ToUtf8(((*volIt).driveLetter).substr(0, 1));
        std::string dstPath = PluginUtils::GetWinSystemDriveForInd() + PathJoin(DEFAULT_MOUNT_PREFIX, mountDir,
            Utf16ToUtf8(((*volIt).driveLetter).substr(0, 1)));
        ProcessDstPathForInd(dstPath);
        if (!CheckTargetIsValidForInd(dstPath)) {
            ERRLOG("target is invalid, %s", dstPath.c_str());
            return false;
        }
        bool ret = MountSingleVolumeForInd(*volIt, dstPath);
        if (!ret) {
            ERRLOG("Mount Failed For Volume: %s, %s", detail.volumeName.c_str(), dstPath.c_str());
            return false;
        }
        mntSuccess = true;
    }
    return mntSuccess;
}

// 如果用户输入是盘符 W , 改成 W:
void WinVolumeIndex::ProcessDstPathForInd(std::string& dstPath) const
{
    if (dstPath.length() == 1 && isalpha(dstPath[0])) {
        dstPath = std::toupper(dstPath[0]);
        dstPath += ":";
    }
}

bool WinVolumeIndex::MountSingleVolumeForInd(const WinVolumeInfo& volume, const std::string& dstPath)
{
    std::string outputDir = PathJoin(m_volumesMountRecordRoot, Utf16ToUtf8(volume.volumeName));
    if (!PluginUtils::CreateDirectory(outputDir)) {
        ERRLOG("Failed to create basic volume mount directory!");
        return false;
    }
    // Perform mount by invoking FS_Backup mount toolkit
    VolumeCopyMountConfig mountConf {};
    mountConf.copyName = DEFAULT_COPY_NAME;
    mountConf.copyMetaDirPath = PathJoin(m_metaFsPath, Utf16ToUtf8(volume.volumeName));
    mountConf.copyDataDirPath = PathJoin(m_curRepo->path[0], Utf16ToUtf8(volume.volumeName));
    mountConf.outputDirPath = outputDir;
    mountConf.mountTargetPath = dstPath;
    mountConf.readOnly = false;
    mountConf.mountFsType = "VHDX_FIXED";
    mountConf.mountOptions = "";
    INFOLOG("mount volume name: %s data path: %s, meta path: %s, record path : %s, target: %s, mountFsType: %s",
        mountConf.copyName.c_str(), mountConf.copyDataDirPath.c_str(), mountConf.copyMetaDirPath.c_str(),
        mountConf.outputDirPath.c_str(), mountConf.mountTargetPath.c_str(), mountConf.mountFsType.c_str());
    std::unique_ptr<VolumeCopyMountProvider> mountProvider = VolumeCopyMountProvider::Build(mountConf);
    if (mountProvider == nullptr) {
        ERRLOG("failed to build volume mount provider");
        return false;
    }
    if (!mountProvider->Mount()) {
        ERRLOG("volume mount failed, message : %s", mountProvider->GetError().c_str());
        return false;
    }
    INFOLOG("volume copy mount successfully, json record path %s", mountProvider->GetMountRecordPath().c_str());
    std::string mountRecordPath = mountProvider->GetMountRecordPath();
    INFOLOG("peform volume mount (%s) from data dir", volume.volumeName.c_str());
    m_volumesMountPaths.push_back(dstPath);
    return true;
}

bool WinVolumeIndex::CheckTargetIsValidForInd(const std::string& dstPath)
{
    INFOLOG("Enter CheckTargetIsValidForInd: %s", dstPath.c_str());
    // 如果指定的目标路径是一个路径，则必须是空路径
    try {
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
            return true;
        }
    } catch (std::exception& e) {
        ERRLOG("Exception caught when CheckTargetIsValidForInd: %s", e.what());
        return false;
    } catch (...) {
        ERRLOG("Unknown exception caught when thread CheckTargetIsValidForInd ");
        return false;
    }
    if (!PluginUtils::CreateDirectory(dstPath)) {
        ERRLOG("CreateDirectory failed, %s", dstPath.c_str());
        return false;
    }
    return true;
}

bool WinVolumeIndex::CopyPreMetaFileToWorkDir() const
{
    vector<string> output;
    vector<string> errOutput;
    string targetPath = PluginUtils::PathJoin(m_cacheFsPath, META, PREVIOUS);
    string preMetaFilePath = PluginUtils::PathJoin(m_preCacheFsPath, META, LATEST);
    string cmd = "xcopy " + preMetaFilePath + " " + targetPath+ " /E /H /K /Y";
    INFOLOG("copy cmd : %s", cmd.c_str());
    uint32_t errCode;
    int ret = Module::ExecWinCmd(cmd, errCode);
    if (ret == Module::SUCCESS) {
        INFOLOG("copy pre metafile to work dir success");
        return true;
    } else {
        ERRLOG("exec win cmd failed! cmd : %s, error code: %d", cmd.c_str(), errCode);
        return false;
    }
}


void WinVolumeIndex::FillScanConfigForScan()
{
    INFOLOG("Enter FillScanConfig for scan");
    m_scanConfig.jobId = m_indexPara->jobId;
    m_scanConfig.scanType = ScanJobType::FULL;
    m_scanConfig.scanIO = IOEngine::WIN32_IO;
    m_scanConfig.usrData = (void*)this;
    m_scanConfig.lastBackupTime = 0;
    m_scanConfig.useLastBackupTime = false;
    m_scanConfig.scanCheckPointEnable = false;

    if (Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "KEEP_RFI_IN_CACHE_REPO") == "1") {
        INFOLOG("set to keep rfifile, jobId %s", m_indexPara->jobId.c_str());
        m_scanConfig.keepRfiFile = true;
    }

    SetScanHashType();
 
    m_scanConfig.scanResultCb = ScannerCtrlFileCallBack;
    m_scanConfig.scanHardlinkResultCb = ScannerHardLinkCallBack;
    m_scanConfig.mtimeCtrlCb = BackupDirMTimeCallBack;
    m_scanConfig.deleteCtrlCb = BackupDelCtrlCallBack;
    m_scanConfig.maxCommonServiceInstance = 1;
    m_scanConfig.scanCopyCtrlFileSize = Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixCopyCtrlFileSize");
    m_scanConfig.scanCtrlMaxDataSize = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlDataSize");
    m_scanConfig.scanCtrlMinDataSize = Module::ConfigReader::getString(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlDataSize");
    m_scanConfig.scanCtrlFileTimeSec = SCAN_CTRL_FILE_TIMES_SEC;

    m_scanConfig.scanCtrlMaxEntriesFullBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesFullBackup");

    m_scanConfig.scanCtrlMaxEntriesIncBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMaxCopyCtrlEntriesIncBackup");

    m_scanConfig.scanCtrlMinEntriesFullBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesFullBackup");

    m_scanConfig.scanCtrlMinEntriesIncBkup =
        Module::ConfigReader::getInt(PLUGIN_CONFIG_KEY, "PosixMinCopyCtrlEntriesIncBackup");

    m_scanConfig.scanMetaFileSize = volumeprotect::ONE_GB;
    m_scanConfig.triggerTime = PluginUtils::GetCurrentTimeInSeconds();
}
}
