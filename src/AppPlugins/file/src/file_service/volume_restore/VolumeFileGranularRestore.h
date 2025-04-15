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
#ifndef VOLUME_FILE_GRANULAR_RESTORE_H
#define VOLUME_FILE_GRANULAR_RESTORE_H

#include "volume/VolumeCommonService.h"
#include "HostCommonStruct.h"
#include "constant/Defines.h"
#include "Scanner.h"
#include "Backup.h"
#include "file_service/volume_index/VolumeIndex.h"

namespace FilePlugin {

class VolumeFileGranularRestore : public VolumeCommonService {
public:
    VolumeFileGranularRestore() {};
    virtual ~VolumeFileGranularRestore() {};
    EXTER_ATTACK int PrerequisiteJob() override;
    EXTER_ATTACK int GenerateSubJob() override;
    EXTER_ATTACK int ExecuteSubJob() override;
    EXTER_ATTACK int PostJob() override;

    bool InitJobInfo();
    bool InitBasicRepoInfo();
    bool InitRepoInfo();
    bool InitRestoreInfo();
    bool InitInfo();

    inline bool IsAbort() const
    {
        return IsAbortJob() || IsJobPause();
    }

    // inner four basic stage
    int PrerequisiteJobInner();
    int GenerateSubJobInner();
    int ExecuteSubJobInner();
    int PostJobInner();

    // execute subjob inner
    bool InitRestoreSubjobInfo();
    int ExecuteVolumeGranularFilePhraseSubJob();

    bool GenerateAllSubTaskFromDCacheFCache();
    bool GenerateTearDownTask();
    bool GenerateGenerateInfo();

    // manual mount/umount
    void ClearMounts();

    // For Scanner
    std::string GetScanMetaDirPath(const std::string& volumeName) const;
    std::string GetScanOutputCtrlDirPath(const std::string& volumeName) const;
    std::string GetRestoreCtrlDirPath(const std::string& volumeName) const;
    std::string GetRestoreSrcRootPath(const std::string& volumeName) const;
    static bool IsValidControlFile(const std::string& path);
    bool WaitScannerTerminate(const std::string& volumeName);
    bool ReportSubJobToAgent(const std::vector<SubJob> &subJobList, std::vector<std::string> &controlFileList);

    // For Backup
    bool WaitBackupComplete();
    void UpdateSpeedAndReport();
    void UpdateSubBackupStats();
    void FillRestoreConfig(BackupParams& backupParams);
    void SerializeBackupStats(const BackupStats& backupStats, BackupStatistic& backupStatistic) const;
    bool CalcuMainBackupStats(BackupStatistic& mainBackupStats) const;
    std::string GetRestoreTargetPath() const;

protected:
    // execute subjob inner
    virtual bool GenerateSubTaskFromDCacheFCache(const std::string& volumeName);

    // manual mount/umount
    virtual bool SetupMounts();

    // For Scanner
    virtual bool GenerateRestoreExecuteSubJob(const std::vector<std::string>& controlFileList,
        const std::string& volumeName);
    virtual bool InitSubJobInfo(SubJob &subJob, const std::string& ctrlPath, const std::string& volumeName);

    virtual void FillGranularRestoreScanConfig(
        ScanConfig& scanConfig,
        const std::string& metaPath,
        const std::string& outputControlDirPath);
    virtual int ExecuteVolumeGranularTearDownSubJob();

    StorageRepository m_dataFs {};
    std::string m_dataFsPath;
    std::string m_metaFsPath;
    std::string m_cacheFsPath;
    std::string m_dataFsPersistMountTarget;
    std::string m_volumesMountTargetRoot;
    std::string m_volumesMountRecordRoot;
    std::string m_scanMetaPathRoot;
    std::vector<VolumeFileGraunlarRestoreInfo> m_VolumeFileGraunlarRestoreDetails;

    ScanStatistics m_scannerStatistic;
    VolumeFileGranularRestoreInfo m_restoreInfo;
    RestoreReplacePolicy m_coveragePolicy {};
    BackupPhase m_backupPhase;

    std::shared_ptr<Scanner> m_scanner;
    /* immutable fields */
    std::shared_ptr<AppProtect::RestoreJob> m_jobInfoPtr {nullptr};
private:
    size_t m_mainJobRequestId { 0 };

    StorageRepository m_cacheFs {};
    StorageRepository m_metaFs {};

    std::string m_copyId;
    std::string m_scanCtrlPathRoot;
    std::string m_restoreCtrlPathRoot;

    /* mutable fields */
    BackupStatistic m_subBackupStats {};
    std::unique_ptr<FS_Backup::Backup> m_backup {};
    /* Fields Used For Volume Granular Restore  */
    SCANNER_STATUS m_scannerStatus;

    /* Used for reporting job progress log to agent */
    ActionResult m_logResult {};
    SubJobDetails m_logSubJobDetails {};
    LogDetail m_logDetail {};
    std::vector<LogDetail> m_logDetailList;
    static uint32_t m_numberOfSubTask;
};
}

#endif