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
#include "LinuxVolumeIndex.h"
#include <iostream>
#include <string>

using namespace std;
using namespace AppProtect;
using namespace Module;

namespace FilePlugin {

namespace {
    const string MODULE = "LinuxVolumeIndex";
    const string RFI = "rfi";
    const char WIN_SEPARATOR = '\\';
    const char POSIX_SEPARATOR = '/';
    const int INDEX_REPORT_INTERVAL = 45;
    const std::string PLUGIN_CONFIG_KEY = "FilePluginConfig";
    const string LIVEMOUNT_RECORD_JSON_NAME = "volumemount.record.json";
    constexpr uint8_t ERROR_POINT_MOUNTED = 201;
    const std::string DEFAULT_COPY_NAME = "volumeprotect";
    const std::string VOLUMEINDEX_MOUNT_PATH_ROOT = "/mnt/databackup/volumeIndex";
    const std::string SYS_BOOT_VOLUME = "boot";
    const std::string ALARM_CODE_INDEXING_FAILED_MOUNT_VOLUME = "0x640340000D";
}

void LinuxVolumeIndex::ProcessVolumeScan()
{
    INFOLOG("Enter Linux ProcessVolumeScan");
    // mount volume-copies and scanner them for generating metafiles
    if (!PrepareBasicDirectory()) {
        ReportJob(SubJobStatus::FAILED);
        return;
    }
    std::shared_ptr<void> defer(nullptr, [&](...) {
        CleanIndexMounts();
    });
    // mount nas share
    if (!MountNasShare()) {
        ERRLOG("mount Linux nas shared failed, jobId: %s", m_indexPara->jobId.c_str());
    }
    // mount volume
    if (!MountVolumes()) {
        ERRLOG("failed to mount all volumes, jobId %s", m_indexPara->jobId.c_str());
    }

    for (const std::string& volumePath : m_volumesMountPaths) {
        std::string volumeName = volumePath.substr(volumePath.find_last_of(dir_sep) + 1);
        FillScanConfigMetaPath(volumeName);
        FillScanConfigForScan();
        m_scanner = ScanMgr::CreateScanInst(m_scanConfig);
        if (m_scanner == nullptr) {
            ERRLOG("Scanner initiate failed!");
            return;
        }
        std::string prefix = PluginUtils::GetPathName(volumePath);
        INFOLOG("Enqueue path: %s, prefix: %s", volumePath.c_str(), prefix.c_str());
        m_scanner->Enqueue(volumePath, prefix);

        if (m_scanner->Start() != SCANNER_STATUS::SUCCESS) {
            ERRLOG("Start scanner instance failed!");
            if (m_scanner != nullptr) {
                m_scanner->Destroy();
                m_scanner.reset();
            }
            return;
        }
        MonitorScanner();
    }
    INFOLOG("Scan Finish!");
    return;
}

bool LinuxVolumeIndex::CopyPreMetaFileToWorkDir() const
{
    vector<string> output;
    vector<string> errOutput;
    string targetPath = PluginUtils::PathJoin(m_cacheFsPath, META, PREVIOUS);
    string preMetaFilePath = PluginUtils::PathJoin(m_preCacheFsPath, META, LATEST);
    string cmd = "cp -ra " + preMetaFilePath + "/* " + targetPath;
    INFOLOG("cp preMetafile cmd : %s", cmd.c_str());
    int ret = runShellCmdWithOutput(INFO, MODULE, 0, cmd, { }, output, errOutput);
    if (ret != Module::SUCCESS) {
        ERRLOG("run shell cmd failed! %s", cmd.c_str());
        for (auto msg : errOutput) {
            ERRLOG("errmsg : %s", msg.c_str());
            return false;
        }
    }
    return true;
}

void LinuxVolumeIndex::FillScanConfigForScan()
{
    INFOLOG("Enter FillScanConfig for scan");
    m_scanConfig.jobId = m_indexPara->jobId;
    m_scanConfig.scanType = ScanJobType::FULL;
    m_scanConfig.scanIO = IOEngine::POSIX;
    m_scanConfig.usrData = (void*)this;
    m_scanConfig.lastBackupTime = 0;
    m_scanConfig.useLastBackupTime = false;
    m_scanConfig.scanCheckPointEnable = false;
    SetScanHashType();
    /* cb */
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